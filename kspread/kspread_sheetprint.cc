/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>,
   2003 Philipp M�ller <philipp.mueller@gmx.de>

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

#include "kspread_sheetprint.h"
#include "kspread_sheet.h"
#include "kspread_doc.h"
#include "kspread_undo.h"

#include <koDocumentInfo.h>

#include <kmessagebox.h>
#include <kprinter.h>
#include <kdebug.h>

#include <qregexp.h>

#include <pwd.h>
#include <unistd.h>

#include "kspread_sheetprint.moc"

#define NO_MODIFICATION_POSSIBLE \
do { \
  KMessageBox::error( 0, i18n ( "You cannot change a protected sheet" ) ); return; \
} while(0)

KSpreadSheetPrint::KSpreadSheetPrint( KSpreadSheet* sheet )
{
    m_pSheet = sheet;
    m_pDoc = m_pSheet->doc();

    m_bPrintGrid = false;
    m_bPrintCommentIndicator = false;
    m_bPrintFormulaIndicator = false;

    m_leftBorder = 20.0;
    m_rightBorder = 20.0;
    m_topBorder = 20.0;
    m_bottomBorder = 20.0;
    m_paperFormat = PG_DIN_A4;
    m_paperWidth = PG_A4_WIDTH;
    m_paperHeight = PG_A4_HEIGHT;
    m_orientation = PG_PORTRAIT;
    m_printRange = QRect( QPoint( 1, 1 ), QPoint( KS_colMax, KS_rowMax ) );
    m_lnewPageListX.append( 1 );
    m_lnewPageListY.append( 1 );
    m_maxCheckedNewPageX = 1;
    m_maxCheckedNewPageY = 1;
    m_dPrintRepeatColumnsWidth = 0.0;
    m_dPrintRepeatRowsHeight = 0.0;
    m_printRepeatColumns = qMakePair( 0, 0 );
    m_printRepeatRows = qMakePair( 0, 0 );
    m_dZoom = 1.0;
    m_iPageLimitX = 0;
    m_iPageLimitY = 0;

    calcPaperSize();
}

KSpreadSheetPrint::~KSpreadSheetPrint()
{
  // nothing todo yet
}
QRect KSpreadSheetPrint::cellsPrintRange()
{
    // Find maximum right/bottom cell with content
    QRect cell_range;
    cell_range.setCoords( 1, 1, 1, 1 );

    KSpreadCell* c = m_pSheet->firstCell();
    for( ;c; c = c->nextCell() )
    {
        if ( c->needsPrinting() )
        {
            if ( c->column() > cell_range.right() )
                cell_range.setRight( c->column() );
            if ( c->row() > cell_range.bottom() )
                cell_range.setBottom( c->row() );
        }
    }

    // Now look at the children
    QPtrListIterator<KoDocumentChild> cit( m_pDoc->children() );
    double dummy;
    int i;
    for( ; cit.current(); ++cit )
    {
        //QRect, because KoChild doesn't use KoRect yet
        QRect bound = cit.current()->boundingRect();

        i = m_pSheet->leftColumn( bound.right(), dummy );
        if ( i > cell_range.right() )
            cell_range.setRight( i );

        i = m_pSheet->topRow( bound.bottom(), dummy );
        if ( i > cell_range.bottom() )
            cell_range.setBottom( i );
    }
    cell_range = cell_range.intersect( m_printRange );

    return cell_range;
}

int KSpreadSheetPrint::pagesX( QRect& cellsPrintRange )
{
    int pages = 0;

    updateNewPageX( m_pSheet->rightColumn( m_pSheet->dblColumnPos( cellsPrintRange.right() ) + printableWidthPts() ) );

    for( int i = cellsPrintRange.left(); i <= cellsPrintRange.right(); i++  )
    {
        if( isOnNewPageX( i ) )
            pages++;
    }
    return pages;
}

int KSpreadSheetPrint::pagesY( QRect& cellsPrintRange )
{
    int pages = 0;

    updateNewPageY( m_pSheet->bottomRow( m_pSheet->dblRowPos( cellsPrintRange.bottom() ) + printableHeightPts() ) );

    for( int i = cellsPrintRange.top(); i <= cellsPrintRange.bottom(); i++  )
    {
        if( isOnNewPageY( i ) )
            pages++;
    }
    return pages;
}


bool KSpreadSheetPrint::pageNeedsPrinting( QRect& page_range )
{
    bool filled = FALSE;

    // Look at the cells
    for( int r = page_range.top(); !filled && ( r <= page_range.bottom() ); ++r )
        for( int c = page_range.left(); !filled && ( c <= page_range.right() ); ++c )
            if ( m_pSheet->cellAt( c, r )->needsPrinting() )
                filled = TRUE;

    if( !filled ) //Page empty, but maybe children on it?
    {
        QRect intView = QRect( QPoint( m_pDoc->zoomItX( m_pSheet->dblColumnPos( page_range.left() ) ),
                                       m_pDoc->zoomItY( m_pSheet->dblRowPos( page_range.top() ) ) ),
                               QPoint( m_pDoc->zoomItX( m_pSheet->dblColumnPos( page_range.right() ) +
                                                        m_pSheet->columnFormat( page_range.right() )->dblWidth() ),
                                       m_pDoc->zoomItY( m_pSheet->dblRowPos( page_range.bottom() ) +
                                                        m_pSheet->rowFormat( page_range.bottom() )->dblHeight() ) ) );

        QPtrListIterator<KoDocumentChild> it( m_pDoc->children() );
        for( ; !filled && it.current(); ++it )
        {
            QRect bound = it.current()->boundingRect();
            if ( bound.intersects( intView ) )
                filled = TRUE;
        }
    }

    return filled;
}

bool KSpreadSheetPrint::print( QPainter &painter, KPrinter *_printer )
{
    kdDebug(36001)<<"PRINTING ...."<<endl;

    // Override the current grid pen setting, when set to disable
    QPen gridPen;
    bool oldShowGrid = m_pSheet->getShowGrid();
    m_pSheet->setShowGrid( m_bPrintGrid );
    if ( !m_bPrintGrid )
    {
        gridPen = m_pDoc->defaultGridPen();
        QPen nopen;
        nopen.setStyle( NoPen );
        m_pDoc->setDefaultGridPen( nopen );
    }

    //Update m_dPrintRepeatColumnsWidth for repeated columns
    //just in case it isn't done yet
    if ( !m_pSheet->isShowPageBorders() && m_printRepeatColumns.first != 0 )
        updatePrintRepeatColumnsWidth();

    //Update m_dPrintRepeatRowsHeight for repeated rows
    //just in case it isn't done yet
    if ( !m_pSheet->isShowPageBorders() && m_printRepeatRows.first != 0 )
        updatePrintRepeatRowsHeight();

    //Calculate the range to be printed
    QRect cell_range = cellsPrintRange();

    //Ensure, that our newPage lists are generated for the whole sheet to print
    //For this we add to the lists the width/height of 1 page
    updateNewPageX( m_pSheet->rightColumn( m_pSheet->dblColumnPos( cell_range.right() ) + printableWidthPts() ) );
    updateNewPageY( m_pSheet->bottomRow( m_pSheet->dblRowPos( cell_range.bottom() ) + printableHeightPts() ) );

    // Find out how many pages need printing
    // and which cells to print on which page.
    QValueList<QRect> page_list;  //contains the cols and rows of a page
    QValueList<KoRect> page_frame_list;  //contains the coordinate range of a page
    QValueList<KoPoint> page_frame_list_offset;  //contains the offset of the not repeated area

    QValueList<KSpreadPrintNewPageEntry>::iterator itX;
    QValueList<KSpreadPrintNewPageEntry>::iterator itY;
    for( itX = m_lnewPageListX.begin(); itX != m_lnewPageListX.end(); ++itX )
    {
        for( itY = m_lnewPageListY.begin(); itY != m_lnewPageListY.end(); ++itY )
        {
            QRect page_range( QPoint( (*itX).startItem(), (*itY).startItem() ),
                              QPoint( (*itX).endItem(),   (*itY).endItem() ) );
            //Append page when there is something to print
            if ( pageNeedsPrinting( page_range ) )
            {
                KoRect view = KoRect( KoPoint( m_pSheet->dblColumnPos( page_range.left() ),
                                               m_pSheet->dblRowPos( page_range.top() ) ),
                                      KoPoint( m_pSheet->dblColumnPos( page_range.right() ) +
                                               m_pSheet->columnFormat( page_range.right() )->dblWidth(),
                                               m_pSheet->dblRowPos( page_range.bottom() ) +
                                               m_pSheet->rowFormat( page_range.bottom() )->dblHeight() ) );
                page_list.append( page_range );
                page_frame_list.append( view );
                page_frame_list_offset.append( KoPoint( (*itX).offset(), (*itY).offset() ) );
            }
        }
    }


    kdDebug(36001) << "PRINTING " << page_list.count() << " pages" << endl;
    m_uprintPages = page_list.count();

    if ( page_list.count() == 0 )
    {
        // nothing to print
        painter.setPen( QPen( Qt::black, 1 ) );
        painter.drawPoint( 1, 1 );
    }
    else
    {

        int pageNo = 1;

        //
        // Print all pages in the list
        //
        QValueList<QRect>::Iterator it = page_list.begin();
        QValueList<KoRect>::Iterator fit = page_frame_list.begin();
        QValueList<KoPoint>::Iterator fito = page_frame_list_offset.begin();

        for( ; it != page_list.end(); ++it, ++fit, ++fito, ++pageNo )
        {
            painter.setClipRect( 0, 0, m_pDoc->zoomItX( paperWidthPts() ),
                                    m_pDoc->zoomItY( paperHeightPts() ) );
            printHeaderFooter( painter, pageNo );

            painter.translate( m_pDoc->zoomItX( leftBorderPts() ),
                            m_pDoc->zoomItY( topBorderPts() ) );

            // Print the page
            printPage( painter, *it, *fit, *fito );

            painter.translate( - m_pDoc->zoomItX( leftBorderPts() ),
                            - m_pDoc->zoomItY( topBorderPts()  ) );

            if ( pageNo < (int)page_list.count() )
                _printer->newPage();
        }
    }

    if ( !m_bPrintGrid )
    {
        // Restore the grid pen
        m_pDoc->setDefaultGridPen( gridPen );
    }
    m_pSheet->setShowGrid( oldShowGrid );

    return ( page_list.count() > 0 );
}

void KSpreadSheetPrint::printPage( QPainter &_painter, const QRect& page_range,
                                   const KoRect& view, const KoPoint _childOffset )
{
      kdDebug(36001) << "Rect x=" << page_range.left() << " y=" << page_range.top() << ", r="
      << page_range.right() << " b="  << page_range.bottom() << "  offsetx: "<< _childOffset.x()
      << "  offsety: " << _childOffset.y() <<"  view-x: "<<view.x()<< endl;

    //Don't paint on the page borders
    QRegion clipRegion( m_pDoc->zoomItX( leftBorderPts() ),
                        m_pDoc->zoomItY( topBorderPts() ),
                        m_pDoc->zoomItX( view.width() + _childOffset.x() ),
                        m_pDoc->zoomItY( view.height() + _childOffset.y() ) );
    _painter.setClipRegion( clipRegion );

    //
    // Draw the cells.
    //
    //Check if we have to repeat some rows and columns (top left rect)
    if ( ( _childOffset.x() != 0.0 ) && ( _childOffset.y() != 0.0 ) )
    {
        QRect _printRect( m_printRepeatColumns.first, m_printRepeatRows.first,
                          m_printRepeatColumns.second, m_printRepeatRows.second );
        KoPoint _topLeft( 0.0, 0.0 );

        printRect( _painter, _topLeft, _printRect, view, clipRegion );
    }

    //Check if we have to repeat some rows (left rect)
    if ( _childOffset.y() != 0 )
    {
        QRect _printRect( page_range.left(), m_printRepeatRows.first,
                          page_range.right(), m_printRepeatRows.second );
        KoPoint _topLeft( _childOffset.x(), 0.0 );

        printRect( _painter, _topLeft, _printRect, view, clipRegion );
    }

    //Check if we have to repeat some columns (top right rect)
    if ( _childOffset.x() != 0 )
    {
        QRect _printRect( m_printRepeatColumns.first, page_range.top(),
                          m_printRepeatColumns.second, page_range.bottom() );
        KoPoint _topLeft( 0.0, _childOffset.y() );

        printRect( _painter, _topLeft, _printRect, view, clipRegion );
    }


    //Print the cells (right data rect)
    KoPoint _topLeft( _childOffset.x(), _childOffset.y() );

    printRect( _painter, _topLeft, page_range, view, clipRegion );
}


void KSpreadSheetPrint::printRect( QPainter& painter, const KoPoint& topLeft,
                                   const QRect& printRect, const KoRect& view,
                                   QRegion &clipRegion )
{
    //
    // Draw the cells.
    //
    KSpreadCell *cell;
    RowFormat *row_lay;
    ColumnFormat *col_lay;

    double xpos;
    double ypos =  topLeft.y();

    int regionBottom = printRect.bottom();
    int regionRight  = printRect.right();
    int regionLeft   = printRect.left();
    int regionTop    = printRect.top();

    //Calculate the output rect
    KoPoint bottomRight( topLeft );
    for ( int x = regionLeft; x <= regionRight; ++x )
        bottomRight.setX( bottomRight.x()
                          + m_pSheet->columnFormat( x )->dblWidth() );
    for ( int y = regionTop; y <= regionBottom; ++y )
        bottomRight.setY( bottomRight.y()
                          + m_pSheet->rowFormat( y )->dblHeight() );
    KoRect rect( topLeft, bottomRight );

    for ( int y = regionTop; y <= regionBottom; ++y )
    {
        row_lay = m_pSheet->rowFormat( y );
        xpos = topLeft.x();

        for ( int x = regionLeft; x <= regionRight; ++x )
        {
            col_lay = m_pSheet->columnFormat( x );

            cell = m_pSheet->cellAt( x, y );

            bool paintBordersBottom = false;
            bool paintBordersRight = false;
            bool paintBordersLeft = false;
            bool paintBordersTop = false;

            QPen rightPen  = cell->effRightBorderPen( x, y );
            QPen leftPen   = cell->effLeftBorderPen( x, y );
            QPen bottomPen = cell->effBottomBorderPen( x, y );
            QPen topPen    = cell->effTopBorderPen( x, y );

            // paint right border if rightmost cell or if the pen is more "worth" than the left border pen
            // of the cell on the left or if the cell on the right is not painted. In the latter case get
            // the pen that is of more "worth"
            if ( x >= KS_colMax )
              paintBordersRight = true;
            else
              if ( x == regionRight )
              {
                paintBordersRight = true;
                if ( cell->effRightBorderValue( x, y ) < m_pSheet->cellAt( x + 1, y )->effLeftBorderValue( x + 1, y ) )
                  rightPen = m_pSheet->cellAt( x + 1, y )->effLeftBorderPen( x + 1, y );
              }
              else
              {
                paintBordersRight = true;
                if ( cell->effRightBorderValue( x, y ) < m_pSheet->cellAt( x + 1, y )->effLeftBorderValue( x + 1, y ) )
                  rightPen = m_pSheet->cellAt( x + 1, y )->effLeftBorderPen( x + 1, y );
              }

            // similiar for other borders...
            // bottom border:
            if ( y >= KS_rowMax )
              paintBordersBottom = true;
            else
              if ( y == regionBottom )
              {
                paintBordersBottom = true;
                if ( cell->effBottomBorderValue( x, y ) < m_pSheet->cellAt( x, y + 1 )->effTopBorderValue( x, y + 1) )
                  bottomPen = m_pSheet->cellAt( x, y + 1 )->effTopBorderPen( x, y + 1 );
              }
              else
              {
                paintBordersBottom = true;
                if ( cell->effBottomBorderValue( x, y ) < m_pSheet->cellAt( x, y + 1 )->effTopBorderValue( x, y + 1) )
                  bottomPen = m_pSheet->cellAt( x, y + 1 )->effTopBorderPen( x, y + 1 );
              }

            // left border:
            if ( x == 1 )
              paintBordersLeft = true;
            else
              if ( x == regionLeft )
              {
                paintBordersLeft = true;
                if ( cell->effLeftBorderValue( x, y ) < m_pSheet->cellAt( x - 1, y )->effRightBorderValue( x - 1, y ) )
                  leftPen = m_pSheet->cellAt( x - 1, y )->effRightBorderPen( x - 1, y );
              }
              else
              {
                paintBordersLeft = true;
                if ( cell->effLeftBorderValue( x, y ) < m_pSheet->cellAt( x - 1, y )->effRightBorderValue( x - 1, y ) )
                  leftPen = m_pSheet->cellAt( x - 1, y )->effRightBorderPen( x - 1, y );
              }

            // top border:
            if ( y == 1 )
              paintBordersTop = true;
            else
              if ( y == regionTop )
              {
                paintBordersTop = true;
                if ( cell->effTopBorderValue( x, y ) < m_pSheet->cellAt( x, y - 1 )->effBottomBorderValue( x, y - 1 ) )
                  topPen = m_pSheet->cellAt( x, y - 1 )->effBottomBorderPen( x, y - 1 );
              }
              else
              {
                paintBordersTop = true;
                if ( cell->effTopBorderValue( x, y ) < m_pSheet->cellAt( x, y - 1 )->effBottomBorderValue( x, y - 1 ) )
                  topPen = m_pSheet->cellAt( x, y - 1 )->effBottomBorderPen( x, y - 1 );
              }

            cell->paintCell( rect, painter, NULL,
                             KoPoint( xpos, ypos ), QPoint( x, y ),
                             paintBordersRight, paintBordersBottom,
                             paintBordersLeft, paintBordersTop,
                             rightPen, bottomPen, leftPen, topPen );

            xpos += col_lay->dblWidth();
        }

        ypos += row_lay->dblHeight();
    }

    //
    // Draw the children
    //
    QRect zoomedView = m_pDoc->zoomRect( view );
    QPtrListIterator<KoDocumentChild> it( m_pDoc->children() );
    QRect bound;
    for( ; it.current(); ++it )
    {
//        QString tmp=QString("Testing child %1/%2 %3/%4 against view %5/%6 %7/%8")
//        .arg(it.current()->contentRect().left())
//        .arg(it.current()->contentRect().top())
//        .arg(it.current()->contentRect().right())
//        .arg(it.current()->contentRect().bottom())
//        .arg(view.left()).arg(view.top()).arg(zoomedView.right()).arg(zoomedView.bottom());
//        kdDebug(36001)<<tmp<<" offset "<<_childOffset.x()<<"/"<<_childOffset.y()<<endl;

        bound = it.current()->boundingRect();
        if ( ( ( KSpreadChild* )it.current() )->table() == m_pSheet &&
             bound.intersects( zoomedView ) )
        {
            painter.save();

            painter.translate( -zoomedView.left() + m_pDoc->zoomItX( topLeft.x() ),
                               -zoomedView.top()  + m_pDoc->zoomItY( topLeft.y() ) );
            bound.moveBy( -bound.x(), -bound.y() );

            it.current()->transform( painter );
            it.current()->document()->paintEverything( painter,
                                                       bound,
                                                       it.current()->isTransparent() );
            painter.restore();
        }
    }

    //Don't let obscuring cells and children overpaint this area
    clipRegion -= QRegion ( m_pDoc->zoomItX( leftBorderPts() + topLeft.x() ),
                            m_pDoc->zoomItY( topBorderPts() + topLeft.y() ),
                            m_pDoc->zoomItX( xpos ),
                            m_pDoc->zoomItY( ypos ) );
    painter.setClipRegion( clipRegion );
}


void KSpreadSheetPrint::printHeaderFooter( QPainter &painter, int pageNo )
{
    double w;
    double headFootDistance = MM_TO_POINT( 10.0 /*mm*/ ) / m_dZoom;
    QFont font( "Times" );
    font.setPointSizeFloat( 0.01 * m_pDoc->zoom() *
                            /* Font size of 10 */ 10.0 / m_dZoom );
    painter.setFont( font );
    QFontMetrics fm = painter.fontMetrics();

    // print head line left
    w = fm.width( headLeft( pageNo, m_pSheet->tableName() ) ) / m_dZoom;
    if ( w > 0 )
        painter.drawText( m_pDoc->zoomItX( leftBorderPts() ),
                          m_pDoc->zoomItY( headFootDistance ),
                          headLeft( pageNo, m_pSheet->tableName() ) );
    // print head line middle
    w = fm.width( headMid( pageNo, m_pSheet->tableName() ) ) / m_dZoom;
    if ( w > 0 )
        painter.drawText( m_pDoc->zoomItX( leftBorderPts() ) +
                          ( m_pDoc->zoomItX( printableWidthPts() ) -
                            w ) / 2.0,
                          m_pDoc->zoomItY( headFootDistance ),
                          headMid( pageNo, m_pSheet->tableName() ) );
    // print head line right
    w = fm.width( headRight( pageNo, m_pSheet->tableName() ) ) / m_dZoom;
    if ( w > 0 )
        painter.drawText( m_pDoc->zoomItX( leftBorderPts() +
                                           printableWidthPts() ) - w,
                          m_pDoc->zoomItY( headFootDistance ),
                          headRight( pageNo, m_pSheet->tableName() ) );

    // print foot line left
    w = fm.width( footLeft( pageNo, m_pSheet->tableName() ) ) / m_dZoom;
    if ( w > 0 )
        painter.drawText( m_pDoc->zoomItX( leftBorderPts() ),
                          m_pDoc->zoomItY( paperHeightPts() - headFootDistance ),
                          footLeft( pageNo, m_pSheet->tableName() ) );
    // print foot line middle
    w = fm.width( footMid( pageNo, m_pSheet->tableName() ) ) / m_dZoom;
    if ( w > 0 )
        painter.drawText( m_pDoc->zoomItX( leftBorderPts() ) +
                          ( m_pDoc->zoomItX( printableWidthPts() ) -
                            w ) / 2.0,
                          m_pDoc->zoomItY( paperHeightPts() - headFootDistance ),
                          footMid( pageNo, m_pSheet->tableName() ) );
    // print foot line right
    w = fm.width( footRight( pageNo, m_pSheet->tableName() ) ) / m_dZoom;
    if ( w > 0 )
        painter.drawText( m_pDoc->zoomItX( leftBorderPts() +
                                           printableWidthPts() ) -
                                           w,
                          m_pDoc->zoomItY( paperHeightPts() - headFootDistance ),
                          footRight( pageNo, m_pSheet->tableName() ) );
}


bool KSpreadSheetPrint::isOnNewPageX( int _column )
{
    if( _column > m_maxCheckedNewPageX )
        updateNewPageX( _column );

    //Are these the edges of the print range?
    if ( _column == m_printRange.left() || _column == m_printRange.right() + 1 )
    {
        return TRUE;
    }

    //beyond the print range it's always false
    if ( _column < m_printRange.left() || _column > m_printRange.right() )
    {
        return TRUE;
    }

    //Now check if we find the column already in the list
    if ( m_lnewPageListX.findIndex( _column ) != -1 )
    {
        if( _column > m_maxCheckedNewPageX )
            m_maxCheckedNewPageX = _column;
        return TRUE;
    }
    return FALSE;
}


void KSpreadSheetPrint::updateNewPageX( int _column )
{
    float offset = 0.0;

    //Are these the edges of the print range?
    if ( _column == m_printRange.left() || _column == m_printRange.right() + 1 )
    {
        if( _column > m_maxCheckedNewPageX )
            m_maxCheckedNewPageX = _column;
        return;
    }

    //We don't check beyond the print range
    if ( _column < m_printRange.left() || _column > m_printRange.right() )
    {
        if( _column > m_maxCheckedNewPageX )
            m_maxCheckedNewPageX = _column;
        if ( _column > m_printRange.right() )
        {
            if ( m_lnewPageListX.last().endItem()==0 )
                m_lnewPageListX.last().setEndItem( m_printRange.right() );
        }
        return;
    }

    //If we start, then add the left printrange
    if ( m_lnewPageListX.empty() )
        m_lnewPageListX.append( m_printRange.left() ); //Add the first entry

    //If _column is greater than the last entry, we need to calculate the result
    if ( _column > m_lnewPageListX.last().startItem() &&
         _column > m_maxCheckedNewPageX ) //this columns hasn't been calculated before
    {
        int startCol = m_lnewPageListX.last().startItem();
        int col = startCol;
        double x = m_pSheet->columnFormat( col )->dblWidth();

        //Add repeated column width, when necessary
        if ( col > m_printRepeatColumns.first )
        {
            x += m_dPrintRepeatColumnsWidth;
            offset = m_dPrintRepeatColumnsWidth;
        }

        while ( ( col <= _column ) && ( col < m_printRange.right() ) )
        {
            if ( x > printableWidthPts() ) //end of page?
            {
                //We found a new page, so add it to the list
                m_lnewPageListX.append( col );

                //Now store into the previous entry the enditem and the width
                QValueList<KSpreadPrintNewPageEntry>::iterator it;
                it = findNewPageColumn( startCol );
                (*it).setEndItem( col - 1 );
                (*it).setSize( x - m_pSheet->columnFormat( col )->dblWidth() );
                (*it).setOffset( offset );

                //start a new page
                startCol = col;
                if ( col == _column )
                {
                    if( _column > m_maxCheckedNewPageX )
                        m_maxCheckedNewPageX = _column;
                    return;
                }
                else
                {
                    x = m_pSheet->columnFormat( col )->dblWidth();
                    if ( col >= m_printRepeatColumns.first )
                    {
                        x += m_dPrintRepeatColumnsWidth;
                        offset = m_dPrintRepeatColumnsWidth;
                    }
                }
            }

            col++;
            x += m_pSheet->columnFormat( col )->dblWidth();
        }
    }

    if( _column > m_maxCheckedNewPageX )
        m_maxCheckedNewPageX = _column;
}


bool KSpreadSheetPrint::isOnNewPageY( int _row )
{
    if( _row > m_maxCheckedNewPageY )
        updateNewPageY( _row );

    //Are these the edges of the print range?
    if ( _row == m_printRange.top() || _row == m_printRange.bottom() + 1 )
    {
        return FALSE;
    }

     //beyond the print range it's always false
    if ( _row < m_printRange.top() || _row > m_printRange.bottom() )
    {
        return FALSE;
    }

    //Now check if we find the row already in the list
    if ( m_lnewPageListY.findIndex( _row ) != -1 )
    {
        if( _row > m_maxCheckedNewPageY )
            m_maxCheckedNewPageY = _row;
        return TRUE;
    }

    return FALSE;
}


void KSpreadSheetPrint::updateNewPageY( int _row )
{
    float offset = 0.0;

    //Are these the edges of the print range?
    if ( _row == m_printRange.top() || _row == m_printRange.bottom() + 1 )
    {
        if( _row > m_maxCheckedNewPageY )
            m_maxCheckedNewPageY = _row;
        return;
    }

     //beyond the print range it's always false
    if ( _row < m_printRange.top() || _row > m_printRange.bottom() )
    {
        if( _row > m_maxCheckedNewPageY )
            m_maxCheckedNewPageY = _row;
        if ( _row > m_printRange.bottom() )
        {
            if ( m_lnewPageListY.last().endItem()==0 )
                m_lnewPageListY.last().setEndItem( m_printRange.bottom() );
        }
        return;
    }

    //If we start, then add the top printrange
    if ( m_lnewPageListY.empty() )
        m_lnewPageListY.append( m_printRange.top() ); //Add the first entry

    //If _column is greater than the last entry, we need to calculate the result
    if ( _row > m_lnewPageListY.last().startItem() &&
         _row > m_maxCheckedNewPageY ) //this columns hasn't been calculated before
    {
        int startRow = m_lnewPageListY.last().startItem();
        int row = startRow;
        double y = m_pSheet->rowFormat( row )->dblHeight();

        //Add repeated row height, when necessary
        if ( row > m_printRepeatRows.first )
        {
            y += m_dPrintRepeatRowsHeight;
            offset = m_dPrintRepeatRowsHeight;
        }

        while ( ( row <= _row ) && ( row < m_printRange.bottom() ) )
        {
            if ( y > printableHeightPts() )
            {
                //We found a new page, so add it to the list
                m_lnewPageListY.append( row );

                //Now store into the previous entry the enditem and the width
                QValueList<KSpreadPrintNewPageEntry>::iterator it;
                it = findNewPageRow( startRow );
                (*it).setEndItem( row - 1 );
                (*it).setSize( y - m_pSheet->rowFormat( row )->dblHeight() );
                (*it).setOffset( offset );

                //start a new page
                startRow = row;
                if ( row == _row )
                {
                    if( _row > m_maxCheckedNewPageY )
                        m_maxCheckedNewPageY = _row;
                    return;
                }
                else
                {
                    y = m_pSheet->rowFormat( row )->dblHeight();
                    if ( row >= m_printRepeatRows.first )
                    {
                        y += m_dPrintRepeatRowsHeight;
                        offset = m_dPrintRepeatRowsHeight;
                    }
                }
            }

            row++;
            y += m_pSheet->rowFormat( row )->dblHeight();
        }
    }

    if( _row > m_maxCheckedNewPageY )
        m_maxCheckedNewPageY = _row;
}


void KSpreadSheetPrint::updateNewPageListX( int _col )
{
    //If the new range is after the first entry, we need to delete the whole list
    if ( m_lnewPageListX.first().startItem() != m_printRange.left() ||
         _col == 0 )
    {
        m_lnewPageListX.clear();
        m_maxCheckedNewPageX = m_printRange.left();
        m_lnewPageListX.append( m_printRange.left() );
        return;
    }

    if ( _col < m_lnewPageListX.last().startItem() )
    {
        //Find the page entry for this column
        QValueList<KSpreadPrintNewPageEntry>::iterator it;
        it = m_lnewPageListX.find( _col );
        while ( ( it == m_lnewPageListX.end() ) && _col > 0 )
        {
            _col--;
            it = m_lnewPageListX.find( _col );
        }

        //Remove later pages
        while ( it != m_lnewPageListX.end() )
            it = m_lnewPageListX.remove( it );

        //Add default page when list is now empty
        if ( m_lnewPageListX.empty() )
            m_lnewPageListX.append( m_printRange.left() );
    }

    m_maxCheckedNewPageX = _col;
}

void KSpreadSheetPrint::updateNewPageListY( int _row )
{
    //If the new range is after the first entry, we need to delete the whole list
    if ( m_lnewPageListY.first().startItem() != m_printRange.top() ||
         _row == 0 )
    {
        m_lnewPageListY.clear();
        m_maxCheckedNewPageY = m_printRange.top();
        m_lnewPageListY.append( m_printRange.top() );
        return;
    }

    if ( _row < m_lnewPageListY.last().startItem() )
    {
        //Find the page entry for this row
        QValueList<KSpreadPrintNewPageEntry>::iterator it;
        it = m_lnewPageListY.find( _row );
        while ( ( it == m_lnewPageListY.end() ) && _row > 0 )
        {
            _row--;
            it = m_lnewPageListY.find( _row );
        }

        //Remove later pages
        while ( it != m_lnewPageListY.end() )
            it = m_lnewPageListY.remove( it );

        //Add default page when list is now empty
        if ( m_lnewPageListY.empty() )
            m_lnewPageListY.append( m_printRange.top() );
    }

    m_maxCheckedNewPageY = _row;
}

void KSpreadSheetPrint::definePrintRange( KSpreadSelection* selectionInfo )
{
    if ( !selectionInfo->singleCellSelection() )
    {
        if ( !m_pDoc->undoBuffer()->isLocked() )
        {
             KSpreadUndoAction* undo = new KSpreadUndoDefinePrintRange( m_pSheet->doc(), m_pSheet );
             m_pDoc->undoBuffer()->appendUndo( undo );
        }

        setPrintRange( selectionInfo->selection() );
    }
}

void KSpreadSheetPrint::resetPrintRange ()
{
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
         KSpreadUndoAction* undo = new KSpreadUndoDefinePrintRange( m_pSheet->doc(), m_pSheet );
         m_pDoc->undoBuffer()->appendUndo( undo );
    }

    setPrintRange( QRect( QPoint( 1, 1 ), QPoint( KS_colMax, KS_rowMax ) ) );
}

void KSpreadSheetPrint::replaceHeadFootLineMacro ( QString &_text, const QString &_search, const QString &_replace )
{
    if ( _search != _replace )
        _text.replace ( QString( "<" + _search + ">" ), "<" + _replace + ">" );
}

QString KSpreadSheetPrint::localizeHeadFootLine ( const QString &_text )
{
    QString tmp = _text;

    /*
      i18n:
      Please use the same words (even upper/lower case) as in
      KoPageLayoutDia.cc function setupTab2(), without the brakets "<" and ">"
    */
    replaceHeadFootLineMacro ( tmp, "page",   i18n("page") );
    replaceHeadFootLineMacro ( tmp, "pages",  i18n("pages") );
    replaceHeadFootLineMacro ( tmp, "file",   i18n("file") );
    replaceHeadFootLineMacro ( tmp, "name",   i18n("name") );
    replaceHeadFootLineMacro ( tmp, "time",   i18n("time") );
    replaceHeadFootLineMacro ( tmp, "date",   i18n("date") );
    replaceHeadFootLineMacro ( tmp, "author", i18n("author") );
    replaceHeadFootLineMacro ( tmp, "email",  i18n("email") );
    replaceHeadFootLineMacro ( tmp, "org",    i18n("org") );
    replaceHeadFootLineMacro ( tmp, "sheet",  i18n("sheet") );

    return tmp;
}


QString KSpreadSheetPrint::delocalizeHeadFootLine ( const QString &_text )
{
    QString tmp = _text;

    /*
      i18n:
      Please use the same words (even upper/lower case) as in
      KoPageLayoutDia.cc function setupTab2(), without the brakets "<" and ">"
    */
    replaceHeadFootLineMacro ( tmp, i18n("page"),   "page" );
    replaceHeadFootLineMacro ( tmp, i18n("pages"),  "pages" );
    replaceHeadFootLineMacro ( tmp, i18n("file"),   "file" );
    replaceHeadFootLineMacro ( tmp, i18n("name"),   "name" );
    replaceHeadFootLineMacro ( tmp, i18n("time"),   "time" );
    replaceHeadFootLineMacro ( tmp, i18n("date"),   "date" );
    replaceHeadFootLineMacro ( tmp, i18n("author"), "author" );
    replaceHeadFootLineMacro ( tmp, i18n("email"),  "email" );
    replaceHeadFootLineMacro ( tmp, i18n("org"),    "org" );
    replaceHeadFootLineMacro ( tmp, i18n("sheet"),  "sheet" );

    return tmp;
}


KoHeadFoot KSpreadSheetPrint::headFootLine() const
{
    KoHeadFoot hf;
    hf.headLeft  = m_headLeft;
    hf.headRight = m_headRight;
    hf.headMid   = m_headMid;
    hf.footLeft  = m_footLeft;
    hf.footRight = m_footRight;
    hf.footMid   = m_footMid;

    return hf;
}


void KSpreadSheetPrint::setHeadFootLine( const QString &_headl, const QString &_headm, const QString &_headr,
                                         const QString &_footl, const QString &_footm, const QString &_footr )
{
    if ( m_pSheet->isProtected() )
        NO_MODIFICATION_POSSIBLE;

    m_headLeft  = _headl;
    m_headRight = _headr;
    m_headMid   = _headm;
    m_footLeft  = _footl;
    m_footRight = _footr;
    m_footMid   = _footm;

    m_pDoc->setModified( TRUE );
}

void KSpreadSheetPrint::setPaperOrientation( KoOrientation _orient )
{
    if ( m_pSheet->isProtected() )
        NO_MODIFICATION_POSSIBLE;

    m_orientation = _orient;
    calcPaperSize();
    updatePrintRepeatColumnsWidth();
    updatePrintRepeatRowsHeight();
    updateNewPageListX( m_printRange.left() ); //Reset the list
    updateNewPageListY( m_printRange.top() ); //Reset the list

    if( m_pSheet->isShowPageBorders() )
        emit sig_updateView( m_pSheet );
}


KoPageLayout KSpreadSheetPrint::paperLayout() const
{
    KoPageLayout pl;
    pl.format = m_paperFormat;
    pl.orientation = m_orientation;
    pl.ptWidth  =  m_paperWidth;
    pl.ptHeight =  m_paperHeight;
    pl.ptLeft   =  m_leftBorder;
    pl.ptRight  =  m_rightBorder;
    pl.ptTop    =  m_topBorder;
    pl.ptBottom =  m_bottomBorder;
    return pl;
}


void KSpreadSheetPrint::setPaperLayout( float _leftBorder, float _topBorder,
                                        float _rightBorder, float _bottomBorder,
                                        KoFormat _paper,
                                        KoOrientation _orientation )
{
    if ( m_pSheet->isProtected() )
        NO_MODIFICATION_POSSIBLE;

    m_leftBorder   = _leftBorder;
    m_rightBorder  = _rightBorder;
    m_topBorder    = _topBorder;
    m_bottomBorder = _bottomBorder;
    m_paperFormat  = _paper;

    setPaperOrientation( _orientation ); //calcPaperSize() is done here already

//    QPtrListIterator<KoView> it( views() );
//    for( ;it.current(); ++it )
//    {
//        KSpreadView *v = static_cast<KSpreadView *>( it.current() );
          // We need to trigger the appropriate repaintings in the cells near the
          // border of the page. The easiest way for this is to turn the borders
          // off and on (or on and off if they were off).
//        bool bBorderWasShown = v->activeTable()->isShowPageBorders();
//        v->activeTable()->setShowPageBorders( !bBorderWasShown );
//        v->activeTable()->setShowPageBorders( bBorderWasShown );
//    }

    m_pDoc->setModified( TRUE );
}

void KSpreadSheetPrint::setPaperLayout( float _leftBorder, float _topBorder,
                                        float _rightBorder, float _bottomBorder,
                                        const QString& _paper,
                                        const QString& _orientation )
{
    if ( m_pSheet->isProtected() )
        NO_MODIFICATION_POSSIBLE;

    KoFormat f = paperFormat();
    KoOrientation o = orientation();

    QString paper( _paper );
    if ( paper[0].isDigit() ) // Custom format
    {
        const int i = paper.find( 'x' );
        if ( i < 0 )
        {
            // We have nothing useful, so assume ISO A4
            f = PG_DIN_A4;
        }
        else
        {
            f = PG_CUSTOM;
            m_paperWidth  = paper.left(i).toFloat();
            m_paperHeight = paper.mid(i+1).toFloat();
            if ( m_paperWidth < 10.0 )
                m_paperWidth = PG_A4_WIDTH;
            if ( m_paperHeight < 10.0 )
                m_paperWidth = PG_A4_HEIGHT;
        }
    }
    else
    {
        f = KoPageFormat::formatFromString( paper );
        if ( f == PG_CUSTOM )
            // We have no idea about height or width, therefore assume ISO A4
            f = PG_DIN_A4;
    }

    if ( _orientation == "Portrait" )
        o = PG_PORTRAIT;
    else if ( _orientation == "Landscape" )
        o = PG_LANDSCAPE;

    setPaperLayout( _leftBorder, _topBorder, _rightBorder, _bottomBorder, f, o );
}

void KSpreadSheetPrint::calcPaperSize()
{
    if ( m_paperFormat != PG_CUSTOM )
    {
        m_paperWidth = KoPageFormat::width( m_paperFormat, m_orientation );
        m_paperHeight = KoPageFormat::height( m_paperFormat, m_orientation );
    }
}

QValueList<KSpreadPrintNewPageEntry>::iterator KSpreadSheetPrint::findNewPageColumn( int col )
{
    QValueList<KSpreadPrintNewPageEntry>::iterator it;
    for( it = m_lnewPageListX.begin(); it != m_lnewPageListX.end(); ++it )
    {
        if( (*it).startItem() == col )
            return it;
    }
    return it;
//                QValueList<KSpreadPrintNewPageEntry>::iterator it;
//                it = m_lnewPageListX.find( startCol );
}

QValueList<KSpreadPrintNewPageEntry>::iterator KSpreadSheetPrint::findNewPageRow( int row )
{
    QValueList<KSpreadPrintNewPageEntry>::iterator it;
    for( it = m_lnewPageListY.begin(); it != m_lnewPageListY.end(); ++it )
    {
        if( (*it).startItem() == row )
            return it;
    }
    return it;
}


QString KSpreadSheetPrint::paperFormatString()const
{
    if ( m_paperFormat == PG_CUSTOM )
    {
        QString tmp;
        tmp.sprintf( "%fx%f", m_paperWidth, m_paperHeight );
        return tmp;
    }

    return KoPageFormat::formatString( m_paperFormat );
}

const char* KSpreadSheetPrint::orientationString() const
{
    switch( m_orientation )
    {
    case KPrinter::Portrait:
        return "Portrait";
    case KPrinter::Landscape:
        return "Landscape";
    }

    kdWarning(36001)<<"KSpreadSheetPrint: Unknown orientation, using now portrait"<<endl;
    return 0;
}

QString KSpreadSheetPrint::completeHeading( const QString &_data, int _page, const QString &_table ) const
{
    QString page( QString::number( _page) );
    QString pages( QString::number( m_uprintPages ) );

    QString pathFileName(m_pDoc->url().path());
    if ( pathFileName.isNull() )
        pathFileName="";

    QString fileName(m_pDoc->url().fileName());
    if( fileName.isNull())
        fileName="";

    QString t(QTime::currentTime().toString());
    QString d(QDate::currentDate().toString());
    QString ta;
    if ( !_table.isEmpty() )
        ta = _table;

    KoDocumentInfo * info = m_pDoc->documentInfo();
    KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
    QString full_name;
    QString email_addr;
    QString organization;
    QString tmp;
    if ( !authorPage )
        kdWarning() << "Author information not found in document Info !" << endl;
    else
    {
        full_name = authorPage->fullName();
        email_addr = authorPage->email();
        organization = authorPage->company();
    }

    char hostname[80];
    struct passwd *p;

    p = getpwuid(getuid());
    gethostname(hostname, sizeof(hostname));

    if(full_name.isEmpty())
 	full_name=p->pw_gecos;

    if( email_addr.isEmpty())
	email_addr = QString("%1@%2").arg(p->pw_name).arg(hostname);

    tmp = _data;
    int pos = 0;
    while ( ( pos = tmp.find( "<page>", pos ) ) != -1 )
        tmp.replace( pos, 6, page );
    pos = 0;
    while ( ( pos = tmp.find( "<pages>", pos ) ) != -1 )
        tmp.replace( pos, 7, pages );
    pos = 0;
    while ( ( pos = tmp.find( "<file>", pos ) ) != -1 )
        tmp.replace( pos, 6, pathFileName );
    pos = 0;
    while ( ( pos = tmp.find( "<name>", pos ) ) != -1 )
        tmp.replace( pos, 6, fileName );
    pos = 0;
    while ( ( pos = tmp.find( "<time>", pos ) ) != -1 )
        tmp.replace( pos, 6, t );
    pos = 0;
    while ( ( pos = tmp.find( "<date>", pos ) ) != -1 )
        tmp.replace( pos, 6, d );
    pos = 0;
    while ( ( pos = tmp.find( "<author>", pos ) ) != -1 )
        tmp.replace( pos, 8, full_name );
    pos = 0;
    while ( ( pos = tmp.find( "<email>", pos ) ) != -1 )
        tmp.replace( pos, 7, email_addr );
    pos = 0;
    while ( ( pos = tmp.find( "<org>", pos ) ) != -1 )
        tmp.replace( pos, 5, organization );
    pos = 0;
    while ( ( pos = tmp.find( "<sheet>", pos ) ) != -1 )
        tmp.replace( pos, 7, ta );

    return tmp;
}

void KSpreadSheetPrint::setPrintRange( QRect _printRange )
{
    if ( m_pSheet->isProtected() )
        NO_MODIFICATION_POSSIBLE;

    if ( m_printRange == _printRange )
        return;

    int oldLeft = m_printRange.left();
    int oldTop = m_printRange.top();
    m_printRange = _printRange;

    //Refresh calculation of stored page breaks, the lower one of old and new
    if ( oldLeft != _printRange.left() )
        updateNewPageListX( QMIN( oldLeft, _printRange.left() ) );
    if ( oldTop != _printRange.top() )
        updateNewPageListY( QMIN( oldTop, _printRange.top() ) );

    m_pDoc->setModified( true );

    emit sig_updateView( m_pSheet );
}

void KSpreadSheetPrint::setPageLimitX( int pages )
{
    if( m_iPageLimitX == pages )
        return;

    m_iPageLimitX = pages;

    if( pages == 0 )
        return;

    calculateZoomForPageLimitX();
}

void KSpreadSheetPrint::setPageLimitY( int pages )
{
    if( m_iPageLimitY == pages )
        return;

    m_iPageLimitY = pages;

    if( pages == 0 )
        return;

    calculateZoomForPageLimitY();
}

void KSpreadSheetPrint::calculateZoomForPageLimitX()
{
    if( m_iPageLimitX == 0 )
        return;

    double origZoom = m_dZoom;

    if( m_dZoom < 1.0 )
        m_dZoom = 1.0;

    QRect printRange = cellsPrintRange();
    updateNewPageX( m_pSheet->rightColumn( m_pSheet->dblColumnPos( printRange.right() ) + printableWidthPts() ) );
    int currentPages = pagesX( printRange );
    while( ( currentPages > m_iPageLimitX ) && ( m_dZoom > 0.01 ) )
    {
        m_dZoom -= 0.01;
        updatePrintRepeatColumnsWidth();
        updateNewPageListX( 0 );
        updateNewPageX( m_pSheet->rightColumn( m_pSheet->dblColumnPos( printRange.right() ) + printableWidthPts() ) );
        currentPages = pagesX( printRange );
    }

    if ( m_dZoom < origZoom )
    {
        double newZoom = m_dZoom;
        m_dZoom += 1.0; //set it to something different
        setZoom( newZoom, false );
    }
    else
        m_dZoom = origZoom;
}

void KSpreadSheetPrint::calculateZoomForPageLimitY()
{
    if( m_iPageLimitY == 0 )
        return;

    double origZoom = m_dZoom;

    if( m_dZoom < 1.0 )
        m_dZoom = 1.0;

    QRect printRange = cellsPrintRange();
    updateNewPageY( m_pSheet->bottomRow( m_pSheet->dblRowPos( printRange.bottom() ) + printableHeightPts() ) );
    int currentPages = pagesY( printRange );
    while( ( currentPages > m_iPageLimitY ) && ( m_dZoom > 0.01 ) )
    {
        m_dZoom -= 0.01;
        updatePrintRepeatRowsHeight();
        updateNewPageListY( 0 );
        currentPages = pagesY( printRange );
    }

    if ( m_dZoom < origZoom )
    {
        double newZoom = m_dZoom;
        m_dZoom += 1.0; //set it to something different
        setZoom( newZoom, false );
    }
    else
        m_dZoom = origZoom;
}

void KSpreadSheetPrint::setPrintGrid( bool _printGrid )
{
   if ( m_bPrintGrid == _printGrid )
        return;

    m_bPrintGrid = _printGrid;
    m_pDoc->setModified( true );
}

void KSpreadSheetPrint::setPrintCommentIndicator( bool _printCommentIndicator )
{
    if ( m_bPrintCommentIndicator == _printCommentIndicator )
        return;

    m_bPrintCommentIndicator = _printCommentIndicator;
    m_pDoc->setModified( true );
}

void KSpreadSheetPrint::setPrintFormulaIndicator( bool _printFormulaIndicator )
{
    if( m_bPrintFormulaIndicator == _printFormulaIndicator )
        return;

    m_bPrintFormulaIndicator = _printFormulaIndicator;
    m_pDoc->setModified( true );
}

void KSpreadSheetPrint::updatePrintRepeatColumnsWidth()
{
    m_dPrintRepeatColumnsWidth = 0.0;
    if( m_printRepeatColumns.first != 0 )
    {
        for( int i = m_printRepeatColumns.first; i <= m_printRepeatColumns.second; i++ )
        {
            m_dPrintRepeatColumnsWidth += m_pSheet->columnFormat( i )->dblWidth();
        }
    }
}


void KSpreadSheetPrint::updatePrintRepeatRowsHeight()
{
    m_dPrintRepeatRowsHeight = 0.0;
    if ( m_printRepeatRows.first != 0 )
    {
        for ( int i = m_printRepeatRows.first; i <= m_printRepeatRows.second; i++)
        {
            m_dPrintRepeatRowsHeight += m_pSheet->rowFormat( i )->dblHeight();
        }
    }
}


void KSpreadSheetPrint::setPrintRepeatColumns( QPair<int, int> _printRepeatColumns )
{
    //Bring arguments in order
    if ( _printRepeatColumns.first > _printRepeatColumns.second )
    {
        int tmp = _printRepeatColumns.first;
        _printRepeatColumns.first = _printRepeatColumns.second;
        _printRepeatColumns.second = tmp;
    }

    //If old are equal to the new setting, nothing is to be done at all
    if ( m_printRepeatColumns == _printRepeatColumns )
        return;

    int oldFirst = m_printRepeatColumns.first;
    m_printRepeatColumns = _printRepeatColumns;

    //Recalcualte the space needed for the repeated columns
    updatePrintRepeatColumnsWidth();

    //Refresh calculation of stored page breaks, the lower one of old and new
    updateNewPageListX( QMIN( oldFirst, _printRepeatColumns.first ) );

    //Refresh view, if page borders are shown
    if ( m_pSheet->isShowPageBorders() )
        emit sig_updateView( m_pSheet );

    m_pDoc->setModified( true );
}

void KSpreadSheetPrint::setPrintRepeatRows( QPair<int, int> _printRepeatRows )
{
    //Bring arguments in order
    if ( _printRepeatRows.first > _printRepeatRows.second )
    {
        int tmp = _printRepeatRows.first;
        _printRepeatRows.first = _printRepeatRows.second;
        _printRepeatRows.second = tmp;
    }

    //If old are equal to the new setting, nothing is to be done at all
    if ( m_printRepeatRows == _printRepeatRows )
        return;

    int oldFirst = m_printRepeatRows.first;
    m_printRepeatRows = _printRepeatRows;

    //Recalcualte the space needed for the repeated rows
    updatePrintRepeatRowsHeight();

    //Refresh calculation of stored page breaks, the lower one of old and new
    updateNewPageListY( QMIN( oldFirst, _printRepeatRows.first ) );

    //Refresh view, if page borders are shown
    if ( m_pSheet->isShowPageBorders() )
        emit sig_updateView( m_pSheet );

    m_pDoc->setModified( true );
}

void KSpreadSheetPrint::insertColumn( int col, int nbCol )
{
    //update print range, when it has been defined
    if ( m_printRange != QRect( QPoint(1, 1), QPoint(KS_colMax, KS_rowMax) ) )
    {
        int left = m_printRange.left();
        int right = m_printRange.right();

        for( int i = 0; i <= nbCol; i++ )
        {
            if ( left >= col ) left++;
            if ( right >= col ) right++;
        }
        //Validity checks
        if ( left > KS_colMax ) left = KS_colMax;
        if ( right > KS_colMax ) right = KS_colMax;
        setPrintRange( QRect( QPoint( left, m_printRange.top() ),
                              QPoint( right, m_printRange.bottom() ) ) );
    }
}

void KSpreadSheetPrint::insertRow( int row, int nbRow )
{
    //update print range, when it has been defined
    if ( m_printRange != QRect( QPoint(1, 1), QPoint(KS_colMax, KS_rowMax) ) )
    {
        int top = m_printRange.top();
        int bottom = m_printRange.bottom();

        for( int i = 0; i <= nbRow; i++ )
        {
            if ( top >= row ) top++;
            if ( bottom >= row ) bottom++;
        }
        //Validity checks
        if ( top > KS_rowMax ) top = KS_rowMax;
        if ( bottom > KS_rowMax ) bottom = KS_rowMax;
        setPrintRange( QRect( QPoint( m_printRange.left(), top ),
                              QPoint( m_printRange.right(), bottom ) ) );
    }
}

void KSpreadSheetPrint::removeColumn( int col, int nbCol )
{
    //update print range, when it has been defined
    if ( m_printRange != QRect( QPoint(1, 1), QPoint(KS_colMax, KS_rowMax) ) )
    {
        int left = m_printRange.left();
        int right = m_printRange.right();

        for( int i = 0; i <= nbCol; i++ )
        {
            if ( left > col ) left--;
            if ( right >= col ) right--;
        }
        //Validity checks
        if ( left < 1 ) left = 1;
        if ( right < 1 ) right = 1;
        setPrintRange( QRect( QPoint( left, m_printRange.top() ),
                              QPoint( right, m_printRange.bottom() ) ) );
    }

    //update repeat columns, when it has been defined
    if ( m_printRepeatColumns.first != 0 )
    {
        int left = m_printRepeatColumns.first;
        int right = m_printRepeatColumns.second;

        for( int i = 0; i <= nbCol; i++ )
        {
            if ( left > col ) left--;
            if ( right >= col ) right--;
        }
        //Validity checks
        if ( left < 1 ) left = 1;
        if ( right < 1 ) right = 1;
        setPrintRepeatColumns ( qMakePair( left, right ) );
    }
}

void KSpreadSheetPrint::removeRow( int row, int nbRow )
{
    //update print range, when it has been defined
    if ( m_printRange != QRect( QPoint(1, 1), QPoint(KS_colMax, KS_rowMax) ) )
    {
        int top = m_printRange.top();
        int bottom = m_printRange.bottom();

        for( int i = 0; i <= nbRow; i++ )
        {
            if ( top > row ) top--;
            if ( bottom >= row ) bottom--;
        }
        //Validity checks
        if ( top < 1 ) top = 1;
        if ( bottom < 1 ) bottom = 1;
        setPrintRange( QRect( QPoint( m_printRange.left(), top ),
                              QPoint( m_printRange.right(), bottom ) ) );
    }

    //update repeat rows, when it has been defined
    if ( m_printRepeatRows.first != 0 )
    {
        int top = m_printRepeatRows.first;
        int bottom = m_printRepeatRows.second;

        for( int i = 0; i <= nbRow; i++ )
        {
            if ( top > row ) top--;
            if ( bottom >= row ) bottom--;
        }
        //Validity checks
        if ( top < 1 ) top = 1;
        if ( bottom < 1 ) bottom = 1;
        setPrintRepeatRows( qMakePair( top, bottom ) );
    }
}

void KSpreadSheetPrint::setZoom( double _zoom, bool checkPageLimit )
{
    if( m_dZoom == _zoom )
    {
        return;
    }

    m_dZoom = _zoom;
    updatePrintRepeatColumnsWidth();
    updatePrintRepeatRowsHeight();
    updateNewPageListX( 0 );
    updateNewPageListY( 0 );
    if( m_pSheet->isShowPageBorders() )
        emit sig_updateView( m_pSheet );

    if( checkPageLimit )
    {
        calculateZoomForPageLimitX();
        calculateZoomForPageLimitY();
    }

    m_pDoc->setModified( true );
}

bool KSpreadPrintNewPageEntry::operator==( KSpreadPrintNewPageEntry const & entry ) const
{
    return m_iStartItem == entry.m_iStartItem;
}

