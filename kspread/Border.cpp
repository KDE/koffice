/* This file is part of the KDE project
   
   Copyright 2006 Robert Knight <robertknight@gmail.com>
   Copyright 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2004 David Faure <faure@kde.org>
   Copyright 2004-2005 Meni Livne <livne@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Hamish Rodda <rodda@kde.org>
   Copyright 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2003 Lukas Tinkl <lukas@kde.org>
   Copyright 2000-2002 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Daniel Naber <daniel.naber@t-online.de>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 1999-2000 Stephan Kulow <coolo@kde.org>
   Copyright 2000 Bernd Wuebben <wuebben@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1999 Michael Reiher <michael.reiher@gmx.de>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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

// Qt
#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QScrollBar>
#include <QPainter>
#include <QRubberBand>
#include <QToolTip>

// KDE
#include <klocale.h>
#include <kwordwrap.h>

// KOffice

// KSpread
#include "Cell.h"
#include "Canvas.h"
#include "CanvasPrivate.h"
#include "Doc.h"
#include "Sheet.h"
#include "Format.h"
#include "Selection.h"
#include "Undo.h"
#include "View.h"

// Local
#include "Border.h"

using namespace KSpread;

/****************************************************************
 *
 * VBorder
 *
 ****************************************************************/

VBorder::VBorder( QWidget *_parent, Canvas *_canvas, View *_view)
    : QWidget( _parent )
{
  m_pView = _view;
  m_pCanvas = _canvas;
  m_lSize = 0;
  m_rubberband = 0;

  setAttribute( Qt::WA_StaticContents );

  setMouseTracking( true );
  m_bResize = false;
  m_bSelection = false;
  m_iSelectionAnchor=1;
  m_bMousePressed = false;

//   m_scrollTimer = new QTimer( this );
  connect( m_pView, SIGNAL( autoScroll( const QPoint & )),
           this, SLOT( slotAutoScroll( const QPoint &)) );
}


VBorder::~VBorder()
{
//     delete m_scrollTimer;
}

void VBorder::mousePressEvent( QMouseEvent * _ev )
{
  if ( !m_pView->koDocument()->isReadWrite() )
    return;

  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

  if ( _ev->button() == Qt::LeftButton )
  {
    m_bMousePressed = true;
    m_pView->enableAutoScroll();
  }

  double ev_PosY = m_pCanvas->d->view->doc()->unzoomItY( _ev->pos().y() ) + m_pCanvas->yOffset();
  double dHeight = m_pCanvas->d->view->doc()->unzoomItY( height() );
  m_bResize = false;
  m_bSelection = false;

  // We were editing a cell -> save value and get out of editing mode
  if ( m_pCanvas->editor() )
  {
    m_pCanvas->deleteEditor( true ); // save changes
  }

//   m_scrollTimer->start( 50 );

  // Find the first visible row and the y position of this row.
  double y;
  int row = sheet->topRow( m_pCanvas->yOffset(), y );

  // Did the user click between two rows?
  while ( y < ( dHeight + m_pCanvas->yOffset() ) && ( !m_bResize ) )
  {
    double h = sheet->rowFormat( row )->dblHeight();
    row++;
    if ( row > KS_rowMax )
      row = KS_rowMax;
    if ( ( ev_PosY >= y + h - 2 ) &&
         ( ev_PosY <= y + h + 1 ) &&
         !( sheet->rowFormat( row )->hidden() && row == 1 ) )
      m_bResize = true;
    y += h;
  }

  //if row is hide and it's the first row
  //you mustn't resize it.
  double tmp2;
  int tmpRow = sheet->topRow( ev_PosY - 1, tmp2 );
  if ( sheet->rowFormat( tmpRow )->hidden() && tmpRow == 1 )
      m_bResize = false;

  // So he clicked between two rows ?
  if ( m_bResize )
  {
    // Determine row to resize
    double tmp;
    m_iResizedRow = sheet->topRow( ev_PosY - 1, tmp );
    if ( !sheet->isProtected() )
      paintSizeIndicator( _ev->pos().y() );
  }
  else
  {
    m_bSelection = true;

    double tmp;
    int hit_row = sheet->topRow( ev_PosY, tmp );
    if ( hit_row > KS_rowMax )
        return;

    m_iSelectionAnchor = hit_row;

    if ( !m_pView->selectionInfo()->contains( QPoint(1, hit_row) ) ||
         !( _ev->button() == Qt::RightButton ) ||
         !m_pView->selectionInfo()->isRowSelected() )
    {
      QPoint newMarker( 1, hit_row );
      QPoint newAnchor( KS_colMax, hit_row );
      if (_ev->modifiers() == Qt::ControlModifier)
      {
        m_pView->selectionInfo()->extend(QRect(newAnchor, newMarker));
      }
      else if (_ev->modifiers() == Qt::ShiftModifier)
      {
        m_pView->selectionInfo()->update(newMarker);
      }
      else
      {
        m_pView->selectionInfo()->initialize(QRect(newAnchor, newMarker));
      }
    }

    if ( _ev->button() == Qt::RightButton )
    {
      QPoint p = mapToGlobal( _ev->pos() );
      m_pView->popupRowMenu( p );
      m_bSelection = false;
    }
    m_pView->updateEditWidget();
  }
}

void VBorder::mouseReleaseEvent( QMouseEvent * _ev )
{
    m_pView->disableAutoScroll();
//     if ( m_scrollTimer->isActive() )
//         m_scrollTimer->stop();

    m_bMousePressed = false;

    if ( !m_pView->koDocument()->isReadWrite() )
        return;

    register Sheet * const sheet = m_pView->activeSheet();
    if (!sheet)
	    return;

    double ev_PosY = m_pCanvas->d->view->doc()->unzoomItY( _ev->pos().y() ) + m_pCanvas->yOffset();

    if ( m_bResize )
    {
        // Remove size indicator painted by paintSizeIndicator
        if ( m_rubberband )
        {
            delete m_rubberband;
            m_rubberband = 0;
        }

        int start = m_iResizedRow;
        int end = m_iResizedRow;
        QRect rect;
        rect.setCoords( 1, m_iResizedRow, KS_colMax, m_iResizedRow );
        if ( m_pView->selectionInfo()->isRowSelected() )
        {
          if ( m_pView->selectionInfo()->contains( QPoint( 1, m_iResizedRow ) ) )
            {
                start = m_pView->selectionInfo()->lastRange().top();
                end = m_pView->selectionInfo()->lastRange().bottom();
                rect = m_pView->selectionInfo()->lastRange();
            }
        }

        double height = 0.0;
        double y = sheet->dblRowPos( m_iResizedRow );
        if ( ev_PosY - y <= 0.0 )
            height = 0.0;
        else
            height = ev_PosY - y;

        if ( !sheet->isProtected() )
        {
          if ( !m_pCanvas->d->view->doc()->undoLocked() )
          {
            //just resize
            if ( height != 0.0 )
            {
              // TODO Stefan: replace this
              UndoResizeColRow *undo = new UndoResizeColRow( m_pCanvas->d->view->doc(), sheet, Region(rect) );
                m_pCanvas->d->view->doc()->addCommand( undo );
            }
          }

          for( int i = start; i <= end; i++ )
          {
            RowFormat *rl = sheet->nonDefaultRowFormat( i );
            if ( height != 0.0 )
            {
              if ( !rl->hidden() )
                rl->setDblHeight( height );
            }
            else
            {
              m_pView->hideRow();
            }
          }

          delete m_lSize;
          m_lSize = 0;
        }
    }
    else if ( m_bSelection )
    {
      QRect rect = m_pView->selectionInfo()->lastRange();

        // TODO: please don't remove. Right now it's useless, but it's for a future feature
        // Norbert
        bool m_frozen = false;
        if ( m_frozen )
        {
            kDebug(36001) << "selected: T " << rect.top() << " B " << rect.bottom() << endl;

            int i;
            RowFormat * row;
            QList<int> hiddenRows;

            for ( i = rect.top(); i <= rect.bottom(); ++i )
            {
                row = sheet->rowFormat( i );
                if ( row->hidden() )
                {
                    hiddenRows.append(i);
                }
            }

            if ( hiddenRows.count() > 0 )
              m_pView->showRow();
        }
    }

    m_bSelection = false;
    m_bResize = false;
}

void VBorder::equalizeRow( double resize )
{
  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

  QRect selection( m_pView->selectionInfo()->selection() );
  if ( !m_pCanvas->d->view->doc()->undoLocked() )
  {
     UndoResizeColRow *undo = new UndoResizeColRow( m_pCanvas->d->view->doc(), sheet, Region(selection) );
     m_pCanvas->d->view->doc()->addCommand( undo );
  }
  RowFormat *rl;
  for ( int i = selection.top(); i <= selection.bottom(); i++ )
  {
     rl = sheet->nonDefaultRowFormat( i );
     resize = qMax( 2.0, resize);
     rl->setDblHeight( resize );
  }
}

void VBorder::mouseDoubleClickEvent(QMouseEvent*)
{
  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

  if ( !m_pView->koDocument()->isReadWrite() || sheet->isProtected() )
    return;

  m_pView->adjustRow();
}


void VBorder::mouseMoveEvent( QMouseEvent * _ev )
{
  if ( !m_pView->koDocument()->isReadWrite() )
    return;

  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

  double ev_PosY = m_pCanvas->d->view->doc()->unzoomItY( _ev->pos().y() ) + m_pCanvas->yOffset();
  double dHeight = m_pCanvas->d->view->doc()->unzoomItY( height() );

  // The button is pressed and we are resizing ?
  if ( m_bResize )
  {
    if ( !sheet->isProtected() )
      paintSizeIndicator( _ev->pos().y() );
  }
  // The button is pressed and we are selecting ?
  else if ( m_bSelection )
  {
    double y;
    int row = sheet->topRow( ev_PosY, y );
    if ( row > KS_rowMax )
      return;

    QPoint newAnchor = m_pView->selectionInfo()->anchor();
    QPoint newMarker = m_pView->selectionInfo()->marker();
    newMarker.setY( row );
    newAnchor.setY( m_iSelectionAnchor );
    m_pView->selectionInfo()->update(newMarker);

    if ( _ev->pos().y() < 0 )
      m_pCanvas->vertScrollBar()->setValue( (int) ev_PosY );
    else if ( _ev->pos().y() > m_pCanvas->height() )
    {
      if ( row < KS_rowMax )
      {
        RowFormat* rowFormat = sheet->rowFormat( row + 1 );
        y = sheet->dblRowPos( row + 1 );
        m_pCanvas->vertScrollBar()->setValue( (int) ( ev_PosY + rowFormat->dblHeight() - dHeight) );
      }
    }
  }
  // No button is pressed and the mouse is just moved
  else
  {

     //What is the internal size of 1 pixel
    const double unzoomedPixel = m_pCanvas->d->view->doc()->unzoomItY( 1.0 );
    double y;
    int tmpRow = sheet->topRow( m_pCanvas->yOffset(), y );

    while ( y < dHeight + m_pCanvas->yOffset() )
    {
      double h = sheet->rowFormat( tmpRow )->dblHeight();
      //if col is hide and it's the first column
      //you mustn't resize it.
      if ( ev_PosY >= y + h - 2 * unzoomedPixel &&
           ev_PosY <= y + h + unzoomedPixel &&
           !( sheet->rowFormat( tmpRow )->hidden() && tmpRow == 1 ) )
      {
        setCursor( Qt::SplitVCursor );
        return;
      }
      y += h;
      tmpRow++;
    }
    setCursor( Qt::ArrowCursor );
  }
}

void VBorder::slotAutoScroll(const QPoint& scrollDistance)
{
  // NOTE Stefan: This slot is triggered by the same signal as
  //              Canvas::slotAutoScroll and HBorder::slotAutoScroll.
  //              Therefore, nothing has to be done except the scrolling was
  //              initiated in this header.
  if (!m_bMousePressed)
    return;
//   kDebug() << "VBorder::slotAutoScroll(" << scrollDistance << " " << endl;
  if (scrollDistance.y() > 0 || scrollDistance.y() < -height())
  {
    m_pView->vertScrollBar()->setValue( m_pView->vertScrollBar()->value() + scrollDistance.y() );
  }
}

// TODO Stefan: Still needed?
#if 0
void VBorder::doAutoScroll()
{
    if ( !m_bMousePressed )
    {
        m_scrollTimer->stop();
        return;
    }

    QPoint pos( mapFromGlobal( QCursor::pos() ) );

    if ( pos.y() < 0 || pos.y() > height() )
    {
        QMouseEvent * event = new QMouseEvent(QEvent::MouseMove, pos,
                                              Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseMoveEvent( event );
        delete event;
    }

    //Restart timer
    m_scrollTimer->start( 50 );
}
#endif

void VBorder::wheelEvent( QWheelEvent* _ev )
{
  if ( m_pCanvas->vertScrollBar() )
    QApplication::sendEvent( m_pCanvas->vertScrollBar(), _ev );
}


void VBorder::paintSizeIndicator( int mouseY )
{
    register Sheet * const sheet = m_pView->activeSheet();
    if (!sheet)
        return;

    m_iResizePos = mouseY;

    // Dont make the row have a height < 2 pixel.
    double y = m_pCanvas->d->view->doc()->zoomItY( sheet->dblRowPos( m_iResizedRow ) - m_pCanvas->yOffset() );
    if ( m_iResizePos < y + 2 )
        m_iResizePos = (int) y;

    if ( !m_rubberband )
    {
        m_rubberband = new QRubberBand( QRubberBand::Line, m_pCanvas );
        m_rubberband->setGeometry( 0, m_iResizePos, m_pCanvas->width(), 2 );
        m_rubberband->show();
    }
    m_rubberband->move( 0, m_iResizePos );

    QString tmpSize;
    if ( m_iResizePos != y )
        tmpSize = i18n("Height: %1 %2", KoUnit::toUserValue( m_pCanvas->doc()->unzoomItY( m_iResizePos - y ), m_pView->doc()->unit() ) , m_pView->doc()->unitName() );
    else
        tmpSize = i18n( "Hide Row" );

    if ( !m_lSize )
    {
        m_lSize = new QLabel( m_pCanvas );
        m_lSize->setAlignment( Qt::AlignVCenter );
        m_lSize->setPalette( QToolTip::palette() );
        m_lSize->setText( tmpSize );
        if ( sheet->layoutDirection()==Sheet::RightToLeft )
            m_lSize->move( m_pCanvas->width() - m_lSize->width() - 3, (int)y + 3 );
        else
            m_lSize->move( 3, (int)y + 3 );
        m_lSize->show();
    }
    else
    {
        m_lSize->setText( tmpSize );
        if ( sheet->layoutDirection()==Sheet::RightToLeft )
            m_lSize->move( m_pCanvas->width() - m_lSize->width() - 3, (int)y + 3 );
        else
            m_lSize->move( 3, (int)y + 3 );
    }
}

void VBorder::updateRows( int from, int to )
{
    register Sheet * const sheet = m_pView->activeSheet();
    if (!sheet)
        return;

    double y0 = sheet->doc()->zoomItY( sheet->dblRowPos( from ) );
    double y1 = sheet->doc()->zoomItY( sheet->dblRowPos( to + 1 ) );
    update( 0, (int) y0, width(), (int) (y1-y0) );
}

void VBorder::paintEvent( QPaintEvent* event )
{
  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

//     ElapsedTime et( "Painting vertical header", ElapsedTime::PrintOnlyTime );

  // FIXME Stefan: Make use of clipping. Find the repaint call after the scrolling.
  // kDebug(36004) << event->rect() << endl;

  // painting rectangle
  const QRectF paintRect = m_pCanvas->d->view->doc()->viewToDocument( event->rect() );

  // the painter
  QPainter painter( this );
  painter.scale( m_pCanvas->d->view->doc()->zoomedResolutionX(), m_pCanvas->d->view->doc()->zoomedResolutionY() );
  painter.setRenderHint( QPainter::TextAntialiasing );

  // fonts
  QFont normalFont( painter.font() );
  QFont boldFont( normalFont );
  boldFont.setBold( true );

  // background brush/color
  const QBrush backgroundBrush( palette().window() );
  const QColor backgroundColor( backgroundBrush.color() );

  // selection brush/color
  QColor selectionColor( palette().highlight().color() );
  selectionColor.setAlpha( 127 );
  const QBrush selectionBrush( selectionColor );

  painter.setClipRect( paintRect );

  double yPos;
  // Get the top row and the current y-position
  int y = sheet->topRow( paintRect.y() + m_pCanvas->yOffset(), yPos );
  // Align to the offset
  yPos = yPos - m_pCanvas->yOffset();
  double width = YBORDER_WIDTH;

  const QSet<int> selectedRows = m_pView->selectionInfo()->rowsSelected();
  const QSet<int> affectedRows = m_pView->selectionInfo()->rowsAffected();
  // Loop through the rows, until we are out of range
  while ( yPos <= paintRect.bottom() )
  {
    const bool selected = (selectedRows.contains(y));
    const bool highlighted = (!selected && affectedRows.contains(y));

    const RowFormat* rowFormat = sheet->rowFormat( y );
    const double height = rowFormat->dblHeight();

    if ( selected || highlighted )
    {
      painter.setPen( selectionColor.dark(150) );
      painter.setBrush( selectionBrush );
    }
    else
    {
      painter.setPen( backgroundColor.dark(150) );
      painter.setBrush( backgroundBrush );
    }
    painter.drawRect( QRectF( 0, yPos, width, height ) );

    QString rowText = QString::number( y );

    // Reset painter
    painter.setFont( normalFont );
    painter.setPen( palette().text().color() );

    if ( selected )
      painter.setPen( palette().highlightedText().color() );
    else if ( highlighted )
      painter.setFont( boldFont );

    double len = painter.fontMetrics().width( rowText );
    if (!rowFormat->hidden())
        painter.drawText( QPointF( ( width - len ) / 2,
                                   yPos + ( height + painter.fontMetrics().ascent()
                                                   - painter.fontMetrics().descent() ) / 2 ),
                          rowText );

    yPos += rowFormat->dblHeight();
    y++;
  }
}


void VBorder::focusOutEvent( QFocusEvent* )
{
    m_pView->disableAutoScroll();
//     if ( m_scrollTimer->isActive() )
//         m_scrollTimer->stop();
    m_bMousePressed = false;
}


/****************************************************************
 *
 * HBorder
 *
 ****************************************************************/

HBorder::HBorder( QWidget *_parent, Canvas *_canvas,View *_view )
    : QWidget( _parent )
{
  m_pView = _view;
  m_pCanvas = _canvas;
  m_lSize = 0;
  m_rubberband = 0;

  setAttribute( Qt::WA_StaticContents );

  setMouseTracking( true );
  m_bResize = false;
  m_bSelection = false;
  m_iSelectionAnchor=1;
  m_bMousePressed = false;

//   m_scrollTimer = new QTimer( this );
  connect( m_pView, SIGNAL( autoScroll( const QPoint & )),
           this, SLOT( slotAutoScroll( const QPoint &)) );
}


HBorder::~HBorder()
{
//     delete m_scrollTimer;
}

void HBorder::mousePressEvent( QMouseEvent * _ev )
{
  if (!m_pView->koDocument()->isReadWrite())
    return;

  if ( _ev->button() == Qt::LeftButton )
  {
    m_bMousePressed = true;
    m_pView->enableAutoScroll();
  }

  const register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
	return;

  // We were editing a cell -> save value and get out of editing mode
  if ( m_pCanvas->editor() )
  {
      m_pCanvas->deleteEditor( true ); // save changes
  }

//   m_scrollTimer->start( 50 );

  double ev_PosX;
  double dWidth = m_pCanvas->d->view->doc()->unzoomItX( width() );
  if ( sheet->layoutDirection() == Sheet::RightToLeft )
    ev_PosX = dWidth - m_pCanvas->d->view->doc()->unzoomItX( _ev->pos().x() ) + m_pCanvas->xOffset();
  else
    ev_PosX = m_pCanvas->d->view->doc()->unzoomItX( _ev->pos().x() ) + m_pCanvas->xOffset();
  m_bResize = false;
  m_bSelection = false;

  // Find the first visible column and the x position of this column.
  double x;

  const double unzoomedPixel = m_pCanvas->d->view->doc()->unzoomItX( 1.0 );
  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    int tmpCol = sheet->leftColumn( m_pCanvas->xOffset(), x );

    kDebug() << "evPos: " << ev_PosX << ", x: " << x << ", COL: " << tmpCol << endl;
    while ( ev_PosX > x && ( !m_bResize ) )
    {
      double w = sheet->columnFormat( tmpCol )->dblWidth();

      kDebug() << "evPos: " << ev_PosX << ", x: " << x << ", w: " << w << ", COL: " << tmpCol << endl;

      ++tmpCol;
      if ( tmpCol > KS_colMax )
        tmpCol = KS_colMax;
      //if col is hide and it's the first column
      //you mustn't resize it.

      if ( ev_PosX >= x + w - unzoomedPixel &&
           ev_PosX <= x + w + unzoomedPixel &&
           !( sheet->columnFormat( tmpCol )->hidden() && tmpCol == 1 ) )
      {
        m_bResize = true;
      }
      x += w;
    }

    //if col is hide and it's the first column
    //you mustn't resize it.
    double tmp2;
    tmpCol = sheet->leftColumn( dWidth - ev_PosX + 1, tmp2 );
    if ( sheet->columnFormat( tmpCol )->hidden() && tmpCol == 0 )
    {
      kDebug() << "No resize: " << tmpCol << ", " << sheet->columnFormat( tmpCol )->hidden() << endl;
      m_bResize = false;
    }

    kDebug() << "Resize: " << m_bResize << endl;
  }
  else
  {
    int col = sheet->leftColumn( m_pCanvas->xOffset(), x );

    // Did the user click between two columns?
    while ( x < ( dWidth + m_pCanvas->xOffset() ) && ( !m_bResize ) )
    {
      double w = sheet->columnFormat( col )->dblWidth();
      col++;
      if ( col > KS_colMax )
        col = KS_colMax;
      if ( ( ev_PosX >= x + w - unzoomedPixel ) &&
         ( ev_PosX <= x + w + unzoomedPixel ) &&
           !( sheet->columnFormat( col )->hidden() && col == 1 ) )
        m_bResize = true;
      x += w;
    }

    //if col is hide and it's the first column
    //you mustn't resize it.
    double tmp2;
    int tmpCol = sheet->leftColumn( ev_PosX - 1, tmp2 );
    if ( sheet->columnFormat( tmpCol )->hidden() && tmpCol == 1 )
      m_bResize = false;
  }

  // So he clicked between two rows ?
  if ( m_bResize )
  {
    // Determine the column to resize
    double tmp;
    if ( sheet->layoutDirection()==Sheet::RightToLeft )
    {
      m_iResizedColumn = sheet->leftColumn( ev_PosX - 1, tmp );
      // kDebug() << "RColumn: " << m_iResizedColumn << ", PosX: " << ev_PosX << endl;

      if ( !sheet->isProtected() )
        paintSizeIndicator( _ev->pos().x() );
    }
    else
    {
      m_iResizedColumn = sheet->leftColumn( ev_PosX - 1, tmp );

      if ( !sheet->isProtected() )
        paintSizeIndicator( _ev->pos().x() );
    }

    // kDebug() << "Column: " << m_iResizedColumn << endl;
  }
  else
  {
    m_bSelection = true;

    double tmp;
    int hit_col = sheet->leftColumn( ev_PosX, tmp );
    if ( hit_col > KS_colMax )
        return;

    m_iSelectionAnchor = hit_col;

    if ( !m_pView->selectionInfo()->contains( QPoint( hit_col, 1 ) ) ||
         !( _ev->button() == Qt::RightButton ) ||
         !m_pView->selectionInfo()->isColumnSelected() )
    {
      QPoint newMarker( hit_col, 1 );
      QPoint newAnchor( hit_col, KS_rowMax );
      if (_ev->modifiers() == Qt::ControlModifier)
      {
        m_pView->selectionInfo()->extend(QRect(newAnchor, newMarker));
      }
      else if (_ev->modifiers() == Qt::ShiftModifier)
      {
        m_pView->selectionInfo()->update(newMarker);
      }
      else
      {
        m_pView->selectionInfo()->initialize(QRect(newAnchor, newMarker));
      }
    }

    if ( _ev->button() == Qt::RightButton )
    {
      QPoint p = mapToGlobal( _ev->pos() );
      m_pView->popupColumnMenu( p );
      m_bSelection = false;
    }
    m_pView->updateEditWidget();
  }
}

void HBorder::mouseReleaseEvent( QMouseEvent * _ev )
{
    m_pView->disableAutoScroll();
//     if ( m_scrollTimer->isActive() )
//         m_scrollTimer->stop();

    m_bMousePressed = false;

    if ( !m_pView->koDocument()->isReadWrite() )
      return;

    register Sheet * const sheet = m_pView->activeSheet();
    if (!sheet)
	    return;

    if ( m_bResize )
    {
        double dWidth = m_pCanvas->d->view->doc()->unzoomItX( width() );
        double ev_PosX;

        // Remove size indicator painted by paintSizeIndicator
        if ( m_rubberband )
        {
            delete m_rubberband;
            m_rubberband = 0;
        }

        int start = m_iResizedColumn;
        int end   = m_iResizedColumn;
        QRect rect;
        rect.setCoords( m_iResizedColumn, 1, m_iResizedColumn, KS_rowMax );
        if ( m_pView->selectionInfo()->isColumnSelected() )
        {
            if ( m_pView->selectionInfo()->contains( QPoint( m_iResizedColumn, 1 ) ) )
            {
                start = m_pView->selectionInfo()->lastRange().left();
                end   = m_pView->selectionInfo()->lastRange().right();
                rect  = m_pView->selectionInfo()->lastRange();
            }
        }

        double width = 0.0;
        double x;

        if ( sheet->layoutDirection()==Sheet::RightToLeft )
          ev_PosX = dWidth - m_pCanvas->d->view->doc()->unzoomItX( _ev->pos().x() ) + m_pCanvas->xOffset();
        else
          ev_PosX = m_pCanvas->d->view->doc()->unzoomItX( _ev->pos().x() ) + m_pCanvas->xOffset();

        x = sheet->dblColumnPos( m_iResizedColumn );

        if ( ev_PosX - x <= 0.0 )
          width = 0.0;
        else
          width = ev_PosX - x;

        if ( !sheet->isProtected() )
        {
          if ( !m_pCanvas->d->view->doc()->undoLocked() )
          {
            //just resize
            if ( width != 0.0 )
            {
              // TODO Stefan: replace this
                UndoResizeColRow *undo = new UndoResizeColRow( m_pCanvas->d->view->doc(), sheet, Region(rect) );
                m_pCanvas->d->view->doc()->addCommand( undo );
            }
          }

          for( int i = start; i <= end; i++ )
          {
            ColumnFormat *cl = sheet->nonDefaultColumnFormat( i );
            if ( width != 0.0 )
            {
                if ( !cl->hidden() )
                    cl->setDblWidth( width );
            }
            else
            {
              m_pView->hideColumn();
            }
          }

          delete m_lSize;
          m_lSize = 0;
        }
    }
    else if ( m_bSelection )
    {
        QRect rect = m_pView->selectionInfo()->lastRange();

        // TODO: please don't remove. Right now it's useless, but it's for a future feature
        // Norbert
        bool m_frozen = false;
        if ( m_frozen )
        {
            kDebug(36001) << "selected: L " << rect.left() << " R " << rect.right() << endl;

            int i;
            ColumnFormat * col;
            QList<int> hiddenCols;

            for ( i = rect.left(); i <= rect.right(); ++i )
            {
                col = sheet->columnFormat( i );
                if ( col->hidden() )
                {
                    hiddenCols.append(i);
                }
            }

            if ( hiddenCols.count() > 0 )
              m_pView->showColumn();
        }
    }

    m_bSelection = false;
    m_bResize = false;
}

void HBorder::equalizeColumn( double resize )
{
  register Sheet * const sheet = m_pView->activeSheet();
  Q_ASSERT( sheet );

  QRect selection( m_pView->selectionInfo()->selection() );
  if ( !m_pCanvas->d->view->doc()->undoLocked() )
  {
      UndoResizeColRow *undo = new UndoResizeColRow( m_pCanvas->d->view->doc(), sheet, Region(selection) );
      m_pCanvas->d->view->doc()->addCommand( undo );
  }
  ColumnFormat *cl;
  for ( int i = selection.left(); i <= selection.right(); i++ )
  {
      cl = sheet->nonDefaultColumnFormat( i );
      resize = qMax( 2.0, resize );
      cl->setDblWidth( resize );
  }

}

void HBorder::mouseDoubleClickEvent(QMouseEvent*)
{
  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

  if ( !m_pView->koDocument()->isReadWrite() || sheet->isProtected() )
    return;

  m_pView->adjustColumn();
}

void HBorder::mouseMoveEvent( QMouseEvent * _ev )
{
  if ( !m_pView->koDocument()->isReadWrite() )
    return;

  register Sheet * const sheet = m_pView->activeSheet();

  if (!sheet)
    return;

  double dWidth = m_pCanvas->d->view->doc()->unzoomItX( width() );
  double ev_PosX;
  if ( sheet->layoutDirection()==Sheet::RightToLeft )
    ev_PosX = dWidth - m_pCanvas->d->view->doc()->unzoomItX( _ev->pos().x() ) + m_pCanvas->xOffset();
  else
    ev_PosX = m_pCanvas->d->view->doc()->unzoomItX( _ev->pos().x() ) + m_pCanvas->xOffset();

  // The button is pressed and we are resizing ?
  if ( m_bResize )
  {
    if ( !sheet->isProtected() )
        paintSizeIndicator( _ev->pos().x() );
  }
  // The button is pressed and we are selecting ?
  else if ( m_bSelection )
  {
    double x;
    int col = sheet->leftColumn( ev_PosX, x );

    if ( col > KS_colMax )
      return;

    QPoint newMarker = m_pView->selectionInfo()->marker();
    QPoint newAnchor = m_pView->selectionInfo()->anchor();
    newMarker.setX( col );
    newAnchor.setX( m_iSelectionAnchor );
    m_pView->selectionInfo()->update(newMarker);

    if ( sheet->layoutDirection()==Sheet::RightToLeft )
    {
      if ( _ev->pos().x() < width() - m_pCanvas->width() )
      {
        ColumnFormat *cl = sheet->columnFormat( col + 1 );
        x = sheet->dblColumnPos( col + 1 );
        m_pCanvas->horzScrollBar()->setValue( m_pCanvas->horzScrollBar()->maximum()
                                              - (int) ( ( ev_PosX + cl->dblWidth() ) - dWidth ) );
      }
      else if ( _ev->pos().x() > width() )
        m_pCanvas->horzScrollBar()->setValue( (int) ( m_pCanvas->horzScrollBar()->maximum() - ( ev_PosX - dWidth + m_pCanvas->d->view->doc()->unzoomItX( m_pCanvas->width() ) ) ) );
    }
    else
    {
      if ( _ev->pos().x() < 0 )
        m_pCanvas->horzScrollBar()->setValue( (int) ev_PosX );
      else if ( _ev->pos().x() > m_pCanvas->width() )
      {
        if ( col < KS_colMax )
        {
          ColumnFormat *cl = sheet->columnFormat( col + 1 );
          x = sheet->dblColumnPos( col + 1 );
          m_pCanvas->horzScrollBar()->setValue( (int) ( ev_PosX + cl->dblWidth() - dWidth ) );
        }
      }
    }

  }
  // No button is pressed and the mouse is just moved
  else
  {
     //What is the internal size of 1 pixel
    const double unzoomedPixel = m_pCanvas->d->view->doc()->unzoomItX( 1.0 );
    double x;

    if ( sheet->layoutDirection()==Sheet::RightToLeft )
    {
      int tmpCol = sheet->leftColumn( m_pCanvas->xOffset(), x );

      while ( ev_PosX > x )
      {
        double w = sheet->columnFormat( tmpCol )->dblWidth();
        ++tmpCol;

        //if col is hide and it's the first column
        //you mustn't resize it.
        if ( ev_PosX >= x + w - unzoomedPixel &&
             ev_PosX <= x + w + unzoomedPixel &&
             !( sheet->columnFormat( tmpCol )->hidden() && tmpCol == 0 ) )
        {
          setCursor( Qt::SplitHCursor );
          return;
        }
        x += w;
      }
      setCursor( Qt::ArrowCursor );
    }
    else
    {
      int tmpCol = sheet->leftColumn( m_pCanvas->xOffset(), x );

      while ( x < m_pCanvas->d->view->doc()->unzoomItY( width() ) + m_pCanvas->xOffset() )
      {
        double w = sheet->columnFormat( tmpCol )->dblWidth();
        //if col is hide and it's the first column
        //you mustn't resize it.
        if ( ev_PosX >= x + w - unzoomedPixel &&
             ev_PosX <= x + w + unzoomedPixel &&
             !( sheet->columnFormat( tmpCol )->hidden() && tmpCol == 1 ) )
        {
          setCursor( Qt::SplitHCursor );
          return;
        }
        x += w;
        tmpCol++;
      }
      setCursor( Qt::ArrowCursor );
    }
  }
}

void HBorder::slotAutoScroll(const QPoint& scrollDistance)
{
  // NOTE Stefan: This slot is triggered by the same signal as
  //              Canvas::slotAutoScroll and VBorder::slotAutoScroll.
  //              Therefore, nothing has to be done except the scrolling was
  //              initiated in this header.
  if (!m_bMousePressed)
    return;
//   kDebug() << "HBorder::slotAutoScroll(" << scrollDistance << " " << endl;
  if (scrollDistance.x() > 0 || scrollDistance.x() < -width())
  {
    m_pView->horzScrollBar()->setValue( m_pView->horzScrollBar()->value() + scrollDistance.x() );
  }
}

// TODO Stefan: Still needed?
#if 0
void HBorder::doAutoScroll()
{
    if ( !m_bMousePressed )
    {
        m_scrollTimer->stop();
        return;
    }

    QPoint pos( mapFromGlobal( QCursor::pos() ) );

    if ( pos.x() < 0 || pos.x() > width() )
    {
        QMouseEvent * event = new QMouseEvent( QEvent::MouseMove, pos,
                                               Qt::NoButton, Qt::NoButton, Qt::NoModifier );
        mouseMoveEvent( event );
        delete event;
    }

    //Restart timer
    m_scrollTimer->start( 50 );
}
#endif

void HBorder::wheelEvent( QWheelEvent* _ev )
{
  if ( m_pCanvas->horzScrollBar() )
    QApplication::sendEvent( m_pCanvas->horzScrollBar(), _ev );
}

void HBorder::resizeEvent( QResizeEvent* _ev )
{
  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

  // workaround to allow horizontal resizing and zoom changing when sheet
  // direction and interface direction don't match (e.g. an RTL sheet on an
  // LTR interface)
  if ( sheet->layoutDirection()==Sheet::RightToLeft && !QApplication::isRightToLeft() )
  {
    int dx = _ev->size().width() - _ev->oldSize().width();
    scroll(dx, 0);
  }
  else if ( sheet->layoutDirection()==Sheet::LeftToRight && QApplication::isRightToLeft() )
  {
    int dx = _ev->size().width() - _ev->oldSize().width();
    scroll(-dx, 0);
  }
}

void HBorder::paintSizeIndicator( int mouseX )
{
    register Sheet * const sheet = m_pView->activeSheet();
    if (!sheet)
        return;

    if ( sheet->layoutDirection()==Sheet::RightToLeft )
      m_iResizePos = mouseX + m_pCanvas->width() - width();
    else
      m_iResizePos = mouseX;

    // Dont make the column have a width < 2 pixels.
    double x = m_pCanvas->d->view->doc()->zoomItX( sheet->dblColumnPos( m_iResizedColumn ) - m_pCanvas->xOffset() );

    if ( sheet->layoutDirection() == Sheet::RightToLeft )
    {
      x = m_pCanvas->width() - x;

      if ( m_iResizePos > x - 2 )
          m_iResizePos = (int) x;
    }
    else
    {
      if ( m_iResizePos < x + 2 )
          m_iResizePos = (int) x;
    }

    if ( !m_rubberband )
    {
        m_rubberband = new QRubberBand( QRubberBand::Line, m_pCanvas );
        m_rubberband->setGeometry( m_iResizePos, 0, 2, m_pCanvas->height() );
        m_rubberband->show();
    }
    m_rubberband->move( m_iResizePos, 0 );

    QString tmpSize;
    if ( m_iResizePos != x )
        tmpSize = i18n("Width: %1 %2", KGlobal::locale()->formatNumber( KoUnit::toUserValue( m_pCanvas->doc()->unzoomItX( (sheet->layoutDirection()==Sheet::RightToLeft) ? x - m_iResizePos : m_iResizePos - x ), m_pView->doc()->unit() )), m_pView->doc()->unitName() );
    else
        tmpSize = i18n( "Hide Column" );

    if ( !m_lSize )
    {
        m_lSize = new QLabel( m_pCanvas );
        m_lSize->setAlignment( Qt::AlignVCenter );
        m_lSize->setPalette( QToolTip::palette() );
        m_lSize->setText( tmpSize );
        if ( sheet->layoutDirection()==Sheet::RightToLeft )
            m_lSize->move( (int) x - m_lSize->width() - 3, 3 );
        else
            m_lSize->move( (int) x + 3, 3 );
        m_lSize->show();
    }
    else
    {
        m_lSize->setText( tmpSize );
        if ( sheet->layoutDirection()==Sheet::RightToLeft )
            m_lSize->move( (int) x - m_lSize->width() - 3, 3 );
        else
            m_lSize->move( (int) x + 3, 3 );
    }
}

void HBorder::updateColumns( int from, int to )
{
    register Sheet * const sheet = m_pView->activeSheet();
    if (!sheet)
        return;

    double x0 = sheet->doc()->zoomItX( sheet->dblColumnPos( from ) );
    double x1 = sheet->doc()->zoomItX( sheet->dblColumnPos( to + 1 ) );
    update( (int) x0, 0, (int) (x1-x0), height() );
}

void HBorder::paintEvent( QPaintEvent* event )
{
  register Sheet * const sheet = m_pView->activeSheet();
  if (!sheet)
    return;

//     ElapsedTime et( "Painting horizontal header", ElapsedTime::PrintOnlyTime );

  // FIXME Stefan: Make use of clipping. Find the repaint call after the scrolling.
  // kDebug(36004) << event->rect() << endl;

  // painting rectangle
  const QRectF paintRect = m_pView->doc()->viewToDocument( event->rect() );

  // the painter
  QPainter painter( this );
  painter.scale( m_pCanvas->d->view->doc()->zoomedResolutionX(), m_pCanvas->d->view->doc()->zoomedResolutionY() );
  painter.setRenderHint( QPainter::TextAntialiasing );

  // fonts
  QFont normalFont( painter.font() );
  QFont boldFont( normalFont );
  boldFont.setBold( true );

  // background brush/color
  const QBrush backgroundBrush( palette().window() );
  const QColor backgroundColor( backgroundBrush.color() );

  // selection brush/color
  QColor selectionColor( palette().highlight().color() );
  selectionColor.setAlpha( 127 );
  const QBrush selectionBrush( selectionColor );

  painter.setClipRect( paintRect );

  double xPos;
  int x;

  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    //Get the left column and the current x-position
    x = sheet->leftColumn( int( m_pCanvas->d->view->doc()->unzoomItX( width() ) - paintRect.x() + m_pCanvas->xOffset() ), xPos );
    //Align to the offset
    xPos = m_pCanvas->d->view->doc()->unzoomItX( width() ) - xPos + m_pCanvas->xOffset();
  }
  else
  {
    //Get the left column and the current x-position
    x = sheet->leftColumn( int( paintRect.x() + m_pCanvas->xOffset() ), xPos );
    //Align to the offset
    xPos = xPos - m_pCanvas->xOffset();
  }

  double height = painter.font().pointSizeF() + 5;

  if ( sheet->layoutDirection()==Sheet::RightToLeft )
  {
    if ( x > KS_colMax )
      x = KS_colMax;

    xPos -= sheet->columnFormat( x )->dblWidth();

    const QSet<int> selectedColumns = m_pView->selectionInfo()->columnsSelected();
    const QSet<int> affectedColumns = m_pView->selectionInfo()->columnsAffected();
    //Loop through the columns, until we are out of range
    while ( xPos <= paintRect.right() )
    {
      bool selected = (selectedColumns.contains(x));
      bool highlighted = (!selected && affectedColumns.contains(x));

      const ColumnFormat * col_lay = sheet->columnFormat( x );
      double width = xPos + col_lay->dblWidth() - xPos;

      if ( selected || highlighted )
      {
        painter.setPen( selectionColor.dark(150) );
        painter.setBrush( selectionBrush );
      }
      else
      {
        painter.setPen( backgroundColor.dark(150) );
        painter.setBrush( backgroundBrush );
      }
      painter.drawRect( QRectF( xPos, 0, width, height ) );

      // Reset painter
      painter.setFont( normalFont );
      painter.setPen( palette().text().color() );

      if ( selected )
        painter.setPen( palette().highlightedText().color() );
      else if ( highlighted )
        painter.setFont( boldFont );
      if ( !sheet->getShowColumnNumber() )
      {
        QString colText = Cell::columnName( x );
        double len = painter.fontMetrics().width( colText );
        if ( !col_lay->hidden() )
          painter.drawText( QPointF( xPos + ( width - len ) / 2,
                                     ( height + painter.fontMetrics().ascent() -
                                                painter.fontMetrics().descent() ) / 2 ),
                            colText );
      }
      else
      {
        QString tmp;
        double len = painter.fontMetrics().width( tmp.setNum(x) );
        if (!col_lay->hidden())
          painter.drawText( QPointF( xPos + ( width - len ) / 2,
                                     ( height + painter.fontMetrics().ascent() -
                                                painter.fontMetrics().descent() ) / 2 ),
                            tmp.setNum(x) );
      }
      xPos += col_lay->dblWidth();
      --x;
    }
  }
  else
  {
    const QSet<int> selectedColumns = m_pView->selectionInfo()->columnsSelected();
    const QSet<int> affectedColumns = m_pView->selectionInfo()->columnsAffected();
    //Loop through the columns, until we are out of range
    while ( xPos <= paintRect.right() )
    {
      bool selected = (selectedColumns.contains(x));
      bool highlighted = (!selected && affectedColumns.contains(x));

      const ColumnFormat *col_lay = sheet->columnFormat( x );
      double width = col_lay->dblWidth();

      QColor backgroundColor = palette().window().color();

      if ( selected || highlighted )
      {
        painter.setPen( selectionColor.dark(150) );
        painter.setBrush( selectionBrush );
      }
      else
      {
        painter.setPen( backgroundColor.dark(150) );
        painter.setBrush( backgroundBrush );
      }
      painter.drawRect( QRectF( xPos, 0, width, height ) );

      // Reset painter
      painter.setFont( normalFont );
      painter.setPen( palette().text().color() );

      if ( selected )
        painter.setPen( palette().highlightedText().color() );
      else if ( highlighted )
        painter.setFont( boldFont );
      if ( !sheet->getShowColumnNumber() )
      {
        QString colText = Cell::columnName( x );
        int len = painter.fontMetrics().width( colText );
        if (!col_lay->hidden())
          painter.drawText( QPointF( xPos + ( width - len ) / 2,
                                     ( height + painter.fontMetrics().ascent() -
                                                painter.fontMetrics().descent() ) / 2 ),
                            colText );
      }
      else
      {
        QString tmp;
        int len = painter.fontMetrics().width( tmp.setNum(x) );
        if (!col_lay->hidden())
          painter.drawText( QPointF( xPos + ( width - len ) / 2,
                                     ( height + painter.fontMetrics().ascent() -
                                                painter.fontMetrics().descent() ) / 2 ),
                            tmp.setNum(x) );
      }
      xPos += col_lay->dblWidth();
      ++x;
    }
  }
}


void HBorder::focusOutEvent( QFocusEvent* )
{
//     if ( m_scrollTimer->isActive() )
//         m_scrollTimer->stop();
    kDebug() << "HBorder::focusOutEvent(" << endl;
    m_pView->disableAutoScroll();
    m_bMousePressed = false;
}


/****************************************************************
 *
 * SelectAllButton
 *
 ****************************************************************/

SelectAllButton::SelectAllButton( View* view  )
    : QWidget( view )
    , m_view( view )
    , m_mousePressed( false )
{
}

SelectAllButton::~SelectAllButton()
{
}

void SelectAllButton::paintEvent( QPaintEvent* event )
{
    // painting rectangle
    const QRectF paintRect = m_view->doc()->viewToDocument( event->rect() );

    // the painter
    QPainter painter( this );
    painter.scale( m_view->doc()->zoomedResolutionX(), m_view->doc()->zoomedResolutionY() );

    painter.setClipRect( paintRect );

    // if all cells are selected
    if ( m_view->selectionInfo()->isAllSelected() )
    {
        // selection brush/color
        QColor selectionColor( palette().highlight().color() );
        selectionColor.setAlpha( 127 );
        const QBrush selectionBrush( selectionColor );

        painter.setPen( selectionColor.dark(150) );
        painter.setBrush( selectionBrush );
    }
    else
    {
        // background brush/color
        const QBrush backgroundBrush( palette().window() );
        const QColor backgroundColor( backgroundBrush.color() );

        painter.setPen( backgroundColor.dark(150) );
        painter.setBrush( backgroundBrush );
    }
    painter.drawRect( QRectF( 0, 0, width(), height() ) );
}

void SelectAllButton::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton )
        m_mousePressed = true;
}

void SelectAllButton::mouseReleaseEvent( QMouseEvent* event )
{
    Q_UNUSED(event);
    if ( !m_mousePressed )
        return;
    m_mousePressed = false;
    m_view->selectAll();
}

#include "Border.moc"
