/*
    Copyright (C) 2001, S.R.Haque (srhaque@iee.org).
    This file is part of the KDE project

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

DESCRIPTION

    This file implements KWord tables.
*/

#include <kdebug.h>
#include <klocale.h>
#include "kwdoc.h"
#include "kwanchor.h"
#include "kwtableframeset.h"
#include "kwcanvas.h"
#include "kwcommand.h"
#include "kwviewmode.h"
#include "kwview.h"
#include <kotextobject.h> // for customItemChar !
#include <qpopupmenu.h>

// I changed this to 1 because it fixes cell border drawing in the normal case
// and because e.g. msword has no big cell-spacing by default.
// The real fix here would be to have a spacing per line and per row, that depends
// on the border (max of the borders of all cells on the line)
const unsigned int KWTableFrameSet::tableCellSpacing = 1; // 3;

KWTableFrameSet::KWTableFrameSet( KWDocument *doc, const QString & name ) :
    KWFrameSet( doc )
{
    m_rows = 0;
    m_cols = 0;
    m_name = QString::null;
    m_showHeaderOnAllPages = true;
    m_hasTmpHeaders = false;
    m_active = true;
    m_cells.setAutoDelete( true );
    frames.setAutoDelete(false);
    //m_anchor = 0L;
    if ( name.isEmpty() )
        m_name = doc->generateFramesetName( i18n( "Table %1" ) );
    else
        m_name = name;
}

KWTableFrameSet::KWTableFrameSet( KWTableFrameSet &original ) :
    KWFrameSet( original.m_doc )
{
    m_doc = original.m_doc;
    m_rows = original.m_rows;
    m_cols = original.m_cols;
    m_name = original.m_name + '_'; // unicity problem !
    m_showHeaderOnAllPages = original.m_showHeaderOnAllPages;
    m_hasTmpHeaders = original.m_hasTmpHeaders;
    m_active = original.m_active;
    m_cells.setAutoDelete( true );
    //m_anchor = 0L;

    // Copy all cells.
    for ( unsigned int i = 0; i < original.m_cells.count(); i++ )
    {
        Cell *cell = new Cell( this, *original.m_cells.at( i ) );
        m_cells.append( cell );
    }
    m_doc->addFrameSet( this );
    frames.setAutoDelete(false);
}

KWTableFrameSet::~KWTableFrameSet()
{
    //delete m_anchor;
    //m_anchor = 0L;
    m_doc = 0L;
}

KWFrameSetEdit * KWTableFrameSet::createFrameSetEdit( KWCanvas * canvas )
{
    return new KWTableFrameSetEdit( this, canvas );
}

void KWTableFrameSet::updateFrames()
{
    if(m_cells.count()==0)
        return;
    for (unsigned int i =0; i < m_cells.count(); i++)
    {
        m_cells.at(i)->updateFrames();
        m_cells.at(i)->invalidate();
    }
    KWFrameSet::updateFrames();
}

void KWTableFrameSet::moveFloatingFrame( int /*frameNum TODO */, const KoPoint &position )
{
    Cell * cell = getCell( 0, 0 );
    Q_ASSERT( cell );
    if ( !cell ) return;
    KoPoint currentPos = cell->getFrame( 0 )->topLeft();
    KoPoint pos( position );
    // position includes the border, we need to adjust accordingly
    pos.rx() += cell->getFrame( 0 )->leftBorder().ptWidth;
    pos.ry() += cell->getFrame( 0 )->topBorder().ptWidth;
    if ( currentPos != pos )
    {
        kdDebug() << "KWTableFrameSet::moveFloatingFrame " << pos.x() << "," << pos.y() << endl;
        KoPoint offset = pos - currentPos;
        moveBy( offset.x(), offset.y() );

        // Recalc all "frames on top" everywhere
        kWordDocument()->updateAllFrames();
        // Don't call any drawing method from here.
        // We are called from KWAnchor::draw, inside a paintevent, so
        // we are not allowed to create a paint event ourselves.
        // KWAnchor draws the table anyway!
    }
}

QSize KWTableFrameSet::floatingFrameSize( int /*frameNum TODO */ )
{
    KoRect r = boundingRect();
    if ( r.isNull() )
        return QSize();
    QRect outerRect( m_doc->zoomRect( r ) );
    //kdDebug() << "floatingFrameSize outerRect initially " << DEBUGRECT( outerRect ) << endl;
    QSize sz = outerRect.size();
    Q_ASSERT( m_anchorTextFs );
    // Need to convert back to internal coords (in case of page breaking)
#warning TODO
#if 0
    QPoint iPoint;
    if ( m_anchorTextFs->normalToInternal( outerRect.topLeft(), iPoint ) )
    {
        outerRect.setLeft( iPoint.x() );
        outerRect.setTop( iPoint.y() );
        QPoint brnPoint; // bottom right in internal coords
        if ( m_anchorTextFs->normalToInternal( outerRect.bottomRight(), brnPoint ) )
        {
            outerRect.setRight( brnPoint.x() );
            outerRect.setBottom( brnPoint.y() );
            //kdDebug() << "floatingFrameSize outerRect now " << DEBUGRECT( outerRect ) << endl;
            sz = outerRect.size();
        }
    }
#endif

    // TODO: in theory, we'd need to take the max of the borders of each cell
    // on the outside rect, to find the global rect needed. Well, if we assume constant
    // cell borders for now it's much simpler.
    KWFrame * firstCell = m_cells.getFirst()->getFrame( 0 );
    KWFrame * lastCell = m_cells.getLast()->getFrame( 0 );
    if ( firstCell && lastCell )
    {
        sz.rwidth() += KoBorder::zoomWidthX( firstCell->leftBorder().ptWidth, m_doc, 1 )
                       + KoBorder::zoomWidthX( lastCell->rightBorder().ptWidth, m_doc, 1 );
        sz.rheight() += KoBorder::zoomWidthY( firstCell->topBorder().ptWidth, m_doc, 1 )
                        + KoBorder::zoomWidthY( lastCell->bottomBorder().ptWidth, m_doc, 1 );
    }
    return sz;
}

KCommand * KWTableFrameSet::anchoredObjectCreateCommand( int /*frameNum*/ )
{
    return new KWCreateTableCommand( i18n("Create table"), this );
}

KCommand * KWTableFrameSet::anchoredObjectDeleteCommand( int /*frameNum*/ )
{
    return new KWDeleteTableCommand( i18n("Delete table"), this );
}

KWAnchor * KWTableFrameSet::createAnchor( KWTextDocument * textdoc, int frameNum )
{
    kdDebug() << "KWTableFrameSet::createAnchor" << endl;
    return new KWAnchor( textdoc, this, frameNum );
}

void KWTableFrameSet::createAnchors( KWTextParag * parag, int index, bool placeHolderExists /*= false */ /*only used when loading*/ )
{
    kdDebug() << "KWTableFrameSet::createAnchors" << endl;
    // TODO make one rect per page, and create one anchor per page
    //if ( !m_anchor )
    {
        // Anchor this frame, after the previous one
        KWAnchor * anchor = createAnchor( m_anchorTextFs->kwTextDocument(), 0 );
        if ( !placeHolderExists )
            parag->insert( index, KoTextObject::customItemChar() );
        parag->setCustomItem( index, anchor, 0 );
        kdDebug() << "KWTableFrameSet::createAnchors setting anchor" << endl;
    }
    parag->setChanged( true );
    emit repaintChanged( m_anchorTextFs );
}

void KWTableFrameSet::deleteAnchors()
{
    KWAnchor * anchor = findAnchor( 0 );
    kdDebug() << "KWTableFrameSet::deleteAnchors anchor=" << anchor << endl;
    deleteAnchor( anchor );
}

void KWTableFrameSet::addCell( Cell *cell )
{
    unsigned int i;

    m_rows = QMAX( cell->m_row + 1, m_rows );
    m_cols = QMAX( cell->m_col + 1, m_cols );

    // Find the insertion point in the list.
    for ( i = 0; i < m_cells.count() && m_cells.at( i )->isAboveOrLeftOf( cell->m_row, cell->m_col ); i++ ) ;
    // cell->setName( m_name + ' ' + cell->m_col + ',' + cell->m_row ); // already done in constructor
    m_cells.insert( i, cell );

}

KoRect KWTableFrameSet::boundingRect()
{
    KWFrame *first = getCell( 0, 0 )->getFrame( 0 );
    Q_ASSERT(first);
    Q_ASSERT(getCell( m_rows - 1, m_cols - 1 ));
    //laurent
    //when you insert a column or row
    //we test last cell but you resize anchor
    //each time that you add a cell you execute this function
    //but cell(m_rows - 1, m_cols - 1) doesn't exist
    //the first time.
    //So return koRect() to fix crash
    if(!getCell( m_rows - 1, m_cols - 1 ))
        return KoRect();

    KWFrame *last = getCell( m_rows - 1, m_cols - 1 )->getFrame( 0 );
    Q_ASSERT(last);
    if (!first || !last)
        return KoRect();

    //kdDebug() << "KWTableFrameSet::boundingRect first=" << DEBUGRECT( *first ) << " last=" << DEBUGRECT( *last ) << endl;
    return first->unite( *last );
}

/* returns the cell that occupies row, col. */
KWTableFrameSet::Cell *KWTableFrameSet::getCell( unsigned int row, unsigned int col )
{
    for ( unsigned int i = 0; i < m_cells.count(); i++ )
    {
        Cell *cell = m_cells.at( i );
        if ( cell->m_row <= row &&
                cell->m_col <= col &&
                cell->m_row + cell->m_rows > row &&
                cell->m_col + cell->m_cols > col )
        {
            return cell;
        }
    }
    return 0L;
}

KWTableFrameSet::Cell *KWTableFrameSet::getCellByPos( double x, double y )
{
    KWFrame *f = frameAtPos(x,y);
    if(f) return static_cast<KWTableFrameSet::Cell *> (f->getFrameSet());
    return 0L;
}

bool KWTableFrameSet::isTableHeader( Cell *cell )
{
    return cell->isRemoveableHeader() || ( cell->m_row == 0 );
}

void KWTableFrameSet::recalcCols(int _col,int _row) {
    //kdDebug() << "KWTableFrameSet::recalcCols" << endl;
    if(m_cells.isEmpty()) return; // assertion

    // check/set sizes of frames
    unsigned int row=0,col=0;
    if(_col!=-1 && _row!=-1)
    {
        row=(unsigned int)_row;
        col=(unsigned int)_col;
    }
    else
        isOneSelected(row,col);

    Cell *activeCell = getCell(row,col);
    double difference = 0;

    if(activeCell->getFrame(0)->left() - activeCell->leftBorder() != m_colPositions[activeCell->m_col]) {
        // left border moved.
        col = activeCell->m_row;
        difference = 0-(activeCell->getFrame(0)->left() - activeCell->leftBorder() - m_colPositions[activeCell->m_col]);
    }

    if(activeCell->getFrame(0)->right() - activeCell->rightBorder() !=
            m_colPositions[activeCell->m_col + activeCell->m_cols-1]) { // right border moved

        col = activeCell->m_col + activeCell->m_cols;
        double difference2 = activeCell->getFrame(0)->right() + activeCell->rightBorder() - m_colPositions[activeCell->m_col + activeCell->m_cols];

        double moved=difference2+difference;
        if(moved > -0.01 && moved < 0.01) { // we were simply moved.
                col=0;
                difference = difference2;
            } else if(difference2!=0)
                difference = difference2;
    }

    redrawFromCol=m_cols; // possible reposition col starting with this one, done in recalcRows
    if(difference!=0) {
        double last=col==0?0:m_colPositions[col-1];
        for(unsigned int i=col; i < m_colPositions.count(); i++) {
            double &colPos = m_colPositions[i];
            colPos = colPos + difference;
            if(colPos-last < minFrameWidth) { // Never make it smaller then allowed!
                difference += minFrameWidth - colPos;
                colPos = minFrameWidth + last;
            }
            last=colPos;
        }
        redrawFromCol=col;
        if(col>0) redrawFromCol--;
    }

    //kdDebug() << "end KWTableFrameSet::recalcCols" << endl;
}

void KWTableFrameSet::recalcRows(int _col, int _row) {
    //kdDebug() << "KWTableFrameSet::recalcRows" << endl;

    if(m_cells.isEmpty()) return; // assertion

    // check/set sizes of frames
    unsigned int row=0,col=0;
    if(_col!=-1 && _row!=-1)
    {
        row=(unsigned int)_row;
        col=(unsigned int)_col;
    }
    else
        isOneSelected(row,col);

    Cell *activeCell = getCell(row,col);
    double difference = 0;

    if(activeCell->getFrame(0)->isSelected())
        activeCell->getFrame(0)->setMinFrameHeight(activeCell->getFrame(0)->height());

    if(activeCell->getFrame(0)->top() - activeCell->topBorder() != getPositionOfRow(activeCell->m_row)) {
        // top moved.
        row = activeCell->m_row;
        difference = 0 - (activeCell->getFrame(0)->top() - activeCell->topBorder() - getPositionOfRow(row));
    }


    if(activeCell->getFrame(0)->bottom() - activeCell->bottomBorder() !=
            getPositionOfRow(activeCell->m_row + activeCell->m_rows)) { // bottom moved

        row = activeCell->m_row + activeCell->m_rows;
        double difference2 = activeCell->getFrame(0)->bottom() + activeCell->bottomBorder() - getPositionOfRow(row);
        double moved=difference2+difference;
        if(moved > -0.01 && moved < 0.01) { // we were simply moved.
            row=0;
            difference = difference2;
        } else if( difference2!=0) 
            difference = difference2;
    }

    unsigned int fromRow=m_rows; // possible reposition rows starting with this one, default to no repositioning
    unsigned int untilRow=0;     // possible reposition rows ending with this one
    if(difference!=0) {
        untilRow=m_rows;
        unsigned int adjustment=0;
        QValueList<unsigned int>::iterator pageBound = m_pageBoundaries.begin();
        while(pageBound != m_pageBoundaries.end() && (*pageBound) <= row) {
            adjustment++;
            pageBound++;
        }
        double last=row==0?0:m_rowPositions[row-1];
        for(unsigned int i=row+adjustment; i < m_rowPositions.count(); i++) {
            double &rowPos = m_rowPositions[i];
            rowPos = rowPos + difference;
            if(rowPos-last < minFrameHeight) { // Never make it smaller then allowed!
                difference += minFrameHeight - rowPos;
                rowPos = minFrameHeight + last;
            }
            if(*pageBound == i) {
                untilRow=QMIN(untilRow, *pageBound);
                break;              // stop at pageBreak.
            }
            last=rowPos;
        }
        fromRow=row;
        if(row>0) fromRow--;
    } else {
        row=0;
    }

    double pageHeight = m_doc->ptPaperHeight() - m_doc->ptBottomBorder() - m_doc->ptTopBorder();
    unsigned int pageNumber=getCell(0,0)->getFrame(0)->pageNum() +1;
    unsigned int rowNumber=0;
    QValueList<unsigned int>::iterator pageBound = m_pageBoundaries.begin();
    QValueList<double>::iterator j = m_rowPositions.begin();

    double pageBottom = pageNumber * m_doc->ptPaperHeight() - m_doc->ptBottomBorder();
    while(++j!=m_rowPositions.end()) { // stuff for multipage tables.
        if(pageBound!=m_pageBoundaries.end() && *pageBound == rowNumber ) {
            if(*j > pageNumber * m_doc->ptPaperHeight() - m_doc->ptBottomBorder() ) { // next page marker exists, and is accurate...
                pageNumber++;
                pageBottom = pageNumber * m_doc->ptPaperHeight() - m_doc->ptBottomBorder();
                untilRow=QMAX(untilRow, *pageBound);
                pageBound++;
            } else { // pagebreak marker should be removed, since it is incorrect.
                untilRow=m_rows;
                pageBound=m_pageBoundaries.erase(pageBound);
                QValueList<double>::iterator tmp = j;
                tmp++;
                double diff = *tmp - *j;
                j=m_rowPositions.erase(j);
                tmp=j;
                while(tmp != m_rowPositions.end()) {
                    (*tmp) = (*tmp) - diff; // move the rest of the rows up.
                    tmp++;
                }
                // TODO remove headers and  m_hasTmpHeaders = false;
            }
        }

        if((*j) > pageBottom) { // a row falls off the page.
            untilRow=m_rows;
            bool hugeRow = false;
            unsigned int breakRow = rowNumber;
            for(unsigned int i=0; i < m_cols ; i++) {
                if(getCell(breakRow,i)->m_row < breakRow) {
                    breakRow = getCell(breakRow,i)->m_row;
                    i=0;
                }
            }
            fromRow=QMIN(fromRow, breakRow);
            for(unsigned int i=0; i < m_cols ; i++) {
                if(getCell(breakRow+1,i) && getCell(breakRow+1,i)->getFrame(0)->height() > pageHeight)
                    hugeRow=true;
            }
            if((*pageBound) != breakRow) {
                // voeg bottom in in rowPositions

                double topOfPage = m_doc->ptPaperHeight() * pageNumber + m_doc->ptTopBorder();
                QValueList<double>::iterator tmp = j;
                tmp--;
                double diff = topOfPage - (*tmp); // diff between bottom of last row on page and top of new page
                j=m_rowPositions.insert(j, topOfPage);

                tmp=j;
                while(++tmp!=m_rowPositions.end()) { // move all succesive rows
                    (*tmp) = (*tmp) + diff;
                }

                // voeg nieuwe pageBound toe.
                pageBound = m_pageBoundaries.insert(pageBound, breakRow);
                pageBound++;
                if(!hugeRow) {
                    // add header-rij toe. (en zet bool) TODO
                    //j++;
                    //rowNumber++;
                    // m_hasTmpHeaders = true;
                }
            }
            pageNumber++;
            pageBottom = pageNumber * m_doc->ptPaperHeight() - m_doc->ptBottomBorder();    
        }
        if ( *j >= m_doc->ptPaperHeight() * m_doc->getPages() )
            m_doc->appendPage();

        rowNumber++;
    }

    //kdDebug () << "Repositioning from row : " << fromRow << " until: " << untilRow << endl;
    //kdDebug () << "Repositioning from col > " << redrawFromCol << endl;
    // do positioning.
    Cell *cell;
    bool setMinFrameSize= activeCell->getFrame(0)->isSelected();
    for(cell=m_cells.first();cell;cell=m_cells.next()) {
        if((cell->m_row + cell->m_rows > fromRow && cell->m_row < untilRow) || cell->m_col + cell->m_cols > redrawFromCol)
            if(setMinFrameSize)
                position(cell, cell->m_row <= row-1 && cell->m_row + cell->m_rows > row-1);
            else
                position(cell);
    }
    redrawFromCol=m_cols;

    //kdDebug() << "KWTableFrameSet::recalcRows done" << endl;
}

void KWTableFrameSet::setBoundingRect( KoRect rect ) {
    // Column postions..
    m_rowPositions.clear();
    double colWidth = rect.width() / m_cols;
    if ( m_widthMode == TblAuto ) {
        rect.setLeft( m_doc->ptLeftBorder() );
        colWidth = (m_doc->ptPaperWidth() - m_doc->ptLeftBorder() - m_doc->ptRightBorder()) / m_cols;
    }

    for(unsigned int i=0; i <= m_cols;i++) {
        m_colPositions.append(rect.x() + colWidth * i);
    }

    // Row positions..
    m_rowPositions.clear();
    m_pageBoundaries.clear();
    double rowHeight = 0;
    if( m_heightMode != TblAuto )
        rowHeight = rect.height() / m_rows;
    rowHeight=QMAX(rowHeight, 22); // m_doc->getDefaultParagLayout()->getFormat().ptFontSize()) // TODO use table style font-size

    for(unsigned int i=0; i <= m_rows;i++) {
        m_rowPositions.append(rect.y() + rowHeight * i);
    }

KoBorder border = KoBorder(QColor(), KoBorder::SOLID, 2);

    Cell *theCell;
    double oneMm = MM_TO_POINT( 1.0 );
    for(theCell=m_cells.first(); theCell; theCell=m_cells.next()) {
        KWFrame *frame = theCell->getFrame(0);
        frame->setBLeft( oneMm );
        frame->setBRight( oneMm );
        frame->setBTop( oneMm );
        frame->setBBottom( oneMm );
        frame->setNewFrameBehaviour( KWFrame::NoFollowup );
        position(theCell, true);
    }
}

void KWTableFrameSet::position( Cell *theCell, bool setMinFrameHeight ) {
    if(!theCell->getFrame(0)) { // sanity check.
        kdDebug(32002) << "screwy table cell!! row:" << theCell->m_row << ", col: " << theCell->m_col << endl;
        return;
    }
    double x= *m_colPositions.at(theCell->m_col);
    double y= getPositionOfRow(theCell->m_row);
    double width = (*m_colPositions.at(theCell->m_col + theCell->m_cols)) - x;
    double height  = getPositionOfRow(theCell->m_row + theCell->m_rows-1, true) - y;

    // Now take the border sizes and make the cell smaller so it still fits inside the grid.
    KWFrame *frame = theCell->getFrame(0);
    x+=theCell->leftBorder();
    width-=theCell->leftBorder();
    width-=theCell->rightBorder();
    y+=theCell->topBorder();
    height-=theCell->topBorder();
    height-=theCell->bottomBorder();

    frame->setRect( x,y,width,height);
    if( setMinFrameHeight ) {
        frame->setMinFrameHeight(height);
    }
}

double KWTableFrameSet::getPositionOfRow( unsigned int row, bool bottom ) {
    unsigned int adjustment=0;
    QValueList<unsigned int>::iterator pageBound = m_pageBoundaries.begin();
    while(pageBound != m_pageBoundaries.end() && (*pageBound) <= row) {
        adjustment++;
        pageBound++;
    }
    if(m_rowPositions.count() < row+adjustment+(bottom?1:0))  // Requested row does not exist.
        return 0;
    return m_rowPositions[row+adjustment+(bottom?1:0)];
}

void KWTableFrameSet::setHeightMode( CellSize mode )
{
    m_heightMode = mode;
}

void KWTableFrameSet::setWidthMode( CellSize mode )
{
    m_widthMode = mode;
}

bool KWTableFrameSet::hasSelectedFrame()
{
    unsigned int a=0,b=0;
    return getFirstSelected(a,b);
}

void KWTableFrameSet::moveBy( double dx, double dy )
{
    if(dx==0 && dy==0)
        return;
    kdDebug() << "KWTableFrameSet::moveBy " << dx << "," << dy << endl;
    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        m_cells.at( i )->getFrame( 0 )->moveBy( dx, dy );
        if(!m_cells.at( i )->isVisible())
            m_cells.at( i )->setVisible(true);
    }
    m_doc->updateAllFrames();

/* leads to resizing while formatting, for inline tables, which qrt doesn't support
    recalcCols();
    recalcRows();
    */
}

/*void KWTableFrameSet::drawAllRects( QPainter &p, int xOffset, int yOffset )
{
    KWFrame *frame;

    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        frame = m_cells.at( i )->getFrame( 0 );
        QRect tmpRect(frame->x() - xOffset,  frame->y() - yOffset, frame->width(), frame->height());
        p.drawRect( m_doc->zoomRect(tmpRect) );
    }
}*/


void KWTableFrameSet::deselectAll()
{
    for ( unsigned int i = 0; i < m_cells.count(); i++ )
        m_cells.at( i )->getFrame( 0 )->setSelected( false );
}

void KWTableFrameSet::refreshSelectedCell()
{
    unsigned int row,col;
    if ( !isOneSelected( row, col ) )
        return;

    Cell *cell=getCell(row,col);
    if(cell)
        cell->getFrame( 0 )->updateResizeHandles();
}

void KWTableFrameSet::selectUntil( double x, double y) {
    KWFrame *f = frameAtPos(x,y);
    if(f) selectUntil(static_cast<KWTableFrameSet::Cell *> (f->getFrameSet()));
}

/* the selectUntil method will select all frames from the first
   selected to the frame of the argument frameset.
*/
void KWTableFrameSet::selectUntil( Cell *cell)
{
    unsigned int toRow = 0, toCol = 0;
    toRow=cell->m_row + cell->m_rows -1;
    toCol=cell->m_col + cell->m_cols -1;

    unsigned int fromRow = 0, fromCol = 0;
    getFirstSelected( fromRow, fromCol );
    if(cell->m_cols!=1)
        fromCol=QMIN(fromCol,cell->m_col);
    if(cell->m_rows!=1)
        fromRow=QMIN(fromRow,cell->m_row);



    if ( fromRow > toRow ) { // doSwap
        fromRow = fromRow^toRow;
        toRow = fromRow^toRow;
        fromRow = fromRow^toRow;
    }

    if ( fromCol > toCol ) { // doSwap
        fromCol = fromCol^toCol;
        toCol = fromCol^toCol;
        fromCol = fromCol^toCol;
    }

    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        cell = m_cells.at(i);
        // check if cell falls completely in square.
        unsigned int row = cell->m_row + cell->m_rows -1;
        unsigned int col = cell->m_col + cell->m_cols -1;
        if(row >= fromRow && row <= toRow && col >= fromCol && col <= toCol)
        {
            cell->getFrame( 0 )->setSelected( true );
            cell->getFrame(0)->createResizeHandles();
            cell->getFrame(0)->updateResizeHandles();
        }
        else
        {
            if(cell->getFrame( 0 )->isSelected())
            {
                cell->getFrame( 0 )->setSelected( false );
                cell->getFrame(0)->removeResizeHandles();
            }
        }
    }
}

/* Return true if exactly one frame is selected. The parameters row
   and col will receive the values of the active row and col.
   When no frame or more then one frame is selected row and col will
   stay unchanged (and false is returned).
*/
bool KWTableFrameSet::isOneSelected(unsigned int &row, unsigned int &col) {
    int selectedCell=-1;
    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        if(m_cells.at(i)->getFrame(0)->isSelected())  {
            if(selectedCell==-1)
                selectedCell=i;
            else
                selectedCell=m_cells.count()+1;
        }
    }
    if(selectedCell>=0 && selectedCell<= static_cast<int> (m_cells.count())) {
        row=m_cells.at(selectedCell)->m_row;
        col=m_cells.at(selectedCell)->m_col;
        return true;
    }
    return false;
}

/* returns true if at least one is selected, excluding the argument frameset.
*/
bool KWTableFrameSet::getFirstSelected( unsigned int &row, unsigned int &col )
{
    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        if (m_cells.at( i )->getFrame( 0 )->isSelected()) {
            row = m_cells.at( i )->m_row;
            col = m_cells.at( i )->m_col;
            return true;
        }
    }
    return false;
}

void KWTableFrameSet::insertRow( unsigned int newRowNumber,QPtrList<KWFrameSet> redoFrameset, QPtrList<KWFrame> redoFrame,bool recalc, bool isAHeader ) {

    unsigned int copyFromRow = newRowNumber==0?0:newRowNumber-1;
    if(newRowNumber==0)
        copyFromRow=1;

    // What height to use for the new row
    double height = 0;
    if(redoFrame.isEmpty())
        height = getPositionOfRow(copyFromRow,true) - getPositionOfRow(copyFromRow);
    else {
        KWFrame *f = redoFrame.at(0);
        height = f->height() + f->topBorder().ptWidth + f->bottomBorder().ptWidth;;
    }

    // Calculate offset in QValueList because of page breaks.
    unsigned int adjustment=0;
    unsigned int untilRow=m_rows;
    QValueList<unsigned int>::iterator pageBound = m_pageBoundaries.begin();
    while(pageBound != m_pageBoundaries.end() && (*pageBound) <= newRowNumber) {
        // Find out how many pages we allready had.
        adjustment++;
        pageBound++;
    }

    // Move all rows down from newRow to bottom of page
    QValueList<double>::iterator tmp = m_rowPositions.at(newRowNumber);
    double newPos = *tmp + height;
    tmp++;
    m_rowPositions.insert(tmp, newPos);
    for(unsigned int i=newRowNumber+adjustment+2; i < m_rowPositions.count(); i++) {
        double &rowPos = m_rowPositions[i];
kdDebug() << "adjusting " << rowPos << " -> " << rowPos + height << endl;
        rowPos = rowPos + height;
        if(*pageBound == i) {
            untilRow= *pageBound;
            break;              // stop at pageBreak.
        }
    }

    // Position all changed cells.
    unsigned int newRows=++m_rows;
    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        Cell *cell = m_cells.at(i);
        if ( cell->m_row >= newRowNumber ) { // move all cells beneath the new row.
            cell->m_row++;
            position(cell);
        }
    }

    unsigned int i =0;
    double oneMm = MM_TO_POINT( 1.0 );
    while(i < m_cols) {
        KWFrame *frame = 0L;
        if(redoFrame.isEmpty())
        {
            frame=new KWFrame(0L, 1, 1, 100, 20, KWFrame::RA_NO); // use dummy values here...
            frame->setFrameBehaviour(KWFrame::AutoExtendFrame);
            frame->setNewFrameBehaviour(KWFrame::NoFollowup);
        }
        else
        {
            if(i<redoFrame.count())
                frame=redoFrame.at(i)->getCopy();
        }

        Cell *newCell=0L;
        if(redoFrameset.isEmpty())
            newCell=new Cell( this, newRowNumber, i, QString::null );
        else
        {
            if( i<redoFrameset.count())
            {
                newCell = static_cast<KWTableFrameSet::Cell*> (redoFrameset.at(i));
                addCell( newCell );
            }
            else
            {
                newCell =getCell(newRowNumber-1,i);
                newCell->m_rows=newCell->m_rows+1;
                continue;
            }
        }

        newCell->m_cols=getCell(copyFromRow,i)->m_cols;
        newCell->setIsRemoveableHeader( isAHeader );
        newCell->addFrame( frame, false );
        frame->setBLeft( oneMm );
        frame->setBRight( oneMm );
        frame->setBTop( oneMm );
        frame->setBBottom( oneMm );
        position(newCell);

        i+=newCell->m_cols;
    }

    m_rows=newRows;
    if ( recalc )
        finalize();
}

void KWTableFrameSet::insertCol( unsigned int newColNumber,QPtrList<KWFrameSet> redoFrameset, QPtrList<KWFrame>redoFrame ) {
    double width=60;
    if(! redoFrame.isEmpty()) {
        KWFrame *f=redoFrameset.at(0)->getFrame(0);
        width=f->width() + f->leftBorder().ptWidth + f->rightBorder().ptWidth;
    }
    unsigned int _cols = m_cols++;

    QValueList<double>::iterator tmp = m_colPositions.at(newColNumber);

    tmp=m_colPositions.insert(tmp, *tmp);
    tmp++;
    while(tmp!=m_colPositions.end()) {
        (*tmp)= (*tmp)+width;
        tmp++;
    }

    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        Cell *cell = m_cells.at(i);
        if ( cell->m_col >= newColNumber) { // move all cells beneath the new row.
            cell->m_col++;
            position(cell);
        }
    }

    for( unsigned int i = 0; i < getRows(); i++ ) {
        int rows;
        double height;
        Cell *cell;
        if(newColNumber > 0 ) {
            cell = getCell(i, newColNumber-1);
            if(cell->m_col + cell->m_cols > newColNumber) {
                // cell overlaps the new column
                cell->m_cols++;
                cell->getFrame(0)->setWidth(cell->getFrame(0)->width() + width + tableCellSpacing - 1);
                continue;
            }
            rows = cell->m_rows;
            height = cell->getFrame(0)->height();
        } else {
            rows = 1;
            cell = getCell(i, newColNumber+1);
            height = cell->getFrame(0)->height();
        }
        Cell *newCell=0L;
        if(redoFrameset.isEmpty())
            newCell = new Cell( this, i, newColNumber, QString::null );
        else
        {
            if( i<redoFrameset.count())
            {
                newCell = static_cast<KWTableFrameSet::Cell*> (redoFrameset.at(i));
                addCell( newCell );
            }
            else
            {
                newCell =getCell(i,newColNumber-1);
                newCell->m_cols=newCell->m_cols+1;
                continue;
            }
        }
        KWFrame *frame = 0L;
        if(redoFrame.isEmpty())
        {
            frame=new KWFrame(newCell, 1, 1, width, 20, KWFrame::RA_NO ); // dummy values..
            frame->setFrameBehaviour(KWFrame::AutoExtendFrame);
        }
        else
        {
            frame=redoFrame.at(i)->getCopy();
        }
        newCell->addFrame( frame,false );
        if(cell->m_rows >1) {
            newCell->m_rows = cell->m_rows;
            i+=cell->m_rows -1;
        }
        position(newCell);
    }

    m_cols = ++_cols;
    finalize();
}

/* Delete all cells that are completely in this row.              */

void KWTableFrameSet::deleteRow( unsigned int row, bool _recalc )
{
    double height=0;
    unsigned int rowspan=1;
    // I want to know the height of the row(s) I am removing.
    for (unsigned int rowspan=1; rowspan < m_rows && height==0; rowspan++) {
        for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
            if(m_cells.at(i)->m_row == row && m_cells.at(i)->m_rows==rowspan) {
                height=m_cells.at(i)->getFrame(0)->height();
                break;
            }
        }
    }

    QValueList<double>::iterator tmp = m_rowPositions.at(row+1);
    tmp=m_rowPositions.erase(tmp);
    while(tmp!=m_rowPositions.end()) {
        (*tmp)= (*tmp)-height;
        tmp++;
    }

    // move/delete cells.
    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        Cell *cell = m_cells.at(i);
        if ( row >= cell->m_row  && row < cell->m_row + cell->m_rows) { // cell is indeed in row
            if(cell->m_rows == 1) { // lets remove it
                frames.remove( cell->getFrame(0) );
                cell->delFrame( cell->getFrame(0));
                m_cells.take(i);
                i--;
            } else { // make cell span rowspan less rows
                cell->m_rows -= rowspan;
                position(cell);
            }
        } else if ( cell->m_row > row ) {
            // move cells to the left
            cell->m_row -= rowspan;
            position(cell);
        }
    }
    m_rows -= rowspan;

    if ( _recalc )
        recalcRows();
}

/* Delete all cells that are completely in this col.              */
void KWTableFrameSet::deleteCol( unsigned int col )
{
    double width=0;
    unsigned int colspan=1;
    // I want to know the width of the col(s) I am removing.
    for (unsigned int colspan=1; colspan < m_cols && width==0; colspan++) {
        for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
            if(m_cells.at(i)->m_col == col && m_cells.at(i)->m_cols==colspan) {
                width=m_cells.at(i)->getFrame(0)->width();
                break;
            }
        }
    }

    QValueList<double>::iterator tmp = m_colPositions.at(col+1);
    tmp=m_colPositions.erase(tmp);
    while(tmp!=m_colPositions.end()) {
        (*tmp)= (*tmp)-width;
        tmp++;
    }

    // move/delete cells.
    for ( unsigned int i = 0; i < m_cells.count(); i++ ) {
        Cell *cell = m_cells.at(i);
        if ( col >= cell->m_col  && col < cell->m_col + cell->m_cols) { // cell is indeed in col
            if(cell->m_cols == 1) { // lets remove it
                frames.remove( cell->getFrame(0) );
                cell->delFrame( cell->getFrame(0));
                //m_cells.remove(  i);
                m_cells.take(i);
                i--;
            } else { // make cell span colspan less cols
                cell->m_cols -= colspan;
                cell->getFrame(0)->setWidth(
                        cell->getFrame(0)->width() - width - (colspan-1) * tableCellSpacing);
            }
        } else if ( cell->m_col > col ) {
            // move cells to the left
            cell->m_col -= colspan;
            cell->getFrame(0)->moveBy( -width, 0);
        }
    }
    m_cols -= colspan;

    recalcCols();
    recalcRows();
}

void KWTableFrameSet::ungroup()
{
    m_cells.setAutoDelete( false );
    m_cells.clear();

    m_active = false;
}

void KWTableFrameSet::group()
{
   m_cells.setAutoDelete( true );
   m_cells.clear();

   m_active = true;
}

KCommand *KWTableFrameSet::joinCells(unsigned int colBegin,unsigned int rowBegin, unsigned int colEnd,unsigned int rowEnd) {
    kdDebug()<<"colBegin :"<<colBegin<<" rowBegin :"<<rowBegin<<" colEnd :"<<colEnd<<" rowEnd :"<<rowEnd<<endl;
    Cell *firstCell = getCell(rowBegin, colBegin);
    if(colBegin==0 && rowBegin==0 && colEnd==0 && rowEnd==0)
    {

        if ( !getFirstSelected( rowBegin, colBegin ) )
            return 0L;

        firstCell = getCell(rowBegin, colBegin);
        colEnd=colBegin+firstCell->m_cols-1;
        rowEnd=rowBegin+firstCell->m_rows-1;

        while(colEnd+1 <getCols()) { // count all horizontal selected cells
            Cell *cell = getCell(rowEnd,colEnd+1);
            if(cell->getFrame(0)->isSelected()) {
                colEnd+=cell->m_cols;
            } else
                break;
        }

        while(rowEnd+1 < getRows()) { // count all vertical selected cells
            Cell *cell = getCell(rowEnd+1, colBegin);
            if(cell->getFrame(0)->isSelected()) {
                for(unsigned int j=1; j <= cell->m_rows; j++) {
                    for(unsigned int i=colBegin; i<=colEnd; i++) {
                        if(! getCell(rowEnd+j,i)->getFrame(0)->isSelected())
                            return 0L; // can't use this selection..
                    }
                }
                rowEnd+=cell->m_rows;
            } else
                break;
        }
        // if just one cell selected for joining; exit.
        if(rowBegin == rowEnd && colBegin == colEnd ||
           getCell(rowBegin,colBegin) == getCell(rowEnd,colEnd))
            return 0L;
    }

    QPtrList<KWFrameSet> listFrameSet;
    QPtrList<KWFrame> listCopyFrame;

    // do the actual merge.
    for(unsigned int i=colBegin; i<=colEnd;i++) {
        for(unsigned int j=rowBegin; j<=rowEnd;j++) {
            Cell *cell = getCell(j,i);
            if(cell && cell!=firstCell) {
                listFrameSet.append(cell);
                listCopyFrame.append(cell->getFrame(0)->getCopy());
                frames.remove( cell->getFrame(0) );
                cell->delFrame( cell->getFrame(0));
                m_cells.take(m_cells.find(cell));
            }
        }
    }

    Q_ASSERT(firstCell);
    // update firstcell properties to reflect the merge
    firstCell->m_cols=colEnd-colBegin+1;
    firstCell->m_rows=rowEnd-rowBegin+1;
    position(firstCell);
    firstCell->getFrame(0)->updateResizeHandles();

    m_doc->updateAllFrames();
    m_doc->repaintAllViews();
    return new KWJoinCellCommand( i18n("Join Cells"), this,colBegin,rowBegin, colEnd,rowEnd,listFrameSet,listCopyFrame);
}

KCommand *KWTableFrameSet::splitCell(unsigned int intoRows, unsigned int intoCols, int _col, int _row,QPtrList<KWFrameSet> listFrameSet, QPtrList<KWFrame>listFrame) {
    //kdDebug()<<"intoRows :"<<intoRows<<" intoCols :"<< intoCols <<" _col :"<<_col<<" _row "<<_row<<" listFrameSet :"<<listFrameSet.count()<<" listFrame :"<<listFrame.count()<<endl;
    if(intoRows < 1 || intoCols < 1)
        return 0L;

    unsigned int col, row;
    if(_col!=-1 && _row!=-1)
    {
        row=_row;
        col=_col;
    }
    else
    {
        if ( !isOneSelected( row, col ) )
            return 0L;
    }


    Cell *cell=getCell(row,col);
    KWFrame *firstFrame = cell->getFrame(0);
    // unselect frame.
    firstFrame->setSelected(false);
    firstFrame->removeResizeHandles();

    double height = (firstFrame->height() -  tableCellSpacing * (intoRows-1)) / intoRows ;
    double width = (firstFrame->width() -  tableCellSpacing * (intoCols-1))/ intoCols  ;
    double extraHeight = 0;

    // will it fit?
    if(width < minFrameWidth) return 0L;
    if(height < minFrameHeight) {
        extraHeight = minFrameHeight * intoRows - firstFrame->height();
        height = minFrameHeight;
    }

    firstFrame->setWidth(width);
    firstFrame->setHeight(height);

    int rowsDiff = intoRows-cell->m_rows;
    int colsDiff = ((int) intoCols)-cell->m_cols;

    // adjust cellspan and rowspan on other cells.
    for (unsigned int i=0; i< m_cells.count() ; i++) {
        Cell *theCell = m_cells.at(i);
        if(cell == theCell) continue;

        if(rowsDiff>0) {
            if(row >= theCell->m_row && row < theCell->m_row + theCell->m_rows)
                theCell->m_rows+=rowsDiff;
            if(theCell->m_row > row) {
                theCell->m_row+=rowsDiff;
                theCell->getFrame(0)->setTop(theCell->getFrame(0)->top()+extraHeight);
        }
        }
        if(colsDiff>0) {
            if(col >= theCell->m_col && col < theCell->m_col + theCell->m_cols)
                theCell->m_cols+=colsDiff;
            if(theCell->m_col > col) theCell->m_col+=colsDiff;
        }
        if(extraHeight != 0 && theCell->m_row == row) {
            theCell->getFrame(0)->setHeight(theCell->getFrame(0)->height()+extraHeight);
        }
    }

    // set new row and col-span. Use intermediate ints otherwise we get strange results as the
    // intermediate result could be negative (which goes wrong with unsigned ints)
    int r = (cell->m_rows +1) - intoRows;
    if(r < 1) r=1;
    cell->m_rows = r;

    int c = (cell->m_cols + 1) - intoCols;
    if(c < 1)  c=1;
    cell->m_cols = c;

    // If we created extra rows/cols, adjust the groupmanager counters.
    if(rowsDiff>0) m_rows+= rowsDiff;
    if(colsDiff>0) m_cols+= colsDiff;
    int i=0;
    // create new cells
    for (unsigned int y = 0; y < intoRows; y++) {
        for (unsigned int x = 0; x < intoCols; x++){
            if(x==0 && y==0)
                continue; // the orig cell takes this spot.

            Cell *lastFrameSet=0L;

            if(listFrameSet.isEmpty())
            {
                lastFrameSet= new Cell( this, y + row, x + col );
                lastFrameSet->m_rows = 1;
                lastFrameSet->m_cols = 1;
            }
            else
            {
                lastFrameSet = static_cast<KWTableFrameSet::Cell*> (listFrameSet.at(i));
                addCell( lastFrameSet );
            }

            KWFrame *frame=0L;
            if(listFrame.isEmpty())
            {
                frame=firstFrame->getCopy();
                frame->setRect(firstFrame->left() + static_cast<double>((width+tableCellSpacing) * x),
                               firstFrame->top() + static_cast<double>((height+tableCellSpacing) * y),
                               width, height);
                frame->setRunAround( KWFrame::RA_NO );
                frame->setFrameBehaviour(KWFrame::AutoExtendFrame);
                frame->setNewFrameBehaviour(KWFrame::NoFollowup);
                lastFrameSet->addFrame( frame,false );
            }
            else
                lastFrameSet->addFrame( listFrame.at(i)->getCopy(),false );
            i++;

            // if the orig cell spans more rows/cols than it is split into, make first col/row wider.
            if(rowsDiff <0 && y==0)
                lastFrameSet->m_rows -=rowsDiff;
            if(colsDiff <0 && x==0)
                lastFrameSet->m_cols -=colsDiff;
        }
    }

    finalize();

    return new KWSplitCellCommand(i18n("Split Cells"),this,col,row,intoCols, intoRows);
}

void KWTableFrameSet::viewFormatting( QPainter &/*painter*/, int )
{
}

/* checks the cells for missing cells or duplicates, will correct
   mistakes.
*/
void KWTableFrameSet::validate()
{
    for (unsigned int j=0; j < getNumCells() ; j++) {
        KWFrame *frame = getCell(j)->getFrame(0);
        if(frame->getFrameBehaviour()==KWFrame::AutoCreateNewFrame) {
            frame->setFrameBehaviour(KWFrame::AutoExtendFrame);
            kdWarning() << "Table cell property frameBehaviour was incorrect; fixed" << endl;
        }
        if(frame->getNewFrameBehaviour()!=KWFrame::NoFollowup) {
            kdWarning() << "Table cell property newFrameBehaviour was incorrect; fixed" << endl;
            frame->setNewFrameBehaviour(KWFrame::NoFollowup);
        }
    }

    QPtrList<Cell> misplacedCells;

    for(unsigned int row=0; row < getRows(); row++) {
        for(unsigned int col=0; col <getCols(); col++) {
            bool found=false;
            for ( unsigned int i = 0; i < m_cells.count(); i++ )
            {
                if ( m_cells.at( i )->m_row <= row &&
                     m_cells.at( i )->m_col <= col &&
                     m_cells.at( i )->m_row+m_cells.at( i )->m_rows > row &&
                     m_cells.at( i )->m_col+m_cells.at( i )->m_cols > col )
                {
                    if(found==true)
                    {
                        kdWarning() << "Found duplicate cell, (" << m_cells.at(i)->m_row << ", " << m_cells.at(i)->m_col << ") moving one out of the way" << endl;
                        misplacedCells.append(m_cells.take(i--));
                    }
                    found=true;
                }
            }
            if(! found) {
                kdWarning() << "Missing cell, creating a new one; ("<< row << "," << col<<")" << endl;
                // worth it ?
                //QString name = m_doc->generateFramesetName( i18n( "1 is table name, 2 is a number", "%1 Auto Added Cell %2" ).arg( getName() ) );
                Cell *_frameSet = new Cell( this, row, col );
                double x=-1, y=-1, width=-1, height=-1;
                for (unsigned int i=0; i < m_cells.count(); i++) {
                    if(m_cells.at(i)->m_row==row)
                        y=m_cells.at(i)->getFrame(0)->y();
                    if(m_cells.at(i)->m_col==col)
                        x=m_cells.at(i)->getFrame(0)->x();
                    if(m_cells.at(i)->m_col==col && m_cells.at(i)->m_cols==1)
                        width=m_cells.at(i)->getFrame(0)->width();
                    if(m_cells.at(i)->m_row==row && m_cells.at(i)->m_rows==1)
                        height=m_cells.at(i)->getFrame(0)->height();
                    if(x!=-1 && y!=-1 && width!=-1 && height != -1)
                        break;
                }
                if(x== -1) x=0;
                if(y== -1) y=0;
                if(width== -1) width=minFrameWidth;
                if(height== -1) height=minFrameHeight;
                kdWarning() << " x: " << x << ", y:" << y << ", width: " << width << ", height: " << height << endl;
                KWFrame *frame = new KWFrame(_frameSet, x, y, width, height, KWFrame::RA_NO );
                frame->setFrameBehaviour(KWFrame::AutoExtendFrame);
                frame->setNewFrameBehaviour(KWFrame::NoFollowup);
                _frameSet->addFrame( frame,false );
                _frameSet->m_rows = 1;
                _frameSet->m_cols = 1;
            }
        }
    }
    double bottom = getCell(m_rows-1,0)->getFrame(0)->bottom();
    while (! misplacedCells.isEmpty()) {
        // append cell at bottom of table.
        Cell *cell = misplacedCells.take(0);
        cell->getFrame(0)->setWidth(boundingRect().width());
        cell->getFrame(0)->moveBy( boundingRect().left() -
                                             cell->getFrame(0)->left(),
                                             bottom - cell->getFrame(0)->top() - tableCellSpacing);
        cell->m_row = m_rows++;
        cell->m_col = 0;
        cell->m_cols = m_cols;
        cell->m_rows = 1;
        bottom=cell->getFrame(0)->bottom();
        m_cells.append(cell);
    }
}

bool KWTableFrameSet::contains( double mx, double my ) {
    /*  m_pageBoundaries is a list of integers.
        The list contains an entry for each page and that entry points to the first cell
        on that page.
        The list contains integers of the index of the cell in the m_cells list.

        the following points to the first cell of the first page.
          m_cells->at(m_pageBoundaries[0])

        The last entry in the list points to the last+1 cell (the cell on a non-existing
        extra page) Beware that this cell does no really exist! This entry is there so we
        can always get the last cell on a page by substracting 1 from the firs cell of the
        next page. (Well, in theory anyway, untill someone joins the last cell of a page ;)
    */
    if(m_cells.count()==0)
        return false;
    if(m_pageBoundaries.count() ==0)
        recalcRows();
    KWFrame *first, *last;
    for (unsigned int i=1 ; i < m_pageBoundaries.count(); i++) {

        first = m_cells.at((m_pageBoundaries[i-1]))->getFrame( 0 );
        if(m_pageBoundaries[i] != 0)
        {
            KWTableFrameSet::Cell *cell=m_cells.at(m_pageBoundaries[i] -1);
            //not cell at right
            if((cell->m_cols+cell->m_col<m_cols-1) || (cell->m_col<m_cols-1) )
                cell=getCell(cell->m_row, m_cols-1);
            last = cell->getFrame( 0 );
        }
        else
            last = first;


        KoRect rect( KoPoint( first->x(), first->y() ), KoPoint( last->right(), last->bottom() ) );
        if(rect.contains(mx,my)) {
            return true;
        }
    }

    return false;
}

void KWTableFrameSet::createEmptyRegion( const QRect & crect, QRegion & emptyRegion, KWViewMode *viewMode )
{
    // Avoid iterating over all cells if we are out of view
    if ( !viewMode->normalToView( m_doc->zoomRect( boundingRect() ) ).intersects( crect ) )
        return;
    QPtrListIterator<KWFrame> frameIt = frameIterator();
    for ( ; frameIt.current(); ++frameIt )
    {
        QRect outerRect( viewMode->normalToView( frameIt.current()->outerRect() ) );
        //kdDebug() << "KWTableFrameSet::createEmptyRegion outerRect=" << DEBUGRECT( outerRect )
        //          << " crect=" << DEBUGRECT( crect ) << endl;
        outerRect &= crect;
        if ( !outerRect.isEmpty() )
            emptyRegion = emptyRegion.subtract( outerRect );
    }
}

void KWTableFrameSet::drawBorders( QPainter& painter, const QRect &crect, KWViewMode *viewMode, KWCanvas *canvas )
{
    painter.save();

    // We need 'i'
    //QPtrListIterator<KWFrame> frameIt = frameIterator();
    //for ( ; frameIt.current(); ++frameIt )
    bool topOfPage = true;
    for(uint i = 0; i < m_cells.count(); i++)
    {
        Cell *cell = m_cells.at(i);
        //KWFrame *frame = frameIt.current();
        KWFrame *frame = cell->getFrame( 0 );
        if ( !frame )
            continue;

        // This sets topOfPage for the first cell that is on top of a page,
        // _leaves_ it to true for the other cells in the same row,
        // and resets it to false at the next cell in the first column.
        if ( m_pageBoundaries.contains( i ) )
            topOfPage = true;
        else if ( cell->m_col == 0 )
            topOfPage = false;

        //kdDebug() << "KWTableFrameSet::drawBorders i=" << i << " row=" << cell->m_row << " col=" << cell->m_col
        //          << " topOfPage=" << topOfPage << endl;

        QRect outerRect( viewMode->normalToView( frame->outerRect() ) );
        if ( !crect.intersects( outerRect ) )
            continue;

        QRect rect( viewMode->normalToView( m_doc->zoomRect( *frame ) ) );
        // Set the background color.
        QBrush bgBrush( frame->getBackgroundColor() );
	bgBrush.setColor( KWDocument::resolveBgColor( bgBrush.color(), &painter ) );
        painter.setBrush( bgBrush );

        // Draw default borders using view settings...
        QPen defaultPen( lightGray ); // TODO use qcolorgroup
        // ...except when printing, or embedded doc, or disabled.
        if ( ( painter.device()->devType() == QInternal::Printer ) ||
             !canvas || !canvas->gui()->getView()->viewFrameBorders() )
        {
            defaultPen.setColor( bgBrush.color() );
        }

        // Draw borders either as the user defined them, or using the view settings.
        // Borders should be drawn _outside_ of the frame area
        // otherwise the frames will erase the border when painting themselves.

        //    Border::drawBorders( painter, m_doc, frameRect,
        //         frame->leftBorder(), frame->rightBorder(),
        //         frame->topBorder(), frame->bottomBorder(),
        //         1, viewSetting );

        const int minborder = 1;

        // ###### We'll need some code to ensure that this->rightborder == cell_on_right->leftborder etc.
        KoBorder topBorder = frame->topBorder();
        KoBorder bottomBorder = frame->bottomBorder();
        KoBorder leftBorder = frame->leftBorder();
        KoBorder rightBorder = frame->rightBorder();
        int topBorderWidth = KoBorder::zoomWidthY( topBorder.ptWidth, m_doc, minborder );
        int bottomBorderWidth = KoBorder::zoomWidthY( bottomBorder.ptWidth, m_doc, minborder );
        int leftBorderWidth = KoBorder::zoomWidthX( leftBorder.ptWidth, m_doc, minborder );
        int rightBorderWidth = KoBorder::zoomWidthX( rightBorder.ptWidth, m_doc, minborder );

        QColor defaultColor = KoTextFormat::defaultTextColor( &painter );

        if ( topOfPage )  // draw top only for 1st row on every page.
            if ( topBorderWidth > 0 )
            {
                if ( topBorder.ptWidth > 0 )
                    painter.setPen( KoBorder::borderPen( topBorder, topBorderWidth, defaultColor ) );
                else
                    painter.setPen( defaultPen );
                int y = rect.top() - topBorderWidth + topBorderWidth/2;
                painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+rightBorderWidth, y );
            }
        if ( bottomBorderWidth > 0 )
        {
            if ( bottomBorder.ptWidth > 0 )
                painter.setPen( KoBorder::borderPen( bottomBorder, bottomBorderWidth, defaultColor ) );
            else
                painter.setPen( defaultPen );
            int y = rect.bottom() + bottomBorderWidth - bottomBorderWidth/2;
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+rightBorderWidth, y );
        }
        if ( cell->m_col == 0 ) // draw left border only for 1st column.
            if ( leftBorderWidth > 0 )
            {
                if ( leftBorder.ptWidth > 0 )
                    painter.setPen( KoBorder::borderPen( leftBorder, leftBorderWidth, defaultColor ) );
                else
                    painter.setPen( defaultPen );
                int x = rect.left() - leftBorderWidth + leftBorderWidth/2;
                painter.drawLine( x, rect.top()-topBorderWidth, x, rect.bottom()+bottomBorderWidth );
            }
        if ( rightBorderWidth > 0 )
        {
            if ( rightBorder.ptWidth > 0 )
                painter.setPen( KoBorder::borderPen( rightBorder, rightBorderWidth, defaultColor ) );
            else
                painter.setPen( defaultPen );
            int x = rect.right() + rightBorderWidth - rightBorderWidth/2;
            painter.drawLine( x, rect.top()-topBorderWidth, x, rect.bottom()+bottomBorderWidth );
        }
    }
    painter.restore();
}

void KWTableFrameSet::drawContents( QPainter * painter, const QRect & crect,
                                    QColorGroup & cg, bool onlyChanged, bool resetChanged,
                                    KWFrameSetEdit * edit, KWViewMode * viewMode, KWCanvas *canvas )
{
    drawBorders( *painter, crect, viewMode, canvas );
    for (unsigned int i=0; i < m_cells.count() ; i++)
    {
        if (edit)
        {
            KWTableFrameSetEdit * tableEdit = static_cast<KWTableFrameSetEdit *>(edit);
            if ( tableEdit->currentCell() && m_cells.at(i) == tableEdit->currentCell()->frameSet() )
            {
                m_cells.at(i)->drawContents( painter, crect, cg, onlyChanged, resetChanged, tableEdit->currentCell(), viewMode, canvas );
                continue;
            }
        }
        m_cells.at(i)->drawContents( painter, crect, cg, onlyChanged, resetChanged, 0L, viewMode, canvas );
    }

}

void KWTableFrameSet::zoom( bool forPrint ) {
    for (unsigned int i =0; i < m_cells.count(); i++) {
        m_cells.at(i)->zoom( forPrint );
    }
}

void KWTableFrameSet::preparePrinting( QPainter *painter, QProgressDialog *progress, int &processedParags ) {
    for (unsigned int i =0; i < m_cells.count(); i++) {
        m_cells.at(i)->preparePrinting( painter, progress, processedParags );
    }
}

QDomElement KWTableFrameSet::save( QDomElement &parentElem, bool saveFrames ) {
    // When saving to a file, we don't have anything specific to the frameset to save.
    // Save the cells only.
    for (unsigned int i =0; i < m_cells.count(); i++) {
        m_cells.at(i)->save(parentElem, saveFrames);
    }
    return QDomElement(); // No englobing element for tables...
}

QDomElement KWTableFrameSet::toXML( QDomElement &parentElem, bool saveFrames )
{
    QDomElement framesetElem = parentElem.ownerDocument().createElement( "FRAMESET" );
    parentElem.appendChild( framesetElem );
    KWFrameSet::saveCommon( framesetElem, false ); // Save the frameset attributes
    // Save the cells
    save( framesetElem, saveFrames );
    return framesetElem;
}

void KWTableFrameSet::fromXML( QDomElement &framesetElem, bool loadFrames, bool useNames )
{
    KWFrameSet::load( framesetElem, false ); // Load the frameset attributes
    // Load the cells
    QDomElement cellElem = framesetElem.firstChild().toElement();
    for ( ; !cellElem.isNull() ; cellElem = cellElem.nextSibling().toElement() )
    {
        if ( cellElem.tagName() == "FRAMESET" )
            loadCell( cellElem, loadFrames, useNames );
    }
}

KWTableFrameSet::Cell* KWTableFrameSet::loadCell( QDomElement &framesetElem, bool loadFrames, bool useNames )
{
    int _row = KWDocument::getAttribute( framesetElem, "row", 0 );
    if(_row <0) _row =0;
    int _col = KWDocument::getAttribute( framesetElem, "col", 0 );
    if(_col <0) _col =0;
    int _rows = KWDocument::getAttribute( framesetElem, "rows", 1 );
    if(_rows <0) _rows =0;
    int _cols = KWDocument::getAttribute( framesetElem, "cols", 1 );
    if(_cols <0) _cols =0;

    while(m_rowPositions.count() <= static_cast<unsigned int>(_row + _rows)) {
        QValueList<double>::iterator tmp = m_rowPositions.end();
        m_rowPositions.insert(tmp,0);
    }
    while(m_colPositions.count() <= static_cast<unsigned int>(_col + _cols)) {
        QValueList<double>::iterator tmp = m_colPositions.end();
        m_colPositions.insert(tmp,0);
    }

    Cell *cell = new Cell( this, _row, _col, QString::null /*unused*/ );
    QString autoName = cell->getName();
    kdDebug() << "KWTableFrameSet::loadCell autoName=" << autoName << endl;
    cell->load( framesetElem, loadFrames );
    cell->m_rows = _rows;
    cell->m_cols = _cols;
    if(cell->getFrame(0)) {
        cell->getFrame(0)->setMinFrameHeight(cell->getFrame(0)->height());
        QValueList<double>::iterator tmp = m_colPositions.at(_col);
        if(*tmp == 0) (*tmp) = cell->getFrame(0)->left();
        else (*tmp) = (cell->getFrame(0)->left() + *tmp) / 2;

        tmp = m_colPositions.at(_col+_cols);
        if(*tmp == 0) (*tmp) = cell->getFrame(0)->right();
        else (*tmp) = (cell->getFrame(0)->right() + *tmp) / 2;

        tmp = m_rowPositions.at(_row);
        if(*tmp == 0) (*tmp) = cell->getFrame(0)->top();
        else (*tmp) = (cell->getFrame(0)->top() + *tmp) / 2;

        tmp = m_rowPositions.at(_row+_rows);
        if(*tmp == 0) (*tmp) = cell->getFrame(0)->bottom();
        else (*tmp) = (cell->getFrame(0)->bottom() + *tmp) / 2;
    }
    if ( !useNames )
        cell->setName( autoName );
    return cell;
}

int KWTableFrameSet::paragraphs()
{
    int paragraphs = 0;
    for (unsigned int i =0; i < m_cells.count(); i++) {
        paragraphs += m_cells.at(i)->paragraphs();
    }
    return paragraphs;
}

bool KWTableFrameSet::statistics( QProgressDialog *progress, ulong & charsWithSpace, ulong & charsWithoutSpace, ulong & words,
    ulong & sentences, ulong & syllables )
{
    for (unsigned int i =0; i < m_cells.count(); i++) {
        if( ! m_cells.at(i)->statistics( progress, charsWithSpace, charsWithoutSpace, words, sentences, syllables ) )
        {
            return false;
        }
    }
    return true;
}

void KWTableFrameSet::finalize( ) {
    //kdDebug() << "KWTableFrameSet::finalize" << endl;
    recalcRows();
    recalcCols();
    KWFrameSet::finalize();
}

bool KWTableFrameSet::canRemovePage( int num ) {
    /*  This one is a lot simpler then the one it overrides, we simply don't have
        to check if the frame contains something, the simple existence of a frame
        is enough
    */
    QPtrListIterator<KWFrame> frameIt( frameIterator() );
    for ( ; frameIt.current(); ++frameIt ) {
        if ( frameIt.current()->pageNum() == num ) {
            return false;
        }
    }
    return true;
}

void KWTableFrameSet::addTextFramesets( QPtrList<KWTextFrameSet> & lst )
{
    for (unsigned int i =0; i < m_cells.count(); i++) {
        lst.append(m_cells.at(i));
    }
}

#ifndef NDEBUG
void KWTableFrameSet::printDebug( KWFrame * frame )
{
    KWTableFrameSet::Cell *cell = dynamic_cast<KWTableFrameSet::Cell *>( frame->getFrameSet() );
    Q_ASSERT( cell );
    if ( cell ) {
        kdDebug() << " |  |- row :" << cell->m_row << endl;
        kdDebug() << " |  |- col :" << cell->m_col << endl;
        kdDebug() << " |  |- rows:" << cell->m_rows << endl;
        kdDebug() << " |  +- cols:" << cell->m_cols << endl;
    }
}

void KWTableFrameSet::printDebug() {
    kdDebug() << " |  Table size (" << m_rows << "x" << m_cols << ")" << endl;
    KWFrameSet::printDebug();
}

#endif

/////

KWTableFrameSet::Cell::Cell( KWTableFrameSet *table, unsigned int row, unsigned int col, const QString &/*name*/ ) :
    KWTextFrameSet( table->m_doc, "." /*dummy and not empty, faster*/ )
{
    m_table = table;
    m_row = row;
    m_col = col;
    m_rows = 1;
    m_cols = 1;
    setGroupManager( m_table );
    m_table->addCell( this );
    m_name = i18n("Hello dear translator :), 1 is the table name, 2 and 3 are row and column", "%1 Cell %2,%3")
             .arg( table->getName() ).arg(m_row).arg(m_col);
}

KWTableFrameSet::Cell::Cell( KWTableFrameSet *table, const Cell &original ) :
    KWTextFrameSet( table->m_doc, "." )
{
    m_table = table;
    m_row = original.m_row;
    m_col = original.m_col;
    m_rows = original.m_rows;
    m_cols = original.m_cols;
    setName(original.m_name+'_'); // unicity problem !
    setGroupManager( m_table );
    m_table->addCell( this );
}

KWTableFrameSet::Cell::~Cell()
{
}

bool KWTableFrameSet::Cell::isAboveOrLeftOf( unsigned row, unsigned col )
{
    return ( m_row < row ) || ( ( m_row == row ) && ( m_col < col ) );
}

void KWTableFrameSet::Cell::addFrame(KWFrame *_frame, bool recalc) {
    if(getGroupManager())
        getGroupManager()->addFrame(_frame, recalc);
    KWTextFrameSet::addFrame(_frame, recalc);
}

double KWTableFrameSet::Cell::leftBorder() {
    double b = getFrame(0)->leftBorder().ptWidth;
    if(b==0) return 0;
    return (b / (m_col==0?1:2));
}

double KWTableFrameSet::Cell::rightBorder() { 
    double b=getFrame(0)->rightBorder().ptWidth;
    if(b==0) return 0;
    return (b / ((m_col+m_cols)==m_table->m_cols?1:2));
}

double KWTableFrameSet::Cell::topBorder() { 
    double b = getFrame(0)->topBorder().ptWidth;
    if(b==0) return 0;
    return (b / (m_row==0?1:2));
}

double KWTableFrameSet::Cell::bottomBorder() { 
    double b = getFrame(0)->bottomBorder().ptWidth;
    if(b==0) return 0;
    return ( b / ((m_row+m_rows)==m_table->m_rows?1:2));
}

void KWTableFrameSet::Cell::setLeftBorder(KoBorder newBorder) {
    KWFrame *f = getFrame(0);
    double diff = f->leftBorder().ptWidth - newBorder.ptWidth;
    f->setLeftBorder(newBorder);

    if((diff > 0.01 || diff < -0.01) && m_col!=0) {
        diff = diff / 2; // if not outer edge only use halve
        m_table->getCell(m_row, m_col-1)->setRightBorder(newBorder);
    }
    f->setLeft(f->left() - diff);
}

void KWTableFrameSet::Cell::setRightBorder(KoBorder newBorder) {
    KWFrame *f = getFrame(0);
    double diff = f->rightBorder().ptWidth - newBorder.ptWidth;
    f->setRightBorder(newBorder);

    if((diff > 0.01 || diff < -0.01) && m_col+m_cols!=m_table->m_cols) {
        diff = diff / 2; // if not outer edge only use halve
        m_table->getCell(m_row, m_col+1)->setLeftBorder(newBorder);
    }
    f->setRight(f->right() + diff);
}

void KWTableFrameSet::Cell::setTopBorder(KoBorder newBorder) {
    KWFrame *f = getFrame(0);
    double diff = f->topBorder().ptWidth - newBorder.ptWidth;
    f->setTopBorder(newBorder);

    if((diff > 0.01 || diff < -0.01) && m_row!=0) {
        diff = diff / 2; // if not outer edge only use halve
        m_table->getCell(m_row-1, m_col)->setBottomBorder(newBorder);
    }
    f->setTop(f->top() - diff);
}

void KWTableFrameSet::Cell::setBottomBorder(KoBorder newBorder) {
    KWFrame *f = getFrame(0);
    double diff = f->bottomBorder().ptWidth - newBorder.ptWidth;
    f->setBottomBorder(newBorder);

    if((diff > 0.01 || diff < -0.01) && m_row+m_rows!=m_table->m_rows) {
        diff = diff / 2; // if not outer edge only use halve
        m_table->getCell(m_row+1, m_col)->setTopBorder(newBorder);
    }
    f->setBottom(f->bottom() + diff);
}


KWTableFrameSetEdit::~KWTableFrameSetEdit()
{
    if ( m_currentCell )
        m_currentCell->terminate();
    delete m_currentCell;
}

void KWTableFrameSetEdit::mousePressEvent( QMouseEvent * e, const QPoint & nPoint, const KoPoint & dPoint )
{
    setCurrentCell( dPoint );
    if ( m_currentCell )
        m_currentCell->mousePressEvent( e, nPoint, dPoint );
}

void KWTableFrameSetEdit::setCurrentCell( const KoPoint & dPoint )
{
    KWFrameSet *fs = tableFrameSet()->getCellByPos( dPoint.x(), dPoint.y() );
    if ( fs && ( !m_currentCell || fs != m_currentCell->frameSet() ) )
        setCurrentCell( fs );
}

void KWTableFrameSetEdit::setCurrentCell( KWFrameSet * fs )
{
    if ( m_currentCell )
        m_currentCell->terminate();
    delete m_currentCell;
    m_currentCell =  fs->createFrameSetEdit( m_canvas );
    m_currentFrame = fs->getFrame( 0 );
    //refresh koruler
    m_canvas->gui()->getView()->slotUpdateRuler();
}

KWFrameSetEdit* KWTableFrameSetEdit::currentTextEdit()
{
    return m_currentCell;
}


void KWTableFrameSetEdit::keyPressEvent( QKeyEvent * e )
{
    if ( !m_currentCell )
        return;
    KWTableFrameSet::Cell *cell = static_cast<KWTableFrameSet::Cell *>(m_currentCell->frameSet());
    KWTextFrameSet *textframeSet=dynamic_cast<KWTextFrameSet *>(m_currentCell->frameSet());
    bool moveToOtherCell=true;
    if(textframeSet)
    {
        //don't move to other cell when we try to select
        //a text
        QTextDocument * textdoc = textframeSet->textDocument();
        if(textdoc->hasSelection( QTextDocument::Standard ))
            moveToOtherCell=false;
    }
    KWFrameSet *fs = 0L;

    if(moveToOtherCell)
    {
        switch( e->key() ) {
            case QKeyEvent::Key_Up:
            {
                if(!(static_cast<KWTextFrameSetEdit *>(m_currentCell))->cursor()->parag()->prev())
                {
                    KWTableFrameSet* tableFrame=tableFrameSet();
                    if ( cell->m_col > 0 )
                        fs = tableFrame->getCell(  cell->m_row, cell->m_col - 1 );
                    else if ( cell->m_row > 0 )
                        fs = tableFrame->getCell(  cell->m_row - 1,tableFrame->getCols() - 1 );
                    else
                        fs = tableFrame->getCell( tableFrame->getRows() - 1,tableFrame->getCols() - 1 );
                }
            }
            break;
            case QKeyEvent::Key_Down:
            {
                if(!(static_cast<KWTextFrameSetEdit *>(m_currentCell))->cursor()->parag()->next())
                {
                    KWTableFrameSet* tableFrame=tableFrameSet();
                    if ( cell->m_col+cell->m_cols < tableFrame->getCols()  )
                        fs = tableFrame->getCell( cell->m_row+cell->m_rows-1, cell->m_col + cell->m_cols );
                    else if ( cell->m_row+cell->m_rows < tableFrame->getRows() )
                        fs = tableFrame->getCell( cell->m_row + cell->m_rows, 0 );
                    else
                        fs = tableFrame->getCell( 0, 0 );
                }
            }
            break;
        }
    }
    if ( fs )
        setCurrentCell( fs );
    else
        m_currentCell->keyPressEvent( e );
}

void KWTableFrameSet::showPopup( KWFrame *frame, KWFrameSetEdit *edit, KWView *view, const QPoint &point )
{
    KWTextFrameSetEdit * textedit = dynamic_cast<KWTextFrameSetEdit *>(edit);
    Q_ASSERT( textedit ); // is it correct that this is always set ?
    if (textedit)
        textedit->showPopup( frame, view, point );
    else
    {
        QPopupMenu * popup = view->popupMenu("text_popup");
        Q_ASSERT(popup);
        if (popup)
            popup->popup( point );
    }
}

#include "kwtableframeset.moc"
