/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "kwanchor.h"
#include "kwtextframeset.h"
#include "kwdoc.h"
#include "kwviewmode.h"
#include <kdebug.h>
#include <kdebugclasses.h>


KWAnchor::KWAnchor( KoTextDocument *textDocument, KWFrameSet * frameset, int frameNum )
    : KoTextCustomItem( textDocument),
      m_frameset( frameset ),
      m_frameNum( frameNum )
{
}

KWAnchor::~KWAnchor()
{
    kdDebug(32001) << "KWAnchor::~KWAnchor" << endl;
}

void KWAnchor::setFormat( KoTextFormat* format )
{
    m_frameset->setAnchorFormat( format, m_frameNum );
}

void KWAnchor::finalize()
{
    if ( m_deleted )
        return;

    int paragx = paragraph()->rect().x();
    int paragy = paragraph()->rect().y();
    kdDebug(32001) << this << " KWAnchor::finalize " << x() << "," << y() << " paragx=" << paragx << " paragy=" << paragy << endl;

    KWTextFrameSet * fs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
    KoPoint dPoint;
    if ( fs->internalToDocument( QPoint( x()+paragx, y()+paragy ), dPoint ) )
    {
        //kdDebug(32001) << "KWAnchor::finalize moving frame to [zoomed pos] " << nPoint.x() << "," << nPoint.y() << endl;
        // Move the frame to position nPoint.
        m_frameset->moveFloatingFrame( m_frameNum, dPoint );
    } else
    {
        // This can happen if the page hasn't been created yet
        kdDebug(32001) << "KWAnchor::move internalToDocument returned 0L for " << x()+paragx << ", " << y()+paragy << endl;
    }
}

//#define DEBUG_DRAWING

void KWAnchor::draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected )
{
    // (x,y) is the position of the inline item (in Layout Units)
    // (cx,cy,cw,ch) is the rectangle to be painted, in layout units too

    if ( m_deleted )
        return;

    Q_ASSERT( x == xpos );
    Q_ASSERT( y == ypos );
    //kdDebug() << "KWAnchor::draw x=" << x << " y=" << y << " xpos=" << xpos << " ypos=" << ypos << endl;

    // The containing text-frameset.
    KWTextFrameSet * fs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
    KoZoomHandler* zh = fs->textDocument()->paintingZoomHandler();

    int paragx = paragraph()->rect().x();
    int paragy = paragraph()->rect().y();
    QRect inlineFrameLU( paragx+xpos, paragy+ypos, width, height );
#ifdef DEBUG_DRAWING
    kdDebug(32001) << "KWAnchor::draw x:" << x << ", y:" << y << " paragx=" << paragx << " paragy=" << paragy << endl;
    kdDebug(32001) << "               inline frame in LU coordinates: " << inlineFrameLU << endl;
#endif

    QRect crectLU = QRect( (cx > 0 ? cx : 0)+paragx, cy+paragy, cw, ch );
#ifdef DEBUG_DRAWING
    kdDebug(32001) << "               crect in LU coordinates: " << DEBUGRECT( crectLU ) << endl;
#endif

    crectLU = crectLU.intersect ( inlineFrameLU ); // KoTextParag::paintDefault could even do this

    // Convert crect to document coordinates, first topleft then bottomright
    QPoint topLeftLU = crectLU.topLeft();
    QPoint bottomRightLU = crectLU.bottomRight();
    KWFrame* containingFrame = fs->currentDrawnFrame(); // always set, except in the textviewmode
    KoPoint topLeftPt = fs->internalToDocumentKnowingFrame( topLeftLU, containingFrame );

#if 0 // not needed anymore, I think. There's no more ITD that can fail.
    if ( containingFrame )
    {
        // Look at the bottom-right of the containing frame - to intersect, if we get out of it
        int rightFrameLU = zh->ptToLayoutUnitPixX( containingFrame->innerWidth() );
        int bottomFrameLU = zh->ptToLayoutUnitPixY( containingFrame->internalY() + containingFrame->innerHeight() );
        //kdDebug() << "KWAnchor::draw rightFrameLU " << rightFrameLU << " bottomFrameLU=" << bottomFrameLU << endl;

        crectLU &= QRect( topLeftLU, QPoint( rightFrameLU, bottomFrameLU ) );
        //kdDebug() << "KWAnchor::draw crectLU now " << crectLU << endl;
        bottomRightLU = crectLU.bottomRight();
    }
#endif

    // Now we can convert the bottomright
    KoPoint bottomRightPt = fs->internalToDocumentKnowingFrame( bottomRightLU, containingFrame );
    KoRect crectPt( topLeftPt, bottomRightPt );

    // Convert crect to view coords
    QRect crect = fs->currentViewMode()->normalToView( zh->zoomRect( crectPt ) );
#ifdef DEBUG_DRAWING
    kdDebug() << "               crect in view coordinates (pixel) : " << DEBUGRECT( crect ) << endl;
#endif

    // Ok, we finally have our crect in view coordinates!
    // Now ensure the containing frame is the one actually containing our text
    // (for copies, e.g. headers and footers, we need to go back until finding a real frame)

    if ( containingFrame && containingFrame->isCopy() )
    {
        // Find last real frame, in case we are in a copied frame
        QPtrListIterator<KWFrame> frameIt( fs->frameIterator() );
        frameIt.toLast(); // from the end to avoid a 2*N in the worst case
        while ( !frameIt.atFirst() && frameIt.current() != containingFrame ) // look for 'containingFrame'
            --frameIt;
        if ( frameIt.atFirst() && frameIt.current() != containingFrame )
            kdWarning() << "KWAnchor::draw: containingFrame not found " << containingFrame << endl;
        while ( !frameIt.atFirst() && frameIt.current()->isCopy() ) // go back to last non-copy
            --frameIt;
        containingFrame = frameIt.current();
        //kdDebug() << "KWAnchor::draw frame=" << containingFrame << endl;
    }

    // Same calculation as in internalToDocument, but we know the frame already
    KoPoint topLeftParagPt( 0, 0 );
    if ( containingFrame ) // 0 in the textviewmode
        topLeftParagPt = containingFrame->innerRect().topLeft();

    topLeftParagPt.rx() += zh->layoutUnitPtToPt( zh->pixelYToPt( paragx ) );
    topLeftParagPt.ry() += zh->layoutUnitPtToPt( zh->pixelYToPt( paragy ) );
    if ( containingFrame ) // 0 in the textviewmode
        topLeftParagPt.ry() -= containingFrame->internalY();

    QPoint topLeftParag = fs->currentViewMode()->normalToView( zh->zoomPoint( topLeftParagPt ) );

    // Finally, make the painter go back to view coord system
    // (this is exactly the opposite of the code in KWFrameSet::drawContents)
    // (It does translate(view - internal), so we do translate(internal - view) - e.g. with (0,0) for internal)
    p->save();
    p->translate( -topLeftParag.x(), -topLeftParag.y() );
#ifdef DEBUG_DRAWING
    kdDebug() << "               translating by " << -topLeftParag.x() << "," << -topLeftParag.y() << endl;
#endif

    QColorGroup cg2( cg );
    m_frameset->drawContents( p, crect, cg2, false, true, 0L, fs->currentViewMode(), fs->currentDrawnCanvas() );

    if( selected && placement() == PlaceInline && p->device()->devType() != QInternal::Printer )
    {
        // The above rects are about the containing frame.
        // To draw the inline frame as selected, we need to look at the inline frame's own size.
        QRect frameRect = crect;
#ifdef DEBUG_DRAWING
    kdDebug() << "KWAnchor::draw selected frame. (frameRect&crect) = " << frameRect << endl;
#endif
            p->fillRect( frameRect, QBrush( cg.highlight(), QBrush::Dense4Pattern) );
    }
    p->restore();

#ifdef DEBUG_DRAWING
    kdDebug() << "KWAnchor::draw done" << endl;
#endif
}

QSize KWAnchor::size() const
{
    KoSize kosz = m_frameset->floatingFrameKoRect( m_frameNum ).size();
    //kdDebug() << "KWAnchor::size " << kosz.width() << "x" << kosz.height() << endl;
    KoZoomHandler * zh = textDocument()->formattingZoomHandler();
    QSize sz( zh->ptToLayoutUnitPixX( kosz.width() ), zh->ptToLayoutUnitPixX( kosz.height() ) );
    if ( sz.isNull() ) // for some reason, we don't know the size yet
        sz = QSize( width, height ); // LU
    return sz;
}

int KWAnchor::ascent() const
{
    int baseline = m_frameset->floatingFrameBaseline( m_frameNum );
    int ret = ( baseline == -1 ) ? height : baseline;
    //kdDebug() << "KWAnchor::ascent " << ret << endl;
    return ret;
}

void KWAnchor::resize()
{
    if ( m_deleted )
        return;
    QSize s = size();
    if ( width != s.width() || height != s.height() )
    {
        width = s.width();
        height = s.height();
        kdDebug(32001) << "KWAnchor::resize " << width << "x" << height << endl;
        KoTextParag * parag = paragraph();
        if ( parag )
        {
            kdDebug(32001) << "KWAnchor::resize invalidating parag " << parag->paragId() << endl;
            parag->invalidate( 0 );
        }
    }
}

KCommand * KWAnchor::createCommand()
{
    kdDebug(32001) << "KWAnchor::addCreateCommand" << endl;
    return m_frameset->anchoredObjectCreateCommand( m_frameNum );
}

KCommand * KWAnchor::deleteCommand()
{
    kdDebug(32001) << "KWAnchor::addDeleteCommand" << endl;
    return m_frameset->anchoredObjectDeleteCommand( m_frameNum );
}

void KWAnchor::setDeleted( bool b )
{
    kdDebug() << "KWAnchor::setDeleted " << b << endl;
    if ( b )
        m_frameset->setAnchored( 0L );
    else
        m_frameset->setAnchored( static_cast<KWTextDocument *>(textDocument())->textFrameSet() );
    KoTextCustomItem::setDeleted( b );
}

void KWAnchor::save( QDomElement &parentElem )
{
    QDomElement anchorElem = parentElem.ownerDocument().createElement( "ANCHOR" );
    parentElem.appendChild( anchorElem );
    anchorElem.setAttribute( "type", "frameset" ); // the only possible value currently
    //KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
    // ## TODO save the frame number as well ? Only the first frame ? to be determined
    // ## or maybe use len=<number of frames>. Difficult :}
    anchorElem.setAttribute( "instance", m_frameset->getName() );
}

