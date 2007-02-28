/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2005-2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#include <QRegExp>

#include <kdebug.h>

#include "Canvas.h"
#include "Cell.h"
#include "Doc.h"
#include "Editors.h"
#include "RowColumnFormat.h"
#include "Sheet.h"
#include "View.h"

#include "Selection.h"

// TODO Stefan: Substract points in selections
// TODO Stefan: KPart signal (Events.h)

using namespace KSpread;

/***************************************************************************
  class Selection::Private
****************************************************************************/

class Selection::Private
{
public:
  Private(View *v)
  {
    view = v;
    sheet = 0;
    anchor = QPoint(1,1);
    cursor = QPoint(1,1);
    marker = QPoint(1,1);

    colors.push_back(Qt::red);
    colors.push_back(Qt::blue);
    colors.push_back(Qt::magenta);
    colors.push_back(Qt::darkRed);
    colors.push_back(Qt::darkGreen);
    colors.push_back(Qt::darkMagenta);
    colors.push_back(Qt::darkCyan);
    colors.push_back(Qt::darkYellow);

    multipleOccurences = false;
    selectionMode = MultipleCells;

    activeElement = 0;
    activeSubRegionStart = 0;
    activeSubRegionLength = 1;
  }

  View*  view;
  Sheet* sheet;
  QPoint anchor;
  QPoint cursor;
  QPoint marker;
  QList<QColor> colors;

  bool multipleOccurences : 1;
  Mode selectionMode : 2;

  int activeElement;
  int activeSubRegionStart;
  int activeSubRegionLength;
};

/***************************************************************************
  class Selection
****************************************************************************/
namespace KSpread {

Selection::Selection(View *view)
    : QObject(view)
    , Region(1,1)
    , d( new Private( view ) )
{
  d->activeSubRegionStart = 0;
  d->activeSubRegionLength = 1;
}

Selection::Selection(const Selection& selection)
    : QObject(selection.d->view)
    , Region()
    , d( new Private( selection.d->view ) )
{
  d->sheet = selection.d->sheet;
  d->activeSubRegionStart = 0;
  d->activeSubRegionLength = cells().count();
}

Selection::~Selection()
{
  delete d;
}

void Selection::initialize(const QPoint& point, Sheet* sheet)
{
  if (!isValid(point))
    return;

  if (!d->view->activeSheet())
    return;

  if (!sheet)
  {
    if (d->sheet)
    {
      sheet = d->sheet;
    }
    else
    {
      sheet = d->view->activeSheet();
    }
  }

  Region changedRegion(*this);
  changedRegion.add(extendToMergedAreas(QRect(d->anchor,d->marker)));

  // for the case of a merged cell
  QPoint topLeft(point);
  Cell cell( d->view->activeSheet(), point );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    topLeft = QPoint(cell.column(), cell.row());
  }

  d->anchor = topLeft;
  d->cursor = point;
  d->marker = topLeft;

  fixSubRegionDimension(); // TODO remove this sanity check
  int index = d->activeSubRegionStart + d->activeSubRegionLength;
  if (insert(index, topLeft, sheet/*, true*/))
  {
    // if the point was inserted
    clearSubRegion();
  }
  Element* element = cells()[d->activeSubRegionStart];
  // we end up with one element in the subregion
  d->activeSubRegionLength = 1;
  if (element && element->type() == Element::Point)
  {
    Point* point = static_cast<Point*>(element);
    point->setColor(d->colors[cells().size() % d->colors.size()]);
  }
  else if (element && element->type() == Element::Range)
  {
    Range* range = static_cast<Range*>(element);
    range->setColor(d->colors[cells().size() % d->colors.size()]);
  }

  d->activeElement = d->activeSubRegionStart;

  if (changedRegion == *this)
  {
    emitChanged(Region(topLeft, sheet));
    return;
  }
  changedRegion.add(topLeft, sheet);

  emitChanged(changedRegion);
}

void Selection::initialize(const QRect& range, Sheet* sheet)
{
  if (!isValid(range) || ( range == QRect(0,0,1,1) ))
      return;

  if (!d->view->activeSheet())
    return;

  if (d->selectionMode == SingleCell)
  {
    initialize(range.bottomRight(), sheet);
    return;
  }

  if (!sheet)
  {
    if (d->sheet)
    {
      sheet = d->sheet;
    }
    else
    {
      sheet = d->view->activeSheet();
    }
  }

  Region changedRegion(*this);
  changedRegion.add(extendToMergedAreas(QRect(d->anchor,d->marker)));

  // for the case of a merged cell
  QPoint topLeft(range.topLeft());
  Cell cell( d->view->activeSheet(), topLeft );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    topLeft = QPoint(cell.column(), cell.row());
  }

  // for the case of a merged cell
  QPoint bottomRight(range.bottomRight());
  cell = Cell( d->view->activeSheet(), bottomRight );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    bottomRight = QPoint(cell.column(), cell.row());
  }

  d->anchor = topLeft;
  d->cursor = bottomRight;
  d->marker = bottomRight;

  fixSubRegionDimension(); // TODO remove this sanity check
  int index = d->activeSubRegionStart + d->activeSubRegionLength;
  if (insert(index, QRect(topLeft, bottomRight), sheet/*, true*/))
  {
    // if the range was inserted
    clearSubRegion();
  }
  Element* element = cells()[d->activeSubRegionStart];
  // we end up with one element in the subregion
  d->activeSubRegionLength = 1;
  if (element && element->type() == Element::Point)
  {
    Point* point = static_cast<Point*>(element);
    point->setColor(d->colors[cells().size() % d->colors.size()]);
  }
  else if (element && element->type() == Element::Range)
  {
    Range* range = static_cast<Range*>(element);
    range->setColor(d->colors[cells().size() % d->colors.size()]);
  }

  d->activeElement = 0;

  if (changedRegion == *this)
  {
    return;
  }
  changedRegion.add(QRect(topLeft, bottomRight), sheet);

  emitChanged(changedRegion);
}

void Selection::initialize(const Region& region, Sheet* sheet)
{
  if (!region.isValid())
      return;

  if (d->selectionMode == SingleCell)
  {
    initialize(cells()[0]->rect().bottomRight(), sheet);
    return;
  }

  if (!sheet)
  {
    if (d->sheet)
    {
      sheet = d->sheet;
    }
    else
    {
      sheet = d->view->activeSheet();
    }
  }

  Region changedRegion(*this);
  changedRegion.add(extendToMergedAreas(QRect(d->anchor,d->marker)));

  // TODO Stefan: handle subregion insertion
  // TODO Stefan: handle obscured cells correctly
  clear();
  Element* element = add(region);
  if (element && element->type() == Element::Point)
  {
    Point* point = static_cast<Point*>(element);
    point->setColor(d->colors[cells().size() % d->colors.size()]);
  }
  else if (element && element->type() == Element::Range)
  {
    Range* range = static_cast<Range*>(element);
    range->setColor(d->colors[cells().size() % d->colors.size()]);
  }

  // for the case of a merged cell
  QPoint topLeft(cells().last()->rect().topLeft());
  Cell cell( d->view->activeSheet(), topLeft );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    topLeft = QPoint(cell.column(), cell.row());
  }

  // for the case of a merged cell
  QPoint bottomRight(cells().last()->rect().bottomRight());
  cell = Cell( d->view->activeSheet(), bottomRight );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    bottomRight = QPoint(cell.column(), cell.row());
  }

  d->anchor = topLeft;
  d->cursor = topLeft;
  d->marker = bottomRight;

  d->activeElement = cells().count() - 1;
  d->activeSubRegionStart = 0;
  d->activeSubRegionLength = cells().count();

  if (changedRegion == *this)
  {
    return;
  }
  changedRegion.add( region );

  emitChanged(changedRegion);
}

void Selection::update()
{
  emitChanged(*this);
}

void Selection::update(const QPoint& point)
{
  uint count = cells().count();

  if (d->selectionMode == SingleCell)
  {
    initialize(point);
    return;
  }

  if (cells().isEmpty())
  {
    add(point);
    d->activeSubRegionLength += cells().count() - count;
    return;
  }
  if (d->activeElement == cells().count())
  {
    // we're not empty, so this will not become again end()
    d->activeElement--;
  }

  Sheet* sheet = cells()[d->activeElement]->sheet();
  if (sheet != d->view->activeSheet())
  {
    extend(point);
    d->activeSubRegionLength += cells().count() - count;
    return;
  }

  // for the case of a merged cell
  QPoint topLeft(point);
  Cell cell( d->view->activeSheet(), point );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    topLeft = QPoint(cell.column(), cell.row());
  }

  if (topLeft == d->marker)
  {
    return;
  }

  QRect area1 = cells()[d->activeElement]->rect();
  QRect newRange = extendToMergedAreas(QRect(d->anchor, topLeft));

  delete cells().takeAt(d->activeElement);
  // returns iterator to the new element (before 'it') or 'it'
  insert(d->activeElement, newRange, sheet, d->multipleOccurences);
  d->activeSubRegionLength += cells().count() - count;

  // The call to insert() above can just return the iterator which has been
  // passed in. This may be cells.end(), if the old active element was the
  // iterator to the list's end (!= last element). So attempts to dereference
  // it will fail.
  if (d->activeElement == cells().count())
  {
    d->activeElement--;
  }

  QRect area2 = cells()[d->activeElement]->rect();
  Region changedRegion;

  bool newLeft   = area1.left() != area2.left();
  bool newTop    = area1.top() != area2.top();
  bool newRight  = area1.right() != area2.right();
  bool newBottom = area1.bottom() != area2.bottom();

  /* first, calculate some numbers that we'll use a few times */
  int farLeft = qMin(area1.left(), area2.left());
  int innerLeft = qMax(area1.left(), area2.left());

  int farTop = qMin(area1.top(), area2.top());
  int innerTop = qMax(area1.top(), area2.top());

  int farRight = qMax(area1.right(), area2.right());
  int innerRight = qMin(area1.right(), area2.right());

  int farBottom = qMax(area1.bottom(), area2.bottom());
  int innerBottom = qMin(area1.bottom(), area2.bottom());

  if (newLeft)
  {
    changedRegion.add(QRect(QPoint(farLeft, innerTop),
                      QPoint(innerLeft-1, innerBottom)));
    if (newTop)
    {
      changedRegion.add(QRect(QPoint(farLeft, farTop),
                        QPoint(innerLeft-1, innerTop-1)));
    }
    if (newBottom)
    {
      changedRegion.add(QRect(QPoint(farLeft, innerBottom+1),
                        QPoint(innerLeft-1, farBottom)));
    }
  }

  if (newTop)
  {
    changedRegion.add(QRect(QPoint(innerLeft, farTop),
                      QPoint(innerRight, innerTop-1)));
  }

  if (newRight)
  {
    changedRegion.add(QRect(QPoint(innerRight+1, innerTop),
                      QPoint(farRight, innerBottom)));
    if (newTop)
    {
      changedRegion.add(QRect(QPoint(innerRight+1, farTop),
                        QPoint(farRight, innerTop-1)));
    }
    if (newBottom)
    {
      changedRegion.add(QRect(QPoint(innerRight+1, innerBottom+1),
                        QPoint(farRight, farBottom)));
    }
  }

  if (newBottom)
  {
    changedRegion.add(QRect(QPoint(innerLeft, innerBottom+1),
                      QPoint(innerRight, farBottom)));
  }

  d->marker = topLeft;
  d->cursor = point;

  emitChanged(changedRegion);
}

void Selection::extend(const QPoint& point, Sheet* sheet)
{
  if (!isValid(point))
    return;

  if (isEmpty() || d->selectionMode == SingleCell)
  {
    initialize(point, sheet);
    return;
  }
  if (d->activeElement == cells().count())
  {
    // we're not empty, so this will not become again end()
    d->activeElement--;
  }

  kDebug() << k_funcinfo << endl;

  if (!sheet)
  {
    if (d->sheet)
    {
      sheet = d->sheet;
    }
    else
    {
      sheet = d->view->activeSheet();
    }
  }

  Region changedRegion = Region(extendToMergedAreas(QRect(d->marker,d->marker)));

  // for the case of a merged cell
  QPoint topLeft(point);
  Cell cell( d->view->activeSheet(), point );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    topLeft = QPoint(cell.column(), cell.row());
  }

  uint count = cells().count();
  if (d->multipleOccurences)
  {
    // always successful
    insert(++d->activeElement, point, sheet, false);
  }
  else
  {
    eor(topLeft, sheet);
    d->activeElement = cells().count() - 1;
  }
  d->anchor = cells()[d->activeElement]->rect().topLeft();
  d->cursor = cells()[d->activeElement]->rect().bottomRight();
  d->marker = d->cursor;

  d->activeSubRegionLength += cells().count() - count;

  changedRegion.add(topLeft, sheet);
  changedRegion.add(*this);

  emitChanged(changedRegion);
}

void Selection::extend(const QRect& range, Sheet* sheet)
{
  if (!isValid(range) || (range == QRect(0,0,1,1)))
    return;

  if (isEmpty() || d->selectionMode == SingleCell)
  {
    initialize(range, sheet);
    return;
  }
  if (d->activeElement == cells().count())
  {
    // we're not empty, so this will not become again end()
    d->activeElement--;
  }

  if (!sheet)
  {
    if (d->sheet)
    {
      sheet = d->sheet;
    }
    else
    {
      sheet = d->view->activeSheet();
    }
  }

  // for the case of a merged cell
  QPoint topLeft(range.topLeft());
  Cell cell( d->view->activeSheet(), topLeft );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    topLeft = QPoint(cell.column(), cell.row());
  }

  // for the case of a merged cell
  QPoint bottomRight(range.bottomRight());
  cell = Cell( d->view->activeSheet(), bottomRight );
  if (cell.isPartOfMerged())
  {
    cell = cell.masterCell();
    bottomRight = QPoint(cell.column(), cell.row());
  }

  d->anchor = topLeft;
  d->cursor = topLeft;
  d->marker = bottomRight;

  uint count = cells().count();
  Element* element = 0;
  if (d->multipleOccurences)
  {
    //always successful
    insert(++d->activeElement, extendToMergedAreas(QRect(topLeft, bottomRight)), sheet, false);
  }
  else
  {
    element = add(extendToMergedAreas(QRect(topLeft, bottomRight)), sheet);
    d->activeElement = cells().count() - 1;
  }
  if (element && element->type() == Element::Point)
  {
    Point* point = static_cast<Point*>(element);
    point->setColor(d->colors[cells().size() % d->colors.size()]);
  }
  else if (element && element->type() == Element::Range)
  {
    Range* range = static_cast<Range*>(element);
    range->setColor(d->colors[cells().size() % d->colors.size()]);
  }

  d->activeSubRegionLength += cells().count() - count;

  emitChanged(*this);
}

void Selection::extend(const Region& region)
{
  if (!region.isValid())
    return;

  uint count = cells().count();
  ConstIterator end(region.constEnd());
  for (ConstIterator it = region.constBegin(); it != end; ++it)
  {
    Element *element = *it;
    if (!element) continue;
    if (element->type() == Element::Point)
    {
      Point* point = static_cast<Point*>(element);
      extend(point->pos(), element->sheet());
    }
    else
    {
      extend(element->rect(), element->sheet());
    }
  }

  d->activeSubRegionLength += cells().count() - count;

  emitChanged(*this);
}

Selection::Element* Selection::eor(const QPoint& point, Sheet* sheet)
{
  if (isSingular())
  {
    return Region::add(point, sheet);
  }
  return Region::eor(point, sheet);
}

const QPoint& Selection::anchor() const
{
  return d->anchor;
}

const QPoint& Selection::cursor() const
{
  return d->cursor;
}

const QPoint& Selection::marker() const
{
  return d->marker;
}

bool Selection::isSingular() const
{
  return Region::isSingular();
}

QRectF Selection::selectionHandleArea() const
{
  int column, row;

  // complete rows/columns are selected, use the marker.
  if (isColumnOrRowSelected())
  {
    column = d->marker.x();
    row = d->marker.y();
  }
  else
  {
    column = lastRange().right();
    row = lastRange().bottom();
  }
  const Cell cell( d->view->activeSheet(), column, row );

  double xpos = d->view->activeSheet()->columnPosition( column );
  double ypos = d->view->activeSheet()->rowPosition( row );
  double width = cell.width();
  double height = cell.height();

  const double unzoomedXPixel = d->view->doc()->unzoomItX( 1.0 );
  const double unzoomedYPixel = d->view->doc()->unzoomItY( 1.0 );

  QRectF handle( xpos + width - 2 * unzoomedXPixel,
                ypos + height - 2 * unzoomedYPixel,
                5 * unzoomedXPixel,
                5 * unzoomedYPixel );
  return handle;
}

QString Selection::name(Sheet* sheet) const
{
  return Region::name(sheet ? sheet : d->sheet);
}

void Selection::setSheet(Sheet* sheet)
{
  d->sheet = sheet;
}

Sheet* Selection::sheet() const
{
  return d->sheet;
}

void Selection::setActiveElement(const QPoint& point)
{
  for (int index = 0; index < cells().count(); ++index)
  {
    QRect range = cells()[index]->rect();
    if (range.topLeft() == point || range.bottomRight() == point)
    {
      d->anchor = range.topLeft();
      d->cursor = range.bottomRight();
      d->marker = range.bottomRight();
      d->activeElement = index;
      d->activeSubRegionStart = index;
      d->activeSubRegionLength = 1;
      if (d->view->canvasWidget()->editor())
      {
        d->view->canvasWidget()->editor()->setCursorToRange(index);
      }
    }
  }
}

void Selection::setActiveElement(int pos)
{
  if (pos >= cells().count() || pos < 0 )
  {
    kDebug() << "Selection::setActiveElement: position exceeds list" << endl;
    d->activeElement = 0;
    return;
  }

  QRect range = cells()[pos]->rect();
  d->anchor = range.topLeft();
  d->cursor = range.bottomRight();
  d->marker = range.bottomRight();
  d->activeElement = pos;
}

Region::Element* Selection::activeElement() const
{
  return (d->activeElement == cells().count()) ? 0 : cells()[d->activeElement];
}

void Selection::clear()
{
  d->activeElement = 0;
  d->activeSubRegionStart = 0;
  d->activeSubRegionLength = 0;
  Region::clear();
}

void Selection::clearSubRegion()
{
  if (isEmpty())
  {
    return;
  }
  for (int index = 0; index < d->activeSubRegionLength; ++index)
  {
    delete cells().takeAt(d->activeSubRegionStart);
  }
  d->activeSubRegionLength = 0;
  d->activeElement = d->activeSubRegionStart;
}

void Selection::fixSubRegionDimension()
{
  if (d->activeSubRegionStart > cells().count())
  {
    kDebug() << "Selection::fixSubRegionDimension: start position exceeds list" << endl;
    d->activeSubRegionStart = 0;
    d->activeSubRegionLength = cells().count();
    return;
  }
  if (d->activeSubRegionStart + d->activeSubRegionLength > cells().count())
  {
    kDebug() << "Selection::fixSubRegionDimension: length exceeds list" << endl;
    d->activeSubRegionLength = cells().count() - d->activeSubRegionStart;
    return;
  }
}

void Selection::setActiveSubRegion(uint start, uint length)
{
//   kDebug() << k_funcinfo << endl;
  d->activeElement = start;
  d->activeSubRegionStart = start;
  d->activeSubRegionLength = length;
  fixSubRegionDimension();
}

QString Selection::activeSubRegionName() const
{
  QStringList names;
  int end = d->activeSubRegionStart + d->activeSubRegionLength;
  for (int index = d->activeSubRegionStart; index < end; ++index)
  {
    names += cells()[index]->name(d->sheet);
  }
  return names.isEmpty() ? "" : names.join(";");
}

void Selection::setMultipleOccurences(bool state)
{
  d->multipleOccurences = state;
}

void Selection::setSelectionMode(Mode mode)
{
  d->selectionMode = mode;
}

const QList<QColor>& Selection::colors() const
{
  return d->colors;
}

QRect Selection::firstRange() const
{
  return cells().isEmpty() ? QRect(1,1,1,1) : cells().first()->rect();
}

QRect Selection::lastRange() const
{
  return cells().isEmpty() ? QRect(1,1,1,1) : cells().last()->rect();
}

QRect Selection::extendToMergedAreas(QRect area) const
{
  if (!d->view->activeSheet())
	  return area;

  area = normalized( area );
  Cell cell( d->view->activeSheet(), area.left(), area.top() );

  if( Region::Range(area).isColumn() || Region::Range(area).isRow() )
  {
    return area;
  }
  else if ( !(cell.isPartOfMerged()) &&
              (cell.mergedXCells() + 1) >= area.width() &&
              (cell.mergedYCells() + 1) >= area.height())
  {
    /* if just a single cell is selected, we need to merge even when
    the obscuring isn't forced.  But only if this is the cell that
    is doing the obscuring -- we still want to be able to click on a cell
    that is being obscured.
    */
    area.setWidth(cell.mergedXCells() + 1);
    area.setHeight(cell.mergedYCells() + 1);
  }
  else
  {
    int top=area.top();
    int left=area.left();
    int bottom=area.bottom();
    int right=area.right();
    for ( int x = area.left(); x <= area.right(); x++ )
      for ( int y = area.top(); y <= area.bottom(); y++ )
    {
      cell = Cell( d->view->activeSheet(), x, y );
      if( cell.doesMergeCells())
      {
        right=qMax(right,cell.mergedXCells()+x);
        bottom=qMax(bottom,cell.mergedYCells()+y);
      }
      else if ( cell.isPartOfMerged() )
      {
        cell = cell.masterCell();
        left=qMin(left,cell.column());
        top=qMin(top,cell.row());
        bottom=qMax(bottom,cell.row() + cell.mergedYCells());
        right=qMax(right,cell.column() + cell.mergedXCells());
      }
    }

    area.setCoords(left,top,right,bottom);
  }
  return area;
}

Selection::Region::Point* Selection::createPoint(const QPoint& point) const
{
  return new Point(point);
}

Selection::Region::Point* Selection::createPoint(const QString& string) const
{
  return new Point(string);
}

Selection::Region::Point* Selection::createPoint(const Point& point) const
{
  return new Point(point);
}

Selection::Region::Range* Selection::createRange(const QRect& rect) const
{
  return new Range(rect);
}

Selection::Region::Range* Selection::createRange(const QString& string) const
{
  return new Range(string);
}

Selection::Region::Range* Selection::createRange(const Range& range) const
{
  return new Range(range);
}

void Selection::emitChanged(const Region& region)
{
  Sheet * const sheet = d->view->activeSheet();
  Region extendedRegion;
  ConstIterator end(region.constEnd());
  for (ConstIterator it = region.constBegin(); it != end; ++it)
  {
    Element* element = *it;
    QRect area = element->rect();

    const ColumnFormat *col;
    const RowFormat *rl;
    //look at if column is hiding.
    //if it's hiding refreshing column+1 (or column -1 )
    int left = area.left();
    int right = area.right();
    int top = area.top();
    int bottom = area.bottom();

    // a merged cells is selected
    if (element->type() == Region::Element::Point)
    {
      Cell cell( sheet, left, top );
      if (cell.doesMergeCells())
      {
        // extend to the merged region
        // prevents artefacts of the selection rectangle
        right += cell.mergedXCells();
        bottom += cell.mergedYCells();
      }
    }

    if ( right < KS_colMax )
    {
      do
      {
        right++;
        col = sheet->columnFormat( right );
      } while ( col->hidden() && right != KS_colMax );
    }
    if ( left > 1 )
    {
      do
      {
        left--;
        col = sheet->columnFormat( left );
      } while ( col->hidden() && left != 1);
    }

    if ( bottom < KS_rowMax )
    {
      do
      {
        bottom++;
        rl = sheet->rowFormat( bottom );
      } while ( rl->hidden() && bottom != KS_rowMax );
    }

    if ( top > 1 )
    {
      do
      {
        top--;
        rl = sheet->rowFormat( top );
      } while ( rl->hidden() && top != 1);
    }

    area.setLeft(left);
    area.setRight(right);
    area.setTop(top);
    area.setBottom(bottom);

    extendedRegion.add(area, element->sheet());
  }

  emit changed(extendedRegion);
}

void Selection::dump() const
{
  kDebug() << *this << endl;
  kDebug() << "d->activeElement: " << d->activeElement << endl;
  kDebug() << "d->activeSubRegionStart: " << d->activeSubRegionStart << endl;
  kDebug() << "d->activeSubRegionLength: " << d->activeSubRegionLength << endl;
}

/***************************************************************************
  class Point
****************************************************************************/

Selection::Point::Point(const QPoint& point)
  : Region::Point(point),
    m_color(Qt::black),
    m_columnFixed(false),
    m_rowFixed(false)
{
}

Selection::Point::Point(const QString& string)
  : Region::Point(string),
    m_color(Qt::black),
    m_columnFixed(false),
    m_rowFixed(false)
{
  if (!isValid())
  {
    return;
  }

  uint p = 0;
  // Fixed?
  if (string[p++] == '$')
  {
    m_columnFixed = true;
  }

  //search for the first character != text
  int result = string.indexOf( QRegExp("[^A-Za-z]+"), p );
  if (string[result] == '$')
  {
    m_rowFixed = true;
  }
}

/***************************************************************************
  class Range
****************************************************************************/

Selection::Range::Range(const QRect& range)
  : Region::Range(range),
    m_color(Qt::black),
    m_leftFixed(false),
    m_rightFixed(false),
    m_topFixed(false),
    m_bottomFixed(false)
{
}

Selection::Range::Range(const QString& string)
  : Region::Range(string),
    m_color(Qt::black),
    m_leftFixed(false),
    m_rightFixed(false),
    m_topFixed(false),
    m_bottomFixed(false)
{
  if (!isValid())
  {
    return;
  }

  int delimiterPos = string.indexOf(':');
  if (delimiterPos == -1)
  {
    return;
  }

  Selection::Point ul(string.left(delimiterPos));
  Selection::Point lr(string.mid(delimiterPos + 1));

  if (!ul.isValid() || !lr.isValid())
  {
    return;
  }
  m_leftFixed = ul.columnFixed();
  m_rightFixed = lr.columnFixed();
  m_topFixed = ul.rowFixed();
  m_bottomFixed = lr.rowFixed();
}

} // namespace KSpread

#include "Selection.moc"
