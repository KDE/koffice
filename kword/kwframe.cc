/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Reginald Stadlbauer <reggie@kde.org>

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

#include "kwdoc.h"
#include "kwview.h"
#include "kwviewmode.h"
#include "kwcanvas.h"
#include "kwcommand.h"
#include "kwframe.h"
#include "defs.h"
#include "kwtextframeset.h"
#include "kwtableframeset.h"
#include "kwanchor.h"
#include "resizehandles.h"
#include <kotextobject.h> // for customItemChar!
#include <qpopupmenu.h>

#include <kcursor.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <kdebug.h>
#include <kdeversion.h>
#if ! KDE_IS_VERSION(3,1,90)
#include <kdebugclasses.h>
#endif
#include <float.h>
#include "KWordFrameSetIface.h"
#include <dcopobject.h>
#include "qdrawutil.h"
#include "KWordTextFrameSetEditIface.h"
#include "KWordFormulaFrameSetIface.h"
#include "KWordFormulaFrameSetEditIface.h"
#include "KWordPictureFrameSetIface.h"
#include "KWordHorizontalLineFrameSetIface.h"

//#define DEBUG_DRAW


/******************************************************************/
/* Class: KWFrameList                                             */
/******************************************************************/

int KWFrameList::compareItems(QPtrCollection::Item a, QPtrCollection::Item b)
{
           int za = ((KWFrame *)a)->zOrder();
           int zb = ((KWFrame *)b)->zOrder();
           if (za == zb) return 0;
           if (za < zb) return -1;
           return 1;
}

/******************************************************************/
/* Class: KWFrame                                                 */
/******************************************************************/

KWFrame::KWFrame(KWFrame * frame)
{
    handles.setAutoDelete(true);
    m_selected = false;
    m_runAround = RA_NO;
    //kdDebug(32001) << "KWFrame::KWFrame this=" << this << " frame=" << frame << endl;
    copySettings( frame );
    m_minFrameHeight=0;
}

KWFrame::KWFrame(KWFrameSet *fs, double left, double top, double width, double height, RunAround _ra, double _gap )
    : KoRect( left, top, width, height ),
      // Initialize member vars here. This ensures they are all initialized, since it's
      // easier to compare this list with the member vars list (compiler ensures order).
      m_sheetSide( AnySide ),
      m_runAround( _ra ),
      m_frameBehavior( AutoExtendFrame ),
      m_newFrameBehavior( ( fs && fs->type() == FT_TEXT ) ? Reconnect : NoFollowup ),
      m_runAroundGap( _gap ),
      bleft( 0 ),
      bright( 0 ),
      btop( 0 ),
      bbottom( 0 ),
      m_minFrameHeight( 0 ),
      m_internalY( 0 ),
      m_zOrder( 0 ),
      m_bCopy( false ),
      m_selected( false ),
      m_drawFootNoteLine( false ),
      m_backgroundColor( (fs && (fs->type() == FT_PICTURE || fs->type() == FT_PART)) ? QBrush( QColor(), Qt::NoBrush) : QBrush( QColor() ) ), // valid brush with invalid color ( default )
      brd_left( QColor(), KoBorder::SOLID, 0 ),
      brd_right( QColor(), KoBorder::SOLID, 0 ),
      brd_top( QColor(), KoBorder::SOLID, 0 ),
      brd_bottom( QColor(), KoBorder::SOLID, 0 ),
      handles(),
      m_framesOnTop(),
      m_framesBelow(),
      m_frameSet( fs )
{
    //kdDebug(32001) << "KWFrame::KWFrame " << this << " left=" << left << " top=" << top << endl;
    handles.setAutoDelete(true);
}

KWFrame::~KWFrame()
{
    //kdDebug(32001) << "KWFrame::~KWFrame " << this << endl;
    if (m_selected)
        removeResizeHandles();
}

void KWFrame::setBackgroundColor( const QBrush &_color )
{
    m_backgroundColor = _color;
}


int KWFrame::pageNum() const
{
    Q_ASSERT( m_frameSet );
    if ( !m_frameSet ) {
        kdDebug() << k_funcinfo << this << " has no frameset!" << endl;
        return 0;
    }
    return pageNum( m_frameSet->kWordDocument() );
}

int KWFrame::pageNum( KWDocument* doc ) const
{
    if ( y() < 0.1 )
        return 0;
    // ### This kind of calculation will break the day we introduce sections
    // (with different page sizes and orientation).
    int page = static_cast<int>(y() / doc->ptPaperHeight());
    return page;
    // Circular dependency. KWDoc uses pageNum to calculate the number of pages!
    //return QMIN( page, doc->numPages()-1 );
}

MouseMeaning KWFrame::getMouseMeaning( const KoPoint & docPoint, MouseMeaning defaultMeaning )
{
    if ( !m_selected )
        return defaultMeaning;

    double hs = 6; // horizontal snap zone (in pt)
    double vs = 6; // vertical snap zone (in pt)
    if ( width() < 18 )
        hs = width() / 3; // frame is not wide enough -> leave some room to click inside it
    if ( height() < 18 )
        vs = height() / 3; // same thing if frame is not high enough
    // Note that this makes a big zone when zooming in .... one that doesn't match
    // the resize handles. Maybe we should calculate the area a bit differently
    // (on both sides of the line), and allow resizing without selecting.

    double mx = docPoint.x();
    double my = docPoint.y();

    // Corners
    if ( mx >= x() && my >= y() && mx <= x() + hs && my <= y() + vs )
        return MEANING_TOPLEFT;
    if ( mx >= x() && my >= y() + height() - vs && mx <= x() + hs && my <= y() + height() )
        return MEANING_BOTTOMLEFT;
    if ( mx >= x() + width() - hs && my >= y() && mx <= x() + width() && my <= y() + vs )
        return MEANING_TOPRIGHT;
    if ( mx >= x() + width() - hs && my >= y() + height() - vs && mx <= x() + width() && my <= y() + height() )
        return MEANING_BOTTOMRIGHT;

    // Middle of edges
    if ( mx >= x() && my >= y() + height() / 2 - vs/2 && mx <= x() + hs && my <= y() + height() / 2 + vs/2 )
        return MEANING_LEFT;
    if ( mx >= x() + width() / 2 - hs/2 && my >= y() && mx <= x() + width() / 2 + hs/2 && my <= y() + vs )
        return MEANING_TOP;
    if ( mx >= x() + width() / 2 - hs/2 && my >= y() + height() - vs && mx <= x() + width() / 2 + hs/2 &&
         my <= y() + height() )
        return MEANING_BOTTOM;
    if ( mx >= x() + width() - hs && my >= y() + height() / 2 - vs/2 && mx <= x() + width() &&
         my <= y() + height() / 2 + vs/2 )
        return MEANING_RIGHT;

    return defaultMeaning;
}

KWFrame *KWFrame::getCopy() {
    /* returns a deep copy of self */
    return new KWFrame(this);
}


void KWFrame::invalidateParentFrameset() {
	if (frameSet()->isFloating())
		frameSet()->anchorFrameset()->invalidate();
}

void KWFrame::copySettings(KWFrame *frm)
{
    setRect(frm->x(), frm->y(), frm->width(), frm->height());
    // Keep order identical as member var order (and init in ctor)
    setSheetSide(frm->sheetSide());
    setRunAround(frm->runAround());
    setRunAroundSide(frm->runAroundSide());
    setFrameBehavior(frm->frameBehavior());
    setNewFrameBehavior(frm->newFrameBehavior());
    setRunAroundGap(frm->runAroundGap());
    setBLeft(frm->bLeft());
    setBRight(frm->bRight());
    setBTop(frm->bTop());
    setBBottom(frm->bBottom());
    setMinFrameHeight(frm->minFrameHeight());
    m_internalY = 0; // internal Y is recalculated
    setZOrder(frm->zOrder());
    setCopy(frm->isCopy());
    setSelected( false );// don't copy this attribute [shouldn't be an attribute of KWFrame]
    m_drawFootNoteLine = false; // recalculated
    setBackgroundColor( frm->backgroundColor() );
    setLeftBorder(frm->leftBorder());
    setRightBorder(frm->rightBorder());
    setTopBorder(frm->topBorder());
    setBottomBorder(frm->bottomBorder());
    setFrameSet( frm->frameSet() );
}

// Insert all resize handles
void KWFrame::createResizeHandles() {
    removeResizeHandles();
    QPtrList <KWView> pages = frameSet()->kWordDocument()->getAllViews();
    for (int i=pages.count() -1; i >= 0; i--)
        createResizeHandlesForPage(pages.at(i)->getGUI()->canvasWidget());
}

// Insert 8 resize handles which will be drawn in param canvas
void KWFrame::createResizeHandlesForPage(KWCanvas *canvas) {
    removeResizeHandlesForPage(canvas);

    for (unsigned int i=0; i < 8; i++) {
        KWResizeHandle * h = new KWResizeHandle( canvas, (KWResizeHandle::Direction)i, this );
        handles.append( h );
    }
}

// remove all the resize handles which will be drawn in param canvas
void KWFrame::removeResizeHandlesForPage(KWCanvas *canvas) {
    for( unsigned int i=0; i < handles.count(); i++) {
        if(handles.at ( i )->getCanvas() == canvas) {
            handles.remove(i--);
        }
    }
}

// remove all resizeHandles
void KWFrame::removeResizeHandles() {
    //kdDebug(32001) << this << " KWFrame::removeResizeHandles " << handles.count() << " handles" << endl;
    handles.clear();
}

// move the resizehandles to current location of frame
void KWFrame::updateResizeHandles() {
    for (unsigned int i=0; i< handles.count(); i++) {
        handles.at(i)->updateGeometry();
    }
}

void KWFrame::repaintResizeHandles() {
    for (unsigned int i=0; i< handles.count(); i++) {
        handles.at(i)->repaint();
    }
}

void KWFrame::updateCursorType()
{
    for (unsigned int i=0; i< handles.count(); i++) {
        handles.at(i)->applyCursorType();
    }
}


void KWFrame::frameBordersChanged() {
	invalidateParentFrameset();
	if (isSelected()) {
		updateResizeHandles();
	}
}


void KWFrame::updateRulerHandles(){
    if(isSelected())
        updateResizeHandles();
    else
    {
        KWDocument *doc = frameSet()->kWordDocument();
        if(doc)
            doc->updateRulerFrameStartEnd();
    }

}

void KWFrame::setSelected( bool _selected )
{
    //kdDebug(32001) << this << " KWFrame::setSelected " << _selected << endl;
    bool s = m_selected;
    m_selected = _selected;
    if ( m_selected )
        createResizeHandles();
    else if ( s )
        removeResizeHandles();
}

QRect KWFrame::outerRect( KWViewMode* viewMode ) const
{
    KWDocument *doc = m_frameSet->kWordDocument();
    QRect outerRect( doc->zoomRect( *this ) );
    bool isFrameVisible = viewMode && viewMode->drawFrameBorders();
    if ( isFrameVisible && !m_frameSet->getGroupManager() ) {
        KWFrame* settingsFrame = m_frameSet->settingsFrame( this );
        outerRect.rLeft() -= KoBorder::zoomWidthX( settingsFrame->leftBorder().width(), doc, 1 );
        outerRect.rTop() -= KoBorder::zoomWidthY( settingsFrame->topBorder().width(), doc, 1 );
        outerRect.rRight() += KoBorder::zoomWidthX( settingsFrame->rightBorder().width(), doc, 1 );
        outerRect.rBottom() += KoBorder::zoomWidthY( settingsFrame->bottomBorder().width(), doc, 1 );
    }
    return outerRect;
}

KoRect KWFrame::outerKoRect() const
{
    KoRect outerRect = *this;
    KWDocument *doc = m_frameSet->kWordDocument();
    KWFrame* settingsFrame = m_frameSet->settingsFrame( this );
    outerRect.rLeft() -= KoBorder::zoomWidthX( settingsFrame->leftBorder().width(), doc, 1 ) / doc->zoomedResolutionX();
    outerRect.rTop() -= KoBorder::zoomWidthY( settingsFrame->topBorder().width(), doc, 1 ) / doc->zoomedResolutionY();
    outerRect.rRight() += KoBorder::zoomWidthX( settingsFrame->rightBorder().width(), doc, 1 ) / doc->zoomedResolutionX();
    outerRect.rBottom() += KoBorder::zoomWidthY( settingsFrame->bottomBorder().width(), doc, 1 ) / doc->zoomedResolutionY();
    return outerRect;
}

KoRect KWFrame::runAroundRect() const
{
    KoRect raRect = outerKoRect();
    raRect.rLeft() -= m_runAroundGap;
    raRect.rTop() -= m_runAroundGap;
    raRect.rRight() += m_runAroundGap;
    raRect.rBottom() += m_runAroundGap;
    return raRect;
}

void KWFrame::save( QDomElement &frameElem )
{
    // setAttribute( double ) uses a default precision of 6, and this seems
    // to be 6 digits, even like '123.123' !
    frameElem.setAttribute( "left", QString::number( left(), 'g', DBL_DIG ) );
    frameElem.setAttribute( "top", QString::number( top(), 'g', DBL_DIG ) );
    frameElem.setAttribute( "right", QString::number( right(), 'g', DBL_DIG ) );
    frameElem.setAttribute( "bottom", QString::number( bottom(), 'g', DBL_DIG ) );
    if ( minFrameHeight() > 0 )
        frameElem.setAttribute( "min-height", QString::number( minFrameHeight(), 'g', DBL_DIG ) );

    if(runAround()!=RA_NO)
    {
        frameElem.setAttribute( "runaround", static_cast<int>( runAround() ) );
        if (runAround() == RA_BOUNDINGRECT)
        {
            if (runAroundSide()==RA_LEFT)
                frameElem.setAttribute( "runaroundSide", "left" );
            else if (runAroundSide()==RA_RIGHT)
                frameElem.setAttribute( "runaroundSide", "right" );
            else
                frameElem.setAttribute( "runaroundSide", "biggest" );
        }
    }
    if(runAroundGap()!=0)
        frameElem.setAttribute( "runaroundGap", runAroundGap() );

    if(leftBorder().penWidth()!=0)
        frameElem.setAttribute( "lWidth", leftBorder().penWidth() );

    if(leftBorder().color.isValid())
    {
        frameElem.setAttribute( "lRed", leftBorder().color.red() );
        frameElem.setAttribute( "lGreen", leftBorder().color.green() );
        frameElem.setAttribute( "lBlue", leftBorder().color.blue() );
    }
    if(leftBorder().getStyle() != KoBorder::SOLID)
        frameElem.setAttribute( "lStyle", static_cast<int>( leftBorder().getStyle()) );

    if(rightBorder().penWidth()!=0)
        frameElem.setAttribute( "rWidth", rightBorder().penWidth() );

    if(rightBorder().color.isValid())
    {
        frameElem.setAttribute( "rRed", rightBorder().color.red() );
        frameElem.setAttribute( "rGreen", rightBorder().color.green() );
        frameElem.setAttribute( "rBlue", rightBorder().color.blue() );
    }
    if(rightBorder().getStyle() != KoBorder::SOLID)
        frameElem.setAttribute( "rStyle", static_cast<int>( rightBorder().getStyle() ) );

    if(topBorder().penWidth()!=0)
        frameElem.setAttribute( "tWidth", topBorder().penWidth() );

    if(topBorder().color.isValid())
    {
        frameElem.setAttribute( "tRed", topBorder().color.red() );
        frameElem.setAttribute( "tGreen", topBorder().color.green() );
        frameElem.setAttribute( "tBlue", topBorder().color.blue() );
    }
    if(topBorder().getStyle() != KoBorder::SOLID)
        frameElem.setAttribute( "tStyle", static_cast<int>( topBorder().getStyle() ) );

    if(bottomBorder().penWidth()!=0) {
        frameElem.setAttribute( "bWidth", bottomBorder().penWidth() );
    }
    if(bottomBorder().color.isValid()) {
        frameElem.setAttribute( "bRed", bottomBorder().color.red() );
        frameElem.setAttribute( "bGreen", bottomBorder().color.green() );
        frameElem.setAttribute( "bBlue", bottomBorder().color.blue() );
    }
    if(bottomBorder().getStyle() != KoBorder::SOLID)
        frameElem.setAttribute( "bStyle", static_cast<int>( bottomBorder().getStyle() ) );

    if(backgroundColor().color().isValid())
    {
        frameElem.setAttribute( "bkRed", backgroundColor().color().red() );
        frameElem.setAttribute( "bkGreen", backgroundColor().color().green() );
        frameElem.setAttribute( "bkBlue", backgroundColor().color().blue() );
        frameElem.setAttribute( "bkStyle", (int)backgroundColor().style ());
    }
    if(bLeft() != 0)
        frameElem.setAttribute( "bleftpt", bLeft() );

    if(bRight()!=0)
        frameElem.setAttribute( "brightpt", bRight() );

    if(bTop()!=0)
        frameElem.setAttribute( "btoppt", bTop() );

    if(bBottom()!=0)
        frameElem.setAttribute( "bbottompt", bBottom() );

    if(frameBehavior()!=AutoCreateNewFrame)
        frameElem.setAttribute( "autoCreateNewFrame", static_cast<int>( frameBehavior()) );

    //if(newFrameBehavior()!=Reconnect) // always save this one, since the default value depends on the type of frame, etc.
    frameElem.setAttribute( "newFrameBehavior", static_cast<int>( newFrameBehavior()) );

    //same reason
    frameElem.setAttribute( "copy", static_cast<int>( m_bCopy ) );

    if(sheetSide()!= AnySide)
        frameElem.setAttribute( "sheetSide", static_cast<int>( sheetSide()) );

    frameElem.setAttribute( "z-index", zOrder() );
}

void KWFrame::load( QDomElement &frameElem, KWFrameSet* frameSet, int syntaxVersion )
{
    m_minFrameHeight = KWDocument::getAttribute( frameElem, "min-height", 0.0 );
    m_runAround = static_cast<RunAround>( KWDocument::getAttribute( frameElem, "runaround", RA_NO ) );
    QString str = frameElem.attribute( "runaroundSide" );
    if ( str == "left" )
        setRunAroundSide( RA_LEFT );
    else if ( str == "right" )
        setRunAroundSide( RA_RIGHT );
    // default case: RA_BIGGEST, since it's 0

    m_runAroundGap = ( frameElem.hasAttribute( "runaroundGap" ) )
                          ? frameElem.attribute( "runaroundGap" ).toDouble()
                          : frameElem.attribute( "runaGapPT" ).toDouble();
    m_sheetSide = static_cast<SheetSide>( KWDocument::getAttribute( frameElem, "sheetSide", AnySide ) );
    m_frameBehavior = static_cast<FrameBehavior>( KWDocument::getAttribute( frameElem, "autoCreateNewFrame", AutoCreateNewFrame ) );
    // Old documents had no "NewFrameBehavior" for footers/headers -> default to Copy.
    NewFrameBehavior defaultValue = frameSet->isHeaderOrFooter() ? Copy : Reconnect;
    // for old document we used the British spelling (newFrameBehaviour), so this is for backwards compatibility.
    defaultValue = static_cast<NewFrameBehavior>( KWDocument::getAttribute( frameElem, "newFrameBehaviour", defaultValue ) );
    m_newFrameBehavior = static_cast<NewFrameBehavior>( KWDocument::getAttribute( frameElem, "newFrameBehavior", defaultValue ) );
    if ( frameSet->isFootEndNote() ) // note that isFootNote/isEndNote are not possible yet
        m_newFrameBehavior = NoFollowup;

    KoBorder l, r, t, b;
    l.setPenWidth( KWDocument::getAttribute( frameElem, "lWidth", 0.0 ));
    r.setPenWidth(KWDocument::getAttribute( frameElem, "rWidth", 0.0 ));
    t.setPenWidth(KWDocument::getAttribute( frameElem, "tWidth", 0.0 ));
    b.setPenWidth(KWDocument::getAttribute( frameElem, "bWidth", 0.0 ));
    if ( frameElem.hasAttribute("lRed") )
        l.color.setRgb(
            KWDocument::getAttribute( frameElem, "lRed", 0 ),
            KWDocument::getAttribute( frameElem, "lGreen", 0 ),
            KWDocument::getAttribute( frameElem, "lBlue", 0 ) );
    if ( frameElem.hasAttribute("rRed") )
        r.color.setRgb(
            KWDocument::getAttribute( frameElem, "rRed", 0 ),
            KWDocument::getAttribute( frameElem, "rGreen", 0 ),
            KWDocument::getAttribute( frameElem, "rBlue", 0 ) );
    if ( frameElem.hasAttribute("tRed") )
        t.color.setRgb(
            KWDocument::getAttribute( frameElem, "tRed", 0 ),
            KWDocument::getAttribute( frameElem, "tGreen", 0 ),
            KWDocument::getAttribute( frameElem, "tBlue", 0 ) );
    if ( frameElem.hasAttribute("bRed") )
        b.color.setRgb(
            KWDocument::getAttribute( frameElem, "bRed", 0 ),
            KWDocument::getAttribute( frameElem, "bGreen", 0 ),
            KWDocument::getAttribute( frameElem, "bBlue", 0 ) );
    l.setStyle(static_cast<KoBorder::BorderStyle>( KWDocument::getAttribute( frameElem, "lStyle", KoBorder::SOLID ) ));
    r.setStyle(static_cast<KoBorder::BorderStyle>( KWDocument::getAttribute( frameElem, "rStyle", KoBorder::SOLID ) ));
    t.setStyle(static_cast<KoBorder::BorderStyle>( KWDocument::getAttribute( frameElem, "tStyle", KoBorder::SOLID ) ));
    b.setStyle( static_cast<KoBorder::BorderStyle>( KWDocument::getAttribute( frameElem, "bStyle", KoBorder::SOLID ) ));
    QColor c;
    if ( frameElem.hasAttribute("bkRed") )
        c.setRgb(
            KWDocument::getAttribute( frameElem, "bkRed", 0 ),
            KWDocument::getAttribute( frameElem, "bkGreen", 0 ),
            KWDocument::getAttribute( frameElem, "bkBlue", 0 ) );

    if ( syntaxVersion < 2 ) // Activate old "white border == no border" conversion
    {
        if(c==l.color && l.penWidth()==1 && l.getStyle()==0 )
            l.setPenWidth(0);
        if(c==r.color  && r.penWidth()==1 && r.getStyle()==0)
            r.setPenWidth(0);
        if(c==t.color && t.penWidth()==1 && t.getStyle()==0 )
            t.setPenWidth(0);
        if(c==b.color && b.penWidth()==1 && b.getStyle()==0 )
            b.setPenWidth(0);
    }
    brd_left = l;
    brd_right = r;
    brd_top = t;
    brd_bottom = b;
    m_backgroundColor = QBrush( c );


    if( frameElem.hasAttribute("bkStyle"))
        m_backgroundColor.setStyle (static_cast<Qt::BrushStyle>(KWDocument::getAttribute( frameElem, "bkStyle", Qt::SolidPattern )));

    bleft = frameElem.attribute( "bleftpt" ).toDouble();
    bright = frameElem.attribute( "brightpt" ).toDouble();
    btop = frameElem.attribute( "btoppt" ).toDouble();
    bbottom = frameElem.attribute( "bbottompt" ).toDouble();
    m_bCopy = KWDocument::getAttribute( frameElem, "copy", frameSet->isHeaderOrFooter() /* default to true for h/f */ );
    m_zOrder = frameElem.attribute( "z-index" ).toInt();
}


bool KWFrame::frameAtPos( const QPoint& point, bool borderOfFrameOnly) {
    // Forwarded to KWFrameSet to make it virtual
    return frameSet()->isFrameAtPos( this, point, borderOfFrameOnly );
}

KoRect KWFrame::innerRect() const
{
    KoRect inner( this->normalize());
    inner.moveBy( bLeft(), bTop());
    inner.setWidth( innerWidth() );
    inner.setHeight( innerHeight() );
    return inner;
}

double KWFrame::innerWidth() const
{
    return QMAX( 0.0, width() - bLeft() - bRight() );
}

double KWFrame::innerHeight() const
{
    return QMAX( 0.0, height() - bTop() - bBottom() );
}

void KWFrame::setFrameMargins( double _left, double _top, double _right, double _bottom)
{
    bleft = _left;
    btop = _top;
    bright = _right;
    bbottom = _bottom;
}

/******************************************************************/
/* Class: KWFrameSet                                              */
/******************************************************************/
KWFrameSet::KWFrameSet( KWDocument *doc )
    : m_doc( doc ), frames(), m_framesInPage(), m_firstPage( 0 ), m_emptyList(),
      m_info( FI_BODY ),
      grpMgr( 0L ), m_removeableHeader( false ), m_visible( true ),
      m_protectSize( false ),
      m_anchorTextFs( 0L ), m_dcop( 0L )
{
    // Send our "repaintChanged" signals to the document.
    connect( this, SIGNAL( repaintChanged( KWFrameSet * ) ),
             doc, SLOT( slotRepaintChanged( KWFrameSet * ) ) );
    frames.setAutoDelete( true );
    m_framesInPage.setAutoDelete( true ); // autodelete the lists in the array (not the frames;)
}

KWordFrameSetIface* KWFrameSet::dcopObject()
 {
    if ( !m_dcop )
	m_dcop = new KWordFrameSetIface( this );

    return m_dcop;
}


KWFrameSet::~KWFrameSet()
{
    delete m_dcop;
}

void KWFrameSet::addFrame( KWFrame *_frame, bool recalc )
{
    if ( frames.findRef( _frame ) != -1 )
        return;

    //kdDebug(32001) << k_funcinfo << getName() << " adding frame" <<  _frame << " recalc=" << recalc << endl;
    frames.append( _frame );
    _frame->setFrameSet(this);
    if(recalc)
        updateFrames();
}

void KWFrameSet::delFrame( unsigned int _num, bool remove, bool recalc )
{
    //kdDebug(32001) << k_funcinfo << getName() << " deleting frame" <<  _num << " remove=" << remove << " recalc=" << recalc << endl;
    KWFrame *frm = frames.at( _num );
    Q_ASSERT( frm );
    if ( !remove )
    {
        frames.take( _num );
        if (frm->isSelected()) // get rid of the resize handles
            frm->setSelected(false);
        frm->setFrameSet(0L);
    }
    else {
        // ###### should something similar be done when just removing a frame from the list?
        frameDeleted( frm, recalc ); // inform kwtableframeset if necessary
        frames.remove( _num );
        //kdDebug(32001) << k_funcinfo << frm << " deleted. Now I have " << frames.count() << " frames" << endl;
    }

    if ( recalc )
        updateFrames();
}

void KWFrameSet::delFrame( KWFrame *frm, bool remove, bool recalc )
{
    //kdDebug(32001) << "KWFrameSet::delFrame " << frm << " remove=" << remove << endl;
    int _num = frames.findRef( frm );
    Q_ASSERT( _num != -1 );
    if ( _num == -1 )
        return;

    delFrame( _num, remove, recalc );
}

void KWFrameSet::deleteAllFrames()
{
    if ( !frames.isEmpty() )
    {
        frames.clear();
        updateFrames();
    }
}

void KWFrameSet::deleteAllCopies()
{
    if ( frames.count() > 1 )
    {
        KWFrame * firstFrame = frames.first()->getCopy();
        frames.clear();
        frames.append( firstFrame );
        updateFrames();
    }
}

void KWFrameSet::createEmptyRegion( const QRect & crect, QRegion & emptyRegion, KWViewMode *viewMode )
{
    int paperHeight = m_doc->paperHeight();
    //kdDebug(32001) << "KWFrameSet::createEmptyRegion " << getName() << endl;
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
    {
        QRect outerRect( viewMode->normalToView( frameIt.current()->outerRect(viewMode) ) );
        //kdDebug(32001) << "KWFrameSet::createEmptyRegion outerRect=" << outerRect << " crect=" << crect << endl;
        outerRect &= crect; // This is important, to avoid calling subtract with a Y difference > 65536
        if ( !outerRect.isEmpty() )
        {
            emptyRegion = emptyRegion.subtract( outerRect );
            //kdDebug(32001) << "KWFrameSet::createEmptyRegion emptyRegion now: " << endl; DEBUGREGION( emptyRegion );
        }
        if ( crect.bottom() + paperHeight < outerRect.top() )
            return; // Ok, we're far below the crect, abort.
    }
}

void KWFrameSet::drawMargins( KWFrame *frame, QPainter *p, const QRect &crect, const QColorGroup &, KWViewMode *viewMode )
{
    QRect outerRect( viewMode->normalToView( frame->outerRect(viewMode) ) );
    //kdDebug(32001) << "KWFrameSet::drawMargins frame: " << frameFromPtr( frame )
    //               << " outerRect: " << outerRect
    //               << " crect: " << crect << endl;

    if ( !crect.intersects( outerRect ) )
    {
#ifdef DEBUG_DRAW
        kdDebug(32001) << "KWFrameSet::drawMargins no intersection with " << crect << endl;
#endif
        return;
    }
    QRect frameRect( viewMode->normalToView( m_doc->zoomRect( *frame ) ) );
    p->save();
    QBrush bgBrush( frame->backgroundColor() );
    bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), p ) );
    p->setBrush( bgBrush );
    int leftMargin = m_doc->zoomItX(frame->bLeft());
    int topMargin = m_doc->zoomItY(frame->bTop());
    int rightMargin = m_doc->zoomItX(frame->bRight());
    int bottomMargin = m_doc->zoomItY(frame->bBottom());
    //kdDebug(32001) << "KWFrameSet::drawMargins leftMargin=" << leftMargin << " topMargin=" << topMargin << " rightMargin=" << rightMargin << " bottomMargin=" << bottomMargin << endl;

    if ( topMargin != 0 )
    {
        QRect r( frameRect.left(), frameRect.top(), frameRect.width(), topMargin );
        p->fillRect( r, bgBrush );
    }
    if ( leftMargin != 0 )
    {
        QRect r( frameRect.left(), frameRect.top(), leftMargin, frameRect.height() );
        p->fillRect( r, bgBrush );
    }
    if ( rightMargin != 0 )
    {
        QRect r( frameRect.right()-rightMargin, frameRect.top(), rightMargin, frameRect.height() );
        p->fillRect( r, bgBrush );
    }
    if ( frame->bBottom() != 0 )
    {
        QRect r( frameRect.left(), frameRect.bottom()-bottomMargin, frameRect.width(), bottomMargin );
        p->fillRect( r, bgBrush );
    }
    p->restore();

}


void KWFrameSet::drawFrameBorder( QPainter *painter, KWFrame *frame, KWFrame *settingsFrame, const QRect &crect, KWViewMode *viewMode )
{
    QRect outerRect( viewMode->normalToView( frame->outerRect( viewMode ) ) );
    //kdDebug(32001) << "KWFrameSet::drawFrameBorder frame: " << frameFromPtr( frame )
    //               << " outerRect: " << outerRect << endl;

    if ( !crect.intersects( outerRect ) )
    {
        //kdDebug(32001) << "KWFrameSet::drawFrameBorder no intersection with " << crect << endl;
        return;
    }

    QRect frameRect( viewMode->normalToView( m_doc->zoomRect(  *frame ) ) );

    painter->save();
    QBrush bgBrush( settingsFrame->backgroundColor() );
    //bool defaultColor = !bgBrush.color().isValid();
    bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), painter ) );
    painter->setBrush( bgBrush );

    // Draw default borders using view settings...
    QPen viewSetting( QApplication::palette().color( QPalette::Active, QColorGroup::Mid ) );
    int minBorder = 1;
    // ...except when printing, or embedded doc, or disabled.
    if ( !viewMode || !viewMode->drawFrameBorders() )
    {
        viewSetting = NoPen;
        minBorder = 0;
    }

    // Draw borders either as the user defined them, or using the view settings.
    // Borders should be drawn _outside_ of the frame area
    // otherwise the frames will erase the border when painting themselves.

    KoBorder::drawBorders( *painter, m_doc, frameRect,
                           settingsFrame->leftBorder(), settingsFrame->rightBorder(),
                           settingsFrame->topBorder(), settingsFrame->bottomBorder(),
                           minBorder, viewSetting );
    painter->restore();
}

void KWFrameSet::setFloating()
{
    // Find main text frame
    QPtrListIterator<KWFrameSet> fit = m_doc->framesetsIterator();
    for ( ; fit.current() ; ++fit )
    {
        KWTextFrameSet * frameSet = dynamic_cast<KWTextFrameSet *>( fit.current() );
        if ( !frameSet || frameSet->frameSetInfo() != FI_BODY )
            continue;

        KoTextParag* parag = 0L;
        int index = 0;
        KoPoint dPoint( frames.first()->topLeft() );
        kdDebug(32001) << "KWFrameSet::setFloating looking for pos at " << dPoint.x() << " " << dPoint.y() << endl;
        frameSet->findPosition( dPoint, parag, index );
        // Create anchor. TODO: refcount the anchors!
        setAnchored( frameSet, parag->paragId(), index );
        frameSet->layout();
        frames.first()->updateResizeHandles();
        m_doc->frameChanged( frames.first() );
        return;
    }
}

void KWFrameSet::setAnchored( KWTextFrameSet* textfs, int paragId, int index, bool placeHolderExists /* = false */, bool repaint )
{
    Q_ASSERT( textfs );
    kdDebug(32001) << "KWFrameSet::setAnchored " << textfs << " " << paragId << " " << index << " " << placeHolderExists << endl;
    if ( isFloating() )
        deleteAnchors();
    m_anchorTextFs = textfs;
    KWTextParag * parag = static_cast<KWTextParag *>( textfs->textDocument()->paragAt( paragId ) );
    Q_ASSERT( parag );
    if ( parag )
        createAnchors( parag, index, placeHolderExists, repaint );

    m_doc->updateAllFrames(); // We just became floating, so we need to be removed from "frames on top/below".
    // TODO pass page number - hmm, we could have several frames in theory
}

void KWFrameSet::setAnchored( KWTextFrameSet* textfs )
{
    m_anchorTextFs = textfs;
    m_doc->updateAllFrames(); // We just became floating, so we need to be removed from "frames on top/below".
    // TODO pass page number - hmm, we could have several frames in theory
}

// Find where our anchor is ( if we are anchored ).
// We can't store a pointers to anchors, because over time we might change anchors
// (Especially, undo/redo of insert/delete can reuse an old anchor and forget a newer one etc.)
KWAnchor * KWFrameSet::findAnchor( int frameNum )
{
    Q_ASSERT( m_anchorTextFs );
    // Yes, a linear search, but only among all customitems of the correct textdoc,
    // whose number is assumed to be quite small.
    QPtrListIterator<KoTextCustomItem> cit( m_anchorTextFs->textDocument()->allCustomItems() );
    for ( ; cit.current() ; ++cit )
    {
        KWAnchor * anchor = dynamic_cast<KWAnchor *>( cit.current() );
        if ( anchor && !anchor->isDeleted()
             && anchor->frameSet() == this && anchor->frameNum() == frameNum )
                return anchor;
    }
    kdWarning() << "KWFrameSet::findAnchor anchor not found (frameset='" << getName()
                << "' frameNum=" << frameNum << ")" << endl;
    return 0L;
}

void KWFrameSet::setFixed()
{
    kdDebug(32001) << "KWFrameSet::setFixed" << endl;
    if ( isFloating() )
        deleteAnchors();
    m_anchorTextFs = 0L;
    // make sure the frames are on top
    // (their z-order didn't matter when they were inline)
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
        frameIt.current()->setZOrder( m_doc->maxZOrder( frameIt.current()->pageNum(m_doc) ) + 1 );
}

KWAnchor * KWFrameSet::createAnchor( KoTextDocument *txt, int frameNum )
{
    KWAnchor * anchor = new KWAnchor( txt, this, frameNum );
    return anchor;
}

void KWFrameSet::createAnchors( KWTextParag * parag, int index, bool placeHolderExists /*= false */ /*only used when loading*/,
                                bool repaint )
{
    kdDebug(32001) << "KWFrameSet::createAnchors" << endl;
    Q_ASSERT( m_anchorTextFs );
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt, ++index )
    {
        //if ( ! frameIt.current()->anchor() )
        {
            // Anchor this frame, after the previous one
            KWAnchor * anchor = createAnchor( m_anchorTextFs->textDocument(), frameFromPtr( frameIt.current() ) );
            if ( !placeHolderExists )
                parag->insert( index, KoTextObject::customItemChar() );
            parag->setCustomItem( index, anchor, 0 );
        }
    }
    parag->setChanged( true );
    if ( repaint )
        emit repaintChanged( m_anchorTextFs );
}

void KWFrameSet::deleteAnchor( KWAnchor * anchor )
{
    // Simple deletion, no undo/redo
    KoTextCursor c( m_anchorTextFs->textDocument() );
    c.setParag( anchor->paragraph() );
    c.setIndex( anchor->index() );
    anchor->setDeleted( true ); // this sets m_anchorTextFs to 0L

    static_cast<KWTextParag*>(c.parag())->removeCustomItem(c.index());
    c.remove(); // This deletes the character where the anchor was
    // We don't delete the anchor since it might be in a customitemmap in a text-insert command
    // TODO: refcount the anchors
    c.parag()->setChanged( true );
}

void KWFrameSet::deleteAnchors()
{
    kdDebug(32002) << "KWFrameSet::deleteAnchors" << endl;
    KWTextFrameSet * textfs = m_anchorTextFs;
    Q_ASSERT( textfs );
    if ( !textfs )
        return;
    //QPtrListIterator<KWFrame> frameIt = frameIterator();
    int frameNum = 0;
    // At the moment there's only one anchor per frameset
    // With tables the loop below will be wrong anyway...
    //for ( ; frameIt.current(); ++frameIt, ++frameNum )
    {
/*        if ( frameIt.current()->anchor() )
            deleteAnchor( frameIt.current()->anchor() );
        frameIt.current()->setAnchor( 0L );
*/
        KWAnchor * anchor = findAnchor( frameNum );
        deleteAnchor( anchor );
    }
    emit repaintChanged( textfs );
}

void KWFrameSet::moveFloatingFrame( int frameNum, const KoPoint &position )
{
    KWFrame * frame = frames.at( frameNum );
    Q_ASSERT( frame );
    if ( !frame ) return;

    KoPoint pos( position );
    // position includes the border, we need to adjust accordingly
    pos.rx() += frame->leftBorder().width();
    pos.ry() += frame->topBorder().width();
    if ( frame->topLeft() != pos )
    {
        kdDebug(32002) << "KWFrameSet::moveFloatingFrame " << pos.x() << "," << pos.y() << endl;
        int oldPageNum = frame->pageNum();
        frame->moveTopLeft( pos );

        updateFrames();
        kWordDocument()->updateFramesOnTopOrBelow( frame->pageNum() );
        if ( oldPageNum != frame->pageNum() )
            kWordDocument()->updateFramesOnTopOrBelow( oldPageNum );

        if ( frame->isSelected() )
            frame->updateResizeHandles();
    }
    invalidate();
}

KoRect KWFrameSet::floatingFrameRect( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    Q_ASSERT( frame );
    Q_ASSERT( isFloating() );

    KWAnchor* anchor = findAnchor( frameNum );
    Q_ASSERT( anchor );
    QRect paragRect = anchor->paragraph()->rect();
    int x = anchor->x() + paragRect.x(); // in LU
    int y = anchor->y() + paragRect.y(); // in LU

    KoPoint topLeft( m_doc->layoutUnitToPixelX( x ), m_doc->layoutUnitToPixelY( y ) );
    return KoRect( topLeft, frame->outerKoRect().size() );
}

KoSize KWFrameSet::floatingFrameSize( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    Q_ASSERT( frame );
    return frame->outerKoRect().size();
}

KCommand * KWFrameSet::anchoredObjectCreateCommand( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    Q_ASSERT( frame );
    return new KWCreateFrameCommand( QString::null, frame );
}

KCommand * KWFrameSet::anchoredObjectDeleteCommand( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    Q_ASSERT( frame );
    return new KWDeleteFrameCommand( QString::null, frame );
}

KWFrame * KWFrameSet::frameByBorder( const QPoint & nPoint )
{
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt ) {
        if(frameIt.current()->frameAtPos(nPoint, true))
            return frameIt.current();
    }
    return 0L;
}

KWFrame * KWFrameSet::frameAtPos( double _x, double _y )
{
    KoPoint docPoint( _x, _y );
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
        if ( frameIt.current()->contains( docPoint ) )
            return frameIt.current();
    return 0L;
}

KWFrame *KWFrameSet::frame( unsigned int _num )
{
    return frames.at( _num );
}

int KWFrameSet::frameFromPtr( KWFrame *frame )
{
    return frames.findRef( frame );
}

KWFrame * KWFrameSet::settingsFrame( const KWFrame* frame )
{
    QPtrListIterator<KWFrame> frameIt( frame->frameSet()->frameIterator() );
    if ( !frame->isCopy() )
        return const_cast<KWFrame *>( frame );
    KWFrame* lastRealFrame=0L;
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame *curFrame = frameIt.current();
        if( curFrame == frame )
            return lastRealFrame ? lastRealFrame : const_cast<KWFrame *>( frame );
        if ( !lastRealFrame || !curFrame->isCopy() )
            lastRealFrame = curFrame;
    }
    return const_cast<KWFrame *>( frame ); //fallback, should never happen
}

void KWFrameSet::updateFrames( int flags )
{
    if ( frames.isEmpty() )
        return; // No frames. This happens when the frameset is deleted (still exists for undo/redo)

    // Not visible ? Don't bother then.
    if ( !isVisible() )
        return;

    //kdDebug(32001) << "KWFrameSet::updateFrames " << this << " " << getName() << endl;

    if ( flags & UpdateFramesInPage ) {
        // For each of our frames, clear old list of frames on top, and grab min/max page nums
        m_firstPage = frames.first()->pageNum(); // we know frames is not empty here
        int lastPage = m_firstPage;
        QPtrListIterator<KWFrame> fIt( frameIterator() );
        for ( ; fIt.current(); ++fIt ) {
            int pg = fIt.current()->pageNum();
            m_firstPage = QMIN( m_firstPage, pg );
            lastPage = QMAX( lastPage, pg );
        }
        //kdDebug(32001) << "firstPage=" << m_firstPage << " lastPage=" << lastPage << endl;

        // Prepare the m_framesInPage structure
        int oldSize = m_framesInPage.size();
        m_framesInPage.resize( lastPage - m_firstPage + 1 );
        // Clear the old elements
        int oldElements = QMIN( oldSize, (int)m_framesInPage.size() );
        for ( int i = 0 ; i < oldElements ; ++i )
            m_framesInPage[i]->clear();
        // Initialize the new elements.
        for ( int i = oldElements ; i < (int)m_framesInPage.size() ; ++i )
            m_framesInPage.insert( i, new QPtrList<KWFrame>() );

        // Iterate over frames again, to fill the m_framesInPage array
        fIt.toFirst();
        for ( ; fIt.current(); ++fIt ) {
            int pg = fIt.current()->pageNum();
            Q_ASSERT( pg <= lastPage );
            m_framesInPage[pg - m_firstPage]->append( fIt.current() );
        }
    }

    if ( isFloating() )
    {
        //kdDebug(32001) << "KWFrameSet::updateFrames " << getName() << " is floating" << endl;
        QPtrListIterator<KWFrame> frameIt = frameIterator();
        int frameNum = 0;
        // At the moment there's only one anchor per frameset
        //for ( ; frameIt.current(); ++frameIt, ++frameNum )
        {
            KWAnchor * anchor = findAnchor( frameNum );
            //kdDebug(32001) << "KWFrameSet::updateFrames anchor=" << anchor << endl;
            if ( anchor )
                anchor->resize();
        }
    }
}

bool KWFrameSet::isPaintedBy( KWFrameSet* fs ) const
{
    if ( fs == this )
        return true;
    if ( isFloating() )
    {
        KWFrameSet* parentFs = anchorFrameset();
        if ( parentFs && parentFs->isPaintedBy( fs ) )
            return true;
    }
    if ( getGroupManager() )
    {
        if ( getGroupManager()->isPaintedBy( fs ) )
            return true;
    }
    return false;
}

const QPtrList<KWFrame> & KWFrameSet::framesInPage( int pageNum ) const
{
    if ( pageNum < m_firstPage || pageNum >= (int)m_framesInPage.size() + m_firstPage )
    {
#ifdef DEBUG_DTI
        kdWarning(32002) << getName() << " framesInPage called for pageNum=" << pageNum << ". "
                    << " Min value: " << m_firstPage
                    << " Max value: " << m_framesInPage.size() + m_firstPage - 1 << endl;
#endif
        return m_emptyList; // QPtrList<KWFrame>() doesn't work, it's a temporary
    }
    return * m_framesInPage[pageNum - m_firstPage];
}

void KWFrameSet::drawContents( QPainter *p, const QRect & crect, const QColorGroup &cg,
                               bool onlyChanged, bool resetChanged,
                               KWFrameSetEdit *edit, KWViewMode *viewMode )
{
#ifdef DEBUG_DRAW
    kdDebug(32001) << "\nKWFrameSet::drawContents " << this << " " << getName()
                   << " onlyChanged=" << onlyChanged << " resetChanged=" << resetChanged
                   << " crect= " << crect
                   << endl;
#endif
    if ( !viewMode->isTextModeFrameset( this ) )
    {
        QPtrListIterator<KWFrame> frameIt( frameIterator() );
        KWFrame * lastRealFrame = 0L;
        //double lastRealFrameTop = 0;
        //double totalHeight = 0; // in pt, to avoid accumulating rounding errors
        for ( ; frameIt.current(); )
        {
            KWFrame *frame = frameIt.current();
            ++frameIt; // Point to the next one, to detect "last copy"
            // The settings come from this frame
            KWFrame * settingsFrame = ( frame->isCopy() && lastRealFrame ) ? lastRealFrame : frame;
            bool lastCopy = !frameIt.current() || !frameIt.current()->isCopy();
            drawFrameAndBorders( frame, p, crect, cg, onlyChanged,
                                 // Only reset the changed flag in the last copy of a given frame (#60678)
                                 resetChanged && lastCopy,
                                 edit,
                                 viewMode, settingsFrame, true /*transparency & double-buffering*/ );
            if ( !lastRealFrame || !frame->isCopy() )
            {
                lastRealFrame = frame;
                //lastRealFrameTop = totalHeight;
            }
            //totalHeight += frame->innerHeight();
        }
    } else {
        // Text view mode
        drawFrame( 0L /*frame*/, p, crect, crect, QPoint(0,0),
                   0L /*settingsFrame*/, cg, onlyChanged, resetChanged, edit, viewMode, true );
    }
}

void KWFrameSet::drawFrameAndBorders( KWFrame *frame,
                                      QPainter *painter, const QRect &crect,
                                      const QColorGroup &cg, bool onlyChanged, bool resetChanged,
                                      KWFrameSetEdit *edit, KWViewMode *viewMode,
                                      KWFrame *settingsFrame, bool drawUnderlyingFrames )
{
    if ( !frame->isValid() )
    {
        kdDebug(32002) << "KWFrameSet::drawFrameAndBorders " << getName() << " frame " << frameFromPtr( frame ) << " " << frame << " isn't valid" << endl;
        return;
    }

    QRect normalOuterFrameRect( frame->outerRect( viewMode ) );
    QRect outerFrameRect( viewMode->normalToView( normalOuterFrameRect ) );
    QRect outerCRect = crect.intersect( outerFrameRect );
#ifdef DEBUG_DRAW
    kdDebug(32001) << "KWFrameSet::drawFrameAndBorders " << getName() << " frame " << frameFromPtr( frame ) << " " << *frame << endl;
    kdDebug(32001) << "                    (outer) normalFrameRect=" << normalOuterFrameRect << " frameRect=" << outerFrameRect << endl;
    kdDebug(32001) << "                    crect=" << crect << " intersec=" << outerCRect << " todraw=" << !outerCRect.isEmpty() << endl;
#endif
    if ( !outerCRect.isEmpty() )
    {
        // Determine settingsFrame if not passed (for speedup)
        if ( !settingsFrame )
            settingsFrame = this->settingsFrame( frame );

        QRect normalInnerFrameRect( m_doc->zoomRect( frame->innerRect() ) );
        QRect innerFrameRect( viewMode->normalToView( normalInnerFrameRect ) );

        // This translates the coordinates in the document contents
        // ( frame and r are up to here in this system )
        // into the frame's own coordinate system.
        int offsetX = normalInnerFrameRect.left();
        int offsetY = normalInnerFrameRect.top() - m_doc->zoomItY( frame->internalY() );

        QRect innerCRect = outerCRect.intersect( innerFrameRect );
        QRect fcrect = viewMode->viewToNormal( innerCRect );
#ifdef DEBUG_DRAW
        kdDebug(32001) << "                    (inner) normalFrameRect=" << normalInnerFrameRect << " frameRect=" << innerFrameRect << endl;
        //kdDebug(32001) << "                    crect after view-to-normal:" << fcrect << "." << " Will move by (" << -offsetX << ", -(" << normalInnerFrameRect.top() << "-" << m_doc->zoomItY(frame->internalY()) << ") == " << -offsetY << ")." << endl;
#endif
        fcrect.moveBy( -offsetX, -offsetY );
        Q_ASSERT( fcrect.x() >= 0 );
        Q_ASSERT( fcrect.y() >= 0 );

        // fcrect is now the portion of the frame to be drawn,
        // in the frame's coordinates and in pixels
#ifdef DEBUG_DRAW
        kdDebug(32001) << "KWFrameSet::drawFrameAndBorders in frame coords:" << fcrect << ". Will translate painter by intersec-fcrect: " << innerCRect.x()-fcrect.x() << "," << innerCRect.y()-fcrect.y() << "." << endl;
#endif
        QRegion reg;
        if ( drawUnderlyingFrames )
            reg = frameClipRegion( painter, frame, outerCRect, viewMode );
        else // false means we are being drawn _as_ an underlying frame, so no clipping!
            reg = painter->xForm( outerCRect );
        if ( !reg.isEmpty() )
        {
            painter->save();
            painter->setClipRegion( reg );

            drawFrame( frame, painter, fcrect, outerCRect,
                       innerCRect.topLeft() - fcrect.topLeft(), // This assume that viewToNormal() is only a translation
                       settingsFrame, cg, onlyChanged, resetChanged,
                       edit, viewMode, drawUnderlyingFrames );

            if( !getGroupManager() ) // not for table cells
                drawFrameBorder( painter, frame, settingsFrame, outerCRect, viewMode );

            painter->restore();
        }
    }
}

void KWFrameSet::drawFrame( KWFrame *frame, QPainter *painter, const QRect &fcrect, const QRect &outerCRect,
                            const QPoint& translationOffset,
                            KWFrame *settingsFrame, const QColorGroup &cg, bool onlyChanged, bool resetChanged,
                            KWFrameSetEdit *edit, KWViewMode* viewMode, bool drawUnderlyingFrames )
{
    // In this method the painter is NOT translated yet. It's still in view coordinates.
    if ( outerCRect.isEmpty() )
        return;
#ifdef DEBUG_DRAW
    kdDebug(32001) << "\nKWFrameSet::drawFrame " << getName() << " outerCRect=" << outerCRect << " frameCrect=" << fcrect << " drawUnderlyingFrames=" << drawUnderlyingFrames << endl;
#endif

    QColorGroup frameColorGroup( cg );
    if ( settingsFrame ) // 0L in text viewmode
    {
        QBrush bgBrush( settingsFrame->backgroundColor() );
        bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), painter ) );
        frameColorGroup.setBrush( QColorGroup::Base, bgBrush );
    }

    if ( drawUnderlyingFrames && frame && !frame->framesBelow().isEmpty() )
    {
        // Double-buffering - not when printing
        QPainter* doubleBufPainter = painter;
        QPixmap* pix = 0L;
        if ( painter->device()->devType() != QInternal::Printer )
        {
            pix = m_doc->doubleBufferPixmap( outerCRect.size() );
            doubleBufPainter = new QPainter;
            doubleBufPainter->begin( pix );
            // Initialize the pixmap to the page background color
            // (if the frame is over the page margins, no underlying frame will paint anything there)
            doubleBufPainter->fillRect( 0, 0, outerCRect.width(), outerCRect.height(), QApplication::palette().active().brush( QColorGroup::Base ) );

            // The double-buffer pixmap has (0,0) at outerCRect.topLeft(), so we need to
            // translate the double-buffer painter; drawFrameAndBorders will draw using view coordinates.
            doubleBufPainter->translate( -outerCRect.x(), -outerCRect.y() );
#ifdef DEBUG_DRAW
//            kdDebug(32001) << "  ... using double buffering. Portion covered: " << outerCRect << endl;
#endif
        }

        // Transparency handling
#ifdef DEBUG_DRAW
        kdDebug(32001) << "  frame->framesBelow(): " << frame->framesBelow().count() << endl;
#endif
        QPtrListIterator<KWFrame> it( frame->framesBelow() );
        for ( ; it.current() ; ++it )
        {
            KWFrame* f = it.current();

#ifdef DEBUG_DRAW
            kdDebug(32001) << "  looking at frame below us: " << f->frameSet()->getName() << " frame " << frameFromPtr( frame ) << endl;
#endif
            QRect viewFrameCRect = outerCRect.intersect( viewMode->normalToView( f->outerRect( viewMode ) ) );
            if ( !viewFrameCRect.isEmpty() )
            {
#ifdef DEBUG_DRAW
                kdDebug(32001) << "  viewFrameRect=" << viewFrameCRect << " calling drawFrameAndBorders." << endl;
#endif
                f->frameSet()->drawFrameAndBorders( f, doubleBufPainter, viewFrameCRect, cg, false, resetChanged,
                                                    edit, viewMode, 0L, false );
            }
        }

        if ( frame->bLeft() || frame->bTop() || frame->bRight() || frame->bBottom() )
            drawMargins( frame, doubleBufPainter, outerCRect, cg, viewMode );
        doubleBufPainter->save();
#ifdef DEBUG_DRAW
        kdDebug(32001) << "  translating by " << translationOffset.x() << ", " << translationOffset.y() << " before drawFrameContents" << endl;
#endif
        doubleBufPainter->translate( translationOffset.x(), translationOffset.y() ); // This assume that viewToNormal() is only a translation
        // We can't "repaint changed parags only" if we just drew the underlying frames, hence the "false"
        drawFrameContents( frame, doubleBufPainter, fcrect, frameColorGroup, false, resetChanged, edit, viewMode );
        doubleBufPainter->restore();

        if ( painter->device()->devType() != QInternal::Printer )
        {
            doubleBufPainter->end();
#ifdef DEBUG_DRAW
            kdDebug(32001) << "  painting double-buf pixmap at position " << outerCRect.topLeft() << " (real painter pos:" << painter->xForm( outerCRect.topLeft() ) << ")" << endl;
#endif
            painter->drawPixmap( outerCRect.topLeft(), *pix );
            delete doubleBufPainter;
        }
    }
    else
    {
        if ( frame && (frame->bLeft() || frame->bTop() || frame->bRight() || frame->bBottom()) )
            drawMargins( frame, painter, outerCRect, cg, viewMode );
        painter->save();
        painter->translate( translationOffset.x(), translationOffset.y() );
        //painter->setBrushOrigin( painter->brushOrigin() + translationOffset );

        drawFrameContents( frame, painter, fcrect, frameColorGroup, onlyChanged, resetChanged, edit, viewMode );
        painter->restore();
    }
}

void KWFrameSet::drawFrameContents( KWFrame *, QPainter *, const QRect &,
                                    const QColorGroup &, bool, bool, KWFrameSetEdit*, KWViewMode * )
{
    kdWarning() << "Default implementation of drawFrameContents called for " << className() << " " << this << " " << getName() << kdBacktrace();
}

bool KWFrameSet::contains( double mx, double my )
{
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
        if ( frameIt.current()->contains( KoPoint( mx, my ) ) )
            return true;

    return false;
}

MouseMeaning KWFrameSet::getMouseMeaning( const QPoint &nPoint, int keyState )
{
    if ( grpMgr ) // Cells forward the call to the table
        return grpMgr->getMouseMeaning( nPoint, keyState );

    bool canMove = isMoveable();
    KoPoint docPoint = m_doc->unzoomPoint( nPoint );
    MouseMeaning defaultCursor = canMove ? MEANING_MOUSE_MOVE : MEANING_MOUSE_SELECT;
    // See if we're over a frame border
    KWFrame * frame = frameByBorder( nPoint );
    if ( frame )
    {
        return frame->getMouseMeaning( docPoint, defaultCursor );
    }

    frame = frameAtPos( docPoint.x(), docPoint.y() );
    if ( frame == 0L ) {
        return MEANING_NONE;
    }

    // Found a frame under the cursor
    // Ctrl -> move or select
    if ( keyState & ControlButton )
        return defaultCursor;

    // Shift _and_ at least a frame is selected already
    // (shift + no frame selected is used to select text)
    if ( (keyState & ShiftButton) && (m_doc->getFirstSelectedFrame() != 0L) )
        return defaultCursor;

    return getMouseMeaningInsideFrame( docPoint );
}

MouseMeaning KWFrameSet::getMouseMeaningInsideFrame( const KoPoint& )
{
    return isMoveable() ? MEANING_MOUSE_MOVE : MEANING_MOUSE_SELECT;
}

void KWFrameSet::saveCommon( QDomElement &parentElem, bool saveFrames )
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return;

    // Save all the common attributes for framesets.
    if (m_doc->specialOutputFlag()==KoDocument::SaveAsKOffice1dot1)
    {
        parentElem.setAttribute( "frameType", static_cast<int>( typeAsKOffice1Dot1() ) );
    }
    else
    {
        parentElem.setAttribute( "frameType", static_cast<int>( type() ) );
    }
    parentElem.setAttribute( "frameInfo", static_cast<int>( m_info ) );
    parentElem.setAttribute( "name", m_name );
    parentElem.setAttribute( "visible", static_cast<int>( m_visible ) );
    parentElem.setAttribute( "protectSize", static_cast<int>( m_protectSize ) );
    if ( saveFrames )
    {
        QPtrListIterator<KWFrame> frameIt = frameIterator();
        for ( ; frameIt.current(); ++frameIt )
        {
            KWFrame *frame = frameIt.current();
            QDomElement frameElem = parentElem.ownerDocument().createElement( "FRAME" );
            parentElem.appendChild( frameElem );

            frame->save( frameElem );

            if(m_doc->processingType() == KWDocument::WP) {
                // Assume that all header/footer frames in the same frameset are
                // perfect copies. This might not be the case some day though.
                if(frameSetInfo() == FI_FIRST_HEADER ||
                   frameSetInfo() == FI_EVEN_HEADER ||
                   frameSetInfo() == FI_ODD_HEADER ||
                   frameSetInfo() == FI_FIRST_FOOTER ||
                   frameSetInfo() == FI_EVEN_FOOTER ||
                   frameSetInfo() == FI_ODD_FOOTER ||
                   frameSetInfo() == FI_FOOTNOTE) break;
            }
        }
    }
}

//
// This function is intended as a helper for all the derived classes. It reads
// in all the attributes common to all framesets and loads all frames.
//
void KWFrameSet::load( QDomElement &framesetElem, bool loadFrames )
{
    m_info = static_cast<KWFrameSet::Info>( KWDocument::getAttribute( framesetElem, "frameInfo", KWFrameSet::FI_BODY ) );
    m_visible = static_cast<bool>( KWDocument::getAttribute( framesetElem, "visible", true ) );
    if ( framesetElem.hasAttribute( "removeable" ) )
        m_removeableHeader = static_cast<bool>( KWDocument::getAttribute( framesetElem, "removeable", false ) );
    else
        m_removeableHeader = static_cast<bool>( KWDocument::getAttribute( framesetElem, "removable", false ) );
    m_protectSize=static_cast<bool>( KWDocument::getAttribute( framesetElem, "protectSize", false ) );
    if ( loadFrames )
    {
        // <FRAME>
        QDomElement frameElem = framesetElem.firstChild().toElement();
        for ( ; !frameElem.isNull() ; frameElem = frameElem.nextSibling().toElement() )
        {
            if ( frameElem.tagName() == "FRAME" )
            {
                KoRect rect;
                rect.setLeft( KWDocument::getAttribute( frameElem, "left", 0.0 ) );
                rect.setTop( KWDocument::getAttribute( frameElem, "top", 0.0 ) );
                rect.setRight( KWDocument::getAttribute( frameElem, "right", 0.0 ) );
                rect.setBottom( KWDocument::getAttribute( frameElem, "bottom", 0.0 ) );
                KWFrame * frame = new KWFrame(this, rect.x(), rect.y(), rect.width(), rect.height() );
                frame->load( frameElem, this, m_doc->syntaxVersion() );
                addFrame( frame, false );
                m_doc->progressItemLoaded();
            }
        }
    }
}

bool KWFrameSet::hasSelectedFrame()
{
    for ( unsigned int i = 0; i < frames.count(); i++ ) {
        if ( frames.at( i )->isSelected() )
            return true;
    }

    return false;
}

void KWFrameSet::setVisible( bool v )
{
    m_visible = v;
    if ( m_visible )
        // updateFrames was disabled while we were invisible
        updateFrames();
}

bool KWFrameSet::isVisible( KWViewMode* viewMode ) const
{
    if ( !m_visible || frames.isEmpty() )
        return false;
    if ( isAHeader() && !m_doc->isHeaderVisible() )
        return false;
    if ( isAFooter() && !m_doc->isFooterVisible() )
        return false;
    if ( viewMode && !viewMode->isFrameSetVisible(this) )
        return false;
    if ( isFloating() && !anchorFrameset()->isVisible( viewMode ) )
         return false;

    KoHFType ht = m_doc->getHeaderType();
    KoHFType ft = m_doc->getFooterType();
    switch( m_info )
    {
    case FI_FIRST_HEADER:
        return ( ht == HF_FIRST_DIFF || ht == HF_FIRST_EO_DIFF );
    case FI_ODD_HEADER:
        return true;
    case FI_EVEN_HEADER:
        return ( ht == HF_EO_DIFF || ht == HF_FIRST_EO_DIFF );
    case FI_FIRST_FOOTER:
        return ( ft == HF_FIRST_DIFF || ft == HF_FIRST_EO_DIFF );
    case FI_ODD_FOOTER:
        return true;
    case FI_EVEN_FOOTER:
        return ( ft == HF_EO_DIFF || ft == HF_FIRST_EO_DIFF );
    default:
        return true;
    }
}

bool KWFrameSet::isAHeader() const
{
    return ( m_info == FI_FIRST_HEADER || m_info == FI_ODD_HEADER || m_info == FI_EVEN_HEADER );
}

bool KWFrameSet::isAFooter() const
{
    return ( m_info == FI_FIRST_FOOTER || m_info == FI_ODD_FOOTER || m_info == FI_EVEN_FOOTER );
}

bool KWFrameSet::isFootEndNote() const
{
    return m_info == FI_FOOTNOTE;
}

bool KWFrameSet::isMainFrameset() const
{
    return ( m_doc->processingType() == KWDocument::WP &&
             m_doc->frameSet( 0 ) == this );
}

bool KWFrameSet::isMoveable() const
{
    if ( isHeaderOrFooter() )
        return false;
    return !isMainFrameset() && !isFloating();
}

void KWFrameSet::zoom( bool )
{
}

void KWFrameSet::finalize()
{
    //kdDebug(32001) << "KWFrameSet::finalize ( calls updateFrames + zoom ) " << this << endl;
    updateFrames();
    zoom( false );
}

QRegion KWFrameSet::frameClipRegion( QPainter * painter, KWFrame *frame, const QRect & crect,
                                     KWViewMode * viewMode )
{
    KWDocument * doc = kWordDocument();
    QRect rc = painter->xForm( crect );
    KoRect clipKoRect = doc->unzoomRect(viewMode->viewToNormal(crect));
#ifdef DEBUG_DRAW
    //kdDebug(32002) << "KWFrameSet::frameClipRegion rc initially " << rc << endl;
#endif

    Q_ASSERT( frame );
#if 0 // done later
    if ( clipFrame )
    {
        rc &= painter->xForm( viewMode->normalToView( doc->zoomRect( (*frame) ) ) ); // intersect
#ifdef DEBUG_DRAW
        kdDebug(32002) << "KWFrameSet::frameClipRegion frame=" << *frame
                       << " clip region rect=" << rc
                       << " rc.isEmpty()=" << rc.isEmpty() << endl;
#endif
    }
#endif
    if ( !rc.isEmpty() )
    {
        QRegion reg( rc );
        // This breaks when a frame is under another one, it still appears if !onlyChanged.
        // cvs log says this is about frame borders... hmm.
        /// ### if ( onlyChanged )

	QPtrListIterator<KWFrame> fIt( frame->framesOnTop() );
        for ( ; fIt.current() ; ++fIt )
        {
            QRect r = painter->xForm( viewMode->normalToView( (*fIt)->outerRect( viewMode ) ) );
#ifdef DEBUG_DRAW
            //kdDebug(32002) << "frameClipRegion subtract rect "<< r << endl;
#endif
            reg -= r; // subtract
        }
#ifdef DEBUG_DRAW
        //kdDebug(32002) << "KWFrameSet::frameClipRegion result:" << reg << endl;
#endif
        return reg;
    } else return QRegion();
}

bool KWFrameSet::canRemovePage( int num )
{
    QPtrListIterator<KWFrame> frameIt( frameIterator() );
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame * frame = frameIt.current();
        if ( frame->pageNum() == num ) // ## TODO: use framesInPage, see KWTextFrameSet
        {
            // Ok, so we have a frame on that page -> we can't remove it unless it's a copied frame
            if ( ! ( frame->isCopy() && frameIt.current() != frames.first() ) )
            {
                //kdDebug(32001) << "KWFrameSet::canRemovePage " << getName() << " frame on page " << num << " -> false" << endl;
                return false;
            }
        }
    }
    return true;
}

void KWFrameSet::showPopup( KWFrame *, KWView *view, const QPoint &point )
{
    QPopupMenu * popup = view->popupMenu("frame_popup");
    Q_ASSERT(popup);
    if (popup)
        popup->popup( point );
}

void KWFrameSet::setFrameBehavior( KWFrame::FrameBehavior fb ) {
    for(KWFrame *f=frames.first();f;f=frames.next())
        f->setFrameBehavior(fb);
}

void KWFrameSet::setNewFrameBehavior( KWFrame::NewFrameBehavior nfb ) {
    for(KWFrame *f=frames.first();f;f=frames.next())
        f->setNewFrameBehavior(nfb);
}

void KWFrameSet::resizeFrameSetCoords( KWFrame* frame, double newLeft, double newTop, double newRight, double newBottom, bool finalSize )
{
    frame->setLeft( newLeft );
    frame->setTop( newTop );
    resizeFrame( frame, newRight - newLeft, newBottom - newTop, finalSize );
}

void KWFrameSet::resizeFrame( KWFrame* frame, double newWidth, double newHeight, bool /*finalSize*/ )
{
    frame->setWidth( newWidth );
    frame->setHeight( newHeight );
    if ( frame->frameBehavior() == KWFrame::AutoExtendFrame )
        frame->setMinFrameHeight( newHeight );
}

bool KWFrameSet::isFrameAtPos( KWFrame* frame, const QPoint& point, bool borderOfFrameOnly) {
    QRect outerRect( frame->outerRect( m_doc->viewMode() ) );
    // Give the user a bit of margin for clicking on it :)
    const int margin = 2;
    outerRect.rLeft() -= margin;
    outerRect.rTop() -= margin;
    outerRect.rRight() += margin;
    outerRect.rBottom() += margin;
    if ( outerRect.contains( point ) ) {
        if(borderOfFrameOnly) {
            QRect innerRect( m_doc->zoomRect( *frame ) );
            innerRect.rLeft() += margin;
            innerRect.rTop() += margin;
            innerRect.rRight() -= margin;
            innerRect.rBottom() -= margin;
            return (!innerRect.contains(point) );
        }
        return true;
    }
    return false;
}

void KWFrameSet::setZOrder()
{
    //kdDebug(32001) << "KWFrameSet::setZOrder (to max) " << getName() << endl;
    QPtrListIterator<KWFrame> fit = frameIterator();
    for ( ; fit.current() ; ++fit )
        fit.current()->setZOrder( m_doc->maxZOrder( fit.current()->pageNum(m_doc) ) + 1 );
}

#ifndef NDEBUG
void KWFrameSet::printDebug()
{
    static const char * typeFrameset[] = { "base", "txt", "picture", "part", "formula", "clipart",
                                           "6", "7", "8", "9", "table",
                                           "ERROR" };
    static const char * infoFrameset[] = { "body", "first header", "even headers", "odd headers",
                                           "first footer", "even footers", "odd footers", "footnote", "ERROR" };
    static const char * frameBh[] = { "AutoExtendFrame", "AutoCreateNewFrame", "Ignore", "ERROR" };
    static const char * newFrameBh[] = { "Reconnect", "NoFollowup", "Copy" };
    static const char * runaround[] = { "No Runaround", "Bounding Rect", "Skip", "ERROR" };
    static const char * runaroundSide[] = { "Biggest", "Left", "Right", "ERROR" };

    kdDebug() << " |  Visible: " << isVisible() << endl;
    kdDebug() << " |  Type: " << typeFrameset[ type() ] << endl;
    kdDebug() << " |  Info: " << infoFrameset[ frameSetInfo() ] << endl;
    kdDebug() << " |  Floating: " << isFloating() << endl;
    kdDebug() << " |  Frames in page array: " << endl;
    for ( uint i = 0 ; i < m_framesInPage.size() ; ++i )
    {
        QPtrListIterator<KWFrame> it( *m_framesInPage[i] );
        int pgNum = i + m_firstPage;
        for ( ; it.current() ; ++it )
            kdDebug() << " |     " << pgNum << ": " << it.current() << "   " << *it.current()
                      << " internalY=" << it.current()->internalY() << "pt "
                      << " (in LU pix:" << m_doc->ptToLayoutUnitPixY( it.current()->internalY() ) << ")"
                      << " innerHeight=" << it.current()->innerHeight()
                      << " (in LU pix:" << m_doc->ptToLayoutUnitPixY( it.current()->innerHeight() ) << ")"
                      << endl;
    }

    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( unsigned int j = 0; frameIt.current(); ++frameIt, ++j ) {
        KWFrame * frame = frameIt.current();
        QCString copy = frame->isCopy() ? "[copy]" : "";
        kdDebug() << " +-- Frame " << j << " of "<< getNumFrames() << "    (" << frame << ")  " << copy << endl;
        printDebug( frame );
        kdDebug() << "     Rectangle : " << frame->x() << "," << frame->y() << " " << frame->width() << "x" << frame->height() << endl;
        kdDebug() << "     RunAround: "<< runaround[ frame->runAround() ] << " side:" << runaroundSide[ frame->runAroundSide() ]<< endl;
        kdDebug() << "     FrameBehavior: "<< frameBh[ frame->frameBehavior() ] << endl;
        kdDebug() << "     NewFrameBehavior: "<< newFrameBh[ frame->newFrameBehavior() ] << endl;
        QColor col = frame->backgroundColor().color();
        kdDebug() << "     BackgroundColor: "<< ( col.isValid() ? col.name().latin1() : "(default)" ) << endl;
        kdDebug() << "     SheetSide "<< frame->sheetSide() << endl;
        kdDebug() << "     Z Order: " << frame->zOrder() << endl;
        kdDebug() << "     Frames below: " << frame->framesBelow().count()
                  << " frames on top: " << frame->framesOnTop().count() << endl;
        kdDebug() << "     minFrameHeight "<< frame->minFrameHeight() << endl;
        if(frame->isSelected())
            kdDebug() << " *   Page "<< frame->pageNum() << endl;
        else
            kdDebug() << "     Page "<< frame->pageNum() << endl;
    }
}

void KWFrameSet::printDebug( KWFrame * )
{
}

#endif

KWFrameSetEdit::KWFrameSetEdit( KWFrameSet * fs, KWCanvas * canvas )
     : m_fs(fs), m_canvas(canvas), m_currentFrame( fs->frame(0) )
{
}

void KWFrameSetEdit::drawContents( QPainter *p, const QRect &crect,
                                   const QColorGroup &cg, bool onlyChanged, bool resetChanged,
                                   KWViewMode *viewMode )
{
    //kdDebug(32001) << "KWFrameSetEdit::drawContents " << frameSet()->getName() << endl;
    frameSet()->drawContents( p, crect, cg, onlyChanged, resetChanged, this, viewMode );
}

void KWFrameSetEdit::showPopup( KWFrame* frame, KWView* view, const QPoint & _point )
{
    frame->frameSet()->showPopup( frame, view, _point );
}

/******************************************************************/
/* Class: KWPictureFrameSet                                       */
/******************************************************************/
KWPictureFrameSet::KWPictureFrameSet( KWDocument *_doc, const QString & name )
    : KWFrameSet( _doc ), m_finalSize( false )
{
    if ( name.isEmpty() )
        m_name = _doc->generateFramesetName( i18n( "Picture %1" ) );
    else
        m_name = name;
    m_keepAspectRatio = true;
}

KWPictureFrameSet::~KWPictureFrameSet() {
}

KWordFrameSetIface* KWPictureFrameSet::dcopObject()
{
    if ( !m_dcop )
	m_dcop = new KWordPictureFrameSetIface( this );

    return m_dcop;
}

void KWPictureFrameSet::loadPicture( const QString & fileName )
{
    KoPictureCollection *collection = m_doc->pictureCollection();

    m_picture = collection->loadPicture( fileName );
}

void KWPictureFrameSet::insertPicture( const KoPicture& picture )
{
    KoPictureCollection *collection = m_doc->pictureCollection();

    m_picture = collection->insertPicture( picture.getKey(), picture );
}

void KWPictureFrameSet::reloadPicture( const KoPictureKey& key )
{
    KoPictureCollection *collection = m_doc->pictureCollection();
    // If the picture is not already in the collection, then it gives a blank picture
    m_picture = collection->insertPicture( key, KoPicture() );
}

void KWPictureFrameSet::setSize( const QSize & /*_imgSize*/ )
{
    // Do nothing as KoPicture needs a size only at drawing time
}

void KWPictureFrameSet::resizeFrame( KWFrame* frame, double newWidth, double newHeight, bool finalSize )
{
    KWFrameSet::resizeFrame( frame, newWidth, newHeight, finalSize );
    //QSize newSize = kWordDocument()->zoomSize( frame->innerRect().size() );
    m_finalSize=finalSize; // Cache the value for drawing time!
}

QDomElement KWPictureFrameSet::save( QDomElement & parentElem, bool saveFrames )
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return QDomElement();
    QDomElement framesetElem = parentElem.ownerDocument().createElement( "FRAMESET" );
    parentElem.appendChild( framesetElem );

    KWFrameSet::saveCommon( framesetElem, saveFrames );

    QDomElement imageElem;

    if (m_doc->specialOutputFlag()==KoDocument::SaveAsKOffice1dot1)
    {
        // KWord 1.1 file format
        QString strElementName=m_picture.isClipartAsKOffice1Dot1() ? QString( "CLIPART" ) : QString( "IMAGE" );
        imageElem = parentElem.ownerDocument().createElement( strElementName );
        framesetElem.appendChild( imageElem );
        if ( !m_picture.isClipartAsKOffice1Dot1() )
        {
            // KWord 1.1 does not save keepAspectRaio for a clipart
            imageElem.setAttribute( "keepAspectRatio", m_keepAspectRatio ? "true" : "false" );
        }
    }
    else
    {
        // KWord 1.3 file format
        imageElem = parentElem.ownerDocument().createElement( "PICTURE" );
        framesetElem.appendChild( imageElem );
        imageElem.setAttribute( "keepAspectRatio", m_keepAspectRatio ? "true" : "false" );
    }
    QDomElement elem = parentElem.ownerDocument().createElement( "KEY" );
    imageElem.appendChild( elem );
    m_picture.getKey().saveAttributes( elem );
    return framesetElem;
}

void KWPictureFrameSet::load( QDomElement &attributes, bool loadFrames )
{
    KWFrameSet::load( attributes, loadFrames );

    QString defaultRatio="true";
    // <PICTURE>
    QDomNode node=attributes.namedItem( "PICTURE" );
    if ( node.isNull() )
    {
        node=attributes.namedItem( "IMAGE" );
        if ( node.isNull() )
        {
            node=attributes.namedItem( "CLIPART" );
            defaultRatio="false";
        }
    }

    QDomElement image = node.toElement();
    if ( !image.isNull() ) {
        m_keepAspectRatio = image.attribute( "keepAspectRatio", defaultRatio ) == "true";
        // <KEY>
        QDomElement keyElement = image.namedItem( "KEY" ).toElement();
        if ( !keyElement.isNull() )
        {
            KoPictureKey key;
            key.loadAttributes( keyElement );
            m_picture.clear();
            m_picture.setKey( key );
            m_doc->addPictureRequest( this );
        }
        else
        {
            // <FILENAME> (old format, up to KWord-1.1-beta2)
            QDomElement filenameElement = image.namedItem( "FILENAME" ).toElement();
            if ( !filenameElement.isNull() )
            {
                QString filename = filenameElement.attribute( "value" );
                m_picture.clear();
                m_picture.setKey( KoPictureKey( filename ) );
                m_doc->addPictureRequest( this );
            }
            else
            {
                kdError(32001) << "Missing KEY tag in IMAGE" << endl;
            }
        }
    } else {
        kdError(32001) << "Missing PICTURE/IMAGE/CLIPART tag in FRAMESET" << endl;
    }
}

void KWPictureFrameSet::drawFrameContents( KWFrame *frame, QPainter *painter, const QRect &crect,
                                           const QColorGroup &, bool, bool, KWFrameSetEdit *, KWViewMode * )
{
#ifdef DEBUG_DRAW
    kdDebug(32001) << "KWPictureFrameSet::drawFrameContents crect=" << crect << " size=" << kWordDocument()->zoomItX( frame->innerWidth() ) << "x" << kWordDocument()->zoomItY( frame->innerHeight() ) << endl;
#endif
    m_picture.draw( *painter, 0, 0, kWordDocument()->zoomItX( frame->innerWidth() ), kWordDocument()->zoomItY( frame->innerHeight() ),
                  crect.x(), crect.y(), crect.width(), crect.height(), !m_finalSize);
}

FrameSetType KWPictureFrameSet::type( void )
{
    return FT_PICTURE;
}

FrameSetType KWPictureFrameSet::typeAsKOffice1Dot1( void )
{
    return m_picture.isClipartAsKOffice1Dot1()?FT_CLIPART:FT_PICTURE;
}

bool KWPictureFrameSet::keepAspectRatio() const
{
    return m_keepAspectRatio;
}

void KWPictureFrameSet::setKeepAspectRatio( bool b )
{
    m_keepAspectRatio = b;
}

#ifndef NDEBUG
void KWPictureFrameSet::printDebug( KWFrame *frame )
{
    KWFrameSet::printDebug( frame );
    if ( !isDeleted() )
    {
        kdDebug(32001) << "Image: key=" << m_picture.getKey().toString() << endl;
    }
}
#endif

#if 0 // KWORD_HORIZONTAL_LINE
/******************************************************************/
/* Class: KWHorzLineFrameSet                                      */
/******************************************************************/
KWHorzLineFrameSet::KWHorzLineFrameSet( KWDocument *_doc, const QString & name )
    : KWPictureFrameSet( _doc, "none" /*don't generate name in kwpictureframeset*/)
{
    kdDebug(32001) << "KWHorzLineFrameSet::KWHorzLineFrameSet" << endl;
    if ( name.isEmpty() )
        m_name = _doc->generateFramesetName( i18n( "Horizontal line %1" ) );
    else
        m_name = name;
}

KWHorzLineFrameSet::~KWHorzLineFrameSet()
{
    //todo
}

FrameSetType KWHorzLineFrameSet::type( void )
{
    return FT_HORZLINE;
}

bool KWHorzLineFrameSet::ownLine() const
{
    return TRUE;
}

KWordFrameSetIface* KWHorzLineFrameSet::dcopObject()
{
    if ( !m_dcop )
	m_dcop = new KWHorizontalLineFrameSetIface( this );
    return m_dcop;
}


void KWHorzLineFrameSet::drawFrameContents( KWFrame *frame, QPainter *painter, const QRect &crect,
                                           const QColorGroup &cg, bool onlyChanged, bool resetChanged, KWFrameSetEdit *edit, KWViewMode * viewmode)
{
    if ( !m_picture.isNull())
        KWPictureFrameSet::drawFrameContents( frame, painter, crect,
                                              cg, onlyChanged, resetChanged, edit, viewmode);
    else
    {
        if ( painter->device()->devType() == QInternal::Printer ) {
            QPen oldPen = painter->pen();
                painter->setPen( QPen( cg.text(), crect.height()/8 ) );
            painter->drawLine( crect.left()-1, crect.y() + crect.height() / 2, crect.right() + 1, crect.y() + crect.height() / 2 );
            painter->setPen( oldPen );
        } else {
            QColorGroup g( cg );
            g.setColor( QColorGroup::Dark, Qt::red );
            qDrawShadeLine( painter, crect.left() - 1, crect.y() + crect.height() / 2, crect.right() + 1, crect.y() + crect.height() / 2, g, TRUE, crect.height() / 8 );
        }

    }
}
#endif

#include "kwframe.moc"
