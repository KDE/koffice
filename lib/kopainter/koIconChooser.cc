/* This file is part of the KDE project
  Copyright (c) 1999 Carsten Pfeiffer (pfeiffer@kde.org)
  Copyright (c) 2002 Igor Jansen (rm@kde.org)

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

#include <koIconChooser.h>

#include <qpainter.h>
#include <qcursor.h>
#include <qhbox.h>
#include <qlayout.h>

KoPixmapWidget::KoPixmapWidget(const QPixmap &aPixmap, QWidget *parent, const char *name):
QFrame(parent, name, WStyle_Customize | WStyle_NoBorder)
{
  setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  mPixmap = aPixmap;
  int w = mPixmap.width() + 2 * lineWidth();
  int h = mPixmap.height() + 2 * lineWidth();
  resize(w, h);

  // center widget under mouse cursor
  QPoint p = QCursor::pos();
  move(p.x() - w / 2, p.y() - h / 2);
  show();
}

KoPixmapWidget::~KoPixmapWidget()
{
}

// paint the centered pixmap; don't overpaint the frame
void KoPixmapWidget::paintEvent(QPaintEvent *e)
{
  QFrame::paintEvent(e);
  QPainter p(this);
  p.setClipRect(e->rect());
  p.drawPixmap(lineWidth(), lineWidth(), mPixmap);
}



KoIconChooser::KoIconChooser(QSize aIconSize, QWidget *parent, const char *name):
QGridView(parent, name)
{
  QGridView::setBackgroundColor(white);

  mMargin = 2;
  setCellWidth(aIconSize.width() + 2 * mMargin);
  setCellHeight(aIconSize.height() + 2 * mMargin);

  mIconList.clear();
  mPixmapWidget = 0L;
  nCols = 0;
  mCurRow = 0;
  mCurCol = 0;
  mItemCount = 0;
  mItemWidth = aIconSize.width();
  mItemHeight = aIconSize.height();
}

KoIconChooser::~KoIconChooser()
{
  mIconList.clear();
  if(mPixmapWidget)
    delete mPixmapWidget;
}

void KoIconChooser::addItem(KoIconItem *item)
{
  mIconList.insert(mItemCount++, item);
  calculateCells();
}

bool KoIconChooser::removeItem(KoIconItem *item)
{
  bool ok = mIconList.remove(item);
  if( ok )
  {
    mItemCount--;
    calculateCells();
  }
  return ok;
}

void KoIconChooser::clear()
{
  mItemCount = 0;
  mIconList.clear();
  calculateCells();
}

// return 0L if there is no current item
KoIconItem *KoIconChooser::currentItem()
{
  return itemAt(mCurRow, mCurCol);
}

// sets the current item to item
// does NOT emit selected()  (should it?)
void KoIconChooser::setCurrentItem(KoIconItem *item)
{
  int index = mIconList.find(item);

  // item is available
  if(index != -1 && nCols > 0)
  {
    int oldRow = mCurRow;
    int oldCol = mCurCol;

    mCurRow = index / nCols;
    mCurCol = index % nCols;

    // repaint the old and the new item
    updateCell(oldRow, oldCol);
    updateCell(mCurRow, mCurCol);
  }
}

// eventually select the item, clicked on
void KoIconChooser::mousePressEvent(QMouseEvent *e)
{
  QGridView::mousePressEvent(e);
  if(e->button() == LeftButton)
  {
    QPoint p = e->pos();
    int row = rowAt(p.y());
    int col = columnAt(p.x());

    KoIconItem *item = itemAt(row, col);
    if(item)
    {
      const QPixmap &pix = item->pixmap();
      if(pix.width() > mItemWidth || pix.height() > mItemHeight)
      showFullPixmap(pix, p);

      int oldRow = mCurRow;
      int oldCol = mCurCol;

      mCurRow = row;
      mCurCol = col;

      updateCell(oldRow, oldCol);
      updateCell(mCurRow, mCurCol);

      emit selected( item );
    }
  }
}

// when a big item is shown in full size, delete it on mouseRelease
void KoIconChooser::mouseReleaseEvent(QMouseEvent */*e*/)
{
  if(mPixmapWidget)
  {
    delete mPixmapWidget;
    mPixmapWidget = 0L;
  }
}

// FIXME: implement keyboard navigation
void KoIconChooser::keyPressEvent(QKeyEvent *e)
{
  QGridView::keyPressEvent(e);
}

// recalculate the number of items that fit into one row
// set the current item again after calculating the new grid
void KoIconChooser::resizeEvent(QResizeEvent *e)
{
  QGridView::resizeEvent(e);

  KoIconItem *item = currentItem();
  int oldNCols = nCols;
  nCols = numCols();

  if(nCols != oldNCols)
  {
    setNumCols(nCols);
    calculateCells();
    setCurrentItem(item);
  }
}

// paint one cell
// mark the current item and center items smaller than the cellSize
// TODO: scale down big pixmaps and paint the size as text into the pixmap
void KoIconChooser::paintCell(QPainter *p, int row, int col)
{
  KoIconItem *item = itemAt(row, col);

  if(item)
  {
    const QPixmap &pix = item->pixmap();

    int x = mMargin;
    int y = mMargin;
    int pw = pix.width();
    int ph = pix.height();
    int cw = cellWidth();
    int ch = cellHeight();

    // center small pixmaps
    if(pw < mItemWidth)
      x = (cw - pw) / 2;
    if(ph < mItemHeight)
      y = (cw - ph) / 2;

    if((!item->hasValidThumb()) || (pw <= mItemWidth && ph <= mItemHeight))
      p->drawPixmap(x, y, pix, 0, 0, mItemWidth, mItemHeight);
    else
    {
      const QPixmap &thumbpix = item->thumbPixmap();
      x = mMargin;
      y = mMargin;
      pw = thumbpix.width();
      ph = thumbpix.height();
      cw = cellWidth();
      ch = cellHeight();

      // center small pixmaps
      if(pw < mItemWidth)
        x = (cw - pw) / 2;
      if(ph < mItemHeight)
        y = (cw - ph) / 2;
      p->drawPixmap(x, y, thumbpix, 0, 0, mItemWidth, mItemHeight);
    }

    // highlight current item
    if(row == mCurRow && col == mCurCol)
    {
      p->setPen(blue);
      p->drawRect(0, 0, cw, ch);
    }
    else
    {
      p->setPen(gray);
      p->drawRect(0, 0, cw+1, ch+1);
    }
  }
  else
  {
    // empty cell
    p->fillRect(0, 0, cellWidth(), cellHeight(), QBrush(white));
  }
}

// return the pointer of the item at (row,col) - beware, resizing disturbs
// rows and cols!
// return 0L if item is not found
KoIconItem *KoIconChooser::itemAt(int row, int col)
{
  return itemAt(cellIndex(row, col));
}

// return the pointer of the item at position index
// return 0L if item is not found
KoIconItem *KoIconChooser::itemAt(int index)
{
  if(index == -1 || index >= mItemCount)
    return 0L;
  else
    return mIconList.at(index);
}

// return the index of a cell, given row and column position
// maps directly to the position in the itemlist
// return -1 on failure
int KoIconChooser::cellIndex(int row, int col)
{
  if(row < 0 || col < 0)
    return -1;
  else
    return((row * nCols) + col);
}

// calculate the grid and set the number of rows and columns
// reorder all items approrpriately
void KoIconChooser::calculateCells()
{
  if(nCols == 0)
    return;
  bool update = isUpdatesEnabled();
  int rows = mItemCount / nCols;
  setUpdatesEnabled(false);
  if((rows * nCols) < mItemCount)
    rows++;
  setNumRows(rows);
  setUpdatesEnabled(update);
  repaint();
}

// show the full pixmap of a large item in an extra widget
void KoIconChooser::showFullPixmap(const QPixmap &pix, const QPoint &/*p*/)
{
  mPixmapWidget = new KoPixmapWidget(pix, 0L);
}

KoPatternChooser::KoPatternChooser( const QPtrList<KoIconItem> &list, QWidget *parent, const char *name )
 : QWidget( parent, name )
{
    // only serves as beautifier for the iconchooser
    //frame = new QHBox( this );
    //frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    chooser = new KoIconChooser( QSize(30,30), this, "pattern chooser" );

	QObject::connect( chooser, SIGNAL(selected( KoIconItem * ) ),
					            this, SIGNAL( selected( KoIconItem * )));

	QPtrListIterator<KoIconItem> itr( list );
	for( itr.toFirst(); itr.current(); ++itr )
		chooser->addItem( itr.current() );

	QVBoxLayout *mainLayout = new QVBoxLayout( this, 1, -1, "main layout" );
	mainLayout->addWidget( chooser, 10 );
}


KoPatternChooser::~KoPatternChooser()
{
  delete chooser;
  //delete frame;
}

// set the active pattern in the chooser - does NOT emit selected() (should it?)
void KoPatternChooser::setCurrentPattern( KoIconItem *pattern )
{
    chooser->setCurrentItem( pattern );
}

void KoPatternChooser::addPattern( KoIconItem *pattern )
{
    chooser->addItem( pattern );
}

// return the active pattern
KoIconItem *KoPatternChooser::currentPattern()
{
    return chooser->currentItem();
}

#include "koIconChooser.moc"
