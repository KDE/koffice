/* This file is part of the KDE project

   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright 2004-2005 Tomas Mecir <mecirt@gmail.com>
   Copyright 2004-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 1999-2002,2004,2005 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Reinhart Geiser <geiseri@kde.org>
   Copyright 2003-2005 Meni Livne <livne@kde.org>
   Copyright 2003 Peter Simonsson <psn@linux.se>
   Copyright 1999-2002 David Faure <faure@kde.org>
   Copyright 2000-2002 Werner Trobin <trobin@kde.org>
   Copyright 1999,2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1998-2000 Torben Weis <weis@kde.org>
   Copyright 2000 Bernd Wuebben <wuebben@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1999 Stephan Kulow <coolo@kde.org>
   Copyright 1999 Michael Reiher <michael.reiher.gmx.de>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1998-1999 Reginald Stadlbauer <reggie@kde.org>


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

#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <math.h>

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qregexp.h>

#include "kspread_canvas.h"
#include "kspread_condition.h"
#include "kspread_doc.h"
#include "kspread_format.h"
#include "kspread_global.h"
#include "kspread_map.h"
#include "kspread_sheet.h"
#include "kspread_sheetprint.h"
#include "kspread_style.h"
#include "kspread_style_manager.h"
#include "kspread_util.h"
#include "ksploadinginfo.h"
#include "kspread_genvalidationstyle.h"
#include "kspread_locale.h"
#include "kspread_value.h"
#include "kspread_view.h"
#include "kspread_value.h"
#include "formula.h"
#include "selection.h"
#include "valueconverter.h"
#include "valueformatter.h"
#include "valueparser.h"

#include <KoStyleStack.h>
#include <KoRect.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoXmlWriter.h>
#include <KoOasisStyles.h>

#include <kmessagebox.h>

#include <kdebug.h>

using namespace KSpread;

#define BORDER_SPACE 1


/**
 * A pointer to the decimal separator
 */

namespace Cell_LNS
{
  QChar decimal_point = '\0';
}

using namespace Cell_LNS;


// Some variables are placed in Cell::Extra because normally they're
// not required in simple case of cell(s). For example, most plain
// text cells don't need to store information about spanned columns
// and rows, as this is only the case with merged cells.
//
// When the cell is getting complex (e.g. merged with other cells,
// contains rich text, has validation criteria, etc), this Cell::Extra
// is allocated by Cell::Private and starts to be
// available. Otherwise, it won't exist at all.

class Cell::Extra
{
public:
  Extra() {}

  // Not empty when the cell holds a link
  QString link;

  // Number of cells explicitly merged by the user in X and Y directions.
  int mergedXCells;
  int mergedYCells;

  // Number of additional cells.
  int extraXCells;
  int extraYCells;

  // If this cell overlaps other cells, then we have the cells width and
  // height stored here.  These values do not mean anything unless
  // extraXCells and/or extraYCells are different from 0.
  double extraWidth;
  double extraHeight;

  // A list of cells that obscure this one.
  // If this list is not empty, then this cell is obscured by another
  // enlarged object. This means that we have to call this object in order
  // of painting it for example instead of painting 'this'.
  //
  // FIXME (comment): If the list consists of more than one obscuring
  //                  element, then is there an order between them that
  //                  is important?
  QValueList<Cell*> obscuringCells;

  // If non-NULL, contains a pointer to a condition or a validity test.
  Conditions  *conditions;
  Validity    *validity;

  // Store the number of line when multirow is used (default is 0)
  int nbLines;

private:
  // Don't allow implicit copy.
  Extra& operator=( const Extra& );
};


class Cell::Private
{
public:

  Private();
  ~Private();

public:

  // This cell's row and column. If either of them is 0, this is the
  // default cell and its row/column can not be determined.  Note that
  // in the isDefault() method, only column is tested.
  int  row;
  int  column;

  // Value of the cell, either typed by user or as result of formula
  Value value;

  // Holds the user's input.
  //
  // FIXME:
  // Eventually, we'll want to get rid of strText and generate
  // user's input on-the-fly. Then, for normal cells, we'll generate
  // this string using converter()->asString
  // (value()).
  //
  // Here the problem is, that strText also holds the formula -
  // we'll need to provide some method to generate it from the
  // parsed version, created in KSpread::Formula. Hence, we won't be
  // able to get rid of strText until we switch to the new formula
  // parser and until we write some method that re-generates the
  // input formula...
  //
  // Alternately, we can keep using strText for formulas and
  // generate it dynamically for static cells...
  //
  //  /Tomas
  //
  QString  strText;

  // This is the text we want to display. Not necessarily the same
  // as strText, e.g. strText="1" and strOutText="1.00" Also holds
  // value that we got from calculation, formerly known as
  // strFormulaOut
  QString  strOutText;

  // the Formula object for the cell
  KSpread::Formula *formula;

  // Position and dimension of displayed text.
  // FIXME (comment): Which coordinate system?  pixels?  mm/cm?  zoom?
  double  textX;
  double  textY;
  double  textWidth;
  double  textHeight;

  // result of "fm.ascent()" in makeLayout. used in offsetAlign.
  int  fmAscent;

  // Pointers to neighboring cells.
  // FIXME (comment): Which order?
  Cell  *nextCell;
  Cell  *previousCell;

  bool        hasExtra() const { return (cellExtra != 0); };
  Extra      *extra();

  Format     *format;
  Q_UINT32   flags;

private:
  // "Extra stuff", see explanation for Cell::Extra.
  Extra  *cellExtra;
};


Cell::Private::Private()
{
  // Some basic data.
  row     = 0;
  column  = 0;
  value   = Value::empty();
  formula = 0;

  // Formatting
  textX      = 0.0;
  textY      = 0.0;
  textWidth  = 0.0;
  textHeight = 0.0;
  fmAscent   = 0;

  nextCell     = 0;
  previousCell = 0;

  // Default is to not have the "extra" stuff in a cell.
  cellExtra = 0;
  format = 0;
  flags = 0;
}


Cell::Private::~Private()
{
  delete cellExtra;
  delete formula;
}


Cell::Extra* Cell::Private::extra()
{
    if ( !cellExtra ) {
      cellExtra = new Extra;
      cellExtra->conditions   = 0;
      cellExtra->validity     = 0;

      cellExtra->mergedXCells = 0;
      cellExtra->mergedYCells = 0;
      cellExtra->extraXCells  = 0;
      cellExtra->extraYCells  = 0;
      cellExtra->extraWidth   = 0.0;
      cellExtra->extraHeight  = 0.0;
      cellExtra->nbLines      = 0;
//      cellExtra->highlight    = QColor(0,0,0);
    }

    return cellExtra;
}


/*****************************************************************************
 *
 *                                 Cell
 *
 *****************************************************************************/


Cell::Cell( Sheet * _sheet, int _column, int _row )
{
  d = new Private;
  d->row = _row;
  d->column = _column;
  d->format = new Format(_sheet, _sheet->doc()->styleManager()->defaultStyle());
  d->format->setCell(this);
  clearAllErrors();
}


Cell::Cell( Sheet * _sheet, Style * _style,  int _column, int _row )
{
  d = new Private;
  d->row = _row;
  d->column = _column;
  d->format = new Format( _sheet, _style );
  d->format->setCell(this);

  clearAllErrors();
}

Format* Cell::format() const
{
  return d->format;
}

// Return the sheet that this cell belongs to.
Sheet * Cell::sheet() const
{
  return d->format->sheet();
}

// Return true if this is the default cell.
bool Cell::isDefault() const
{
  return ( d->column == 0 );
}

// Return the row number of this cell.
int Cell::row() const
{
  // Make sure this isn't called for the default cell.  This assert
  // can save you (could have saved me!) the hassle of some very
  // obscure bugs.

  if ( isDefault() )
  {
    kdWarning(36001) << "Error: Calling Cell::row() for default cell" << endl;
    return 0;
  }

  return d->row;
}


// Return the column number of this cell.
//
int Cell::column() const
{
  // Make sure this isn't called for the default cell.  This assert
  // can save you (could have saved me!) the hassle of some very
  // obscure bugs.
  if ( isDefault() )
  {
    kdWarning(36001) << "Error: Calling Cell::column() for default cell" << endl;
    return 0;
  }
  return d->column;
}


// Return the name of this cell, i.e. the string that the user would
// use to reference it.  Example: A1, BZ16
//
QString Cell::name() const
{
    return name( d->column, d->row );
}


// Return the name of any cell given by (col, row).
//
QString Cell::name( int col, int row )
{
    return columnName( col ) + QString::number( row );
}


// Return the name of this cell, including the sheet name.
// Example: sheet1!A5
//
QString Cell::fullName() const
{
    return fullName( sheet(), d->column, d->row );
}


// Return the full name of any cell given a sheet and (col, row).
//
QString Cell::fullName( const Sheet* s, int col, int row )
{
  return s->sheetName() + "!" + name( col, row );
}


// Return the symbolic name of the column of this cell.  Examples: A, BB.
//
QString Cell::columnName() const
{
  return columnName( d->column );
}

KLocale* Cell::locale() const
{
  return d->format->sheet()->doc()->locale();
}

// Return the symbolic name of any column.
//
QString Cell::columnName( uint column )
{
    QString   str;
    unsigned  digits = 1;
    unsigned  offset = 0;

    column--;

    if( column > 4058115285U ) return  QString("@@@");

    for( unsigned limit = 26; column >= limit+offset; limit *= 26, digits++ )
        offset += limit;

    for( unsigned c = column - offset; digits; --digits, c/=26 )
        str.prepend( QChar( 'A' + (c%26) ) );

    return str;
}


// Return true if this cell is a formula.
//
bool Cell::isFormula() const
{
    return d->strText[0] == '=';
}


// Return the input text of this cell.  This could, for instance, be a
// formula.
//
// FIXME: These two functions are inconsistently named.  It should be
//        either text() and outText() or strText() and strOutText().
//
QString Cell::text() const
{
    return d->strText;
}


// Return the out text, i.e. the text that is visible in the cells
// square when shown.  This could, for instance, be the calculated
// result of a formula.
//
QString Cell::strOutText() const
{
    return d->strOutText;
}

Formula *Cell::formula () const
{
  return d->formula;
}


// Return the value of this cell.
//
const Value Cell::value() const
{
  return d->value;
}


// Set the value of this cell.  It also clears all errors if the value
// itself is not an error.
//
// In addition to this, it calculates the outstring and sets the dirty
// flags so that a redraw is forced.
//
void Cell::setValue( const Value& v )
{
  if (v.type() != Value::Error)
    clearAllErrors();

  //If the value has not changed then we don't need to do anything
  //(ie. no need to relayout, update dependant cells etc.),
  //unless this cell contains a formula, in which case its dependancies might have changed
  //even though the value has not.  For example, if this cell was previously empty (and its value is
  //therefore empty) and a new dependency upon an empty cell has been added.  The new value would still
  //be empty, but the dependencies need to be updated (via the call to valueChanged() below).
  if ( ( d->value == v ) && ( !isFormula() ) )
    return;

  d->value = v;

  setFlag(Flag_LayoutDirty);
  setFlag(Flag_TextFormatDirty);

  // Format and set the outText.
  setOutputText();

  // Set the displayed text, if we hold an error value.
  if (d->value.type() == Value::Error)
    d->strOutText = d->value.errorMessage ();

  // Value of the cell has changed - trigger necessary actions
  valueChanged ();

    if ( !format()->sheet()->isLoading() )
        format()->sheet()->setRegionPaintDirty(cellRect());
}

void Cell::setCellValue (const Value &v, FormatType fmtType, const QString &txt)
{
    if ( !txt.isNull() )
    {
        d->strText = txt;
        if ( isFormula() )
            makeFormula();
    }
    else if ( !isFormula() )
        d->strText = sheet()->doc()->converter()->asString (v).asString();
    if (fmtType != No_format)
        format()->setFormatType (fmtType);
    setValue (v);
}

// FIXME: Continue commenting and cleaning here (ingwa)


Cell* Cell::previousCell() const
{
    return d->previousCell;
}

Cell* Cell::nextCell() const
{
    return d->nextCell;
}

void Cell::setPreviousCell( Cell* c )
{
    d->previousCell = c;
}

void Cell::setNextCell( Cell* c )
{
    d->nextCell = c;
}

Validity* Cell::getValidity( int newStruct  )
{
    if ( (!newStruct) && (!d->hasExtra()))
      //we don't have validity struct and we don't want one
      return 0;

    if( ( d->extra()->validity == 0 ) && ( newStruct == -1 ) )
        d->extra()->validity = new Validity;
    return  d->extra()->validity;
}

void Cell::removeValidity()
{
    if (!d->hasExtra())
      return;

    delete d->extra()->validity;
    d->extra()->validity = 0;
}


void Cell::copyFormat( const int column , const int row )
{
    const Cell * cell = format()->sheet()->cellAt( column , row );

    copyFormat( cell );
}

void Cell::copyFormat( const Cell* cell )
{

    Q_ASSERT(cell);

    d->value.setFormat(cell->d->value.format());
    format()->copy(*(cell->format()));

    /*format()->setAlign( cell->format()->align( _column, _row ) );
    format()->setAlignY( cell->format()->alignY( _column, _row ) );
    format()->setTextFont( cell->format()->textFont( _column, _row ) );
    format()->setTextColor( cell->format()->textColor( _column, _row ) );
    format()->setBgColor( cell->bgColor( _column, _row) );
    setLeftBorderPen( cell->leftBorderPen( _column, _row ) );
    setTopBorderPen( cell->topBorderPen( _column, _row ) );
    setBottomBorderPen( cell->bottomBorderPen( _column, _row ) );
    setRightBorderPen( cell->rightBorderPen( _column, _row ) );
    format()->setFallDiagonalPen( cell->format()->fallDiagonalPen( _column, _row ) );
    format()->setGoUpDiagonalPen( cell->format()->goUpDiagonalPen( _column, _row ) );
    format()->setBackGroundBrush( cell->backGroundBrush( _column, _row) );
    format()->setPrecision( cell->format()->precision( _column, _row ) );
    format()->setPrefix( cell->format()->prefix( _column, _row ) );
    format()->setPostfix( cell->format()->postfix( _column, _row ) );
    format()->setFloatFormat( cell->format()->floatFormat( _column, _row ) );
    format()->setFloatColor( cell->format()->floatColor( _column, _row ) );
    format()->setMultiRow( cell->format()->multiRow( _column, _row ) );
    format()->setVerticalText( cell->format()->verticalText( _column, _row ) );
    format()->setDontPrintText( cell->format()->getDontprintText(_column, _row ) );
    format()->setNotProtected( cell->format()->notProtected(_column, _row ) );
    format()->setHideAll(cell->format()->isHideAll(_column, _row ) );
    format()->setHideFormula(cell->format()->isHideFormula(_column, _row ) );
    format()->setIndent( cell->format()->getIndent(_column, _row ) );
    format()->setAngle( cell->format()->getAngle(_column, _row) );
    format()->setFormatType( cell->format()->getFormatType(_column, _row) );
    Format::Currency c;
    if ( cell->format()->currencyInfo( c ) )
      format()->setCurrency( c );*/

    QValueList<Conditional> conditionList = cell->conditionList();
    if (d->hasExtra())
      delete d->extra()->conditions;
    if ( cell->d->hasExtra() && cell->d->extra()->conditions )
      setConditionList( conditionList );
    else
      if (d->hasExtra())
        d->extra()->conditions = 0;

    /*format()->setComment( cell->format()->comment( _column, _row ) );*/
}

void Cell::copyAll( Cell *cell )
{
    Q_ASSERT( !isDefault() ); // trouble ahead...
    copyFormat( cell );
    copyContent( cell );
}

void Cell::copyContent( const Cell* cell )
{
    Q_ASSERT( !isDefault() ); // trouble ahead...

    if (cell->isFormula() && cell->column() > 0 && cell->row() > 0)
    {
      // change all the references, e.g. from A1 to A3 if copying
      // from e.g. B2 to B4
      QString d = cell->encodeFormula();
      setCellText( cell->decodeFormula( d ) );
    }
    else
      setCellText( cell->text() );

}

void Cell::defaultStyle()
{
  format()->defaultStyleFormat();

  if (!d->hasExtra())
    return;

  if ( d->extra()->conditions )
  {
    delete d->extra()->conditions;
    d->extra()->conditions = 0;
  }

  delete d->extra()->validity;
  d->extra()->validity = 0L;
}


// Merge a number of cells, i.e. make this cell obscure a number of
// other cells.  If _x and _y == 0, then the merging is removed.

void Cell::mergeCells( int _col, int _row, int _x, int _y )
{
  // Start by unobscuring the cells that we obscure right now
  int  extraXCells = d->hasExtra() ? d->extra()->extraXCells : 0;
  int  extraYCells = d->hasExtra() ? d->extra()->extraYCells : 0;
  for ( int x = _col; x <= _col + extraXCells; ++x ) {
    for ( int y = _row; y <= _row + extraYCells; ++y ) {
      if ( x != _col || y != _row )
        format()->sheet()->nonDefaultCell( x, y )->unobscure(this);
    }
  }

  // If no merging, then remove all traces, and return.
  if ( _x == 0 && _y == 0 ) {
    clearFlag( Flag_Merged );
    if (d->hasExtra()) {
      d->extra()->extraXCells  = 0;
      d->extra()->extraYCells  = 0;
      d->extra()->extraWidth   = 0.0;
      d->extra()->extraHeight  = 0.0;
      d->extra()->mergedXCells = 0;
      d->extra()->mergedYCells = 0;
    }

    // Refresh the layout
    setFlag( Flag_LayoutDirty );
    return;
  }

  // At this point, we know that we will merge some cells.
  setFlag(Flag_Merged);
  d->extra()->extraXCells  = _x;
  d->extra()->extraYCells  = _y;
  d->extra()->mergedXCells = _x;
  d->extra()->mergedYCells = _y;

  // Obscure the cells
  for ( int x = _col; x <= _col + _x; ++x ) {
    for ( int y = _row; y <= _row + _y; ++y ) {
      if ( x != _col || y != _row )
  format()->sheet()->nonDefaultCell( x, y )->obscure( this, true );
    }
  }

  // Refresh the layout
  setFlag( Flag_LayoutDirty );
}

void Cell::move( int col, int row )
{
    setLayoutDirtyFlag();
    setCalcDirtyFlag();
    setDisplayDirtyFlag();

    //int ex = extraXCells();
    //int ey = d->extra()->extraYCells();

    if (d->hasExtra())
      d->extra()->obscuringCells.clear();

    // Unobscure the objects we obscure right now
    int extraXCells = d->hasExtra() ? d->extra()->extraXCells : 0;
    int extraYCells = d->hasExtra() ? d->extra()->extraYCells : 0;
    for( int x = d->column; x <= d->column + extraXCells; ++x )
        for( int y = d->row; y <= d->row + extraYCells; ++y )
            if ( x != d->column || y != d->row )
            {
                Cell *cell = format()->sheet()->nonDefaultCell( x, y );
                cell->unobscure(this);
            }

    d->column = col;
    d->row    = row;

    if (d->hasExtra())
    {
      //    d->extra()->extraXCells = 0;
      //    d->extra()->extraYCells = 0;
      d->extra()->mergedXCells = 0;
      d->extra()->mergedYCells = 0;
    }

    // Cell value has been changed (because we're another cell now).
    valueChanged ();
}

void Cell::setLayoutDirtyFlag( bool format )
{
    setFlag( Flag_LayoutDirty );
    if ( format )
        setFlag( Flag_TextFormatDirty );

    if (!d->hasExtra())
      return;

    QValueList<Cell*>::iterator it  = d->extra()->obscuringCells.begin();
    QValueList<Cell*>::iterator end = d->extra()->obscuringCells.end();
    for ( ; it != end; ++it ) {
      (*it)->setLayoutDirtyFlag( format );
    }
}

bool Cell::needsPrinting() const
{
  if ( isDefault() )
    return false;

  if ( !d->strText.stripWhiteSpace().isEmpty() ) {
    return true;
  }

  // Cell borders?
  if ( format()->hasProperty( Format::PTopBorder )
       || format()->hasProperty( Format::PLeftBorder )
       || format()->hasProperty( Format::PRightBorder )
       || format()->hasProperty( Format::PBottomBorder )
       || format()->hasProperty( Format::PFallDiagonal )
       || format()->hasProperty( Format::PGoUpDiagonal ) ) {
    return true;
  }

  // Background color or brush?
  if ( format()->hasProperty( Format::PBackgroundBrush ) ) {

	const QBrush& brush=backGroundBrush(column(),row());

	//Only brushes that are visible (ie. they have a brush style and are not white)
	//need to be drawn
	if ( (brush.style() != Qt::NoBrush) &&
	     (brush.color() != Qt::white || brush.pixmap()) )
		return true;

  }

  if ( format()->hasProperty( Format::PBackgroundColor ) ) {
    kdDebug() << "needsPrinting: Has background colour" << endl;
    QColor backgroundColor=bgColor(column(),row());

    //We don't need to print anything if the background is white
    if (backgroundColor != Qt::white)
    	return true;
  }

  return false;
}

bool Cell::isEmpty() const
{
    return isDefault() || d->strText.isEmpty();
}


// Return true if this cell is obscured by some other cell.

bool Cell::isObscured() const
{
  if (!d->hasExtra())
    return false;

  return !( d->extra()->obscuringCells.isEmpty() );
}


// Return true if this cell is part of a merged cell, but not the
// master cell.

bool Cell::isPartOfMerged() const
{
  if (!d->hasExtra())
    return false;

  QValueList<Cell*>::const_iterator it = d->extra()->obscuringCells.begin();
  QValueList<Cell*>::const_iterator end = d->extra()->obscuringCells.end();
  for ( ; it != end; ++it ) {
    Cell *cell = *it;

    if (cell->doesMergeCells()) {
      // The cell might merge extra cells, and then overlap even
      // beyond that so just knowing that the obscuring cell merges
      // extra isn't enough.  We have to know that this cell is one of
      // the ones it is forcing over.
      if (column() <= cell->column() + cell->d->extra()->mergedXCells
    && row() <= cell->row() + cell->mergedYCells() )
  return true;
    }
  }

  return false;
}


// Return the cell that obscures this one.  If no cell is obscuring,
// then return this.  This method is slightly complicated because we
// can have several layers of obscuring.
//
// Update: it seems that if we do an actual merge, then the obscuring
// cell is prepended and if just expanding, then it is appended.  This
// means that we should be able to just look at the first one.

Cell *Cell::ultimateObscuringCell() const
{
  if (!d->hasExtra())
    return (Cell *) this;

  else if (d->extra()->obscuringCells.isEmpty())
    return (Cell *) this;

  else
    return d->extra()->obscuringCells.first();

#if 0
  QValueList<Cell*>::const_iterator it = d->extra()->obscuringCells.begin();
  QValueList<Cell*>::const_iterator end = d->extra()->obscuringCells.end();
  for ( ; it != end; ++it ) {
    Cell *cell = *it;

    if (cell->doesMergeCells()) {
      // The cell might merge extra cells, and then overlap even
      // beyond that so just knowing that the obscuring cell merges
      // extra isn't enough.  We have to know that this cell is one of
      // the ones it is forcing over.
      if (column() <= cell->column() + cell->d->extra()->mergedXCells
    && row() <= cell->row() + cell->mergedYCells() )
  return true;
    }
  }

  return false;
#endif
}


QValueList<Cell*> Cell::obscuringCells() const
{
  if (!d->hasExtra())
  {
    QValueList<Cell*> empty;
    return empty;
  }
  return d->extra()->obscuringCells;
}

void Cell::clearObscuringCells()
{
  if (!d->hasExtra())
    return;
  d->extra()->obscuringCells.clear();
}

void Cell::obscure( Cell *cell, bool isForcing )
{
  if (d->hasExtra())
  {
    d->extra()->obscuringCells.remove( cell ); // removes *all* occurrences
    cell->clearObscuringCells();
  }
  if ( isForcing )
  {
    d->extra()->obscuringCells.prepend( cell );
  }
  else
  {
    d->extra()->obscuringCells.append( cell );
  }
  setFlag(Flag_LayoutDirty);
  format()->sheet()->setRegionPaintDirty( cellRect() );
}

void Cell::unobscure( Cell * cell )
{
  if (d->hasExtra())
    d->extra()->obscuringCells.remove( cell );
  setFlag( Flag_LayoutDirty );
  format()->sheet()->setRegionPaintDirty( cellRect() );
}

QString Cell::encodeFormula( bool _era, int _col, int _row ) const
{
    if ( _col == -1 )
        _col = d->column;
    if ( _row == -1 )
        _row = d->row;

    QString erg = "";

    if(d->strText.isEmpty())
        return d->strText;

    bool fix1 = false;
    bool fix2 = false;
    bool onNumber = false;
    unsigned int pos = 0;
    const unsigned int length = d->strText.length();

    // All this can surely be made 10 times faster, but I just "ported" it to QString
    // without any attempt to optimize things -- this is really brittle (Werner)
    while ( pos < length )
    {
        if ( d->strText[pos] == '"' )
        {
            erg += d->strText[pos++];
            while ( pos < length && d->strText[pos] != '"' )  // till the end of the world^H^H^H "string"
            {
                erg += d->strText[pos++];
                // Allow escaped double quotes (\")
                if ( pos < length && d->strText[pos] == '\\' && d->strText[pos+1] == '"' )
                {
                    erg += d->strText[pos++];
                    erg += d->strText[pos++];
                }
            }
            if ( pos < length )  // also copy the trailing double quote
                erg += d->strText[pos++];

            onNumber = false;
        }
        else if ( d->strText[pos].isDigit() )
        {
          erg += d->strText[pos++];
          fix1 = fix2 = false;
          onNumber = true;
        }
        else if ( d->strText[pos] != '$' && !d->strText[pos].isLetter() )
        {
            erg += d->strText[pos++];
            fix1 = fix2 = false;
            onNumber = false;
        }
        else
        {
            QString tmp = "";
            if ( d->strText[pos] == '$' )
            {
                tmp = "$";
                pos++;
                fix1 = true;
            }
            if ( d->strText[pos].isLetter() )
            {
                QString buffer;
                unsigned int pos2 = 0;
                while ( pos < length && d->strText[pos].isLetter() )
                {
                    tmp += d->strText[pos];
                    buffer[pos2++] = d->strText[pos++];
                }
                if ( d->strText[pos] == '$' )
                {
                    tmp += "$";
                    pos++;
                    fix2 = true;
                }
                if ( d->strText[pos].isDigit() )
                {
                    const unsigned int oldPos = pos;
                    while ( pos < length && d->strText[pos].isDigit() ) ++pos;
                    int row = 0;
                    if ( pos != oldPos )
                        row = d->strText.mid(oldPos, pos-oldPos).toInt();
                    // Is it a sheet name || is it a function name like DEC2HEX
                    /* or if we're parsing a number, this could just be the
                       exponential part of it  (1.23E4) */
                    if ( ( d->strText[pos] == '!' ) ||
                         d->strText[pos].isLetter() ||
                         onNumber )
                    {
                        erg += tmp;
                        fix1 = fix2 = false;
                        pos = oldPos;
                    }
                    else // It must be a cell identifier
                    {
                        //now calculate the row as integer value
                        int col = 0;
                        col = util_decodeColumnLabelText( buffer );
                        if ( fix1 )
                            erg += QString( "$%1" ).arg( col );
                        else
                            if (_era)
                                erg += QChar(0xA7) + QString( "%1" ).arg( col );
                            else
                                erg += QString( "#%1" ).arg( col - _col );

                        if ( fix2 )
                            erg += QString( "$%1#").arg( row );
                        else
                            if (_era)
                                erg += QChar(0xA7) + QString( "%1#" ).arg( row );
                            else
                                erg += QString( "#%1#" ).arg( row - _row );
                    }
                }
                else
                {
                    erg += tmp;
                    fix1 = fix2 = false;
                }
            }
            else
            {
                erg += tmp;
                fix1 = false;
            }
            onNumber = false;
        }
    }

    return erg;
}

QString Cell::decodeFormula( const QString &_text, int _col, int _row) const
{
    if ( _col == -1 )
        _col = d->column;
    if ( _row == -1 )
        _row = d->row;

    QString erg = "";
    unsigned int pos = 0;
    const unsigned int length = _text.length();

    if ( _text.isEmpty() )
        return QString();

    while ( pos < length )
    {
        if ( _text[pos] == '"' )
        {
            erg += _text[pos++];
            while ( pos < length && _text[pos] != '"' )
            {
                erg += _text[pos++];
                // Allow escaped double quotes (\")
                if ( pos < length && _text[pos] == '\\' && _text[pos+1] == '"' )
                {
                    erg += _text[pos++];
                    erg += _text[pos++];
                }
            }
            if ( pos < length )
                erg += _text[pos++];
        }
        else if ( _text[pos] == '#' || _text[pos] == '$' || _text[pos] == QChar(0xA7))
        {
            bool abs1 = false;
            bool abs2 = false;
            bool era1 = false; // if 1st is relative but encoded absolutely
            bool era2 = false;

            QChar _t = _text[pos++];
            if ( _t == '$' )
                abs1 = true;
            else if ( _t == QChar(0xA7) )
                era1 = true;

            int col = 0;
            unsigned int oldPos = pos;
            while ( pos < length && ( _text[pos].isDigit() || _text[pos] == '-' ) ) ++pos;
            if ( pos != oldPos )
                col = _text.mid(oldPos, pos-oldPos).toInt();
            if ( !abs1 && !era1 )
                col += _col;
            // Skip '#' or '$'

            _t = _text[pos++];
            if ( _t == '$' )
                 abs2 = true;
            else if ( _t == QChar(0xA7) )
                 era2 = true;

            int row = 0;
            oldPos = pos;
            while ( pos < length && ( _text[pos].isDigit() || _text[pos] == '-' ) ) ++pos;
            if ( pos != oldPos )
                row = _text.mid(oldPos, pos-oldPos).toInt();
            if ( !abs2 && !era2)
                row += _row;
            // Skip '#' or '$'
            ++pos;
            if ( row < 1 || col < 1 || row > KS_rowMax || col > KS_colMax )
            {
                kdDebug(36001) << "Cell::decodeFormula: row or column out of range (col: " << col << " | row: " << row << ")" << endl;
                erg = "=\"#### " + i18n("REFERENCE TO COLUMN OR ROW IS OUT OF RANGE") + "\"";
                return erg;
            }
            if ( abs1 )
                erg += "$";
            erg += Cell::columnName(col); //Get column text

            if ( abs2 )
                erg += "$";
            erg += QString::number( row );
        }
        else
            erg += _text[pos++];
    }

    return erg;
}


void Cell::freeAllObscuredCells()
{
    //
    // Free all obscured cells.
    //

  if (!d->hasExtra())
    return;

  for ( int x = d->column + d->extra()->mergedXCells;
  x <= d->column + d->extra()->extraXCells; ++x ) {
    for ( int y = d->row + d->extra()->mergedYCells;
    y <= d->row + d->extra()->extraYCells; ++y ) {
      if ( x != d->column || y != d->row ) {
        Cell *cell = format()->sheet()->cellAt( x, y );
        cell->unobscure(this);
      }
    }
  }

  d->extra()->extraXCells = d->extra()->mergedXCells;
  d->extra()->extraYCells = d->extra()->mergedYCells;

}


// ----------------------------------------------------------------
//                              Layout


// Recalculate the entire layout.  This includes the following members:
//
//   d->textX,     d->textY
//   d->textWidth, d->textHeight
//   d->fmAscent
//   d->extra()->extraXCells, d->extra()->extraYCells
//   d->extra()->extraWidth,  d->extra()->extraHeight
//   d->extra()->nbLines (if multirow)
//
// and, of course,
//
//   d->strOutText
//

void Cell::makeLayout( QPainter &_painter, int _col, int _row )
{
  // Are _col and _row really needed ?
  //
  // Yes they are: they are useful if this is the default layout, in
  // which case d->row and d->column are 0 and 0, but _col and _row
  // are the real coordinates of the cell.

  // There is no need to remake the layout if it hasn't changed.
  if ( !testFlag( Flag_LayoutDirty ) )
    return;

  // Some initializations.
  if (d->hasExtra())
    d->extra()->nbLines = 0;
  clearFlag( Flag_CellTooShortX );
  clearFlag( Flag_CellTooShortY );

  // Initiate the cells that this one is obscuring to the ones that
  // are actually merged.
  freeAllObscuredCells();
  if (d->hasExtra())
    mergeCells( d->column, d->row,
         d->extra()->mergedXCells, d->extra()->mergedYCells );

  // If the column for this cell is hidden or the row is too low,
  // there is no use in remaking the layout.
  ColumnFormat  *cl1 = format()->sheet()->columnFormat( _col );
  RowFormat     *rl1 = format()->sheet()->rowFormat( _row );
  if ( cl1->isHide()
       || ( rl1->dblHeight() <= format()->sheet()->doc()->unzoomItY( 2 ) ) ) {
      clearFlag( Flag_LayoutDirty );
      return;
  }

  // Recalculate the output text, d->strOutText.
  setOutputText();

  // Empty text?  Reset the outstring and, if this is the default
  // cell, return.
  if ( d->strOutText.isEmpty() ) {
    d->strOutText = QString::null;

    if ( isDefault() ) {
      clearFlag( Flag_LayoutDirty );
      return;
    }
  }

  // Up to here, we have just cared about the contents, not the
  // painting of it.  Now it is time to see if the contents fits into
  // the cell and, if not, maybe rearrange the outtext a bit.
  //
  // First, Determine the correct font with zoom taken into account,
  // and apply it to _painter.  Then calculate text dimensions, i.e.
  // d->textWidth and d->textHeight.
  applyZoomedFont( _painter, _col, _row );
  textSize( _painter );

  //
  // Calculate the size of the cell
  //
  RowFormat     *rl = format()->sheet()->rowFormat( d->row );
  ColumnFormat  *cl = format()->sheet()->columnFormat( d->column );

  double         width  = cl->dblWidth();
  double         height = rl->dblHeight();

  // Calculate extraWidth and extraHeight if we have a merged cell.
  if ( testFlag( Flag_Merged ) ) {
    int  extraXCells = d->hasExtra() ? d->extra()->extraXCells : 0;
    int  extraYCells = d->hasExtra() ? d->extra()->extraYCells : 0;

    // FIXME: Introduce double extraWidth/Height here and use them
    //        instead (see FIXME about this in paintCell()).

    for ( int x = _col + 1; x <= _col + extraXCells; x++ )
      width += format()->sheet()->columnFormat( x )->dblWidth();

    for ( int y = _row + 1; y <= _row + extraYCells; y++ )
      height += format()->sheet()->rowFormat( y )->dblHeight();
  }

  // Cache the newly calculated extraWidth and extraHeight if we have
  // already allocated a struct for it.  Otherwise it will be zero, so
  // don't bother.
  if (d->hasExtra()) {
    d->extra()->extraWidth  = width;
    d->extra()->extraHeight = height;
  }

  QFontMetrics  fm = _painter.fontMetrics();
  d->fmAscent = fm.ascent();

  // Check if we need to break the line into multiple lines and are
  // allowed to do so.  If so, set `lines' to the number of lines that
  // are needed to fit into the total width of the combined cell.
  //
  // Also recalculate d->textHeight, d->textWidth, d->extra->nbLines
  // and d->strOutText.
  //
  int  lines = 1;
  if ( d->textWidth > (width - 2 * BORDER_SPACE
       - format()->leftBorderWidth( _col, _row )
       - format()->rightBorderWidth( _col, _row ) )
       && format()->multiRow( _col, _row ) )
  {
    // Copy of d->strOutText but without the newlines.
//     QString  o = d->strOutText.replace( QChar('\n'), " " );

    // don't remove the existing LF, these are intended line wraps (whishlist #9881)
    QString  o = d->strOutText;

    // Break the line at appropriate places, i.e. spaces, if
    // necessary.  This means to change the spaces where breaks occur
    // into newlines.
    if ( o.find(' ') != -1 )
    {
      d->strOutText = "";

      // Make sure that we have a space at the end.
      o += ' ';

      int start = 0;    // Start of the line we are handling now
      int breakpos = 0;   // The next candidate pos to break the string
      int pos1 = 0;
      int availableWidth = (int) ( width - 2 * BORDER_SPACE
          - format()->leftBorderWidth( _col, _row )
          - format()->rightBorderWidth( _col, _row ) );

      do {

        breakpos = o.find( ' ', breakpos );
        int linefeed = o.find( '\n', pos1 );

//         kdDebug() << "start: " << start << "; breakpos: " << breakpos << "; pos1: " << pos1 << "; linefeed: " << linefeed << endl;

        //don't miss LF as a position to calculate current lineWidth
        int work_breakpos = breakpos;
        if (pos1 < linefeed && linefeed < breakpos)
          work_breakpos = linefeed;

        double lineWidth = format()->sheet()->doc()
            ->unzoomItX( fm.width( d->strOutText.mid( start, (pos1 - start) )
                + o.mid( pos1, work_breakpos - pos1 ) ) );

        //linefeed could be -1 when no linefeed is found!
        if (breakpos > linefeed && linefeed > 0)
        {
//           kdDebug() << "applying linefeed to start;" << endl;
          start = linefeed;
          lines++;
        }

        if ( lineWidth <= availableWidth ) {
            // We have room for the rest of the line.  End it here.
            d->strOutText += o.mid( pos1, breakpos - pos1 );
            pos1 = breakpos;
        }
        else {
            // Still not enough room.  Try to split further.
            if ( o.at( pos1 ) == ' ' )
            pos1++;

            if ( pos1 != 0 && breakpos != -1 ) {
            d->strOutText += "\n" + o.mid( pos1, breakpos - pos1 );
            lines++;
            }
            else
            d->strOutText += o.mid( pos1, breakpos - pos1 );

            start = pos1;
            pos1 = breakpos;
        }

        breakpos++;
      } while( o.find( ' ', breakpos ) != -1 );
    }
    else
    {
      lines = o.contains('\n');
    }

    d->textHeight *= lines;
    if (lines > 1)
      d->extra()->nbLines = lines;

    d->textX = 0.0;

    // Calculate the maximum width, taking into account linebreaks,
    // and put it in d->textWidth.
    QString  t;
    int      i;
    int      pos = 0;
    d->textWidth = 0.0;
    do {
      i = d->strOutText.find( "\n", pos );

      if ( i == -1 )
  t = d->strOutText.mid( pos, d->strOutText.length() - pos );
      else {
  t = d->strOutText.mid( pos, i - pos );
  pos = i + 1;
      }

      double  tw = format()->sheet()->doc()->unzoomItX( fm.width( t ) );
      if ( tw > d->textWidth )
  d->textWidth = tw;
    } while ( i != -1 );
  }

  // Calculate d->textX and d->textY
  offsetAlign( _col, _row );

  int a = effAlignX();

  // Get indentation.  This is only used for left aligned text.
  double indent = 0.0;
  if ( a == Format::Left && !isEmpty() )
    indent = format()->getIndent( _col, _row );

  // Set Flag_CellTooShortX if the text is vertical or angled, and too
  // high for the cell.
  if ( format()->verticalText( _col, _row ) || format()->getAngle( _col, _row ) != 0 ) {
    //RowFormat  *rl = format()->sheet()->rowFormat( _row );

    if ( d->textHeight >= height/*rl->dblHeight()*/ )
      setFlag( Flag_CellTooShortX );
  }

  // Do we have to occupy additional cells to the right?  This is only
  // done for cells that have no merged cells in the Y direction.
  //
  // FIXME: Check if all cells along the merged edge to the right are
  //        empty and use the extra space?  No, probably not.
  //
  if ( d->textWidth + indent > ( width - 2 * BORDER_SPACE
         - format()->leftBorderWidth( _col, _row )
         - format()->rightBorderWidth( _col, _row ) )
       && ( !d->hasExtra() || d->extra()->mergedYCells == 0 ) )
  {
    int c = d->column;

    // Find free cells to the right of this one.
    int end = 0;
    while ( !end ) {
      ColumnFormat  *cl2  = format()->sheet()->columnFormat( c + 1 );
      Cell   *cell = format()->sheet()->visibleCellAt( c + 1, d->row );

      if ( cell->isEmpty() ) {
  width += cl2->dblWidth() - 1;
  c++;

  // Enough space?
  if ( d->textWidth + indent <= ( width - 2 * BORDER_SPACE
          - format()->leftBorderWidth( _col, _row )
          - format()->rightBorderWidth( _col, _row ) ) )
    end = 1;
      }
      else
  // Not enough space, but the next cell is not empty
  end = -1;
    }

    // Try to use additional space from the neighboring cells that
    // were calculated in the last step.  This is the place that we
    // set d->extra()->extraXCells and d->extra()->extraWidth.
    //
    // Currently this is only done for left aligned cells. We have to
    // check to make sure we haven't already force-merged enough cells
    //
    // FIXME: Why not right/center aligned text?
    //
    // FIXME: Shouldn't we check to see if end == -1 here before
    //        setting Flag_CellTooShortX?
    //
    if ( format()->align( _col, _row ) == Format::Left
         || ( format()->align( _col, _row ) == Format::Undefined
        && !value().isNumber() ) )
    {
      if ( c - d->column > d->extra()->mergedXCells ) {
  d->extra()->extraXCells = c - d->column;
  d->extra()->extraWidth  = width;
  for ( int i = d->column + 1; i <= c; ++i ) {
    Cell *cell = format()->sheet()->nonDefaultCell( i, d->row );
    cell->obscure( this );
  }

  // Not enough space
  if ( end == -1 )
    setFlag( Flag_CellTooShortX );
      }
      else
  setFlag( Flag_CellTooShortX );
    }
    else
      setFlag( Flag_CellTooShortX );
  }

  // Do we have to occupy additional cells at the bottom ?
  //
  // FIXME: Setting to make the current cell grow.
  //
  if ( format()->multiRow( _col, _row )
       && d->textHeight > ( height - 2 * BORDER_SPACE
          - format()->topBorderWidth( _col, _row )
          - format()->bottomBorderWidth( _col, _row ) ) )
  {
    int  r   = d->row;
    int  end = 0;

    // Find free cells bottom to this one
    while ( !end ) {
      RowFormat    *rl2  = format()->sheet()->rowFormat( r + 1 );
      Cell  *cell = format()->sheet()->visibleCellAt( d->column, r + 1 );

      if ( cell->isEmpty() ) {
  height += rl2->dblHeight() - 1.0;
  r++;

  // Enough space ?
  if ( d->textHeight <= ( height - 2 * BORDER_SPACE
        - format()->topBorderWidth( _col, _row )
        - format()->bottomBorderWidth( _col, _row ) ) )
    end = 1;
      }
      else
  // Not enough space, but the next cell is not empty.
  end = -1;
    }

    // Check to make sure we haven't already force-merged enough cells.
    if ( r - d->row > d->extra()->mergedYCells )
    {
      d->extra()->extraYCells = r - d->row;
      d->extra()->extraHeight = height;

      for ( int i = d->row + 1; i <= r; ++i )
      {
  	Cell  *cell = format()->sheet()->nonDefaultCell( d->column, i );
  	cell->obscure( this );
      }

      // Not enough space?
      if ( end == -1 )
  	setFlag( Flag_CellTooShortY );
    }
    else
      setFlag( Flag_CellTooShortY );
  }

  clearFlag( Flag_LayoutDirty );

  return;
}


void Cell::valueChanged ()
{
  update();

  format()->sheet()->valueChanged (this);
}


// Recalculate d->strOutText.
//

void Cell::setOutputText()
{
  if ( isDefault() ) {
    d->strOutText = QString::null;

    if ( d->hasExtra() && d->extra()->conditions )
      d->extra()->conditions->checkMatches();

    return;
  }

  // If nothing has changed, we don't need to remake the text layout.
  if ( !testFlag(Flag_TextFormatDirty) )
    return;

  // We don't want to remake the layout unnecessarily.
  clearFlag( Flag_TextFormatDirty );

  // Display a formula if warranted.  If not, display the value instead;
  // this is the most common case.
  if ( (!hasError()) && isFormula() && format()->sheet()->getShowFormula()
       && !( format()->sheet()->isProtected() && format()->isHideFormula( d->column, d->row ) )
       || isEmpty() )
    d->strOutText = d->strText;
  else {
    d->strOutText = sheet()->doc()->formatter()->formatText (this,
                   formatType());
  }

  // Check conditions if needed.
  if ( d->hasExtra() && d->extra()->conditions )
    d->extra()->conditions->checkMatches();
}


// Recalculate d->textX and d->textY.
//
// Used in makeLayout() and calculateTextParameters().
//

void Cell::offsetAlign( int _col, int _row )
{
  int       a;
  Format::AlignY  ay;
  int       tmpAngle;
  bool      tmpVerticalText;
  bool      tmpMultiRow;

  if ( d->hasExtra()
       && d->extra()->conditions
       && d->extra()->conditions->matchedStyle() )
  {
    Style  *style = d->extra()->conditions->matchedStyle();

    if ( style->hasFeature( Style::SAlignX, true ) )
      a = style->alignX();
    else
      a = format()->align( _col, _row );

    if ( style->hasFeature( Style::SVerticalText, true ) )
      tmpVerticalText = style->hasProperty( Style::PVerticalText );
    else
      tmpVerticalText = format()->verticalText( _col, _row );

    if ( style->hasFeature( Style::SMultiRow, true ) )
      tmpMultiRow = style->hasProperty( Style::PMultiRow );
    else
      tmpMultiRow = format()->multiRow( _col, _row );

    if ( style->hasFeature( Style::SAlignY, true ) )
      ay = style->alignY();
    else
      ay = format()->alignY( _col, _row );

    if ( style->hasFeature( Style::SAngle, true ) )
      tmpAngle = style->rotateAngle();
    else
      tmpAngle = format()->getAngle( _col, _row );
  }
  else {
    a               = format()->align( _col, _row );
    ay              = format()->alignY( _col, _row );
    tmpAngle        = format()->getAngle( _col, _row );
    tmpVerticalText = format()->verticalText( _col, _row );
    tmpMultiRow     = format()->multiRow( _col, _row );
  }

  RowFormat     *rl = format()->sheet()->rowFormat( _row );
  ColumnFormat  *cl = format()->sheet()->columnFormat( _col );

  double  w = cl->dblWidth();
  double  h = rl->dblHeight();

  if ( d->hasExtra() ) {
    if ( d->extra()->extraXCells )  w = d->extra()->extraWidth;
    if ( d->extra()->extraYCells )  h = d->extra()->extraHeight;
  }

  const double effTop = BORDER_SPACE + 0.5 * effTopBorderPen( _col, _row ).width();
  const double effBottom = h - BORDER_SPACE - 0.5 * effBottomBorderPen( _col, _row ).width();

  // Calculate d->textY based on the vertical alignment and a few
  // other inputs.
  switch( ay )
  {
  case Format::Top:
  {
    if ( tmpAngle == 0 )
    {
      d->textY = effTop + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
    }
    else if ( tmpAngle < 0 )
    {
      d->textY = effTop;
    }
    else
    {
      d->textY = effTop
               + (double) d->fmAscent * cos( tmpAngle * M_PI / 180 ) / format()->sheet()->doc()->zoomedResolutionY();
    }
    break;
  }
  case Format::Bottom:
  {
    if ( !tmpVerticalText && !tmpMultiRow && !tmpAngle )
    {
      d->textY = effBottom;
    }
    else if ( tmpAngle != 0 )
    {
      // Is enough place available?
      if ( effBottom - effTop - d->textHeight > 0 )
      {
        if ( tmpAngle < 0 )
        {
          d->textY = effBottom - d->textHeight;
        }
        else
        {
          d->textY = effBottom - d->textHeight
                   + ( (double) d->fmAscent * cos( tmpAngle * M_PI / 180 ) / format()->sheet()->doc()->zoomedResolutionY() );
        }
      }
      else
      {
        if ( tmpAngle < 0 )
        {
          d->textY = effTop;
        }
        else
        {
          d->textY = effTop
                  + ( (double) d->fmAscent * cos( tmpAngle * M_PI / 180 )
                  / format()->sheet()->doc()->zoomedResolutionY() );
        }
      }
    }
    else if ( tmpMultiRow && !tmpVerticalText )
    {
      // Is enough place available?
      if ( effBottom - effTop - d->textHeight > 0 )
      {
        d->textY = effBottom - d->textHeight
                 + (double)d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
      else
      {
        d->textY = effTop
                 + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
    }
    else
    {
      // Is enough place available?
      if ( effBottom - effTop - d->textHeight > 0 )
      {
        d->textY = effBottom - d->textHeight
                 + (double)d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
      else
      {
        d->textY = effTop
                 + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
    }
    break;
  }
  case Format::Middle:
  case Format::UndefinedY:
  {
    if ( !tmpVerticalText && !tmpMultiRow && !tmpAngle )
    {
      d->textY = ( h - d->textHeight ) / 2
               + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
    }
    else if ( tmpAngle != 0 )
    {
      // Is enough place available?
      if ( effBottom - effTop - d->textHeight > 0 )
      {
        if ( tmpAngle < 0 )
        {
          d->textY = ( h - d->textHeight ) / 2;
        }
        else
        {
          d->textY = ( h - d->textHeight ) / 2
                   + (double) d->fmAscent * cos( tmpAngle * M_PI / 180 ) / format()->sheet()->doc()->zoomedResolutionY();
        }
      }
      else
      {
        if ( tmpAngle < 0 )
        {
          d->textY = effTop;
        }
        else
        {
          d->textY = effTop
                   + ( (double)d->fmAscent * cos( tmpAngle * M_PI / 180 ) / format()->sheet()->doc()->zoomedResolutionY() );
        }
      }
    }
    else if ( tmpMultiRow && !tmpVerticalText )
    {
      // Is enough place available?
      if ( effBottom - effTop - d->textHeight > 0 )
      {
        d->textY = ( h - d->textHeight ) / 2
                 + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
      else
      {
        d->textY = effTop
                 + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
    }
    else
    {
      // Is enough place available?
      if ( effBottom - effTop - d->textHeight > 0 )
      {
        d->textY = ( h - d->textHeight ) / 2
                 + (double) d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
      }
      else
        d->textY = effTop
                 + (double)d->fmAscent / format()->sheet()->doc()->zoomedResolutionY();
    }
    break;
  }
  }

  a = effAlignX();
  if ( format()->sheet()->getShowFormula() &&
       !( format()->sheet()->isProtected() && format()->isHideFormula( _col, _row ) ) )
  {
    a = Format::Left;
  }

  // Calculate d->textX based on alignment and textwidth.
  switch ( a ) {
  case Format::Left:
    d->textX = 0.5 * effLeftBorderPen( _col, _row ).width() + BORDER_SPACE;
    break;
  case Format::Right:
    d->textX = w - BORDER_SPACE - d->textWidth
             - 0.5 * effRightBorderPen( _col, _row ).width();
    break;
  case Format::Center:
    d->textX = 0.5 * ( w - BORDER_SPACE - d->textWidth -
                       0.5 * effRightBorderPen( _col, _row ).width() );
    break;
  }
}


// Recalculate the current text dimensions, i.e. d->textWidth and
// d->textHeight.
//
// Used in makeLayout() and calculateTextParameters().
//
void Cell::textSize( QPainter &_paint )
{
  QFontMetrics  fm = _paint.fontMetrics();
  // Horizontal text ?

  int    tmpAngle;
  int    _row = row();
  int    _col = column();
  bool   tmpVerticalText;
  bool   fontUnderlined;
  Format::AlignY ay;

  // Set tmpAngle, tmpeVerticalText, ay and fontUnderlined according
  // to if there is a matching condition or not.
  if ( d->hasExtra()
       && d->extra()->conditions
       && d->extra()->conditions->matchedStyle() )
  {
    Style  *style = d->extra()->conditions->matchedStyle();

    if ( style->hasFeature( Style::SAngle, true ) )
      tmpAngle = style->rotateAngle();
    else
      tmpAngle = format()->getAngle( _col, _row );

    if ( style->hasFeature( Style::SVerticalText, true ) )
      tmpVerticalText = style->hasProperty( Style::PVerticalText );
    else
      tmpVerticalText = format()->verticalText( _col, _row );

    if ( style->hasFeature( Style::SAlignY, true ) )
      ay = style->alignY();
    else
      ay = format()->alignY( _col, _row );

    if ( style->hasFeature( Style::SFontFlag, true ) )
      fontUnderlined = ( style->fontFlags() & (uint) Style::FUnderline );
    else
      fontUnderlined = format()->textFontUnderline( _col, _row );
  }
  else {
    // The cell has no condition with a maxed style.
    tmpAngle        = format()->getAngle( _col, _row );
    tmpVerticalText = format()->verticalText( _col, _row );
    ay              = format()->alignY( _col, _row );
    fontUnderlined  = format()->textFontUnderline( _col, _row );
  }

  // Set d->textWidth and d->textHeight to correct values according to
  // if the text is horizontal, vertical or rotated.
  if ( !tmpVerticalText && !tmpAngle ) {
    // Horizontal text.

    d->textWidth = format()->sheet()->doc()->unzoomItX( fm.width( d->strOutText ) );
    int offsetFont = 0;
    if ( ( ay == Format::Bottom ) && fontUnderlined ) {
      offsetFont = fm.underlinePos() + 1;
    }

    d->textHeight = format()->sheet()->doc()->unzoomItY( fm.ascent() + fm.descent()
            + offsetFont );
  }
  else if ( tmpAngle!= 0 ) {
    // Rotated text.

    d->textHeight = format()->sheet()->doc()
      ->unzoomItY( int( cos( tmpAngle * M_PI / 180 )
      * ( fm.ascent() + fm.descent() )
      + abs( int( ( fm.width( d->strOutText )
              * sin( tmpAngle * M_PI / 180 ) ) ) ) ) );

    d->textWidth = format()->sheet()->doc()
      ->unzoomItX( int( abs( int( ( sin( tmpAngle * M_PI / 180 )
            * ( fm.ascent() + fm.descent() ) ) ) )
      + fm.width( d->strOutText )
        * cos ( tmpAngle * M_PI / 180 ) ) );
  }
  else {
    // Vertical text.
    int width = 0;
    for ( unsigned int i = 0; i < d->strOutText.length(); i++ )
      width = QMAX( width, fm.width( d->strOutText.at( i ) ) );

    d->textWidth  = format()->sheet()->doc()->unzoomItX( width );
    d->textHeight = format()->sheet()->doc()->unzoomItY( ( fm.ascent() + fm.descent() )
            * d->strOutText.length() );
  }
}


// Get the effective font to use after the zooming and apply it to `painter'.
//
// Used in makeLayout() and calculateTextParameters().
//

void Cell::applyZoomedFont( QPainter &painter, int _col, int _row )
{
  QFont  tmpFont( format()->textFont( _col, _row ) );

  // If there is a matching condition on this cell then set the
  // according style parameters.
  if ( d->hasExtra()
       && d->extra()->conditions
       && d->extra()->conditions->matchedStyle() ) {

    Style * s = d->extra()->conditions->matchedStyle();

    // Other size?
    if ( s->hasFeature( Style::SFontSize, true ) )
      tmpFont.setPointSizeFloat( s->fontSize() );

    // Other attributes?
    if ( s->hasFeature( Style::SFontFlag, true ) ) {
      uint flags = s->fontFlags();

      tmpFont.setBold(      flags & (uint) Style::FBold );
      tmpFont.setUnderline( flags & (uint) Style::FUnderline );
      tmpFont.setItalic(    flags & (uint) Style::FItalic );
      tmpFont.setStrikeOut( flags & (uint) Style::FStrike );
    }

    // Other family?
    if ( s->hasFeature( Style::SFontFamily, true ) )
      tmpFont.setFamily( s->fontFamily() );
  }
#if 0
  else
  /*
   * could somebody please explaint why we check for isProtected or isHideFormula here
   */
   if ( d->extra()->conditions
  && d->extra()->conditions->currentCondition( condition )
  && !(format()->sheet()->getShowFormula()
       && !( format()->sheet()->isProtected()
       && format()->isHideFormula( d->column, d->row ) ) ) )
   {
     if ( condition.fontcond )
       tmpFont = *(condition.fontcond);
     else
       tmpFont = condition.style->font();
   }
#endif

  // Scale the font size according to the current zoom.
  tmpFont.setPointSizeFloat( 0.01 * format()->sheet()->doc()->zoom()
           * tmpFont.pointSizeFloat() );

  painter.setFont( tmpFont );
}


//used in Sheet::adjustColumnHelper and Sheet::adjustRow
void Cell::calculateTextParameters( QPainter &_painter,
             int _col, int _row )
{
  // Apply the correct font to _painter.
  applyZoomedFont( _painter, _col, _row );

  // Recalculate d->textWidth and d->textHeight
  textSize( _painter );

  // Recalculate d->textX and d->textY.
  offsetAlign( _col, _row );
}


// ----------------------------------------------------------------
//                          Formula handling


bool Cell::makeFormula()
{
  clearFormula ();

  d->formula = new KSpread::Formula (sheet(), this);
  d->formula->setExpression (d->strText);

  if (!d->formula->isValid ()) {
  // Did a syntax error occur ?
    clearFormula();

    if (format()->sheet()->doc()->getShowMessageError())
    {
      QString tmp(i18n("Error in cell %1\n\n"));
      tmp = tmp.arg( fullName() );
      KMessageBox::error( (QWidget*)0L, tmp);
    }
    setFlag(Flag_ParseError);
    Value v;
    v.setError ( "####" );
    setValue (v);
    return false;
  }

  // we must recalc
  setCalcDirtyFlag ();

  return true;
}

void Cell::clearFormula()
{
  delete d->formula;
  d->formula = 0L;
}

bool Cell::calc(bool delay)
{
  if ( !isFormula() )
    return true;

  if (d->formula == 0)
  {
    if ( testFlag( Flag_ParseError ) )  // there was a parse error
      return false;
    else
    {
      /* we were probably at a "isLoading() = true" state when we originally
       * parsed
       */
      makeFormula ();

      if ( d->formula == 0 ) // there was a parse error
        return false;
    }
  }

  if ( !testFlag( Flag_CalcDirty ) )
    return true;

  if ( delay )
  {
    if ( format()->sheet()->doc()->delayCalculation() )
      return true;
  }

  setFlag(Flag_LayoutDirty);
  setFlag(Flag_TextFormatDirty);
  clearFlag(Flag_CalcDirty);

  Value result = d->formula->eval ();
  setValue (result);
  if (result.isNumber())
    checkNumberFormat(); // auto-chooses number or scientific

  clearFlag(Flag_CalcDirty);
  setFlag(Flag_LayoutDirty);

  return true;
}


// ================================================================
//                            Painting


// Paint the cell.  This is the main function that calls a lot of
//                  helper functions.
//
// `rect'       is the rectangle that we should paint on.  If the cell
//              does not overlap this rectangle, we can return immediately.
// `coordinate' is the origin (the upper left) of the cell in document
//              coordinates.
//

void Cell::paintCell( const KoRect   &rect, QPainter & painter,
          View    *view,
          const KoPoint  &coordinate,
          const QPoint   &cellRef,
          int paintBorder,
          QPen & rightPen, QPen & bottomPen,
          QPen & leftPen,  QPen & topPen,
          QValueList<QPoint> &mergedCellsPainted,
          bool drawCursor )
{
  bool paintBorderRight  = paintBorder & Border_Right;
  bool paintBorderBottom = paintBorder & Border_Bottom;
  bool paintBorderLeft   = paintBorder & Border_Left;
  bool paintBorderTop    = paintBorder & Border_Top;

  // If we are already painting this cell, then return immediately.
  // This avoids infinite recursion.
  if ( testFlag( Flag_PaintingCell ) )
    return;

  // Indicate that we are painting this cell now.
  setFlag( Flag_PaintingCell );

  // This flag indicates that we are working on drawing the cells that
  // another cell is obscuring.  The value is the number of levels down we
  // are currently working -- i.e. a cell obscured by a cell which is
  // obscured by a cell.
  static int  paintingObscured = 0;

#if 0
  if (paintingObscured == 0)
    kdDebug(36001) << "painting cell " << name() << endl;
  else
    kdDebug(36001) << "  painting obscured cell " << name() << endl;
#endif

  // Sanity check: If we're working on drawing an obscured cell, that
  // means this cell should have a cell that obscures it.
  Q_ASSERT(!(paintingObscured > 0 && d->extra()->obscuringCells.isEmpty()));

  // The parameter cellref should be *this, unless this is the default cell.
  Q_ASSERT(isDefault()
     || (((cellRef.x() == d->column) && (cellRef.y() == d->row))));

  Sheet::LayoutDirection sheetDir =  format()->sheet()->layoutDirection();

  double left = coordinate.x();

  ColumnFormat * colFormat = format()->sheet()->columnFormat( cellRef.x() );
  RowFormat    * rowFormat = format()->sheet()->rowFormat( cellRef.y() );

  // Set width, height to the total width and height that this cell
  // covers, including obscured cells, and width0, height0 to the
  // width and height of this cell, maybe merged but never implicitly
  // extended.
  double  width0  = colFormat->dblWidth();
  double  height0 = rowFormat->dblHeight();
  double  width   = width0;
  double  height  = height0;

  // Handle right-to-left layout.
  // In an RTL sheet the cells have to be painted at their opposite horizontal
  // location on the canvas, meaning that column A will be the rightmost column
  // on screen, column B will be to the left of it and so on. Here we change
  // the horizontal coordinate at which we start painting the cell in case the
  // sheet's direction is RTL. We do this only if paintingObscured is 0,
  // otherwise the cell's painting location will flip back and forth in
  // consecutive calls to paintCell when painting obscured cells.
  if ( sheetDir == Sheet::RightToLeft && paintingObscured == 0
       && view && view->canvasWidget() )
  {
    double  dwidth = view->doc()->unzoomItX(view->canvasWidget()->width());
    left = dwidth - coordinate.x() - width;
  }

  // See if this cell is merged or has overflown into neighbor cells.
  // In that case, the width/height is greater than just the cell
  // itself.
  if (d->hasExtra()) {
    if (d->extra()->mergedXCells > 0 || d->extra()->mergedYCells > 0) {
      // merged cell extends to the left if sheet is RTL
      if ( sheetDir == Sheet::RightToLeft ) {
        left -= d->extra()->extraWidth - width;
      }
      width0  = d->extra()->extraWidth;
      height0 = d->extra()->extraHeight;
      width   = d->extra()->extraWidth;
      height  = d->extra()->extraHeight;
    }
    else {
#if 0
      width  += d->extra()->extraXCells ? d->extra()->extraWidth  : 0;
      height += d->extra()->extraYCells ? d->extra()->extraHeight : 0;
#else
      // FIXME: Make extraWidth/Height really contain the *extra* width/height.
      if ( d->extra()->extraXCells )
  width  = d->extra()->extraWidth;
      if ( d->extra()->extraYCells )
  height = d->extra()->extraHeight;
#endif
    }
  }

  // Check if the cell is "selected", i.e. it should be drawn with the
  // color that indicates selection (dark blue).  If more than one
  // square is selected, the last one uses the ordinary colors.  In
  // that case, "selected" will be set to false even though the cell
  // itself really is selected.
  bool  selected = false;
  if ( view != NULL ) {
    selected = view->selectionInfo()->contains( cellRef );

    // But the cell doesn't look selected if this is the marker cell.
    Cell  *cell = format()->sheet()->cellAt( view->selectionInfo()->marker() );
    QPoint bottomRight( view->selectionInfo()->marker().x() + cell->extraXCells(),
       view->selectionInfo()->marker().y() + cell->extraYCells() );
    QRect markerArea( view->selectionInfo()->marker(), bottomRight );
    selected = selected && !( markerArea.contains( cellRef ) );

    // Don't draw any selection at all when printing.
    if ( painter.device()->isExtDev() || !drawCursor )
      selected = false;
  }

  // Need to make a new layout ?
  //
  // FIXME: We have already used (at least) extraWidth/Height above,
  //        and now we are recalculating the layout.  This has to be
  //        moved up above all uses.
  //
  // FIXME: This needs to be taken out eventually - it is done in
  //        canvas::paintUpdates().
  if ( testFlag( Flag_LayoutDirty ) )
    makeLayout( painter, cellRef.x(), cellRef.y() );

  // ----------------  Start the actual painting.  ----------------

  // If the rect of this cell doesn't intersect the rect that should
  // be painted, we can skip the rest and return. (Note that we need
  // to calculate `left' first before we can do this.)
  const KoRect  cellRect( left, coordinate.y(), width, height );
  const KoRect  cellRect0( left, coordinate.y(), width0, height0 );
  if ( !cellRect.intersects( rect ) ) {
    clearFlag( Flag_PaintingCell );
    return;
  }

  // Get the background color.
  //
  // If there is a condition giving the background color for this cell
  // (and it matches), use that one, otherwise get the standard
  // background.
  QColor backgroundColor;
  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SBackgroundColor, true ) )
    backgroundColor = d->extra()->conditions->matchedStyle()->bgColor();
  else
    backgroundColor = bgColor( cellRef.x(), cellRef.y() );

  // 1. Paint the background.
  if ( !isPartOfMerged() )
    paintBackground( painter, cellRect0, cellRef, selected, backgroundColor );

  // 2. Paint the default borders if we are on screen or if we are printing
  //    and the checkbox to do this is checked.
  if ( painter.device()->devType() != QInternal::Printer
       || format()->sheet()->print()->printGrid())
    paintDefaultBorders( painter, rect, cellRect, cellRef,
       paintBorderRight, paintBorderBottom,
                         paintBorderLeft,  paintBorderTop,
       rightPen, bottomPen, leftPen, topPen );

  // 3. Paint all the cells that this one obscures.  They may only be
  //    partially obscured.
  //
  // The `paintingObscured' variable is used to avoid infinite
  // recursion since cells sometimes paint their obscuring cell as
  // well.
  paintingObscured++;

  if (d->hasExtra() && (d->extra()->extraXCells > 0
      || d->extra()->extraYCells > 0)) {
    //kdDebug(36001) << "painting obscured cells for " << name() << endl;

    paintObscuredCells( rect, painter, view, cellRect, cellRef,
      paintBorderRight, paintBorderBottom,
      paintBorderLeft,  paintBorderTop,
      rightPen, bottomPen, leftPen, topPen,
      mergedCellsPainted);

    // FIXME: Is this the right place for this?
    if ( d->extra()->mergedXCells > 0 || d->extra()->mergedYCells > 0 )
      mergedCellsPainted.prepend( cellRef );
  }
  paintingObscured--;

  // 4. Paint the borders of the cell if no other cell is forcing this
  // one, i.e. this cell is not part of a merged cell.
  //

  // If we print pages, then we disable clipping, otherwise borders are
  // cut in the middle at the page borders.
  if ( painter.device()->isExtDev() )
    painter.setClipping( false );

  // Paint the borders if this cell is not part of another merged cell.
  if ( !isPartOfMerged() ) {
    // if (!testFlag(Flag_Highlight))
    paintCellBorders( painter, rect, cellRect0,
          cellRef,
          paintBorderRight, paintBorderBottom,
                      paintBorderLeft,  paintBorderTop,
          rightPen, bottomPen, leftPen, topPen );
  }

  // Turn clipping back on.
  if ( painter.device()->isExtDev() )
    painter.setClipping( true );

  // 5. Paint diagonal lines and page borders.
  paintCellDiagonalLines( painter, cellRect0, cellRef );

  paintPageBorders( painter, cellRect0, cellRef,
        paintBorderRight, paintBorderBottom );


  // 6. Now paint the content, if this cell isn't obscured.
  if ( !isObscured() ) {

    // 6a. Paint possible comment indicator.
    if ( !painter.device()->isExtDev()
   || format()->sheet()->print()->printCommentIndicator() )
      paintCommentIndicator( painter, cellRect, cellRef, backgroundColor );

    // 6b. Paint possible formula indicator.
    if ( !painter.device()->isExtDev()
   || format()->sheet()->print()->printFormulaIndicator() )
      paintFormulaIndicator( painter, cellRect, backgroundColor );

    // 6c. Paint possible indicator for clipped text.
    paintMoreTextIndicator( painter, cellRect, backgroundColor );

     //6c. Paint cell highlight
#if 0
    if (highlightBorder != Border_None)
      paintCellHighlight ( painter, cellRect, cellRef, highlightBorder,
         rightHighlightPen, bottomHighlightPen,
         leftHighlightPen,  topHighlightPen );
#endif

    // 6d. Paint the text in the cell unless:
    //  a) it is empty
    //  b) something indicates that the text should not be painted
    //  c) the sheet is protected and the cell is hidden.
    if ( !d->strOutText.isEmpty()
   && ( !painter.device()->isExtDev()
        || !format()->getDontprintText( cellRef.x(), cellRef.y() ) )
   && !( format()->sheet()->isProtected()
         && format()->isHideAll( cellRef.x(), cellRef.y() ) ) )
    {
      paintText( painter, cellRect, cellRef );
    }
  }

  // 7. If this cell is obscured and we are not already painting obscured
  //    cells, then paint the obscuring cell(s).  Otherwise don't do
  //    anything so that we don't cause an infinite loop.
  if ( isObscured() && paintingObscured == 0 &&
       !( sheetDir == Sheet::RightToLeft && painter.device()->isExtDev() ) )
  {

    //kdDebug(36001) << "painting cells that obscure " << name() << endl;

    // Store the obscuringCells list in a list of QPoint(column, row)
    // This avoids crashes during the iteration through
    // obscuringCells, when the cells may get non valid or the list
    // itself gets changed during a call of obscuringCell->paintCell
    // (this happens e.g. when there is an updateDepend)
    if (d->hasExtra()) {
      QValueList<QPoint>           listPoints;
      QValueList<Cell*>::iterator  it = d->extra()->obscuringCells.begin();
      QValueList<Cell*>::iterator  end = d->extra()->obscuringCells.end();
      for ( ; it != end; ++it ) {
        Cell *obscuringCell = *it;

        listPoints.append( QPoint( obscuringCell->column(), obscuringCell->row() ) );
      }

      QValueList<QPoint>::iterator  it1  = listPoints.begin();
      QValueList<QPoint>::iterator  end1 = listPoints.end();
      for ( ; it1 != end1; ++it1 ) {
        QPoint obscuringCellRef = *it1;

  // Only paint those obscuring cells that haven't been already
  // painted yet.
  //
  // This optimization removes an O(n^4) behaviour where n is
  // the number of cells on one edge in a merged cell.
  if ( mergedCellsPainted.contains( obscuringCellRef ) )
    continue;

        Cell *obscuringCell = format()->sheet()->cellAt( obscuringCellRef.x(),
               obscuringCellRef.y() );

        if ( obscuringCell != 0 ) {
          double x = format()->sheet()->dblColumnPos( obscuringCellRef.x() );
          double y = format()->sheet()->dblRowPos( obscuringCellRef.y() );
          if ( view != 0 ) {
            x -= view->canvasWidget()->xOffset();
            y -= view->canvasWidget()->yOffset();
          }

          KoPoint corner( x, y );
          painter.save();

    // Get the effective pens for the borders.  These are
    // determined by possible conditions on the cell with
    // associated styles.
          QPen rp( obscuringCell->effRightBorderPen( obscuringCellRef.x(),
                 obscuringCellRef.y() ) );
          QPen bp( obscuringCell->effBottomBorderPen( obscuringCellRef.x(),
                  obscuringCellRef.y() ) );
          QPen lp( obscuringCell->effLeftBorderPen( obscuringCellRef.x(),
                obscuringCellRef.y() ) );
          QPen tp( obscuringCell->effTopBorderPen( obscuringCellRef.x(),
               obscuringCellRef.y() ) );


    //kdDebug(36001) << "  painting obscuring cell "
    //     << obscuringCell->name() << endl;
    // QPen highlightPen;

    //Note: Painting of highlight isn't quite right.  If several
    //      cells are merged, then the whole merged cell will be
    //      painted with the colour of the last cell referenced
    //      which is inside the merged range.
          obscuringCell->paintCell( rect, painter, view,
                                    corner, obscuringCellRef,
            Border_Left|Border_Top|Border_Right|Border_Bottom,
            rp, bp, lp, tp,
            mergedCellsPainted); // new pens
          painter.restore();
        }
      }
    }
  }

  // We are done with the painting, so remove the flag on the cell.
  clearFlag( Flag_PaintingCell );
}



// The following code was commented out in the above function.  I'll
// leave it here in case this functionality is ever re-implemented and
// someone wants some code to start from.
//
#if 0

  /**
     * Modification for drawing the button
     */
  if ( d->style == Cell::ST_Button ) {
    QBrush fill( Qt::lightGray );
    QApplication::style().drawControl( QStyle::CE_PushButton, &_painter, this,
               QRect( _tx + 1, _ty + 1, w2 - 1, h2 - 1 ),
               defaultColorGroup ); //, selected, &fill );
  }

    /**
     * Modification for drawing the combo box
     */
  else if ( d->style == Cell::ST_Select ) {
    QApplication::style().drawComboButton(  &_painter, _tx + 1, _ty + 1,
              w2 - 1, h2 - 1,
              defaultColorGroup, selected );
  }
#endif


#if 0
 void Cell::paintCellHighlight(QPainter& painter,
             const KoRect& cellRect,
             const QPoint& cellRef,
             const int highlightBorder,
             const QPen& rightPen,
             const QPen& bottomPen,
             const QPen& leftPen,
             const QPen& topPen
             )
{
  //painter.drawLine(cellRect.left(),cellRect.top(),cellRect.right(),cellRect.bottom());
  //QPen pen(d->extra()->highlight);
  //painter.setPen(highlightPen);

  QBrush nullBrush;
  painter.setBrush(nullBrush);

  QRect zoomedCellRect = sheet()->doc()->zoomRect( cellRect );

  //The highlight rect is just inside the main cell rect
  //This saves the hassle of repainting nearby cells when the highlight is changed as the highlight areas
  //do not overlap
  zoomedCellRect.setLeft(zoomedCellRect.left()+1);
  //zoomedCellRect.setRight(zoomedCellRect.right()-1);
  zoomedCellRect.setTop(zoomedCellRect.top()+1);
  //zoomedCellRect.setBottom(zoomedCellRect.bottom()-1);

  if ( cellRef.x() != KS_colMax )
    zoomedCellRect.setWidth( zoomedCellRect.width() - 1 );
  if ( cellRef.y() != KS_rowMax )
  zoomedCellRect.setHeight( zoomedCellRect.height() - 1 );

  if (highlightBorder & Border_Top)
  {
    painter.setPen(topPen);
    painter.drawLine(zoomedCellRect.left(),zoomedCellRect.top(),zoomedCellRect.right(),zoomedCellRect.top());
  }
  if (highlightBorder & Border_Left)
  {
    painter.setPen(leftPen);
    painter.drawLine(zoomedCellRect.left(),zoomedCellRect.top(),zoomedCellRect.left(),zoomedCellRect.bottom());
  }
  if (highlightBorder & Border_Right)
  {
    painter.setPen(rightPen);
    painter.drawLine(zoomedCellRect.right(),zoomedCellRect.top(),zoomedCellRect.right(),zoomedCellRect.bottom());
  }
  if (highlightBorder & Border_Bottom)
  {
    painter.setPen(bottomPen);
    painter.drawLine(zoomedCellRect.left(),zoomedCellRect.bottom(),zoomedCellRect.right(),zoomedCellRect.bottom());
  }

  if (highlightBorder & Border_SizeGrip)
  {
    QBrush brush(rightPen.color());
    painter.setBrush(brush);
    painter.setPen(rightPen);
    painter.drawRect(zoomedCellRect.right()-3,zoomedCellRect.bottom()-3,4,4);
  }

  //painter.drawRect(zoomedCellRect.left(),zoomedCellRect.top(),zoomedCellRect.width(),zoomedCellRect.height());
}
#endif


// Paint all the cells that this cell obscures (helper function to paintCell).
//
void Cell::paintObscuredCells(const KoRect& rect, QPainter& painter,
            View* view,
            const KoRect &cellRect,
            const QPoint &cellRef,
            bool paintBorderRight,
            bool _paintBorderBottom,
            bool paintBorderLeft,
            bool _paintBorderTop,
            QPen & rightPen, QPen & _bottomPen,
            QPen & leftPen,  QPen & _topPen,
            QValueList<QPoint> &mergedCellsPainted)
{
  // If there are no obscured cells, return.
  if ( !extraXCells() && !extraYCells() )
    return;

  double  ypos = cellRect.y();
  int     maxY = extraYCells();
  int     maxX = extraXCells();

  // Loop through the rectangle of squares that we obscure and paint them.
  for ( int y = 0; y <= maxY; ++y ) {
    double xpos = cellRect.x();
    RowFormat* rl = format()->sheet()->rowFormat( cellRef.y() + y );

    for( int x = 0; x <= maxX; ++ x ) {
      ColumnFormat * cl = format()->sheet()->columnFormat( cellRef.x() + x );
      if ( y != 0 || x != 0 ) {
  uint  column = cellRef.x() + x;
  uint  row    = cellRef.y() + y;

  QPen  topPen;
  QPen  bottomPen;
  bool  paintBorderTop;
  bool  paintBorderBottom;

  Cell  *cell = format()->sheet()->cellAt( column, row );
  KoPoint       corner( xpos, ypos );

  // Check if the upper and lower borders should be painted, and
  // if so which pens we should use.  There used to be a nasty
  // bug here (#61452).
  // Check top pen.  Only check if this is not on the top row.
  topPen         = _topPen;
  paintBorderTop = _paintBorderTop;
  if ( row > 1 && !cell->isPartOfMerged() ) {
    Cell  *cellUp = format()->sheet()->cellAt( column, row - 1 );

    if ( cellUp->isDefault() )
      paintBorderTop = false;
    else {
      // If the cell towards the top is part of a merged cell, get
      // the pointer to the master cell.
      cellUp = cellUp->ultimateObscuringCell();

      topPen = cellUp->effBottomBorderPen( cellUp->column(),
             cellUp->row() );

#if 0
      int  penWidth = QMAX(1, sheet()->doc()->zoomItY( topPen.width() ));
      topPen.setWidth( penWidth );
#endif
    }
  }

  // FIXME: I thought we had to check bottom pen as well.
  //        However, it looks as if we don't need to.  It works anyway.
  bottomPen         = _bottomPen;
  paintBorderBottom = _paintBorderBottom;

  int  paintBorder = Border_None;
  if (paintBorderLeft)   paintBorder |= Cell::Border_Left;
  if (paintBorderRight)  paintBorder |= Cell::Border_Right;
  if (paintBorderTop)    paintBorder |= Cell::Border_Top;
  if (paintBorderBottom) paintBorder |= Cell::Border_Bottom;

  /*Cell::BorderSides highlightBorder = Border_None;
    QPen highlightPen;*/


  //kdDebug(36001) << "calling paintcell for obscured cell "
  //       << cell->name() << endl;
  cell->paintCell( rect, painter, view,
       corner,
       QPoint( cellRef.x() + x, cellRef.y() + y ),
       paintBorder,
       rightPen, bottomPen, leftPen, topPen,
       mergedCellsPainted);
      }
      xpos += cl->dblWidth();
    }

    ypos += rl->dblHeight();
  }
}


// Paint the background of this cell.
//
void Cell::paintBackground( QPainter& painter, const KoRect &cellRect,
          const QPoint &cellRef, bool selected,
          QColor &backgroundColor )
{
  QColorGroup  defaultColorGroup = QApplication::palette().active();
  QRect        zoomedCellRect    = sheet()->doc()->zoomRect( cellRect );

  // If this is not the KS_rowMax and/or KS_colMax, then we reduce
  // width and/or height by one.  This is due to the fact that the
  // right/bottom most pixel is shared with the left/top most pixel of
  // the following cell.  Only in the case of KS_colMax/KS_rowMax we
  // need to draw even this pixel, as there isn't a following cell to
  // draw the background pixel.
   if ( cellRef.x() != KS_colMax )
     zoomedCellRect.setWidth( zoomedCellRect.width() - 1 );
   if ( cellRef.y() != KS_rowMax )
     zoomedCellRect.setHeight( zoomedCellRect.height() - 1 );

  // Determine the correct background color
  if ( selected )
  {
    //If the cell's background color is too bright, use the default highlight color
    //Otherwise use a lighter version of the cell's background color.
    QColor c;

    int averageColor = (backgroundColor.red() + backgroundColor.green() + backgroundColor.blue()) / 3;

    if (averageColor > 180)
	if (averageColor > 225)
       		c = View::highlightColor();
    	else
		c = backgroundColor.light( 115 ); //15% lighter
    else
       c = backgroundColor.light( 125 ); //25% lighter

    painter.setBackgroundColor( c );
  }
  else {
    QColor bg( backgroundColor );

    // Handle printers separately.
    if ( !painter.device()->isExtDev() ) {
      if ( bg.isValid() )
        painter.setBackgroundColor( bg );
      else
        painter.setBackgroundColor( defaultColorGroup.base() );
    }
    else {
      //bad hack but there is a qt bug
      //so I can print backgroundcolor
      QBrush bb( bg );
      if ( !bg.isValid() )
        bb.setColor( Qt::white );

      painter.fillRect( zoomedCellRect, bb );
      return;
    }
  }

  // Erase the background of the cell.
  if ( !painter.device()->isExtDev() )
    painter.eraseRect( zoomedCellRect );

  // Get a background brush
  QBrush bb;
  if ( d->hasExtra()
       && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SBackgroundBrush, true ) )
    bb = d->extra()->conditions->matchedStyle()->backGroundBrush();
  else
    bb = backGroundBrush( cellRef.x(), cellRef.y() );

  // Draw background pattern if necessary.
  if ( bb.style() != Qt::NoBrush )
    painter.fillRect( zoomedCellRect, bb );

  backgroundColor = painter.backgroundColor();
}


// Paint the standard light grey borders that are always visible.
//
void Cell::paintDefaultBorders( QPainter& painter, const KoRect &rect,
        const KoRect &cellRect,
        const QPoint &cellRef,
        bool paintBorderRight, bool /*paintBorderBottom*/,
        bool paintBorderLeft,  bool paintBorderTop,
        QPen const & rightPen, QPen const & /*bottomPen*/,
        QPen const & leftPen, QPen const & topPen )
{
    /*
        *** Notes about optimisation ***

        This function was painting the top , left , right & bottom lines in almost all cells previously, contrary to what the comment
        below says should happen.  There doesn't appear to be a UI option to enable or disable showing of the grid when printing at the moment,
        so I have disabled drawing of right and bottom borders for all cells.

        I also couldn't work out under what conditions the variables dt / db would come out as anything other than 0 in the code
        for painting the various borders.  The effTopBorderPen / effBottomBorderPen calls were taking up a lot of time
        according some profiling I did.  If that code really is necessary, we need to find a more efficient way of getting the widths
        than grabbing the whole QPen object and asking it.


        --Robert Knight (robertknight@gmail.com)
    */
  Doc* doc = sheet()->doc();

  Sheet::LayoutDirection sheetDir =  format()->sheet()->layoutDirection();
  bool paintingToExternalDevice = painter.device()->isExtDev();

  // Each cell is responsible for drawing it's top and left portions
  // of the "default" grid. --Or not drawing it if it shouldn't be
  // there.  It's also responsible to paint the right and bottom, if
  // it is the last cell on a print out.

  bool paintTop;
  bool paintLeft;
  bool paintBottom=false;
  bool paintRight=false;

  paintLeft   = ( paintBorderLeft && leftPen.style() == Qt::NoPen
      && sheet()->getShowGrid() && sheetDir==Sheet::LeftToRight );
  paintRight  = ( paintBorderRight && rightPen.style() == Qt::NoPen
      && sheet()->getShowGrid() && sheetDir==Sheet::RightToLeft );
  paintTop    = ( paintBorderTop && topPen.style() == Qt::NoPen
      && sheet()->getShowGrid() );
//  paintBottom = ( paintBorderBottom && sheet()->getShowGrid()
//                  && bottomPen.style() == Qt::NoPen );


   //Set the single-pixel with pen for drawing the borders with.
   painter.setPen( QPen( sheet()->doc()->gridColor(), 1, Qt::SolidLine ) );

  // If there are extra cells, there might be more conditions.
  if (d->hasExtra()) {
    QValueList<Cell*>::const_iterator it  = d->extra()->obscuringCells.begin();
    QValueList<Cell*>::const_iterator end = d->extra()->obscuringCells.end();
    for ( ; it != end; ++it ) {
      Cell *cell = *it;

      paintTop  = paintTop && ( cell->row() == cellRef.y() );
      paintBottom = false;

      if ( sheetDir == Sheet::RightToLeft ) {
        paintRight = paintRight && ( cell->column() == cellRef.x() );
        paintLeft = false;
      }
      else {
        paintLeft = paintLeft && ( cell->column() == cellRef.x() );
        paintRight = false;
      }
    }
  }

  // The left border.
  if ( paintLeft ) {
    int dt = 0;
    int db = 0;

    #if 0
    if ( cellRef.x() > 1 ) {
      Cell  *cell_west = format()->sheet()->cellAt( cellRef.x() - 1,
                cellRef.y() );
      QPen t = cell_west->effTopBorderPen( cellRef.x() - 1, cellRef.y() );
      QPen b = cell_west->effBottomBorderPen( cellRef.x() - 1, cellRef.y() );

      if ( t.style() != Qt::NoPen )
        dt = ( t.width() + 1 )/2;
      if ( b.style() != Qt::NoPen )
        db = ( t.width() / 2);
    }
    #endif

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( paintingToExternalDevice ) {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( doc->zoomItX( QMAX( rect.left(),   cellRect.right() ) ),
                          doc->zoomItY( QMAX( rect.top(),    cellRect.y() + dt ) ),
                          doc->zoomItX( QMIN( rect.right(),  cellRect.right() ) ),
                          doc->zoomItY( QMIN( rect.bottom(), cellRect.bottom() - db ) ) );
      else
        painter.drawLine( doc->zoomItX( QMAX( rect.left(),   cellRect.x() ) ),
                          doc->zoomItY( QMAX( rect.top(),    cellRect.y() + dt ) ),
                          doc->zoomItX( QMIN( rect.right(),  cellRect.x() ) ),
                          doc->zoomItY( QMIN( rect.bottom(), cellRect.bottom() - db ) ) );
    }
    else {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( doc->zoomItX( cellRect.right() ),
                          doc->zoomItY( cellRect.y() + dt ),
                          doc->zoomItX( cellRect.right() ),
                          doc->zoomItY( cellRect.bottom() - db ) );
      else
        painter.drawLine( doc->zoomItX( cellRect.x() ),
                          doc->zoomItY( cellRect.y() + dt ),
                          doc->zoomItX( cellRect.x() ),
                          doc->zoomItY( cellRect.bottom() - db ) );
    }
  }


  // The top border.
  if ( paintTop ) {
    int dl = 0;
    int dr = 0;

    #if 0
    if ( cellRef.y() > 1 ) {
      Cell  *cell_north = format()->sheet()->cellAt( cellRef.x(),
                 cellRef.y() - 1 );

      QPen l = cell_north->effLeftBorderPen(  cellRef.x(), cellRef.y() - 1 );
      QPen r = cell_north->effRightBorderPen( cellRef.x(), cellRef.y() - 1 );

      if ( l.style() != Qt::NoPen )
        dl = ( l.width() - 1 ) / 2 + 1;
      if ( r.style() != Qt::NoPen )
        dr = r.width() / 2;
    }
    #endif



    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( paintingToExternalDevice ) {
      painter.drawLine( doc->zoomItX( QMAX( rect.left(),   cellRect.x() + dl ) ),
                        doc->zoomItY( QMAX( rect.top(),    cellRect.y() ) ),
                        doc->zoomItX( QMIN( rect.right(),  cellRect.right() - dr ) ),
                        doc->zoomItY( QMIN( rect.bottom(), cellRect.y() ) ) );
    }
    else {
      painter.drawLine( doc->zoomItX( cellRect.x() + dl ),
                        doc->zoomItY( cellRect.y() ),
                        doc->zoomItX( cellRect.right() - dr ),
                        doc->zoomItY( cellRect.y() ) );
    }
  }


  // The right border.
  if ( paintRight ) {
    int dt = 0;
    int db = 0;

    #if 0
    if ( cellRef.x() < KS_colMax ) {
      Cell  *cell_east = format()->sheet()->cellAt( cellRef.x() + 1,
                cellRef.y() );

      QPen t = cell_east->effTopBorderPen(    cellRef.x() + 1, cellRef.y() );
      QPen b = cell_east->effBottomBorderPen( cellRef.x() + 1, cellRef.y() );

      if ( t.style() != Qt::NoPen )
        dt = ( t.width() + 1 ) / 2;
      if ( b.style() != Qt::NoPen )
        db = ( t.width() / 2);
    }
    #endif

    //painter.setPen( QPen( sheet()->doc()->gridColor(), 1, Qt::SolidLine ) );

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() )     {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( doc->zoomItX( QMAX( rect.left(),   cellRect.x() ) ),
                          doc->zoomItY( QMAX( rect.top(),    cellRect.y() + dt ) ),
                          doc->zoomItX( QMIN( rect.right(),  cellRect.x() ) ),
                          doc->zoomItY( QMIN( rect.bottom(), cellRect.bottom() - db ) ) );
      else
        painter.drawLine( doc->zoomItX( QMAX( rect.left(),   cellRect.right() ) ),
                          doc->zoomItY( QMAX( rect.top(),    cellRect.y() + dt ) ),
                          doc->zoomItX( QMIN( rect.right(),  cellRect.right() ) ),
                          doc->zoomItY( QMIN( rect.bottom(), cellRect.bottom() - db ) ) );
    }
    else {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( doc->zoomItX( cellRect.x() ),
                          doc->zoomItY( cellRect.y() + dt ),
                          doc->zoomItX( cellRect.x() ),
                          doc->zoomItY( cellRect.bottom() - db ) );
      else
        painter.drawLine( doc->zoomItX( cellRect.right() ),
                          doc->zoomItY( cellRect.y() + dt ),
                          doc->zoomItX( cellRect.right() ),
                          doc->zoomItY( cellRect.bottom() - db ) );
    }
  }

  // The bottom border.
  /*if ( paintBottom ) {
    int dl = 0;
    int dr = 0;
    if ( cellRef.y() < KS_rowMax ) {
      Cell  *cell_south = format()->sheet()->cellAt( cellRef.x(),
                 cellRef.y() + 1 );

      QPen l = cell_south->effLeftBorderPen(  cellRef.x(), cellRef.y() + 1 );
      QPen r = cell_south->effRightBorderPen( cellRef.x(), cellRef.y() + 1 );

      if ( l.style() != Qt::NoPen )
        dl = ( l.width() - 1 ) / 2 + 1;
      if ( r.style() != Qt::NoPen )
        dr = r.width() / 2;
    }

    painter.setPen( QPen( sheet()->doc()->gridColor(), 1, Qt::SolidLine ) );

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() ) {
      painter.drawLine( doc->zoomItX( QMAX( rect.left(),   cellRect.x() + dl ) ),
                        doc->zoomItY( QMAX( rect.top(),    cellRect.bottom() ) ),
                        doc->zoomItX( QMIN( rect.right(),  cellRect.right() - dr ) ),
                        doc->zoomItY( QMIN( rect.bottom(), cellRect.bottom() ) ) );
    }
    else {
      painter.drawLine( doc->zoomItX( cellRect.x() + dl ),
                        doc->zoomItY( cellRect.bottom() ),
                        doc->zoomItX( cellRect.right() - dr ),
                        doc->zoomItY( cellRect.bottom() ) );
    }
  }*/
}


// Paint a comment indicator if the cell has a comment.
//
void Cell::paintCommentIndicator( QPainter& painter,
          const KoRect &cellRect,
          const QPoint &/*cellRef*/,
          QColor &backgroundColor )
{
  Doc * doc = sheet()->doc();

  // Point the little corner if there is a comment attached
  // to this cell.
  if ( ( format()->propertiesMask() & (uint) Format::PComment )
       && cellRect.width() > 10.0
       && cellRect.height() > 10.0
       && ( sheet()->print()->printCommentIndicator()
           || ( !painter.device()->isExtDev() && sheet()->getShowCommentIndicator() ) ) ) {
    QColor penColor = Qt::red;

    // If background has high red part, switch to blue.
    if ( qRed( backgroundColor.rgb() ) > 127 &&
         qGreen( backgroundColor.rgb() ) < 80 &&
         qBlue( backgroundColor.rgb() ) < 80 )
    {
        penColor = Qt::blue;
    }

    // Get the triangle.
    QPointArray  point( 3 );
    if ( format()->sheet()->layoutDirection()==Sheet::RightToLeft ) {
      point.setPoint( 0, doc->zoomItX( cellRect.x() + 6.0 ),
                         doc->zoomItY( cellRect.y() ) );
      point.setPoint( 1, doc->zoomItX( cellRect.x() ),
                         doc->zoomItY( cellRect.y() ) );
      point.setPoint( 2, doc->zoomItX( cellRect.x() ),
                         doc->zoomItY( cellRect.y() + 6.0 ) );
    }
    else {
      point.setPoint( 0, doc->zoomItX( cellRect.right() - 5.0 ),
                         doc->zoomItY( cellRect.y() ) );
      point.setPoint( 1, doc->zoomItX( cellRect.right() ),
                         doc->zoomItY( cellRect.y() ) );
      point.setPoint( 2, doc->zoomItX( cellRect.right() ),
                         doc->zoomItY( cellRect.y() + 5.0 ) );
    }

    // And draw it.
    painter.setBrush( QBrush( penColor ) );
    painter.setPen( Qt::NoPen );
    painter.drawPolygon( point );
  }
}



// Paint a small rectangle if this cell holds a formula.
//
void Cell::paintFormulaIndicator( QPainter& painter,
          const KoRect &cellRect,
          QColor &backgroundColor )
{
  if ( isFormula() &&
      format()->sheet()->getShowFormulaIndicator() &&
      cellRect.width()  > 10.0 &&
      cellRect.height() > 10.0 )
  {
    Doc* doc = sheet()->doc();

    QColor penColor = Qt::blue;
    // If background has high blue part, switch to red.
    if ( qRed( backgroundColor.rgb() ) < 80 &&
        qGreen( backgroundColor.rgb() ) < 80 &&
        qBlue( backgroundColor.rgb() ) > 127 )
    {
        penColor = Qt::red;
    }

    // Get the triangle...
    QPointArray point( 3 );
    if ( format()->sheet()->layoutDirection()==Sheet::RightToLeft ) {
      point.setPoint( 0, doc->zoomItX( cellRect.right() - 6.0 ),
                         doc->zoomItY( cellRect.bottom() ) );
      point.setPoint( 1, doc->zoomItX( cellRect.right() ),
                         doc->zoomItY( cellRect.bottom() ) );
      point.setPoint( 2, doc->zoomItX( cellRect.right() ),
                         doc->zoomItY( cellRect.bottom() - 6.0 ) );
    }
    else {
      point.setPoint( 0, doc->zoomItX( cellRect.x() ),
                         doc->zoomItY( cellRect.bottom() - 6.0 ) );
      point.setPoint( 1, doc->zoomItX( cellRect.x() ),
                         doc->zoomItY( cellRect.bottom() ) );
      point.setPoint( 2, doc->zoomItX( cellRect.x() + 6.0 ),
                         doc->zoomItY( cellRect.bottom() ) );
    }

    // ...and draw it.
    painter.setBrush( QBrush( penColor ) );
    painter.setPen( Qt::NoPen );
    painter.drawPolygon( point );
  }
}


// Paint an indicator that the text in the cell is cut.
//
void Cell::paintMoreTextIndicator( QPainter& painter,
           const KoRect &cellRect,
           QColor &backgroundColor )
{
  // Show a red triangle when it's not possible to write all text in cell.
  // Don't print the red triangle if we're printing.
  if( testFlag( Flag_CellTooShortX ) &&
      !painter.device()->isExtDev() &&
      cellRect.height() > 4.0  &&
      cellRect.width()  > 4.0 )
  {
    Doc* doc = sheet()->doc();

    QColor penColor = Qt::red;
    // If background has high red part, switch to blue.
    if ( qRed( backgroundColor.rgb() ) > 127
   && qGreen( backgroundColor.rgb() ) < 80
   && qBlue( backgroundColor.rgb() ) < 80 )
    {
      penColor = Qt::blue;
    }

    // Get the triangle...
    QPointArray point( 3 );
    if ( d->strOutText.isRightToLeft() ) {
      point.setPoint( 0, doc->zoomItX( cellRect.left() + 4.0 ),
                         doc->zoomItY( cellRect.y() + cellRect.height() / 2.0 -4.0 ) );
      point.setPoint( 1, doc->zoomItX( cellRect.left() ),
                         doc->zoomItY( cellRect.y() + cellRect.height() / 2.0 ));
      point.setPoint( 2, doc->zoomItX( cellRect.left() + 4.0 ),
                         doc->zoomItY( cellRect.y() + cellRect.height() / 2.0 +4.0 ) );
    }
    else {
      point.setPoint( 0, doc->zoomItX( cellRect.right() - 4.0 ),
                         doc->zoomItY( cellRect.y() + cellRect.height() / 2.0 - 4.0 ) );
      point.setPoint( 1, doc->zoomItX( cellRect.right() ),
                         doc->zoomItY( cellRect.y() + cellRect.height() / 2.0 ) );
      point.setPoint( 2, doc->zoomItX( cellRect.right() - 4.0 ),
                         doc->zoomItY( cellRect.y() + cellRect.height() / 2.0 + 4.0 ) );
    }

    // ...and paint it.
    painter.setBrush( QBrush( penColor ) );
    painter.setPen( Qt::NoPen );
    painter.drawPolygon( point );
  }
}


// Paint the real contents of a cell - the text.
//
void Cell::paintText( QPainter& painter,
          const KoRect &cellRect,
          const QPoint &cellRef )
{
  Doc    *doc = sheet()->doc();

  ColumnFormat  *colFormat         = format()->sheet()->columnFormat( cellRef.x() );

  QColorGroup    defaultColorGroup = QApplication::palette().active();
  QColor         textColorPrint    = effTextColor( cellRef.x(), cellRef.y() );

  // Resolve the text color if invalid (=default).
  if ( !textColorPrint.isValid() ) {
    if ( painter.device()->isExtDev() )
      textColorPrint = Qt::black;
    else
      textColorPrint = QApplication::palette().active().text();
  }

  QPen tmpPen( textColorPrint );

  // Set the font according to the current zoom.
  applyZoomedFont( painter, cellRef.x(), cellRef.y() );

  // Check for red font color for negative values.
  if ( !d->hasExtra()
       || !d->extra()->conditions
       || !d->extra()->conditions->matchedStyle() ) {
    if ( value().isNumber()
         && !( format()->sheet()->getShowFormula()
               && !( format()->sheet()->isProtected()
         && format()->isHideFormula( d->column, d->row ) ) ) )
    {
      double v = value().asFloat();
      if ( format()->floatColor( cellRef.x(), cellRef.y()) == Format::NegRed
     && v < 0.0 )
        tmpPen.setColor( Qt::red );
    }
  }

  // Check for blue color, for hyperlink.
  if ( !link().isEmpty() ) {
    tmpPen.setColor( QApplication::palette().active().link() );
    QFont f = painter.font();
    f.setUnderline( true );
    painter.setFont( f );
  }

#if 0
/****

 For now I am commenting this out -- with the default color display you
 can read normal text through a highlighted background.  Maybe this isn't
 always the case, though, and we can put the highlighted text color back in.
 In that case, we need to somewhere in here figure out if the text overlaps
 another cell outside of the selection, otherwise that portion of the text
 will be printed white on white.  So just that portion would need to be
 painted again in the normal color.

 This should probably be done eventually, anyway, because I like using the
 reverse text color for highlighted cells.  I just don't like extending the
 cell 'highlight' background outside of the selection rectangle because it
 looks REALLY ugly.
*/

  if ( selected && ( cellRef.x() != marker.x() || cellRef.y() != marker.y() ) )
  {
    QPen p( tmpPen );
    p.setColor( defaultColorGroup.highlightedText() );
    painter.setPen( p );
  }
  else {
    painter.setPen(tmpPen);
  }
#endif
  painter.setPen( tmpPen );

  QString  tmpText   = d->strOutText;
  double   tmpHeight = d->textHeight;
  double   tmpWidth  = d->textWidth;

  // If the cell is to narrow to paint the whole contents, then pick
  // out a part of the content that we paint.  The result of this is
  // dependent on the data type of the content.
  //
  // FIXME: Make this dependent on the height as well.
  //
  if ( testFlag( Flag_CellTooShortX ) ) {
    d->strOutText = textDisplaying( painter );

    // Recalculate the text width and the offset.
    textSize( painter );
    offsetAlign( column(), row() );
  }

  // Hide zero.
  if ( format()->sheet()->getHideZero()
       && value().isNumber()
       && value().asFloat() == 0 ) {
    d->strOutText = QString::null;
  }

  // Clear extra cell if column or row is hidden
  //
  // FIXME: I think this should be done before the call to
  // textDisplaying() above.
  //
  if ( colFormat->isHide() || ( cellRect.height() <= 2 ) ) {
    freeAllObscuredCells();  /* TODO: This looks dangerous...must check when I
                                have time */
    d->strOutText = "";
  }

  double indent = 0.0;
  double offsetCellTooShort = 0.0;
  int a = effAlignX();

  // Apply indent if text is align to left not when text is at right or middle.
  if (  a == Format::Left && !isEmpty() ) {
    // FIXME: The following condition should be remade into a call to
    //        a new convenience function:
    //   if ( hasConditionStyleFeature( Style::SIndent, true )...
    //        This should be done throughout the entire file.
    //
    if ( d->hasExtra()
   && d->extra()->conditions
         && d->extra()->conditions->matchedStyle()
         && d->extra()->conditions->matchedStyle()->hasFeature( Style::SIndent, true ) )
      indent = d->extra()->conditions->matchedStyle()->indent();
    else
      indent = format()->getIndent( column(), row() );
  }

  // Made an offset, otherwise ### is under red triangle.
  if ( a == Format::Right && !isEmpty() && testFlag( Flag_CellTooShortX ) )
    offsetCellTooShort = format()->sheet()->doc()->unzoomItX( 4 );

  QFontMetrics fm2 = painter.fontMetrics();
  double offsetFont = 0.0;

  if ( format()->alignY( column(), row() ) == Format::Bottom
       && format()->textFontUnderline( column(), row() ) )
    offsetFont = format()->sheet()->doc()->unzoomItX( fm2.underlinePos() + 1 );

  int  tmpAngle;
  bool tmpMultiRow;
  bool tmpVerticalText;

  // Check for angled or vertical text.
  if ( d->hasExtra()
       && d->extra()->conditions
       && d->extra()->conditions->matchedStyle() )
  {
    Style  *matchedStyle = d->extra()->conditions->matchedStyle();

    if ( matchedStyle->hasFeature( Style::SAngle, true ) )
      tmpAngle = d->extra()->conditions->matchedStyle()->rotateAngle();
    else
      tmpAngle = format()->getAngle( cellRef.x(), cellRef.y() );

    if ( matchedStyle->hasFeature( Style::SVerticalText, true ) )
      tmpVerticalText = matchedStyle->hasProperty( Style::PVerticalText );
    else
      tmpVerticalText = format()->verticalText( cellRef.x(), cellRef.y() );

    if ( matchedStyle->hasFeature( Style::SMultiRow, true ) )
      tmpMultiRow = matchedStyle->hasProperty( Style::PMultiRow );
    else
      tmpMultiRow = format()->multiRow( cellRef.x(), cellRef.y() );
  }
  else {
    tmpAngle        = format()->getAngle( cellRef.x(), cellRef.y() );
    tmpVerticalText = format()->verticalText( cellRef.x(), cellRef.y() );
    tmpMultiRow     = format()->multiRow( cellRef.x(), cellRef.y() );
  }

  // Actually paint the text.
  //    There are 4 possible cases:
  //        - One line of text , horizontal
  //        - Angled text
  //        - Multiple rows of text , horizontal
  //        - Vertical text
  if ( !tmpMultiRow && !tmpVerticalText && !tmpAngle ) {
    // Case 1: The simple case, one line, no angle.

    painter.drawText( doc->zoomItX( indent + cellRect.x() + d->textX - offsetCellTooShort ),
                      doc->zoomItY( cellRect.y() + d->textY - offsetFont ), d->strOutText );
  }
  else if ( tmpAngle != 0 ) {
    // Case 2: an angle.

    int angle = tmpAngle;
    QFontMetrics fm = painter.fontMetrics();

    painter.rotate( angle );
    double x;
    if ( angle > 0 )
      x = indent + cellRect.x() + d->textX;
    else
      x = indent + cellRect.x() + d->textX
        - doc->unzoomItX((int) (( fm.descent() + fm.ascent() ) * sin( angle * M_PI / 180 )));
    double y;
    if ( angle > 0 )
      y = cellRect.y() + d->textY;
    else
      y = cellRect.y() + d->textY + d->textHeight;
    painter.drawText( doc->zoomItX( x * cos( angle * M_PI / 180 ) +
                                    y * sin( angle * M_PI / 180 ) ),
                      doc->zoomItY( -x * sin( angle * M_PI / 180 ) +
                                     y * cos( angle * M_PI / 180 ) ),
                      d->strOutText );
    painter.rotate( -angle );
  }
  else if ( tmpMultiRow && !tmpVerticalText ) {
    // Case 3: Multiple rows, but horizontal.

    QString t;
    int i;
    int pos = 0;
    double dy = 0.0;
    QFontMetrics fm = painter.fontMetrics();
    do {
      i = d->strOutText.find( "\n", pos );
      if ( i == -1 )
        t = d->strOutText.mid( pos, d->strOutText.length() - pos );
      else {
        t = d->strOutText.mid( pos, i - pos );
        pos = i + 1;
      }

      int align = effAlignX();
      if ( format()->sheet()->getShowFormula()
     && !( format()->sheet()->isProtected()
     && format()->isHideFormula( d->column, d->row ) ) )
        align = Format::Left;

      // #### Torben: This looks duplicated for me
      switch ( align ) {
       case Format::Left:
        d->textX = effLeftBorderPen( cellRef.x(), cellRef.y() ).width() + BORDER_SPACE;
        break;

       case Format::Right:
        d->textX = cellRect.width() - BORDER_SPACE - doc->unzoomItX( fm.width( t ) )
          - effRightBorderPen( cellRef.x(), cellRef.y() ).width();
        break;

       case Format::Center:
        d->textX = ( cellRect.width() - doc->unzoomItX( fm.width( t ) ) ) / 2;
      }

      painter.drawText( doc->zoomItX( indent + cellRect.x() + d->textX ),
                        doc->zoomItY( cellRect.y() + d->textY + dy ), t );
      dy += doc->unzoomItY( fm.descent() + fm.ascent() );
    } while ( i != -1 );
  }
  else if ( tmpVerticalText && !d->strOutText.isEmpty() ) {
    // Case 4: Vertical text.

    QString t;
    int i = 0;
    int len = 0;
    double dy = 0.0;
    QFontMetrics fm = painter.fontMetrics();
    do {
      len = d->strOutText.length();
      t = d->strOutText.at( i );
      painter.drawText( doc->zoomItX( indent + cellRect.x() + d->textX ),
                        doc->zoomItY( cellRect.y() + d->textY + dy ), t );
      dy += doc->unzoomItY( fm.descent() + fm.ascent() );
      i++;
    } while ( i != len );
  }

  // Check for too short cell and set the outText for future reference.
  if ( testFlag( Flag_CellTooShortX ) ) {
    d->strOutText = tmpText;
    d->textHeight = tmpHeight;
    d->textWidth  = tmpWidth;
  }

  if ( format()->sheet()->getHideZero() && value().isNumber()
       && value().asFloat() == 0 )
    d->strOutText = tmpText;

  if ( colFormat->isHide() || ( cellRect.height() <= 2 ) )
    d->strOutText = tmpText;
}


// Paint page borders on the page.  Only do this on the screen.
//
void Cell::paintPageBorders( QPainter& painter,
           const KoRect &cellRect,
           const QPoint &cellRef,
           bool paintBorderRight,
           bool paintBorderBottom )
{
  // Not screen?  Return immediately.
  if ( painter.device()->isExtDev() )
    return;

  if ( ! format()->sheet()->isShowPageBorders() )
    return;

  SheetPrint* print = format()->sheet()->print();

  Sheet::LayoutDirection sheetDir =  format()->sheet()->layoutDirection();

  Doc* doc = sheet()->doc();
  int zcellRect_left = doc->zoomItX (cellRect.left());
  int zcellRect_right = doc->zoomItX (cellRect.right());
  int zcellRect_top = doc->zoomItY (cellRect.top());
  int zcellRect_bottom = doc->zoomItY (cellRect.bottom());

  // Draw page borders

  if ( cellRef.x() >= print->printRange().left()
       && cellRef.x() <= print->printRange().right() + 1
       && cellRef.y() >= print->printRange().top()
       && cellRef.y() <= print->printRange().bottom() + 1 )
  {
    if ( print->isOnNewPageX( cellRef.x() )
   && cellRef.y() <= print->printRange().bottom() )
    {
      painter.setPen( sheet()->doc()->pageBorderColor() );

      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( zcellRect_right, zcellRect_top,
                          zcellRect_right, zcellRect_bottom );
      else
        painter.drawLine( zcellRect_left, zcellRect_top,
                          zcellRect_left, zcellRect_bottom );
    }

    if ( print->isOnNewPageY( cellRef.y() ) &&
          ( cellRef.x() <= print->printRange().right() ) )
    {
      painter.setPen( sheet()->doc()->pageBorderColor() );
      painter.drawLine( zcellRect_left,  zcellRect_top,
                        zcellRect_right, zcellRect_top );
    }

    if ( paintBorderRight ) {
      if ( print->isOnNewPageX( cellRef.x() + 1 )
            && cellRef.y() <= print->printRange().bottom() ) {
        painter.setPen( sheet()->doc()->pageBorderColor() );

        if ( sheetDir == Sheet::RightToLeft )
          painter.drawLine( zcellRect_left, zcellRect_top,
                            zcellRect_left, zcellRect_bottom );
        else
          painter.drawLine( zcellRect_right, zcellRect_top,
                            zcellRect_right, zcellRect_bottom );
      }
    }

    if ( paintBorderBottom ) {
      if ( print->isOnNewPageY( cellRef.y() + 1 )
          && cellRef.x() <= print->printRange().right() ) {
        painter.setPen( sheet()->doc()->pageBorderColor() );
        painter.drawLine( zcellRect_left,  zcellRect_bottom,
                          zcellRect_right, zcellRect_bottom );
      }
    }
  }
}


// Paint the cell borders.
//
void Cell::paintCellBorders( QPainter& painter, const KoRect& rect,
           const KoRect &cellRect,
           const QPoint &cellRef,
           bool paintRight, bool paintBottom,
           bool paintLeft,  bool paintTop,
           QPen & _rightPen, QPen & _bottomPen,
           QPen & _leftPen,  QPen & _topPen )
{

  //Sanity check: If we are not painting any of the borders then the function
  //really shouldn't be called at all.
  if ( (!paintLeft) && (!paintRight) && (!paintTop) && (!paintBottom) )
        return;

  Doc * doc = sheet()->doc();

  Sheet::LayoutDirection sheetDir =  format()->sheet()->layoutDirection();

  // compute zoomed rectangles
  // I don't use KoRect, because that ends up producing lots of warnings
  // about double->int conversions in calls to painter.drawLine
  int zrect_left (doc->zoomItX (rect.left()));
  int zrect_right (doc->zoomItX (rect.right()));
  int zrect_top (doc->zoomItY (rect.top()));
  int zrect_bottom (doc->zoomItY (rect.bottom()));
  int zcellRect_left (doc->zoomItX (cellRect.left()));
  int zcellRect_right (doc->zoomItX (cellRect.right()));
  int zcellRect_top (doc->zoomItY (cellRect.top()));
  int zcellRect_bottom (doc->zoomItY (cellRect.bottom()));

  /* we might not paint some borders if this cell is merged with another in
     that direction
  bool paintLeft   = paintBorderLeft;
  bool paintRight  = paintBorderRight;
  bool paintTop    = paintBorderTop;
  bool paintBottom = paintBorderBottom;
  */

  // paintRight  = paintRight  && ( extraXCells() == 0 );
  // paintBottom = paintBottom && ( d->extra()->extraYCells() == 0 );

  if (d->hasExtra()) {
    QValueList<Cell*>::const_iterator it  = d->extra()->obscuringCells.begin();
    QValueList<Cell*>::const_iterator end = d->extra()->obscuringCells.end();
    for ( ; it != end; ++it ) {
      Cell* cell = *it;

      int xDiff = cellRef.x() - cell->column();
      int yDiff = cellRef.y() - cell->row();
      paintLeft = paintLeft && xDiff == 0;
      paintTop  = paintTop  && yDiff == 0;

      // Paint the border(s) if either this one should or if we have a
      // merged cell with this cell as its border.
      paintRight  = paintRight  && cell->mergedXCells() == xDiff;
      paintBottom = paintBottom && cell->mergedYCells() == yDiff;
    }
  }

  // Must create copies of these since otherwise the zoomIt()
  // operation will be performed on them repeatedly.
  QPen  leftPen( _leftPen );
  QPen  rightPen( _rightPen );
  QPen  topPen( _topPen );
  QPen  bottomPen( _bottomPen );

  // Determine the pens that should be used for drawing
  // the borders.
  //
  int left_penWidth   = QMAX( 1, doc->zoomItX( leftPen.width() ) );
  int right_penWidth  = QMAX( 1, doc->zoomItX( rightPen.width() ) );
  int top_penWidth    = QMAX( 1, doc->zoomItY( topPen.width() ) );
  int bottom_penWidth = QMAX( 1, doc->zoomItY( bottomPen.width() ) );

  leftPen.setWidth( left_penWidth );
  rightPen.setWidth( right_penWidth );
  topPen.setWidth( top_penWidth );
  bottomPen.setWidth( bottom_penWidth );

  if ( paintLeft && leftPen.style() != Qt::NoPen ) {
    int top = ( QMAX( 0, -1 + top_penWidth ) ) / 2 +
              ( ( QMAX( 0, -1 + top_penWidth ) ) % 2 );
    int bottom = ( QMAX( 0, -1 + bottom_penWidth ) ) / 2 + 1;

    painter.setPen( leftPen );

    //kdDebug(36001) << "    painting left border of cell " << name() << endl;

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() ) {
      // FIXME: There is probably Cut&Paste bugs here as well as below.
      //        The QMIN/QMAX and left/right pairs don't really make sense.
      //
      //    UPDATE: In fact, most of these QMIN/QMAX combinations
      //            are TOTALLY BOGUS.  For one thing, the idea
      //            that we always have full cells on paper is wrong
      //            since we can have embedded sheets in e.g. kword,
      //            and those can be arbitrarily clipped.  WE HAVE TO
      //            REVISE THIS WHOLE BORDER PAINTING SECTION!
      //
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( QMIN( zrect_right,  zcellRect_right ),
                          QMAX( zrect_top,    zcellRect_top - top ),
                          QMIN( zrect_right,  zcellRect_right ),
                          QMIN( zrect_bottom, zcellRect_bottom + bottom ) );
      else
        painter.drawLine( QMAX( zrect_left,   zcellRect_left ),
                          QMAX( zrect_top,    zcellRect_top - top ),
                          QMAX( zrect_left,   zcellRect_left ),
                          QMIN( zrect_bottom, zcellRect_bottom + bottom ) );
    }
    else {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( zcellRect_right,
                          zcellRect_top - top,
                          zcellRect_right,
                          zcellRect_bottom + bottom );
      else
        painter.drawLine( zcellRect_left,
                          zcellRect_top - top,
                          zcellRect_left,
                          zcellRect_bottom + bottom );
    }
  }

  if ( paintRight && rightPen.style() != Qt::NoPen ) {
    int top = ( QMAX( 0, -1 + top_penWidth ) ) / 2 +
              ( ( QMAX( 0, -1 + top_penWidth ) ) % 2 );
    int bottom = ( QMAX( 0, -1 + bottom_penWidth ) ) / 2 + 1;

    painter.setPen( rightPen );

    //kdDebug(36001) << "    painting right border of cell " << name() << endl;

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() ) {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( QMAX( zrect_left, zcellRect_left ),
                          QMAX( zrect_top, zcellRect_top - top ),
                          QMAX( zrect_left, zcellRect_left ),
                          QMIN( zrect_bottom, zcellRect_bottom + bottom ) );
      else {
  // FIXME: This is the way all these things should look.
  //        Make it so.
  //
  // Only print the right border if it is visible.
  if ( zcellRect_right <= zrect_right + right_penWidth / 2)
    painter.drawLine( zcellRect_right,
          QMAX( zrect_top, zcellRect_top - top ),
          zcellRect_right,
          QMIN( zrect_bottom, zcellRect_bottom + bottom ) );
      }
    }
    else {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( zcellRect_left,
                          zcellRect_top - top,
                          zcellRect_left,
                          zcellRect_bottom + bottom );
      else
        painter.drawLine( zcellRect_right,
                          zcellRect_top - top,
                          zcellRect_right,
                          zcellRect_bottom + bottom );
    }
  }

  if ( paintTop && topPen.style() != Qt::NoPen ) {
    painter.setPen( topPen );

    //kdDebug(36001) << "    painting top border of cell " << name()
    //       << " [" << zcellRect_left << "," << zcellRect_right
    //       << ": " << zcellRect_right - zcellRect_left << "]" << endl;

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() ) {
      if ( zcellRect_top >= zrect_top + top_penWidth / 2)
  painter.drawLine( QMAX( zrect_left,   zcellRect_left ),
        zcellRect_top,
        QMIN( zrect_right,  zcellRect_right ),
        zcellRect_top );
    }
    else {
      painter.drawLine( zcellRect_left, zcellRect_top,
                        zcellRect_right, zcellRect_top );
    }
  }

  if ( paintBottom && bottomPen.style() != Qt::NoPen ) {
    painter.setPen( bottomPen );

    //kdDebug(36001) << "    painting bottom border of cell " << name()
    //       << " [" << zcellRect_left << "," << zcellRect_right
    //       << ": " << zcellRect_right - zcellRect_left << "]" << endl;

    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() ) {
      if ( zcellRect_bottom <= zrect_bottom + bottom_penWidth / 2)
  painter.drawLine( QMAX( zrect_left,   zcellRect_left ),
        zcellRect_bottom,
        QMIN( zrect_right,  zcellRect_right ),
        zcellRect_bottom );
    }
    else {
      painter.drawLine( zcellRect_left, zcellRect_bottom,
                        zcellRect_right, zcellRect_bottom );
    }
  }

  // FIXME: Look very closely at when the following code is really needed.
  //        I can't really see any case, but I might be wrong.
  //        Since the code below is buggy, and incredibly complex,
  //        I am currently disabling it.  If somebody wants to enable
  //        it again, then please also solve bug 68977: "Embedded KSpread
  //        document printing problem" at the same time.
  return;

#if 0
  // Look at the cells on our corners. It may happen that we
  // just erased parts of their borders corner, so we might need
  // to repaint these corners.
  //
  QPen  vert_pen, horz_pen;
  int   vert_penWidth, horz_penWidth;

  // Some useful referenses.
  Cell  *cell_north     = format()->sheet()->cellAt( cellRef.x(),
                 cellRef.y() - 1 );
  Cell  *cell_northwest = format()->sheet()->cellAt( cellRef.x() - 1,
                 cellRef.y() - 1 );
  Cell  *cell_west      = format()->sheet()->cellAt( cellRef.x() - 1,
                 cellRef.y() );
  Cell  *cell_northeast = format()->sheet()->cellAt( cellRef.x() + 1,
                 cellRef.y() - 1 );
  Cell  *cell_east      = format()->sheet()->cellAt( cellRef.x() + 1,
                 cellRef.y() );
  Cell  *cell_south     = format()->sheet()->cellAt( cellRef.x(),
                 cellRef.y() + 1 );
  Cell  *cell_southwest = format()->sheet()->cellAt( cellRef.x() - 1,
                 cellRef.y() + 1 );
  Cell  *cell_southeast = format()->sheet()->cellAt( cellRef.x() + 1,
                 cellRef.y() + 1 );

  // Fix the borders which meet at the top left corner
  if ( cell_north->effLeftBorderValue( cellRef.x(), cellRef.y() - 1 )
       >= cell_northwest->effRightBorderValue( cellRef.x() - 1, cellRef.y() - 1 ) )
    vert_pen = cell_north->effLeftBorderPen( cellRef.x(), cellRef.y() - 1 );
  else
    vert_pen = cell_northwest->effRightBorderPen( cellRef.x() - 1,
              cellRef.y() - 1 );

  vert_penWidth = QMAX( 1, doc->zoomItX( vert_pen.width() ) );
  vert_pen.setWidth( vert_penWidth );

  if ( vert_pen.style() != Qt::NoPen ) {
    if ( cell_west->effTopBorderValue( cellRef.x() - 1, cellRef.y() )
         >= cell_northwest->effBottomBorderValue( cellRef.x() - 1, cellRef.y() - 1 ) )
      horz_pen = cell_west->effTopBorderPen( cellRef.x() - 1, cellRef.y() );
    else
      horz_pen = cell_northwest->effBottomBorderPen( cellRef.x() - 1,
                 cellRef.y() - 1 );

    horz_penWidth = QMAX( 1, doc->zoomItY( horz_pen.width() ) );
    int bottom = ( QMAX( 0, -1 + horz_penWidth ) ) / 2 + 1;

    painter.setPen( vert_pen );
    // If we are on paper printout, we limit the length of the lines.
    // On paper, we always have full cells, on screen not.
    if ( painter.device()->isExtDev() ) {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( QMAX( zrect_left, zcellRect_right ),
                          QMAX( zrect_top, zcellRect_top ),
                          QMIN( zrect_right, zcellRect_right ),
                          QMIN( zrect_bottom, zcellRect_top + bottom ) );
      else
        painter.drawLine( QMAX( zrect_left, zcellRect_left ),
                          QMAX( zrect_top, zcellRect_top ),
                          QMIN( zrect_right, zcellRect_left ),
                          QMIN( zrect_bottom, zcellRect_top + bottom ) );
    }
    else {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( zcellRect_right, zcellRect_top,
                          zcellRect_right, zcellRect_top + bottom );
      else
        painter.drawLine( zcellRect_left, zcellRect_top,
                          zcellRect_left, zcellRect_top + bottom );
    }
  }

  // Fix the borders which meet at the top right corner
  if ( cell_north->effRightBorderValue( cellRef.x(), cellRef.y() - 1 )
       >= cell_northeast->effLeftBorderValue( cellRef.x() + 1,
                cellRef.y() - 1 ) )
    vert_pen = cell_north->effRightBorderPen( cellRef.x(), cellRef.y() - 1 );
  else
    vert_pen = cell_northeast->effLeftBorderPen( cellRef.x() + 1,
             cellRef.y() - 1 );

  // vert_pen = effRightBorderPen( cellRef.x(), cellRef.y() - 1 );
  vert_penWidth = QMAX( 1, doc->zoomItX( vert_pen.width() ) );
  vert_pen.setWidth( vert_penWidth );
  if ( ( vert_pen.style() != Qt::NoPen ) && ( cellRef.x() < KS_colMax ) ) {
    if ( cell_east->effTopBorderValue( cellRef.x() + 1, cellRef.y() )
         >= cell_northeast->effBottomBorderValue( cellRef.x() + 1,
              cellRef.y() - 1 ) )
      horz_pen = cell_east->effTopBorderPen( cellRef.x() + 1, cellRef.y() );
    else
      horz_pen = cell_northeast->effBottomBorderPen( cellRef.x() + 1,
                 cellRef.y() - 1 );

    // horz_pen = effTopBorderPen( cellRef.x() + 1, cellRef.y() );
    horz_penWidth = QMAX( 1, doc->zoomItY( horz_pen.width() ) );
    int bottom = ( QMAX( 0, -1 + horz_penWidth ) ) / 2 + 1;

    painter.setPen( vert_pen );
    //If we are on paper printout, we limit the length of the lines
    //On paper, we always have full cells, on screen not
    if ( painter.device()->isExtDev() ) {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( QMAX( zrect_left, zcellRect_left ),
                          QMAX( zrect_top, zcellRect_top ),
                          QMIN( zrect_right, zcellRect_left ),
                          QMIN( zrect_bottom, zcellRect_top + bottom ) );
      else
        painter.drawLine( QMAX( zrect_left, zcellRect_right ),
                          QMAX( zrect_top, zcellRect_top ),
                          QMIN( zrect_right, zcellRect_right ),
                          QMIN( zrect_bottom, zcellRect_top + bottom ) );
    }
    else {
      if ( sheetDir == Sheet::RightToLeft )
        painter.drawLine( zcellRect_left, zcellRect_top,
                          zcellRect_left, zcellRect_top + bottom );
      else
        painter.drawLine( zcellRect_right, zcellRect_top,
                          zcellRect_right, zcellRect_top + bottom );
    }
  }

  // Bottom
  if ( cellRef.y() < KS_rowMax ) {
    // Fix the borders which meet at the bottom left corner
    if ( cell_south->effLeftBorderValue( cellRef.x(), cellRef.y() + 1 )
         >= cell_southwest->effRightBorderValue( cellRef.x() - 1,
             cellRef.y() + 1 ) )
      vert_pen = cell_south->effLeftBorderPen( cellRef.x(), cellRef.y() + 1 );
    else
      vert_pen = cell_southwest->effRightBorderPen( cellRef.x() - 1,
                cellRef.y() + 1 );

    // vert_pen = effLeftBorderPen( cellRef.x(), cellRef.y() + 1 );
    vert_penWidth = QMAX( 1, doc->zoomItY( vert_pen.width() ) );
    vert_pen.setWidth( vert_penWidth );
    if ( vert_pen.style() != Qt::NoPen ) {
      if ( cell_west->effBottomBorderValue( cellRef.x() - 1, cellRef.y() )
           >= cell_southwest->effTopBorderValue( cellRef.x() - 1,
             cellRef.y() + 1 ) )
        horz_pen = cell_west->effBottomBorderPen( cellRef.x() - 1,
              cellRef.y() );
      else
        horz_pen = cell_southwest->effTopBorderPen( cellRef.x() - 1,
                cellRef.y() + 1 );

      // horz_pen = effBottomBorderPen( cellRef.x() - 1, cellRef.y() );
      horz_penWidth = QMAX( 1, doc->zoomItX( horz_pen.width() ) );
      int bottom = ( QMAX( 0, -1 + horz_penWidth ) ) / 2;

      painter.setPen( vert_pen );
      // If we are on paper printout, we limit the length of the lines.
      // On paper, we always have full cells, on screen not.
      if ( painter.device()->isExtDev() ) {
        if ( sheetDir == Sheet::RightToLeft )
          painter.drawLine( QMAX( zrect_left, zcellRect_right ),
                            QMAX( zrect_top, zcellRect_bottom - bottom ),
                            QMIN( zrect_right, zcellRect_right ),
                            QMIN( zrect_bottom, zcellRect_bottom ) );
        else
          painter.drawLine( QMAX( zrect_left, zcellRect_left ),
                            QMAX( zrect_top, zcellRect_bottom - bottom ),
                            QMIN( zrect_right, zcellRect_left ),
                            QMIN( zrect_bottom, zcellRect_bottom ) );
      }
      else {
        if ( sheetDir == Sheet::RightToLeft )
          painter.drawLine( zcellRect_right, zcellRect_bottom - bottom,
                            zcellRect_right, zcellRect_bottom );
        else
          painter.drawLine( zcellRect_left, zcellRect_bottom - bottom,
                            zcellRect_left, zcellRect_bottom );
      }
    }

    // Fix the borders which meet at the bottom right corner
    if ( cell_south->effRightBorderValue( cellRef.x(), cellRef.y() + 1 )
         >= cell_southeast->effLeftBorderValue( cellRef.x() + 1,
            cellRef.y() + 1 ) )
      vert_pen = cell_south->effRightBorderPen( cellRef.x(), cellRef.y() + 1 );
    else
      vert_pen = cell_southeast->effLeftBorderPen( cellRef.x() + 1,
               cellRef.y() + 1 );

    // vert_pen = effRightBorderPen( cellRef.x(), cellRef.y() + 1 );
    vert_penWidth = QMAX( 1, doc->zoomItY( vert_pen.width() ) );
    vert_pen.setWidth( vert_penWidth );
    if ( ( vert_pen.style() != Qt::NoPen ) && ( cellRef.x() < KS_colMax ) ) {
      if ( cell_east ->effBottomBorderValue( cellRef.x() + 1, cellRef.y() )
           >= cell_southeast->effTopBorderValue( cellRef.x() + 1,
             cellRef.y() + 1 ) )

        horz_pen = format()->sheet()->cellAt( cellRef.x() + 1, cellRef.y() )
    ->effBottomBorderPen( cellRef.x() + 1, cellRef.y() );
      else
        horz_pen = format()->sheet()->cellAt( cellRef.x() + 1, cellRef.y() + 1 )
    ->effTopBorderPen( cellRef.x() + 1, cellRef.y() + 1 );

      // horz_pen = effBottomBorderPen( cellRef.x() + 1, cellRef.y() );
      horz_penWidth = QMAX( 1, doc->zoomItX( horz_pen.width() ) );
      int bottom = ( QMAX( 0, -1 + horz_penWidth ) ) / 2;

      painter.setPen( vert_pen );
      // If we are on paper printout, we limit the length of the lines.
      // On paper, we always have full cells, on screen not.
      if ( painter.device()->isExtDev() )      {
        if ( sheetDir == Sheet::RightToLeft )
          painter.drawLine( QMAX( zrect_left, zcellRect_left ),
                            QMAX( zrect_top, zcellRect_bottom - bottom ),
                            QMIN( zrect_right, zcellRect_left ),
                            QMIN( zrect_bottom, zcellRect_bottom ) );
        else
          painter.drawLine( QMAX( zrect_left, zcellRect_right ),
                            QMAX( zrect_top, zcellRect_bottom - bottom ),
                            QMIN( zrect_right, zcellRect_right ),
                            QMIN( zrect_bottom, zcellRect_bottom ) );
      }
      else {
        if ( sheetDir == Sheet::RightToLeft )
          painter.drawLine( zcellRect_left, zcellRect_bottom - bottom,
                            zcellRect_left, zcellRect_bottom );
        else
          painter.drawLine( zcellRect_right, zcellRect_bottom - bottom,
                            zcellRect_right, zcellRect_bottom );
      }
    }
  }
  #endif
}


// Paint diagonal lines through the cell.
//
void Cell::paintCellDiagonalLines( QPainter& painter,
           const KoRect &cellRect,
           const QPoint &cellRef )
{
  if ( isPartOfMerged() )
    return;

  Doc* doc = sheet()->doc();

  if ( effFallDiagonalPen( cellRef.x(), cellRef.y() ).style() != Qt::NoPen ) {
    painter.setPen( effFallDiagonalPen( cellRef.x(), cellRef.y() ) );
    painter.drawLine( doc->zoomItX( cellRect.x() ),
          doc->zoomItY( cellRect.y() ),
          doc->zoomItX( cellRect.right() ),
          doc->zoomItY( cellRect.bottom() ) );
  }

  if ( effGoUpDiagonalPen( cellRef.x(), cellRef.y() ).style() != Qt::NoPen ) {
    painter.setPen( effGoUpDiagonalPen( cellRef.x(), cellRef.y() ) );
    painter.drawLine( doc->zoomItX( cellRect.x() ),
          doc->zoomItY( cellRect.bottom() ),
          doc->zoomItX( cellRect.right() ),
          doc->zoomItY( cellRect.y() ) );
  }
}


//                        End of Painting
// ================================================================


int Cell::defineAlignX()
{
  int a = format()->align( column(), row() );
  if ( a == Format::Undefined )
  {
    //numbers should be right-aligned by default, as well as BiDi text
    if ((formatType() == Text_format) || value().isString())
      a = (d->strOutText.isRightToLeft()) ?
                               Format::Right : Format::Left;
    else {
      Value val = value();
      while (val.isArray()) val = val.element (0, 0);
      if (val.isBoolean() || val.isNumber())
        a = Format::Right;
      else
        a = Format::Left;
    }
  }
  return a;
}

int Cell::effAlignX()
{
  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SAlignX, true ) )
    return d->extra()->conditions->matchedStyle()->alignX();

  return defineAlignX();
}

// Cut strOutText, so that it only holds the part that can be displayed.
//
// Used in paintText().
//

QString Cell::textDisplaying( QPainter &_painter )
{
  QFontMetrics  fm = _painter.fontMetrics();
  int           a  = format()->align( column(), row() );

  bool isNumeric = value().isNumber();

  if ( !format()->verticalText( column(),row() ) ) {
    // Non-vertical text: the ordinary case.

    // Not enough space but align to left
    double  len = 0.0;
    int     extraXCells = d->hasExtra() ? d->extra()->extraXCells : 0;

    for ( int i = column(); i <= column() + extraXCells; i++ ) {
      ColumnFormat *cl2 = format()->sheet()->columnFormat( i );
      len += cl2->dblWidth() - 1.0; //-1.0 because the pixel in between 2 cells is shared between both cells
    }

    QString  tmp;
    double   tmpIndent = 0.0;
    if ( !isEmpty() )
      tmpIndent = format()->getIndent( column(), row() );

    // Start out with the whole text, cut one character at a time, and
    // when the text finally fits, return it.
    for ( int i = d->strOutText.length(); i != 0; i-- )
    {
      //Note that numbers are always treated as left-aligned since if we have to cut digits off, they should
      //always be the least significant ones at the end of the string
      if ( a == Format::Left || a == Format::Undefined || isNumeric)
  		tmp = d->strOutText.left(i);
      else if ( a == Format::Right)
  		tmp = d->strOutText.right(i);
      else
  		tmp = d->strOutText.mid( ( d->strOutText.length() - i ) / 2, i);

      if (isNumeric)
      {
	  //For numeric values, we can cut off digits after the decimal point to make it fit,
	  //but not the integer part of the number.
	  //If this number still contains a fraction part then we don't need to do anything, if we have run
	  //out of space to fit even the integer part of the number then display #########
	  //TODO Perhaps try to display integer part in standard form if there is not enough room for it?

	  if (!tmp.contains('.'))
	  	d->strOutText=QString().fill('#',20);
      }

      // 4 equal length of red triangle +1 point.
      if ( format()->sheet()->doc()->unzoomItX( fm.width( tmp ) ) + tmpIndent
     < len - 4.0 - 1.0 )
      {
  	if ( format()->getAngle( column(), row() ) != 0 )
	{
    		QString tmp2;
    		RowFormat *rl = format()->sheet()->rowFormat( row() );
    		if ( d->textHeight > rl->dblHeight() )
		{
      			for ( int j = d->strOutText.length(); j != 0; j-- )
			{
        			tmp2 = d->strOutText.left( j );
        			if ( format()->sheet()->doc()->unzoomItY( fm.width( tmp2 ) ) < rl->dblHeight() - 1.0 )
        			{
    					return d->strOutText.left( QMIN( tmp.length(), tmp2.length() ) );
        			}
      			}
    		}
    		else
      			return tmp;

  	}
  	else
    		return tmp;
      }
    }
    return QString( "" );
  }
  else if ( format()->verticalText( column(), row() ) ) {
    // Vertical text.

    RowFormat  *rl = format()->sheet()->rowFormat( row() );
    double      tmpIndent = 0.0;

    // Not enough space but align to left.
    double  len = 0.0;
    int     extraXCells = d->hasExtra() ? d->extra()->extraXCells : 0;

    for ( int i = column(); i <= column() + extraXCells; i++ ) {
      ColumnFormat  *cl2 = format()->sheet()->columnFormat( i );

      // -1.0 because the pixel in between 2 cells is shared between both cells
      len += cl2->dblWidth() - 1.0;
    }

    if ( !isEmpty() )
      tmpIndent = format()->getIndent( column(), row() );

    if ( ( d->textWidth + tmpIndent > len ) || d->textWidth == 0.0 )
      return QString( "" );

    for ( int i = d->strOutText.length(); i != 0; i-- ) {
      if ( format()->sheet()->doc()->unzoomItY( fm.ascent() + fm.descent() ) * i
     < rl->dblHeight() - 1.0 )
  return d->strOutText.left( i );
    }

    return QString( "" );
  }

  ColumnFormat  *cl = format()->sheet()->columnFormat( column() );
  double         w = cl->dblWidth();

  if ( d->hasExtra() && (d->extra()->extraWidth != 0.0) )
    w = d->extra()->extraWidth;

  QString tmp;
  for ( int i = d->strOutText.length(); i != 0; i-- ) {
    tmp = d->strOutText.left( i );

    // 4 equals lenght of red triangle +1 pixel
    if ( format()->sheet()->doc()->unzoomItX( fm.width( tmp ) ) < w - 4.0 - 1.0 )
      return tmp;
  }

  return  QString::null;
}


double Cell::dblWidth( int _col, const Canvas *_canvas ) const
{
  if ( _col < 0 )
    _col = d->column;

  if ( _canvas )
  {
    if ( testFlag(Flag_Merged) )
      return d->extra()->extraWidth;

    const ColumnFormat *cl = format()->sheet()->columnFormat( _col );
    return cl->dblWidth( _canvas );
  }

  if ( testFlag(Flag_Merged) )
    return d->extra()->extraWidth;

  const ColumnFormat *cl = format()->sheet()->columnFormat( _col );
  return cl->dblWidth();
}

int Cell::width( int _col, const Canvas *_canvas ) const
{
  return int( dblWidth( _col, _canvas ) );
}

double Cell::dblHeight( int _row, const Canvas *_canvas ) const
{
  if ( _row < 0 )
    _row = d->row;

  if ( _canvas )
  {
    if ( testFlag(Flag_Merged) )
      return d->extra()->extraHeight;

    const RowFormat *rl = format()->sheet()->rowFormat( _row );
    return rl->dblHeight( _canvas );
  }

  if ( testFlag(Flag_Merged) )
    return d->extra()->extraHeight;

  const RowFormat *rl = format()->sheet()->rowFormat( _row );
  return rl->dblHeight();
}

int Cell::height( int _row, const Canvas *_canvas ) const
{
  return int( dblHeight( _row, _canvas ) );
}

///////////////////////////////////////////
//
// Misc Properties.
// Reimplementation of Format methods.
//
///////////////////////////////////////////

const QBrush& Cell::backGroundBrush( int _col, int _row ) const
{
  if ( d->hasExtra() && (!d->extra()->obscuringCells.isEmpty()) )
  {
    const Cell* cell = d->extra()->obscuringCells.first();
    return cell->backGroundBrush( cell->column(), cell->row() );
  }

  return format()->backGroundBrush( _col, _row );
}

const QColor& Cell::bgColor( int _col, int _row ) const
{
  if ( d->hasExtra() && (!d->extra()->obscuringCells.isEmpty()) )
  {
    const Cell* cell = d->extra()->obscuringCells.first();
    return cell->bgColor( cell->column(), cell->row() );
  }

  return format()->bgColor( _col, _row );
}

///////////////////////////////////////////
//
// Borders.
// Reimplementation of Format methods.
//
///////////////////////////////////////////

void Cell::setLeftBorderPen( const QPen& p )
{
  if ( column() == 1 )
  {
    Cell* cell = format()->sheet()->cellAt( column() - 1, row() );
    if ( cell && cell->format()->hasProperty( Format::PRightBorder )
         && format()->sheet()->cellAt( column(), row() ) == this )
        cell->format()->clearProperty( Format::PRightBorder );
  }

  format()->setLeftBorderPen( p );
}

void Cell::setTopBorderPen( const QPen& p )
{
  if ( row() == 1 )
  {
    Cell* cell = format()->sheet()->cellAt( column(), row() - 1 );
    if ( cell && cell->format()->hasProperty( Format::PBottomBorder )
         && format()->sheet()->cellAt( column(), row() ) == this )
        cell->format()->clearProperty( Format::PBottomBorder );
  }
  format()->setTopBorderPen( p );
}

void Cell::setRightBorderPen( const QPen& p )
{
    Cell* cell = 0L;
    if ( column() < KS_colMax )
        cell = format()->sheet()->cellAt( column() + 1, row() );

    if ( cell && cell->format()->hasProperty( Format::PLeftBorder )
         && format()->sheet()->cellAt( column(), row() ) == this )
        cell->format()->clearProperty( Format::PLeftBorder );

    format()->setRightBorderPen( p );
}

void Cell::setBottomBorderPen( const QPen& p )
{
    Cell* cell = 0L;
    if ( row() < KS_rowMax )
        cell = format()->sheet()->cellAt( column(), row() + 1 );

    if ( cell && cell->format()->hasProperty( Format::PTopBorder )
         && format()->sheet()->cellAt( column(), row() ) == this )
        cell->format()->clearProperty( Format::PTopBorder );

    format()->setBottomBorderPen( p );
}

const QPen& Cell::rightBorderPen( int _col, int _row ) const
{
    if ( !format()->hasProperty( Format::PRightBorder ) && ( _col < KS_colMax ) )
    {
        Cell * cell = format()->sheet()->cellAt( _col + 1, _row );
        if ( cell && cell->format()->hasProperty( Format::PLeftBorder ) )
            return cell->leftBorderPen( _col + 1, _row );
    }

    return format()->rightBorderPen( _col, _row );
}

const QPen& Cell::leftBorderPen( int _col, int _row ) const
{
    if ( !format()->hasProperty( Format::PLeftBorder ) )
    {
        const Cell * cell = format()->sheet()->cellAt( _col - 1, _row );
        if ( cell && cell->format()->hasProperty( Format::PRightBorder ) )
            return cell->rightBorderPen( _col - 1, _row );
    }

    return format()->leftBorderPen( _col, _row );
}

const QPen& Cell::bottomBorderPen( int _col, int _row ) const
{
    if ( !format()->hasProperty( Format::PBottomBorder ) && ( _row < KS_rowMax ) )
    {
        const Cell * cell = format()->sheet()->cellAt( _col, _row + 1 );
        if ( cell && cell->format()->hasProperty( Format::PTopBorder ) )
            return cell->topBorderPen( _col, _row + 1 );
    }

    return format()->bottomBorderPen( _col, _row );
}

const QPen& Cell::topBorderPen( int _col, int _row ) const
{
    if ( !format()->hasProperty( Format::PTopBorder ) )
    {
        const Cell * cell = format()->sheet()->cellAt( _col, _row - 1 );
        if ( cell->format()->hasProperty( Format::PBottomBorder ) )
            return cell->bottomBorderPen( _col, _row - 1 );
    }

    return format()->topBorderPen( _col, _row );
}

const QColor & Cell::effTextColor( int col, int row ) const
{
  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::STextPen, true ) )
    return d->extra()->conditions->matchedStyle()->pen().color();

  return format()->textColor( col, row );
}

const QPen& Cell::effLeftBorderPen( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effLeftBorderPen( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SLeftBorder, true ) )
    return d->extra()->conditions->matchedStyle()->leftBorderPen();

  return leftBorderPen( col, row );
}

const QPen& Cell::effTopBorderPen( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effTopBorderPen( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::STopBorder, true ) )
    return d->extra()->conditions->matchedStyle()->topBorderPen();

  return topBorderPen( col, row );
}

const QPen& Cell::effRightBorderPen( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effRightBorderPen( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SRightBorder, true ) )
    return d->extra()->conditions->matchedStyle()->rightBorderPen();

  return rightBorderPen( col, row );
}

const QPen& Cell::effBottomBorderPen( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effBottomBorderPen( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SBottomBorder, true ) )
    return d->extra()->conditions->matchedStyle()->bottomBorderPen();

  return bottomBorderPen( col, row );
}

const QPen & Cell::effGoUpDiagonalPen( int col, int row ) const
{
  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SGoUpDiagonal, true ) )
    return d->extra()->conditions->matchedStyle()->goUpDiagonalPen();

  return format()->goUpDiagonalPen( col, row );
}

const QPen & Cell::effFallDiagonalPen( int col, int row ) const
{
  if ( d->hasExtra() && d->extra()->conditions
       && d->extra()->conditions->matchedStyle()
       && d->extra()->conditions->matchedStyle()->hasFeature( Style::SFallDiagonal, true ) )
    return d->extra()->conditions->matchedStyle()->fallDiagonalPen();

  return format()->fallDiagonalPen( col, row );
}

uint Cell::effBottomBorderValue( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effBottomBorderValue( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
      && d->extra()->conditions->matchedStyle() )
    return d->extra()->conditions->matchedStyle()->bottomPenValue();

  return format()->bottomBorderValue( col, row );
}

uint Cell::effRightBorderValue( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effRightBorderValue( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
      && d->extra()->conditions->matchedStyle() )
    return d->extra()->conditions->matchedStyle()->rightPenValue();

  return format()->rightBorderValue( col, row );
}

uint Cell::effLeftBorderValue( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effLeftBorderValue( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
      && d->extra()->conditions->matchedStyle() )
    return d->extra()->conditions->matchedStyle()->leftPenValue();

  return format()->leftBorderValue( col, row );
}

uint Cell::effTopBorderValue( int col, int row ) const
{
  if ( isPartOfMerged() )
  {
    Cell * cell = d->extra()->obscuringCells.first();
    return cell->effTopBorderValue( cell->column(), cell->row() );
  }

  if ( d->hasExtra() && d->extra()->conditions
      && d->extra()->conditions->matchedStyle() )
    return d->extra()->conditions->matchedStyle()->topPenValue();

  return format()->topBorderValue( col, row );
}

///////////////////////////////////////////
//
// Precision
//
///////////////////////////////////////////

void Cell::incPrecision()
{
  //TODO: This is ugly. Why not simply regenerate the text to display? Tomas

  if ( !value().isNumber() )
    return;
  int tmpPreci = format()->precision( column(), row() );

  if ( tmpPreci == -1 )
  {
    int pos = d->strOutText.find(decimal_point);
    if ( pos == -1 )
        pos = d->strOutText.find('.');
    if ( pos == -1 )
      format()->setPrecision(1);
    else
    {
      int start = 0;
      if ( d->strOutText.find('%') != -1 )
        start = 2;
      else if ( d->strOutText.find(locale()->currencySymbol()) == ((int)(d->strOutText.length()-locale()->currencySymbol().length())) )
        start = locale()->currencySymbol().length() + 1;
      else if ( (start=d->strOutText.find('E')) != -1 )
        start = d->strOutText.length() - start;

      //kdDebug(36001) << "start=" << start << " pos=" << pos << " length=" << d->strOutText.length() << endl;
      format()->setPrecision( QMAX( 0, (int)d->strOutText.length() - start - pos ) );
    }
  }
  else if ( tmpPreci < 10 )
  {
    format()->setPrecision( ++tmpPreci );
  }
  setFlag(Flag_LayoutDirty);
}

void Cell::decPrecision()
{
  //TODO: This is ugly. Why not simply regenerate the text to display? Tomas

  if ( !value().isNumber() )
    return;
  int preciTmp = format()->precision( column(), row() );
//  kdDebug(36001) << "decPrecision: tmpPreci = " << tmpPreci << endl;
  if ( format()->precision(column(),row()) == -1 )
  {
    int pos = d->strOutText.find( decimal_point );
    int start = 0;
    if ( d->strOutText.find('%') != -1 )
        start = 2;
    else if ( d->strOutText.find(locale()->currencySymbol()) == ((int)(d->strOutText.length()-locale()->currencySymbol().length())) )
        start = locale()->currencySymbol().length() + 1;
    else if ( (start = d->strOutText.find('E')) != -1 )
        start = d->strOutText.length() - start;
    else
        start = 0;

    if ( pos == -1 )
      return;

    format()->setPrecision(d->strOutText.length() - pos - 2 - start);
    //   if ( preciTmp < 0 )
    //      format()->setPrecision( preciTmp );
  }
  else if ( preciTmp > 0 )
  {
    format()->setPrecision( --preciTmp );
  }
  setFlag( Flag_LayoutDirty );
}

//set numerical value
//used in Sheet::setSeries (nowhere else yet)
void Cell::setNumber( double number )
{
  setValue( Value( number ) );

  d->strText.setNum( number );
  setDisplayText(d->strText);
  checkNumberFormat();
}

void Cell::setCellText( const QString& _text, bool asText )
{
 // QString ctext = _text;

// (Tomas) is this trim necessary for anything ?
//  if( ctext.length() > 5000 )
//    ctext = ctext.left( 5000 );

  // empty string ?
  if (_text.length() == 0) {
    d->strOutText = d->strText = "";
    setValue (Value::empty());
    return;
  }

  // as text ?
  if (asText) {
    d->strOutText = _text;
    d->strText    = _text;
    setValue (Value (_text));

    return;
  }

  QString oldText = d->strText;
  setDisplayText( _text );
  if(!format()->sheet()->isLoading() && !testValidity() )
  {
    //reapply old value if action == stop
    setDisplayText( oldText );
  }
}

void Cell::setDisplayText( const QString& _text )
{
  bool isLoading = format()->sheet()->isLoading();

  if (!isLoading)
    format()->sheet()->doc()->emitBeginOperation( false );

  d->strText = _text;

  /**
   * A real formula "=A1+A2*3" was entered.
   */
  if ( !d->strText.isEmpty() && d->strText[0] == '=' )
  {
    setFlag(Flag_LayoutDirty);
    setFlag(Flag_TextFormatDirty);

    if ( !makeFormula() )
      kdError(36001) << "ERROR: Syntax ERROR" << endl;
    setCalcDirtyFlag ();
  }

  /**
   * Some numeric value or a string.
   */
  else
  {
    // Find out what data type it is
    checkTextInput();

    setFlag(Flag_LayoutDirty);
    setFlag(Flag_TextFormatDirty);
  }

  if ( !isLoading )
    format()->sheet()->doc()->emitEndOperation( QRect( d->column, d->row, 1, 1 ) );
}

void Cell::setLink( const QString& link )
{
  d->extra()->link = link;

  if( !link.isEmpty() && d->strText.isEmpty() )
    setCellText( link );
}

QString Cell::link() const
{
  return d->hasExtra() ? d->extra()->link : QString::null;
}

void Cell::update()
{
  /* those obscuring us need to redo their layout cause they can't obscure us
     now that we've got text.
     This includes cells obscuring cells that we are obscuring
  */
  for (int x = d->column; x <= d->column + extraXCells(); x++)
  {
    for (int y = d->row; y <= d->row + extraYCells(); y++)
    {
      Cell* cell = format()->sheet()->cellAt(x,y);
      cell->setLayoutDirtyFlag();
    }
  }

  setCalcDirtyFlag();

  /* TODO - is this a good place for this? */
  updateChart(true);
}

bool Cell::testValidity() const
{
    bool valid = false;
    if( d->hasExtra() && d->extra()->validity && d->extra()->validity->m_restriction != Restriction::None )
    {
        //fixme
        if ( d->extra()->validity->allowEmptyCell && d->strText.isEmpty() )
            return true;

        if( value().isNumber() &&
            (d->extra()->validity->m_restriction == Restriction::Number ||
             (d->extra()->validity->m_restriction == Restriction::Integer &&
              value().asFloat() == ceil(value().asFloat()))))
        {
            switch( d->extra()->validity->m_cond)
            {
              case Conditional::Equal:
                valid = ( value().asFloat() - d->extra()->validity->valMin < DBL_EPSILON
                          && value().asFloat() - d->extra()->validity->valMin >
                          (0.0 - DBL_EPSILON));
                break;
              case Conditional::DifferentTo:
                valid = !(  ( value().asFloat() - d->extra()->validity->valMin < DBL_EPSILON
                              && value().asFloat() - d->extra()->validity->valMin >
                              (0.0 - DBL_EPSILON)) );
                break;
              case Conditional::Superior:
                valid = ( value().asFloat() > d->extra()->validity->valMin);
                break;
              case Conditional::Inferior:
                valid = ( value().asFloat()  <d->extra()->validity->valMin);
                break;
              case Conditional::SuperiorEqual:
                valid = ( value().asFloat() >= d->extra()->validity->valMin);
                break;
              case Conditional::InferiorEqual:
                valid = (value().asFloat() <= d->extra()->validity->valMin);
                break;
              case Conditional::Between:
                valid = ( value().asFloat() >= d->extra()->validity->valMin &&
                          value().asFloat() <= d->extra()->validity->valMax);
                break;
              case Conditional::Different:
                valid = (value().asFloat() < d->extra()->validity->valMin ||
                         value().asFloat() > d->extra()->validity->valMax);
                break;
            default :
                break;
            }
        }
        else if(d->extra()->validity->m_restriction==Restriction::Text)
        {
            valid = value().isString();
        }
        else if ( d->extra()->validity->m_restriction == Restriction::List )
        {
            //test int value
            if ( value().isString() && d->extra()->validity->listValidity.contains( value().asString() ) )
                valid = true;
        }
        else if(d->extra()->validity->m_restriction==Restriction::TextLength)
        {
            if( value().isString() )
            {
                int len = d->strOutText.length();
                switch( d->extra()->validity->m_cond)
                {
                  case Conditional::Equal:
                    if (len == d->extra()->validity->valMin)
                        valid = true;
                    break;
                  case Conditional::DifferentTo:
                    if (len != d->extra()->validity->valMin)
                        valid = true;
                    break;
                  case Conditional::Superior:
                    if(len > d->extra()->validity->valMin)
                        valid = true;
                    break;
                  case Conditional::Inferior:
                    if(len < d->extra()->validity->valMin)
                        valid = true;
                    break;
                  case Conditional::SuperiorEqual:
                    if(len >= d->extra()->validity->valMin)
                        valid = true;
                    break;
                  case Conditional::InferiorEqual:
                    if(len <= d->extra()->validity->valMin)
                        valid = true;
                    break;
                  case Conditional::Between:
                    if(len >= d->extra()->validity->valMin && len <= d->extra()->validity->valMax)
                        valid = true;
                    break;
                  case Conditional::Different:
                    if(len <d->extra()->validity->valMin || len >d->extra()->validity->valMax)
                        valid = true;
                    break;
                default :
                    break;
                }
            }
        }
        else if(d->extra()->validity->m_restriction == Restriction::Time && isTime())
        {
            switch( d->extra()->validity->m_cond)
            {
              case Conditional::Equal:
                valid = (value().asTime() == d->extra()->validity->timeMin);
                break;
              case Conditional::DifferentTo:
                valid = (value().asTime() != d->extra()->validity->timeMin);
                break;
              case Conditional::Superior:
                valid = (value().asTime() > d->extra()->validity->timeMin);
                break;
              case Conditional::Inferior:
                valid = (value().asTime() < d->extra()->validity->timeMin);
                break;
              case Conditional::SuperiorEqual:
                valid = (value().asTime() >= d->extra()->validity->timeMin);
                break;
              case Conditional::InferiorEqual:
                valid = (value().asTime() <= d->extra()->validity->timeMin);
                break;
              case Conditional::Between:
                valid = (value().asTime() >= d->extra()->validity->timeMin &&
                         value().asTime() <= d->extra()->validity->timeMax);
                break;
              case Conditional::Different:
                valid = (value().asTime() < d->extra()->validity->timeMin ||
                         value().asTime() > d->extra()->validity->timeMax);
                break;
            default :
                break;

            }
        }
        else if(d->extra()->validity->m_restriction == Restriction::Date && isDate())
        {
            switch( d->extra()->validity->m_cond)
            {
              case Conditional::Equal:
                valid = (value().asDate() == d->extra()->validity->dateMin);
                break;
              case Conditional::DifferentTo:
                valid = (value().asDate() != d->extra()->validity->dateMin);
                break;
              case Conditional::Superior:
                valid = (value().asDate() > d->extra()->validity->dateMin);
                break;
              case Conditional::Inferior:
                valid = (value().asDate() < d->extra()->validity->dateMin);
                break;
              case Conditional::SuperiorEqual:
                valid = (value().asDate() >= d->extra()->validity->dateMin);
                break;
              case Conditional::InferiorEqual:
                valid = (value().asDate() <= d->extra()->validity->dateMin);
                break;
              case Conditional::Between:
                valid = (value().asDate() >= d->extra()->validity->dateMin &&
                         value().asDate() <= d->extra()->validity->dateMax);
                break;
              case Conditional::Different:
                valid = (value().asDate() < d->extra()->validity->dateMin ||
                         value().asDate() > d->extra()->validity->dateMax);
                break;
            default :
                break;

            }
        }
    }
    else
    {
        valid= true;
    }

    if(!valid &&d->extra()->validity != NULL && d->extra()->validity->displayMessage)
    {
        switch (d->extra()->validity->m_action )
        {
          case Action::Stop:
            KMessageBox::error((QWidget*)0L, d->extra()->validity->message,
                               d->extra()->validity->title);
            break;
          case Action::Warning:
            KMessageBox::warningYesNo((QWidget*)0L, d->extra()->validity->message,
                                      d->extra()->validity->title);
            break;
          case Action::Information:
            KMessageBox::information((QWidget*)0L, d->extra()->validity->message,
                                     d->extra()->validity->title);
            break;
        }
    }
    if (!d->hasExtra())
        return true;  //okay if there's no validity
    return (valid || d->extra()->validity == NULL || d->extra()->validity->m_action != Action::Stop);
}

FormatType Cell::formatType() const
{
    return format()->getFormatType( d->column, d->row );
}

double Cell::textWidth() const
{
    return d->textWidth;
}

double Cell::textHeight() const
{
    return d->textHeight;
}

int Cell::mergedXCells() const
{
    return d->hasExtra() ? d->extra()->mergedXCells : 0;
}

int Cell::mergedYCells() const
{
    return d->hasExtra() ? d->extra()->mergedYCells : 0;
}

int Cell::extraXCells() const
{
    return d->hasExtra() ? d->extra()->extraXCells : 0;
}

int Cell::extraYCells() const
{
    return d->hasExtra() ? d->extra()->extraYCells : 0;
}

double Cell::extraWidth() const
{
    return d->hasExtra() ? d->extra()->extraWidth : 0;
}

double Cell::extraHeight() const
{
    return d->hasExtra() ? d->extra()->extraHeight : 0;
}


bool Cell::isDate() const
{
  FormatType ft = formatType();

  return (formatIsDate (ft) || ((ft == Generic_format) &&
      (value().format() == Value::fmt_Date)));
}

bool Cell::isTime() const
{
  FormatType ft = formatType();

  return (formatIsTime (ft) || ((ft == Generic_format) &&
      (value().format() == Value::fmt_Time)));
}

void Cell::setCalcDirtyFlag()
{
  if ( !isFormula() )
  {
    //don't set the flag if we don't hold a formula
    clearFlag(Flag_CalcDirty);
    return;
  }
  setFlag(Flag_CalcDirty);
  format()->sheet()->setRegionPaintDirty(cellRect());
}


bool Cell::updateChart(bool refresh)
{
    // Update a chart for example if it depends on this cell.
    if ( d->row != 0 && d->column != 0 )
    {
        CellBinding *bind;
        for ( bind = format()->sheet()->firstCellBinding(); bind != 0L; bind = format()->sheet()->nextCellBinding() )
        {
            if ( bind->contains( d->column, d->row ) )
            {
                if (!refresh)
                    return true;

                bind->cellChanged( this );
            }
        }
        return true;
    }
    return false;

}

double Cell::getDouble ()
{
  if (isDefault())
    return 0.0;
  //(Tomas) umm can't we simply call value().asFloat() ?
  if (isDate())
  {
    QDate date = value().asDate();
    QDate dummy (1900, 1, 1);
    return (dummy.daysTo (date) + 1);
  }
  if (isTime())
  {
    QTime time  = value().asTime();
    QTime dummy;
    return dummy.secsTo( time );
  }
  if (value().isNumber())
    return value().asFloat();

  return 0.0;
}

void Cell::convertToDouble ()
{
  if (isDefault())
    return;

  setValue (getDouble ());
}

void Cell::convertToPercent ()
{
  if (isDefault())
    return;

  setValue (getDouble ());
  d->value.setFormat (Value::fmt_Percent);
}

void Cell::convertToMoney ()
{
  if (isDefault())
    return;

  setValue (getDouble ());
  d->value.setFormat (Value::fmt_Money);
  format()->setPrecision (locale()->fracDigits());
}

void Cell::convertToTime ()
{
  //(Tomas) This is weird. And I mean *REALLY* weird. First, we
  //generate a time (QTime), then we convert it to text, then
  //we give the text to the cell and ask it to parse it. Weird...

  if (isDefault() || isEmpty())
    return;

  setValue (getDouble ());
  QTime time = value().asDateTime().time();
  int msec = (int) ( (value().asFloat() - (int) value().asFloat()) * 1000 );
  time = time.addMSecs( msec );
  setCellText( time.toString() );
}

void Cell::convertToDate ()
{
  //(Tomas) This is weird. And I mean *REALLY* weird. First, we
  //generate a date (QDate), then we convert it to text, then
  //we give the text to the cell and ask it to parse it. Weird...

  if (isDefault() || isEmpty())
    return;

  setValue (getDouble ());

  //TODO: why did we call setValue(), when we override it here?
  QDate date(1900, 1, 1);
  date = date.addDays( (int) value().asFloat() - 1 );
  date = value().asDateTime().date();
  setCellText (locale()->formatDate (date, true));
}

void Cell::checkTextInput()
{
  // Goal of this method: determine the value of the cell
  clearAllErrors();

  d->value = Value::empty();

  // Get the text from that cell
  QString str = d->strText;

  sheet()->doc()->parser()->parse (str, this);

  // Parsing as time acts like an autoformat: we even change d->strText
  // [h]:mm:ss -> might get set by ValueParser
  if (isTime() && (formatType() != Time_format7))
    d->strText = locale()->formatTime( value().asDateTime().time(), true);

  // convert first letter to uppercase ?
  if (format()->sheet()->getFirstLetterUpper() && value().isString() &&
      (!d->strText.isEmpty()))
  {
    QString str = value().asString();
    setValue( Value( str[0].upper() + str.right( str.length()-1 ) ) );
  }
}

//used in calc, setNumber, ValueParser
void Cell::checkNumberFormat()
{
    if ( formatType() == Number_format && value().isNumber() )
    {
        if ( value().asFloat() > 1e+10 )
            format()->setFormatType( Scientific_format );
    }
}


// ================================================================
//                       Saving and loading


QDomElement Cell::save( QDomDocument& doc,
             int _x_offset, int _y_offset,
             bool force, bool copy, bool era )
{
    // Save the position of this cell
    QDomElement cell = doc.createElement( "cell" );
    cell.setAttribute( "row", d->row - _y_offset );
    cell.setAttribute( "column", d->column - _x_offset );
    //
    // Save the formatting information
    //
    QDomElement formatElement = format()->save( doc, d->column, d->row, force, copy );
    if ( formatElement.hasChildNodes() || formatElement.attributes().length() ) // don't save empty tags
        cell.appendChild( formatElement );

    if ( doesMergeCells() )
    {
        if ( extraXCells() )
            formatElement.setAttribute( "colspan", extraXCells() );
        if ( extraYCells() )
            formatElement.setAttribute( "rowspan", extraYCells() );
    }

    if ( d->hasExtra() && d->extra()->conditions )
    {
      QDomElement conditionElement = d->extra()->conditions->saveConditions( doc );

      if ( !conditionElement.isNull() )
        cell.appendChild( conditionElement );
    }

    if ( d->hasExtra() && (d->extra()->validity != 0) )
    {
        QDomElement validity = doc.createElement("validity");

        QDomElement param=doc.createElement("param");
        param.setAttribute("cond",(int)d->extra()->validity->m_cond);
        param.setAttribute("action",(int)d->extra()->validity->m_action);
        param.setAttribute("allow",(int)d->extra()->validity->m_restriction);
        param.setAttribute("valmin",d->extra()->validity->valMin);
        param.setAttribute("valmax",d->extra()->validity->valMax);
        param.setAttribute("displaymessage",d->extra()->validity->displayMessage);
        param.setAttribute("displayvalidationinformation",d->extra()->validity->displayValidationInformation);
        param.setAttribute("allowemptycell",d->extra()->validity->allowEmptyCell);
        if ( !d->extra()->validity->listValidity.isEmpty() )
            param.setAttribute( "listvalidity", d->extra()->validity->listValidity.join( ";" ) );
        validity.appendChild(param);
        QDomElement title = doc.createElement( "title" );
        title.appendChild( doc.createTextNode( d->extra()->validity->title ) );
        validity.appendChild( title );
        QDomElement message = doc.createElement( "message" );
        message.appendChild( doc.createCDATASection( d->extra()->validity->message ) );
        validity.appendChild( message );

        QDomElement inputTitle = doc.createElement( "inputtitle" );
        inputTitle.appendChild( doc.createTextNode( d->extra()->validity->titleInfo ) );
        validity.appendChild( inputTitle );

        QDomElement inputMessage = doc.createElement( "inputmessage" );
        inputMessage.appendChild( doc.createTextNode( d->extra()->validity->messageInfo ) );
        validity.appendChild( inputMessage );



        QString tmp;
        if ( d->extra()->validity->timeMin.isValid() )
        {
                QDomElement timeMin = doc.createElement( "timemin" );
                tmp=d->extra()->validity->timeMin.toString();
                timeMin.appendChild( doc.createTextNode( tmp ) );
                validity.appendChild( timeMin );
        }
        if ( d->extra()->validity->timeMax.isValid() )
        {
                QDomElement timeMax = doc.createElement( "timemax" );
                tmp=d->extra()->validity->timeMax.toString();
                timeMax.appendChild( doc.createTextNode( tmp ) );
                validity.appendChild( timeMax );
        }

        if ( d->extra()->validity->dateMin.isValid() )
        {
                QDomElement dateMin = doc.createElement( "datemin" );
                QString tmp("%1/%2/%3");
                tmp = tmp.arg(d->extra()->validity->dateMin.year()).arg(d->extra()->validity->dateMin.month()).arg(d->extra()->validity->dateMin.day());
                dateMin.appendChild( doc.createTextNode( tmp ) );
                validity.appendChild( dateMin );
        }
        if ( d->extra()->validity->dateMax.isValid() )
        {
                QDomElement dateMax = doc.createElement( "datemax" );
                QString tmp("%1/%2/%3");
                tmp = tmp.arg(d->extra()->validity->dateMax.year()).arg(d->extra()->validity->dateMax.month()).arg(d->extra()->validity->dateMax.day());
                dateMax.appendChild( doc.createTextNode( tmp ) );
                validity.appendChild( dateMax );
        }

        cell.appendChild( validity );
    }

    if ( format()->comment() )
    {
        QDomElement comment = doc.createElement( "comment" );
        comment.appendChild( doc.createCDATASection( *format()->comment() ) );
        cell.appendChild( comment );
    }

    //
    // Save the text
    //
    if ( !d->strText.isEmpty() )
    {
        // Formulas need to be encoded to ensure that they
        // are position independent.
        if ( isFormula() )
        {
            QDomElement text = doc.createElement( "text" );
            // if we are cutting to the clipboard, relative references need to be encoded absolutely
            text.appendChild( doc.createTextNode( encodeFormula( era ) ) );
            cell.appendChild( text );

            /* we still want to save the results of the formula */
            QDomElement formulaResult = doc.createElement( "result" );
            saveCellResult( doc, formulaResult, d->strOutText );
            cell.appendChild( formulaResult );

        }
        else if ( !link().isEmpty() )
        {
            // KSpread pre 1.4 saves link as rich text, marked with first char '
            // Have to be saved in some CDATA section because of too many special charatcers.
            QDomElement text = doc.createElement( "text" );
            QString qml = "!<a href=\"" + link() + "\">" + d->strText + "</a>";
            text.appendChild( doc.createCDATASection( qml ) );
            cell.appendChild( text );
        }
        else
        {
            // Save the cell contents (in a locale-independent way)
            QDomElement text = doc.createElement( "text" );
            saveCellResult( doc, text, d->strText );
            cell.appendChild( text );
        }
    }
    if ( cell.hasChildNodes() || cell.attributes().length() > 2 ) // don't save empty tags
        // (the >2 is due to "row" and "column" attributes)
        return cell;
    else
        return QDomElement();
}

bool Cell::saveCellResult( QDomDocument& doc, QDomElement& result,
                                  QString str )
{
  QString dataType = "Other"; // fallback

  if ( value().isNumber() )
  {
      if ( isDate() )
      {
          // serial number of date
          QDate dd = value().asDateTime().date();
          dataType = "Date";
          str = "%1/%2/%3";
          str = str.arg(dd.year()).arg(dd.month()).arg(dd.day());
      }
      else if( isTime() )
      {
          // serial number of time
          dataType = "Time";
          str = value().asDateTime().time().toString();
      }
      else
      {
          // real number
          dataType = "Num";
          if (value().isInteger())
            str = QString::number(value().asInteger());
          else
            str = QString::number(value().asFloat(), 'g', DBL_DIG);
      }
  }

  if ( value().isBoolean() )
  {
      dataType = "Bool";
      str = value().asBoolean() ? "true" : "false";
  }

  if ( value().isString() )
  {
      dataType = "Str";
      str = value().asString();
  }

  result.setAttribute( "dataType", dataType );
  if ( !d->strOutText.isEmpty() )
    result.setAttribute( "outStr", d->strOutText );
  result.appendChild( doc.createTextNode( str ) );

  return true; /* really isn't much of a way for this function to fail */
}

void Cell::saveOasisAnnotation( KoXmlWriter &xmlwriter )
{
    if ( format()->comment() )
    {
        //<office:annotation draw:style-name="gr1" draw:text-style-name="P1" svg:width="2.899cm" svg:height="2.691cm" svg:x="2.858cm" svg:y="0.001cm" draw:caption-point-x="-2.858cm" draw:caption-point-y="-0.001cm">
        xmlwriter.startElement( "office:annotation" );
        QStringList text = QStringList::split( "\n", *format()->comment() );
        for ( QStringList::Iterator it = text.begin(); it != text.end(); ++it ) {
            xmlwriter.startElement( "text:p" );
            xmlwriter.addTextNode( *it );
            xmlwriter.endElement();
        }
        xmlwriter.endElement();
    }
}



QString Cell::saveOasisCellStyle( KoGenStyle &currentCellStyle, KoGenStyles &mainStyles )
{
    if ( d->hasExtra() && d->extra()->conditions )
    {
        // this has to be an automatic style
        currentCellStyle = KoGenStyle( Doc::STYLE_CELL_AUTO, "table-cell" );
        d->extra()->conditions->saveOasisConditions( currentCellStyle );
    }
    return format()->saveOasisCellStyle( currentCellStyle, mainStyles );
}


bool Cell::saveOasis( KoXmlWriter& xmlwriter, KoGenStyles &mainStyles,
                      int row, int column, int &repeated,
                      GenValidationStyles &valStyle )
{
    if ( !isPartOfMerged() )
        xmlwriter.startElement( "table:table-cell" );
    else
        xmlwriter.startElement( "table:covered-table-cell" );
#if 0
    //add font style
    QFont font;
    Value const value( cell->value() );
    if ( !cell->isDefault() )
    {
      font = cell->format()->textFont( i, row );
      m_styles.addFont( font );

      if ( cell->format()->hasProperty( Format::PComment ) )
        hasComment = true;
    }
#endif
    // NOTE save the value before the style as long as the Formatter does not work correctly
    if ( link().isEmpty() )
      saveOasisValue (xmlwriter);

    KoGenStyle currentCellStyle; // the type determined in saveOasisCellStyle
    saveOasisCellStyle( currentCellStyle,mainStyles );
    // skip 'table:style-name' attribute for the default style
    if ( !currentCellStyle.isDefaultStyle() )
      xmlwriter.addAttribute( "table:style-name", mainStyles.styles()[currentCellStyle] );

    // group empty cells with the same style
    if ( isEmpty() && !format()->hasProperty( Format::PComment ) &&
         !isPartOfMerged() && !doesMergeCells() )
    {
      bool refCellIsDefault = isDefault();
      int j = column + 1;
      Cell *nextCell = format()->sheet()->getNextCellRight( column, row );
      while ( nextCell )
      {
        // if
        //   the next cell is not the adjacent one
        // or
        //   the next cell is not empty
        if ( nextCell->column() != j || !nextCell->isEmpty() )
        {
          if ( refCellIsDefault )
          {
            // if the origin cell was a default cell,
            // we count the default cells
            repeated = nextCell->column() - j + 1;
          }
          // otherwise we just stop here to process the adjacent
          // cell in the next iteration of the outer loop
          // (in Sheet::saveOasisCells)
          break;
        }

        KoGenStyle nextCellStyle; // the type is determined in saveOasisCellStyle
        nextCell->saveOasisCellStyle( nextCellStyle,mainStyles );

        if ( nextCell->isPartOfMerged() || nextCell->doesMergeCells() ||
             nextCell->format()->hasProperty( Format::PComment ) ||
             !(nextCellStyle == currentCellStyle) )
        {
          break;
        }
        ++repeated;
        // get the next cell and set the index to the adjacent cell
        nextCell = format()->sheet()->getNextCellRight( j++, row );
      }
      kdDebug() << "Cell::saveOasis: empty cell in column " << column << " "
                << "repeated " << repeated << " time(s)" << endl;

      if ( repeated > 1 )
        xmlwriter.addAttribute( "table:number-columns-repeated", QString::number( repeated ) );
    }


    if (d->hasExtra() && d->extra()->validity)
    {
        GenValidationStyle styleVal(d->extra()->validity);
        xmlwriter.addAttribute( "table:validation-name", valStyle.lookup( styleVal ) );
    }
    if ( isFormula() )
    {
        //kdDebug() << "Formula found" << endl;
      QString formula( convertFormulaToOasisFormat( text() ) );
      xmlwriter.addAttribute( "table:formula", formula );
    }
    else if ( !link().isEmpty() )
    {
        //kdDebug()<<"Link found \n";
        xmlwriter.startElement( "text:p" );
        xmlwriter.startElement( "text:a" );
        //Reference cell is started by "#"
        if ( localReferenceAnchor( link() ) )
            xmlwriter.addAttribute( " xlink:href", ( "#"+link() ) );
        else
            xmlwriter.addAttribute( " xlink:href", link() );
        xmlwriter.addTextNode( text() );
        xmlwriter.endElement();
        xmlwriter.endElement();
    }

    if ( doesMergeCells() )
    {
      int colSpan = mergedXCells() + 1;
      int rowSpan = mergedYCells() + 1;

      if ( colSpan > 1 )
        xmlwriter.addAttribute( "table:number-columns-spanned", QString::number( colSpan ) );

      if ( rowSpan > 1 )
        xmlwriter.addAttribute( "table:number-rows-spanned", QString::number( rowSpan ) );
    }

    if ( !isEmpty() && link().isEmpty() )
    {
        xmlwriter.startElement( "text:p" );
        xmlwriter.addTextNode( strOutText().utf8() );
        xmlwriter.endElement();
    }

    saveOasisAnnotation( xmlwriter );

    xmlwriter.endElement();
    return true;
}

void Cell::saveOasisValue (KoXmlWriter &xmlWriter)
{
  switch (value().format())
  {
    case Value::fmt_None: break;  //NOTHING HERE
    case Value::fmt_Boolean:
    {
      xmlWriter.addAttribute( "office:value-type", "boolean" );
      xmlWriter.addAttribute( "office:boolean-value", ( value().asBoolean() ?
          "true" : "false" ) );
      break;
    }
    case Value::fmt_Number:
    {
      xmlWriter.addAttribute( "office:value-type", "float" );
      if (value().isInteger())
        xmlWriter.addAttribute( "office:value", QString::number( value().asInteger() ) );
      else
        xmlWriter.addAttribute( "office:value", QString::number( value().asFloat(), 'g', DBL_DIG ) );
      break;
    }
    case Value::fmt_Percent:
    {
      xmlWriter.addAttribute( "office:value-type", "percentage" );
      xmlWriter.addAttribute( "office:value",
          QString::number( value().asFloat() ) );
      break;
    }
    case Value::fmt_Money:
    {
      xmlWriter.addAttribute( "office:value-type", "currency" );
      Format::Currency currency;
      if (format()->currencyInfo(currency))
        xmlWriter.addAttribute( "office:currency", Currency::getCurrencyCode(currency.type) );
      xmlWriter.addAttribute( "office:value",
          QString::number( value().asFloat() ) );
      break;
    }
    case Value::fmt_DateTime: break;  //NOTHING HERE
    case Value::fmt_Date:
    {
      xmlWriter.addAttribute( "office:value-type", "date" );
      xmlWriter.addAttribute( "office:date-value",
          value().asDate().toString( Qt::ISODate ) );
      break;
    }
    case Value::fmt_Time:
    {
      xmlWriter.addAttribute( "office:value-type", "time" );
      xmlWriter.addAttribute( "office:time-value",
          value().asTime().toString( "PThhHmmMssS" ) );
      break;
    }
    case Value::fmt_String:
    {
      xmlWriter.addAttribute( "office:value-type", "string" );
      xmlWriter.addAttribute( "office:string-value", value().asString() );
      break;
    }
  };
}

QString Cell::convertFormulaToOasisFormat( const QString & formula ) const
{
    QString s;
    QRegExp exp("(\\$?)([a-zA-Z]+)(\\$?)([0-9]+)");
    int n = exp.search( formula, 0 );
    kdDebug() << "Exp: " << formula << ", n: " << n << ", Length: " << formula.length()
              << ", Matched length: " << exp.matchedLength() << endl;

    bool inQuote1 = false;
    bool inQuote2 = false;
    int i = 0;
    int l = (int) formula.length();
    if ( l <= 0 )
        return formula;
    while ( i < l )
    {
        if ( ( n != -1 ) && ( n < i ) )
        {
            n = exp.search( formula, i );
            kdDebug() << "Exp: " << formula.right( l - i ) << ", n: " << n << endl;
        }
        if ( formula[i] == '"' )
        {
            inQuote1 = !inQuote1;
            s += formula[i];
            ++i;
            continue;
        }
        if ( formula[i] == '\'' )
        {
            // named area
            inQuote2 = !inQuote2;
            ++i;
            continue;
        }
        if ( inQuote1 || inQuote2 )
        {
            s += formula[i];
            ++i;
            continue;
        }
        if ( ( formula[i] == '=' ) && ( formula[i + 1] == '=' ) )
        {
            s += '=';
            ++i;++i;
            continue;
        }
        if ( formula[i] == '!' )
        {
            insertBracket( s );
            s += '.';
            ++i;
            continue;
        }
        if ( formula[i] == ',' )
        {
            s += '.';
            ++i;
            continue;
        }
        if ( n == i )
        {
            int ml = exp.matchedLength();
            if ( formula[ i + ml ] == '!' )
            {
                kdDebug() << "No cell ref but sheet name" << endl;
                s += formula[i];
                ++i;
                continue;
            }
            if ( ( i > 0 ) && ( formula[i - 1] != '!' ) )
                s += "[.";
            for ( int j = 0; j < ml; ++j )
            {
                s += formula[i];
                ++i;
            }
            s += ']';
            continue;
        }

        s += formula[i];
        ++i;
    }

    return s;
}

void Cell::loadOasisConditional( QDomElement * style )
{
    if ( style )//safe
    {
        QDomElement e;
        forEachElement( e, style->toElement() )
        {
            if ( e.localName() == "map" && e.namespaceURI() == KoXmlNS::style )
            {
                if (d->hasExtra())
                    delete d->extra()->conditions;
                d->extra()->conditions = new Conditions( this );
                d->extra()->conditions->loadOasisConditions( e );
                d->extra()->conditions->checkMatches();
                // break here
                // Conditions::loadOasisConditions finishes the iteration
                break;
            }
        }
    }
}

bool Cell::loadOasis( const QDomElement& element , KoOasisLoadingContext& oasisContext , Style* style )
{
    kdDebug() << "*** Loading cell properties ***** at " << column() << "," << row () << endl;

    if ( element.hasAttributeNS( KoXmlNS::table, "style-name" ) )
    {
        kdDebug()<<" table:style-name: "<<element.attributeNS( KoXmlNS::table, "style-name", QString::null )<<endl;
        oasisContext.fillStyleStack( element, KoXmlNS::table, "styleName", "table-cell" );

        QString str = element.attributeNS( KoXmlNS::table, "style-name", QString::null );
        const QDomElement* cellStyle = oasisContext.oasisStyles().findStyle( str, "table-cell" );

        if ( cellStyle )
            loadOasisConditional( const_cast<QDomElement *>( cellStyle ) );
    }

    if (style)
    {
        format()->setStyle( style );
    }

    //Search and load each paragraph of text. Each paragraph is separated by a line break.
    loadOasisCellText( element );

    //
    // formula
    //
    bool isFormula = false;
    if ( element.hasAttributeNS( KoXmlNS::table, "formula" ) )
    {
        kdDebug()<<" formula :"<<element.attributeNS( KoXmlNS::table, "formula", QString::null )<<endl;
        isFormula = true;
        QString oasisFormula( element.attributeNS( KoXmlNS::table, "formula", QString::null ) );
        //necessary to remove it to load formula from oocalc2.0 (use namespace)
        if (oasisFormula.startsWith( "oooc:" ) )
            oasisFormula= oasisFormula.mid( 5 );
        else if (oasisFormula.startsWith( "kspr:" ) )
            oasisFormula= oasisFormula.mid( 5 );
        // TODO Stefan: merge this into Oasis::decodeFormula
        checkForNamedAreas( oasisFormula );
        oasisFormula = Oasis::decodeFormula( oasisFormula, locale() );
        setCellText( oasisFormula );
    }
    else if ( d->strText.at(0) == '=' ) //prepend ' to the text to avoid = to be painted
        d->strText.prepend('\'');

    //
    // validation
    //
    if ( element.hasAttributeNS( KoXmlNS::table, "validation-name" ) )
    {
        kdDebug()<<" validation-name: "<<element.attributeNS( KoXmlNS::table, "validation-name", QString::null )<<endl;
        loadOasisValidation( element.attributeNS( KoXmlNS::table, "validation-name", QString::null ) );
    }

    //
    // value type
    //
    if( element.hasAttributeNS( KoXmlNS::office, "value-type" ) )
    {
        QString valuetype = element.attributeNS( KoXmlNS::office, "value-type", QString::null );
        kdDebug()<<"  value-type: " << valuetype << endl;
        if( valuetype == "boolean" )
        {
          QString val = element.attributeNS( KoXmlNS::office, "boolean-value", QString::null ).lower();
            if( ( val == "true" ) || ( val == "false" ) )
            {
                bool value = val == "true";
                setCellValue( value );
            }
        }

        // integer and floating-point value
        else if( valuetype == "float" )
        {
            bool ok = false;
            double value = element.attributeNS( KoXmlNS::office, "value", QString::null ).toDouble( &ok );
            if( ok )
                setCellValue( value );

            if ( !isFormula && d->strText.isEmpty())
            {
                QString str = locale()->formatNumber( value, 15 );
                setCellText( str );
            }
        }

        // currency value
        else if( valuetype == "currency" )
        {
            bool ok = false;
            double value = element.attributeNS( KoXmlNS::office, "value", QString::null ).toDouble( &ok );
            if( ok )
            {
                setCellValue( value, Money_format );

                if (element.hasAttributeNS( KoXmlNS::office, "currency" ) )
                {
                  Currency currency(element.attributeNS( KoXmlNS::office, "currency", QString::null ) );
                  format()->setCurrency( currency.getIndex(), currency.getDisplayCode() );
                }
            }
        }
        else if( valuetype == "percentage" )
        {
            bool ok = false;
            double v = element.attributeNS( KoXmlNS::office, "value", QString::null ).toDouble( &ok );
            if( ok )
            {
                Value value;
                value.setValue (v);
                value.setFormat (Value::fmt_Percent);
                setCellValue( value );

                if ( !isFormula && d->strText.isEmpty())
                {
                    QString str = locale()->formatNumber( v, 15 );
                    setCellText( str );
                }

                format()->setFormatType (Percentage_format);
            }
        }
        else if ( valuetype == "date" )
        {
            QString value = element.attributeNS( KoXmlNS::office, "value", QString::null );
            if ( value.isEmpty() )
                value = element.attributeNS( KoXmlNS::office, "date-value", QString::null );
            kdDebug() << "Type: date, value: " << value << endl;

            // "1980-10-15"
            int year = 0, month = 0, day = 0;
            bool ok = false;

            int p1 = value.find( '-' );
            if ( p1 > 0 )
                year  = value.left( p1 ).toInt( &ok );

            kdDebug() << "year: " << value.left( p1 ) << endl;

            int p2 = value.find( '-', ++p1 );

            if ( ok )
                month = value.mid( p1, p2 - p1  ).toInt( &ok );

            kdDebug() << "month: " << value.mid( p1, p2 - p1 ) << endl;

            if ( ok )
                day = value.right( value.length() - p2 - 1 ).toInt( &ok );

            kdDebug() << "day: " << value.right( value.length() - p2 ) << endl;

            if ( ok )
            {
                setCellValue( QDate( year, month, day ) );
                if ( style )
                    format()->setFormatType (style->formatType());
                kdDebug() << "Set QDate: " << year << " - " << month << " - " << day << endl;
            }

        }
        else if ( valuetype == "time" )
        {
            QString value = element.attributeNS( KoXmlNS::office, "value", QString::null );
            if ( value.isEmpty() )
                value = element.attributeNS( KoXmlNS::office, "time-value", QString::null );
            kdDebug() << "Type: time: " << value << endl;
            // "PT15H10M12S"
            int hours = 0, minutes = 0, seconds = 0;
            int l = value.length();
            QString num;
            bool ok = false;
            for ( int i = 0; i < l; ++i )
            {
                if ( value[i].isNumber() )
                {
                    num += value[i];
                    continue;
                }
                else if ( value[i] == 'H' )
                    hours   = num.toInt( &ok );
                else if ( value[i] == 'M' )
                    minutes = num.toInt( &ok );
                else if ( value[i] == 'S' )
                    seconds = num.toInt( &ok );
                else
                    continue;

                kdDebug() << "Num: " << num << endl;

                num = "";
                if ( !ok )
                    break;
            }
            kdDebug() << "Hours: " << hours << ", " << minutes << ", " << seconds << endl;

            if ( ok )
            {
                // Value kval( timeToNum( hours, minutes, seconds ) );
                // cell->setValue( kval );
                setCellValue( QTime( hours % 24, minutes, seconds ) );
                if ( style )
                    format()->setFormatType (style->formatType());
            }
        }
        else if( valuetype == "string" )
        {
            QString value = element.attributeNS( KoXmlNS::office, "value", QString::null );
            if ( value.isEmpty() && element.hasAttributeNS( KoXmlNS::office, "string-value" ))
            {
                //if there is not string-value entry don't overwrite value stored into <text:p>
                value = element.attributeNS( KoXmlNS::office, "string-value", QString::null );
                setCellValue( value );
            }
            format()->setFormatType (Text_format);
        }
        else
            kdDebug()<<" type of value found : "<<valuetype<<endl;
    }

    //
    // merged cells ?
    //
    int colSpan = 1;
    int rowSpan = 1;
    if ( element.hasAttributeNS( KoXmlNS::table, "number-columns-spanned" ) )
    {
        bool ok = false;
        int span = element.attributeNS( KoXmlNS::table, "number-columns-spanned", QString::null ).toInt( &ok );
        if( ok ) colSpan = span;
    }
    if ( element.hasAttributeNS( KoXmlNS::table, "number-rows-spanned" ) )
    {
        bool ok = false;
        int span = element.attributeNS( KoXmlNS::table, "number-rows-spanned", QString::null ).toInt( &ok );
        if( ok ) rowSpan = span;
    }
    if ( colSpan > 1 || rowSpan > 1 )
        mergeCells( d->column, d->row, colSpan - 1, rowSpan - 1 );

    //
    // cell comment/annotation
    //
    QDomElement annotationElement = KoDom::namedItemNS( element, KoXmlNS::office, "annotation" );
    if ( !annotationElement.isNull() )
    {
        QString comment;
        QDomNode node = annotationElement.firstChild();
        while( !node.isNull() )
        {
            QDomElement commentElement = node.toElement();
            if( !commentElement.isNull() )
                if( commentElement.localName() == "p" && commentElement.namespaceURI() == KoXmlNS::text )
                {
                    if( !comment.isEmpty() ) comment.append( '\n' );
                    comment.append( commentElement.text() );
                }

            node = node.nextSibling();
        }

        if( !comment.isEmpty() )
            format()->setComment( comment );
    }

    QDomElement frame = KoDom::namedItemNS( element, KoXmlNS::draw, "frame" );
    if ( !frame.isNull() )
      loadOasisObjects( frame, oasisContext );

    if (isFormula)
      setCalcDirtyFlag ();   // formulas must be recalculated

    return true;
}

void Cell::loadOasisCellText( const QDomElement& parent )
{
    //Search and load each paragraph of text. Each paragraph is separated by a line break
    QDomElement textParagraphElement;
    QString cellText;

    bool multipleTextParagraphsFound=false;

    forEachElement( textParagraphElement , parent )
    {
        if ( textParagraphElement.localName()=="p" &&
             textParagraphElement.namespaceURI()== KoXmlNS::text )
        {
            // our text, could contain formating for value or result of formul
            if (cellText.isEmpty())
                cellText = textParagraphElement.text();
            else
            {
                cellText += "\n"+textParagraphElement.text();
                multipleTextParagraphsFound=true;
            }

            QDomElement textA = KoDom::namedItemNS( textParagraphElement, KoXmlNS::text, "a" );
            if( !textA.isNull() )
            {
                if ( textA.hasAttributeNS( KoXmlNS::xlink, "href" ) )
                {
                    QString link = textA.attributeNS( KoXmlNS::xlink, "href", QString::null );
                    cellText = textA.text();
                    setCellText( cellText );
                    setValue( cellText );
                    if ( link[0]=='#' )
                        link=link.remove( 0, 1 );
                    setLink( link );
                }
            }
        }
    }

    if (!cellText.isNull())
    {
        setCellText( cellText );
        setValue( cellText );
    }

    //Enable word wrapping if multiple lines of text have been found.
    if ( multipleTextParagraphsFound )
    {
        format()->setMultiRow(true);
    }
}

void Cell::loadOasisObjects( const QDomElement &parent, KoOasisLoadingContext& oasisContext )
{
    for( QDomElement e = parent; !e.isNull(); e = e.nextSibling().toElement() )
    {
        if ( e.localName() == "frame" && e.namespaceURI() == KoXmlNS::draw )
        {
          EmbeddedObject *obj = 0;
          QDomNode object = KoDom::namedItemNS( e, KoXmlNS::draw, "object" );
          if ( !object.isNull() )
          {
            if ( !object.toElement().attributeNS( KoXmlNS::draw, "notify-on-update-of-ranges", QString::null).isNull() )
              obj = new EmbeddedChart( sheet()->doc(), sheet() );
            else
              obj = new EmbeddedKOfficeObject( sheet()->doc(), sheet() );
          }
          else
          {
            QDomNode image = KoDom::namedItemNS( e, KoXmlNS::draw, "image" );
            if ( !image.isNull() )
              obj = new EmbeddedPictureObject( sheet(), sheet()->doc()->pictureCollection() );
            else
              kdDebug() << "Object type wasn't loaded!" << endl;
          }

          if ( obj )
          {
            obj->loadOasis( e, oasisContext );
            sheet()->doc()->insertObject( obj );

            QString ref = e.attributeNS( KoXmlNS::table, "end-cell-address", QString::null );
            if ( ref.isNull() )
              continue;

            ref = Oasis::decodeFormula( ref );
            Point point( ref );
            if ( !point.isValid() )
              continue;

            KoRect geometry = obj->geometry();
            geometry.setLeft( geometry.left() + sheet()->columnPos( d->column, 0 ) );
            geometry.setTop( geometry.top() + sheet()->rowPos( d->row, 0 ) );

            QString str = e.attributeNS( KoXmlNS::table, "end-x", QString::null );
            if ( !str.isNull() )
            {
              uint end_x = (uint) KoUnit::parseValue( str );
              geometry.setRight( sheet()->columnPos( point.column(), 0) + end_x );
            }

            str = e.attributeNS( KoXmlNS::table, "end-y", QString::null );
            if ( !str.isNull() )
            {
              uint end_y = (uint) KoUnit::parseValue( str );
              geometry.setBottom( sheet()->rowPos( point.row(), 0) + end_y );
            }

            obj->setGeometry( geometry );
          }
        }
    }
}

void Cell::loadOasisValidation( const QString& validationName )
{
    QDomElement element = sheet()->doc()->loadingInfo()->validation( validationName);
    if (d->hasExtra())
      delete d->extra()->validity;
    d->extra()->validity = new Validity;
    if ( element.hasAttributeNS( KoXmlNS::table, "condition" ) )
    {
        QString valExpression = element.attributeNS( KoXmlNS::table, "condition", QString::null );
        kdDebug()<<" element.attribute( table:condition ) "<<valExpression<<endl;
        //Condition ::= ExtendedTrueCondition | TrueFunction 'and' TrueCondition
        //TrueFunction ::= cell-content-is-whole-number() | cell-content-is-decimal-number() | cell-content-is-date() | cell-content-is-time()
        //ExtendedTrueCondition ::= ExtendedGetFunction | cell-content-text-length() Operator Value
        //TrueCondition ::= GetFunction | cell-content() Operator Value
        //GetFunction ::= cell-content-is-between(Value, Value) | cell-content-is-not-between(Value, Value)
        //ExtendedGetFunction ::= cell-content-text-length-is-between(Value, Value) | cell-content-text-length-is-not-between(Value, Value)
        //Operator ::= '<' | '>' | '<=' | '>=' | '=' | '!='
        //Value ::= NumberValue | String | Formula
        //A Formula is a formula without an equals (=) sign at the beginning. See section 8.1.3 for more information.
        //A String comprises one or more characters surrounded by quotation marks.
        //A NumberValue is a whole or decimal number. It must not contain comma separators for numbers of 1000 or greater.

        //ExtendedTrueCondition
        if ( valExpression.contains( "cell-content-text-length()" ) )
        {
            //"cell-content-text-length()>45"
            valExpression = valExpression.remove("oooc:cell-content-text-length()" );
            kdDebug()<<" valExpression = :"<<valExpression<<endl;
            d->extra()->validity->m_restriction = Restriction::TextLength;

            loadOasisValidationCondition( valExpression );
        }
        else if ( valExpression.contains( "cell-content-is-text()" ) )
        {
            d->extra()->validity->m_restriction = Restriction::Text;
        }
        //cell-content-text-length-is-between(Value, Value) | cell-content-text-length-is-not-between(Value, Value) | cell-content-is-in-list( StringList )
        else if ( valExpression.contains( "cell-content-text-length-is-between" ) )
        {
            d->extra()->validity->m_restriction = Restriction::TextLength;
            d->extra()->validity->m_cond = Conditional::Between;
            valExpression = valExpression.remove( "oooc:cell-content-text-length-is-between(" );
            kdDebug()<<" valExpression :"<<valExpression<<endl;
            valExpression = valExpression.remove( ")" );
            QStringList listVal = QStringList::split( ",", valExpression );
            loadOasisValidationValue( listVal );
        }
        else if ( valExpression.contains( "cell-content-text-length-is-not-between" ) )
        {
            d->extra()->validity->m_restriction = Restriction::TextLength;
            d->extra()->validity->m_cond = Conditional::Different;
            valExpression = valExpression.remove( "oooc:cell-content-text-length-is-not-between(" );
            kdDebug()<<" valExpression :"<<valExpression<<endl;
            valExpression = valExpression.remove( ")" );
            kdDebug()<<" valExpression :"<<valExpression<<endl;
            QStringList listVal = QStringList::split( ",", valExpression );
            loadOasisValidationValue( listVal );
        }
        else if ( valExpression.contains( "cell-content-is-in-list(" ) )
        {
            d->extra()->validity->m_restriction = Restriction::List;
            valExpression = valExpression.remove( "oooc:cell-content-is-in-list(" );
            kdDebug()<<" valExpression :"<<valExpression<<endl;
            valExpression = valExpression.remove( ")" );
            d->extra()->validity->listValidity = QStringList::split( ";", valExpression );

        }
        //TrueFunction ::= cell-content-is-whole-number() | cell-content-is-decimal-number() | cell-content-is-date() | cell-content-is-time()
        else
        {
            if (valExpression.contains( "cell-content-is-whole-number()" ) )
            {
                d->extra()->validity->m_restriction =  Restriction::Number;
                valExpression = valExpression.remove( "oooc:cell-content-is-whole-number() and " );
            }
            else if (valExpression.contains( "cell-content-is-decimal-number()" ) )
            {
                d->extra()->validity->m_restriction = Restriction::Integer;
                valExpression = valExpression.remove( "oooc:cell-content-is-decimal-number() and " );
            }
            else if (valExpression.contains( "cell-content-is-date()" ) )
            {
                d->extra()->validity->m_restriction = Restriction::Date;
                valExpression = valExpression.remove( "oooc:cell-content-is-date() and " );
            }
            else if (valExpression.contains( "cell-content-is-time()" ) )
            {
                d->extra()->validity->m_restriction = Restriction::Time;
                valExpression = valExpression.remove( "oooc:cell-content-is-time() and " );
            }
            kdDebug()<<"valExpression :"<<valExpression<<endl;

            if ( valExpression.contains( "cell-content()" ) )
            {
                valExpression = valExpression.remove( "cell-content()" );
                loadOasisValidationCondition( valExpression );
            }
            //GetFunction ::= cell-content-is-between(Value, Value) | cell-content-is-not-between(Value, Value)
            //for the moment we support just int/double value, not text/date/time :(
            if ( valExpression.contains( "cell-content-is-between(" ) )
            {
                valExpression = valExpression.remove( "cell-content-is-between(" );
                valExpression = valExpression.remove( ")" );
                QStringList listVal = QStringList::split( "," , valExpression );
                loadOasisValidationValue( listVal );
                d->extra()->validity->m_cond = Conditional::Between;
            }
            if ( valExpression.contains( "cell-content-is-not-between(" ) )
            {
                valExpression = valExpression.remove( "cell-content-is-not-between(" );
                valExpression = valExpression.remove( ")" );
                QStringList listVal = QStringList::split( ",", valExpression );
                loadOasisValidationValue( listVal );
                d->extra()->validity->m_cond = Conditional::Different;
            }
        }
    }
    if ( element.hasAttributeNS( KoXmlNS::table, "allow-empty-cell" ) )
    {
        kdDebug()<<" element.hasAttribute( table:allow-empty-cell ) :"<<element.hasAttributeNS( KoXmlNS::table, "allow-empty-cell" )<<endl;
        d->extra()->validity->allowEmptyCell = ( ( element.attributeNS( KoXmlNS::table, "allow-empty-cell", QString::null )=="true" ) ? true : false );
    }
    if ( element.hasAttributeNS( KoXmlNS::table, "base-cell-address" ) )
    {
        //todo what is it ?
    }

    QDomElement help = KoDom::namedItemNS( element, KoXmlNS::table, "help-message" );
    if ( !help.isNull() )
    {
        if ( help.hasAttributeNS( KoXmlNS::table, "title" ) )
        {
            kdDebug()<<"help.attribute( table:title ) :"<<help.attributeNS( KoXmlNS::table, "title", QString::null )<<endl;
            d->extra()->validity->titleInfo = help.attributeNS( KoXmlNS::table, "title", QString::null );
        }
        if ( help.hasAttributeNS( KoXmlNS::table, "display" ) )
        {
            kdDebug()<<"help.attribute( table:display ) :"<<help.attributeNS( KoXmlNS::table, "display", QString::null )<<endl;
            d->extra()->validity->displayValidationInformation = ( ( help.attributeNS( KoXmlNS::table, "display", QString::null )=="true" ) ? true : false );
        }
        QDomElement attrText = KoDom::namedItemNS( help, KoXmlNS::text, "p" );
        if ( !attrText.isNull() )
        {
            kdDebug()<<"help text :"<<attrText.text()<<endl;
            d->extra()->validity->messageInfo = attrText.text();
        }
    }

    QDomElement error = KoDom::namedItemNS( element, KoXmlNS::table, "error-message" );
    if ( !error.isNull() )
    {
        if ( error.hasAttributeNS( KoXmlNS::table, "title" ) )
            d->extra()->validity->title = error.attributeNS( KoXmlNS::table, "title", QString::null );
        if ( error.hasAttributeNS( KoXmlNS::table, "message-type" ) )
        {
            QString str = error.attributeNS( KoXmlNS::table, "message-type", QString::null );
            if ( str == "warning" )
              d->extra()->validity->m_action = Action::Warning;
            else if ( str == "information" )
              d->extra()->validity->m_action = Action::Information;
            else if ( str == "stop" )
              d->extra()->validity->m_action = Action::Stop;
            else
                kdDebug()<<"validation : message type unknown  :"<<str<<endl;
        }

        if ( error.hasAttributeNS( KoXmlNS::table, "display" ) )
        {
            kdDebug()<<" display message :"<<error.attributeNS( KoXmlNS::table, "display", QString::null )<<endl;
            d->extra()->validity->displayMessage = (error.attributeNS( KoXmlNS::table, "display", QString::null )=="true");
        }
        QDomElement attrText = KoDom::namedItemNS( error, KoXmlNS::text, "p" );
        if ( !attrText.isNull() )
            d->extra()->validity->message = attrText.text();
    }
}


void Cell::loadOasisValidationValue( const QStringList &listVal )
{
    bool ok = false;
    kdDebug()<<" listVal[0] :"<<listVal[0]<<" listVal[1] :"<<listVal[1]<<endl;

    if ( d->extra()->validity->m_restriction == Restriction::Date )
    {
        d->extra()->validity->dateMin = QDate::fromString( listVal[0] );
        d->extra()->validity->dateMax = QDate::fromString( listVal[1] );
    }
    else if ( d->extra()->validity->m_restriction == Restriction::Time )
    {
        d->extra()->validity->timeMin = QTime::fromString( listVal[0] );
        d->extra()->validity->timeMax = QTime::fromString( listVal[1] );
    }
    else
    {
        d->extra()->validity->valMin = listVal[0].toDouble(&ok);
        if ( !ok )
        {
            d->extra()->validity->valMin = listVal[0].toInt(&ok);
            if ( !ok )
                kdDebug()<<" Try to parse this value :"<<listVal[0]<<endl;

#if 0
            if ( !ok )
                d->extra()->validity->valMin = listVal[0];
#endif
        }
        ok=false;
        d->extra()->validity->valMax = listVal[1].toDouble(&ok);
        if ( !ok )
        {
            d->extra()->validity->valMax = listVal[1].toInt(&ok);
            if ( !ok )
                kdDebug()<<" Try to parse this value :"<<listVal[1]<<endl;

#if 0
            if ( !ok )
                d->extra()->validity->valMax = listVal[1];
#endif
        }
    }
}

void Cell::loadOasisValidationCondition( QString &valExpression )
{
    QString value;
    if (valExpression.find( "<=" )==0 )
    {
        value = valExpression.remove( 0,2 );
        d->extra()->validity->m_cond = Conditional::InferiorEqual;
    }
    else if (valExpression.find( ">=" )==0 )
    {
        value = valExpression.remove( 0,2 );
        d->extra()->validity->m_cond = Conditional::SuperiorEqual;
    }
    else if (valExpression.find( "!=" )==0 )
    {
        //add Differentto attribute
        value = valExpression.remove( 0,2 );
        d->extra()->validity->m_cond = Conditional::DifferentTo;
    }
    else if ( valExpression.find( "<" )==0 )
    {
        value = valExpression.remove( 0,1 );
        d->extra()->validity->m_cond = Conditional::Inferior;
    }
    else if(valExpression.find( ">" )==0 )
    {
        value = valExpression.remove( 0,1 );
        d->extra()->validity->m_cond = Conditional::Superior;
    }
    else if (valExpression.find( "=" )==0 )
    {
        value = valExpression.remove( 0,1 );
        d->extra()->validity->m_cond = Conditional::Equal;
    }
    else
        kdDebug()<<" I don't know how to parse it :"<<valExpression<<endl;
    if ( d->extra()->validity->m_restriction == Restriction::Date )
    {
        d->extra()->validity->dateMin = QDate::fromString( value );
    }
    else if (d->extra()->validity->m_restriction == Restriction::Date )
    {
        d->extra()->validity->timeMin = QTime::fromString( value );
    }
    else
    {
        bool ok = false;
        d->extra()->validity->valMin = value.toDouble(&ok);
        if ( !ok )
        {
            d->extra()->validity->valMin = value.toInt(&ok);
            if ( !ok )
                kdDebug()<<" Try to parse this value :"<<value<<endl;

#if 0
            if ( !ok )
                d->extra()->validity->valMin = value;
#endif
        }
    }
}


bool Cell::load( const QDomElement & cell, int _xshift, int _yshift,
                 Paste::Mode pm, Paste::Operation op, bool paste )
{
    bool ok;

    //
    // First of all determine in which row and column this
    // cell belongs.
    //
    d->row = cell.attribute( "row" ).toInt( &ok ) + _yshift;
    if ( !ok ) return false;
    d->column = cell.attribute( "column" ).toInt( &ok ) + _xshift;
    if ( !ok ) return false;

    // Validation
    if ( d->row < 1 || d->row > KS_rowMax )
    {
        kdDebug(36001) << "Cell::load: Value out of Range Cell:row=" << d->row << endl;
        return false;
    }
    if ( d->column < 1 || d->column > KS_colMax )
    {
        kdDebug(36001) << "Cell::load: Value out of Range Cell:column=" << d->column << endl;
        return false;
    }

    //
    // Load formatting information.
    //
    QDomElement f = cell.namedItem( "format" ).toElement();
    if ( !f.isNull()
          && ( (pm == Paste::Normal) || (pm == Paste::Format) || (pm == Paste::NoBorder) ) )
    {
        // send pm parameter. Didn't load Borders if pm==NoBorder

      if ( !format()->load( f, pm, paste ) )
            return false;

        if ( f.hasAttribute( "colspan" ) )
        {
            int i = f.attribute("colspan").toInt( &ok );
            if ( !ok ) return false;
            // Validation
            if ( i < 0 || i > KS_spanMax )
            {
                kdDebug(36001) << "Value out of range Cell::colspan=" << i << endl;
                return false;
            }
            if (i || d->hasExtra())
              d->extra()->extraXCells = i;
            if ( i > 0 )
            {
              setFlag(Flag_Merged);
            }
        }

        if ( f.hasAttribute( "rowspan" ) )
        {
            int i = f.attribute("rowspan").toInt( &ok );
            if ( !ok ) return false;
            // Validation
            if ( i < 0 || i > KS_spanMax )
            {
                kdDebug(36001) << "Value out of range Cell::rowspan=" << i << endl;
                return false;
            }
            if (i || d->hasExtra())
              d->extra()->extraYCells = i;
            if ( i > 0 )
            {
              setFlag(Flag_Merged);
            }
        }

        if ( testFlag( Flag_Merged ) )
        {
            if (d->hasExtra())
              mergeCells( d->column, d->row, d->extra()->extraXCells, d->extra()->extraYCells );
        }

    }

    //
    // Load the condition section of a cell.
    //
    QDomElement conditionsElement = cell.namedItem( "condition" ).toElement();
    if ( !conditionsElement.isNull())
    {
      if (d->hasExtra())
        delete d->extra()->conditions;
      d->extra()->conditions = new Conditions( this );
      d->extra()->conditions->loadConditions( conditionsElement );
      d->extra()->conditions->checkMatches();
    }
    else if ((pm == Paste::Normal) || (pm == Paste::NoBorder))
    {
      //clear the conditional formatting
      if (d->hasExtra())
      {
        delete d->extra()->conditions;
        d->extra()->conditions = 0;
      }
    }

    QDomElement validity = cell.namedItem( "validity" ).toElement();
    if ( !validity.isNull())
    {
        QDomElement param = validity.namedItem( "param" ).toElement();
        if(!param.isNull())
        {
          d->extra()->validity = new Validity;
          if ( param.hasAttribute( "cond" ) )
          {
            d->extra()->validity->m_cond = (Conditional::Type) param.attribute("cond").toInt( &ok );
            if ( !ok )
              return false;
          }
          if ( param.hasAttribute( "action" ) )
          {
            d->extra()->validity->m_action = (Action::Type) param.attribute("action").toInt( &ok );
            if ( !ok )
              return false;
          }
          if ( param.hasAttribute( "allow" ) )
          {
            d->extra()->validity->m_restriction = (Restriction::Type) param.attribute("allow").toInt( &ok );
            if ( !ok )
              return false;
          }
          if ( param.hasAttribute( "valmin" ) )
          {
            d->extra()->validity->valMin = param.attribute("valmin").toDouble( &ok );
            if ( !ok )
              return false;
          }
          if ( param.hasAttribute( "valmax" ) )
          {
            d->extra()->validity->valMax = param.attribute("valmax").toDouble( &ok );
            if ( !ok )
              return false;
          }
          if ( param.hasAttribute( "displaymessage" ) )
          {
              d->extra()->validity->displayMessage = ( bool )param.attribute("displaymessage").toInt();
          }
          if ( param.hasAttribute( "displayvalidationinformation" ) )
          {
              d->extra()->validity->displayValidationInformation = ( bool )param.attribute("displayvalidationinformation").toInt();
          }
          if ( param.hasAttribute( "allowemptycell" ) )
          {
              d->extra()->validity->allowEmptyCell = ( bool )param.attribute("allowemptycell").toInt();
          }
          if ( param.hasAttribute("listvalidity") )
          {
              d->extra()->validity->listValidity=QStringList::split(";", param.attribute("listvalidity") );
          }
        }
        QDomElement inputTitle = validity.namedItem( "inputtitle" ).toElement();
        if (!inputTitle.isNull())
        {
            d->extra()->validity->titleInfo = inputTitle.text();
        }
        QDomElement inputMessage = validity.namedItem( "inputmessage" ).toElement();
        if (!inputMessage.isNull())
        {
            d->extra()->validity->messageInfo = inputMessage.text();
        }

        QDomElement title = validity.namedItem( "title" ).toElement();
        if (!title.isNull())
        {
            d->extra()->validity->title = title.text();
        }
        QDomElement message = validity.namedItem( "message" ).toElement();
        if (!message.isNull())
        {
            d->extra()->validity->message = message.text();
        }
        QDomElement timeMin = validity.namedItem( "timemin" ).toElement();
        if ( !timeMin.isNull()  )
        {
            d->extra()->validity->timeMin = toTime(timeMin);
        }
        QDomElement timeMax = validity.namedItem( "timemax" ).toElement();
        if ( !timeMax.isNull()  )
        {
            d->extra()->validity->timeMax = toTime(timeMax);
         }
        QDomElement dateMin = validity.namedItem( "datemin" ).toElement();
        if ( !dateMin.isNull()  )
        {
            d->extra()->validity->dateMin = toDate(dateMin);
         }
        QDomElement dateMax = validity.namedItem( "datemax" ).toElement();
        if ( !dateMax.isNull()  )
        {
            d->extra()->validity->dateMax = toDate(dateMax);
         }
    }
    else if ((pm == Paste::Normal) || (pm == Paste::NoBorder))
    {
      // clear the validity
      removeValidity();
    }

    //
    // Load the comment
    //
    QDomElement comment = cell.namedItem( "comment" ).toElement();
    if ( !comment.isNull() && ( pm == Paste::Normal || pm == Paste::Comment || pm == Paste::NoBorder ))
    {
        QString t = comment.text();
        //t = t.stripWhiteSpace();
        format()->setComment( t );
    }

    //
    // The real content of the cell is loaded here. It is stored in
    // the "text" tag, which contains either a text or a CDATA section.
    //
    // TODO: make this suck less. We set data twice, in loadCellData, and
    // also here. Not good.
    QDomElement text = cell.namedItem( "text" ).toElement();

    if ( !text.isNull() &&
          ( pm == Paste::Normal || pm == Paste::Text || pm == Paste::NoBorder || pm == Paste::Result ) )
    {
      /* older versions mistakenly put the datatype attribute on the cell
         instead of the text.  Just move it over in case we're parsing
         an old document */
      if ( cell.hasAttribute( "dataType" ) ) // new docs
        text.setAttribute( "dataType", cell.attribute( "dataType" ) );

      QDomElement result = cell.namedItem( "result" ).toElement();
      QString txt = text.text();
      if ((pm == Paste::Result) && (txt[0] == '='))
        // paste text of the element, if we want to paste result
        // and the source cell contains a formula
        // note that we mustn't use setCellValue after this, or else we lose
        // all the formulas ...
          d->strText = result.text();
      else
          //otherwise copy everything
          loadCellData(text, op);

      if ( !result.isNull() )
      {
        QString dataType;
        QString t = result.text();

        if ( result.hasAttribute( "dataType" ) )
          dataType = result.attribute( "dataType" );
        if ( result.hasAttribute( "outStr" ) )
        {
          d->strOutText = result.attribute( "outStr" );
          if ( !d->strOutText.isEmpty() )
            clearFlag( Flag_TextFormatDirty );
        }

        bool clear = true;
        // boolean ?
        if( dataType == "Bool" )
        {
          if ( t == "false" )
            setValue( false );
          else if ( t == "true" )
            setValue( true );
          else
            clear = false;
        }
        else if( dataType == "Num" )
        {
          bool ok = false;
          double dd = t.toDouble( &ok );
          if ( ok )
            setValue ( dd );
          else
            clear = false;
        }
        else if( dataType == "Date" )
        {
          bool ok = false;
          double dd = t.toDouble( &ok );
          if ( ok )
            setValue ( dd );
          else
          {
            int pos   = t.find( '/' );
            int year  = t.mid( 0, pos ).toInt();
            int pos1  = t.find( '/', pos + 1 );
            int month = t.mid( pos + 1, ( ( pos1 - 1 ) - pos ) ).toInt();
            int day   = t.right( t.length() - pos1 - 1 ).toInt();
            QDate date( year, month, day );
            if ( date.isValid() )
              setValue( date );
            else
              clear = false;
          }
        }
        else if( dataType == "Time" )
        {
          bool ok = false;
          double dd = t.toDouble( &ok );
          if ( ok )
            setCellValue( dd );
          else
          {
            int hours   = -1;
            int minutes = -1;
            int second  = -1;
            int pos, pos1;
            pos   = t.find( ':' );
            hours = t.mid( 0, pos ).toInt();
            pos1  = t.find( ':', pos + 1 );
            minutes = t.mid( pos + 1, ( ( pos1 - 1 ) - pos ) ).toInt();
            second  = t.right( t.length() - pos1 - 1 ).toInt();
            QTime time( hours, minutes, second );
            if ( time.isValid() )
              setValue( time );
            else
              clear = false;
          }
        }
        else
        {
          setValue( t );
        }

        // if ( clear )
        //   clearFlag( Flag_CalcDirty );
      }
    }

    return true;
}

bool Cell::loadCellData(const QDomElement & text, Paste::Operation op )
{
  //TODO: use converter()->asString() to generate strText

  QString t = text.text();
  t = t.stripWhiteSpace();

  setFlag(Flag_LayoutDirty);
  setFlag(Flag_TextFormatDirty);

  // A formula like =A1+A2 ?
  if( t[0] == '=' )
  {
    t = decodeFormula( t, d->column, d->row );
    setCellText (pasteOperation( t, d->strText, op ));

    setFlag(Flag_CalcDirty);
    clearAllErrors();

    if ( !makeFormula() )
      kdError(36001) << "ERROR: Syntax ERROR" << endl;
  }
  // rich text ?
  else if (t[0] == '!' )
  {
      // KSpread pre 1.4 stores hyperlink as rich text (first char is '!')
      // extract the link and the correspoding text
      // This is a rather dirty hack, but enough for KSpread generated XML
      bool inside_tag = false;
      QString qml_text;
      QString tag;
      QString qml_link;

      for( unsigned i = 1; i < t.length(); i++ )
      {
        QChar ch = t[i];
        if( ch == '<' )
        {
          if( !inside_tag )
          {
            inside_tag = true;
            tag = QString::null;
          }
        }
        else if( ch == '>' )
        {
          if( inside_tag )
          {
            inside_tag = false;
            if( tag.startsWith( "a href=\"", true ) )
            if( tag.endsWith( "\"" ) )
              qml_link = tag.mid( 8, tag.length()-9 );
            tag = QString::null;
          }
        }
        else
        {
          if( !inside_tag )
            qml_text += ch;
          else
            tag += ch;
        }
      }

      if( !qml_link.isEmpty() )
        d->extra()->link = qml_link;
      d->strText = qml_text;
      setValue( d->strText );
  }
  else
  {
    bool newStyleLoading = true;
    QString dataType;

    if ( text.hasAttribute( "dataType" ) ) // new docs
    {
        dataType = text.attribute( "dataType" );
    }
    else // old docs: do the ugly solution of calling checkTextInput to parse the text
    {
      // ...except for date/time
      if (isDate() && ( t.contains('/') == 2 ))
        dataType = "Date";
      else if (isTime() && ( t.contains(':') == 2 ) )
        dataType = "Time";
      else
      {
        d->strText = pasteOperation( t, d->strText, op );
        checkTextInput();
        //kdDebug(36001) << "Cell::load called checkTextInput, got dataType=" << dataType << "  t=" << t << endl;
        newStyleLoading = false;
      }
    }

    if ( newStyleLoading )
    {
      d->value = Value::empty();
      clearAllErrors();

      // boolean ?
      if( dataType == "Bool" )
      {
        bool val = (t.lower() == "true");
        setCellValue (val);
      }

      // number ?
      else if( dataType == "Num" )
      {
        bool ok = false;
        if (t.contains('.'))
          setValue ( Value( t.toDouble(&ok) ) ); // We save in non-localized format
        else
          setValue ( Value( t.toLong(&ok) ) );
        if ( !ok )
  {
          kdWarning(36001) << "Couldn't parse '" << t << "' as number." << endl;
  }
  /* We will need to localize the text version of the number */
  KLocale* locale = format()->sheet()->doc()->locale();

        /* KLocale::formatNumber requires the precision we want to return.
        */
        int precision = t.length() - t.find('.') - 1;

  if ( formatType() == Percentage_format )
        {
          if (value().isInteger())
            t = locale->formatNumber( value().asInteger() * 100 );
          else
            t = locale->formatNumber( value().asFloat() * 100.0, precision );
          d->strText = pasteOperation( t, d->strText, op );
          d->strText += '%';
        }
        else
        {
          if (value().isInteger())
            t = locale->formatLong(value().asInteger());
          else
            t = locale->formatNumber(value().asFloat(), precision);
          d->strText = pasteOperation( t, d->strText, op );
        }
      }

      // date ?
      else if( dataType == "Date" )
      {
        int pos = t.find('/');
        int year = t.mid(0,pos).toInt();
        int pos1 = t.find('/',pos+1);
        int month = t.mid(pos+1,((pos1-1)-pos)).toInt();
        int day = t.right(t.length()-pos1-1).toInt();
        setValue( QDate(year,month,day) );
        if ( value().asDate().isValid() ) // Should always be the case for new docs
          d->strText = locale()->formatDate( value().asDate(), true );
        else // This happens with old docs, when format is set wrongly to date
        {
          d->strText = pasteOperation( t, d->strText, op );
          checkTextInput();
        }
      }

      // time ?
      else if( dataType == "Time" )
      {
        int hours = -1;
        int minutes = -1;
        int second = -1;
        int pos, pos1;
        pos = t.find(':');
        hours = t.mid(0,pos).toInt();
        pos1 = t.find(':',pos+1);
        minutes = t.mid(pos+1,((pos1-1)-pos)).toInt();
        second = t.right(t.length()-pos1-1).toInt();
        setValue( QTime(hours,minutes,second) );
        if ( value().asTime().isValid() ) // Should always be the case for new docs
          d->strText = locale()->formatTime( value().asTime(), true );
        else  // This happens with old docs, when format is set wrongly to time
        {
          d->strText = pasteOperation( t, d->strText, op );
          checkTextInput();
        }
      }

      else
      {
        // Set the cell's text
        d->strText = pasteOperation( t, d->strText, op );
        setValue( d->strText );
      }
    }
  }

  if ( text.hasAttribute( "outStr" ) ) // very new docs
  {
    d->strOutText = text.attribute( "outStr" );
    if ( !d->strOutText.isEmpty() )
      clearFlag( Flag_TextFormatDirty );
  }

  if ( !format()->sheet()->isLoading() )
    setCellText( d->strText );

  if ( d->hasExtra() && d->extra()->conditions )
    d->extra()->conditions->checkMatches();

  return true;
}

QTime Cell::toTime(const QDomElement &element)
{
    //TODO: can't we use tryParseTime (after modification) instead?
    QString t = element.text();
    t = t.stripWhiteSpace();
    int hours = -1;
    int minutes = -1;
    int second = -1;
    int pos, pos1;
    pos = t.find(':');
    hours = t.mid(0,pos).toInt();
    pos1 = t.find(':',pos+1);
    minutes = t.mid(pos+1,((pos1-1)-pos)).toInt();
    second = t.right(t.length()-pos1-1).toInt();
    setValue( Value( QTime(hours,minutes,second)) );
    return value().asTime();
}

QDate Cell::toDate(const QDomElement &element)
{
    QString t = element.text();
    int pos;
    int pos1;
    int year = -1;
    int month = -1;
    int day = -1;
    pos = t.find('/');
    year = t.mid(0,pos).toInt();
    pos1 = t.find('/',pos+1);
    month = t.mid(pos+1,((pos1-1)-pos)).toInt();
    day = t.right(t.length()-pos1-1).toInt();
    setValue( Value( QDate(year,month,day) ) );
    return value().asDate();
}

QString Cell::pasteOperation( const QString &new_text, const QString &old_text, Paste::Operation op )
{
  if ( op == Paste::OverWrite )
        return new_text;

    QString tmp_op;
    QString tmp;
    QString old;

    if( !new_text.isEmpty() && new_text[0] == '=' )
    {
        tmp = new_text.right( new_text.length() - 1 );
    }
    else
    {
        tmp = new_text;
    }

    if ( old_text.isEmpty() &&
         ( op == Paste::Add || op == Paste::Mul || op == Paste::Sub || op == Paste::Div ) )
    {
      old = "=0";
    }

    if( !old_text.isEmpty() && old_text[0] == '=' )
    {
        old = old_text.right( old_text.length() - 1 );
    }
    else
    {
        old = old_text;
    }

    bool b1, b2;
    tmp.toDouble( &b1 );
    old.toDouble( &b2 );
    if (b1 && !b2 && old.length() == 0)
    {
      old = "0";
      b2 = true;
    }

    if( b1 && b2 )
    {
        switch( op )
        {
          case  Paste::Add:
            tmp_op = QString::number(old.toDouble()+tmp.toDouble());
            break;
          case Paste::Mul :
            tmp_op = QString::number(old.toDouble()*tmp.toDouble());
            break;
          case Paste::Sub:
            tmp_op = QString::number(old.toDouble()-tmp.toDouble());
            break;
          case Paste::Div:
            tmp_op = QString::number(old.toDouble()/tmp.toDouble());
            break;
        default:
            Q_ASSERT( 0 );
        }

        setFlag(Flag_LayoutDirty);
        clearAllErrors();

        return tmp_op;
    }
    else if ( ( new_text[0] == '=' && old_text[0] == '=' ) ||
              ( b1 && old_text[0] == '=' ) || ( new_text[0] == '=' && b2 ) )
    {
        switch( op )
        {
          case Paste::Add :
            tmp_op="=("+old+")+"+"("+tmp+")";
            break;
          case Paste::Mul :
            tmp_op="=("+old+")*"+"("+tmp+")";
            break;
          case Paste::Sub:
            tmp_op="=("+old+")-"+"("+tmp+")";
            break;
          case Paste::Div:
            tmp_op="=("+old+")/"+"("+tmp+")";
            break;
        default :
            Q_ASSERT( 0 );
        }

        tmp_op = decodeFormula( tmp_op, d->column, d->row );
        setFlag(Flag_LayoutDirty);
        clearAllErrors();

        return tmp_op;
    }

    tmp = decodeFormula( new_text, d->column, d->row );
    setFlag(Flag_LayoutDirty);
    clearAllErrors();

    return tmp;
}

QString Cell::testAnchor( int x, int y ) const
{
  if( link().isEmpty() )
    return QString::null;

  const Doc* doc = format()->sheet()->doc();
  int x1 = doc->zoomItX( d->textX );
  int y1 = doc->zoomItX( d->textY - d->textHeight );
  int x2 = doc->zoomItX( d->textX + d->textWidth );
  int y2 = doc->zoomItX( d->textY );

  if( x > x1 ) if( x < x2 )
  if( y > y1 ) if( y < y2 )
    return link();

  return QString::null;
}

void Cell::sheetDies()
{
    // Avoid unobscuring the cells in the destructor.
    if (d->hasExtra())
    {
      d->extra()->extraXCells = 0;
      d->extra()->extraYCells = 0;
      d->extra()->mergedXCells = 0;
      d->extra()->mergedYCells = 0;
    }

    //d->nextCell = 0;
    //d->previousCell = 0;
}

Cell::~Cell()
{
    if ( d->nextCell )
        d->nextCell->setPreviousCell( d->previousCell );
    if ( d->previousCell )
        d->previousCell->setNextCell( d->nextCell );

    if (d->hasExtra())
    {
      delete d->extra()->validity;
    }

    // Unobscure cells.
    int extraXCells = d->hasExtra() ? d->extra()->extraXCells : 0;
    int extraYCells = d->hasExtra() ? d->extra()->extraYCells : 0;
    for( int x = 0; x <= extraXCells; ++x )
        for( int y = (x == 0) ? 1 : 0; // avoid looking at (+0,+0)
             y <= extraYCells; ++y )
    {
        Cell* cell = format()->sheet()->cellAt( d->column + x, d->row + y );
        if ( cell )
            cell->unobscure(this);
    }

    d->value = Value::empty();

    if (!isDefault())
      valueChanged ();  //our value has been changed (is now null), but only if we aren't default

    delete d->format;
    delete d;
}

bool Cell::operator > ( const Cell & cell ) const
{
  if ( value().isNumber() ) // ### what about bools ?
  {
    if ( cell.value().isNumber() )
      return value().asFloat() > cell.value().asFloat();
    else
      return false; // numbers are always < than texts
  }
  else if(isDate())
  {
     if( cell.isDate() )
        return value().asDate() > cell.value().asDate();
     else if (cell.value().isNumber())
        return true;
     else
        return false; //date are always < than texts and time
  }
  else if(isTime())
  {
     if( cell.isTime() )
        return value().asTime() > cell.value().asTime();
     else if( cell.isDate())
        return true; //time are always > than date
     else if( cell.value().isNumber())
        return true;
     else
        return false; //time are always < than texts
  }
  else
  {
      if ( Map::respectCase )
          return value().asString().compare(cell.value().asString()) > 0;
      else
          return ( value().asString() ).lower().compare(cell.value().asString().lower()) > 0;
  }
}

bool Cell::operator < ( const Cell & cell ) const
{
  if ( value().isNumber() )
  {
    if ( cell.value().isNumber() )
      return value().asFloat() < cell.value().asFloat();
    else
      return true; // numbers are always < than texts
  }
  else if(isDate())
  {
     if( cell.isDate() )
        return value().asDateTime().date() < cell.value().asDateTime().date();
     else if( cell.value().isNumber())
        return false;
     else
        return true; //date are always < than texts and time
  }
  else if(isTime())
  {
     if( cell.isTime() )
        return value().asDateTime().time() < cell.value().asDateTime().time();
     else if(cell.isDate())
        return false; //time are always > than date
     else if( cell.value().isNumber())
        return false;
     else
        return true; //time are always < than texts
  }
  else
  {
      if ( Map::respectCase )
          return value().asString().compare(cell.value().asString()) < 0;
      else
          return ( value().asString() ).lower().compare(cell.value().asString().lower()) < 0;
  }
}

bool Cell::operator==( const Cell& other ) const
{
  if ( d->strText != other.d->strText )
    return false;
  if ( d->value != other.d->value )
    return false;
  if ( *d->format != *other.d->format )
    return false;
  if ( d->hasExtra() )
  {
    if ( !other.d->hasExtra() )
      return false;
    if ( d->extra()->link != other.d->extra()->link )
      return false;
    if ( d->extra()->mergedXCells != other.d->extra()->mergedXCells )
      return false;
    if ( d->extra()->mergedYCells != other.d->extra()->mergedYCells )
      return false;
    if ( d->extra()->conditions )
    {
      if ( !other.d->extra()->conditions )
        return false;
      if ( *d->extra()->conditions != *other.d->extra()->conditions )
        return false;
    }
    if ( d->extra()->validity )
    {
      if ( !other.d->extra()->validity )
        return false;
      if ( *d->extra()->validity != *other.d->extra()->validity )
        return false;
    }
  }
  return true;
}

QRect Cell::cellRect()
{
  Q_ASSERT(!isDefault());
  return QRect(QPoint(d->column, d->row), QPoint(d->column, d->row));
}

QValueList<Conditional> Cell::conditionList() const
{
  if ( !d->hasExtra() || !d->extra()->conditions )
  {
    QValueList<Conditional> emptyList;
    return emptyList;
  }

  return d->extra()->conditions->conditionList();
}

void Cell::setConditionList( const QValueList<Conditional> & newList )
{
  if (d->hasExtra())
    delete d->extra()->conditions;
  d->extra()->conditions = new Conditions( this );
  d->extra()->conditions->setConditionList( newList );
  d->extra()->conditions->checkMatches();
}

bool Cell::hasError() const
{
  return ( testFlag(Flag_ParseError) ||
           testFlag(Flag_CircularCalculation) ||
           testFlag(Flag_DependancyError));
}

void Cell::clearAllErrors()
{
  clearFlag( Flag_ParseError );
  clearFlag( Flag_CircularCalculation );
  clearFlag( Flag_DependancyError );
}

bool Cell::calcDirtyFlag()
{
  return isFormula() ? testFlag( Flag_CalcDirty ) : false;
}

bool Cell::layoutDirtyFlag() const
{
  return testFlag( Flag_LayoutDirty );
}

void Cell::clearDisplayDirtyFlag()
{
  clearFlag( Flag_DisplayDirty );
}

void Cell::setDisplayDirtyFlag()
{
  setFlag( Flag_DisplayDirty );
}

bool Cell::doesMergeCells() const
{
  return testFlag( Flag_Merged );
}

void Cell::clearFlag( CellFlags flag )
{
  d->flags &= ~(Q_UINT32)flag;
}

void Cell::setFlag( CellFlags flag )
{
  d->flags |= (Q_UINT32)flag;
}

bool Cell::testFlag( CellFlags flag ) const
{
  return ( d->flags & (Q_UINT32)flag );
}


void Cell::checkForNamedAreas( QString & formula ) const
{
  KSPLoadingInfo* loadinginfo = sheet()->doc()->loadingInfo();
  if(! loadinginfo) {
    kdDebug() << "Cell::checkForNamedAreas loadinginfo is NULL" << endl;
    return;
  }

  int l = formula.length();
  int i = 0;
  QString word;
  int start = 0;
  while ( i < l )
  {
    if ( formula[i].isLetterOrNumber() )
    {
      word += formula[i];
      ++i;
      continue;
    }
    if ( !word.isEmpty() )
    {
      if ( loadinginfo->findWordInAreaList(word) )
      {
        formula = formula.replace( start, word.length(), "'" + word + "'" );
        l = formula.length();
        ++i;
        kdDebug() << "Formula: " << formula << ", L: " << l << ", i: " << i + 1 <<endl;
      }
    }

    ++i;
    word = "";
    start = i;
  }
  if ( !word.isEmpty() )
  {
    if ( loadinginfo->findWordInAreaList(word) )
    {
      formula = formula.replace( start, word.length(), "'" + word + "'" );
      l = formula.length();
      ++i;
      kdDebug() << "Formula: " << formula << ", L: " << l << ", i: " << i + 1 <<endl;
    }
  }
}
