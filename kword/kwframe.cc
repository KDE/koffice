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
#include "kwutils.h"
#include "kwtextframeset.h"
#include "kwanchor.h"
#include "resizehandles.h"
#include <qpicture.h>

#include <kformulacontainer.h>
#include <kformuladocument.h>
#include <kformulaview.h>

#include <kcursor.h>
#include <kdebug.h>

/******************************************************************/
/* Class: KWFrame                                                 */
/******************************************************************/

KWFrame::KWFrame(KWFrame * frame)
{
    handles.setAutoDelete(true);
    m_minFrameHeight=0;
    copySettings( frame );
}

KWFrame::KWFrame(KWFrameSet *fs, double left, double top, double width, double height, RunAround _ra, double _gap )
    : KoRect( left, top, width, height ),
      // Initialize member vars here. This ensures they are all initialized, since it's
      // easier to compare this list with the member vars list (compiler ensures order).
      sheetSide( AnySide ),
      m_runAround( _ra ),
      frameBehaviour( AutoCreateNewFrame ),
      newFrameBehaviour( ( fs && fs->type() == FT_TEXT ) ? Reconnect : NoFollowup ),
      m_runAroundGap( _gap ),
      bleft( 0 ),
      bright( 0 ),
      btop( 0 ),
      bbottom( 0 ),
      m_bCopy( false ),
      selected( false ),
      m_internalY( 0 ),
      backgroundColor( QBrush( QColor() ) ), // valid brush with invalid color ( default )
      brd_left( QColor(), Border::SOLID, 0 ),
      brd_right( QColor(), Border::SOLID, 0 ),
      brd_top( QColor(), Border::SOLID, 0 ),
      brd_bottom( QColor(), Border::SOLID, 0 ),
      frameSet( fs )
{
    //kdDebug() << "KWFrame::KWFrame " << this << " left=" << left << " top=" << top << endl;
    handles.setAutoDelete(true);
    m_minFrameHeight=0;
}

KWFrame::~KWFrame()
{
    //kdDebug() << "KWFrame::~KWFrame " << this << endl;
    if (selected)
        removeResizeHandles();
}

int KWFrame::pageNum() const
{
    ASSERT( frameSet );
    if ( !frameSet )
        return 0;
    KWDocument *doc = frameSet->kWordDocument();
    int page = static_cast<int>(y() / doc->ptPaperHeight());
    return QMIN( page, doc->getPages()-1 );
}

QCursor KWFrame::getMouseCursor( const KoPoint & docPoint, bool table, QCursor defaultCursor )
{
    if ( !selected && !table )
        return defaultCursor;

    double mx = docPoint.x();
    double my = docPoint.y();

    if ( !table ) {
        if ( mx >= x() && my >= y() && mx <= x() + 6 && my <= y() + 6 )
            return Qt::sizeFDiagCursor;
        if ( mx >= x() && my >= y() + height() / 2 - 3 && mx <= x() + 6 && my <= y() + height() / 2 + 3 )
            return Qt::sizeHorCursor;
        if ( mx >= x() && my >= y() + height() - 6 && mx <= x() + 6 && my <= y() + height() )
            return Qt::sizeBDiagCursor;
        if ( mx >= x() + width() / 2 - 3 && my >= y() && mx <= x() + width() / 2 + 3 && my <= y() + 6 )
            return Qt::sizeVerCursor;
        if ( mx >= x() + width() / 2 - 3 && my >= y() + height() - 6 && mx <= x() + width() / 2 + 3 &&
             my <= y() + height() )
            return Qt::sizeVerCursor;
        if ( mx >= x() + width() - 6 && my >= y() && mx <= x() + width() && my <= y() + 6 )
            return Qt::sizeBDiagCursor;
        if ( mx >= x() + width() - 6 && my >= y() + height() / 2 - 3 && mx <= x() + width() &&
             my <= y() + height() / 2 + 3 )
            return Qt::sizeHorCursor;
        if ( mx >= x() + width() - 6 && my >= y() + height() - 6 && mx <= x() + width() && my <= y() + height() )
            return Qt::sizeFDiagCursor;

        //if ( selected )
        //    return Qt::sizeAllCursor;
    } else { // Tables
        // ### TODO move to KWTableFrameSet
        if ( mx >= x() + width() - 6 && my >= y() && mx <= x() + width() && my <= y() + height() )
            return Qt::sizeHorCursor;
        if ( mx >= x() && my >= y() + height() - 6 && mx <= x() + width() && my <= y() + height() )
            return Qt::sizeVerCursor;
        //return Qt::sizeAllCursor;
    }

    return defaultCursor;
}

KWFrame *KWFrame::getCopy() {
    /* returns a deep copy of self */
    return new KWFrame(this);
}

void KWFrame::copySettings(KWFrame *frm)
{
    //kdDebug() << "KWFrame::copySettings this=" << this << " frm=" << frm << endl;
    //necessary to reapply these parameters
    setFrameSet( frm->getFrameSet() );
    setRect(frm->x(), frm->y(), frm->width(), frm->height());
    setRunAroundGap( frm->runAroundGap());
    setRunAround( frm->runAround());

    //
    setBackgroundColor( frm->getBackgroundColor() );
    setFrameBehaviour(frm->getFrameBehaviour());
    setNewFrameBehaviour(frm->getNewFrameBehaviour());
    setSheetSide(frm->getSheetSide());
    setLeftBorder(frm->leftBorder());
    setRightBorder(frm->rightBorder());
    setTopBorder(frm->topBorder());
    setBottomBorder(frm->bottomBorder());
    setBLeft(frm->getBLeft());
    setBRight(frm->getBRight());
    setBTop(frm->getBTop());
    setBBottom(frm->getBBottom());
    setCopy(frm->isCopy());
    selected = false; // don't copy this attribute
    /*if(frm->anchor())
        setAnchor(frm->anchor());*/
}

// Insert all resize handles
void KWFrame::createResizeHandles() {
    QList <KWView> pages = getFrameSet()->kWordDocument()->getAllViews();
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
    handles.clear();
}

// move the resizehandles to current location of frame
void KWFrame::updateResizeHandles() {
    for (unsigned int i=0; i< handles.count(); i++) {
        handles.at(i)->updateGeometry();
    }
}


void KWFrame::updateRulerHandles(){
    if(isSelected())
        updateResizeHandles();
    else
    {
        KWDocument *doc = getFrameSet()->kWordDocument();
        if(doc)
            doc->updateRulerFrameStartEnd();
    }

}

void KWFrame::setSelected( bool _selected )
{
    bool s = selected;
    selected = _selected;
    //kdDebug() << "KWFrame::setSelected(" << _selected << ") - was selected:" << s << endl;
    if ( selected )
        createResizeHandles();
    else if ( s )
        removeResizeHandles();
}

QRect KWFrame::outerRect() const
{
    KWDocument *doc = getFrameSet()->kWordDocument();
    QRect outerRect( doc->zoomRect( *this ) );
    outerRect.rLeft() -= Border::zoomWidthX( brd_left.ptWidth, doc, 1 );
    outerRect.rTop() -= Border::zoomWidthY( brd_top.ptWidth, doc, 1 );
    outerRect.rRight() += Border::zoomWidthX( brd_right.ptWidth, doc, 1 );
    outerRect.rBottom() += Border::zoomWidthY( brd_bottom.ptWidth, doc, 1 );
    return outerRect;
}

KoRect KWFrame::outerKoRect() const
{
    KoRect outerRect = *this;
    KWDocument *doc = getFrameSet()->kWordDocument();
/*    outerRect.rLeft() -= brd_left.ptWidth;
    outerRect.rTop() -= brd_top.ptWidth;
    outerRect.rRight() += brd_right.ptWidth;
    outerRect.rBottom() += brd_bottom.ptWidth;*/
    outerRect.rLeft() -= Border::zoomWidthX( brd_left.ptWidth, doc, 1 ) / doc->zoomedResolutionX();
    outerRect.rTop() -= Border::zoomWidthY( brd_top.ptWidth, doc, 1 ) / doc->zoomedResolutionY();
    outerRect.rRight() += Border::zoomWidthX( brd_right.ptWidth, doc, 1 ) / doc->zoomedResolutionX();
    outerRect.rBottom() += Border::zoomWidthY( brd_bottom.ptWidth, doc, 1 ) / doc->zoomedResolutionY();
    return outerRect;
}

void KWFrame::save( QDomElement &frameElem )
{
    frameElem.setAttribute( "left", left() );
    frameElem.setAttribute( "top", top() );
    frameElem.setAttribute( "right", right() );
    frameElem.setAttribute( "bottom", bottom() );

    if(runAround()!=RA_NO)
        frameElem.setAttribute( "runaround", static_cast<int>( runAround() ) );

    if(runAroundGap()!=0)
        frameElem.setAttribute( "runaroundGap", runAroundGap() );

    if(leftBorder().ptWidth!=0)
        frameElem.setAttribute( "lWidth", leftBorder().ptWidth );

    if(leftBorder().color.isValid())
    {
        frameElem.setAttribute( "lRed", leftBorder().color.red() );
        frameElem.setAttribute( "lGreen", leftBorder().color.green() );
        frameElem.setAttribute( "lBlue", leftBorder().color.blue() );
    }
    if(leftBorder().style != Border::SOLID)
        frameElem.setAttribute( "lStyle", static_cast<int>( leftBorder().style ) );

    if(rightBorder().ptWidth!=0)
        frameElem.setAttribute( "rWidth", rightBorder().ptWidth );

    if(rightBorder().color.isValid())
    {
        frameElem.setAttribute( "rRed", rightBorder().color.red() );
        frameElem.setAttribute( "rGreen", rightBorder().color.green() );
        frameElem.setAttribute( "rBlue", rightBorder().color.blue() );
    }
    if(rightBorder().style != Border::SOLID)
        frameElem.setAttribute( "rStyle", static_cast<int>( rightBorder().style ) );

    if(topBorder().ptWidth!=0)
        frameElem.setAttribute( "tWidth", topBorder().ptWidth );

    if(topBorder().color.isValid())
    {
        frameElem.setAttribute( "tRed", topBorder().color.red() );
        frameElem.setAttribute( "tGreen", topBorder().color.green() );
        frameElem.setAttribute( "tBlue", topBorder().color.blue() );
    }
    if(topBorder().style != Border::SOLID)
        frameElem.setAttribute( "tStyle", static_cast<int>( topBorder().style ) );

    if(bottomBorder().ptWidth!=0) {
        frameElem.setAttribute( "bWidth", bottomBorder().ptWidth );
    }
    if(bottomBorder().color.isValid()) {
        frameElem.setAttribute( "bRed", bottomBorder().color.red() );
        frameElem.setAttribute( "bGreen", bottomBorder().color.green() );
        frameElem.setAttribute( "bBlue", bottomBorder().color.blue() );
    }
    if(bottomBorder().style != Border::SOLID)
        frameElem.setAttribute( "bStyle", static_cast<int>( bottomBorder().style ) );

    if(getBackgroundColor().color().isValid())
    {
        frameElem.setAttribute( "bkRed", getBackgroundColor().color().red() );
        frameElem.setAttribute( "bkGreen", getBackgroundColor().color().green() );
        frameElem.setAttribute( "bkBlue", getBackgroundColor().color().blue() );
    }
    if(getBLeft() != 0)
        frameElem.setAttribute( "bleftpt", getBLeft() );

    if(getBRight()!=0)
        frameElem.setAttribute( "brightpt", getBRight() );

    if(getBTop()!=0)
        frameElem.setAttribute( "btoppt", getBTop() );

    if(getBBottom()!=0)
        frameElem.setAttribute( "bbottompt", getBBottom() );

    if(getFrameBehaviour()!=AutoCreateNewFrame)
        frameElem.setAttribute( "autoCreateNewFrame", static_cast<int>( getFrameBehaviour()) );

    //if(getNewFrameBehaviour()!=Reconnect) // always save this one, since the default value depends on the type of frame, etc.
    frameElem.setAttribute( "newFrameBehaviour", static_cast<int>( getNewFrameBehaviour()) );

    //same reason
    frameElem.setAttribute( "copy", static_cast<int>( m_bCopy ) );

    if(getSheetSide()!= AnySide)
        frameElem.setAttribute( "sheetSide", static_cast<int>( getSheetSide()) );
}

void KWFrame::load( QDomElement &frameElem, bool headerOrFooter, int syntaxVersion )
{
    m_runAround = static_cast<RunAround>( KWDocument::getAttribute( frameElem, "runaround", RA_NO ) );
    m_runAroundGap = ( frameElem.hasAttribute( "runaroundGap" ) )
                          ? frameElem.attribute( "runaroundGap" ).toDouble()
                          : frameElem.attribute( "runaGapPT" ).toDouble();
    sheetSide = static_cast<SheetSide>( KWDocument::getAttribute( frameElem, "sheetSide", AnySide ) );
    frameBehaviour = static_cast<FrameBehaviour>( KWDocument::getAttribute( frameElem, "autoCreateNewFrame", AutoCreateNewFrame ) );
    // Old documents had no "NewFrameBehaviour" for footers/headers -> default to Copy.
    NewFrameBehaviour defaultValue = headerOrFooter ? Copy : Reconnect;
    newFrameBehaviour = static_cast<NewFrameBehaviour>( KWDocument::getAttribute( frameElem, "newFrameBehaviour", defaultValue ) );

    Border l, r, t, b;
    l.ptWidth = KWDocument::getAttribute( frameElem, "lWidth", 0.0 );
    r.ptWidth = KWDocument::getAttribute( frameElem, "rWidth", 0.0 );
    t.ptWidth = KWDocument::getAttribute( frameElem, "tWidth", 0.0 );
    b.ptWidth = KWDocument::getAttribute( frameElem, "bWidth", 0.0 );
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
    l.style = static_cast<Border::BorderStyle>( KWDocument::getAttribute( frameElem, "lStyle", Border::SOLID ) );
    r.style = static_cast<Border::BorderStyle>( KWDocument::getAttribute( frameElem, "rStyle", Border::SOLID ) );
    t.style = static_cast<Border::BorderStyle>( KWDocument::getAttribute( frameElem, "tStyle", Border::SOLID ) );
    b.style = static_cast<Border::BorderStyle>( KWDocument::getAttribute( frameElem, "bStyle", Border::SOLID ) );
    QColor c;
    if ( frameElem.hasAttribute("bkRed") )
        c.setRgb(
            KWDocument::getAttribute( frameElem, "bkRed", 0 ),
            KWDocument::getAttribute( frameElem, "bkGreen", 0 ),
            KWDocument::getAttribute( frameElem, "bkBlue", 0 ) );

    if ( syntaxVersion < 2 ) // Activate old "white border == no border" conversion
    {
        if(c==l.color && l.ptWidth==1 && l.style==0 )
            l.ptWidth=0;
        if(c==r.color  && r.ptWidth==1 && r.style==0)
            r.ptWidth=0;
        if(c==t.color && t.ptWidth==1 && t.style==0 )
            t.ptWidth=0;
        if(c==b.color && b.ptWidth==1 && b.style==0 )
            b.ptWidth=0;
    }
    brd_left = l;
    brd_right = r;
    brd_top = t;
    brd_bottom = b;
    backgroundColor = QBrush( c );
    bleft = frameElem.attribute( "bleftpt" ).toDouble();
    bright = frameElem.attribute( "brightpt" ).toDouble();
    btop = frameElem.attribute( "btoppt" ).toDouble();
    bbottom = frameElem.attribute( "bbottompt" ).toDouble();
    m_bCopy = KWDocument::getAttribute( frameElem, "copy", headerOrFooter /* default to true for h/f */ );
}

/******************************************************************/
/* Class: KWFrameSet                                              */
/******************************************************************/
KWFrameSet::KWFrameSet( KWDocument *doc )
    : m_doc( doc ), frames(), m_framesOnTop(), m_info( FI_BODY ),
      m_current( 0 ), grpMgr( 0L ), m_removeableHeader( false ), m_visible( true ),
      m_anchorTextFs( 0L ), m_currentDrawnCanvas( 0L )
{
    // Send our "repaintChanged" signals to the document.
    connect( this, SIGNAL( repaintChanged( KWFrameSet * ) ),
             doc, SLOT( slotRepaintChanged( KWFrameSet * ) ) );
    frames.setAutoDelete( true );
}

KWFrameSet::~KWFrameSet()
{
}

void KWFrameSet::addFrame( KWFrame *_frame, bool recalc )
{
    if ( frames.findRef( _frame ) != -1 )
        return;

    frames.append( _frame );
    _frame->setFrameSet(this);
    if(recalc)
        updateFrames();
}

void KWFrameSet::delFrame( unsigned int _num )
{
    KWFrame *frm = frames.at( _num );
    ASSERT( frm );
    delFrame(frm,true);
}

void KWFrameSet::delFrame( KWFrame *frm, bool remove )
{
    //kdDebug() << "KWFrameSet::delFrame " << frm << " " << remove << endl;
    int _num = frames.findRef( frm );
    ASSERT( _num != -1 );
    if ( _num == -1 )
        return;

    frm->setFrameSet(0L);
    if ( !remove )
    {
        frames.take( _num );
        if (frm->isSelected()) // get rid of the resize handles
            frm->setSelected(false);
    }
    else
        frames.remove( _num );

    updateFrames();
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
    //kdDebug() << "KWFrameSet::createEmptyRegion " << getName() << endl;
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
    {
        QRect outerRect( viewMode->normalToView( frameIt.current()->outerRect() ) );
        //kdDebug() << "KWFrameSet::createEmptyRegion outerRect=" << DEBUGRECT( outerRect ) << " crect=" << DEBUGRECT( crect ) << endl;
        outerRect &= crect; // This is important, to avoid calling subtract with a Y difference > 65536
        if ( !outerRect.isEmpty() )
        {
            emptyRegion = emptyRegion.subtract( outerRect );
            //kdDebug() << "KWFrameSet::createEmptyRegion emptyRegion now: " << endl; DEBUGREGION( emptyRegion );
        }
        if ( crect.bottom() + paperHeight < outerRect.top() )
            return; // Ok, we're far below the crect, abort.
    }
}

void KWFrameSet::drawFrameBorder( QPainter *painter, KWFrame *frame, KWFrame *settingsFrame, const QRect &crect, KWViewMode *viewMode, KWCanvas *canvas )
{
    QRect outerRect( viewMode->normalToView( frame->outerRect() ) );
    //kdDebug(32002) << "KWFrameSet::drawFrameBorder frame: " << frame
    //               << " outerRect: " << DEBUGRECT( outerRect ) << endl;

    if ( !crect.intersects( outerRect ) )
    {
        //kdDebug() << "KWFrameSet::drawFrameBorder no intersection with " << DEBUGRECT(crect) << endl;
        return;
    }

    QRect frameRect( viewMode->normalToView( m_doc->zoomRect(  *frame ) ) );

    painter->save();
    QBrush bgBrush( settingsFrame->getBackgroundColor() );
    bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), painter ) );
    painter->setBrush( bgBrush );

    // Draw default borders using view settings...
    QPen viewSetting( lightGray ); // TODO use qcolorgroup
    // ...except when printing, or embedded doc, or disabled.
    if ( ( painter->device()->devType() == QInternal::Printer ) ||
         !canvas || !canvas->gui()->getView()->viewFrameBorders() )
    {
        viewSetting.setColor( bgBrush.color() );
    }

    // Draw borders either as the user defined them, or using the view settings.
    // Borders should be drawn _outside_ of the frame area
    // otherwise the frames will erase the border when painting themselves.

    Border::drawBorders( *painter, m_doc, frameRect,
                         settingsFrame->leftBorder(), settingsFrame->rightBorder(),
                         settingsFrame->topBorder(), settingsFrame->bottomBorder(),
                         1, viewSetting );
    painter->restore();
}

void KWFrameSet::setFloating()
{
    // Find main text frame
    QListIterator<KWFrameSet> fit = m_doc->framesetsIterator();
    for ( ; fit.current() ; ++fit )
    {
        KWTextFrameSet * frameSet = dynamic_cast<KWTextFrameSet *>( fit.current() );
        if ( !frameSet || frameSet->frameSetInfo() != FI_BODY )
            continue;

        QTextParag* parag = 0L;
        int index = 0;
        QPoint cPoint( m_doc->zoomPoint( frames.first()->topLeft() ) );
        kdDebug() << "KWFrameSet::setFloating looking for pos at " << cPoint.x() << " " << cPoint.y() << endl;
        frameSet->findPosition( cPoint, parag, index );
        // Create anchor. TODO: refcount the anchors!
        setAnchored( frameSet, parag->paragId(), index );
        frameSet->layout();
        frames.first()->updateResizeHandles();
        m_doc->frameChanged(  frames.first() );
        return;
    }
}

void KWFrameSet::setAnchored( KWTextFrameSet* textfs, int paragId, int index, bool placeHolderExists /* = false */ )
{
    ASSERT( textfs );
    kdDebug() << "KWFrameSet::setAnchored " << textfs << " " << paragId << " " << index << " " << placeHolderExists << endl;
    if ( isFloating() )
        deleteAnchors();
    m_anchorTextFs = textfs;
    KWTextParag * parag = static_cast<KWTextParag *>( textfs->textDocument()->paragAt( paragId ) );
    ASSERT( parag );
    if ( parag )
        createAnchors( parag, index, placeHolderExists );
}

void KWFrameSet::setAnchored( KWTextFrameSet* textfs )
{
    m_anchorTextFs = textfs;
}

// Find where our anchor is ( if we are anchored ).
// We can't store a pointers to anchors, because over time we might change anchors
// (Especially, undo/redo of insert/delete can reuse an old anchor and forget a newer one etc.)
KWAnchor * KWFrameSet::findAnchor( int frameNum )
{
    ASSERT( m_anchorTextFs );
    QListIterator<QTextCustomItem> cit( m_anchorTextFs->textDocument()->allCustomItems() );
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
    kdDebug() << "KWFrameSet::setFixed" << endl;
    if ( isFloating() )
        deleteAnchors();
    m_anchorTextFs = 0L;
}

KWAnchor * KWFrameSet::createAnchor( KWTextDocument * textdoc, int frameNum )
{
    //KWFrame * frame = getFrame( frameNum );
    KWAnchor * anchor = new KWAnchor( textdoc, this, frameNum );
    //frame->setAnchor( anchor );
    return anchor;
}

void KWFrameSet::createAnchors( KWTextParag * parag, int index, bool placeHolderExists /*= false */ /*only used when loading*/ )
{
    kdDebug() << "KWFrameSet::createAnchors" << endl;
    ASSERT( m_anchorTextFs );
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt, ++index )
    {
        //if ( ! frameIt.current()->anchor() )
        {
            // Anchor this frame, after the previous one
            KWAnchor * anchor = createAnchor( m_anchorTextFs->textDocument(), getFrameFromPtr( frameIt.current() ) );
            if ( !placeHolderExists )
                parag->insert( index, KWTextFrameSet::customItemChar() );
            parag->setCustomItem( index, anchor, 0 );
        }
    }
    parag->setChanged( true );
    emit repaintChanged( m_anchorTextFs );
}

void KWFrameSet::deleteAnchor( KWAnchor * anchor )
{
    // Simple deletion, no undo/redo
    QTextCursor c( m_anchorTextFs->textDocument() );
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
    kdDebug() << "KWFrameSet::deleteAnchors" << endl;
    KWTextFrameSet * textfs = m_anchorTextFs;
    ASSERT( textfs );
    if ( !textfs )
        return;
    QListIterator<KWFrame> frameIt = frameIterator();
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

void KWFrameSet::moveFloatingFrame( int frameNum, const QPoint &position )
{
    KWFrame * frame = frames.at( frameNum );
    ASSERT( frame );
    if ( !frame ) return;

    QPoint pos( position );
    // position includes the border, we need to adjust accordingly
    pos.rx() += Border::zoomWidthX( frame->leftBorder().ptWidth, m_doc, 1 );
    pos.ry() += Border::zoomWidthY( frame->topBorder().ptWidth, m_doc, 1 );
    // Now we can unzoom
    KoPoint kopos = m_doc->unzoomPoint( pos );
    if ( frame->topLeft() != kopos )
    {
        kdDebug() << "KWFrameSet::moveFloatingFrame " << kopos.x() << "," << kopos.y() << endl;
        frame->moveTopLeft( kopos );
        kWordDocument()->updateAllFrames();
        if ( frame->isSelected() )
            frame->updateResizeHandles();
    }
}

QSize KWFrameSet::floatingFrameSize( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    ASSERT( frame );
    return frame->outerRect().size();
}

KCommand * KWFrameSet::anchoredObjectCreateCommand( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    ASSERT( frame );
    return new KWCreateFrameCommand( QString::null, frame );
}

KCommand * KWFrameSet::anchoredObjectDeleteCommand( int frameNum )
{
    KWFrame * frame = frames.at( frameNum );
    ASSERT( frame );
    return new KWDeleteFrameCommand( QString::null, frame );
}

KWFrame * KWFrameSet::frameByBorder( const QPoint & nPoint )
{
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
    {
        QRect outerRect( frameIt.current()->outerRect() );
        // Give the user a bit of margin for clicking on it :)
        const int margin = 2;
        outerRect.rLeft() -= margin;
        outerRect.rTop() -= margin;
        outerRect.rRight() += margin;
        outerRect.rBottom() += margin;
        if ( outerRect.contains( nPoint ) )
        {
            QRect innerRect( m_doc->zoomRect( *frameIt.current() ) );
            innerRect.rLeft() += margin;
            innerRect.rTop() += margin;
            innerRect.rRight() -= margin;
            innerRect.rBottom() -= margin;
            if ( !innerRect.contains( nPoint ) )
                return frameIt.current();
        }
    }
    return 0L;
}

KWFrame * KWFrameSet::frameAtPos( double _x, double _y )
{
    KoPoint docPoint( _x, _y );
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
        if ( frameIt.current()->contains( docPoint ) )
            return frameIt.current();
    return 0L;
}

KWFrame *KWFrameSet::getFrame( unsigned int _num )
{
    return frames.at( _num );
}

int KWFrameSet::getFrameFromPtr( KWFrame *frame )
{
    return frames.findRef( frame );
}

KWFrame * KWFrameSet::settingsFrame(KWFrame* frame)
{
    QListIterator<KWFrame> frameIt( frame->getFrameSet()->frameIterator() );
    if ( !frame->isCopy() )
        return frame;
    KWFrame* lastRealFrame=0L;
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame *curFrame = frameIt.current();
        if( curFrame == frame )
            return lastRealFrame ? lastRealFrame : frame;
        if ( !lastRealFrame || !curFrame->isCopy() )
            lastRealFrame = curFrame;
    }
    return frame; //fallback, should never happen
}

void KWFrameSet::updateFrames()
{
    m_framesOnTop.clear();

    // hack: table cells are not handled here, since they're not in the doc's frameset list.
    // ( so 'this' will never be found, and the whole method is useless )
    // TODO: hmm, well, store the "parent of this frameset", whether doc or frameset,
    // and look for the frameset list there. Hmm.
    if ( grpMgr )
        return;

    // Not visible ? Don't bother then.
    if ( !isVisible() )
        return;

    //kdDebug() << "KWFrameSet::updateFrames " << this << " " << getName() << endl;
    // Iterate over ALL framesets, to find those which have frames on top of us.
    // We'll use this information in various methods (adjust[LR]Margin, drawContents etc.)
    // So we want it cached.
    QListIterator<KWFrameSet> framesetIt( m_doc->framesetsIterator() );
    bool foundThis = false;
    for (; framesetIt.current(); ++framesetIt )
    {
        KWFrameSet *frameSet = framesetIt.current();

        if ( frameSet == this )
        {
            foundThis = true;
            continue;
        }

        if ( !foundThis || !frameSet->isVisible() )
            continue;

        // Floating frames are not "on top", they are "inside".
        if ( frameSet->isFloating() )
            continue;

        //kdDebug() << "KWFrameSet::updateFrames considering frameset " << frameSet << endl;

        QListIterator<KWFrame> frameIt( frameSet->frameIterator() );
        for ( ; frameIt.current(); ++frameIt )
        {
            KWFrame *frameOnTop = frameIt.current();
            // Is this frame over any of our frames ?
            QListIterator<KWFrame> fIt( frameIterator() );
            for ( ; fIt.current(); ++fIt )
            {
                KoRect intersect = fIt.current()->intersect( frameOnTop->outerKoRect() );
                if( !intersect.isEmpty() )
                {
                    //kdDebug() << "KWFrameSet::updateFrames adding frame on top " << DEBUGRECT(intersect)
                    //          << " (zoomed: " << DEBUGRECT( kWordDocument()->zoomRect( intersect ) ) << endl;
                    m_framesOnTop.append( FrameOnTop( intersect, frameOnTop ) );
                }
            }
        }
    }
    //kdDebug(32002) << "KWTextFrameSet " << this << " updateFrames() : frame on top:"
    //               << m_framesOnTop.count() << endl;

    if ( isFloating() )
    {
        //kdDebug() << "KWFrameSet::updateFrames " << getName() << " is floating" << endl;
        QListIterator<KWFrame> frameIt = frameIterator();
        int frameNum = 0;
        // At the moment there's only one anchor per frameset
        //for ( ; frameIt.current(); ++frameIt, ++frameNum )
        {
            KWAnchor * anchor = findAnchor( frameNum );
            //KWAnchor * anchor = frameIt.current()->anchor();
            //kdDebug() << "KWFrameSet::updateFrames anchor=" << anchor << endl;
            if ( anchor )
                anchor->resize();
        }
    }
}

void KWFrameSet::drawContents( QPainter *p, const QRect & crect, QColorGroup &cg,
                               bool onlyChanged, bool resetChanged,
                               KWFrameSetEdit *edit, KWViewMode *viewMode, KWCanvas *canvas )
{
    /* kdDebug(32002) << "KWFrameSet::drawContents " << this << " " << getName()
                   << " onlyChanged=" << onlyChanged << " resetChanged=" << resetChanged
                   << " crect= " << DEBUGRECT(crect)
                   << endl; */
    m_currentDrawnCanvas = canvas;
    bool drawBorders = ( getGroupManager() == 0 );

    QListIterator<KWFrame> frameIt( frameIterator() );
    KWFrame * lastRealFrame = 0L;
    int lastRealFrameTop = 0;
    int totalHeight = 0;
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame *frame = frameIt.current();
        if ( !frame->isValid() )
        {
            kdDebug(32002) << "KWFrameSet::drawContents invalid frame " << frame << endl;
            continue;
        }

        QRect r(crect);
        QRect normalFrameRect( m_doc->zoomRect( *frame ) );
        QRect frameRect( viewMode->normalToView( normalFrameRect ) );
        //kdDebug(32002) << "                    frame=" << frame << " crect=" << DEBUGRECT(r) << endl;
        r = r.intersect( frameRect );
        //kdDebug(32002) << "                    framerect=" << DEBUGRECT(*frame) << " intersec=" << DEBUGRECT(r) << " todraw=" << !r.isEmpty() << endl;
        if ( !r.isEmpty() )
        {
            // This translates the coordinates in the document contents
            // ( frame and r are up to here in this system )
            // into the QTextDocument's coordinate system
            // (which doesn't have frames, borders, etc.)
            int offsetX = normalFrameRect.left();
            int offsetY = normalFrameRect.top() - ( ( frame->isCopy() && lastRealFrame ) ? lastRealFrameTop : totalHeight );

            QRect icrect = viewMode->viewToNormal( r );
            //kdDebug() << "KWFrameSet::drawContents crect after view-to-normal:" << DEBUGRECT( icrect )  << endl;
            // y=-1 means all (in QRT), so let's not go there !
            //QPoint tl( QMAX( 0, icrect.left() - offsetX ), QMAX( 0, icrect.top() - offsetY ) );
            //icrect.moveTopLeft( tl );
            icrect.moveBy( -offsetX, -offsetY );
            ASSERT( icrect.x() >= 0 );
            ASSERT( icrect.y() >= 0 );

            // icrect is now the portion of the frame to be drawn, in qrt coords
            //kdDebug() << "KWFrameSet::drawContents in internal coords:" << DEBUGRECT( icrect ) << endl;

            // The settings come from this frame
            KWFrame * settingsFrame = ( frame->isCopy() && lastRealFrame ) ? lastRealFrame : frame;

            QRegion reg = frameClipRegion( p, frame, r, viewMode, onlyChanged );
            if ( !reg.isEmpty() )
            {
                p->save();
                p->setClipRegion( reg );
                p->translate( r.x() - icrect.x(), r.y() - icrect.y() ); // This assume that viewToNormal() is only a translation

                QBrush bgBrush( settingsFrame->getBackgroundColor() );
                bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), p ) );
                cg.setBrush( QColorGroup::Base, bgBrush );

                drawFrame( frame, p, icrect, cg, onlyChanged, resetChanged, edit );

                p->restore();
            }
        }
        if ( drawBorders )
        {
            QRect outerRect( viewMode->normalToView( frame->outerRect() ) );
            r = crect.intersect( outerRect );
            if ( !r.isEmpty() )
            {
                // Now draw the frame border
                // Clip frames on top if onlyChanged, but don't clip to the frame
                QRegion reg = frameClipRegion( p, 0L, r, viewMode, onlyChanged );
                if ( !reg.isEmpty() )
                {
                    p->save();
                    p->setClipRegion( reg );
                    KWFrame * settingsFrame = ( frame->isCopy() && lastRealFrame ) ? lastRealFrame : frame;
                    drawFrameBorder( p, frame, settingsFrame, r, viewMode, canvas );
                    p->restore();
                }// else kdDebug() << "KWFrameSet::drawContents not drawing border for frame " << frame << endl;
            }
        }
        if ( !lastRealFrame || !frame->isCopy() )
        {
            lastRealFrame = frame;
            lastRealFrameTop = totalHeight;
        }
        totalHeight += normalFrameRect.height();
    }
    m_currentDrawnCanvas = 0L;
}

bool KWFrameSet::contains( double mx, double my )
{
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
        if ( frameIt.current()->contains( KoPoint( mx, my ) ) )
            return true;

    return false;
}

bool KWFrameSet::getMouseCursor( const QPoint &nPoint, bool controlPressed, QCursor & cursor )
{
    bool canMove = isMoveable();
    KoPoint docPoint = m_doc->unzoomPoint( nPoint );
    QCursor defaultCursor = ( canMove && !isFloating() ) ? Qt::sizeAllCursor : KCursor::handCursor();
    // See if we're over a frame border
    KWFrame * frame = frameByBorder( nPoint );
    if ( frame )
    {
        cursor = frame->getMouseCursor( docPoint, grpMgr ? true : false, defaultCursor );
        return true;
    }

    frame = frameAtPos( docPoint.x(), docPoint.y() );
    if ( frame == 0L )
        return false;

    if ( controlPressed )
        cursor = defaultCursor;
    else
        cursor = frame->getMouseCursor( docPoint, grpMgr ? true : false, Qt::ibeamCursor );
    return true;
}

void KWFrameSet::saveCommon( QDomElement &parentElem, bool saveFrames )
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return;

    // Save all the common attributes for framesets.
    parentElem.setAttribute( "frameType", static_cast<int>( type() ) );
    parentElem.setAttribute( "frameInfo", static_cast<int>( m_info ) );
    parentElem.setAttribute( "name", correctQString( m_name ) );
    parentElem.setAttribute( "visible", static_cast<int>( m_visible ) );

    if ( saveFrames )
    {
        QListIterator<KWFrame> frameIt = frameIterator();
        for ( ; frameIt.current(); ++frameIt )
        {
            KWFrame *frame = frameIt.current();
            QDomElement frameElem = parentElem.ownerDocument().createElement( "FRAME" );
            parentElem.appendChild( frameElem );

            frame->save( frameElem );

            if(m_doc->processingType() == KWDocument::WP) {
                if(m_doc->getFrameSet(0) == this) break;
                if(frameSetInfo() == FI_FIRST_HEADER ||
                   frameSetInfo() == FI_ODD_HEADER ||
                   frameSetInfo() == FI_EVEN_HEADER ||
                   frameSetInfo() == FI_FIRST_FOOTER ||
                   frameSetInfo() == FI_ODD_FOOTER ||
                   frameSetInfo() == FI_EVEN_FOOTER ||
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
                frame->load( frameElem, isHeaderOrFooter(), m_doc->syntaxVersion() );
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

bool KWFrameSet::isVisible() const
{
    return ( m_visible &&
             !frames.isEmpty() &&
             (!isAHeader() || m_doc->isHeaderVisible()) &&
             (!isAFooter() || m_doc->isFooterVisible()) &&
             !isAWrongHeader( m_doc->getHeaderType() ) &&
             !isAWrongFooter( m_doc->getFooterType() ) );
}

bool KWFrameSet::isAHeader() const
{
    return ( m_info == FI_FIRST_HEADER || m_info == FI_EVEN_HEADER || m_info == FI_ODD_HEADER );
}

bool KWFrameSet::isAFooter() const
{
    return ( m_info == FI_FIRST_FOOTER || m_info == FI_EVEN_FOOTER || m_info == FI_ODD_FOOTER );
}

bool KWFrameSet::isAWrongHeader( KoHFType t ) const
{
    switch ( m_info ) {
    case FI_FIRST_HEADER: {
        if ( t == HF_FIRST_DIFF ) return false;
        return true;
    } break;
    case FI_EVEN_HEADER: {
        return false;
    } break;
    case FI_ODD_HEADER: {
        if ( t == HF_EO_DIFF ) return false;
        return true;
    } break;
    default: return false;
    }
}

bool KWFrameSet::isAWrongFooter( KoHFType t ) const
{
    switch ( m_info ) {
    case FI_FIRST_FOOTER: {
        if ( t == HF_FIRST_DIFF ) return false;
        return true;
    } break;
    case FI_EVEN_FOOTER: {
        return false;
    } break;
    case FI_ODD_FOOTER: {
        if ( t == HF_EO_DIFF ) return false;
        return true;
    } break;
    default: return false;
    }
}

bool KWFrameSet::isMainFrameset() const
{
    return ( m_doc->processingType() == KWDocument::WP &&
             m_doc->getFrameSet( 0 ) == this );
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
    //kdDebug() << "KWFrameSet::finalize ( calls updateFrames + zoom ) " << this << endl;
    updateFrames();
    zoom( false );
}

// This determines where to clip the painter to draw the contents of a given frame
// It clips to the frame, and clips out any "on top" frame if onlyChanged=true.
QRegion KWFrameSet::frameClipRegion( QPainter * painter, KWFrame *frame, const QRect & crect,
                                     KWViewMode * viewMode, bool onlyChanged )
{
    KWDocument * doc = kWordDocument();
    QRect rc = painter->xForm( crect );
    if ( frame )
    {
        //kdDebug(32002) << "KWFrameSet::frameClipRegion rc initially " << DEBUGRECT(rc) << endl;
        rc &= painter->xForm( viewMode->normalToView( doc->zoomRect( *frame ) ) ); // intersect
        //kdDebug(32002) << "KWFrameSet::frameClipRegion frame=" << DEBUGRECT(*frame)
        //               << " clip region rect=" << DEBUGRECT(rc)
        //               << " rc.isEmpty()=" << rc.isEmpty() << endl;
    }
    if ( !rc.isEmpty() )
    {
        QRegion reg( rc );
        if ( onlyChanged )
        {
            QValueListIterator<FrameOnTop> fIt = m_framesOnTop.begin();
            for ( ; fIt != m_framesOnTop.end() ; ++fIt )
            {
                QRect r = painter->xForm( viewMode->normalToView( (*fIt).frame->outerRect() ) );
                //kdDebug(32002) << "frameClipRegion subtract rect "<< DEBUGRECT(r) << endl;
                reg -= r; // subtract
            }
        }
        return reg;
    } else return QRegion();
}

bool KWFrameSet::canRemovePage( int num )
{
    QListIterator<KWFrame> frameIt( frameIterator() );
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame * frame = frameIt.current();
        if ( frame->pageNum() == num )
        {
            // Ok, so we have a frame on that page -> we can't remove it unless it's a copied frame
            if ( ! ( frame->isCopy() && frameIt.current() != frames.first() ) )
                return false;
        }
    }
    return true;
}

#ifndef NDEBUG
void KWFrameSet::printDebug()
{
    static const char * typeFrameset[] = { "base", "txt", "pic", "part", "formula", "clipart",
                                           "6", "7", "8", "9", "table",
                                           "ERROR" };
    static const char * infoFrameset[] = { "body", "first header", "odd headers", "even headers",
                                           "first footer", "odd footers", "even footers", "footnote", "ERROR" };
    static const char * frameBh[] = { "AutoExtendFrame", "AutoCreateNewFrame", "Ignore", "ERROR" };
    static const char * newFrameBh[] = { "Reconnect", "NoFollowup", "Copy" };
    static const char * runaround[] = { "No Runaround", "Bounding Rect", "Horizontal Space", "ERROR" };

    kdDebug() << " |  Visible: " << isVisible() << endl;
    kdDebug() << " |  Type: " << typeFrameset[ type() ] << endl;
    kdDebug() << " |  Info: " << infoFrameset[ frameSetInfo() ] << endl;
    kdDebug() << " |  Floating: " << isFloating() << endl;
    kdDebug() << " |  Number of frames on top: " << m_framesOnTop.count() << endl;

    QListIterator<KWFrame> frameIt = frameIterator();
    for ( unsigned int j = 0; frameIt.current(); ++frameIt, ++j ) {
        KWFrame * frame = frameIt.current();
        QCString copy = frame->isCopy() ? "[copy]" : "";
        kdDebug() << " +-- Frame " << j << " of "<< getNumFrames() << "    (" << frame << ")  " << copy << endl;
        printDebug( frame );
        kdDebug() << "     Rectangle : " << frame->x() << "," << frame->y() << " " << frame->width() << "x" << frame->height() << endl;
        kdDebug() << "     RunAround: "<< runaround[ frame->runAround() ] << endl;
        kdDebug() << "     FrameBehaviour: "<< frameBh[ frame->getFrameBehaviour() ] << endl;
        kdDebug() << "     NewFrameBehaviour: "<< newFrameBh[ frame->getNewFrameBehaviour() ] << endl;
        QColor col = frame->getBackgroundColor().color();
        kdDebug() << "     BackgroundColor: "<< ( col.isValid() ? col.name().latin1() : "(default)" ) << endl;
        kdDebug() << "     SheetSide "<< frame->getSheetSide() << endl;
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
     : m_fs(fs), m_canvas(canvas), m_currentFrame( fs->getFrame(0) )
{
}

void KWFrameSetEdit::drawContents( QPainter *p, const QRect &crect,
                                   QColorGroup &cg, bool onlyChanged, bool resetChanged,
                                   KWViewMode *viewMode, KWCanvas *canvas )
{
    //kdDebug() << "KWFrameSetEdit::drawContents " << frameSet()->getName() << endl;
    frameSet()->drawContents( p, crect, cg, onlyChanged, resetChanged, this, viewMode, canvas );
}

/******************************************************************/
/* Class: KWPictureFrameSet                                       */
/******************************************************************/
KWPictureFrameSet::KWPictureFrameSet( KWDocument *_doc, const QString & name )
    : KWFrameSet( _doc )
{
    if ( name.isEmpty() )
        m_name = _doc->generateFramesetName( i18n( "Picture %1" ) );
    else
        m_name = name;
    m_keepAspectRatio = true;
}

KWPictureFrameSet::~KWPictureFrameSet() {
}

void KWPictureFrameSet::loadImage( const QString & fileName, const QSize &_imgSize )
{
    KWImageCollection *collection = m_doc->imageCollection();

    m_image = collection->loadImage( fileName );

    m_image = m_image.scale( _imgSize );
}

void KWPictureFrameSet::setSize( const QSize & _imgSize )
{
    m_image = m_image.scale( _imgSize );
}

QDomElement KWPictureFrameSet::save( QDomElement & parentElem, bool saveFrames )
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return QDomElement();
    QDomElement framesetElem = parentElem.ownerDocument().createElement( "FRAMESET" );
    parentElem.appendChild( framesetElem );

    KWFrameSet::saveCommon( framesetElem, saveFrames );

    QDomElement imageElem = parentElem.ownerDocument().createElement( "IMAGE" );
    framesetElem.appendChild( imageElem );
    imageElem.setAttribute( "keepAspectRatio", m_keepAspectRatio ? "true" : "false" );
    QDomElement elem = parentElem.ownerDocument().createElement( "KEY" );
    imageElem.appendChild( elem );
    m_image.key().saveAttributes( elem );
    return framesetElem;
}

void KWPictureFrameSet::load( QDomElement &attributes, bool loadFrames )
{
    KWFrameSet::load( attributes, loadFrames );

    // <IMAGE>
    QDomElement image = attributes.namedItem( "IMAGE" ).toElement();
    if ( !image.isNull() ) {
        m_keepAspectRatio = image.attribute( "keepAspectRatio", "true" ) == "true";
        // <KEY>
        QDomElement keyElement = image.namedItem( "KEY" ).toElement();
        if ( !keyElement.isNull() )
        {
            KoImageKey key;
            key.loadAttributes( keyElement, QDate(), QTime() );
            m_image = KWImage( key, QImage() );
            m_doc->addImageRequest( this );
        }
        else
        {
            // <FILENAME> (old format, up to KWord-1.1-beta2)
            QDomElement filenameElement = image.namedItem( "FILENAME" ).toElement();
            if ( !filenameElement.isNull() )
            {
                QString filename = filenameElement.attribute( "value" );
                m_image = KWImage( KoImageKey( filename, QDateTime::currentDateTime() ), QImage() );
                m_doc->addImageRequest( this );
            }
            else
            {
                kdError(32001) << "Missing KEY tag in IMAGE" << endl;
            }
        }
    } else {
        kdError(32001) << "Missing IMAGE tag in FRAMESET" << endl;
    }
}

void KWPictureFrameSet::drawFrame( KWFrame *frame, QPainter *painter, const QRect &,
                                   QColorGroup &, bool, bool, KWFrameSetEdit * )
{
    QSize screenSize ( kWordDocument()->zoomItX( frame->width() ), kWordDocument()->zoomItY( frame->height() ) );

    bool scaleImage =  painter->device()->isExtDev()
                      && ( m_image.size().width()<m_image.originalSize().width()
                           || m_image.size().height()<m_image.originalSize().height() );
    if( scaleImage ) {
        if( m_image.size().width()>0 && m_image.size().height()>0 ) {

            // use full resolution of image
            double xScale = double(screenSize.width())/m_image.originalSize().width();
            double yScale = double(screenSize.height())/m_image.originalSize().height();

            if( xScale>0 && yScale>0 ) {
                m_image = m_image.scale( m_image.originalSize() );
                painter->scale( xScale, yScale );
                painter->drawPixmap( 0, 0, m_image.pixmap() );
                painter->scale( 1/xScale, 1/yScale );
            }

        }
    } else {
        if ( screenSize!=m_image.image().size() )
            m_image = m_image.scale( screenSize );
        painter->drawPixmap( 0, 0, m_image.pixmap() );
    }

}

KWFrame *KWPictureFrameSet::frameByBorder( const QPoint & nPoint )
{
    // For pictures/cliparts there is nothing to do when clicking
    // inside the frame, so the whole frame is a 'border' (clicking in it selects the frame)
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
    {
        QRect outerRect( frameIt.current()->outerRect() );
        if ( outerRect.contains( nPoint ) )
            return frameIt.current();
    }
    return 0L;
}

/******************************************************************/
/* Class: KWClipartFrameSet                                       */
/******************************************************************/
KWClipartFrameSet::KWClipartFrameSet( KWDocument *_doc, const QString & name )
    : KWFrameSet( _doc )
{
    if ( name.isEmpty() )
        m_name = _doc->generateFramesetName( i18n( "Clipart %1" ) );
    else
        m_name = name;
}

void KWClipartFrameSet::loadClipart( const QString & fileName )
{
    KoClipartCollection *collection = m_doc->clipartCollection();
    m_clipart = collection->loadClipart( fileName );
}

QDomElement KWClipartFrameSet::save( QDomElement & parentElem, bool saveFrames )
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return QDomElement();
    QDomElement framesetElem = parentElem.ownerDocument().createElement( "FRAMESET" );
    parentElem.appendChild( framesetElem );

    KWFrameSet::saveCommon( framesetElem, saveFrames );

    QDomElement imageElem = parentElem.ownerDocument().createElement( "CLIPART" );
    framesetElem.appendChild( imageElem );
    QDomElement elem = parentElem.ownerDocument().createElement( "KEY" );
    imageElem.appendChild( elem );
    m_clipart.key().saveAttributes( elem );
    return framesetElem;
}

void KWClipartFrameSet::load( QDomElement &attributes, bool loadFrames )
{
    KWFrameSet::load( attributes, loadFrames );

    // <CLIPART>
    QDomElement image = attributes.namedItem( "CLIPART" ).toElement();
    if ( !image.isNull() ) {
        // <KEY>
        QDomElement keyElement = image.namedItem( "KEY" ).toElement();
        if ( !keyElement.isNull() )
        {
            KoClipartKey key;
            key.loadAttributes( keyElement, QDate(), QTime() );
            m_clipart = KoClipart( key, QPicture() );
            m_doc->addClipartRequest( this );
        }
        else
            kdError(32001) << "Missing KEY tag in CLIPART" << endl;
    } else
        kdError(32001) << "Missing CLIPART tag in FRAMESET" << endl;
}

void KWClipartFrameSet::drawFrame( KWFrame *frame, QPainter *painter, const QRect &,
                                   QColorGroup &, bool, bool, KWFrameSetEdit * )
{
    if ( m_clipart.isNull() )
        return;
    QSize s ( kWordDocument()->zoomItX( frame->width() ), kWordDocument()->zoomItY( frame->height() ) );
    painter->save();
    QRect vp( 0, 0, s.width(), s.height() );
    painter->setViewport( painter->worldMatrix().map( vp ) ); // stolen from kontour
    painter->setWorldMatrix (QWMatrix ());
    painter->drawPicture( *m_clipart.picture() );
    painter->restore();
}

KWFrame *KWClipartFrameSet::frameByBorder( const QPoint & nPoint )
{
    // For pictures/cliparts there is nothing to do when clicking
    // inside the frame, so the whole frame is a 'border' (clicking in it selects the frame)
    QListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
    {
        QRect outerRect( frameIt.current()->outerRect() );
        if ( outerRect.contains( nPoint ) )
            return frameIt.current();
    }
    return 0L;
}

/******************************************************************/
/* Class: KWPartFrameSet                                          */
/******************************************************************/
KWPartFrameSet::KWPartFrameSet( KWDocument *_doc, KWChild *_child, const QString & name )
    : KWFrameSet( _doc )
{
    m_child = _child;
    kdDebug() << "KWPartFrameSet::KWPartFrameSet" << endl;
    if ( name.isEmpty() )
        m_name = _doc->generateFramesetName( i18n( "Object %1" ) );
    else
        m_name = name;
}

KWPartFrameSet::~KWPartFrameSet()
{
}

void KWPartFrameSet::drawFrame( KWFrame* frame, QPainter * painter, const QRect & /*crect TODO*/,
                                QColorGroup &, bool onlyChanged, bool, KWFrameSetEdit * )
{
    if (!onlyChanged)
    {
        if ( !m_child || !m_child->document() )
        {
            kdDebug() << "KWPartFrameSet::drawFrame " << this << " aborting. child=" << m_child << " child->document()=" << m_child->document() << endl;
            return;
        }

        // We have to define better what the rect that we pass, means. Does it include zooming ? (yes I think)
        // Does it define the area to be repainted only ? (here it doesn't, really, but it should)
        QRect rframe( 0, 0, kWordDocument()->zoomItX( frame->width() ),
                      kWordDocument()->zoomItY( frame->height() ) );
        //kdDebug() << "rframe=" << DEBUGRECT( rframe ) << endl;

        m_child->document()->paintEverything( *painter, rframe, true, 0L,
                                            kWordDocument()->zoomedResolutionX(), kWordDocument()->zoomedResolutionY() );

    } //else kdDebug() << "KWPartFrameSet::drawFrame " << this << " onlychanged=true!" << endl;
}

void KWPartFrameSet::updateFrames()
{
    if( frames.isEmpty() ) // Deleted frameset -> don't refresh
        return;
    KWFrameSet::updateFrames();
}

void KWPartFrameSet::updateChildGeometry()
{
    if( frames.isEmpty() ) // Deleted frameset
        return;
    // Set the child geometry from the frame geometry, with no viewmode applied
    // This is necessary e.g. before saving, but shouldn't be done while the part
    // is being activated.
    m_child->setGeometry( frames.first()->toQRect() );
}

QDomElement KWPartFrameSet::save( QDomElement &parentElem, bool saveFrames )
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return QDomElement();
    KWFrameSet::saveCommon( parentElem, saveFrames );
    // Ok, this one is a bit hackish. KWDocument calls us for saving our stuff into
    // the SETTINGS element, which it creates for us. So our save() doesn't really have
    // the same behaviour as a normal KWFrameSet::save()....
    return QDomElement();
}

void KWPartFrameSet::load( QDomElement &attributes, bool loadFrames )
{
    KWFrameSet::load( attributes, loadFrames );
}


KWFrameSetEdit * KWPartFrameSet::createFrameSetEdit( KWCanvas * canvas )
{
    return new KWPartFrameSetEdit( this, canvas );
}

KWPartFrameSetEdit::KWPartFrameSetEdit( KWPartFrameSet * fs, KWCanvas * canvas )
    : KWFrameSetEdit( fs, canvas )
{
    m_cmdMoveChild=0L;
    QObject::connect( partFrameSet()->getChild(), SIGNAL( changed( KoChild * ) ),
                      this, SLOT( slotChildChanged() ) );
    QObject::connect( m_canvas->gui()->getView() ,SIGNAL(activated( bool ))
                      ,this,SLOT(slotChildActivated(bool) ) );
}

KWPartFrameSetEdit::~KWPartFrameSetEdit()
{
    kdDebug() << "KWPartFrameSetEdit::~KWPartFrameSetEdit" << endl;
}


void KWPartFrameSetEdit::slotChildActivated(bool b)
{
    //we store command when we desactivate child.
    if( b)
        return;
    if(m_cmdMoveChild && m_cmdMoveChild->frameMoved())
        partFrameSet()->kWordDocument()->addCommand(m_cmdMoveChild);
    else
        delete m_cmdMoveChild;
    m_cmdMoveChild=0L;
}

void KWPartFrameSetEdit::slotChildChanged()
{
    // This is called when the KoDocumentChild is resized (using the KoFrame)
    // We need to react on it in KWPartFrameSetEdit because the view-mode has to be taken into account
    QListIterator<KWFrame>listFrame=partFrameSet()->frameIterator();
    KWFrame *frame = listFrame.current();
    if ( frame  )
    {
        // We need to apply the viewmode conversion correctly (the child is in unzoomed view coords!)
        KoRect childGeom = KoRect::fromQRect( partFrameSet()->getChild()->geometry() );
        // r is the rect in normal coordinates
        QRect r(m_canvas->viewMode()->viewToNormal( frameSet()->kWordDocument()->zoomRect( childGeom ) ) );
        frame->setLeft( r.left() / frameSet()->kWordDocument()->zoomedResolutionX() );
        frame->setTop( r.top() / frameSet()->kWordDocument()->zoomedResolutionY() );
        frame->setWidth( r.width() / frameSet()->kWordDocument()->zoomedResolutionX() );
        frame->setHeight( r.height() / frameSet()->kWordDocument()->zoomedResolutionY() );
        // ## TODO an undo/redo command (move frame)

        kdDebug() << "KWPartFrameSet::slotChildChanged child's geometry " << DEBUGRECT(partFrameSet()->getChild()->geometry() )
                  << " frame set to " << DEBUGRECT( *frame ) << endl;
        partFrameSet()->kWordDocument()->frameChanged( frame );
        //there is just a frame
        if(m_cmdMoveChild)
            m_cmdMoveChild->listFrameMoved().sizeOfEnd = frame->normalize();
    }
}

void KWPartFrameSetEdit::mousePressEvent( QMouseEvent *e, const QPoint &, const KoPoint & )
{
    if ( e->button() != Qt::LeftButton )
        return;

    // activate child part
    partFrameSet()->updateFrames();
    QListIterator<KWFrame>listFrame = partFrameSet()->frameIterator();
    KWFrame *frame = listFrame.current();
    // Set the child geometry from the frame geometry, applying the viewmode
    // (the child is in unzoomed view coords!)
    QRect r( m_canvas->viewMode()->normalToView( frameSet()->kWordDocument()->zoomRect( *frame ) ) );
    partFrameSet()->getChild()->setGeometry( frameSet()->kWordDocument()->unzoomRect( r ).toQRect() );
    kdDebug() << "KWPartFrameSetEdit: activating. child set to " << DEBUGRECT( partFrameSet()->getChild()->geometry() ) << endl;

    KoDocument* part = partFrameSet()->getChild()->document();
    if ( !part )
        return;
    KWView * view = m_canvas->gui()->getView();
    kdDebug() << "Child activated. part=" << part << " child=" << partFrameSet()->getChild() << endl;
    view->partManager()->addPart( part, false );
    view->partManager()->setActivePart( part, view );

    //create undo/redo move command
    FrameIndex index( frame );
    FrameResizeStruct tmpMove;
    tmpMove.sizeOfBegin = frame->normalize();
    tmpMove.sizeOfEnd = KoRect();

    if(!m_cmdMoveChild)
        m_cmdMoveChild=new KWFramePartMoveCommand( i18n("Move Frame"), index, tmpMove );
}

void KWPartFrameSetEdit::mouseDoubleClickEvent( QMouseEvent *, const QPoint &, const KoPoint & )
{
    /// ## Pretty useless since single-click does it now...
    //activate( m_canvas->gui()->getView() );
}


using namespace KFormula;

/******************************************************************/
/* Class: KWFormulaFrameSet                                       */
/******************************************************************/
KWFormulaFrameSet::KWFormulaFrameSet( KWDocument *_doc, const QString & name )
    : KWFrameSet( _doc ), m_changed( false )
{
    formula = _doc->getFormulaDocument()->createFormula();
    // With the new drawing scheme (drawFrame being called with translated painter)
    // there is no need to move the KFormulaContainer anymore, it remains at (0,0).
    formula->moveTo( 0, 0 );

    connect(formula, SIGNAL(formulaChanged(int, int)),
            this, SLOT(slotFormulaChanged(int, int)));
    if ( name.isEmpty() )
        m_name = _doc->generateFramesetName( i18n( "Formula %1" ) );
    else
        m_name = name;

    /*
    if ( isFloating() ) {
        // we need to look for the anchor every time, don't cache this value.
        // undo/redo creates/deletes anchors
        KWAnchor * anchor = findAnchor( 0 );
        if ( anchor ) {
            QTextFormat * format = anchor->format();
            formula->setFontSize( format->font().pointSize() );
        }
    }
    */
}


KWFormulaFrameSet::~KWFormulaFrameSet()
{
    delete formula;
}

KWFrameSetEdit* KWFormulaFrameSet::createFrameSetEdit(KWCanvas* canvas)
{
    return new KWFormulaFrameSetEdit(this, canvas);
}

void KWFormulaFrameSet::drawFrame( KWFrame* /*frame*/, QPainter* painter, const QRect& crect,
                                   QColorGroup& cg, bool onlyChanged, bool resetChanged,
                                   KWFrameSetEdit *edit )
{
    //kdDebug() << "KWFormulaFrameSet::drawFrame m_changed=" << m_changed << " onlyChanged=" << onlyChanged << endl;
    if ( m_changed || !onlyChanged )
    {
        if ( resetChanged )
            m_changed = false;

        /*
          It's a bad idea to change the formula font size while drawing.
          We need to be notified as soon as the change occurs. But this will
          have to wait.
        if ( isFloating() ) {
            // we need to look for the anchor every time, don't cache this value.
            // undo/redo creates/deletes anchors
            KWAnchor * anchor = findAnchor( 0 );
            if ( anchor ) {
                QTextFormat * format = anchor->format();
                bool forPrint = painter->device()->devType() != QInternal::Printer;
                formula->setFontSize( format->font().pointSize(), forPrint );
            }
        }
        */

        if ( edit )
        {
            KWFormulaFrameSetEdit * formulaEdit = static_cast<KWFormulaFrameSetEdit *>(edit);
            formulaEdit->getFormulaView()->draw( *painter, crect, cg );
        }
        else
        {
            //kdDebug() << "KWFormulaFrameSet::drawFrame drawing (without edit) crect=" << DEBUGRECT( crect ) << endl;
            formula->draw( *painter, crect, cg );
        }
    }
}

void KWFormulaFrameSet::slotFormulaChanged( int width, int height )
{
    if ( frames.isEmpty() )
        return;

    int newWidth = static_cast<int>( static_cast<double>( width ) /
                                     kWordDocument()->zoomedResolutionX() ) + 2;
    int newHeight = static_cast<int>( static_cast<double>( height ) /
                                      kWordDocument()->zoomedResolutionY() ) + 1;

    double oldWidth = frames.first()->width();
    double oldHeight = frames.first()->height();

    frames.first()->setWidth( newWidth );
    frames.first()->setHeight( newHeight );

    updateFrames();
    kWordDocument()->layout();
    if ( ( oldWidth != newWidth ) || ( oldHeight != newHeight ) ) {
        kWordDocument()->repaintAllViews( false );
        kWordDocument()->updateRulerFrameStartEnd();
    }

    m_changed = true;
    emit repaintChanged( this );
}

void KWFormulaFrameSet::updateFrames()
{
    KWFrameSet::updateFrames();
}

QDomElement KWFormulaFrameSet::save(QDomElement& parentElem, bool saveFrames)
{
    if ( frames.isEmpty() ) // Deleted frameset -> don't save
        return QDomElement();
    QDomElement framesetElem = parentElem.ownerDocument().createElement("FRAMESET");
    parentElem.appendChild(framesetElem);

    KWFrameSet::saveCommon(framesetElem, saveFrames);

    QDomElement formulaElem = parentElem.ownerDocument().createElement("FORMULA");
    framesetElem.appendChild(formulaElem);
    formula->save(formulaElem);
    return framesetElem;
}

void KWFormulaFrameSet::load(QDomElement& attributes, bool loadFrames)
{
    KWFrameSet::load(attributes, loadFrames);
    QDomElement formulaElem = attributes.namedItem("FORMULA").toElement();
    if (!formulaElem.isNull()) {
        if (formula == 0) {
            formula = m_doc->getFormulaDocument()->createFormula();
            connect(formula, SIGNAL(formulaChanged(int, int)),
                    this, SLOT(slotFormulaChanged(int, int)));
        }
        if (!formula->load(formulaElem)) {
            kdError(32001) << "Error loading formula" << endl;
        }
    }
    else {
        kdError(32001) << "Missing FORMULA tag in FRAMESET" << endl;
    }
}

void KWFormulaFrameSet::zoom( bool forPrint )
{
    if ( !frames.isEmpty() )
    {
        formula->recalc();
        KWFrameSet::zoom( forPrint );
    }
}

int KWFormulaFrameSet::floatingFrameBaseline( int /*frameNum*/ )
{
    if ( !frames.isEmpty() )
    {
        return formula->baseline();
    }
    return -1;
}


KWFormulaFrameSetEdit::KWFormulaFrameSetEdit(KWFormulaFrameSet* fs, KWCanvas* canvas)
        : KWFrameSetEdit(fs, canvas)
{
    kdDebug(32001) << "KWFormulaFrameSetEdit::KWFormulaFrameSetEdit" << endl;
    formulaView = new KFormulaView(fs->getFormula());
    //formulaView->setSmallCursor(true);

    connect( formulaView, SIGNAL( cursorChanged( bool, bool ) ),
             this, SLOT( cursorChanged( bool, bool ) ) );

    m_canvas->gui()->getView()->showFormulaToolbar(true);
    focusInEvent();
}

KWFormulaFrameSetEdit::~KWFormulaFrameSetEdit()
{
    kdDebug(32001) << "KWFormulaFrameSetEdit::~KWFormulaFrameSetEdit" << endl;
    focusOutEvent();
    m_canvas->gui()->getView()->showFormulaToolbar(false);
    delete formulaView;
}

void KWFormulaFrameSetEdit::keyPressEvent( QKeyEvent* event )
{
    //kdDebug(32001) << "KWFormulaFrameSetEdit::keyPressEvent" << endl;
    int action = event->key();
    if ( event->state() == 0 ) {
        switch ( action ) {
        case Qt::Key_Left:
            if ( formulaView->isHome() ) {
                // leave left
                return;
            }
            break;
        case Qt::Key_Right:
            if ( formulaView->isEnd() ) {
                // leave right
                return;
            }
            break;
        }
    }
    formulaView->keyPressEvent( event );;
}

void KWFormulaFrameSetEdit::mousePressEvent(QMouseEvent* event, const QPoint &, const KoPoint & )
{
    // [Note that this method is called upon RMB and MMB as well, now]
    QPoint nPoint = frameSet()->kWordDocument()->zoomPoint( m_currentFrame->topLeft() );
    nPoint.setX( event->pos().x()-nPoint.x() );
    nPoint.setY( event->pos().y()-nPoint.y() );
    QMouseEvent e( event->type(), nPoint, event->button(), event->state() );
    formulaView->mousePressEvent( &e );
}

void KWFormulaFrameSetEdit::mouseMoveEvent(QMouseEvent* event, const QPoint &, const KoPoint & )
{
    QPoint nPoint = frameSet()->kWordDocument()->zoomPoint( m_currentFrame->topLeft() );
    nPoint.setX( event->pos().x()-nPoint.x() );
    nPoint.setY( event->pos().y()-nPoint.y() );
    QMouseEvent e( event->type(), nPoint, event->button(), event->state() );
    formulaView->mouseMoveEvent( &e );
}

void KWFormulaFrameSetEdit::mouseReleaseEvent(QMouseEvent* event, const QPoint &, const KoPoint & )
{
    QPoint nPoint = frameSet()->kWordDocument()->zoomPoint( m_currentFrame->topLeft() );
    nPoint.setX( event->pos().x()-nPoint.x() );
    nPoint.setY( event->pos().y()-nPoint.y() );
    QMouseEvent e( event->type(), nPoint, event->button(), event->state() );
    formulaView->mouseReleaseEvent( &e );
}

void KWFormulaFrameSetEdit::focusInEvent()
{
    //kdDebug(32001) << "KWFormulaFrameSetEdit::focusInEvent" << endl;
    formulaView->focusInEvent(0);
}

void KWFormulaFrameSetEdit::focusOutEvent()
{
    //kdDebug(32001) << "KWFormulaFrameSetEdit::focusOutEvent" << endl;
    formulaView->focusOutEvent(0);
}

void KWFormulaFrameSetEdit::copy()
{
    formulaView->getDocument()->copy();
}

void KWFormulaFrameSetEdit::cut()
{
    formulaView->getDocument()->cut();
}

void KWFormulaFrameSetEdit::paste()
{
    formulaView->getDocument()->paste();
}

void KWFormulaFrameSetEdit::selectAll()
{
    formulaView->slotSelectAll();
}

void KWFormulaFrameSetEdit::moveHome()
{
    formulaView->slotMoveHome( WordMovement );
}
void KWFormulaFrameSetEdit::moveEnd()
{
    formulaView->slotMoveEnd( WordMovement );
}

void KWFormulaFrameSetEdit::cursorChanged( bool visible, bool /*selecting*/ )
{
    if ( visible ) {
        if ( m_currentFrame )
        {
            // Add the cursor position to the (zoomed) frame position
            QPoint nPoint = frameSet()->kWordDocument()->zoomPoint( m_currentFrame->topLeft() );
            nPoint += formulaView->getCursorPoint();
            // Apply viewmode conversion
            QPoint p = m_canvas->viewMode()->normalToView( nPoint );
            m_canvas->ensureVisible( p.x(), p.y() );
        }
    }
    formulaFrameSet()->setChanged();
    m_canvas->repaintChanged( formulaFrameSet(), true );
}

#include "kwframe.moc"
