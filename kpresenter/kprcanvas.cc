/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qprogressdialog.h>
#include <qdragobject.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qapplication.h>
#include "koPointArray.h"
#include <qpopupmenu.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qdropsite.h>

#include <qrect.h>
#include <qsize.h>
#include <qpoint.h>
#include "styledia.h"
#include "kprcanvas.h"
#include "kprcanvas.moc"

#include <kpresenter_view.h>
#include <qwmf.h>
#include <kpbackground.h>
#include <kpclipartobject.h>
#include <kppixmapobject.h>
#include <gotopage.h>
#include <kptextobject.h>
#include <kpresenter_sound_player.h>
#include <notebar.h>
#include <kppartobject.h>
#include "kpresenter_utils.h"
#include <koparagcounter.h>
#include <kapplication.h>
#include <kmimemagic.h>
#include <kurl.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kprinter.h>
#include <kglobal.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kprcommand.h>
#include <kcursor.h>
#include <koPoint.h>
#include <kozoomhandler.h>
#include <stdlib.h>
#include <qclipboard.h>

#include "kprpage.h"
#include <koRect.h>
#include <math.h>

/******************************************************************/
/* class KPrCanvas - KPrCanvas                                    */
/******************************************************************/

/*====================== constructor =============================*/
KPrCanvas::KPrCanvas( QWidget *parent, const char *name, KPresenterView *_view )
    : QWidget( parent, name, WStaticContents|WResizeNoErase|WRepaintNoErase ), buffer( size() )
{
    presMenu = 0;
    m_currentTextObjectView=0L;
    m_activePage=0L;
    m_xOffset = 0;
    m_yOffset = 0;

    if ( parent ) {
        mousePressed = false;
	drawContour = false;
        modType = MT_NONE;
        resizeObjNum = 0L;
        editNum = 0L;
	rotateNum = 0L;
        setBackgroundMode( Qt::NoBackground );
        m_view = _view;
        setupMenus();
        setMouseTracking( true );
        show();
        editMode = true;
        currPresPage = 1;
        currPresStep = 0;
        subPresStep = 0;
        _presFakt = 1.0;
        goingBack = false;
        drawMode = false;
        fillBlack = true;
        drawRubber = false;
        toolEditMode = TEM_MOUSE;
        tmpObjs.setAutoDelete( false );
        setAcceptDrops( true );
        inEffect = false;
        ratio = 0.0;
        keepRatio = false;
        mouseSelectedObject = false;
        selectedObjectPosition = -1;
        nextPageTimer = true;
        drawLineInDrawMode = false;
        soundPlayer = 0;
        m_drawPolyline = false;
        m_drawCubicBezierCurve = false;
        m_drawLineWithCubicBezierCurve = true;
        m_oldCubicBezierPointArray.putPoints( 0, 4, 0,0, 0,0, 0,0, 0,0 );
    } else {
        m_view = 0;
        hide();
    }

    setFocusPolicy( QWidget::StrongFocus );
    setFocus();
    setKeyCompression( true );
    installEventFilter( this );
    KCursor::setAutoHideCursor( this, true, true );
    m_zoomBeforePresentation=100;
    m_activePage=m_view->kPresenterDoc()->pageList().getFirst();
    connect( m_view->kPresenterDoc(), SIGNAL( sig_terminateEditing( KPTextObject * ) ),
             this, SLOT( terminateEditing( KPTextObject * ) ) );

}

/*======================== destructor ============================*/
KPrCanvas::~KPrCanvas()
{
    // disconnect all signals to avoid crashes on exit
    // (exitEditMode) emits signals
    disconnect( this, 0, 0, 0 );

    // deactivate possible opened textobject to avoid double deletion, KPTextObject deletes this already

    exitEditMode();

    delete presMenu;

    stopSound();
    delete soundPlayer;
}

void KPrCanvas::scrollX( int x )
{
    int oldXOffset = m_xOffset;
    m_xOffset = x;
    scroll( oldXOffset - x, 0 );
}

void KPrCanvas::scrollY( int y )
{
    int oldYOffset = m_yOffset;
    m_yOffset = y;
    scroll( 0, oldYOffset - y );
}

bool KPrCanvas::eventFilter( QObject *o, QEvent *e )
{

    if ( !o || !e )
        return TRUE;
    if ( m_currentTextObjectView  )
        KCursor::autoHideEventFilter( o, e );
    switch ( e->type() )
    {
    case QEvent::AccelOverride:
    {
        QKeyEvent * keyev = static_cast<QKeyEvent *>(e);
        if ( m_currentTextObjectView &&
             (keyev->key()==Key_Home ||keyev->key()==Key_End
              || keyev->key()==Key_Tab || keyev->key()==Key_Prior
                 || keyev->key()==Key_Next) )
        {
            m_currentTextObjectView->keyPressEvent( keyev );
            return true;
        }
    }
    case QEvent::FocusIn:
        if ( m_currentTextObjectView )
            m_currentTextObjectView->focusInEvent();
        return TRUE;
    case QEvent::FocusOut:
        if ( m_currentTextObjectView  )
            m_currentTextObjectView->focusOutEvent();
        return TRUE;
    case QEvent::KeyPress:
    {
#ifndef NDEBUG
        QKeyEvent * keyev = static_cast<QKeyEvent *>(e);
        // Debug keys
        if ( ( keyev->state() & ControlButton ) && ( keyev->state() & ShiftButton ) )
        {
            switch ( keyev->key() ) {
                case Key_P: // 'P' -> paragraph debug
                    printRTDebug( 0 );
                    break;
                case Key_V: // 'V' -> verbose parag debug
                    printRTDebug( 1 );
                    break;
                default:
                    break;
            }
        }
#endif
    }
        break;
    default:
        break;
    }
    return QWidget::eventFilter(o,e);
}

bool KPrCanvas::focusNextPrevChild( bool )
{
    return TRUE; // Don't allow to go out of the canvas widget by pressing "Tab"
}

void KPrCanvas::paintEvent( QPaintEvent* paintEvent )
{
    if ( isUpdatesEnabled() )
    {
        QPainter bufPainter;
        bufPainter.begin( &buffer, this ); // double-buffering - (the buffer is as big as the widget)
        bufPainter.translate( -diffx(), -diffy() );
        bufPainter.setBrushOrigin( -diffx(), -diffy() );

        QRect crect( paintEvent->rect() ); // the rectangle that needs to be repainted, in widget coordinates
        //kdDebug(33001) << "KPrCanvas::paintEvent " << DEBUGRECT( crect ) << endl;
        crect.moveBy( diffx(), diffy() ); // now in contents coordinates
        //kdDebug(33001) << "KPrCanvas::paintEvent after applying diffx/diffy: " << DEBUGRECT( crect ) << endl;

        if ( editMode || !fillBlack )
            bufPainter.fillRect( crect, white );
        else
            bufPainter.fillRect( crect, black );

        drawBackground( &bufPainter, crect );

        SelectionMode selectionMode;

	if ( toolEditMode == TEM_MOUSE )
	    selectionMode = SM_MOVERESIZE;
	else if ( toolEditMode == TEM_ROTATE )
	    selectionMode = SM_ROTATE;
	else
	    selectionMode = SM_NONE;

        if ( !editMode )
            selectionMode = SM_NONE; // case of screen presentation mode
        drawObjects( &bufPainter, crect, true, selectionMode, true );

        bufPainter.end();

        bitBlt( this, paintEvent->rect().topLeft(), &buffer, paintEvent->rect() );
    }
    //else kdDebug(33001) << "KPrCanvas::paintEvent with updates disabled" << endl;
}

/*======================= draw background ========================*/
void KPrCanvas::drawBackground( QPainter *painter, const QRect& rect )
{
    QRegion grayRegion( rect );
    if ( editMode )
    {
        //kdDebug(33001) << "KPrCanvas::drawBackground drawing bg for page " << i+1 << " editMode=" << editMode << endl;
        QRect pageRect = m_activePage->getZoomPageRect();
        //kdDebug() << "KPrCanvas::drawBackground pageRect=" << DEBUGRECT(pageRect) << endl;
        //kdDebug() << "KPrCanvas::drawBackground rect=" << DEBUGRECT(rect) << endl;
        if ( rect.intersects( pageRect ) )
        {
            m_activePage->background()->draw( painter, m_view->zoomHandler(), rect, true );
        }
        // Include the border now
        pageRect.rLeft() -= 1;
        pageRect.rTop() -= 1;
        pageRect.rRight() += 1;
        pageRect.rBottom() += 1;
        grayRegion -= pageRect;

        // In edit mode we also want to draw the gray area out of the pages
        if ( !grayRegion.isEmpty() )
        {
            eraseEmptySpace( painter, grayRegion, QApplication::palette().active().brush( QColorGroup::Mid ) );
        }
    }
    else
    {
        // TODO use m_pageList
        m_view->kPresenterDoc()->pageList().at( currPresPage-1 )->background()->draw( painter, m_view->zoomHandler(), rect, false );
    }
}

// 100% stolen from KWord
void KPrCanvas::eraseEmptySpace( QPainter * painter, const QRegion & emptySpaceRegion, const QBrush & brush )
{
    painter->save();
    painter->setClipRegion( emptySpaceRegion, QPainter::CoordPainter );
    painter->setPen( Qt::NoPen );

    //kdDebug(33001) << "KWDocument::eraseEmptySpace emptySpaceRegion: " << DEBUGRECT( emptySpaceRegion.boundingRect() ) << endl;
    painter->fillRect( emptySpaceRegion.boundingRect(), brush );
    painter->restore();
}

// Draw all object in page : draw object in current page and sticky objects
void KPrCanvas::drawObjectsInPage(QPainter *painter, const KoRect& rect2, bool drawCursor,
				  SelectionMode selectMode, bool doSpecificEffects,
				  const QPtrList<KPObject> & obj)
{
    QPtrListIterator<KPObject> it( obj );
    for ( ; it.current() ; ++it )
    {
        SelectionMode selectionMode=selectMode;
        if ( objectIsAHeaderFooterHidden(it.current()))
            continue;
        //don't draw rotate indicator when we are a header or footer
        if( m_view->kPresenterDoc()->isHeaderFooter(it.current()))
            selectionMode=SM_HEADERFOOTER;

	if ( it.current()->isSticky() || editMode ||
	     ( rect2.intersects( it.current()->getBoundingRect(m_view->zoomHandler() ) ) && editMode ) ||
	     ( !editMode &&
	       it.current()->getPresNum() <= static_cast<int>( currPresStep ) &&
	       ( !it.current()->getDisappear() || it.current()->getDisappear() &&
		 it.current()->getDisappearNum() > static_cast<int>( currPresStep ) ) ) )

        {
 	    if ( inEffect && it.current()->getPresNum() >= static_cast<int>( currPresStep ) )
 		continue;

	    if ( !editMode && doSpecificEffects && static_cast<int>( currPresStep ) == it.current()->getPresNum() && !goingBack ) {
                //kdDebug(33001) << "                 setSubPresStep " << subPresStep << endl;
		it.current()->setSubPresStep( subPresStep );
		it.current()->doSpecificEffects( true, false );
	    }
            //kdDebug(33001) << "                 drawing object at " << diffx() << "," << diffy() << "  and setting subpresstep to 0 !" << endl;
            if ( drawCursor && it.current()->getType() == OT_TEXT && m_currentTextObjectView )
            {
                KPTextObject* textObject = static_cast<KPTextObject*>( it.current() );
                if ( m_currentTextObjectView->kpTextObject() == textObject ) // This is the object we are editing
                {
                    textObject->paintEdited( painter,m_view->zoomHandler(),
                                             false /*onlyChanged. Pass as param ?*/,
                                             m_currentTextObjectView->cursor(), true /* idem */ );
                }
		else
                    it.current()->draw( painter, m_view->zoomHandler(), selectionMode,
					((it.current())->isSelected()) && drawContour );
            }
            else
            {
                it.current()->draw( painter, m_view->zoomHandler(), selectionMode,
				    ((it.current())->isSelected()) && drawContour );

            }
	    it.current()->setSubPresStep( 0 );
	    it.current()->doSpecificEffects( false );
	}
    }

}

/*========================= draw objects =========================*/
void KPrCanvas::drawObjects( QPainter *painter, const QRect& rect, bool drawCursor,
			     SelectionMode selectionMode, bool doSpecificEffects )
{
    int pgNum = editMode ? (int)m_view->getCurrPgNum() : currPresPage;
    KoRect rect2 = m_view->zoomHandler()->unzoomRect(rect);

    //objects in current page
    drawObjectsInPage( painter, rect2, drawCursor, selectionMode, doSpecificEffects,
		       m_view->kPresenterDoc()->pageList().at(pgNum-1)->objectList());

    //draw sticky object
    drawObjectsInPage( painter, rect2, drawCursor, selectionMode, doSpecificEffects,
		       m_view->kPresenterDoc()->stickyPage()->objectList());

}

/*================================================================*/
// This one is used to generate the pixmaps for the HTML presentation,
// for the pres-structure-dialog, for the sidebar previews, for template icons.
void KPrCanvas::drawAllObjectsInPage( QPainter *painter, const QPtrList<KPObject> & obj )
{
    QPtrListIterator<KPObject> it( obj );
    for ( ; it.current(); ++it ) {
        if ( objectIsAHeaderFooterHidden( it.current() ) )
            continue;
        it.current()->draw( painter, m_view->zoomHandler(), SM_NONE, false );
    }
}

QRect KPrCanvas::getOldBoundingRect(KPObject *obj)
{
    KoRect oldKoBoundingRect = obj->getBoundingRect(m_view->zoomHandler());
    double _dx = oldKoBoundingRect.x() - 5.0;
    double _dy = oldKoBoundingRect.y() - 5.0;
    double _dw = oldKoBoundingRect.width() + 10.0;
    double _dh = oldKoBoundingRect.height() + 10.0;
    oldKoBoundingRect.setRect( _dx, _dy, _dw, _dh );
    oldBoundingRect = m_view->zoomHandler()->zoomRect( oldKoBoundingRect );
    return oldBoundingRect;
}

/*==================== handle mouse pressed ======================*/
void KPrCanvas::mousePressEvent( QMouseEvent *e )
{
    if(!m_view->koDocument()->isReadWrite())
        return;
    QPoint contentsPoint( e->pos().x()+diffx(), e->pos().y()+diffy() );
    KoPoint docPoint = m_view->zoomHandler()->unzoomPoint( contentsPoint );
    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( docPoint,m_view->zoomHandler() ))
        {
            KoPoint pos = docPoint - txtObj->getOrig(); // in pt, but now translated into the object's coordinate system
            mousePressed=true;
            if(e->button() == RightButton)
            {
                m_currentTextObjectView->showPopup( m_view, QCursor::pos(), m_view->actionList() );
                mousePressed=false;
            }
            else if( e->button() == MidButton )
            {
                QApplication::clipboard()->setSelectionMode( true );
                m_currentTextObjectView->paste();
                QApplication::clipboard()->setSelectionMode( false );
            }
            else
                m_currentTextObjectView->mousePressEvent(e, m_view->zoomHandler()->ptToLayoutUnitPix( pos ) ); // in LU pixels
            return;
        }
    }


    //disallow selecting objects outside the "page"
    if ( editMode ) {
        if( !m_activePage->getPageRect().contains( docPoint,m_view->zoomHandler() ) )
            return;
    }

    if ( e->state() & ControlButton )
        keepRatio = true;

    KPObject *kpobject = 0;

    oldMx = contentsPoint.x();
    oldMy = contentsPoint.y();
    QPoint rasterPoint( ( oldMx / rastX() ) * rastX() - diffx(), ( oldMy / rastY() ) * rastY() - diffy() );

    resizeObjNum = 0L;

    exitEditMode();

    if ( editMode ) {
        if ( e->button() == LeftButton ) {
            mousePressed = true;

            if ( m_drawPolyline && toolEditMode == INS_POLYLINE ) {
                m_dragStartPoint = rasterPoint;
                m_pointArray.putPoints( m_indexPointArray, 1,m_view->zoomHandler()->unzoomItX( m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y()) );
                ++m_indexPointArray;
                return;
            }

            if ( m_drawCubicBezierCurve && ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE ) ) {
                if ( m_drawLineWithCubicBezierCurve ) {
                    QPainter p( this );
                    p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
                    p.setBrush( Qt::NoBrush );
                    p.setRasterOp( Qt::NotROP );

                    QPoint oldStartPoint = m_dragStartPoint;

                    m_dragStartPoint = rasterPoint;

                    p.drawLine( oldStartPoint, m_dragStartPoint );  // erase old line
                    p.end();

                    m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX( m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY( m_dragStartPoint.y()) );
                    ++m_indexPointArray;
                    m_drawLineWithCubicBezierCurve = false;
                }
                else {
                    QPoint _oldEndPoint = rasterPoint;
                    QPainter p( this );
                    QPen _pen = QPen( Qt::black, 1, Qt::DashLine );
                    p.setPen( _pen );
                    p.setBrush( Qt::NoBrush );
                    p.setRasterOp( Qt::NotROP );

                    p.save();
                    double _angle = KoPoint::getAngle( _oldEndPoint, m_dragStartPoint );
                    //FIXME
                    drawFigure( L_SQUARE, &p, m_view->zoomHandler()->unzoomPoint( _oldEndPoint ),
                                _pen.color(), _pen.width(), _angle,m_view->zoomHandler()  ); // erase old figure
                    p.restore();

                    p.drawLine( m_dragStartPoint, _oldEndPoint ); // erase old line

                    int p_x = m_dragStartPoint.x() * 2 - _oldEndPoint.x();
                    int p_y = m_dragStartPoint.y() * 2 - _oldEndPoint.y();
                    QPoint _oldSymmetricEndPoint = QPoint( p_x, p_y );

                    p.save();
                    _angle = KoPoint::getAngle( _oldSymmetricEndPoint, m_dragStartPoint );
                    drawFigure( L_SQUARE, &p, m_view->zoomHandler()->unzoomPoint( _oldSymmetricEndPoint ),
                                _pen.color(), _pen.width(), _angle,m_view->zoomHandler() );  // erase old figure
                    p.restore();

                    p.drawLine( m_dragStartPoint, _oldSymmetricEndPoint );  // erase old line

                    m_pointArray.putPoints( m_indexPointArray, 3, m_CubicBezierSecondPoint.x(), m_CubicBezierSecondPoint.y(),
                                            m_CubicBezierThirdPoint.x(), m_CubicBezierThirdPoint.y(),
                                            m_view->zoomHandler()->unzoomItX(m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y()) );
                    m_indexPointArray += 3;
                    m_drawLineWithCubicBezierCurve = true;
                    m_oldCubicBezierPointArray = KoPointArray();
                    m_oldCubicBezierPointArray.putPoints( 0, 4, 0.0,0.0, 0.0,0.0, 0.0,0.0, 0.0,0.0 );
                    m_dragEndPoint = m_dragStartPoint;
                }

                return;
            }

            switch ( toolEditMode ) {
                case TEM_MOUSE: {
                    bool overObject = false;
                    bool deSelAll = true;
                    bool _resizeObj = false;
                    KPObject *kpobject = 0;

		    firstX = contentsPoint.x();
                    firstY = contentsPoint.y();
		    kpobject = m_activePage->getObjectResized( docPoint, modType, deSelAll, overObject, _resizeObj );
		    if ( kpobject ) {
                        if(_resizeObj)
                        {
			    oldBoundingRect = getOldBoundingRect( kpobject );
                            resizeObjNum = kpobject;
                        }
                    }
                    else
                    {
			_resizeObj = false;
                        kpobject = m_view->kPresenterDoc()->stickyPage()->getObjectResized( docPoint, modType, deSelAll, overObject, _resizeObj );
                        if( kpobject && m_view->kPresenterDoc()->isHeaderFooter(kpobject))
                        {
                            if(objectIsAHeaderFooterHidden(kpobject))
                                kpobject=0L;
                        }
                        if( kpobject && _resizeObj ) {
                            oldBoundingRect = getOldBoundingRect( kpobject );
                            resizeObjNum = kpobject;
                        }
                    }
                    if ( deSelAll && !( e->state() & ShiftButton ) && !( e->state() & ControlButton ) )
                        deSelectAllObj();

                    if ( overObject ) {
                        if ( kpobject ) {
                            selectObj( kpobject );
                            modType = MT_NONE;
                            raiseObject( kpobject );
			}
                    }
                    else {
                        modType = MT_NONE;
                        if ( !( e->state() & ShiftButton ) && !( e->state() & ControlButton ) )
                            deSelectAllObj();
                        drawRubber = true;
                        rubber = QRect( e->x(), e->y(), 0, 0 );
                    }

		    // update hotspot
		    calcBoundingRect();
                    m_hotSpot = docPoint - m_boundingRect.topLeft();
                } break;
		case TEM_ROTATE: {
                    bool deSelAll = true;
                    bool _resizeObj = false;
                    KPObject *kpobject = 0;

                    firstX = contentsPoint.x();
                    firstY = contentsPoint.y();

		    // find object on active page
		    kpobject = m_activePage->getEditObj( docPoint );

		    // find object on sticky page (ignore header/footer)
		    if ( !kpobject ) {
			kpobject = m_view->kPresenterDoc()->stickyPage()->getEditObj( docPoint );
                        if( kpobject && m_view->kPresenterDoc()->isHeaderFooter(kpobject))
                            if(objectIsAHeaderFooterHidden(kpobject))
                                kpobject=0L;
		    }

		    // clear old selections even if shift or control are pressed
		    // we don't support rotating multiple objects yet
		    deSelectAllObj();

		    // deselect all if no object is found
		    if ( !kpobject )
			deSelectAllObj();

		    // select and raise object
		    else {
			rotateNum = kpobject;
			startAngle = -kpobject->getAngle();
			selectObj( kpobject );
			raiseObject( kpobject );
		    }

		    // set axis to center of selected objects bounding rect
		    if ( kpobject ) {
			calcBoundingRect();
			axisX =	m_boundingRect.center().x();
			axisY = m_boundingRect.center().y();
		    }
		} break;
                case INS_FREEHAND: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_indexPointArray = 0;
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                    m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX(m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y()) );
                    ++m_indexPointArray;
                } break;
                case INS_POLYLINE: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_drawPolyline = true;
                    m_indexPointArray = 0;
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                    m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX(m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y()) );
                    ++m_indexPointArray;
                } break;
                case INS_CUBICBEZIERCURVE: case INS_QUADRICBEZIERCURVE: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_drawCubicBezierCurve = true;
                    m_drawLineWithCubicBezierCurve = true;
                    m_indexPointArray = 0;
                    m_oldCubicBezierPointArray.putPoints( 0, 4, 0,0, 0,0, 0,0, 0,0 );
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                    m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX(m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y() ));
                    ++m_indexPointArray;
                } break;
                case INS_POLYGON: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );

                    m_indexPointArray = 0;
                    m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                               ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                    m_dragEndPoint = m_dragStartPoint;
                } break;
                default: {
                    deSelectAllObj();
                    mousePressed = true;
                    insRect = QRect( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                     ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy(), 0, 0 );
                } break;
            }
        }

        if ( e->button() == RightButton && toolEditMode == INS_POLYLINE && !m_pointArray.isNull() && m_drawPolyline ) {
            m_dragStartPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                       ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
            m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX(m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y() ));
            ++m_indexPointArray;
            endDrawPolyline();

            mouseMoveEvent( e );

            return;
        }

        if ( e->button() == RightButton && ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE )
             && !m_pointArray.isNull() && m_drawCubicBezierCurve ) {
            if ( m_drawLineWithCubicBezierCurve ) {
                QPoint point = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                       ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX(point.x()), m_view->zoomHandler()->unzoomItY(point.y()) );
                ++m_indexPointArray;
            }
            else {
                m_pointArray.putPoints( m_indexPointArray, 2, m_CubicBezierSecondPoint.x(), m_CubicBezierSecondPoint.y(),
                                        m_CubicBezierThirdPoint.x(), m_CubicBezierThirdPoint.y() );
                m_indexPointArray += 2;
            }

            endDrawCubicBezierCurve();

            mouseMoveEvent( e );

            return;
        }

        if ( e->button() == RightButton && toolEditMode == TEM_MOUSE ) {
            KPObject*obj = getObjectAt( docPoint );
            if(objectIsAHeaderFooterHidden(obj))
                obj=0L;
            if ( obj ) {
                kpobject = obj;
		QPoint pnt = QCursor::pos();
		mousePressed = false;
                bool state=!( e->state() & ShiftButton ) && !( e->state() & ControlButton ) && !kpobject->isSelected();
                if ( kpobject->getType() == OT_PICTURE ) {
                    deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuPicObject(pnt);
                } else if ( kpobject->getType() == OT_CLIPART ) {
                    deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuClipObject(pnt);
                } else if ( kpobject->getType() == OT_TEXT ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuTextObject(pnt);
                } else if ( kpobject->getType() == OT_PIE ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuPieObject( pnt );
                } else if ( kpobject->getType() == OT_RECT ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuRectangleObject( pnt );
                } else if ( kpobject->getType() == OT_PART ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuPartObject( pnt );
                } else if ( kpobject->getType() == OT_POLYGON ) {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuPolygonObject( pnt );
                } else {
                    if ( state )
                        deSelectAllObj();
                    selectObj( kpobject );
                    m_view->openPopupMenuGraphMenu( pnt );
                }
            } else {
                QPoint pnt = QCursor::pos();
                m_view->openPopupMenuMenuPage( pnt );
                mousePressed = false;
            }
	    modType = MT_NONE;

        }
        else if( e->button() == RightButton && toolEditMode != TEM_MOUSE ) {
            //deactivate tools when you click on right button
            setToolEditMode( TEM_MOUSE );
        }
    } else {
        oldMx = e->x();
        oldMy = e->y();
        if ( e->button() == LeftButton ) {
            if ( presMenu->isVisible() ) {
                presMenu->hide();
                setCursor( blankCursor );
            } else {
                if ( drawMode )
                    drawLineInDrawMode = true;
                else
                    m_view->screenNext();
            }
        } else if ( e->button() == MidButton )
            m_view->screenPrev();
        else if ( e->button() == RightButton ) {
            if ( !drawMode && !spManualSwitch() )
                m_view->autoScreenPresStopTimer();

            setCursor( arrowCursor );
            QPoint pnt = QCursor::pos();
            presMenu->popup( pnt );
        }
    }

    // ME: I have no idea why this is needed at all
    if ( toolEditMode == TEM_MOUSE )
	mouseMoveEvent( e );

    if ( modType != MT_NONE && modType != MT_MOVE ) {
        KPObject *kpobject=resizeObjNum;
        if ( kpobject ) {
            ratio = static_cast<double>( static_cast<double>( kpobject->getSize().width() ) /
                                         static_cast<double>( kpobject->getSize().height() ) );
            //oldRect = QRect( kpobject->getOrig().x(), kpobject->getOrig().y(),
            //                 kpobject->getSize().width(), kpobject->getSize().height() );
            oldRect = m_view->zoomHandler()->zoomRect( kpobject->getBoundingRect(m_view->zoomHandler()) );
        }
    }
}

void KPrCanvas::calcBoundingRect()
{
  m_boundingRect = KoRect();

  m_boundingRect=m_activePage->getBoundingRect(m_boundingRect, m_view->kPresenterDoc());
  m_boundingRect=m_view->kPresenterDoc()->stickyPage()->getBoundingRect(m_boundingRect, m_view->kPresenterDoc());

}


/*=================== handle mouse released ======================*/
void KPrCanvas::mouseReleaseEvent( QMouseEvent *e )
{
    QPoint contentsPoint( e->pos().x()+diffx(), e->pos().y()+diffy() );
    KoPoint docPoint = m_view->zoomHandler()->unzoomPoint( contentsPoint );
    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( docPoint,m_view->zoomHandler() ))
        {
            m_currentTextObjectView->mouseReleaseEvent( e, contentsPoint );
            mousePressed=false;
            emit objectSelectedChanged();
            return;
        }
    }

    if ( e->button() != LeftButton ) {
        ratio = 0.0;
        keepRatio = false;
        return;
    }

    if ( drawMode ) {
        drawLineInDrawMode = false;
        return;
    }

    int mx = contentsPoint.x();
    int my = contentsPoint.y();
    mx = ( mx / rastX() ) * rastX();
    my = ( my / rastY() ) * rastY();
    firstX = ( firstX / rastX() ) * rastX();
    firstY = ( firstY / rastY() ) * rastY();
    QPtrList<KPObject> _objects;
    _objects.setAutoDelete( false );
    KPObject *kpobject = 0;

    if ( ( m_drawPolyline && toolEditMode == INS_POLYLINE )
         || ( m_drawCubicBezierCurve && ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE ) ) ) {
        return;
    }

    if ( toolEditMode != INS_LINE )
        insRect = insRect.normalize();

    KoPoint mv;
    KoSize sz;
    if ( toolEditMode == TEM_MOUSE && modType != MT_NONE && modType != MT_MOVE  && resizeObjNum ) {
        kpobject = resizeObjNum;
        if ( kpobject ) {
            mv = KoPoint( kpobject->getOrig().x() - m_view->zoomHandler()->unzoomItX( oldRect.x()),
                         kpobject->getOrig().y() - m_view->zoomHandler()->unzoomItY(oldRect.y()) );
            sz = KoSize( kpobject->getSize().width() - m_view->zoomHandler()->unzoomItX(oldRect.width()),
                        kpobject->getSize().height() - m_view->zoomHandler()->unzoomItY(oldRect.height()) );
        }
        kpobject = 0L;
    }

    switch ( toolEditMode ) {
    case TEM_MOUSE: {
	drawContour = FALSE;
        switch ( modType ) {
        case MT_NONE: {
            if ( drawRubber ) {
                QPainter p;
                p.begin( this );
                p.setRasterOp( NotROP );
                p.setPen( QPen( black, 0, DotLine ) );
                p.drawRect( rubber );
                p.end();
                drawRubber = false;

                rubber = rubber.normalize();
                rubber.moveBy(diffx(),diffy());

                QPtrListIterator<KPObject> it( getObjectList() );
                for ( ; it.current() ; ++it )
                {
                    if ( it.current()->intersects( m_view->zoomHandler()->unzoomRect(rubber),m_view->zoomHandler() ) )
                        selectObj( it.current() );
                }

                QPtrListIterator<KPObject> sIt(m_view->kPresenterDoc()->stickyPage()->objectList() );
                for ( ; sIt.current() ; ++sIt )
                {
                    if ( sIt.current()->intersects( m_view->zoomHandler()->unzoomRect(rubber),m_view->zoomHandler() ) )
                    {
                        if( m_view->kPresenterDoc()->isHeaderFooter(sIt.current()))
                        {
                            if( objectIsAHeaderFooterHidden(sIt.current()))
                                continue;
                        }
                        selectObj( sIt.current() );
                    }
                }


            }
        } break;
        case MT_MOVE: {
            if ( firstX != mx || firstY != my ) {
                KMacroCommand *macro=new KMacroCommand(i18n("Move object(s)"));
                bool cmdCreate=false;
                int x=(mx - firstX);
                int y=(my - firstY);
                KCommand *cmd=m_activePage->moveObject(m_view,x,y);
                if(cmd)
                {
                    macro->addCommand(cmd);
                    cmdCreate=true;
                }
                cmd=m_view->kPresenterDoc()->stickyPage()->moveObject(m_view,x,y);
                if(cmd)
                {
                    macro->addCommand(cmd);
                    cmdCreate=true;
                }
                if(cmdCreate)
                    m_view->kPresenterDoc()->addCommand(macro );
                else
                    delete macro;
            } else
            {
                m_view->kPresenterDoc()->stickyPage()->repaintObj();
                m_activePage->repaintObj();
            }
        }
            break;
        case MT_RESIZE_UP: {
            if ( !resizeObjNum ) break;
	    kpobject = resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object up" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_DN: {
	    if ( !resizeObjNum ) break;
	    kpobject =  resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object down" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_LF: {
	    if ( !resizeObjNum ) break;
	    kpobject = resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object left" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_RT: {
            if ( !resizeObjNum ) break;
	    kpobject =  resizeObjNum ;

            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object right" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_LU: {
            if ( !resizeObjNum ) break;
	    kpobject = resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object left up" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_LD: {
	    if ( !resizeObjNum ) break;
	    kpobject =  resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object left and down" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        case MT_RESIZE_RU: {
            if ( !resizeObjNum ) break;
	    kpobject =  resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object right and up" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
	case MT_RESIZE_RD: {
	    if ( !resizeObjNum ) break;
	    kpobject = resizeObjNum;
            if ( firstX != mx || firstY != my ) {
                ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Resize object right and down" ), mv, sz,
                                                      kpobject, m_view->kPresenterDoc() );
                resizeCmd->unexecute( false );
                resizeCmd->execute();
                m_view->kPresenterDoc()->addCommand( resizeCmd );
            }
            _repaint( oldBoundingRect );
            _repaint( kpobject );
        } break;
        }
    } break;
    case INS_TEXT: {
        if ( !insRect.isNull() ) {
            KPTextObject* kptextobject = insertTextObject( insRect );
            setToolEditMode( TEM_MOUSE );

            // User-friendlyness: automatically start editing this textobject
            activePage()->deSelectAllObj();
	    m_view->kPresenterDoc()->stickyPage()->deSelectAllObj();
            createEditing( kptextobject );
            //setTextBackground( kptextobject );
            //setCursor( arrowCursor );
        }
    } break;
    case TEM_ROTATE: {
	drawContour = FALSE;
	if ( !rotateNum )
	    break;
	if ( startAngle != rotateNum->getAngle() ) {
	    QPtrList<RotateCmd::RotateValues> list;
	    RotateCmd::RotateValues *v = new RotateCmd::RotateValues;
	    v->angle = startAngle;
	    list.append( v );
	    QPtrList<KPObject> objects;
	    objects.append( rotateNum );
	    RotateCmd *rotateCmd = new RotateCmd( i18n( "Change Rotation" ), list,
						  rotateNum->getAngle(),
						  objects, m_view->kPresenterDoc() );
	    m_view->kPresenterDoc()->addCommand( rotateCmd );
	}
    }break;
    case INS_LINE: {
        if ( insRect.width() != 0 && insRect.height() != 0 ) {
            if ( insRect.top() == insRect.bottom() ) {
                bool reverse = insRect.left() > insRect.right();
                insRect = insRect.normalize();
                insRect.setRect( insRect.left(), insRect.top() - rastY() / 2,
                                 insRect.width(), rastY() );
                insertLineH( insRect, reverse );
            } else if ( insRect.left() == insRect.right() ) {
                bool reverse = insRect.top() > insRect.bottom();
                insRect = insRect.normalize();
                insRect.setRect( insRect.left() - rastX() / 2, insRect.top(),
                                 rastX(), insRect.height() );
                insertLineV( insRect, reverse );
            } else if ( insRect.left() < insRect.right() && insRect.top() < insRect.bottom() ||
                      insRect.left() > insRect.right() && insRect.top() > insRect.bottom() ) {
                bool reverse = insRect.left() > insRect.right() && insRect.top() > insRect.bottom();
                insertLineD1( insRect.normalize(), reverse );
            } else {
                bool reverse = insRect.right() < insRect.left() && insRect.top() < insRect.bottom();
                insertLineD2( insRect.normalize(), reverse );
            }
        }
    } break;
    case INS_RECT:
        if ( !insRect.isNull() ) insertRect( insRect );
        break;
    case INS_ELLIPSE:
        if ( !insRect.isNull() ) insertEllipse( insRect );
        break;
    case INS_PIE:
        if ( !insRect.isNull() ) insertPie( insRect );
        break;
    case INS_OBJECT:
    case INS_DIAGRAMM:
    case INS_TABLE:
    case INS_FORMULA: {
        if ( !insRect.isNull() ) insertObject( insRect );
        setToolEditMode( TEM_MOUSE );
    } break;
    case INS_AUTOFORM: {
        bool reverse = insRect.left() > insRect.right() || insRect.top() > insRect.bottom();
        if ( !insRect.isNull() ) insertAutoform( insRect, reverse );
        setToolEditMode( TEM_MOUSE );
    } break;
    case INS_FREEHAND:
        if ( !m_pointArray.isNull() ) insertFreehand( m_pointArray );
        break;
    case INS_POLYGON:
        if ( !m_pointArray.isNull() ) insertPolygon( m_pointArray );
        break;
    case INS_PICTURE: {
        if ( !insRect.isNull() ) insertPicture( insRect );
        setToolEditMode( TEM_MOUSE );
    } break;
    case INS_CLIPART: {
        if ( !insRect.isNull() ) insertClipart( insRect );
        setToolEditMode( TEM_MOUSE );
    } break;
    default: break;
    }
    emit objectSelectedChanged();
    if ( toolEditMode != TEM_MOUSE && editMode )
        repaint( false );

    mousePressed = false;
    modType = MT_NONE;
    resizeObjNum = 0L;
    mouseMoveEvent( e );
    ratio = 0.0;
    keepRatio = false;
    calcBoundingRect();
}

/*==================== handle mouse moved ========================*/
void KPrCanvas::mouseMoveEvent( QMouseEvent *e )
{
    QPoint contentsPoint( e->pos().x()+diffx(), e->pos().y()+diffy() );
    KoPoint docPoint = m_view->zoomHandler()->unzoomPoint( contentsPoint );
    if(m_currentTextObjectView)
    {
        KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
        Q_ASSERT(txtObj);
        if(txtObj->contains( docPoint,m_view->zoomHandler() )&&mousePressed)
        {
            KoPoint pos = docPoint - txtObj->getOrig();
            m_currentTextObjectView->mouseMoveEvent( e, m_view->zoomHandler()->ptToLayoutUnitPix( pos ) ); // in LU pixels
        }
        return;
    }

    if ( editMode ) {
	m_view->setRulerMousePos( e->x(), e->y() );

	KPObject *kpobject;

	if ( ( !mousePressed || ( !drawRubber && modType == MT_NONE ) ) &&
	     toolEditMode == TEM_MOUSE ) {
	    bool cursorAlreadySet = false;
	    if ( (int)objectList().count() - 1 >= 0 || (int)m_view->kPresenterDoc()->stickyPage()->objectList().count() -1>=0 )
            {
                kpobject=m_activePage->getCursor(docPoint);
                if( kpobject)
                {
                    setCursor( kpobject->getCursor( docPoint, modType,m_view->kPresenterDoc()) );

                    cursorAlreadySet = true;
                }
                else
                {
                    kpobject=m_view->kPresenterDoc()->stickyPage()->getCursor(docPoint );
                    if( kpobject)
                    {
                        setCursor( kpobject->getCursor( docPoint, modType,m_view->kPresenterDoc() ) );
                        cursorAlreadySet = true;
                    }
                }
	    }
	    if ( !cursorAlreadySet )
		setCursor( arrowCursor );
	    else
		return;
	} else if ( mousePressed ) {
	    int mx = e->x()+diffx();
	    int my = e->y()+diffy();
	    mx = ( mx / rastX() ) * rastX();
	    my = ( my / rastY() ) * rastY();
	    switch ( toolEditMode ) {
	    case TEM_MOUSE: {
		drawContour = TRUE;

		oldMx = ( oldMx / rastX() ) * rastX();
		oldMy = ( oldMy / rastY() ) * rastY();

		if ( modType == MT_NONE ) {
		    if ( drawRubber ) {
			QPainter p;
			p.begin( this );
			p.setRasterOp( NotROP );
			p.setPen( QPen( black, 0, DotLine ) );
			p.drawRect( rubber );
			rubber.setRight( e->x() );
			rubber.setBottom( e->y() );
			p.drawRect( rubber );
			p.end();
		    }
		} else if ( modType == MT_MOVE ) {
                    m_hotSpot = docPoint - m_boundingRect.topLeft();
		    moveObject( mx - oldMx , my - oldMy, false );
		} else if ( modType != MT_NONE && resizeObjNum ) {
                    resizeObject( modType, mx - oldMx, my - oldMy );
		}

		oldMx = e->x()+diffx();
		oldMy = e->y()+diffy();
	    } break;
	    case TEM_ROTATE: {
		drawContour = TRUE;
		double angle = KoPoint::getAngle( KoPoint( e->x() + diffx(), e->y() + diffy() ),
						  KoPoint( axisX, axisY ) );
		double angle1 = KoPoint::getAngle( KoPoint( firstX, firstY ),
						   KoPoint( axisX, axisY ) );

		angle -= angle1;
		angle -= startAngle;

		activePage()->rotateObj( angle );
		m_view->kPresenterDoc()->stickyPage()->rotateObj( angle );
	    }break;
	    case INS_TEXT: case INS_OBJECT: case INS_TABLE:
	    case INS_DIAGRAMM: case INS_FORMULA: case INS_AUTOFORM:
            case INS_PICTURE: case INS_CLIPART: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawRect( insRect );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                limitSizeOfObject();
		p.drawRect( insRect );
		p.end();

		mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
	    } break;
	    case INS_ELLIPSE: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawEllipse( insRect );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                limitSizeOfObject();
		p.drawEllipse( insRect );
		p.end();

		mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
	    } break;
	    case INS_RECT: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawRoundRect( insRect, m_view->getRndX(), m_view->getRndY() );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                limitSizeOfObject();
		p.drawRoundRect( insRect, m_view->getRndX(), m_view->getRndY() );
		p.end();

		mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
	    } break;
	    case INS_LINE: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 )
		    p.drawLine( insRect.topLeft(), insRect.bottomRight() );
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                limitSizeOfObject();
		p.drawLine( insRect.topLeft(), insRect.bottomRight() );
		p.end();

		mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
	    } break;
	    case INS_PIE: {
		QPainter p( this );
		p.setPen( QPen( black, 1, SolidLine ) );
		p.setBrush( NoBrush );
		p.setRasterOp( NotROP );
		if ( insRect.width() != 0 && insRect.height() != 0 ) {
                    drawPieObject(&p);
		}
		insRect.setRight( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx() );
		insRect.setBottom( ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                limitSizeOfObject();
                drawPieObject(&p);
		p.end();

		mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
	    } break;
            case INS_FREEHAND: {
                m_dragEndPoint = QPoint( e->x() + diffx(), e->y() + diffy() );

                QPainter p( this );
                p.setPen( QPen( black, 1, SolidLine ) );
                p.setBrush( NoBrush );
                p.setRasterOp( NotROP );

                m_dragEndPoint=limitOfPoint(m_dragEndPoint);

                p.drawLine( m_dragStartPoint, m_dragEndPoint );
                p.end();

                m_pointArray.putPoints( m_indexPointArray, 1, m_view->zoomHandler()->unzoomItX(m_dragStartPoint.x()), m_view->zoomHandler()->unzoomItY(m_dragStartPoint.y()) );
                ++m_indexPointArray;
                m_dragStartPoint = m_dragEndPoint;

                mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
            } break;
            case INS_POLYLINE: {
                QPainter p( this );
                p.setPen( QPen( black, 1, SolidLine ) );
                p.setBrush( NoBrush );
                p.setRasterOp( NotROP );
                p.drawLine( m_dragStartPoint, m_dragEndPoint ); //
                m_dragEndPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                         ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                m_dragEndPoint=limitOfPoint(m_dragEndPoint);

                p.drawLine( m_dragStartPoint, m_dragEndPoint );
                p.end();

                mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
            } break;
            case INS_CUBICBEZIERCURVE: case INS_QUADRICBEZIERCURVE:{
                drawCubicBezierCurve( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                      ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );

                mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
            } break;
            case INS_POLYGON: {
                drawPolygon( m_view->zoomHandler()->unzoomPoint( m_dragStartPoint ),
                             m_view->zoomHandler()->unzoomPoint( m_dragEndPoint ) ); // erase old polygon

                m_dragEndPoint = QPoint( ( ( e->x() + diffx() ) / rastX() ) * rastX() - diffx(),
                                         ( ( e->y() + diffy() ) / rastY() ) * rastY() - diffy() );
                m_dragEndPoint=limitOfPoint(m_dragEndPoint);

                drawPolygon( m_view->zoomHandler()->unzoomPoint( m_dragStartPoint ),
                             m_view->zoomHandler()->unzoomPoint( m_dragEndPoint ) ); // draw new polygon

                mouseSelectedObject = true;

                m_view->penColorChanged( m_view->getPen() );
                m_view->brushColorChanged( m_view->getBrush() );
            } break;
            default: break;
	    }
	}
    } else if ( !editMode && drawMode && drawLineInDrawMode ) {
	QPainter p;
	p.begin( this );
	p.setPen( m_view->kPresenterDoc()->presPen() );
	p.drawLine( oldMx, oldMy, e->x(), e->y() );
	oldMx = e->x();
	oldMy = e->y();
	p.end();
    }
    if ( !editMode && !drawMode && !presMenu->isVisible() && fillBlack )
	setCursor( blankCursor );
}

/*==================== mouse double click ========================*/
void KPrCanvas::mouseDoubleClickEvent( QMouseEvent *e )
{
  if(!m_view->koDocument()->isReadWrite())
    return;
  QPoint contentsPoint( e->pos().x()+diffx(), e->pos().y()+diffy() );
  KoPoint docPoint = m_view->zoomHandler()->unzoomPoint( contentsPoint );
  if(m_currentTextObjectView)
    {
      KPTextObject *txtObj=m_currentTextObjectView->kpTextObject();
      Q_ASSERT(txtObj);
      if(txtObj->contains( docPoint,m_view->zoomHandler() ))
        {
	  KoPoint pos = contentsPoint - txtObj->getOrig();
	  //pos=m_view->zoomHandler()->pixelToLayoutUnit(QPoint(pos.x(),pos.y()));
	  m_currentTextObjectView->mouseDoubleClickEvent( e, m_view->zoomHandler()->ptToLayoutUnitPix( pos ) );
	  return;
        }
    }

  //disallow activating objects outside the "page"
  if ( !m_activePage->getPageRect().contains(docPoint,m_view->zoomHandler()))
    return;

  if ( toolEditMode != TEM_MOUSE || !editMode ) return;

  deSelectAllObj();
  KPObject *kpobject = 0;
  kpobject=m_activePage->getEditObj(docPoint);
  if( !kpobject)
    {
      kpobject=m_view->kPresenterDoc()->stickyPage()->getEditObj(docPoint );
      if( kpobject && m_view->kPresenterDoc()->isHeaderFooter(kpobject))
      {
          if( objectIsAHeaderFooterHidden(kpobject))
              kpobject=0L;
      }
    }
  if(kpobject)
    {
      if ( kpobject->getType() == OT_TEXT )
	{
	KPTextObject *kptextobject = dynamic_cast<KPTextObject*>( kpobject );
	if(m_currentTextObjectView)
	  {
	    m_currentTextObjectView->terminate();
	    delete m_currentTextObjectView;
	  }
	m_currentTextObjectView=kptextobject->createKPTextView(this);

	setTextBackground( kptextobject );
	setCursor( arrowCursor );
	editNum = kpobject;
      }
      else if ( kpobject->getType() == OT_PART )
	{
	  KPPartObject * obj=static_cast<KPPartObject *>(kpobject);
	  obj->activate( m_view );
	  editNum = obj;
	}
    }
}

void KPrCanvas::drawPieObject(QPainter *p)
{
    switch ( m_view->getPieType() ) {
    case PT_PIE:
        p->drawPie( insRect.x(), insRect.y(), insRect.width() - 2,
                   insRect.height() - 2, m_view->getPieAngle(), m_view->getPieLength() );
        break;
    case PT_ARC:
        p->drawArc( insRect.x(), insRect.y(), insRect.width() - 2,
                   insRect.height() - 2, m_view->getPieAngle(), m_view->getPieLength() );
        break;
    case PT_CHORD:
        p->drawChord( insRect.x(), insRect.y(), insRect.width() - 2,
                     insRect.height() - 2, m_view->getPieAngle(), m_view->getPieLength() );
        break;
    default: break;
    }

}

void KPrCanvas::limitSizeOfObject()
{
    QRect pageRect=m_activePage->getZoomPageRect();

    if(insRect.right()>pageRect.right()-1)
        insRect.setRight(pageRect.width()-1);
    else if( insRect.right()<pageRect.left()-1)
        insRect.setRight(pageRect.left()+1);
    if(insRect.bottom()>pageRect.bottom()-1)
        insRect.setBottom(pageRect.height()-1);
    else if( insRect.bottom()<pageRect.top()-1)
        insRect.setBottom(pageRect.top()+1);
}

QPoint KPrCanvas::limitOfPoint(const QPoint& _point)
{
    QRect pageRect=m_activePage->getZoomPageRect();
    QPoint point(_point);
    if(point.x()>pageRect.right()-1)
        point.setX(pageRect.width()-1);
    else if( point.x()<pageRect.left()-1)
        point.setX(pageRect.left()+1);
    if(point.y()>pageRect.bottom()-1)
        point.setY(pageRect.height()-1);
    else if( point.y()<pageRect.top()-1)
        point.setY(pageRect.top()+1);
    return point;
}

/*====================== mouse wheel event =========================*/
void KPrCanvas::wheelEvent( QWheelEvent *e )
{
    if ( !editMode && !drawMode ) {
        if ( e->delta() == -120 )     // wheel down
            m_view->screenNext();
        else if ( e->delta() == 120 ) // wheel up
            m_view->screenPrev();
        e->accept();
    }
    else if ( editMode )
        emit sigMouseWheelEvent( e );
}


/*====================== key press event =========================*/
void KPrCanvas::keyPressEvent( QKeyEvent *e )
{
    if ( !editMode ) {
	switch ( e->key() ) {
	case Key_Space: case Key_Right: case Key_Down: case Key_Next:
	    m_view->screenNext(); break;
	case Key_Backspace: case Key_Left: case Key_Up: case Key_Prior:
	    m_view->screenPrev(); break;
	case Key_Escape: case Key_Q: case Key_X:
	    m_view->screenStop(); break;
	case Key_G:
            if ( !spManualSwitch() )
                m_view->autoScreenPresStopTimer();
	    slotGotoPage(); break;
        case Key_Home:  // go to first page
            if ( slideListIterator != slideList.begin() ) {
                gotoPage( 1 );
                if ( !spManualSwitch() ) {
                    m_view->setCurrentTimer( 1 );
                    setNextPageTimer( true );
                }
            }
            break;
        case Key_End:  // go to last page
            if ( slideListIterator != slideList.begin() ) {
                gotoPage( slideList.count() );
                if ( !spManualSwitch() ) {
                    m_view->setCurrentTimer( 1 );
                    setNextPageTimer( true );
                }
            }
            break;
	default: break;
	}
    } else if ( editNum ) {
	if ( e->key() == Key_Escape ) {
            exitEditMode();
	}
        else if ( m_currentTextObjectView )
        {
            m_currentTextObjectView->keyPressEvent( e );
        }
    } else if ( mouseSelectedObject ) {
	if ( e->state() & ControlButton ) {
            int offsetx=QMAX(1,m_view->zoomHandler()->zoomItX(10));
            int offsety=QMAX(1,m_view->zoomHandler()->zoomItY(10));
            m_hotSpot = KoPoint(0,0);
	    switch ( e->key() ) {
	    case Key_Up:
		moveObject( 0, -offsety, true );
		break;
	    case Key_Down:
		moveObject( 0, offsety, true );
		break;
	    case Key_Right:
		moveObject( offsetx, 0, true );
		break;
	    case Key_Left:
		moveObject( -offsetx, 0, true );
		break;
	    default: break;
	    }
	} else {
	    switch ( e->key() ) {
	    case Key_Up:
		moveObject( 0, -1, true );
		break;
	    case Key_Down:
		moveObject( 0, 1, true );
		break;
	    case Key_Right:
		moveObject( 1, 0, true );
		break;
	    case Key_Left:
		moveObject( -1, 0, true );
		break;
	    case Key_Delete: case Key_Backspace:
		m_view->editDelete();
		break;
	    case Key_Escape:
		setToolEditMode( TEM_MOUSE );
		break;
	    default: break;
	    }
	}
    } else {
	switch ( e->key() ) {
	case Key_Next:
	    m_view->nextPage();
	    break;
	case Key_Prior:
	    m_view->prevPage();
	    break;
	case Key_Down:
	    m_view->getVScrollBar()->addLine();
	    break;
	case Key_Up:
	    m_view->getVScrollBar()->subtractLine();
	    break;
	case Key_Right:
	    m_view->getHScrollBar()->addLine();
	    break;
	case Key_Left:
	    m_view->getHScrollBar()->subtractLine();
	    break;
	case Key_Tab:
	    selectNext();
	    break;
	case Key_Backtab:
	    selectPrev();
	    break;
	case Key_Home:
	    m_view->getVScrollBar()->setValue( 0 );
	    break;
	case Key_End:
	    m_view->getVScrollBar()->setValue( m_view->getVScrollBar()->maxValue());
	    break;
	default: break;
	}
    }
}

void KPrCanvas::keyReleaseEvent( QKeyEvent *e )
{
    if ( editMode && m_currentTextObjectView )
    {
        m_currentTextObjectView->keyReleaseEvent( e );
    }
    else
    {
        if ( mouseSelectedObject )
        {
            if(e->key()==Key_Up || e->key()==Key_Down || e->key()==Key_Right || e->key()==Key_Left)
                emit objectSelectedChanged();
        }
    }
}

/*========================= resize Event =========================*/
void KPrCanvas::resizeEvent( QResizeEvent *e )
{
    if ( editMode )
        QWidget::resizeEvent( e );
    else
        QWidget::resizeEvent( new QResizeEvent( QApplication::desktop()->size(),
                                                e->oldSize() ) );
    if ( editMode ) // ### what happens in fullscreen mode ? No double-buffering !?!?
        buffer.resize( size() );
}

/*========================== get object ==========================*/
KPObject* KPrCanvas::getObjectAt( const KoPoint&pos )
{
  KPObject *kpobject=m_activePage->getObjectAt(pos);
  if( !kpobject)
    {
      kpobject=m_view->kPresenterDoc()->stickyPage()->getObjectAt(pos);
    }
  return kpobject;
}

/*======================= select object ==========================*/
void KPrCanvas::selectObj( int num )
{
    if ( num < static_cast<int>( objectList().count() ) ) {
        selectObj( objectList().at( num ) );
        emit objectSelectedChanged();
    }
}

/*======================= deselect object ========================*/
void KPrCanvas::deSelectObj( int num )
{
    if ( num < static_cast<int>( objectList().count() ) )
        deSelectObj( objectList().at( num ) );
}

/*======================= select object ==========================*/
void KPrCanvas::selectObj( KPObject *kpobject )
{
    kpobject->setSelected( true );
    //FIXME
    m_view->penColorChanged( m_activePage->getPen( QPen( Qt::black, 1, Qt::SolidLine ) ) );
    m_view->brushColorChanged( m_activePage->getBrush( QBrush( Qt::white, Qt::SolidPattern ) ) );
    _repaint( kpobject );
    emit objectSelectedChanged();

    mouseSelectedObject = true;
}

/*======================= deselect object ========================*/
void KPrCanvas::deSelectObj( KPObject *kpobject )
{
    kpobject->setSelected( false );
    _repaint( kpobject );

    mouseSelectedObject = false;
    emit objectSelectedChanged();
}

/*====================== select all objects ======================*/
void KPrCanvas::selectAllObj()
{
    int nbObj=objectList().count()+m_view->kPresenterDoc()->stickyPage()->objectList().count();
    if(nbObj==(m_view->kPresenterDoc()->stickyPage()->numSelected()+m_activePage->numSelected()))
        return;

    QProgressDialog progress( i18n( "Selecting..." ), 0,
                              nbObj, this );
    int i=0;
    QPtrListIterator<KPObject> it( m_view->kPresenterDoc()->stickyPage()->objectList() );
    for ( ; it.current() ; ++it )
    {
        selectObj(it.current());
        progress.setProgress( i );
        kapp->processEvents();
        i++;
    }

    it= m_activePage->objectList();
    for ( ; it.current() ; ++it )
    {
        selectObj(it.current());
        progress.setProgress( i );

        kapp->processEvents();
        i++;
    }

    mouseSelectedObject = true;
    emit objectSelectedChanged();
}


/*==================== deselect all objects ======================*/
void KPrCanvas::deSelectAllObj()
{
  if(m_activePage->numSelected()==0 && m_view->kPresenterDoc()->stickyPage()->numSelected()==0)
        return;

    if ( !m_view->kPresenterDoc()->raiseAndLowerObject && selectedObjectPosition != -1 ) {
        lowerObject();
        selectedObjectPosition = -1;
    }
    else
        m_view->kPresenterDoc()->raiseAndLowerObject = false;

    m_activePage->deSelectAllObj();
    m_view->kPresenterDoc()->stickyPage()->deSelectAllObj();

    //desactivate kptextview when we switch of page
    if(m_currentTextObjectView)
    {
        m_currentTextObjectView->terminate();
        delete m_currentTextObjectView;
        m_currentTextObjectView=0L;
    }
    mouseSelectedObject = false;
    emit objectSelectedChanged();
}


void KPrCanvas::setMouseSelectedObject(bool b)
{
    mouseSelectedObject = b;
    emit objectSelectedChanged();
}

/*======================== setup menus ===========================*/
void KPrCanvas::setupMenus()
{
    // create right button presentation menu
    presMenu = new QPopupMenu();
    Q_CHECK_PTR( presMenu );
    presMenu->setCheckable( true );
    PM_SM = presMenu->insertItem( i18n( "&Switching mode" ), this, SLOT( switchingMode() ) );
    PM_DM = presMenu->insertItem( i18n( "&Drawing mode" ), this, SLOT( drawingMode() ) );
    presMenu->insertSeparator();
    presMenu->insertItem( SmallIcon("goto"), i18n( "&Goto Page..." ), this, SLOT( slotGotoPage() ) );
    presMenu->insertSeparator();
    presMenu->insertItem( i18n( "&Exit Presentation" ), this, SLOT( slotExitPres() ) );
    presMenu->setItemChecked( PM_SM, true );
    presMenu->setItemChecked( PM_DM, false );
    presMenu->setMouseTracking( true );
}

/*======================== clipboard cut =========================*/
void KPrCanvas::clipCut()
{
    if ( m_currentTextObjectView )
        m_currentTextObjectView->cut();
    m_view->editCut();
}

/*======================== clipboard copy ========================*/
void KPrCanvas::clipCopy()
{
    if ( m_currentTextObjectView )
        m_currentTextObjectView->copy();
    m_view->editCopy();
}

/*====================== clipboard paste =========================*/
void KPrCanvas::clipPaste()
{
    if ( m_currentTextObjectView )
        m_currentTextObjectView->paste();
    m_view->editPaste();
}

/*======================= change picture  ========================*/
void KPrCanvas::chPic()
{
    bool state=m_activePage->chPic( m_view);
    if( state)
        return;
    m_view->kPresenterDoc()->stickyPage()->chPic(m_view);
}

/*======================= change clipart  ========================*/
void KPrCanvas::chClip()
{
    bool state=m_activePage->chClip( m_view);
    if( state)
        return;
    m_view->kPresenterDoc()->stickyPage()->chClip(m_view);
}

void KPrCanvas::setFont(const QFont &font, bool _subscript, bool _superscript, const QColor &col, const QColor &backGroundColor, int flags)

{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setFont( font, _subscript, _superscript, col, backGroundColor, flags );
}

void KPrCanvas::setTextColor( const QColor &color )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextColor( color );
}

void KPrCanvas::setTextBackgroundColor( const QColor &color )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextBackgroundColor( color );
}

void KPrCanvas::setTextBold( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setBold( b );
}

void KPrCanvas::setTextItalic( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setItalic( b );
}

void KPrCanvas::setTextUnderline( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setUnderline( b );
}

void KPrCanvas::setTextStrikeOut( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setStrikeOut( b );
}

void KPrCanvas::setTextFamily( const QString &f )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setFamily( f );
}

void KPrCanvas::setTextPointSize( int s )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setPointSize( s );
}

void KPrCanvas::setTextSubScript( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextSubScript( b );
}

void KPrCanvas::setTextSuperScript( bool b )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTextSuperScript( b );
}


void KPrCanvas::setTextDefaultFormat( )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setDefaultFormat( );
}

void KPrCanvas::setIncreaseFontSize()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    int size=12;
    if(!lst.isEmpty())
        size=static_cast<int>( KoTextZoomHandler::layoutUnitPtToPt(lst.first()->currentFormat()->font().pointSize()));
    for ( ; it.current() ; ++it )
        it.current()->setPointSize( size+1 );
}

void KPrCanvas::setDecreaseFontSize()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
        int size=12;
    if(!lst.isEmpty())
        size=static_cast<int>( KoTextZoomHandler::layoutUnitPtToPt(lst.first()->currentFormat()->font().pointSize()));
    for ( ; it.current() ; ++it )
        it.current()->setPointSize( size-1 );
}

/*===================== set text alignment =======================*/
void KPrCanvas::setTextAlign( int align )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setAlign(align);
}

void KPrCanvas::setTabList( const KoTabulatorList & tabList )
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setTabList(tabList );
}

void KPrCanvas::setTextDepthPlus()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    double leftMargin=0.0;
    if(!lst.isEmpty())
        leftMargin=lst.first()->currentParagLayoutFormat()->margins[QStyleSheetItem::MarginLeft];
    double indent = m_view->kPresenterDoc()->getIndentValue();
    double newVal = leftMargin + indent;
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginLeft, newVal);
    if(!lst.isEmpty())
    {
        const KoParagLayout *layout=lst.first()->currentParagLayoutFormat();
        m_view->showRulerIndent( layout->margins[QStyleSheetItem::MarginLeft], layout->margins[QStyleSheetItem::MarginFirstLine], layout->margins[QStyleSheetItem::MarginRight]);
    }
}

void KPrCanvas::setTextDepthMinus()
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    double leftMargin=0.0;
    if(!lst.isEmpty())
        leftMargin=lst.first()->currentParagLayoutFormat()->margins[QStyleSheetItem::MarginLeft];
    double indent = m_view->kPresenterDoc()->getIndentValue();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    double newVal = leftMargin - indent;
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginLeft, QMAX( newVal, 0 ));
    if(!lst.isEmpty())
    {
        const KoParagLayout *layout=lst.first()->currentParagLayoutFormat();
        m_view->showRulerIndent( layout->margins[QStyleSheetItem::MarginLeft], layout->margins[QStyleSheetItem::MarginFirstLine], layout->margins[QStyleSheetItem::MarginRight]);
    }
}

void KPrCanvas::setNewFirstIndent(double _firstIndent)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    double index=0.0;
    if(!lst.isEmpty())
        index=lst.first()->currentParagLayoutFormat()->margins[QStyleSheetItem::MarginLeft];
    QPtrListIterator<KoTextFormatInterface> it( lst );
    double val = _firstIndent - index;
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginFirstLine, val);
}

void KPrCanvas::setNewLeftIndent(double _leftIndent)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginLeft, _leftIndent);
}

void KPrCanvas::setNewRightIndent(double _rightIndent)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setMargin(QStyleSheetItem::MarginRight, _rightIndent);
}

void KPrCanvas::setTextCounter(KoParagCounter counter)
{
    QPtrList<KoTextFormatInterface> lst = applicableTextInterfaces();
    QPtrListIterator<KoTextFormatInterface> it( lst );
    for ( ; it.current() ; ++it )
        it.current()->setCounter(counter );
}

#ifndef NDEBUG
void KPrCanvas::printRTDebug( int info )
{
    KPTextObject *kpTxtObj = 0;
    if ( m_currentTextObjectView )
        kpTxtObj = m_currentTextObjectView->kpTextObject();
    else
        kpTxtObj = selectedTextObjs().first();
    if ( kpTxtObj )
        kpTxtObj->textObject()->printRTDebug( info );
}
#endif

bool KPrCanvas::haveASelectedPictureObj()
{
    bool state=m_activePage->haveASelectedPictureObj();
    if(state)
        return true;
    return m_view->kPresenterDoc()->stickyPage()->haveASelectedPictureObj();
}

bool KPrCanvas::haveASelectedPartObj()
{
    bool state = m_activePage->haveASelectedPartObj();
    if ( state )
        return true;
    return m_view->kPresenterDoc()->stickyPage()->haveASelectedPartObj();
}

QPtrList<KPTextObject> KPrCanvas::applicableTextObjects() const
{
    QPtrList<KPTextObject> lst;
    // If we're editing a text object, then that's the one we return
    if ( m_currentTextObjectView )
        lst.append( m_currentTextObjectView->kpTextObject() );
    else
        lst = selectedTextObjs();
    return lst;
}

QPtrList<KoTextFormatInterface> KPrCanvas::applicableTextInterfaces() const
{
    QPtrList<KoTextFormatInterface> lst;
    // If we're editing a text object, then that's the one we return
    if ( m_currentTextObjectView )
        lst.append( m_currentTextObjectView );
    else
    {
        QPtrListIterator<KPObject> it(getObjectList());
        for ( ; it.current(); ++it ) {
            if ( it.current()->isSelected() && it.current()->getType() == OT_TEXT )
                lst.append( static_cast<KPTextObject*>( it.current() )->textObject() );
        }
        //get sticky obj
        it=m_view->kPresenterDoc()->stickyPage()->objectList();
        for ( ; it.current(); ++it ) {
            if ( it.current()->isSelected() && it.current()->getType() == OT_TEXT )
                lst.append( static_cast<KPTextObject*>( it.current() )->textObject() );
        }
    }
    return lst;

}

QPtrList<KPTextObject> KPrCanvas::selectedTextObjs() const
{
    QPtrList<KPTextObject> lst;
    QPtrListIterator<KPObject> it(getObjectList());
    for ( ; it.current(); ++it ) {
        if ( it.current()->isSelected() && it.current()->getType() == OT_TEXT )
            lst.append( static_cast<KPTextObject*>( it.current() ) );
    }
    return lst;
}

/*====================== start screenpresentation ================*/
void KPrCanvas::startScreenPresentation( float presFakt, int curPgNum /* 1-based */)
{
    _presFakt = presFakt;
    m_showOnlyPage = curPgNum;
    kdDebug(33001) << "KPrCanvas::startScreenPresentation m_showOnlyPage=" << m_showOnlyPage << endl;
    kdDebug(33001) << "                              _presFakt=" << _presFakt << endl;

    presMenu->setItemChecked( PM_SM, true );
    presMenu->setItemChecked( PM_DM, false );

    setCursor( waitCursor );
    tmpObjs.clear();

    exitEditMode();

    //kdDebug(33001) << "Page::startScreenPresentation Zooming backgrounds" << endl;
    // Zoom backgrounds to the correct size for full screen
    KPresenterDoc * doc = m_view->kPresenterDoc();
    m_zoomBeforePresentation=doc->zoomHandler()->zoom();
    // ## TODO get rid of presFakt
    doc->zoomHandler()->setZoomAndResolution( qRound(_presFakt*m_zoomBeforePresentation), QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY() );
    doc->newZoomAndResolution(false,false);

    if(m_showOnlyPage==-1)
    {
        QPtrListIterator<KPObject> oIt(doc->pageList().at(0)->objectList());
        for (; oIt.current(); ++oIt )
             tmpObjs.append( oIt.current() );

    }
    else
    {
        QPtrListIterator<KPObject> oIt(doc->pageList().at(m_showOnlyPage-1)->objectList());
        for (; oIt.current(); ++oIt )
             tmpObjs.append( oIt.current() );
    }
    if ( m_showOnlyPage != -1 ) // only one page
    {
        slideList.clear();
        slideList << m_showOnlyPage;
    }
    else // all selected pages
    {
        slideList = doc->selectedSlides();
        // ARGLLLRGLRLGRLG selectedSlides gets us 0-based numbers,
        // and here we want 1-based numbers !
        QString debugstr;
        for ( QValueList<int>::Iterator it = slideList.begin() ; it != slideList.end(); ++ it )
        {
            *it = (*it)+1;
            debugstr += QString::number(*it) + ',';
        }
    }
    Q_ASSERT( slideList.count() );

    slideListIterator = slideList.begin();
    setCursor( blankCursor );

    currPresPage = (unsigned int) -1; // force gotoPage to do something
    gotoPage( *slideListIterator );
    kdDebug(33001) << "Page::startScreenPresentation - done" << endl;
}

/*====================== stop screenpresentation =================*/
void KPrCanvas::stopScreenPresentation()
{
    //kdDebug(33001) << "KPrCanvas::stopScreenPresentation m_showOnlyPage=" << m_showOnlyPage << endl;
    setCursor( waitCursor );

    KPresenterDoc * doc = m_view->kPresenterDoc();
    _presFakt = 1.0;
    doc->zoomHandler()->setZoomAndResolution( m_zoomBeforePresentation, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY() );
    doc->newZoomAndResolution(false,false);
    goingBack = false;
    currPresPage = 1;
    editMode = true;
    drawMode = false;
    repaint( false );
    setToolEditMode( toolEditMode );
    tmpObjs.clear();
    setWFlags( WResizeNoErase );
}

/*========================== next ================================*/
bool KPrCanvas::pNext( bool )
{
    //bool clearSubPres = false;

    goingBack = false;

    kdDebug(33001) << "\n-------\nKPrCanvas::pNext currPresStep=" << currPresStep << " subPresStep=" << subPresStep << endl;

    // First try to go one sub-step further, if any object requires it
    QPtrListIterator<KPObject> oit(getObjectList());
    for ( int i = 0 ; oit.current(); ++oit, ++i )
    {
        KPObject *kpobject = oit.current();
        if ( kpobject->getPresNum() == static_cast<int>( currPresStep )
             && kpobject->getType() == OT_TEXT && kpobject->getEffect2() != EF2_NONE )
        {
            if ( static_cast<int>( subPresStep + 1 ) < kpobject->getSubPresSteps() )
            {
                kdDebug(33001) << "Page::pNext addSubPres subPresStep is now " << subPresStep+1 << endl;
                subPresStep++;
                doObjEffects();
                return false;
            }
        }
    }

    // Then try to see if there is still one step to do in the current page
    if ( (int)currPresStep < *( --presStepList.end() ) )
    {
        QValueList<int>::ConstIterator it = presStepList.find( currPresStep );
        currPresStep = *( ++it );
        subPresStep = 0;
        //kdDebug(33001) << "Page::pNext setting currPresStep to " << currPresStep << endl;

        if ( currPresStep == 0 )
        {
            QPainter p;
            p.begin( this );
            drawBackground( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ) );
            p.end();
        }

        doObjEffects();
        return false;
    }

    // No more steps in this page, try to go to the next page
    QValueList<int>::ConstIterator test(  slideListIterator );
    if ( ++test != slideList.end() )
    {
        if ( !spManualSwitch() && nextPageTimer ) {
            QValueList<int>::ConstIterator it( slideListIterator );
            m_view->setCurrentTimer( m_view->kPresenterDoc()->pageList().at((*it) - 1 )->getPageTimer() );

            nextPageTimer = false;

            return false;
        }

        QPixmap _pix1( QApplication::desktop()->width(), QApplication::desktop()->height() );
        drawCurrentPageInPix( _pix1 );

        currPresPage = *( ++slideListIterator );
        subPresStep = 0;
        kdDebug(33001) << "Page::pNext going to page " << currPresPage << endl;

        tmpObjs.clear();

        setActivePage(m_view->kPresenterDoc()->pageList().at(currPresPage-1));

        QPtrListIterator<KPObject> oIt( getObjectList() );
        for (; oIt.current(); ++oIt )
            tmpObjs.append(oIt.current());

        presStepList = m_view->kPresenterDoc()->reorderPage( currPresPage-1 );
        currPresStep = *presStepList.begin();

        QPixmap _pix2( QApplication::desktop()->width(), QApplication::desktop()->height() );
        int pageHeight = m_view->kPresenterDoc()->pageList().at(currPresPage-1)->getZoomPageRect().height();
        int yOffset = ( presPage() - 1 ) * pageHeight;
        if ( height() > pageHeight )
            yOffset -= ( height() - pageHeight ) / 2;
        drawCurrentPageInPix( _pix2 );

        QValueList<int>::ConstIterator it( slideListIterator );
        --it;

        if ( !spManualSwitch() )
            m_view->autoScreenPresStopTimer();

        KPBackGround * backtmp=m_view->kPresenterDoc()->pageList().at( ( *it ) - 1 )->background();
        PageEffect _pageEffect = backtmp->getPageEffect();

        bool _soundEffect = backtmp->getPageSoundEffect();
        QString _soundFileName = backtmp->getPageSoundFileName();

        if ( _pageEffect != PEF_NONE && _soundEffect && !_soundFileName.isEmpty() ) {
            stopSound();
            playSound( _soundFileName );
        }

        changePages( _pix1, _pix2, _pageEffect );

        if ( m_view->kPresenterDoc()->presentationDuration() )
            m_view->setPresentationDuration( currPresPage - 2 );


        if ( !spManualSwitch() )
            m_view->autoScreenPresReStartTimer();

        return true;
    }

    // No more slides. Redisplay last one, then
    kdDebug(33001) << "Page::pNext last slide -> again" << endl;
    emit stopPres();
    presStepList = m_view->kPresenterDoc()->reorderPage( currPresPage-1);
    currPresStep = *presStepList.begin();
    doObjEffects();
    return false;
}

/*====================== previous ================================*/
bool KPrCanvas::pPrev( bool /*manual*/ )
{
    goingBack = true;
    subPresStep = 0;

    if ( (int)currPresStep > *presStepList.begin() ) {
        QValueList<int>::ConstIterator it = presStepList.find( currPresStep );
        currPresStep = *( --it );
        repaint( false );
        return false;
    } else {
        if ( slideListIterator == slideList.begin() ) {
            presStepList = m_view->kPresenterDoc()->reorderPage( currPresPage - 1 );
            currPresStep = *presStepList.begin();
            repaint( false );
            return false;
        }
        currPresPage = *( --slideListIterator );

        tmpObjs.clear();
        //change active page.
        setActivePage(m_view->kPresenterDoc()->pageList().at( currPresPage - 1 ) );
        QPtrListIterator<KPObject> oIt( getObjectList() );
        for (; oIt.current(); ++oIt )
            tmpObjs.append(oIt.current());
        presStepList = m_view->kPresenterDoc()->reorderPage( currPresPage - 1 );
        currPresStep = *( --presStepList.end() );

        if ( m_view->kPresenterDoc()->presentationDuration() )
            m_view->setPresentationDuration( currPresPage );

        return true;
    }

    return false;
}

/*================================================================*/
bool KPrCanvas::canAssignEffect( QPtrList<KPObject> &objs ) const
{
    QPtrListIterator<KPObject> oIt( m_activePage->objectList() );
    for (; oIt.current(); ++oIt )
        if ( oIt.current()->isSelected() )
            objs.append( oIt.current() );
    oIt= m_view->kPresenterDoc()->stickyPage()->objectList();
    for (; oIt.current(); ++oIt )
    {
        //can't assign a effect to header/footer
        if(m_view->kPresenterDoc()->isHeaderFooter(oIt.current()))
            continue;
        if ( oIt.current()->isSelected() )
            objs.append( oIt.current() );
    }
    return !objs.isEmpty();
}

/*================================================================*/
bool KPrCanvas::isOneObjectSelected()
{
    bool state=m_activePage->isOneObjectSelected();
    if( state)
        return true;
    return m_view->kPresenterDoc()->stickyPage()->isOneObjectSelected();
}

/*================================================================*/
// This one is used to generate the pixmaps for the HTML presentation,
// for the pres-structure-dialog, for the sidebar previews, for template icons.
void KPrCanvas::drawPageInPix( QPixmap &_pix, int pgnum )
{
    //kdDebug(33001) << "Page::drawPageInPix" << endl;
    currPresPage = pgnum + 1;

    QPainter p;
    p.begin( &_pix );

    bool _editMode = editMode;
    editMode = false;

    drawBackground( &p, _pix.rect() );

    //objects in current page
    drawAllObjectsInPage( &p, m_view->kPresenterDoc()->pageList().at( currPresPage-1 )->objectList() );

    //draw sticky object
    drawAllObjectsInPage( &p, m_view->kPresenterDoc()->stickyPage()->objectList() );

    editMode = _editMode;
    p.end();
}

/*==================== draw a page in a pixmap ===================*/
// This one is used in fullscreenmode, to generate the pixmaps used for the
// page effects.
void KPrCanvas::drawCurrentPageInPix( QPixmap &_pix )
{
    //kdDebug(33001) << "Page::drawCurrentPageInPix" << endl;

    QPainter p;
    p.begin( &_pix );

    drawBackground( &p, _pix.rect() );
    drawObjects( &p, _pix.rect(), false/*no cursor*/, SM_NONE, true/*obj-specific effects*/ );

    p.end();
}

/*==================== print a page ===================*/
void KPrCanvas::printPage( QPainter* painter, int pageNum )
{
    //kdDebug(33001) << "KPrCanvas::printPage" << endl;
    KPrPage* page = m_view->kPresenterDoc()->pageList().at(pageNum);
    QRect rect = page->getZoomPageRect();
// TODO set current page to "page" ?
    drawBackground( painter, rect );
    drawObjects( painter, rect, false, SM_NONE, false/*no specific effects*/ );
}

/*=========================== change pages =======================*/
void KPrCanvas::changePages( QPixmap _pix1, QPixmap _pix2, PageEffect _effect )
{
    QTime _time;
    int _step = 0, _steps, _h, _w, _x, _y;

    switch ( _effect )
    {
    case PEF_NONE:
    {
        bitBlt( this, 0, 0, &_pix2, 0, 0, _pix2.width(), _pix2.height() );
    } break;
    case PEF_CLOSE_HORZ:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height()/( 2 * _steps ) ) * _step;
                _h = _h > _pix2.height() / 2 ? _pix2.height() / 2 : _h;

                bitBlt( this, 0, 0, &_pix2, 0, _pix2.height() / 2 - _h, width(), _h );
                bitBlt( this, 0, height() - _h, &_pix2, 0, _pix2.height() / 2, width(), _h );

                _time.restart();
            }
            if ( ( _pix2.height()/( 2 * _steps ) ) * _step >= _pix2.height() / 2 ) break;
        }
    } break;
    case PEF_CLOSE_VERT:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width()/( 2 * _steps ) ) * _step;
                _w = _w > _pix2.width() / 2 ? _pix2.width() / 2 : _w;

                bitBlt( this, 0, 0, &_pix2, _pix2.width() / 2 - _w, 0, _w, height() );
                bitBlt( this, width() - _w, 0, &_pix2, _pix2.width() / 2, 0, _w, height() );

                _time.restart();
            }
            if ( ( _pix2.width()/( 2 * _steps ) ) * _step >= _pix2.width() / 2 ) break;
        }
    } break;
    case PEF_CLOSE_ALL:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width()/( 2 * _steps ) ) * _step;
                _w = _w > _pix2.width() / 2 ? _pix2.width() / 2 : _w;

                _h = ( _pix2.height()/( 2 * _steps ) ) * _step;
                _h = _h > _pix2.height() / 2 ? _pix2.height() / 2 : _h;

                bitBlt( this, 0, 0, &_pix2, 0, 0, _w, _h );
                bitBlt( this, width() - _w, 0, &_pix2, width() - _w, 0, _w, _h );
                bitBlt( this, 0, height() - _h, &_pix2, 0, height() - _h, _w, _h );
                bitBlt( this, width() - _w, height() - _h, &_pix2, width() - _w, height() - _h, _w, _h );

                _time.restart();
            }
            if ( ( _pix2.width()/( 2 * _steps ) ) * _step >= _pix2.width() / 2
                 && ( _pix2.height()/( 2 * _steps ) ) * _step >= _pix2.height() / 2 ) break;
        }
    } break;
    case PEF_OPEN_HORZ:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                _y = _pix2.height() / 2;

                bitBlt( this, 0, _y - _h / 2, &_pix2, 0, _y - _h / 2, width(), _h );

                _time.restart();
            }
            if ( ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_OPEN_VERT:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                _x = _pix2.width() / 2;

                bitBlt( this, _x - _w / 2, 0, &_pix2, _x - _w / 2, 0, _w, height() );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() ) break;
        }
    } break;
    case PEF_OPEN_ALL:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                _x = _pix2.width() / 2;

                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                _y = _pix2.height() / 2;

                bitBlt( this, _x - _w / 2, _y - _h / 2, &_pix2, _x - _w / 2, _y - _h / 2, _w, _h );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() &&
                 ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_INTERLOCKING_HORZ_1:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                bitBlt( this, 0, 0, &_pix2, 0, 0, _w, height() / 4 );
                bitBlt( this, 0, height() / 2, &_pix2, 0, height() / 2, _w, height() / 4 );
                bitBlt( this, width() - _w, height() / 4, &_pix2, width() - _w, height() / 4, _w, height() / 4 );
                bitBlt( this, width() - _w, height() / 2 + height() / 4, &_pix2, width() - _w,
                        height() / 2 + height() / 4, _w, height() / 4 );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() ) break;
        }
    } break;
    case PEF_INTERLOCKING_HORZ_2:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _w = ( _pix2.width() / _steps ) * _step;
                _w = _w > _pix2.width() ? _pix2.width() : _w;

                bitBlt( this, 0, height() / 4, &_pix2, 0, height() / 4, _w, height() / 4 );
                bitBlt( this, 0, height() / 2 + height() / 4, &_pix2, 0, height() / 2 + height() / 4, _w, height() / 4 );
                bitBlt( this, width() - _w, 0, &_pix2, width() - _w, 0, _w, height() / 4 );
                bitBlt( this, width() - _w, height() / 2, &_pix2, width() - _w, height() / 2, _w, height() / 4 );

                _time.restart();
            }
            if ( ( _pix2.width() / _steps ) * _step >= _pix2.width() ) break;
        }
    } break;
    case PEF_INTERLOCKING_VERT_1:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                bitBlt( this, 0, 0, &_pix2, 0, 0, width() / 4, _h );
                bitBlt( this, width() / 2, 0, &_pix2, width() / 2, 0, width() / 4, _h );
                bitBlt( this, width() / 4, height() - _h, &_pix2, width() / 4, height() - _h, width() / 4, _h );
                bitBlt( this, width() / 2 + width() / 4, height() - _h, &_pix2, width() / 2 + width() / 4, height() - _h,
                        width() / 4, _h );

                _time.restart();
            }
            if ( ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_INTERLOCKING_VERT_2:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                _h = ( _pix2.height() / _steps ) * _step;
                _h = _h > _pix2.height() ? _pix2.height() : _h;

                bitBlt( this, width() / 4, 0, &_pix2, width() / 4, 0, width() / 4, _h );
                bitBlt( this, width() / 2 + width() / 4, 0, &_pix2, width() / 2 + width() / 4, 0, width() / 4, _h );
                bitBlt( this, 0, height() - _h, &_pix2, 0, height() - _h, width() / 4, _h );
                bitBlt( this, width() / 2, height() - _h, &_pix2, width() / 2, height() - _h, width() / 4, _h );

                _time.restart();
            }
            if ( ( _pix2.height() / _steps ) * _step >= _pix2.height() ) break;
        }
    } break;
    case PEF_SURROUND1:
    {
        int wid = kapp->desktop()->width() / 10;
        int hei = kapp->desktop()->height() / 10;

        int curr = 1;
        int curr2 = 1;

        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->width() ) / pageSpeedFakt() );
        _time.start();

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;

                if ( curr == 1 || curr == 5 || curr == 9 || curr == 13 || curr == 17 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _h = ( _pix2.height() / _steps ) * _step;
                    if ( _h >= _pix2.height() - 2 * dy )
                    {
                        _h = _pix2.height() - 2 * dy;
                        curr++;
                        _step = 0;
                    }
                    bitBlt( this, dx, dy, &_pix2, dx, dy, wid, _h );
                }
                else if ( curr == 2 || curr == 6 || curr == 10 || curr == 14 || curr == 18 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _w = ( _pix2.width() / _steps ) * _step;
                    if ( _w >= _pix2.width() - wid - 2 * dx )
                    {
                        _w = _pix2.width() - wid - 2 * dx;
                        curr++;
                        _step = 0;
                    }
                    bitBlt( this, dx + wid, height() - hei - dy, &_pix2, dx + wid, height() - hei - dy, _w, hei );
                }
                else if ( curr == 3 || curr == 7 || curr == 11 || curr == 15 || curr == 19 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _h = ( _pix2.height() / _steps ) * _step;
                    if ( _h >= _pix2.height() - hei - 2 * dy )
                    {
                        _h = _pix2.height() - hei - 2 * dy;
                        curr++;
                        _step = 0;
                    }
                    bitBlt( this, _pix2.width() - wid - dx, _pix2.height() - hei - dy - _h, &_pix2,
                            _pix2.width() - wid - dx, _pix2.height() - hei - dy - _h, wid, _h );
                }
                else if ( curr == 4 || curr == 8 || curr == 12 || curr == 16 || curr == 20 )
                {
                    int dx = ( curr2 / 4 ) * wid;
                    int dy = ( curr2 / 4 ) * hei;
                    _w = ( _pix2.width() / _steps ) * _step;
                    if ( _w >= _pix2.width() - 2 * wid - 2 * dx )
                    {
                        _w = _pix2.width() - 2 * wid - 2 * dx;
                        _steps *= 2;
                        _steps = static_cast<int>( static_cast<float>( _steps ) / 1.5 );
                        curr++;
                        curr2 += 4;
                        _step = 0;
                    }
                    bitBlt( this, _pix2.width() - dx - wid - _w, dy, &_pix2, _pix2.width() - dx - wid - _w,
                            dy, _w, hei );
                }
                _time.restart();
            }
            if ( curr == 21 )
            {
                bitBlt( this, 0, 0, &_pix2, 0, 0, _pix2.width(), _pix2.height() );
                break;
            }
        }
    } break;
    case PEF_FLY1:
    {
        _steps = static_cast<int>( static_cast<float>( kapp->desktop()->height() ) / pageSpeedFakt() );
        _time.start();

        int _psteps = _steps / 5;
        QRect oldRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() );
        QSize ps;
        QPixmap pix3;

        for ( ; ; )
        {
            kapp->processEvents();
            if ( _time.elapsed() >= 1 )
            {
                _step++;
                if ( _step < _psteps )
                {
                    pix3 = QPixmap( _pix1 );
                    QPixmap pix4( _pix2 );
                    float dw = static_cast<float>( _step * ( ( pix3.width() - ( pix3.width() / 10 ) ) /
                                                             ( 2 * _psteps ) ) );
                    float dh = static_cast<float>( _step * ( ( pix3.height() - ( pix3.height() / 10 ) ) /
                                                             ( 2 * _psteps ) ) );

                    dw *= 2;
                    dh *= 2;

                    QWMatrix m;
                    m.scale( static_cast<float>( pix3.width() - dw ) / static_cast<float>( pix3.width() ),
                             static_cast<float>( pix3.height() - dh ) / static_cast<float>( pix3.height() ) );
                    pix3 = pix3.xForm( m );
                    ps = pix3.size();

                    bitBlt( &pix4, ( pix4.width() - pix3.width() ) / 2, ( pix4.height() - pix3.height() ) / 2,
                            &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( ( pix4.width() - pix3.width() ) / 2, ( pix4.height() - pix3.height() ) / 2,
                                   pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                if ( _step > _psteps && _step < _psteps * 2 )
                {
                    QPixmap pix4( _pix2 );
                    int yy = ( _pix1.height() - pix3.height() ) / 2 - ( ( ( _pix1.height() - pix3.height() ) / 2 ) /
                                                                        _psteps ) * ( _step - _psteps );

                    bitBlt( &pix4, ( pix4.width() - pix3.width() ) / 2, yy,
                            &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( ( pix4.width() - pix3.width() ) / 2, yy,
                                   pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                if ( _step > 2 * _psteps && _step < _psteps * 3 )
                {
                    QPixmap pix4( _pix2 );
                    int xx = ( _pix1.width() - pix3.width() ) / 2 - ( ( ( _pix1.width() - pix3.width() ) / 2 ) /
                                                                      _psteps ) * ( _step - 2 * _psteps );
                    int yy = ( ( ( _pix1.height() - pix3.height() ) / 2 ) / _psteps ) * ( _step - 2 * _psteps );

                    bitBlt( &pix4, xx, yy, &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( xx, yy, pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                if ( _step > 3 * _psteps && _step < _psteps * 5 )
                {
                    QPixmap pix4( _pix2 );
                    int xx = ( ( _pix1.width() - pix3.width() ) / _psteps ) * ( _step - 3 * _psteps );
                    int yy = ( ( _pix1.height() - pix3.height() ) / 2 ) +
                             ( ( ( _pix1.height() - pix3.height() ) / 2 ) / _psteps ) * ( _step - 3 * _psteps );

                    bitBlt( &pix4, xx, yy, &pix3, 0, 0, pix3.width(), pix3.height() );
                    QRect newRect( xx, yy, pix3.width(), pix3.height() );
                    QRect r = newRect.unite( oldRect );
                    bitBlt( this, r.x(), r.y(), &pix4, r.x(), r.y(), r.width(), r.height() );
                    oldRect = newRect;
                }
                _time.restart();
            }
            if ( _step >= _steps )
            {
                bitBlt( this, 0, 0, &_pix2, 0, 0, _pix2.width(), _pix2.height() );
                break;
            }
        }
    } break;
    }
}

/*================================================================*/
void KPrCanvas::doObjEffects()
{
    /// ### Note: this is for full-screen mode only.
    /// ### There should be NO use of diffx(), diffy() anywhere in this method!

    QPixmap screen_orig( kapp->desktop()->width(), kapp->desktop()->height() );
    bool drawn = false;

    // YABADABADOOOOOOO.... That's a hack :-)
    if ( subPresStep == 0 && currPresPage > 0 )
    {
        //kdDebug(33001) << "Page::doObjEffects - in the strange hack" << endl;
        inEffect = true;
        QPainter p;
        p.begin( &screen_orig );
	QRect desktopRect = QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() );
        drawBackground( &p, desktopRect );
        drawObjects( &p, desktopRect, false, SM_NONE, true );
        p.end();
        inEffect = false;
        bitBlt( this, 0, 0, &screen_orig, 0, 0, screen_orig.width(), screen_orig.height() );
        drawn = true;
    }

    QPtrList<KPObject> _objList;
    QTime _time;
    int _step = 0, _steps1 = 0, _steps2 = 0, x_pos1 = 0, y_pos1 = 0;
    int x_pos2 = kapp->desktop()->width(), y_pos2 = kapp->desktop()->height(), _step_width = 0, _step_height = 0;
    int w_pos1 = 0, h_pos1;
    bool effects = false;
    bool nothingHappens = false;
    int timer = 0;
    bool _soundEffect = false;
    QString _soundFileName = QString::null;
    if ( !drawn )
        bitBlt( &screen_orig, 0, 0, this, 0, 0, kapp->desktop()->width(), kapp->desktop()->height() );
    QPixmap *screen = new QPixmap( screen_orig );

    QPtrListIterator<KPObject> oit(getObjectList());
    for ( int i = 0 ; oit.current(); ++oit, ++i )
    {
        KPObject *kpobject = oit.current();
        if (  kpobject->getPresNum() == static_cast<int>( currPresStep ) )
        {
            if ( !spManualSwitch() )
                timer = kpobject->getAppearTimer();

            if ( kpobject->getEffect() != EF_NONE )
            {
                _soundEffect = kpobject->getAppearSoundEffect();
                _soundFileName = kpobject->getAppearSoundEffectFileName();
                _objList.append( kpobject );

                QRect br = m_view->zoomHandler()->zoomRect( kpobject->getBoundingRect(m_view->zoomHandler()) );
                int x = br.x();
                int y = br.y();
                int w = br.width();
                int h = br.height();

                switch ( kpobject->getEffect() )
                {
                case EF_COME_LEFT:
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    break;
                case EF_COME_TOP:
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                    break;
                case EF_COME_RIGHT:
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    break;
                case EF_COME_BOTTOM:
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                    break;
                case EF_COME_LEFT_TOP:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF_COME_LEFT_BOTTOM:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF_COME_RIGHT_TOP:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF_COME_RIGHT_BOTTOM:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF_WIPE_LEFT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF_WIPE_RIGHT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF_WIPE_TOP:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                case EF_WIPE_BOTTOM:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                default: break;
                }
                effects = true;
            }
        }
        else if (  kpobject->getDisappear() && kpobject->getDisappearNum() == static_cast<int>( currPresStep ) )
        {
            if ( !spManualSwitch() )
                timer = kpobject->getDisappearTimer();

            if ( kpobject->getEffect3() != EF3_NONE )
            {
                _soundEffect = kpobject->getDisappearSoundEffect();
                _soundFileName = kpobject->getDisappearSoundEffectFileName();

                _objList.append( kpobject );

                int x = 0, y = 0, w = 0, h = 0;
                QRect br = m_view->zoomHandler()->zoomRect( kpobject->getBoundingRect(m_view->zoomHandler()) );
                x = br.x(); y = br.y(); w = br.width(); h = br.height();

                switch ( kpobject->getEffect3() )
                {
                case EF3_GO_LEFT:
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    break;
                case EF3_GO_TOP:
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                    break;
                case EF3_GO_RIGHT:
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    break;
                case EF3_GO_BOTTOM:
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                    break;
                case EF3_GO_LEFT_TOP:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF3_GO_LEFT_BOTTOM:
                {
                    x_pos1 = QMAX( x_pos1, x - diffx() + w );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF3_GO_RIGHT_TOP:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos1 = QMAX( y_pos1, y - diffy() + h );
                } break;
                case EF3_GO_RIGHT_BOTTOM:
                {
                    x_pos2 = QMIN( x_pos2, x - diffx() );
                    y_pos2 = QMIN( y_pos2, y - diffy() );
                } break;
                case EF3_WIPE_LEFT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF3_WIPE_RIGHT:
                    x_pos1 = QMAX( x_pos1, w );
                    break;
                case EF3_WIPE_TOP:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                case EF3_WIPE_BOTTOM:
                    y_pos1 = QMAX( y_pos1, h );
                    break;
                default: break;
                }
                effects = true;
            }
        }
    }

    if ( effects )
    {
        if ( !spManualSwitch() && timer > 0 )
            m_view->autoScreenPresStopTimer();

        if ( _soundEffect && !_soundFileName.isEmpty() ) {
            stopSound();
            playSound( _soundFileName );
        }

        _step_width = static_cast<int>( ( static_cast<float>( kapp->desktop()->width() ) / objSpeedFakt() ) );
        _step_height = static_cast<int>( ( static_cast<float>( kapp->desktop()->height() ) / objSpeedFakt() ) );
        _steps1 = x_pos1 > y_pos1 ? x_pos1 / _step_width : y_pos1 / _step_height;
        _steps2 = kapp->desktop()->width() - x_pos2 > kapp->desktop()->height() - y_pos2 ?
                  ( kapp->desktop()->width() - x_pos2 ) / _step_width : ( kapp->desktop()->height() - y_pos2 ) / _step_height;
        _time.start();

        QPtrList<QRect> xy;
        xy.setAutoDelete( true );

        for ( ; ; )
        {
            kapp->processEvents();
            if ( nothingHappens ) break; // || _step >= _steps1 && _step >= _steps2 ) break;

            QPtrList<QRect> changes;
            changes.setAutoDelete( true );

            if ( _time.elapsed() >= 1 )
            {
                nothingHappens = true;
                _step++;

                changes.clear();

                for ( int i = 0; i < static_cast<int>( _objList.count() ); i++ )
                {
                    KPObject * kpobject = _objList.at( i );
                    // Origin of the object, in pixels
                    QPoint objectOrig = m_view->zoomHandler()->zoomPoint( kpobject->getOrig() );
                    // Distance from object to bottom right position of the screen...
                    int _w =  kapp->desktop()->width() - ( objectOrig.x() /*- diffx()*/ );
                    int _h =  kapp->desktop()->height() - ( objectOrig.y() /*- diffy()*/ );
                    QRect objectRect = m_view->zoomHandler()->zoomRect( kpobject->getRect() );
                    int ox = objectRect.x();
                    int oy = objectRect.y();
                    int ow = objectRect.width();
                    int oh = objectRect.height();

                    QRect br = m_view->zoomHandler()->zoomRect( kpobject->getBoundingRect(m_view->zoomHandler()) );
                    int bx = br.x();
                    int by = br.y();
                    int bw = br.width();
                    int bh = br.height();

                    QRect oldRect;
                    QRect newRect;

                    if ( static_cast<int>( xy.count() - 1 ) < i )
                    {
                        xy.append( new QRect( 0, 0, 0, 0 ) );
                        oldRect.setRect( bx - diffx(), by - diffy(), bw, bh );
                    }
                    else
                        oldRect.setRect( bx - ( diffx() - xy.at( i )->x() ), by - ( diffy() - xy.at( i )->y() ), bw - xy.at( i )->width(), bh - xy.at( i )->height() );

                    if ( !kpobject->getDisappear() || kpobject->getDisappear() && kpobject->getDisappearNum() != static_cast<int>( currPresStep ) )
                    {
                        switch ( kpobject->getEffect() )
                        {
                        case EF_NONE:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                                drawObject( kpobject, screen, ox, oy, 0, 0, 0, 0 );
                        } break;
                        case EF_COME_LEFT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = 0;
                                drawObject( kpobject, screen, -x_pos1, y_pos1, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -x_pos1 );
                                xy.at( i )->setY( y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                x_pos1 = 0;
                                drawObject( kpobject, screen, x_pos1, -y_pos1, 0, 0, 0, 0 );
                                if ( y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos1 );
                                xy.at( i )->setY( -y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_RIGHT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = 0;
                                drawObject( kpobject, screen, x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                x_pos2 = 0;
                                drawObject( kpobject, screen, x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_LEFT_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                drawObject( kpobject, screen, -x_pos1, -y_pos1, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -x_pos1 );
                                xy.at( i )->setY( -y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_LEFT_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                drawObject( kpobject, screen, -x_pos1, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -x_pos1 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_RIGHT_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                drawObject( kpobject, screen, x_pos2, -y_pos1, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( -y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_COME_RIGHT_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                drawObject( kpobject, screen, x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_WIPE_LEFT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                w_pos1 = _step_width * ( _steps1 - _step ) > 0 ? _step_width * ( _steps1 - _step ) : 0;
                                drawObject( kpobject, screen, 0, 0, w_pos1, 0, 0, 0 );
                                if ( w_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( 0 );
                                xy.at( i )->setY( 0 );
                                xy.at( i )->setWidth( w_pos1 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_WIPE_RIGHT:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                w_pos1 = _step_width * ( _steps1 - _step ) > 0 ? _step_width * ( _steps1 - _step ) : 0;
                                x_pos1 = w_pos1;
                                drawObject( kpobject, screen, 0, 0, w_pos1, 0, x_pos1, 0 );
                                if ( w_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos1 );
                                xy.at( i )->setY( 0 );
                                xy.at( i )->setWidth( w_pos1 );
                                xy.at( i )->setHeight( 0 );
                            }
                        } break;
                        case EF_WIPE_TOP:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                h_pos1 = _step_height * ( _steps1 - _step ) > 0 ? _step_height * ( _steps1 - _step ) : 0;
                                drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, 0 );
                                if ( h_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( 0 );
                                xy.at( i )->setY( 0 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( h_pos1 );
                            }
                        } break;
                        case EF_WIPE_BOTTOM:
                        {
                            if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT && kpobject->getEffect2() == EF2T_PARA )
                            {
                                h_pos1 = _step_height * ( _steps1 - _step ) > 0 ? _step_height * ( _steps1 - _step ) : 0;
                                y_pos1 = h_pos1;
                                drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, y_pos1 );
                                if ( h_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( 0 );
                                xy.at( i )->setY( y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( h_pos1 );
                            }
                        } break;
                        default: break;
                        }
                    }
                    else
                    {
                        if ( subPresStep == 0 )
                        {
                            switch ( kpobject->getEffect3() )
                            {
                            case EF3_NONE:
                                //drawObject( kpobject, screen, ox, oy, 0, 0, 0, 0 );
                                break;
                            case EF3_GO_LEFT:
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = 0;
                                drawObject( kpobject, screen, -( ox + ow - x_pos1 ), y_pos1, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -( ox + ow - x_pos1 ) );
                                xy.at( i )->setY( y_pos1 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_TOP:
                            {
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                x_pos1 = 0;
                                drawObject( kpobject, screen, x_pos1, -( ( oy - diffy() ) + oh - y_pos1 ), 0, 0, 0, 0 );
                                if ( y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos1 );
                                xy.at( i )->setY( -( ( oy - diffy() ) + oh - y_pos1 ) );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_RIGHT:
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = 0;
                                int __w = kapp->desktop()->width() - ox;
                                drawObject( kpobject, screen, __w - x_pos2, y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( __w - x_pos2 );
                                xy.at( i )->setY( y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_BOTTOM:
                            {
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                x_pos2 = 0;
                                int __h = kapp->desktop()->height() - ( oy - diffy() );
                                drawObject( kpobject, screen, x_pos2, __h - y_pos2, 0, 0, 0, 0 );
                                if ( y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( x_pos2 );
                                xy.at( i )->setY( __h - y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_LEFT_TOP:
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                drawObject( kpobject, screen, -( ox + ow - x_pos1 ), -( ( oy - diffy() ) + oh - y_pos1 ), 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -( ox + ow - x_pos1 ) );
                                xy.at( i )->setY( -( ( oy - diffy() ) + oh - y_pos1 ) );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_LEFT_BOTTOM:
                            {
                                x_pos1 = _step_width * _step < ox - diffx() + ow ?
                                         ox - diffx() + ow - _step_width * _step : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                int __h = kapp->desktop()->height() - ( oy - diffy() );
                                drawObject( kpobject, screen, -( ox + ow - x_pos1 ), __h -  y_pos2, 0, 0, 0, 0 );
                                if ( x_pos1 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( -( ox + ow - x_pos1 ) );
                                xy.at( i )->setY( __h - y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_RIGHT_TOP:
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos1 = _step_height * _step < oy - diffy() + oh ?
                                         oy - diffy() + oh - _step_height * _step : 0;
                                int __w = kapp->desktop()->width() - ox;
                                drawObject( kpobject, screen, __w - x_pos2, -( ( oy - diffy() ) + oh - y_pos1 ), 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos1 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( __w - x_pos2 );
                                xy.at( i )->setY( -( ( oy - diffy() ) + oh - y_pos1 ) );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_GO_RIGHT_BOTTOM:
                            {
                                x_pos2 = _w - ( _step_width * _step ) + ( ox - diffx() ) > ox - diffx() ?
                                         _w - ( _step_width * _step ) : 0;
                                y_pos2 = _h - ( _step_height * _step ) + ( oy - diffy() ) > oy - diffy() ?
                                         _h - ( _step_height * _step ) : 0;
                                int __w = kapp->desktop()->width() - ox;
                                int __h = kapp->desktop()->height() - ( oy - diffy() );
                                drawObject( kpobject, screen, __w - x_pos2, __h - y_pos2, 0, 0, 0, 0 );
                                if ( x_pos2 != 0 || y_pos2 != 0 ) nothingHappens = false;
                                xy.at( i )->setX( __w - x_pos2 );
                                xy.at( i )->setY( __h - y_pos2 );
                                xy.at( i )->setWidth( 0 );
                                xy.at( i )->setHeight( 0 );
                            } break;
                            case EF3_WIPE_LEFT:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    w_pos1 = _step_width * ( _step - 1 );
                                    x_pos1 = w_pos1;
                                    drawObject( kpobject, screen, 0, 0, w_pos1, 0, x_pos1, 0 );
                                    if ( ( _step_width * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( x_pos1 );
                                    xy.at( i )->setY( 0 );
                                    xy.at( i )->setWidth( w_pos1 );
                                    xy.at( i )->setHeight( 0 );
                                }
                            } break;
                            case EF3_WIPE_RIGHT:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    w_pos1 = _step_width * ( _step - 1 );
                                    drawObject( kpobject, screen, 0, 0, w_pos1, 0, 0, 0 );
                                    if ( ( _step_width * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( 0 );
                                    xy.at( i )->setY( 0 );
                                    xy.at( i )->setWidth( w_pos1 );
                                    xy.at( i )->setHeight( 0 );
                                }
                            } break;
                            case EF3_WIPE_TOP:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    h_pos1 = _step_height * ( _step - 1 );
                                    y_pos1 = h_pos1;
                                    drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, y_pos1 );
                                    if ( ( _step_height * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( 0 );
                                    xy.at( i )->setY( y_pos1 );
                                    xy.at( i )->setWidth( 0 );
                                    xy.at( i )->setHeight( h_pos1 );
                                }
                            } break;
                            case EF3_WIPE_BOTTOM:
                            {
                                if ( subPresStep == 0 || subPresStep != 0 && kpobject->getType() == OT_TEXT &&
                                     kpobject->getEffect2() == EF2T_PARA )
                                {
                                    h_pos1 = _step_height * ( _step - 1 );
                                    drawObject( kpobject, screen, 0, 0, 0, h_pos1, 0, 0 );
                                    if ( ( _step_height * ( _steps1 - _step ) ) != 0 ) nothingHappens = false;
                                    xy.at( i )->setX( 0 );
                                    xy.at( i )->setY( 0 );
                                    xy.at( i )->setWidth( 0 );
                                    xy.at( i )->setHeight( h_pos1 );
                                }
                            } break;
                            default:
                                break;
                            }
                        }
                    }
                    newRect.setRect( bx - ( diffx() - xy.at( i )->x() ), by - ( diffy() - xy.at( i )->y() ), bw - xy.at( i )->width(), bh - xy.at( i )->height() );
                    newRect = newRect.unite( oldRect );

                    bool append = true;
                    for ( unsigned j = 0; j < changes.count(); j++ )
                    {
                        if ( changes.at( j )->intersects( newRect ) )
                        {
                            QRect r = changes.at( j )->intersect( newRect );
                            int s1 = r.width() * r.height();
                            int s2 = newRect.width() * newRect.height();
                            int s3 = changes.at( j )->width() * changes.at( j )->height();

                            if ( s1 > ( s2 / 100 ) * 50 || s1 > ( s3 / 100 ) * 50 )
                            {
                                QRect rr = changes.at( j )->unite( newRect );
                                changes.at( j )->setRect( rr.x(), rr.y(), rr.width(), rr.height() );
                                append = false;
                            }

                            break;
                        }
                    }

                    if ( append )
                        changes.append( new QRect( newRect ) );
                }

                QRect *changed;
                for ( int i = 0; i < static_cast<int>( changes.count() ); i++ )
                {
                    changed = changes.at( i );
                    bitBlt( this, changed->x(), changed->y(), screen, changed->x(), changed->y(), changed->width(), changed->height() );
                }

                delete screen;
                screen = new QPixmap( screen_orig );

                _time.restart();
            }
        }
    }

    if ( !effects )
    {
        //kdDebug(33001) << "KPrCanvas::doObjEffects no effects" << endl;
        QPainter p;
        p.begin( this );
        p.drawPixmap( 0, 0, screen_orig );
        drawObjects( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ),
		     false, SM_NONE, true );
        p.end();
    }
    else
    {
        //kdDebug(33001) << "KPrCanvas::doObjEffects effects" << endl;
        QPainter p;
        p.begin( screen );
        drawObjects( &p, QRect( 0, 0, kapp->desktop()->width(), kapp->desktop()->height() ),
		     false, SM_NONE, true );
        p.end();
        bitBlt( this, 0, 0, screen );
    }

    if ( !spManualSwitch() && timer > 0 )
        m_view->setCurrentTimer( timer );

    delete screen;
}

/*======================= draw object ============================*/
void KPrCanvas::drawObject( KPObject *kpobject, QPixmap *screen, int _x, int _y, int _w, int _h, int _cx, int _cy )
{
    // ### TODO use _x and _y !! painter translation maybe ?
    if ( kpobject->getDisappear() &&
         kpobject->getDisappearNum() < static_cast<int>( currPresStep ) )
        return;
    int ox, oy, ow, oh;
    KoRect br = kpobject->getBoundingRect( m_view->zoomHandler() );
    QRect brpix = m_view->zoomHandler()->zoomRect( br );
    ox = brpix.x(); oy = brpix.y(); ow = brpix.width(); oh = brpix.height();
    bool ownClipping = true;

    QPainter p;
    p.begin( screen );

    if ( _w != 0 || _h != 0 )
    {
        p.setClipping( true );
        p.setClipRect( ox + _cx, oy + _cy, ow - _w, oh - _h, QPainter::CoordPainter );
        ownClipping = false;
    }

    if ( !editMode && static_cast<int>( currPresStep ) == kpobject->getPresNum() && !goingBack )
    {
        kpobject->setSubPresStep( subPresStep );
        kpobject->doSpecificEffects( true );
        kpobject->setOwnClipping( ownClipping );
    }
    p.translate(_x,_y);
    kpobject->draw( &p, m_view->zoomHandler(), SM_NONE,
		    ( kpobject->isSelected()) && drawContour );
    kpobject->setSubPresStep( 0 );
    kpobject->doSpecificEffects( false );
    kpobject->setOwnClipping( true );

    KPObject *obj = 0;
    for ( unsigned int i = tmpObjs.findRef( kpobject ) +1 ; i < tmpObjs.count(); i++ ) {
        obj = tmpObjs.at( i );
        if ( kpobject->getBoundingRect(m_view->zoomHandler() ).intersects( obj->getBoundingRect( m_view->zoomHandler() ) ) &&
             obj->getPresNum() < static_cast<int>( currPresStep ) )
        {
            obj->draw( &p, m_view->zoomHandler(), SM_NONE,
		       (obj->isSelected()) && drawContour);
        }
    }

    p.end();
}

/*======================== print =================================*/
void KPrCanvas::print( QPainter *painter, KPrinter *printer, float /*left_margin*/, float /*top_margin*/ )
{
    deSelectAllObj();
    printer->setFullPage( true );
    int i = 0;

    repaint( false );
    kapp->processEvents();

    editMode = false;
    fillBlack = false;
    _presFakt = 1.0;

    //int _xOffset = diffx();
    //int _yOffset = diffy();

    currPresStep = 1000;
    subPresStep = 1000;

    //m_view->setDiffX( -static_cast<int>( MM_TO_POINT( left_margin ) ) );
    //m_view->setDiffY( -static_cast<int>( MM_TO_POINT( top_margin ) ) );

    //QColor c = kapp->winStyleHighlightColor();
    //kapp->setWinStyleHighlightColor( colorGroup().highlight() ); // deprecated in Qt3

    QProgressDialog progress( i18n( "Printing..." ), i18n( "Cancel" ),
                              printer->toPage() - printer->fromPage() + 2, this );

    int j = 0;
    progress.setProgress( 0 );

    /*if ( printer->fromPage() > 1 )
        m_view->setDiffY( ( printer->fromPage() - 1 ) * ( getPageRect( 1, 1.0, false ).height() ) -
                        (int)MM_TO_POINT( top_margin ) );*/
    QValueList<int> list=printer->pageList();
    QValueList<int>::iterator it;
    for( it=list.begin();it!=list.end();++it)
    {
      i=(*it);
        progress.setProgress( ++j );
        kapp->processEvents();

        if ( progress.wasCancelled() )
            break;

        currPresPage = i;
        if ( i > printer->fromPage() ) printer->newPage();

        painter->resetXForm();
        painter->fillRect( m_activePage->getZoomPageRect(), white );

        printPage( painter, i - 1 );
        kapp->processEvents();

        painter->resetXForm();
        kapp->processEvents();

        /*m_view->setDiffY( i * ( getPageRect( 1, 1.0, false ).height() )
                        - static_cast<int>( MM_TO_POINT( top_margin ) ) );*/
    }

    NoteBar *noteBar = m_view->getNoteBar();
    //don't printing note when there is not note to print
    if ( noteBar && !noteBar->getAllNoteTextForPrinting().isEmpty()) {
        printer->newPage();
        painter->resetXForm();
        noteBar->printNote( painter, printer );
        painter->resetXForm();
    }

    setToolEditMode( toolEditMode );
    //m_view->setDiffX( _xOffset );
    //m_view->setDiffY( _yOffset );

    progress.setProgress( printer->toPage() - printer->fromPage() + 2 );
    //kapp->setWinStyleHighlightColor( c );

    currPresPage = 1;
    currPresStep = 0;
    subPresStep = 0;
    _presFakt = 1.0;
    fillBlack = true;
    editMode = true;
    repaint( false );
}

/*================================================================*/
KPTextObject* KPrCanvas::insertTextObject( const QRect& _r )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    KPTextObject* obj = m_activePage->insertTextObject( rect );
    selectObj( obj );
    return obj;
}

/*================================================================*/
void KPrCanvas::insertLineH( const QRect& _r, bool rev )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertLine( rect, m_view->getPen(),
                                       !rev ? m_view->getLineBegin() : m_view->getLineEnd(), !rev ? m_view->getLineEnd() : m_view->getLineBegin(),
                                       LT_HORZ );
}

/*================================================================*/
void KPrCanvas::insertLineV( const QRect &_r, bool rev )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertLine( rect, m_view->getPen(),
                                       !rev ? m_view->getLineBegin() : m_view->getLineEnd(), !rev ? m_view->getLineEnd() : m_view->getLineBegin(),
                                       LT_VERT );
}

/*================================================================*/
void KPrCanvas::insertLineD1( const QRect &_r, bool rev )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertLine( rect, m_view->getPen(),
                                       !rev ? m_view->getLineBegin() : m_view->getLineEnd(), !rev ? m_view->getLineEnd() : m_view->getLineBegin(),
                                       LT_LU_RD );
}

/*================================================================*/
void KPrCanvas::insertLineD2( const QRect &_r, bool rev )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertLine(rect, m_view->getPen(),
                                       !rev ? m_view->getLineBegin() : m_view->getLineEnd(), !rev ? m_view->getLineEnd() : m_view->getLineBegin(),
                                       LT_LD_RU );
}

/*================================================================*/
void KPrCanvas::insertRect( const QRect& _r )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertRectangle( rect, m_view->getPen(), m_view->getBrush(), m_view->getFillType(),
                                   m_view->getGColor1(), m_view->getGColor2(), m_view->getGType(), m_view->getRndX(), m_view->getRndY(),
                                   m_view->getGUnbalanced(), m_view->getGXFactor(), m_view->getGYFactor() );
}

/*================================================================*/
void KPrCanvas::insertEllipse( const QRect &_r )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertCircleOrEllipse( rect, m_view->getPen(), m_view->getBrush(), m_view->getFillType(),
                                         m_view->getGColor1(), m_view->getGColor2(),
                                         m_view->getGType(), m_view->getGUnbalanced(), m_view->getGXFactor(), m_view->getGYFactor() );
}

/*================================================================*/
void KPrCanvas::insertPie( const QRect &_r )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertPie( rect, m_view->getPen(), m_view->getBrush(), m_view->getFillType(),
                             m_view->getGColor1(), m_view->getGColor2(), m_view->getGType(),
                             m_view->getPieType(), m_view->getPieAngle(), m_view->getPieLength(),
                             m_view->getLineBegin(), m_view->getLineEnd(), m_view->getGUnbalanced(), m_view->getGXFactor(), m_view->getGYFactor() );
}

/*================================================================*/
void KPrCanvas::insertAutoform( const QRect &_r, bool rev )
{
    rev = false;
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertAutoform( rect, m_view->getPen(), m_view->getBrush(),
                                  !rev ? m_view->getLineBegin() : m_view->getLineEnd(), !rev ? m_view->getLineEnd() : m_view->getLineBegin(),
                                  m_view->getFillType(), m_view->getGColor1(), m_view->getGColor2(), m_view->getGType(),
                                  autoform, m_view->getGUnbalanced(), m_view->getGXFactor(), m_view->getGYFactor() );
}

/*================================================================*/
void KPrCanvas::insertObject( const QRect &_r )
{
    QRect r(_r);
    r.moveBy(diffx(),diffy());
    KoRect rect=m_view->zoomHandler()->unzoomRect(r);
    m_activePage->insertObject( rect, partEntry );
}

/*================================================================*/
void KPrCanvas::insertFreehand( const KoPointArray &_pointArray )
{
    KoRect rect = _pointArray.boundingRect();

    double ox = rect.x();
    double oy = rect.y();

    unsigned int index = 0;

    KoPointArray points( _pointArray );
    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() - ox ;
        double tmpY = point.y() - oy ;
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }
    rect.moveBy(m_view->zoomHandler()->unzoomItX(diffx()),m_view->zoomHandler()->unzoomItY(diffy()));
    m_activePage->insertFreehand( tmpPoints, rect, m_view->getPen(), m_view->getLineBegin(),
                                           m_view->getLineEnd() );

    m_pointArray = KoPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void KPrCanvas::insertPolyline( const KoPointArray &_pointArray )
{
    KoRect rect = _pointArray.boundingRect();

    double ox = rect.x();
    double oy = rect.y();
    unsigned int index = 0;

    KoPointArray points( _pointArray );
    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() - ox ;
        double tmpY = point.y() - oy ;
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }
    rect.moveBy(m_view->zoomHandler()->unzoomItX(diffx()),m_view->zoomHandler()->unzoomItY(diffy()));
    m_activePage->insertPolyline( tmpPoints, rect, m_view->getPen(), m_view->getLineBegin(),
                                           m_view->getLineEnd() );

    m_pointArray = KoPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void KPrCanvas::insertCubicBezierCurve( const KoPointArray &_pointArray )
{
    KoPointArray _points( _pointArray );
    KoPointArray _allPoints;
    unsigned int pointCount = _points.count();
    KoRect _rect;

    if ( pointCount == 2 ) { // line
        _rect = _points.boundingRect();
        _allPoints = _points;
    }
    else { // cubic bezier curve
        KoPointArray tmpPointArray;
        unsigned int _tmpIndex = 0;
        unsigned int count = 0;
        while ( count < pointCount ) {
            if ( pointCount >= ( count + 4 ) ) { // for cubic bezier curve
                double _firstX = _points.at( count ).x();
                double _firstY = _points.at( count ).y();

                double _fourthX = _points.at( count + 1 ).x();
                double _fourthY = _points.at( count + 1 ).y();

                double _secondX = _points.at( count + 2 ).x();
                double _secondY = _points.at( count + 2 ).y();

                double _thirdX = _points.at( count + 3 ).x();
                double _thirdY = _points.at( count + 3 ).y();

                KoPointArray _cubicBezierPoint;
                _cubicBezierPoint.putPoints( 0, 4, _firstX,_firstY, _secondX,_secondY, _thirdX,_thirdY, _fourthX,_fourthY );

                _cubicBezierPoint = _cubicBezierPoint.cubicBezier();

                KoPointArray::ConstIterator it;
                for ( it = _cubicBezierPoint.begin(); it != _cubicBezierPoint.end(); ++it ) {
                    KoPoint _point = (*it);
                    tmpPointArray.putPoints( _tmpIndex, 1, _point.x(), _point.y() );
                    ++_tmpIndex;
                }

                count += 4;
            }
            else { // for line
                double _x1 = _points.at( count ).x();
                double _y1 = _points.at( count ).y();

                double _x2 = _points.at( count + 1 ).x();
                double _y2 = _points.at( count + 1 ).y();

                tmpPointArray.putPoints( _tmpIndex, 2, _x1,_y1, _x2,_y2 );
                _tmpIndex += 2;
                count += 2;
            }
        }

        _rect = tmpPointArray.boundingRect();
        _allPoints = tmpPointArray;
    }

    double ox = _rect.x();
    double oy = _rect.y();
    unsigned int index = 0;

    KoPointArray points( _pointArray );
    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() - ox;
        double tmpY = point.y() - oy;
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }

    index = 0;
    KoPointArray tmpAllPoints;
    for ( it = _allPoints.begin(); it != _allPoints.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() - ox ;
        double tmpY = point.y() - oy;
        tmpAllPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }
    _rect.moveBy(m_view->zoomHandler()->unzoomItX(diffx()),m_view->zoomHandler()->unzoomItY(diffy()));
    if ( toolEditMode == INS_CUBICBEZIERCURVE ) {
        m_activePage->insertCubicBezierCurve( tmpPoints, tmpAllPoints, _rect, m_view->getPen(),
                                                       m_view->getLineBegin(), m_view->getLineEnd() );
    }
    else if ( toolEditMode == INS_QUADRICBEZIERCURVE ) {
        m_activePage->insertQuadricBezierCurve( tmpPoints, tmpAllPoints, _rect, m_view->getPen(),
                                                         m_view->getLineBegin(), m_view->getLineEnd() );
    }

    m_pointArray = KoPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void KPrCanvas::insertPolygon( const KoPointArray &_pointArray )
{
    KoPointArray points( _pointArray );
    KoRect rect= points.boundingRect();
    double ox = rect.x();
    double oy = rect.y();
    unsigned int index = 0;

    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() - ox;
        double tmpY = point.y() - oy;
        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }
    rect.moveBy(m_view->zoomHandler()->unzoomItX(diffx()),m_view->zoomHandler()->unzoomItY(diffy()));
    m_activePage->insertPolygon( tmpPoints, rect, m_view->getPen(), m_view->getBrush(), m_view->getFillType(),
                                          m_view->getGColor1(), m_view->getGColor2(), m_view->getGType(), m_view->getGUnbalanced(),
                                          m_view->getGXFactor(), m_view->getGYFactor(),
                                          m_view->getCheckConcavePolygon(), m_view->getCornersValue(), m_view->getSharpnessValue() );

    m_pointArray = KoPointArray();
    m_indexPointArray = 0;
}

/*================================================================*/
void KPrCanvas::insertPicture( const QRect &_r )
{
    QRect r( _r );
    r.moveBy( diffx(), diffy() );
    KoRect rect = m_view->zoomHandler()->unzoomRect( r );
    QString file = m_activePage->insPictureFile();

    QCursor c = cursor();
    setCursor( waitCursor );
    if ( !file.isEmpty() ) {
        m_activePage->insertPicture( file, rect );
        m_activePage->setInsPictureFile( QString::null );
    }
    setCursor( c );
}

/*================================================================*/
void KPrCanvas::insertClipart( const QRect &_r )
{
    QRect r( _r );
    r.moveBy( diffx(), diffy() );
    KoRect rect = m_view->zoomHandler()->unzoomRect( r );
    QString file = m_activePage->insClipartFile();

    QCursor c = cursor();
    setCursor( waitCursor );
    if ( !file.isEmpty() ) {
        m_activePage->insertClipart( file, rect );
        m_activePage->setInsClipartFile( QString::null );
    }
    setCursor( c );
}

/*================================================================*/
void KPrCanvas::setToolEditMode( ToolEditMode _m, bool updateView )
{
    //store m_pointArray if !m_pointArray.isNull()
    if(toolEditMode == INS_POLYLINE && !m_pointArray.isNull())
    {
        endDrawPolyline();
    }

    if ( ( toolEditMode == INS_CUBICBEZIERCURVE || toolEditMode == INS_QUADRICBEZIERCURVE ) && !m_pointArray.isNull() )
        endDrawCubicBezierCurve();


    exitEditMode();
    toolEditMode = _m;

    if ( toolEditMode == TEM_MOUSE )
    {
        setCursor( arrowCursor );
        QPoint pos=QCursor::pos();
        KPObject *obj=m_activePage->getCursor( pos);
        if(obj)
            setCursor( obj->getCursor( pos, modType,m_view->kPresenterDoc() ) );
        else
        {
            obj=m_view->kPresenterDoc()->stickyPage()->getCursor( pos );
            if(obj)
                setCursor( obj->getCursor( pos, modType,m_view->kPresenterDoc() ) );
        }
    }
    else
        setCursor( crossCursor );

    if ( updateView )
        m_view->setTool( toolEditMode );
    repaint();
}


void KPrCanvas::endDrawPolyline()
{
    m_drawPolyline = false;
    insertPolyline( m_pointArray );
    emit objectSelectedChanged();
    if ( toolEditMode != TEM_MOUSE && editMode )
        repaint( false );
    mousePressed = false;
    modType = MT_NONE;
    resizeObjNum = 0L;
    ratio = 0.0;
    keepRatio = false;
}

void KPrCanvas::endDrawCubicBezierCurve()
{
    m_drawCubicBezierCurve = false;
    m_oldCubicBezierPointArray = KoPointArray();
    insertCubicBezierCurve( m_pointArray );
    emit objectSelectedChanged();
    if ( toolEditMode != TEM_MOUSE && editMode )
        repaint( false );
    mousePressed = false;
    modType = MT_NONE;
    resizeObjNum = 0L;
    ratio = 0.0;
    keepRatio = false;
}

/*================================================================*/
void KPrCanvas::selectNext()
{
    if ( objectList().count() == 0 ) return;

    if ( m_activePage->numSelected() == 0 )
        objectList().at( 0 )->setSelected( true );
    else {
        int i = objectList().findRef( m_activePage->getSelectedObj() );
        if ( i < static_cast<int>( objectList().count() ) - 1 ) {
            m_view->kPresenterDoc()->deSelectAllObj();
            objectList().at( ++i )->setSelected( true );
        } else {
            m_view->kPresenterDoc()->deSelectAllObj();
            objectList().at( 0 )->setSelected( true );
        }
    }
    if ( !QRect( diffx(), diffy(), width(), height() ).
         contains( m_view->zoomHandler()->zoomRect(m_activePage->getSelectedObj()->getBoundingRect( m_view->zoomHandler() ) ),m_view->zoomHandler() ))
        m_view->makeRectVisible( m_view->zoomHandler()->zoomRect( m_activePage->getSelectedObj()->getBoundingRect( m_view->zoomHandler() ) ));
    _repaint( false );
}

/*================================================================*/
void KPrCanvas::selectPrev()
{
    if ( objectList().count() == 0 ) return;

    if ( m_activePage->numSelected() == 0 )
        objectList().at( objectList().count() - 1 )->setSelected( true );
    else {
        int i = objectList().findRef( m_activePage->getSelectedObj() );
        if ( i > 0 ) {
            m_view->kPresenterDoc()->deSelectAllObj();
            objectList().at( --i )->setSelected( true );
        } else {
            m_view->kPresenterDoc()->deSelectAllObj();
            objectList().at( objectList().count() - 1 )->setSelected( true );
        }
    }
    m_view->makeRectVisible( m_view->zoomHandler()->zoomRect(m_activePage->getSelectedObj()->getBoundingRect(m_view->zoomHandler() )) );
    _repaint( false );
}

/*================================================================*/
void KPrCanvas::dragEnterEvent( QDragEnterEvent *e )
{
    if ( m_currentTextObjectView )
    {
        m_currentTextObjectView->dragEnterEvent( e );
    }
    else if ( /*QTextDrag::canDecode( e ) ||*/
         QImageDrag::canDecode( e ) )
        e->accept();
    else
        e->ignore();
}

/*================================================================*/
void KPrCanvas::dragLeaveEvent( QDragLeaveEvent *e )
{
    if(m_currentTextObjectView)
        m_currentTextObjectView->dragLeaveEvent( e );
}

/*================================================================*/
void KPrCanvas::dragMoveEvent( QDragMoveEvent *e )
{
    if( m_currentTextObjectView)
    {
        m_currentTextObjectView->dragMoveEvent( e, QPoint() );
    }
    else if ( /*QTextDrag::canDecode( e ) ||*/
         QImageDrag::canDecode( e ) )
        e->accept();
    else
        e->ignore();
}

/*================================================================*/
void KPrCanvas::dropEvent( QDropEvent *e )
{
    //disallow dropping objects outside the "page"
    KoPoint docPoint = m_view->zoomHandler()->unzoomPoint( e->pos()+QPoint(diffx(),diffy()) );
    if ( !m_activePage->getZoomPageRect().contains(e->pos()))
        return;

    if ( QImageDrag::canDecode( e ) ) {
        setToolEditMode( TEM_MOUSE );
        deSelectAllObj();

        QImage pix;
        QImageDrag::decode( e, pix );

        KTempFile tmpFile;
        tmpFile.setAutoDelete(true);

	if( tmpFile.status() != 0 ) {
	    return;
	}
        tmpFile.close();

        pix.save( tmpFile.name(), "PNG" );
        QCursor c = cursor();
        setCursor( waitCursor );
        m_activePage->insertPicture( tmpFile.name(), e->pos().x(), e->pos().y()  );
        setCursor( c );

        e->accept();
    } else if ( QUriDrag::canDecode( e ) ) {
        setToolEditMode( TEM_MOUSE );
        deSelectAllObj();

        QStringList lst;
        QUriDrag::decodeToUnicodeUris( e, lst );

        QStringList::ConstIterator it = lst.begin();
        for ( ; it != lst.end(); ++it ) {
            KURL url( *it );

            QString filename;
            if ( !url.isLocalFile() ) {
                filename = QString::null;
                // #### todo download file
            } else {
                filename = url.path();
            }

            KMimeMagicResult *res = KMimeMagic::self()->findFileType( filename );

            if ( res && res->isValid() ) {
                QString mimetype = res->mimeType();
                if ( mimetype.contains( "image" ) ) {
                    QCursor c = cursor();
                    setCursor( waitCursor );
                    m_activePage->insertPicture( filename, e->pos().x(), e->pos().y()  );
                    setCursor( c );
                } else if ( mimetype.contains( "text" ) ) {
                    QCursor c = cursor();
                    setCursor( waitCursor );
                    QFile f( filename );
                    QTextStream t( &f );
                    QString text = QString::null, tmp;

                    if ( f.open( IO_ReadOnly ) ) {
                        while ( !t.eof() ) {
                            tmp = t.readLine();
                            tmp += "\n";
                            text.append( tmp );
                        }
                        f.close();
                    }
                    m_activePage->insertTextObject( m_view->zoomHandler()->unzoomRect(QRect( e->pos().x(), e->pos().y(), 250, 250 )), text, m_view );

                    setCursor( c );
                }
            }
        }
    }
    else if (m_currentTextObjectView)
    {
        m_currentTextObjectView->dropEvent( e );
    }
    else if ( QTextDrag::canDecode( e ) ) {
        setToolEditMode( TEM_MOUSE );
        deSelectAllObj();

        QString text;
        QTextDrag::decode( e, text );
        m_activePage->insertTextObject( m_view->zoomHandler()->unzoomRect( QRect( e->pos().x(), e->pos().y(), 250, 250 )), text, m_view );
        e->accept();
    } else
        e->ignore();

}

/*================================================================*/
void KPrCanvas::slotGotoPage()
{
    setCursor( blankCursor );
    int pg = currPresPage;
    pg = KPGotoPage::gotoPage( m_view->kPresenterDoc(), slideList, pg, this );
    gotoPage( pg );

    if ( !spManualSwitch() ) {
        m_view->setCurrentTimer( 1 );
        setNextPageTimer( true );
    }

    if ( m_view->kPresenterDoc()->presentationDuration() )
        m_view->setPresentationDuration( pg - 1 );

}

/*================================================================*/
void KPrCanvas::gotoPage( int pg )
{
    if ( pg != static_cast<int>( currPresPage ) ) {
        currPresPage = pg;
        kdDebug(33001) << "Page::gotoPage currPresPage=" << currPresPage << endl;
        slideListIterator = slideList.find( currPresPage );
        editMode = false;
        drawMode = false;
        presStepList = m_view->kPresenterDoc()->reorderPage( currPresPage-1);
        currPresStep = *presStepList.begin();
        subPresStep = 0;
        //change active page
        m_activePage=m_view->kPresenterDoc()->pageList().at(currPresPage-1);


        resize( QApplication::desktop()->width(), QApplication::desktop()->height() );
        repaint( false );
        setFocus();
        m_view->refreshPageButton();
    }
}

/*================================================================*/
KPTextObject* KPrCanvas::kpTxtObj()
{
    return ( ( editNum && editNum->getType() == OT_TEXT ) ?
             dynamic_cast<KPTextObject*>( editNum ) : 0 );
    // ### return m_currentTextObjectView->kpTextObject()
}

void KPrCanvas::copyObjs()
{
    QDomDocument doc("DOC");
    QDomElement presenter=doc.createElement("DOC");
    presenter.setAttribute("editor", "KPresenter");
    presenter.setAttribute("mime", "application/x-kpresenter-selection");
    doc.appendChild(presenter);
    m_activePage->copyObjs(doc, presenter);
    m_view->kPresenterDoc()->stickyPage()->copyObjs(doc, presenter);

    QStoredDrag * drag = new QStoredDrag( "application/x-kpresenter-selection" );
    drag->setEncodedData( doc.toCString() );
    kdDebug()<<"doc.toCString() :"<<doc.toCString()<<endl;
    QApplication::clipboard()->setData( drag );
}

/*================================================================*/
void KPrCanvas::deleteObjs()
{
    KMacroCommand *macro=new KMacroCommand(i18n( "Delete object(s)" ));
    bool macroCreate=false;
    KCommand *cmd=m_activePage->deleteObjs();
    if( cmd)
    {
        macro->addCommand(cmd);
        macroCreate=true;
    }
    cmd=m_view->kPresenterDoc()->stickyPage()->deleteObjs();
    if( cmd)
    {
        macro->addCommand(cmd);
        macroCreate=true;
    }
    m_view->kPresenterDoc()->deSelectAllObj();
    if(macroCreate)
        m_view->kPresenterDoc()->addCommand(macro);
    else
        delete macro;
    setToolEditMode( toolEditMode );
}

/*================================================================*/
void KPrCanvas::rotateObjs()
{
    m_view->extraRotate();
    setToolEditMode( toolEditMode );
}

/*================================================================*/
void KPrCanvas::shadowObjs()
{
    m_view->extraShadow();
    setToolEditMode( toolEditMode );
}

/*================================================================*/
void KPrCanvas::enterEvent( QEvent *e )
{
    m_view->setRulerMousePos( ( ( QMouseEvent* )e )->x(), ( ( QMouseEvent* )e )->y() );
    m_view->setRulerMouseShow( true );
}

/*================================================================*/
void KPrCanvas::leaveEvent( QEvent * /*e*/ )
{
    m_view->setRulerMouseShow( false );
}


/*================================================================*/
QPtrList<KPObject> KPrCanvas::objectList()
{
    return m_activePage->objectList();
}
const QPtrList<KPObject> &KPrCanvas::getObjectList() const
{
    return m_activePage->objectList();
}

/*================================================================*/
unsigned int KPrCanvas::objNums() const
{
    return m_activePage->objNums();
}

/*================================================================*/
unsigned int KPrCanvas::currPgNum() const
{
    return m_view->getCurrPgNum();
}

/*================================================================*/
unsigned int KPrCanvas::rastX() const
{
    return m_view->zoomHandler()->zoomItX(m_view->kPresenterDoc()->rastX());
}

/*================================================================*/
unsigned int KPrCanvas::rastY() const
{
    return m_view->zoomHandler()->zoomItY(m_view->kPresenterDoc()->rastY());
}

/*================================================================*/
QColor KPrCanvas::txtBackCol() const
{
    return m_view->kPresenterDoc()->txtBackCol();
}

/*================================================================*/
bool KPrCanvas::spInfinitLoop() const
{
    return m_view->kPresenterDoc()->spInfinitLoop();
}

/*================================================================*/
bool KPrCanvas::spManualSwitch() const
{
    return m_view->kPresenterDoc()->spManualSwitch();
}

/*================================================================*/
QRect KPrCanvas::getPageRect( bool decBorders )
{
    // ### TODO remove diffx, diffy
    return m_view->kPresenterDoc()->getPageRect( decBorders );
}

/*================================================================*/
unsigned int KPrCanvas::pageNums()
{
    return m_view->kPresenterDoc()->getPageNums();
}

/*================================================================*/
float KPrCanvas::objSpeedFakt()
{
    /*
      Used to be 0(slow)->70, 1(medium)->50, 2(fast)->30.
      It's now 0->75, 1->50, 2->37, etc. That's the reason for this strange formula :)
     */
    return 150.0 / static_cast<float>( m_view->kPresenterDoc()->getPresSpeed() + 2 );
    //return ObjSpeed[ static_cast<int>( m_view->kPresenterDoc()->getPresSpeed() ) ];
}

/*================================================================*/
float KPrCanvas::pageSpeedFakt()
{
    /*
      Used to be 0(slow)->8, 1(medium)->16, 2(fast)->32.
      It's now 0->10, 1->20, 2->30, 3->40, 4->50......
     */
    return 10.0 * ( m_view->kPresenterDoc()->getPresSpeed() + 1 );
    //return PageSpeed[ static_cast<int>( m_view->kPresenterDoc()->getPresSpeed() ) ];
}

/*================================================================*/
void KPrCanvas::_repaint( bool /*erase*/ )
{
    m_view->kPresenterDoc()->repaint( false );
}

/*================================================================*/
void KPrCanvas::_repaint( const QRect &r )
{
    m_view->kPresenterDoc()->repaint( r );
}

/*================================================================*/
void KPrCanvas::_repaint( KPObject *o )
{
    m_view->kPresenterDoc()->repaint( o );
}


/*================================================================*/
void KPrCanvas::slotExitPres()
{
    m_view->screenStop();
}


/*================================================================*/
void KPrCanvas::drawingMode()
{
    if(!presMenu->isItemChecked ( PM_DM ))
    {
        presMenu->setItemChecked( PM_DM, true );
        presMenu->setItemChecked( PM_SM, false );
        drawMode = true;
        setCursor( arrowCursor );
    }
}

/*================================================================*/
void KPrCanvas::switchingMode()
{
    if(!presMenu->isItemChecked ( PM_SM ))
    {
        presMenu->setItemChecked( PM_DM, false );
        presMenu->setItemChecked( PM_SM, true );
        drawMode = false; setCursor( blankCursor );

        if ( !spManualSwitch() )
            m_view->autoScreenPresIntervalTimer();
    }
}

/*================================================================*/
bool KPrCanvas::calcRatio( double &dx, double &dy, KPObject *kpobject, double ratio ) const
{
    if ( fabs( dy ) > fabs( dx ) )
        dx = ( dy ) * ratio ;
    else
        dy =  dx  / ratio;
    if ( kpobject->getSize().width() + dx < 20 ||
         kpobject->getSize().height() + dy < 20 )
        return false;
    return true;
}

/*================================================================*/
void KPrCanvas::exitEditMode()
{
  if ( editNum )
    {
      if ( editNum->getType() == OT_TEXT )
	{
	  if(m_currentTextObjectView)
            {
	      m_currentTextObjectView->clearSelection();
	      //hide cursor when we desactivate textObjectView
	      m_currentTextObjectView->drawCursor( false );
	      m_currentTextObjectView->terminate();
	      delete m_currentTextObjectView;
	      m_currentTextObjectView=0L;
            }
	  // Title of slide may have changed
	  emit updateSideBarItem( currPgNum()-1 );
	  emit objectSelectedChanged();
	  editNum=0L;
        }
      else if (editNum->getType() == OT_PART )
	{
	  static_cast<KPPartObject *>(editNum)->deactivate();
	  _repaint( editNum );
	  editNum=0L;
	  return;
        }
    }
}

/*================================================================*/
bool KPrCanvas::getPixmapOrigAndCurrentSize( KPPixmapObject *&obj, KoSize *origSize, KoSize *currentSize )
{
    *origSize = KoSize(obj->originalSize().width(),obj->originalSize().height());
    *currentSize = obj->getSize();
    return true;
}

/*================================================================*/
void KPrCanvas::picViewOrignalSize()
{
    picViewOrigHelper( -1, -1 );
}

/*================================================================*/
void KPrCanvas::picViewOrig640x480()
{
  picViewOrigHelper(640, 480);
}

/*================================================================*/
void KPrCanvas::picViewOrig800x600()
{
  picViewOrigHelper(800, 600);
}

/*================================================================*/
void KPrCanvas::picViewOrig1024x768()
{
  picViewOrigHelper(1024, 768);
}

/*================================================================*/
void KPrCanvas::picViewOrig1280x1024()
{
  picViewOrigHelper(1280, 1024);
}

/*================================================================*/
void KPrCanvas::picViewOrig1600x1200()
{
  picViewOrigHelper(1600, 1200);
}

void KPrCanvas::picViewOrigHelper(int x, int y)
{
  KPPixmapObject *obj = 0;

  KoSize origSize;
  KoSize currentSize;

  obj=m_activePage->picViewOrigHelper();
  if( !obj)
      obj=m_view->kPresenterDoc()->stickyPage()->picViewOrigHelper();


  if ( obj && !getPixmapOrigAndCurrentSize( obj, &origSize, &currentSize ) )
      return;

  KoSize pgSize = m_activePage->getPageRect().size();

  if ( x == -1 && y == -1 ) {
      x = (int)origSize.width();
      y = (int)origSize.height();
  }

  QSize presSize( x, y );

  scalePixmapToBeOrigIn( currentSize, pgSize, presSize, obj );
}

/*================================================================*/
void KPrCanvas::picViewOrigFactor()
{
}

/*================================================================*/
void KPrCanvas::scalePixmapToBeOrigIn( const KoSize &currentSize, const KoSize &pgSize,
                                       const QSize &presSize, KPPixmapObject *obj )
{
    double faktX = (double)presSize.width() / (double)QApplication::desktop()->width();
    double faktY = (double)presSize.height() / (double)QApplication::desktop()->height();
    double w = pgSize.width() * faktX;
    double h = pgSize.height() * faktY;

    ResizeCmd *resizeCmd = new ResizeCmd( i18n( "Scale Picture to be shown 1:1 in presentation mode" ),
                                          KoPoint( 0, 0 ), KoSize( w - currentSize.width(), h - currentSize.height() ),
                                          obj, m_view->kPresenterDoc() );
    resizeCmd->execute();
    m_view->kPresenterDoc()->addCommand( resizeCmd );
}

void KPrCanvas::setTextBackground( KPTextObject */*obj*/ )
{
    // This is called when double-clicking on a text object.
    // What should happen exactly ? (DF)
#if 0
    QPixmap pix( m_activePage->getZoomPageRect().size() );
    QPainter painter( &pix );
    m_activePage->background()->draw( &painter, FALSE );
    QPixmap bpix( obj->getSize().toQSize() ); // ## zoom it !
    bitBlt( &bpix, 0, 0, &pix, obj->getOrig().x(), obj->getOrig().y() -
            m_activePage->getZoomPageRect().height() * ( m_view->getCurrPgNum() - 1 ), bpix.width(), bpix.height() );
    QBrush b( white, bpix );
    QPalette pal( obj->textObjectView()->palette() );
    pal.setBrush( QColorGroup::Base, b );
    obj->textObjectView()->setPalette( pal );
#endif
}

QValueList<int> KPrCanvas::pages(const QString &range) {

    if(range.isEmpty())
        return QValueList<int> ();
    QValueList<int> list;
    int start=-1;
    int end=range.find(',');
    bool ok=true;
    QString tmp;
    while(end!=-1 && start!=end && ok) {
        tmp=range.mid(start+1, end-start-1);
        ok=pagesHelper(tmp, list);
        start=range.find(',', end);
        end=range.find(',', start+1);
    }
    pagesHelper(range.mid(start+1), list);
    return list;
}

bool KPrCanvas::pagesHelper(const QString &chunk, QValueList<int> &list) {

    bool ok=true;
    int mid=chunk.find('-');
    if(mid!=-1) {
        int start=chunk.left(mid).toInt(&ok);
        int end=chunk.mid(mid+1).toInt(&ok);
        while(ok && start<=end)
            list.append(start++);
    }
    else
        list.append(chunk.toInt(&ok));
    return ok;
}

void KPrCanvas::moveObject( int x, int y, bool key )
{
    double newPosX=m_view->zoomHandler()->unzoomItX(x);
    double newPosY=m_view->zoomHandler()->unzoomItY(y);
    KoRect boundingRect = m_boundingRect;
    KoPoint point( m_boundingRect.topLeft() );
    KoRect pageRect=m_activePage->getPageRect();
    point.setX( (m_boundingRect.x()+newPosX) );
    m_boundingRect.moveTopLeft( point );
    if ( ( boundingRect.left()+m_hotSpot.x() < pageRect.left() ) || ( m_boundingRect.left() < pageRect.left() ) )
    {
        point.setX( pageRect.left() );
        m_boundingRect.moveTopLeft( point );
    }
    else if ( ( boundingRect.left()+m_hotSpot.x() > pageRect.width() ) || ( m_boundingRect.right() > pageRect.width() ) )
    {
        point.setX( pageRect.width()-m_boundingRect.width() );
        m_boundingRect.moveTopLeft( point );
    }

    point = m_boundingRect.topLeft();
    point.setY( m_boundingRect.y()+newPosY );
    m_boundingRect.moveTopLeft( point );

    if ( ( boundingRect.top()+m_hotSpot.y() < pageRect.top() ) || ( m_boundingRect.top() < pageRect.top() ) )
    {
        point.setY( pageRect.top() );
        m_boundingRect.moveTopLeft( point );
    }
    else if( ( boundingRect.top()+m_hotSpot.y() > pageRect.height() ) || ( m_boundingRect.bottom() > pageRect.height() ) )
    {
        point.setY( pageRect.height() - m_boundingRect.height() );
        m_boundingRect.moveTopLeft( point );
    }


    if( m_boundingRect.topLeft() == boundingRect.topLeft() )
        return; // nothing happende (probably due to the grid)
    KoPoint _move=m_boundingRect.topLeft()-boundingRect.topLeft();
    KMacroCommand *macro=new KMacroCommand(i18n( "Move object(s)" ));
    bool macroCreate=false;
    KCommand *cmd=m_activePage->moveObject(m_view,_move,key);
    if( cmd && key)
    {
        macro->addCommand(cmd);
        macroCreate=true;
    }
    cmd=m_view->kPresenterDoc()->stickyPage()->moveObject(m_view,_move,key);
    if( cmd && key)
    {
        macro->addCommand(cmd);
        macroCreate=true;
    }
    if(macroCreate)
        m_view->kPresenterDoc()->addCommand(macro);
    else
        delete macro;
}

void KPrCanvas::resizeObject( ModifyType _modType, int _dx, int _dy )
{
    double dx = m_view->zoomHandler()->unzoomItX( _dx);
    double dy = m_view->zoomHandler()->unzoomItY( _dy);
    KoRect page=m_activePage->getPageRect();
    KPObject *kpobject=resizeObjNum;

    KoRect objRect=kpobject->getBoundingRect(m_view->zoomHandler());
    KoRect pageRect=m_activePage->getPageRect();
    KoPoint point=objRect.topLeft();
    QPainter p;
    p.begin( this );
    kpobject->moveBy(m_view->zoomHandler()->unzoomItX(-diffx()),m_view->zoomHandler()->unzoomItY(-diffy()));
    kpobject->draw( &p, m_view->zoomHandler(), SM_MOVERESIZE,
		    (kpobject->isSelected()) && drawContour);
    switch ( _modType ) {
    case MT_RESIZE_LU: {
        if( (point.x()+dx) <(pageRect.left()-1))
            dx=0;
        if( (point.y()+dy) <(pageRect.top()-1))
            dy=0;
        if ( keepRatio && ratio != 0.0 ) {
            if ( !calcRatio( dx, dy, kpobject, ratio ) )
                break;
        }
        kpobject->moveBy( KoPoint( dx, dy ) );
        kpobject->resizeBy( -dx, -dy );
    } break;
    case MT_RESIZE_LF: {
        dy = 0;
        if( (point.x()+dx) <(pageRect.left()-1))
            dx=0;
        if ( keepRatio && ratio != 0.0 ) {
            if ( !calcRatio( dx, dy, kpobject, ratio ) )
                break;
        }
        kpobject->moveBy( KoPoint( dx, 0 ) );
        kpobject->resizeBy( -dx, -dy );
    } break;
    case MT_RESIZE_LD: {
        if( (point.y()+objRect.height()+dy) > pageRect.height())
            dy=0;
        if( (point.x()+dx) <(pageRect.left()-1))
            dx=0;
        if ( keepRatio && ratio != 0.0 )
            break;
        kpobject->moveBy( KoPoint( dx, 0 ) );
        kpobject->resizeBy( -dx, dy );
    } break;
    case MT_RESIZE_RU: {
        if( (point.x()+objRect.width()+dx) > pageRect.width())
            dx=0;
        if( (point.y()+dy) <(pageRect.top()-1))
            dy=0;
        if ( keepRatio && ratio != 0.0 )
            break;
        kpobject->moveBy( KoPoint( 0, dy ) );
        kpobject->resizeBy( dx, -dy );
    } break;
    case MT_RESIZE_RT: {
        dy = 0;
        if( (point.x()+objRect.width()+dx) > pageRect.width())
            dx=0;
        if ( keepRatio && ratio != 0.0 ) {
            if ( !calcRatio( dx, dy, kpobject, ratio ) )
                break;
        }
        kpobject->resizeBy( dx, dy );
    } break;
    case MT_RESIZE_RD: {
        if( (point.y()+objRect.height()+dy) > pageRect.height())
            dy=0;
        if( (point.x()+objRect.width()+dx) > pageRect.width())
            dx=0;
        if ( keepRatio && ratio != 0.0 ) {
            if ( !calcRatio( dx, dy, kpobject, ratio ) )
                break;
        }
        kpobject->resizeBy( KoSize( dx, dy ) );
    } break;
    case MT_RESIZE_UP: {
        dx = 0;
        if( (point.y()+dy) <(pageRect.top()-1))
            dy=0;
        if ( keepRatio && ratio != 0.0 ) {
            if ( !calcRatio( dx, dy, kpobject, ratio ) )
                break;
        }
        kpobject->moveBy( KoPoint( 0, dy ) );
        kpobject->resizeBy( -dx, -dy );
    } break;
    case MT_RESIZE_DN: {
        dx = 0;
        if( (point.y()+objRect.height()+dy) > pageRect.height())
            dy=0;
        if ( keepRatio && ratio != 0.0 ) {
            if ( !calcRatio( dx, dy, kpobject, ratio ) )
                break;
        }
        kpobject->resizeBy( dx, dy );
    } break;
    default: break;
    }
    kpobject->draw( &p, m_view->zoomHandler(), SM_MOVERESIZE,
		    (kpobject->isSelected()) && drawContour );
    kpobject->moveBy(m_view->zoomHandler()->unzoomItX(diffx()),m_view->zoomHandler()->unzoomItY(diffy()));
    p.end();

    _repaint( oldBoundingRect );
    _repaint( kpobject );

    oldBoundingRect = getOldBoundingRect(kpobject);
}

void KPrCanvas::raiseObject( KPObject *_kpobject )
{
    if ( selectedObjectPosition == -1 ) {
        if ( m_activePage->numSelected() == 1 ) { // execute this if user selected is one object.
            QPtrList<KPObject> _list = objectList();
            _list.setAutoDelete( false );

            if ( _kpobject->isSelected() ) {
                selectedObjectPosition = objectList().find( _kpobject );
                _list.take( selectedObjectPosition );
                _list.append( _kpobject );
            }

            m_activePage->setObjectList( _list );
        }
        else
            selectedObjectPosition = -1;
    }
}

void KPrCanvas::lowerObject()
{
    if( objectList().count()==0)
        return;
    KPObject *kpobject = objectList().last();
    QPtrList<KPObject> _list = objectList();
    _list.setAutoDelete( false );

    if ( kpobject->isSelected() ) {
        _list.take( _list.count() - 1 );
        if(objectList().find( kpobject )!=-1)
            _list.insert( selectedObjectPosition, kpobject );
    }
    m_activePage->setObjectList( _list );
}

void KPrCanvas::playSound( const QString &soundFileName )
{
    if(soundPlayer)
        delete soundPlayer;
    soundPlayer = new KPresenterSoundPlayer( soundFileName );
    soundPlayer->play();
}

void KPrCanvas::stopSound()
{
    if ( soundPlayer ) {
        soundPlayer->stop();
        delete soundPlayer;
        soundPlayer = 0;
    }
}

void KPrCanvas::setXimPosition( int x, int y, int w, int h, QFont *f )
{
    QWidget::setMicroFocusHint( x - diffx(), y - diffy(), w, h, true, f );
}

void KPrCanvas::createEditing( KPTextObject *textObj )
{
    if( m_currentTextObjectView)
    {
        m_currentTextObjectView->terminate();
        delete m_currentTextObjectView;
        m_currentTextObjectView = 0L;
        editNum = 0L;
    }
    m_currentTextObjectView=textObj->createKPTextView( this );
    // ## we should really replace editNum with a pointer
    editNum=textObj;
}

void KPrCanvas::terminateEditing( KPTextObject *textObj )
{
    if ( m_currentTextObjectView && m_currentTextObjectView->kpTextObject() == textObj )
    {
        m_currentTextObjectView->terminate();
        delete m_currentTextObjectView;
        m_currentTextObjectView = 0L;
        editNum = 0L;
    }
}

void KPrCanvas::drawCubicBezierCurve( int _dx, int _dy )
{
    QPoint oldEndPoint = m_dragEndPoint;
    m_dragEndPoint = QPoint( _dx, _dy );

    unsigned int pointCount = m_pointArray.count();

    QPainter p( this );

    if ( !m_drawLineWithCubicBezierCurve ) {
        QPen _pen = QPen( Qt::black, 1, Qt::DashLine );
        p.setPen( _pen );
        p.setBrush( Qt::NoBrush );
        p.setRasterOp( Qt::NotROP );

        p.save();
        double _angle = KoPoint::getAngle( oldEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_view->zoomHandler()->unzoomPoint( oldEndPoint ), _pen.color(),
                    _pen.width(), _angle,m_view->zoomHandler() ); // erase old figure
        p.restore();

        p.drawLine( m_dragStartPoint, oldEndPoint ); // erase old line

        int p_x = m_dragStartPoint.x() * 2 - oldEndPoint.x();
        int p_y = m_dragStartPoint.y() * 2 - oldEndPoint.y();
        m_dragSymmetricEndPoint = QPoint( p_x, p_y );

        p.save();
        _angle = KoPoint::getAngle( m_dragSymmetricEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_view->zoomHandler()->unzoomPoint( m_dragSymmetricEndPoint ),
                    _pen.color(), _pen.width(), _angle,m_view->zoomHandler() );  // erase old figure
        p.restore();

        p.drawLine( m_dragStartPoint, m_dragSymmetricEndPoint );  // erase old line


        p.save();
        _angle = KoPoint::getAngle( m_dragEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_view->zoomHandler()->unzoomPoint( m_dragEndPoint ),
                    _pen.color(), _pen.width(), _angle,m_view->zoomHandler() ); // draw new figure
        p.restore();

        p.drawLine( m_dragStartPoint, m_dragEndPoint );  // draw new line

        p_x = m_dragStartPoint.x() * 2 - m_dragEndPoint.x();
        p_y = m_dragStartPoint.y() * 2 - m_dragEndPoint.y();
        m_dragSymmetricEndPoint = QPoint( p_x, p_y );

        p.save();
        _angle = KoPoint::getAngle( m_dragSymmetricEndPoint, m_dragStartPoint );
        drawFigure( L_SQUARE, &p, m_view->zoomHandler()->unzoomPoint( m_dragSymmetricEndPoint ),
                    _pen.color(), _pen.width(), _angle,m_view->zoomHandler() ); // draw new figure
        p.restore();

        p.drawLine( m_dragStartPoint, m_dragSymmetricEndPoint );  // draw new line
    }
    else if ( m_drawLineWithCubicBezierCurve ) {
        p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
        p.setBrush( Qt::NoBrush );
        p.setRasterOp( Qt::NotROP );

        QPoint startPoint( m_view->zoomHandler()->zoomItX( m_pointArray.at( m_indexPointArray - 1 ).x() ),
                           m_view->zoomHandler()->zoomItY( m_pointArray.at( m_indexPointArray - 1 ).y() ) );

        p.drawLine( startPoint, oldEndPoint );  // erase old line

        p.drawLine( startPoint, m_dragEndPoint );  // draw new line
    }

    if ( !m_drawLineWithCubicBezierCurve && ( ( pointCount % 2 ) == 0 ) ) {
        p.save();

        p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
        p.setBrush( Qt::NoBrush );
        p.setRasterOp( Qt::NotROP );
        // erase old cubic bezier curve
        p.drawCubicBezier( m_oldCubicBezierPointArray.zoomPointArray( m_view->zoomHandler() ) );

        double _firstX = m_pointArray.at( m_indexPointArray - 2 ).x();
        double _firstY = m_pointArray.at( m_indexPointArray - 2 ).y();

        double _fourthX = m_pointArray.at( m_indexPointArray - 1 ).x();
        double _fourthY = m_pointArray.at( m_indexPointArray - 1 ).y();

        double _midpointX = (_firstX + _fourthX ) / 2;
        double _midpointY = (_firstY + _fourthY ) / 2;
        double _diffX = _fourthX - _midpointX;
        double _diffY = _fourthY - _midpointY;

        double _secondX = m_view->zoomHandler()->unzoomItX( m_dragEndPoint.x() ) - _diffX;
        double _secondY = m_view->zoomHandler()->unzoomItY( m_dragEndPoint.y() ) - _diffY;
        m_CubicBezierSecondPoint = KoPoint( _secondX, _secondY );

        double _thirdX = m_view->zoomHandler()->unzoomItX( m_dragSymmetricEndPoint.x() ) - _diffX;
        double _thirdY = m_view->zoomHandler()->unzoomItY( m_dragSymmetricEndPoint.y() ) - _diffY;
        m_CubicBezierThirdPoint = KoPoint( _thirdX, _thirdY );

        if ( toolEditMode == INS_QUADRICBEZIERCURVE ) {
            _secondX = _thirdX;
            _secondY = _thirdY;
            m_CubicBezierSecondPoint = KoPoint( _secondX, _secondY );
        }

        KoPointArray points;
        points.putPoints( 0, 4, _firstX,_firstY, _secondX,_secondY, _thirdX,_thirdY, _fourthX,_fourthY );
        // draw new cubic bezier curve
        p.drawCubicBezier( points.zoomPointArray( m_view->zoomHandler() ) );

        m_oldCubicBezierPointArray = points;

        p.restore();
    }

    p.end();
}

void KPrCanvas::drawPolygon( const KoPoint &startPoint, const KoPoint &endPoint )
{
    bool checkConcavePolygon = m_view->getCheckConcavePolygon();
    int cornersValue = m_view->getCornersValue();
    int sharpnessValue = m_view->getSharpnessValue();

    QPainter p;
    p.begin( this );
    p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
    p.setRasterOp( Qt::NotROP );

    double angle = 2 * M_PI / cornersValue;
    double dx = QABS( startPoint.x () - endPoint.x () );
    double dy = QABS( startPoint.y () - endPoint.y () );
    double radius = ( dx > dy ? dx / 2.0 : dy / 2.0 );

    //xoff / yoff : coordinate of centre of the circle.
    double xoff = startPoint.x() + ( startPoint.x() < endPoint.x() ? radius : -radius );
    double yoff = startPoint.y() + ( startPoint.y() < endPoint.y() ? radius : -radius );

    KoPointArray points( checkConcavePolygon ? cornersValue * 2 : cornersValue );
    points.setPoint( 0, xoff, -radius + yoff );

    if ( checkConcavePolygon ) {
        angle = angle / 2.0;
        double a = angle;
        double r = radius - ( sharpnessValue / 100.0 * radius );
        for ( int i = 1; i < cornersValue * 2; ++i ) {
            double xp, yp;
            if ( i % 2 ) {
                xp =  r * sin( a );
                yp = -r * cos( a );
            }
            else {
                xp = radius * sin( a );
                yp = -radius * cos( a );
            }
            a += angle;
	    points.setPoint( i, xp + xoff, yp + yoff );
        }
    }
    else {
        double a = angle;
        for ( int i = 1; i < cornersValue; ++i ) {
            double xp = radius * sin( a );
            double yp = -radius * cos( a );
            a += angle;
            points.setPoint( i, xp + xoff, yp + yoff );
        }
    }
    p.drawPolygon( points.zoomPointArray( m_view->zoomHandler() ) );
    p.end();

    m_pointArray = points;
}


bool KPrCanvas::oneObjectTextExist()
{
    bool state=m_activePage->oneObjectTextExist();
    if(state)
        return true;
    return m_view->kPresenterDoc()->stickyPage()->oneObjectTextExist();
}

KPrPage* KPrCanvas::activePage()
{
    return m_activePage;
}

void KPrCanvas::setActivePage( KPrPage* _active)
{
    Q_ASSERT(_active);
    //kdDebug(33001)<<"KPrCanvas::setActivePage( KPrPage* _active) :"<<_active<<endl;
    m_activePage=_active;
}

void KPrCanvas::slotSetActivePage( KPrPage* _active)
{
    Q_ASSERT(_active);
    //kdDebug(33001)<<"void KPrCanvas::slotSetActivePage( KPrPage* _active) :"<<_active<<endl;
    m_activePage=_active;
}

//return true if object is a header/footer hidden
bool KPrCanvas::objectIsAHeaderFooterHidden(KPObject *obj)
{
    if((obj==m_view->kPresenterDoc()->header() && !m_view->kPresenterDoc()->hasHeader())||(obj==m_view->kPresenterDoc()->footer() && !m_view->kPresenterDoc()->hasFooter()))
        return true;
    return false;
}

int KPrCanvas::numberOfObjectSelected()
{
    int nb=activePage()->numSelected();
    nb+=m_view->kPresenterDoc()->stickyPage()->numSelected();
    return nb;
}

KPObject *KPrCanvas::getSelectedObj()
{
    KPObject *obj=activePage()->getSelectedObj();
    if(obj)
        return obj;
    obj=m_view->kPresenterDoc()->stickyPage()->getSelectedObj();
    return obj;
}

int KPrCanvas::getPenBrushFlags()
{
    int flags=0;
    flags=activePage()->getPenBrushFlags(activePage()->objectList());
    flags=flags |m_view->kPresenterDoc()->stickyPage()->getPenBrushFlags(m_view->kPresenterDoc()->stickyPage()->objectList());
    if(flags==0)
      flags = StyleDia::SdAll;
    return flags;
}

void KPrCanvas::ungroupObjects()
{
    m_activePage->ungroupObjects();
    m_view->kPresenterDoc()->stickyPage()->ungroupObjects();
}

void KPrCanvas::groupObjects()
{
    m_activePage->groupObjects();
    m_view->kPresenterDoc()->stickyPage()->groupObjects();
}
