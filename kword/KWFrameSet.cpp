/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KWFrameSet.h"
#include "KWDocument.h"
#include "KWViewMode.h"
#include "KWCommand.h"
#include "KWFrame.h"
#include "KWTextFrameSet.h"
#include "KWTableFrameSet.h"
#include "KWAnchor.h"
#include "KWordFrameSetIface.h"
#include "KWFrameList.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KWFrameViewManager.h"
#include "KWFrameView.h"

#include <KoOasisContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include <qpopupmenu.h>
#include <qapplication.h>

//#define DEBUG_DRAW

KWFrameSet::KWFrameSet( KWDocument *doc )
    : m_doc( doc ), m_frames(), m_framesInPage(), m_firstPage( 0 ), m_emptyList(),
      m_info( FI_BODY ),
      m_groupmanager( 0L ), m_visible( true ),
      m_protectSize( false ),
      m_anchorTextFs( 0L ), m_dcop( 0L ), m_pageManager( 0 )
{
    // Send our "repaintChanged" signals to the document.
    setName("KWFrameSet");
    if(m_doc) {
        connect( this, SIGNAL( repaintChanged( KWFrameSet * ) ),
             doc, SLOT( slotRepaintChanged( KWFrameSet * ) ) );
        m_pageManager = doc->pageManager();
    }
    m_frames.setAutoDelete( true );
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

void KWFrameSet::addFrame( KWFrame *frame, bool recalc )
{
    if ( m_frames.findRef( frame ) != -1 )
        return;

    //kdDebug(32001) << k_funcinfo << name() << " adding frame" <<  frame << " recalc=" << recalc << endl;
    if(m_doc)
        KWFrameList::createFrameList(frame, m_doc);
    frame->setFrameSet(this);
    m_frames.append( frame );
    if(recalc)
        updateFrames();

    emit sigFrameAdded(frame);
}

void KWFrameSet::deleteFrame( unsigned int num, bool remove, bool recalc )
{
    //kdDebug(32001) << k_funcinfo << name() << " deleting frame" <<  num << " remove=" << remove << " recalc=" << recalc << endl; //kdBacktrace();
    KWFrame *frm = m_frames.at( num );
    Q_ASSERT( frm );
    m_frames.take( num );
    Q_ASSERT( !m_frames.contains(frm) );

    unsigned int index = frm->pageNumber() - m_firstPage;
    if(m_framesInPage.count() >= index) {
        QPtrList<KWFrame> *lst = m_framesInPage.at(index);
        lst->remove(frm);
    }

    KWFrameList *stack = frm->frameStack();
    if( stack ) {
        stack->update(); // will update the other frames on the page.
        frm->setFrameStack(0);
        delete stack;
    }
    emit sigFrameRemoved(frm);
    if ( !remove )
        frm->setFrameSet(0L);
    else {
        // ###### should something similar be done when just removing a frame from the list?
        frameDeleted( frm, recalc ); // inform kwtableframeset if necessary
        delete frm;
        //kdDebug(32001) << k_funcinfo << frm << " deleted. Now I have " << m_frames.count() << " m_frames" << endl;
    }

    if ( recalc )
        updateFrames();
}

void KWFrameSet::deleteFrame( KWFrame *frm, bool remove, bool recalc )
{
    //kdDebug(32001) << "KWFrameSet::deleteFrame " << frm << " remove=" << remove << endl;
    int num = m_frames.findRef( frm );
    Q_ASSERT( num != -1 );
    if ( num == -1 )
        return;

    deleteFrame( num, remove, recalc );
}

void KWFrameSet::deleteAllFrames()
{
    if ( !m_frames.isEmpty() )
    {
        for ( QPtrListIterator<KWFrame> frameIt( m_frames ); frameIt.current(); ++frameIt )
            emit sigFrameRemoved( *frameIt );
        m_frames.clear();
        updateFrames();
    }
}

void KWFrameSet::deleteAllCopies()
{
    if ( m_frames.count() > 1 )
    {
        KWFrame * firstFrame = m_frames.take(0);
        deleteAllFrames();
        m_frames.append( firstFrame );
        updateFrames();
    }
}

void KWFrameSet::createEmptyRegion( const QRect & crect, QRegion & emptyRegion, KWViewMode *viewMode )
{
    KWPage *page = m_doc->pageManager()->page( frame(0) );
    if( !page ) {
        kdWarning(31001) << "The first frame of '" << name() << "' is outside all pages!!" << endl;
        return;
    }
    double paperHeight = page->height();
    //kdDebug(32001) << "KWFrameSet::createEmptyRegion " << name() << endl;
    for (QPtrListIterator<KWFrame> frameIt = frameIterator(); frameIt.current(); ++frameIt )
    {
        if ( !frameIt.current()->isTransparent() )
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
}

void KWFrameSet::drawPadding( KWFrame *frame, QPainter *p, const QRect &crect, const QColorGroup &, KWViewMode *viewMode )
{
    QRect outerRect( viewMode->normalToView( frame->outerRect(viewMode) ) );
    //kdDebug(32001) << "KWFrameSet::drawPadding frame: " << frameFromPtr( frame )
    //               << " outerRect: " << outerRect
    //               << " crect: " << crect << endl;

    if ( !crect.intersects( outerRect ) )
    {
#ifdef DEBUG_DRAW
        kdDebug(32001) << "KWFrameSet::drawPadding no intersection with " << crect << endl;
#endif
        return;
    }
    QRect frameRect( viewMode->normalToView( m_doc->zoomRect( *frame ) ) );
    p->save();
    QBrush bgBrush( frame->backgroundColor() );
    bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), p ) );
    p->setBrush( bgBrush );
    int leftMargin = m_doc->zoomItX(frame->paddingLeft());
    int topMargin = m_doc->zoomItY(frame->paddingTop());
    int rightMargin = m_doc->zoomItX(frame->paddingRight());
    int bottomMargin = m_doc->zoomItY(frame->paddingBottom());
    //kdDebug(32001) << "KWFrameSet::drawPadding leftMargin=" << leftMargin << " topMargin=" << topMargin << " rightMargin=" << rightMargin << " bottomMargin=" << bottomMargin << endl;

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
    if ( bottomMargin != 0 )
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
        KoPoint dPoint( m_frames.first()->topLeft() );
        kdDebug(32001) << "KWFrameSet::setFloating looking for pos at " << dPoint.x() << " " << dPoint.y() << endl;
        frameSet->findPosition( dPoint, parag, index );
        // Create anchor. TODO: refcount the anchors!
        setAnchored( frameSet, parag, index );
        frameSet->layout();
        m_doc->frameChanged( m_frames.first() );
        return;
    }
}

void KWFrameSet::setProtectSize( bool b)
{
    m_protectSize = b;
}

void KWFrameSet::setAnchored( KWTextFrameSet* textfs, int paragId, int index, bool placeHolderExists /* = false */, bool repaint )
{
    KWTextParag * parag = static_cast<KWTextParag *>( textfs->textDocument()->paragAt( paragId ) );
    Q_ASSERT( parag );
    if ( parag )
        setAnchored( textfs, parag, index, placeHolderExists, repaint );
}

void KWFrameSet::setAnchored( KWTextFrameSet* textfs, KoTextParag* parag, int index, bool placeHolderExists /* = false */, bool repaint )
{
    kdDebug(32001) << "KWFrameSet::setAnchored " << textfs << " " << parag->paragId() << " " << index << " " << placeHolderExists << endl;
    Q_ASSERT( textfs );
    Q_ASSERT( parag );
    if ( isFloating() )
        deleteAnchors();
    m_anchorTextFs = textfs;
    KWFrameList::createFrameList(textfs, m_doc); // remove ourselves from others list now we are inline
    if ( parag )
        createAnchors( parag, index, placeHolderExists, repaint );

    if ( !placeHolderExists ) // i.e. not while loading
    {
        m_doc->updateAllFrames(); // We just became floating, so we need to be removed from "frames on top/below".
        // TODO pass page number to updateAllFrames - hmm, we could have several frames in theory
    }
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
    kdWarning() << "KWFrameSet::findAnchor anchor not found (frameset='" << name()
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
        frameIt.current()->setZOrder( m_doc->maxZOrder( frameIt.current()->pageNumber(m_doc) ) + 1 );

    m_doc->repaintAllViews();
    m_doc->updateRulerFrameStartEnd();
}

KWAnchor * KWFrameSet::createAnchor( KoTextDocument *txt, int frameNum )
{
    KWAnchor * anchor = new KWAnchor( txt, this, frameNum );
    return anchor;
}

void KWFrameSet::createAnchors( KoTextParag * parag, int index, bool placeHolderExists /*= false */ /*only used when loading*/,
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
    KWFrame * frame = m_frames.at( frameNum );
    Q_ASSERT( frame );
    if ( !frame ) return;

    KoPoint pos( position );
    // position includes the border, we need to adjust accordingly
    pos.rx() += frame->leftBorder().width();
    pos.ry() += frame->topBorder().width();
    if ( frame->topLeft() != pos )
    {
        kdDebug(32002) << "KWFrameSet::moveFloatingFrame " << pos.x() << "," << pos.y() << endl;
        int oldPageNum = frame->pageNumber();
        frame->moveTopLeft( pos );

        updateFrames();
        if( frame->frameStack() )
            frame->frameStack()->updateAfterMove( oldPageNum );
    }
    invalidate();
}

KoRect KWFrameSet::floatingFrameRect( int frameNum )
{
    KWFrame * frame = m_frames.at( frameNum );
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
    KWFrame * frame = m_frames.at( frameNum );
    Q_ASSERT( frame );
    return frame->outerKoRect().size();
}

KCommand * KWFrameSet::anchoredObjectCreateCommand( int frameNum )
{
    KWFrame * frame = m_frames.at( frameNum );
    Q_ASSERT( frame );
    return new KWCreateFrameCommand( QString::null, frame );
}

KCommand * KWFrameSet::anchoredObjectDeleteCommand( int frameNum )
{
    KWFrame * frame = m_frames.at( frameNum );
    Q_ASSERT( frame );
    return new KWDeleteFrameCommand( QString::null, frame );
}

KWFrame * KWFrameSet::frameAtPos( double x, double y ) const
{
    KoPoint docPoint( x, y );
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
        if ( frameIt.current()->contains( docPoint ) )
            return frameIt.current();
    return 0L;
}

KWFrame *KWFrameSet::frame( unsigned int num ) const
{
    // QPtrList sucks
    return const_cast<KWFrameSet*>( this )->m_frames.at( num );
}

int KWFrameSet::frameFromPtr( KWFrame *frame )
{
    return m_frames.findRef( frame );
}

KWFrame * KWFrameSet::settingsFrame( const KWFrame* frame )
{
    if ( !frame->isCopy() )
        return const_cast<KWFrame *>( frame );
    KWFrame* lastRealFrame=0L;
    QPtrListIterator<KWFrame> frameIt( frame->frameSet()->frameIterator() );
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame *curFrame = frameIt.current();
        if ( curFrame == frame )
            return lastRealFrame ? lastRealFrame : const_cast<KWFrame *>( frame );
        if ( !lastRealFrame || !curFrame->isCopy() )
            lastRealFrame = curFrame;
    }
    return const_cast<KWFrame *>( frame ); //fallback, should never happen
}

void KWFrameSet::updateFrames( int flags )
{
    if ( m_frames.isEmpty() )
        return; // No frames. This happens when the frameset is deleted (still exists for undo/redo)

    // Not visible ? Don't bother then.
    if ( !isVisible() )
        return;

    //kdDebug(32001) << "KWFrameSet::updateFrames " << this << " " << name() << endl;

    if ( flags & UpdateFramesInPage ) {
        // For each of our frames, clear old list of frames on top, and grab min/max page nums
        m_firstPage = m_frames.first()->pageNumber(); // we know m_frames is not empty here
        int lastPage = m_firstPage;
        QPtrListIterator<KWFrame> fIt( frameIterator() );
        for ( ; fIt.current(); ++fIt ) {
            int pg = fIt.current()->pageNumber();
            m_firstPage = KMIN( m_firstPage, pg );
            lastPage = KMAX( lastPage, pg );
        }
        //kdDebug(32001) << "firstPage=" << m_firstPage << " lastPage=" << lastPage << endl;

        // Prepare the m_framesInPage structure
        int oldSize = m_framesInPage.size();
        m_framesInPage.resize( lastPage - m_firstPage + 1 );
        // Clear the old elements
        int oldElements = KMIN( oldSize, (int)m_framesInPage.size() );
        for ( int i = 0 ; i < oldElements ; ++i )
            m_framesInPage[i]->clear();
        // Initialize the new elements.
        for ( int i = oldElements ; i < (int)m_framesInPage.size() ; ++i )
            m_framesInPage.insert( i, new QPtrList<KWFrame>() );

        // Iterate over m_frames again, to fill the m_framesInPage array
        fIt.toFirst();
        for ( ; fIt.current(); ++fIt ) {
            int pg = fIt.current()->pageNumber();
            Q_ASSERT( pg <= lastPage );
            m_framesInPage[pg - m_firstPage]->append( fIt.current() );
        }
    }

    if ( isFloating() )
    {
        //kdDebug(32001) << "KWFrameSet::updateFrames " << name() << " is floating" << endl;
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
    if ( groupmanager() )
    {
        if ( groupmanager()->isPaintedBy( fs ) )
            return true;
    }
    return false;
}

const QPtrList<KWFrame> & KWFrameSet::framesInPage( int pageNum ) const
{
    if ( pageNum < m_firstPage || pageNum >= (int)m_framesInPage.size() + m_firstPage )
    {
#ifdef DEBUG_DTI
        kdWarning(32002) << name() << " framesInPage called for pageNum=" << pageNum << ". "
                    << " Min value: " << m_firstPage
                    << " Max value: " << m_framesInPage.size() + m_firstPage - 1 << endl;
#endif
        return m_emptyList; // QPtrList<KWFrame>() doesn't work, it's a temporary
    }
    return * m_framesInPage[pageNum - m_firstPage];
}

void KWFrameSet::drawContents( QPainter *p, const QRect & crect, const QColorGroup &cg,
                               bool onlyChanged, bool resetChanged,
                               KWFrameSetEdit *edit, KWViewMode *viewMode,
                               KWFrameViewManager *frameViewManager )
{
#ifdef DEBUG_DRAW
    kdDebug(32001) << "\nKWFrameSet::drawContents " << this << " " << name()
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
            if(viewMode->drawSelections() && frameViewManager) {
                KWFrameView* view = frameViewManager->view(frame);
                if(view)
                    view->paintFrameAttributes(p, crect, viewMode, m_doc);
            }

            if ( !lastRealFrame || !frame->isCopy() )
            {
                lastRealFrame = frame;
                //lastRealFrameTop = totalHeight;
            }
            //totalHeight += frame->innerHeight();
        }
    }
    else { // Text view mode
        QRect normalRect = viewMode->viewToNormal(crect);
        drawFrame( 0L /*frame*/, p, normalRect, crect, QPoint(KWViewModeText::OFFSET, 0),
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
        kdDebug(32002) << "KWFrameSet::drawFrameAndBorders " << name() << " frame " << frameFromPtr( frame ) << " " << frame << " isn't valid" << endl;
        return;
    }

    QRect normalOuterFrameRect( frame->outerRect( viewMode ) );
    QRect outerFrameRect( viewMode->normalToView( normalOuterFrameRect ) );
    QRect outerCRect = crect.intersect( outerFrameRect );
#ifdef DEBUG_DRAW
    kdDebug(32001) << "KWFrameSet::drawFrameAndBorders " << name() << " frame " << frameFromPtr( frame ) << " " << *frame << endl;
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
        if ( !innerCRect.isEmpty() )
        {
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

                if( !groupmanager() ) // not for table cells
                    drawFrameBorder( painter, frame, settingsFrame, outerCRect, viewMode );
                painter->restore();
            }
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
    kdDebug(32001) << "\nKWFrameSet::drawFrame " << name() << " outerCRect=" << outerCRect << " frameCrect=" << fcrect << " drawUnderlyingFrames=" << drawUnderlyingFrames << endl;
#endif
    Q_ASSERT( fcrect.isValid() );

    QColorGroup frameColorGroup( cg );
    if ( settingsFrame ) // 0L in text viewmode
    {
        QBrush bgBrush( settingsFrame->backgroundColor() );
        bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), painter ) );
        frameColorGroup.setBrush( QColorGroup::Base, bgBrush );
    }

    if ( drawUnderlyingFrames && frame && frame->frameStack()) {
        QValueList<KWFrame*> below = frame->frameStack()->framesBelow();
        if ( !below.isEmpty() )
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
            kdDebug(32001) << "  below: " << below.count() << endl;
#endif
            for (QValueListIterator<KWFrame*> it = below.begin(); it != below.end(); ++it )
            {
                KWFrame* f = (*it);

#ifdef DEBUG_DRAW
                kdDebug(32001) << "  looking at frame below us: " << f->frameSet()->name() << " frame " << frameFromPtr( frame ) << endl;
#endif
                QRect viewFrameCRect = outerCRect.intersect( viewMode->normalToView( f->outerRect( viewMode ) ) );
                if ( !viewFrameCRect.isEmpty() )
                {
#ifdef DEBUG_DRAW
                    kdDebug(32001) << "  viewFrameRect=" << viewFrameCRect << " calling drawFrameAndBorders." << endl;
#endif
                    f->frameSet()->drawFrameAndBorders( f, doubleBufPainter, viewFrameCRect, cg,
                            false, resetChanged, edit, viewMode, 0L, false );
                }
            }

            if ( frame->paddingLeft() || frame->paddingTop() || frame->paddingRight() || frame->paddingBottom() )
                drawPadding( frame, doubleBufPainter, outerCRect, cg, viewMode );
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
            return; // done! :)
        }
        else
        {
            // nothing below? paint a bg color then
            frameColorGroup.setBrush( QColorGroup::Base, m_doc->defaultBgColor( painter ) );
        }
    }
    if ( frame && (frame->paddingLeft() || frame->paddingTop() ||
                frame->paddingRight() || frame->paddingBottom()) )
        drawPadding( frame, painter, outerCRect, cg, viewMode );
    painter->save();
    painter->translate( translationOffset.x(), translationOffset.y() );

    drawFrameContents( frame, painter, fcrect, frameColorGroup, onlyChanged, resetChanged, edit, viewMode );
    painter->restore();
}

void KWFrameSet::drawFrameContents( KWFrame *, QPainter *, const QRect &,
                                    const QColorGroup &, bool, bool, KWFrameSetEdit*, KWViewMode * )
{
    kdWarning() << "Default implementation of drawFrameContents called for " << className() << " " << this << " " << name() << kdBacktrace();
}

void KWFrameSet::saveCommon( QDomElement &parentElem, bool saveFrames )
{
    if ( m_frames.isEmpty() ) // Deleted frameset -> don't save
        return;

    // Save all the common attributes for framesets.
    parentElem.setAttribute( "frameType", static_cast<int>( type() ) );
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

KWFrame* KWFrameSet::loadOasisFrame( const QDomElement& tag, KoOasisContext& context )
{
    double width = 100;
    if ( tag.hasAttributeNS( KoXmlNS::svg, "width" ) ) { // fixed width
        // TODO handle percentage (of enclosing table/frame/page)
        width = KoUnit::parseValue( tag.attributeNS( KoXmlNS::svg, "width", QString::null ) );
    } else if ( tag.hasAttributeNS( KoXmlNS::fo, "min-width" ) ) {
        // min-width is not supported in KWord. Let's use it as a fixed width.
        width = KoUnit::parseValue( tag.attributeNS( KoXmlNS::fo, "min-width", QString::null ) );
    } else {
        kdWarning(32001) << "Error in frame " << tag.tagName() << " " << tag.attributeNS( KoXmlNS::draw, "name", QString::null ) << " : neither width nor min-width specified!" << endl;
    }
    double height = 100;
    if ( tag.hasAttributeNS( KoXmlNS::svg, "height" ) ) { // fixed height
        // TODO handle percentage (of enclosing table/frame/page)
        height = KoUnit::parseValue( tag.attributeNS( KoXmlNS::svg, "height", QString::null ) );
    }
    //kdDebug(32001) << k_funcinfo << "width=" << width << " height=" << height << " pt" << endl;

    KWFrame * frame = new KWFrame(this,
                                  KoUnit::parseValue( tag.attributeNS( KoXmlNS::svg, "x", QString::null ) ),
                                  KoUnit::parseValue( tag.attributeNS( KoXmlNS::svg, "y", QString::null ) ),
                                  width, height );

    frame->setZOrder( tag.attributeNS( KoXmlNS::draw, "z-index", QString::null ).toInt() );
    // Copy-frames.
    // We currently ignore the value of the copy-of attribute. It probably needs to
    // be handled like chain-next-name (kwtextframeset.cc) but for all types of frameset.
    frame->setCopy( tag.hasAttributeNS( KoXmlNS::draw, "copy-of" ) );
    frame->loadCommonOasisProperties( context, this, "graphic" );

    addFrame( frame, false );

    // Protect (OASIS 14.27.7, also in OO-1.1)
    // A frame with protected contents means that the frameset is protected (makes sense)
    // A frame with protected size means that the frameset is size-protected (hmm, kword did it that way)
    // TODO implement position protection
    QString protectList = context.styleStack().attributeNS( KoXmlNS::style, "protect" );
    if ( protectList.contains( "content" ) )
        setProtectContent( true );
    if ( protectList.contains( "size" ) )
        m_protectSize = true;

    // TODO m_visible ? User-toggeable or internal?

    return frame;
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
    if ( !m_visible || m_frames.isEmpty() )
        return false;
    if ( isAHeader() && !m_doc->isHeaderVisible() )
        return false;
    if ( isAFooter() && !m_doc->isFooterVisible() )
        return false;
    if ( viewMode && !viewMode->isFrameSetVisible(this) )
        return false;
    if ( isFloating() && !anchorFrameset()->isVisible( viewMode ) )
         return false;

    KoHFType ht = m_doc != 0 ? m_doc->headerType(): HF_FIRST_DIFF;
    KoHFType ft = m_doc != 0 ? m_doc->footerType(): HF_FIRST_DIFF;
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
    return ( m_doc && m_doc->processingType() == KWDocument::WP &&
             m_doc->frameSet( 0 ) == this );
}

bool KWFrameSet::isMoveable() const
{
    if ( isHeaderOrFooter() )
        return false;
    return !isMainFrameset() && !isFloating();
}

const char* KWFrameSet::headerFooterTag() const
{
    switch ( m_info ) {
    case KWFrameSet::FI_ODD_HEADER:
        return "style:header";
    case KWFrameSet::FI_EVEN_HEADER:
        return "style:header-left";
    case KWFrameSet::FI_ODD_FOOTER:
        return "style:footer";
    case KWFrameSet::FI_EVEN_FOOTER:
        return "style:footer-left";
    case KWFrameSet::FI_FIRST_HEADER:
        return "style:header-first"; // NOT OASIS COMPLIANT
    case KWFrameSet::FI_FIRST_FOOTER:
        return "style:footer-first"; // NOT OASIS COMPLIANT
    default: // shouldn't be called for body or footnote
        return 0;
    }
}

void KWFrameSet::finalize()
{
    //kdDebug(32001) << "KWFrameSet::finalize ( calls updateFrames + zoom ) " << this << endl;
    updateFrames();
}

QRegion KWFrameSet::frameClipRegion( QPainter * painter, KWFrame *frame, const QRect & crect,
                                     KWViewMode * viewMode )
{
//    KWDocument * doc = kWordDocument();
    QRect rc = painter->xForm( crect );
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

        Q_ASSERT( frame->frameStack() );

        QValueList<KWFrame *> onTop = frame->frameStack()->framesOnTop();
        for (QValueListIterator<KWFrame*> fIt = onTop.begin(); fIt != onTop.end(); ++fIt )
        {
            KWFrame* frameOnTop = (*fIt);
            Q_ASSERT( frameOnTop->frameSet() );
            QRect r = painter->xForm( viewMode->normalToView( frameOnTop->outerRect( viewMode ) ) );
#ifdef DEBUG_DRAW
            //kdDebug(32002) << "frameClipRegion subtract rect "<< r << endl;
#endif
            reg -= r; // subtract
        }
#ifdef DEBUG_DRAW
        //kdDebug(32002) << "KWFrameSet::frameClipRegion result:" << reg << endl;
#endif
        return reg;
    }
    return QRegion();
}

bool KWFrameSet::canRemovePage( int num )
{
    QPtrListIterator<KWFrame> frameIt( frameIterator() );
    for ( ; frameIt.current(); ++frameIt )
    {
        KWFrame * frame = frameIt.current();
        if ( frame->pageNumber() == num ) // ## TODO: use framesInPage, see KWTextFrameSet
        {
            // Ok, so we have a frame on that page -> we can't remove it unless it's a copied frame
            if ( ! ( frame->isCopy() && frameIt.current() != m_frames.first() ) )
            {
                kdDebug(32001) << "KWFrameSet::canRemovePage " << name() << " frame on page " << num << " -> false" << endl;
                return false;
            }
        }
    }
    return true;
}

void KWFrameSet::setFrameBehavior( KWFrame::FrameBehavior fb ) {
    for(KWFrame *f=m_frames.first();f;f=m_frames.next())
        f->setFrameBehavior(fb);
}

void KWFrameSet::setNewFrameBehavior( KWFrame::NewFrameBehavior nfb ) {
    for(KWFrame *f=m_frames.first();f;f=m_frames.next())
        f->setNewFrameBehavior(nfb);
}

// ## this should pass the viewmode as argument, probably.
bool KWFrameSet::isFrameAtPos( const KWFrame* frame, const QPoint& point, bool borderOfFrameOnly) const {
    QRect outerRect( frame->outerRect( m_doc->layoutViewMode() ) );
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
    //kdDebug(32001) << "KWFrameSet::setZOrder (to max) " << name() << endl;
    QPtrListIterator<KWFrame> fit = frameIterator();
    for ( ; fit.current() ; ++fit )
        fit.current()->setZOrder( m_doc->maxZOrder( fit.current()->pageNumber(m_doc) ) + 1 );
}

void KWFrameSet::setName( const QString &name )
{
    m_name = name;
    emit sigNameChanged(this);
}

#ifndef NDEBUG
#include "KWFrameViewManager.h"
#include "KWFrameView.h"
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

    KWFrameViewManager *fvm = 0;
    if ( !m_doc->getAllViews().isEmpty() ) {
        KWView *view = m_doc->getAllViews().first();
        if(view)
            fvm = view->frameViewManager();
    }

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
        kdDebug() << " +-- Frame " << j << " of "<< frameCount() << "    (" << frame << ")  " << copy << endl;
        printDebug( frame );
        kdDebug() << "     Rectangle : " << frame->x() << "," << frame->y() << " " << frame->width() << "x" << frame->height() << endl;
        kdDebug() << "     RunAround: "<< runaround[ frame->runAround() ] << " side:" << runaroundSide[ frame->runAroundSide() ]<< endl;
        kdDebug() << "     FrameBehavior: "<< frameBh[ frame->frameBehavior() ] << endl;
        kdDebug() << "     NewFrameBehavior: "<< newFrameBh[ frame->newFrameBehavior() ] << endl;
        QColor col = frame->backgroundColor().color();
        kdDebug() << "     BackgroundColor: "<< ( col.isValid() ? col.name().latin1() : "(default)" ) << endl;
        kdDebug() << "     SheetSide "<< frame->sheetSide() << endl;
        kdDebug() << "     Z Order: " << frame->zOrder() << endl;

        if( frame->frameStack() ) {
            QValueList<KWFrame*> onTop = frame->frameStack()->framesOnTop();
            QValueList<KWFrame*> below = frame->frameStack()->framesBelow();

            kdDebug() << "     Frames below: " << below.count()
                      << ", frames on top: " << onTop.count() << endl;
        }
        else
            kdDebug() << "     no frameStack set." << endl;
        kdDebug() << "     minFrameHeight "<< frame->minimumFrameHeight() << endl;
        QString page = pageManager() && pageManager()->pageCount() > 0 ? QString::number(frame->pageNumber()) : " [waiting for pages to be created]";

        KWFrameView *fv = 0;
        if(fvm) fv = fvm->view(frame);
        if(fv && fv->selected())
            kdDebug() << " *   Page "<< page << endl;
        else
            kdDebug() << "     Page "<< page << endl;
    }
}

void KWFrameSet::printDebug( KWFrame * )
{
}

#endif

#include "KWFrameSet.moc"
