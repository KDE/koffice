/* This file is part of the KDE project
   Copyright (C) 2006 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "DataManipulators.h"

#include <klocale.h>

#include "Cell.h"
#include "CellStorage.h"
#include "Doc.h"
#include "PointStorage.h"
#include "Sheet.h"
#include "StyleStorage.h"
#include "ValueCalc.h"

#include <float.h>
#include <math.h>

using namespace KSpread;

namespace KSpread {
class FormulaStorage : public PointStorage<Formula> {};
class LinkStorage : public PointStorage<QString> {};
class ValueStorage : public PointStorage<Value> {};
}

AbstractDataManipulator::AbstractDataManipulator ()
{
}

AbstractDataManipulator::~AbstractDataManipulator ()
{
  oldData.clear ();
}

bool AbstractDataManipulator::process (Element* element)
{
  QRect range = element->rect();
  for (int col = range.left(); col <= range.right(); ++col)
    for (int row = range.top(); row <= range.bottom(); ++row) {
      Value val;
      QString text;
      int colidx = col - range.left();
      int rowidx = row - range.top();
      bool parse = false;
      Format::Type fmtType = Format::None;
      
      // do nothing if we don't want a change here
      if (!wantChange (element, col, row))
        continue;
      
      if (m_reverse) {
        // reverse - use the stored value
        if (oldData.contains (colidx) && oldData[colidx].contains (rowidx)) {
          val = oldData[colidx][rowidx].val;
          text = oldData[colidx][rowidx].text;
          fmtType = oldData[colidx][rowidx].format;
          parse = (!text.isEmpty());   // parse if text not empty
        }
      } else {
        val = newValue (element, col, row, &parse, &fmtType);
        if (parse)
          text = val.asString();
      }

      // we have the data - set it !
      if (parse) {
        Cell cell = Cell( m_sheet, col, row);
        if (fmtType != Format::None)
        {
            Style style;
            style.setFormatType (fmtType);
            cell.setStyle( style );
        }
        cell.setCellText (text);
      } else {
        Cell cell( m_sheet, col, row );
        if (!(val.isEmpty() && cell.isDefault()))
        // nothing if value and cell both empty
        {
          Cell cell = Cell( m_sheet, col, row);
          cell.setCellValue (val, fmtType, text);
        }
      }
    }
  return true;
}

Value AbstractDataManipulator::stored (int col, int row, bool *parse)
{
  Value val = oldData[col][row].val;
  QString text = oldData[col][row].text;
  *parse = false;
  if (!text.isEmpty()) {
    val = Value (text);
    *parse = true;
  }
  return val;
}

bool AbstractDataManipulator::preProcessing ()
{
  // not the first run - data already stored ...
  if (!m_firstrun) return true;

  Region::Iterator endOfList(cells().end());
  for (Region::Iterator it = cells().begin(); it != endOfList; ++it)
  {
    QRect range = (*it)->rect();
    for (int col = range.left(); col <= range.right(); ++col)
      for (int row = range.top(); row <= range.bottom(); ++row)
      {
        Cell cell = Cell( m_sheet, col, row);
        if (!cell.isDefault())  // non-default cell - remember it
        {
          ADMStorage st;

          int colidx = col - range.left();
          int rowidx = row - range.top();

          if (cell.isFormula())
            st.text = cell.inputText();
          st.val = cell.value();
          st.format = cell.formatType();
          oldData[colidx][rowidx] = st;
        }
      }
  }
  return true;
}

AbstractDFManipulator::AbstractDFManipulator ()
{
  m_changeformat = true;
}

AbstractDFManipulator::~AbstractDFManipulator ()
{
  // delete all stored formats ...
  formats.clear();
}

bool AbstractDFManipulator::process (Element* element)
{
  // let parent class process it first
  AbstractDataManipulator::process (element);

  // don't continue if we have to change formatting
  if (!m_changeformat) return true;

  QRect range = element->rect();
  for (int col = range.left(); col <= range.right(); ++col)
  {
    for (int row = range.top(); row <= range.bottom(); ++row) {
      Cell cell( m_sheet, col, row );
      int colidx = col - range.left();
      int rowidx = row - range.top();
      // only non-default cells will be formatted
      // TODO: is this really correct ?
      if (!cell.isDefault())
      {
        Style style = m_reverse ? formats[colidx][rowidx] : newFormat (element, col, row);
        cell.setStyle( style );
      }
    }
  }
  return true;
}

bool AbstractDFManipulator::preProcessing ()
{
  // not the first run - data already stored ...
  if (!m_firstrun) return true;

  AbstractDataManipulator::preProcessing ();

  Region::Iterator endOfList(cells().end());
  for (Region::Iterator it = cells().begin(); it != endOfList; ++it)
  {
    QRect range = (*it)->rect();
    for (int col = range.left(); col <= range.right(); ++col)
      for (int row = range.top(); row <= range.bottom(); ++row)
      {
        Cell cell = Cell( m_sheet, col, row);
        if (!cell.isDefault())  // non-default cell - remember it
        {
          int colidx = col - range.left();
          int rowidx = row - range.top();
          Style style = cell.style();
          formats[colidx][rowidx] = style;
        }
      }
  }
  return true;
}

DataManipulator::DataManipulator ()
  : m_format (Format::None),
  m_parsing (false)
{
  // default name for DataManipulator, can be changed using setName
  m_name = i18n ("Change Value");
}

DataManipulator::~DataManipulator ()
{
}

Value DataManipulator::newValue (Element *element, int col, int row,
    bool *parsing, Format::Type *formatType)
{
  *parsing = m_parsing;
  if (m_format != Format::None)
    *formatType = m_format;
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  return data.element (colidx, rowidx);
}

SeriesManipulator::SeriesManipulator ()
{
  setName (i18n ("Insert Series"));
  
  m_type = Linear;
  m_last = -2;
}

SeriesManipulator::~SeriesManipulator ()
{
}

void SeriesManipulator::setupSeries (const QPoint &_marker, double start,
    double end, double step, Series mode, Series type)
{
  m_type = type;
  m_start = Value (start);
  m_step = Value (step);
  // compute cell count
  int numberOfCells = 1;
  if (type == Linear)
    numberOfCells = (int) ((end - start) / step + 1);
  if (type == Geometric)
    /* basically, A(n) = start * step ^ n
    * so when is end >= start * step ^ n ??
    * when n = ln(end/start) / ln(step)
    */
    // DBL_EPSILON is added to prevent rounding errors
    numberOfCells = (int) (log (end / start) / log (step) + DBL_EPSILON) + 1;

  // with this, generate range information
  Region range (_marker.x(), _marker.y(), (mode == Column) ? 1 : numberOfCells,
      (mode == Row) ? 1 : numberOfCells);

  // and add the range to the manipulator
  add (range);
}

Value SeriesManipulator::newValue (Element *element, int col, int row,
    bool *parse, Format::Type *)
{
  *parse = false;
  ValueCalc *calc = m_sheet->doc()->calc();

  // either colidx or rowidx is always zero
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  int which = (colidx > 0) ? colidx : rowidx;
  Value val;
  if (which == m_last + 1) {
    // if we are requesting next item in the series, which should almost always
    // be the case, we can use the pre-computed value to speed up the process
    if (m_type == Linear)
      val = calc->add (m_prev, m_step);
    if (m_type == Geometric)
      val = calc->mul (m_prev, m_step);
  }
  else {
    // otherwise compute from scratch
    val = m_start;
    for (int i = 0; i < which; ++i)
    {
      if (m_type == Linear)
        val = calc->add (val, m_step);
      if (m_type == Geometric)
        val = calc->mul (val, m_step);
    } 
  }
  // store last value
  m_prev = val;
  m_last = which;
  
  // return the computed value
  return val;
}

ArrayFormulaManipulator::ArrayFormulaManipulator ()
{
  setName (i18n ("Set Array Formula"));
}

ArrayFormulaManipulator::~ArrayFormulaManipulator ()
{
}

Value ArrayFormulaManipulator::newValue (Element *element, int col, int row,
                                 bool *parsing, Format::Type *)
{
  *parsing = true;
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();

  // fill in the cells ... top-left one gets the formula, the rest gets =INDEX
  // TODO: also fill in information about cells being a part of a range for GUI
  if (colidx || rowidx) {
    return Value(cellRef + QString::number (rowidx+1) + ';' +
        QString::number (colidx+1) + ')');
  } else {
    Cell cell = Cell( m_sheet, col, row);
    cellRef = "=INDEX(" + cell.name() + ';';
    return Value(m_text);
  }
}

FillManipulator::FillManipulator ()
{
  m_dir = Down;
  m_changeformat = true;
  m_name = i18n ("Fill Selection");
}

FillManipulator::~FillManipulator ()
{
}

Value FillManipulator::newValue (Element *element, int col, int row,
    bool *parse, Format::Type *fmtType)
{
  Q_UNUSED(fmtType)
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  int rows = element->rect().bottom() - element->rect().top() + 1;
  int cols = element->rect().right() - element->rect().left() + 1;
  switch (m_dir) {
    case Up: rowidx = rows - 1; break;
    case Down: rowidx = 0; break;
    case Left: colidx = cols - 1; break;
    case Right: colidx = 0; break;
  };
  return stored (colidx, rowidx, parse);
}

Style FillManipulator::newFormat (Element *element, int col, int row)
{
  QRect range = element->rect();
  int colidx = col - range.left();
  int rowidx = row - range.top();
  int rows = element->rect().bottom() - element->rect().top() + 1;
  int cols = element->rect().right() - element->rect().left() + 1;
  switch (m_dir) {
    case Up: rowidx = rows - 1; break;
    case Down: rowidx = 0; break;
    case Left: colidx = cols - 1; break;
    case Right: colidx = 0; break;
  };
  return formats[colidx][rowidx];
}

CaseManipulator::CaseManipulator ()
{
  m_mode = Upper;
  m_name = i18n ("Change Case");
}

CaseManipulator::~CaseManipulator ()
{
}

Value CaseManipulator::newValue (Element *element, int col, int row,
    bool *parse, Format::Type *)
{
  Q_UNUSED(element)
  // if we are here, we know that we want the change
  *parse = false;
  QString str = Cell( m_sheet, col, row).value().asString();
  switch (m_mode) {
    case Upper: str = str.toUpper();
    break;
    case Lower: str = str.toLower();
    break;
    case FirstUpper:
      if (str.length() > 0)
        str = str.at(0).toUpper() + str.right(str.length()-1);
    break;
  };
  return Value (str);
}

bool CaseManipulator::wantChange (Element *element, int col, int row)
{
  Q_UNUSED(element)
  Cell cell( m_sheet, col, row );
  // don't change cells with a formula
  if (cell.isFormula())
    return false;
  // don't change cells containing other things than strings
  if (!cell.value().isString())
    return false;
  // original version was dismissing text starting with '!' and '*', is this
  // necessary ?
  return true;
}



ShiftManipulator::ShiftManipulator()
    : Manipulator()
    , m_mode( Insert )
{
}

void ShiftManipulator::setReverse( bool reverse )
{
    m_reverse = reverse;
    m_mode = reverse ? Delete : Insert;
}

bool ShiftManipulator::process(Element* element)
{
    const QRect range = element->rect();
    if ( !m_reverse ) // insertion
    {
        // insert rows
        QVector< QPair<QPoint,Formula> > undoFormulas;
        QVector< QPair<QPoint,Value> > undoValues;
        QVector< QPair<QPoint,QString> > undoLinks;
        QList< QPair<QRectF,bool> > undoFusion;
        QList< QPair<QRectF,SharedSubStyle> > undoStyles;
        QList< QPair<QRectF,QString> > undoComment;
        QList< QPair<QRectF,Conditions> > undoConditions;
        QList< QPair<QRectF,Validity> > undoValidity;
        if ( m_direction == ShiftBottom )
        {
            m_sheet->insertShiftDown( range );
            undoFormulas = m_sheet->formulaStorage()->insertShiftDown( range );
            undoValues = m_sheet->valueStorage()->insertShiftDown( range );
            undoLinks = m_sheet->linkStorage()->insertShiftDown( range );
            undoFusion = m_sheet->fusionStorage()->insertShiftDown( range );
            undoStyles = m_sheet->styleStorage()->insertShiftDown( range );
            undoComment = m_sheet->commentStorage()->insertShiftDown( range );
            undoConditions = m_sheet->conditionsStorage()->insertShiftDown( range );
            undoValidity = m_sheet->validityStorage()->insertShiftDown( range );
        }
        else if ( m_direction == ShiftRight )
        {
            m_sheet->insertShiftRight( range );
            undoFormulas = m_sheet->formulaStorage()->insertShiftRight( range );
            undoValues = m_sheet->valueStorage()->insertShiftRight( range );
            undoLinks = m_sheet->linkStorage()->insertShiftRight( range );
            undoFusion = m_sheet->fusionStorage()->insertShiftRight( range );
            undoStyles = m_sheet->styleStorage()->insertShiftRight( range );
            undoComment = m_sheet->commentStorage()->insertShiftRight( range );
            undoConditions = m_sheet->conditionsStorage()->insertShiftRight( range );
            undoValidity = m_sheet->validityStorage()->insertShiftRight( range );
        }

        // create undo for styles, comments, conditions, validity
        if ( m_firstrun )
        {
            m_undoFormulas = undoFormulas;
            m_undoValues = undoValues;
            m_undoLinks = undoLinks;
            m_undoFusion = undoFusion;
            m_undoStyles = undoStyles;
            m_undoComment = undoComment;
            m_undoConditions = undoConditions;
            m_undoValidity = undoValidity;
        }

        // undo deletion
        if ( m_mode == Delete )
        {
            for ( int i = 0; i < m_undoFormulas.count(); ++i )
                m_sheet->formulaStorage()->insert( m_undoFormulas[i].first.x(), m_undoFormulas[i].first.y(), m_undoFormulas[i].second );
            for ( int i = 0; i < m_undoValues.count(); ++i )
                m_sheet->valueStorage()->insert( m_undoValues[i].first.x(), m_undoValues[i].first.y(), m_undoValues[i].second );
            for ( int i = 0; i < m_undoLinks.count(); ++i )
                m_sheet->linkStorage()->insert( m_undoLinks[i].first.x(), m_undoLinks[i].first.y(), m_undoLinks[i].second );
            for ( int i = 0; i < m_undoFusion.count(); ++i )
                m_sheet->fusionStorage()->insert( Region(m_undoFusion[i].first.toRect()), m_undoFusion[i].second );
            for ( int i = 0; i < m_undoStyles.count(); ++i )
                m_sheet->styleStorage()->insert( m_undoStyles[i].first.toRect(), m_undoStyles[i].second );
            for ( int i = 0; i < m_undoComment.count(); ++i )
                m_sheet->commentStorage()->insert( Region(m_undoComment[i].first.toRect()), m_undoComment[i].second );
            for ( int i = 0; i < m_undoConditions.count(); ++i )
                m_sheet->conditionsStorage()->insert( Region(m_undoConditions[i].first.toRect()), m_undoConditions[i].second );
            for ( int i = 0; i < m_undoValidity.count(); ++i )
                m_sheet->validityStorage()->insert( Region(m_undoValidity[i].first.toRect()), m_undoValidity[i].second );
        }
    }
    else // deletion
    {
        // delete rows
        QVector< QPair<QPoint,Formula> > undoFormulas;
        QVector< QPair<QPoint,Value> > undoValues;
        QVector< QPair<QPoint,QString> > undoLinks;
        QList< QPair<QRectF,bool> > undoFusion;
        QList< QPair<QRectF,SharedSubStyle> > undoStyles;
        QList< QPair<QRectF,QString> > undoComment;
        QList< QPair<QRectF,Conditions> > undoConditions;
        QList< QPair<QRectF,Validity> > undoValidity;
        if ( m_direction == ShiftBottom )
        {
            m_sheet->removeShiftUp( range );
            undoFormulas = m_sheet->formulaStorage()->removeShiftUp( range );
            undoValues = m_sheet->valueStorage()->removeShiftUp( range );
            undoLinks = m_sheet->linkStorage()->removeShiftUp( range );
            undoFusion = m_sheet->fusionStorage()->removeShiftUp( range );
            undoStyles = m_sheet->styleStorage()->removeShiftUp( range );
            undoComment = m_sheet->commentStorage()->removeShiftUp( range );
            undoConditions = m_sheet->conditionsStorage()->removeShiftUp( range );
            undoValidity = m_sheet->validityStorage()->removeShiftUp( range );
        }
        else if ( m_direction == ShiftRight )
        {
            m_sheet->removeShiftLeft( range );
            undoFormulas = m_sheet->formulaStorage()->removeShiftLeft( range );
            undoValues = m_sheet->valueStorage()->removeShiftLeft( range );
            undoLinks = m_sheet->linkStorage()->removeShiftLeft( range );
            undoFusion = m_sheet->fusionStorage()->removeShiftLeft( range );
            undoStyles = m_sheet->styleStorage()->removeShiftLeft( range );
            undoComment = m_sheet->commentStorage()->removeShiftLeft( range );
            undoConditions = m_sheet->conditionsStorage()->removeShiftLeft( range );
            undoValidity = m_sheet->validityStorage()->removeShiftLeft( range );
        }

        // create undo for styles, comments, conditions, validity
        if ( m_firstrun )
        {
            m_undoFormulas = undoFormulas;
            m_undoValues = undoValues;
            m_undoLinks = undoLinks;
            m_undoFusion = undoFusion;
            m_undoStyles = undoStyles;
            m_undoComment = undoComment;
            m_undoConditions = undoConditions;
            m_undoValidity = undoValidity;
        }

        // undo insertion
        if ( m_mode == Insert )
        {
            for ( int i = 0; i < m_undoFormulas.count(); ++i )
                m_sheet->formulaStorage()->insert( m_undoFormulas[i].first.x(), m_undoFormulas[i].first.y(), m_undoFormulas[i].second );
            for ( int i = 0; i < m_undoValues.count(); ++i )
                m_sheet->valueStorage()->insert( m_undoValues[i].first.x(), m_undoValues[i].first.y(), m_undoValues[i].second );
            for ( int i = 0; i < m_undoLinks.count(); ++i )
                m_sheet->linkStorage()->insert( m_undoLinks[i].first.x(), m_undoLinks[i].first.y(), m_undoLinks[i].second );
            for ( int i = 0; i < m_undoFusion.count(); ++i )
                m_sheet->fusionStorage()->insert( Region(m_undoFusion[i].first.toRect()), m_undoFusion[i].second );
            for ( int i = 0; i < m_undoStyles.count(); ++i )
                m_sheet->styleStorage()->insert( m_undoStyles[i].first.toRect(), m_undoStyles[i].second );
            for ( int i = 0; i < m_undoComment.count(); ++i )
                m_sheet->commentStorage()->insert( Region(m_undoComment[i].first.toRect()), m_undoComment[i].second );
            for ( int i = 0; i < m_undoConditions.count(); ++i )
                m_sheet->conditionsStorage()->insert( Region(m_undoConditions[i].first.toRect()), m_undoConditions[i].second );
            for ( int i = 0; i < m_undoValidity.count(); ++i )
                m_sheet->validityStorage()->insert( Region(m_undoValidity[i].first.toRect()), m_undoValidity[i].second );
        }
    }
    return true;
}

bool ShiftManipulator::postProcessing()
{
    m_sheet->refreshMergedCell();
    m_sheet->recalc();
    return true;
}

QString ShiftManipulator::name() const
{
    if ( !m_reverse )
        return i18n( "Insert Cells" );
    else
        return i18n( "Remove Cells" );
}
