/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <qpainter.h>
#include <qapplication.h>
#include <qdrawutil.h>
#include <qkeycode.h>
#include <qregexp.h>
#include <qpoint.h>
#include <koprinter.h>
#include <qcursor.h>
#include <qstack.h>
#include <qbuffer.h>
#include <qmessagebox.h>
#include <qclipboard.h>
#include <qpicture.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qdragobject.h>
#include <qmime.h>

#include <klocale.h>
#include <kglobal.h>
#include <koFind.h>
#include <koReplace.h>

#include "kspread_table.h"
#include "kspread_undo.h"
#include "kspread_map.h"
#include "kspread_doc.h"
#include "kspread_util.h"
#include "kspread_canvas.h"

#include "KSpreadTableIface.h"

#include <koscript_context.h>

#include <kdebug.h>
#include <assert.h>

#include <koChart.h>

/*****************************************************************************
 *
 * CellBinding
 *
 *****************************************************************************/

CellBinding::CellBinding( KSpreadTable *_table, const QRect& _area )
{
  m_rctDataArea = _area;

  m_pTable = _table;
  m_pTable->addCellBinding( this );

  m_bIgnoreChanges = false;
}

CellBinding::~CellBinding()
{
  m_pTable->removeCellBinding( this );
}

void CellBinding::cellChanged( KSpreadCell *_cell )
{
  if ( m_bIgnoreChanges )
    return;

  emit changed( _cell );
}

bool CellBinding::contains( int _x, int _y )
{
  return m_rctDataArea.contains( QPoint( _x, _y ) );
}

/*****************************************************************************
 *
 * ChartBinding
 *
 *****************************************************************************/

ChartBinding::ChartBinding( KSpreadTable *_table, const QRect& _area, ChartChild *_child )
    : CellBinding( _table, _area )
{
  m_child = _child;
}

ChartBinding::~ChartBinding()
{
}

void ChartBinding::cellChanged( KSpreadCell* )
{
    kdDebug(36001) << "######### void ChartBinding::cellChanged( KSpreadCell* )" << endl;

    if ( m_bIgnoreChanges )
        return;

    kdDebug(36001) << "with=" << m_rctDataArea.width() << "  height=" << m_rctDataArea.height() << endl;

    KoChart::Data matrix( m_rctDataArea.height(), m_rctDataArea.width() );

    for ( int y = 0; y < m_rctDataArea.height(); y++ )
        for ( int x = 0; x < m_rctDataArea.width(); x++ )
        {
            KSpreadCell* cell = m_pTable->cellAt( m_rctDataArea.left() + x, m_rctDataArea.top() + y );
            if ( cell && cell->isValue() )
                matrix.cell( y, x ) = KoChart::Value( cell->valueDouble() );
            else if ( cell )
                matrix.cell( y, x ) = KoChart::Value( cell->valueString() );
            else
                matrix.cell( y, x ) = KoChart::Value();
        }

    // ######### Kalle may be interested in that, too
    /* Chart::Range range;
       range.top = m_rctDataArea.top();
       range.left = m_rctDataArea.left();
       range.right = m_rctDataArea.right();
       range.bottom = m_rctDataArea.bottom();
       range.table = m_pTable->name(); */

    m_child->chart()->setData( matrix );

    // Force a redraw of the chart on all views
    table()->emit_polygonInvalidated( m_child->framePointArray() );
}

/*****************************************************************************
 *
 * KSpreadTable
 *
 *****************************************************************************/

int KSpreadTable::s_id = 0L;
QIntDict<KSpreadTable>* KSpreadTable::s_mapTables;

KSpreadTable* KSpreadTable::find( int _id )
{
  if ( !s_mapTables )
    return 0L;

  return (*s_mapTables)[ _id ];
}

KSpreadTable::KSpreadTable( KSpreadMap *_map, const QString &tableName, const char *_name )
    : QObject( _map, _name )
{
  if ( s_mapTables == 0L )
    s_mapTables = new QIntDict<KSpreadTable>;

  m_id = s_id++;
  s_mapTables->insert( m_id, this );

  m_defaultLayout = new KSpreadLayout( this );

  m_emptyPen.setStyle( Qt::NoPen );

  m_marker.setCoords( 1, 1, 1, 1 );

  m_pMap = _map;
  m_pDoc = _map->doc();
  m_dcop = 0;
  m_bShowPageBorders = FALSE;

  m_lstCellBindings.setAutoDelete( FALSE );

  m_strName = tableName;

  // m_lstChildren.setAutoDelete( true );

  m_cells.setAutoDelete( true );
  m_rows.setAutoDelete( true );
  m_columns.setAutoDelete( true );

  m_pDefaultCell = new KSpreadCell( this, 0, 0 );
  m_pDefaultRowLayout = new RowLayout( this, 0 );
  m_pDefaultRowLayout->setDefault();
  m_pDefaultColumnLayout = new ColumnLayout( this, 0 );
  m_pDefaultColumnLayout->setDefault();

  // No selection is active
  m_rctSelection.setCoords( 0, 0, 0, 0 );

  m_pWidget = new QWidget();
  m_pPainter = new QPainter;
  m_pPainter->begin( m_pWidget );

  m_iMaxColumn = 256;
  m_iMaxRow = 256;
  m_bScrollbarUpdates = true;

  setHidden( false );
  m_bShowGrid=true;
  m_bShowFormular=false;
  m_bLcMode=false;
  m_bShowColumnNumber=false;
  m_bHideZero=false;
  m_bFirstLetterUpper=false;
  m_bAutoCalc=true;
  // Get a unique name so that we can offer scripting
  if ( !_name )
  {
      QCString s;
      s.sprintf("Table%i", s_id );
      QObject::setName( s.data() );
  }
  m_oldPos=QPoint(1,1);
  m_iScrollPosX=0;
  m_iScrollPosY=0;
}

bool KSpreadTable::isEmpty( unsigned long int x, unsigned long int y )
{
  KSpreadCell* c = cellAt( x, y );
  if ( !c || c->isEmpty() )
    return true;

  return false;
}

const ColumnLayout* KSpreadTable::columnLayout( int _column ) const
{
    const ColumnLayout *p = m_columns.lookup( _column );
    if ( p != 0L )
        return p;

    return m_pDefaultColumnLayout;
}

ColumnLayout* KSpreadTable::columnLayout( int _column )
{
    ColumnLayout *p = m_columns.lookup( _column );
    if ( p != 0L )
        return p;

    return m_pDefaultColumnLayout;
}

const RowLayout* KSpreadTable::rowLayout( int _row ) const
{
    const RowLayout *p = m_rows.lookup( _row );
    if ( p != 0L )
        return p;

    return m_pDefaultRowLayout;
}

RowLayout* KSpreadTable::rowLayout( int _row )
{
    RowLayout *p = m_rows.lookup( _row );
    if ( p != 0L )
        return p;

    return m_pDefaultRowLayout;
}

int KSpreadTable::leftColumn( int _xpos, int &_left, KSpreadCanvas *_canvas )
{
    if ( _canvas )
    {
        _xpos += _canvas->xOffset();
        _left = -_canvas->xOffset();
    }
    else
        _left = 0;

    int col = 1;
    int x = columnLayout( col )->width( _canvas );
    while ( x < _xpos )
    {
        // Should never happen
        if ( col == 0x10000 )
            return 1;
        _left += columnLayout( col )->width( _canvas );
        col++;
        x += columnLayout( col )->width( _canvas );
    }

    return col;
}

int KSpreadTable::rightColumn( int _xpos, KSpreadCanvas *_canvas )
{
    if ( _canvas )
        _xpos += _canvas->xOffset();

    int col = 1;
    int x = 0;
    while ( x < _xpos )
    {
        // Should never happen
        if ( col == 0x10000 )
            return 0x10000;
        x += columnLayout( col )->width( _canvas );
        col++;
    }

    return col;
}

int KSpreadTable::topRow( int _ypos, int & _top, KSpreadCanvas *_canvas )
{
    if ( _canvas )
    {
        _ypos += _canvas->yOffset();
        _top = -_canvas->yOffset();
    }
    else
        _top = 0;

    int row = 1;
    int y = rowLayout( row )->height( _canvas );
    while ( y < _ypos )
    {
        // Should never happen
        if ( row == 0x10000 )
            return 1;
        _top += rowLayout( row )->height( _canvas );
        row++;
        y += rowLayout( row )->height( _canvas);
    }

    return row;
}

int KSpreadTable::bottomRow( int _ypos, KSpreadCanvas *_canvas )
{
    if ( _canvas )
        _ypos += _canvas->yOffset();

    int row = 1;
    int y = 0;
    while ( y < _ypos )
    {
        // Should never happen
        if ( row == 0x10000 )
            return 0x10000;
        y += rowLayout( row )->height( _canvas );
        row++;
    }

    return row;
}

int KSpreadTable::columnPos( int _col, KSpreadCanvas *_canvas )
{
    int x = 0;
    if ( _canvas )
      x -= _canvas->xOffset();
    for ( int col = 1; col < _col; col++ )
    {
        // Should never happen
        if ( col == 0x10000 )
            return x;

        x += columnLayout( col )->width( _canvas );
    }

    return x;
}

int KSpreadTable::rowPos( int _row, KSpreadCanvas *_canvas )
{
    int y = 0;
    if ( _canvas )
      y -= _canvas->yOffset();
    for ( int row = 1 ; row < _row ; row++ )
    {
        // Should never happen
        if ( row == 0x10000 )
            return y;

        y += rowLayout( row )->height( _canvas );
    }

    return y;
}

KSpreadCell* KSpreadTable::visibleCellAt( int _column, int _row, bool _no_scrollbar_update )
{
  KSpreadCell* cell = cellAt( _column, _row, _no_scrollbar_update );
  if ( cell->isObscured() )
    return cellAt( cell->obscuringCellsColumn(), cell->obscuringCellsRow(), _no_scrollbar_update );

  return cell;
}

KSpreadCell* KSpreadTable::firstCell()
{
    return m_cells.firstCell();
}

RowLayout* KSpreadTable::firstRow()
{
    return m_rows.first();
}

ColumnLayout* KSpreadTable::firstCol()
{
    return m_columns.first();
}

const KSpreadCell* KSpreadTable::cellAt( int _column, int _row ) const
{
    const KSpreadCell *p = m_cells.lookup( _column, _row );
    if ( p != 0L )
        return p;

    return m_pDefaultCell;
}

KSpreadCell* KSpreadTable::cellAt( int _column, int _row, bool _no_scrollbar_update )
{
  if ( !_no_scrollbar_update && m_bScrollbarUpdates )
  {
    if ( _column > m_iMaxColumn )
    {
      m_iMaxColumn = _column;
      emit sig_maxColumn( _column );
    }
    if ( _row > m_iMaxRow )
    {
      m_iMaxRow = _row;
      emit sig_maxRow( _row );
    }
  }

  KSpreadCell *p = m_cells.lookup( _column, _row );
  if ( p != 0L )
    return p;

  return m_pDefaultCell;
}

ColumnLayout* KSpreadTable::nonDefaultColumnLayout( int _column, bool force_creation )
{
    ColumnLayout *p = m_columns.lookup( _column );
    if ( p != 0L || !force_creation )
        return p;

    p = new ColumnLayout( this, _column );
    //p->setWidth( m_pDefaultColumnLayout->width() );
    p->setWidth( static_cast<int>(colWidth) );
    m_columns.insertElement( p, _column );

    return p;
}

RowLayout* KSpreadTable::nonDefaultRowLayout( int _row, bool force_creation )
{
    RowLayout *p = m_rows.lookup( _row );
    if ( p != 0L || !force_creation )
        return p;

    p = new RowLayout( this, _row );
    // TODO: copy the default RowLayout here!!
    //    p->setHeight( m_pDefaultRowLayout->height() );
    //Laurent :
    // I used  heightOfRow because before it doesn't work:
    //  we used POINT_TO_MM  and after  MM_TO_POINT
    //  POINT_TO_MM !=  1/MM_TO_POINT
    // so it didn't give the good result
    p->setHeight(static_cast<int>(heightOfRow));

    m_rows.insertElement( p, _row );

    return p;
}

KSpreadCell* KSpreadTable::nonDefaultCell( int _column, int _row,
                                           bool _no_scrollbar_update )
{
  if ( !_no_scrollbar_update && m_bScrollbarUpdates )
  {
    if ( _column > m_iMaxColumn )
    {
      m_iMaxColumn = _column;
      emit sig_maxColumn( _column );
    }
    if ( _row > m_iMaxRow )
    {
      m_iMaxRow = _row;
      emit sig_maxRow( _row );
    }
  }

  KSpreadCell *p = m_cells.lookup( _column, _row );
  if ( p != 0L )
    return p;

  KSpreadCell *cell = new KSpreadCell( this, _column, _row );
  m_cells.insert( cell, _column, _row );

  return cell;
}

void KSpreadTable::setText( int _row, int _column, const QString& _text, bool updateDepends )
{
    KSpreadCell *cell = nonDefaultCell( _column, _row );

    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
        KSpreadUndoSetText *undo = new KSpreadUndoSetText( m_pDoc, this, cell->text(), _column, _row,cell->getFormatNumber(_column, _row) );
        m_pDoc->undoBuffer()->appendUndo( undo );
    }

    // The cell will force a display refresh itself, so we dont have to care here.
    cell->setCellText( _text, updateDepends );
    //refresh anchor
    if(_text.at(0)=='!')
      emit sig_updateView( this, QRect(_column,_row,_column,_row) );
}

void KSpreadTable::setLayoutDirtyFlag()
{
    KSpreadCell* c = m_cells.firstCell();
    for( ; c; c = c->nextCell() )
        c->setLayoutDirtyFlag();
}

void KSpreadTable::setCalcDirtyFlag()
{
    KSpreadCell* c = m_cells.firstCell();
    for( ; c; c = c->nextCell() )
        {
        if(!(c->isObscured() &&c->isObscuringForced()))
                c->setCalcDirtyFlag();
        }
}

void KSpreadTable::recalc(bool m_depend)
{
  //    kdDebug(36001) << "KSpreadTable::recalc(" << m_depend << ") STARTING" << endl;
    // First set all cells as dirty
    setCalcDirtyFlag();

    // Now recalc cells - it is important to do it AFTER, so that when
    // calculating one cell calculates many others, those are not done again.
    KSpreadCell* c = m_cells.firstCell();
    for( ; c; c = c->nextCell() )
        {
        if(!(c->isObscured() &&c->isObscuringForced()))
                c->calc( m_depend );
        }

    //    kdDebug(36001) << "KSpreadTable::recalc(" << m_depend << ") DONE" << endl;
}

void KSpreadTable::setChooseRect( const QRect &_sel )
{
    if ( _sel == m_chooseRect )
        return;

    QRect old( m_chooseRect );
    m_chooseRect = _sel;

    emit sig_changeChooseSelection( this, old, m_chooseRect );
}

void KSpreadTable::unselect()
{
    // No selection? Then do nothing.
    if ( m_rctSelection.left() == 0 )
        return;

    QRect r = m_rctSelection;
    // Discard the selection
    m_rctSelection.setCoords( 0, 0, 0, 0 );

    // Emit signal so that the views can update.
    emit sig_unselect( this, r );
}

void KSpreadTable::setMarker( const QPoint& _point, KSpreadCanvas *_canvas )
{
    setSelection( QRect(), _point, _canvas );
}

QRect KSpreadTable::markerRect() const
{
    QRect r;
    if ( m_rctSelection.left() == 0 )
        r = m_marker;
    else
        r = m_rctSelection;

    if ( r.topLeft() == r.bottomRight() )
    {
        const KSpreadCell* cell = cellAt( r.left(), r.top() );
        if ( cell->extraXCells() || cell->extraYCells() )
            r.setCoords( r.left(), r.top(),
                         r.left() + cell->extraXCells(), r.top() + cell->extraYCells() );
    }

    return r;
}

QRect KSpreadTable::marker() const
{
    return m_marker;
}

void KSpreadTable::setSelection( const QRect &_sel, KSpreadCanvas *_canvas )
{   m_oldPos=QPoint( m_marker.topLeft());
    if ( _sel.left() == 0 )
        setSelection( _sel, m_marker.topLeft(), _canvas );
    else
    {
        if ( m_marker.topLeft() != _sel.topLeft() && m_marker.topRight() != _sel.topRight() &&
             m_marker.bottomLeft() != _sel.bottomLeft() && m_marker.bottomRight() != _sel.bottomRight() )
            setSelection( _sel, _sel.topLeft(), _canvas );
        else
            setSelection( _sel, m_marker.topLeft(), _canvas );
    }
}

void KSpreadTable::setSelection( const QRect &_sel, const QPoint& m, KSpreadCanvas *_canvas )
{
  if ( _sel == m_rctSelection && m == m_marker.topLeft() )
    return;

  // We want to see whether a single cell was clicked like a button.
  // This is only of interest if no cell was selected before
  if ( _sel.left() == 0 )
  {
    // So we test first whether only a single cell was selected
    KSpreadCell *cell = cellAt( m_rctSelection.left(), m_rctSelection.top() );
    // Did we mark only a single cell ?
    // Take care: One cell may obscure other cells ( extra size! ).
    if ( m_rctSelection.left() + cell->extraXCells() == m_rctSelection.right() &&
         m_rctSelection.top() + cell->extraYCells() == m_rctSelection.bottom() )
      cell->clicked( _canvas );
  }

  QRect old_marker = m_marker;
  QRect old( m_rctSelection );
  m_rctSelection = _sel;

  KSpreadCell* cell = cellAt( m.x(), m.y() );
  if ( cell->extraXCells() || cell->extraYCells())
      m_marker.setCoords( m.x(), m.y(), m.x() + cell->extraXCells(), m.y() + cell->extraYCells() );
  else if(cell->isObscuringForced())
        {
        KSpreadCell* cell2 = cellAt( cell->obscuringCellsColumn(),
        cell->obscuringCellsRow() );
        QRect extraArea;
        extraArea.setCoords( cell->obscuringCellsColumn(),cell->obscuringCellsRow(),
        cell->obscuringCellsColumn()+ cell2->extraXCells(),cell->obscuringCellsRow()+ cell2->extraYCells());
        if(extraArea.contains(m.x(),m.y()))
                {
                m_marker=extraArea;
                }
        else
                {
                m_oldPos=QPoint( m.x(),m.y());
                m_marker = QRect( m, m );
                }
        }
  else
        {
        m_oldPos=QPoint( m.x(),m.y());
      m_marker = QRect( m, m );
      }

  emit sig_changeSelection( this, old, old_marker );
}

/*
 Methods working on selections:

 TYPE A:
 { columns selected:
   for all rows with properties X,X':
     if default-cell create new cell
 }
 post undo object (always a KSpreadUndoCellLayout; difference in title only)
 { rows selected:
   if condition Y clear properties X,X' of cells;
   set properties X,X' of rowlayouts
   emit complete update;
 }
 { columns selected:
   if condition Y clear properties X,X' of cells;
   set properties X,X' of columnlayouts;
   for all rows with properties X,X':
     create cells if necessary and set properties X,X'
   emit complete update;
 }
 { cells selected:
   for all cells with condition Y:
     create if necessary and set properties X,X' and do Z;
   emit update on selected region;
 }

 USED in:
 setSelectionFont
 setSelectionSize
 setSelectionAngle
 setSelectionTextColor
 setSelectionBgColor
 setSelectionPercent
 borderAll
 borderRemove (exceptions: ### creates cells (why?), ### changes default cell if cell-regions selected?)
 setSelectionAlign
 setSelectionAlignY
 setSelectionMoneyFormat
 increaseIndent
 decreaseIndent

 TYPE B:
 post undo object
 { rows selected:
   if condition Y do X with cells;
   emit update on selection;
 }
 { columns selected:
   if condition Y do X with cells;
   emit update on selection;
 }
 { cells selected:
   if condition Y do X with cells; create cell if non-default;
   emit update on selection;
 }

 USED in:
 setSelectionUpperLower (exceptions: no undo; no create-if-default; ### modifies default-cell?)
 setSelectionFirstLetterUpper (exceptions: no undo; no create-if-default; ### modifies default-cell?)
 setSelectionVerticalText
 setSelectionComment
 setSelectionRemoveComment (exeception: no create-if-default and work only on non-default-cells for cell regions)
 setSelectionBorderColor (exeception: no create-if-default and work only on non-default-cells for cell regions)
 setSelectionMultiRow
 setSelectionPrecision
 clearTextSelection (exception: all only if !areaIsEmpty())
 clearValiditySelection (exception: all only if !areaIsEmpty())
 clearConditionalSelection (exception: all only if !areaIsEmpty())
 setConditional (exception: conditional after create-if-default for cell regions)
 setValidity (exception: conditional after create-if-default for cell regions)

 OTHERS:
 borderBottom
 borderRight
 borderLeft
 borderTop
 borderOutline
 => these work only on some cells (at the border); undo only if cells affected; rest is similar to type A
 --> better not use CellWorker/workOnCells()

 defaultSelection
 => similar to TYPE B, but works on columns/rows if complete columns/rows selected
 --> use emit_signal=false and return value of workOnCells to finish

 getWordSpelling
 => returns text, no signal emitted, no cell-create, similar to TYPE B
 --> use emit_signal=false, create_if_default=false and type B

 setWordSpelling
 => no signal emitted, no cell-create, similar to type B
 --> use emit_signal=false, create_if_default=false and type B
 */

class KSpreadUndoAction* KSpreadTable::CellWorkerTypeA::createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r )
{
    QString title = getUndoTitle();
    return new KSpreadUndoCellLayout( doc, table, r, title );
}

KSpreadTable::SelectionType KSpreadTable::workOnCells( const QPoint& _marker, CellWorker& worker )
{
    // see what is selected; if nothing, take marker position
    bool selected = ( m_rctSelection.left() != 0 );
    QRect r( m_rctSelection );
    if ( !selected )
	r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

    // create cells in rows if complete columns selected
    if ( !worker.type_B && selected && m_rctSelection.bottom() == 0x7FFF )
    {
	for ( RowLayout* rw =m_rows.first(); rw; rw = rw->next() )
	{
	    if ( !rw->isDefault() && worker.testCondition( rw ) )
	    {
		for ( int i=m_rctSelection.left(); i<=m_rctSelection.right(); i++ )
		{
		    KSpreadCell *cell = cellAt( i, rw->row() );
		    if ( cell == m_pDefaultCell )
			// '&& worker.create_if_default' unneccessary as never used in type A
		    {
			cell = new KSpreadCell( this, i, rw->row() );
			m_cells.insert( cell, i, rw->row() );
		    }
		}
	    }
	}
    }

    // create an undo action
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
	KSpreadUndoAction *undo = worker.createUndoAction( m_pDoc, this, r );
        // test if the worker has an undo action
        if ( undo != 0L )
	    m_pDoc->undoBuffer()->appendUndo( undo );
    }

    // complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
	for ( KSpreadCell* cell = m_cells.firstCell(); cell; cell = cell->nextCell() )
	{
	    int row = cell->row();
	    if ( m_rctSelection.top() <= row && m_rctSelection.bottom() >= row
		 && worker.testCondition( cell ) )
		if ( worker.type_B )
		    worker.doWork( cell, false, cell->column(), row );
		else
		    worker.prepareCell( cell );
	}

	if ( worker.type_B ) {
            // for type B there's nothing left to do
	    if ( worker.emit_signal )
		emit sig_updateView( this, r );
	} else {
            // for type A now work on row layouts
	    for ( int i=m_rctSelection.top(); i<=m_rctSelection.bottom(); i++ )
	    {
		RowLayout *rw=nonDefaultRowLayout(i);
		worker.doWork( rw );
	    }
	    if ( worker.emit_signal )
		emit sig_updateView( this );
	}
	return CompleteRows;
    }
    // complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
	for ( KSpreadCell* cell = m_cells.firstCell(); cell; cell = cell->nextCell() )
	{
	    int col = cell->column();
	    if ( m_rctSelection.left() <= col && m_rctSelection.right() >= col
		 && worker.testCondition( cell ) )
		if ( worker.type_B )
		    worker.doWork( cell, false, col, cell->row() );
		else
		    worker.prepareCell( cell );
	}

	if ( worker.type_B ) {
	    if ( worker.emit_signal )
		emit sig_updateView( this, r );
	} else {
	    // for type A now work on column layouts
	    for ( int i=m_rctSelection.left(); i<=m_rctSelection.right(); i++ )
	    {
		ColumnLayout *cl=nonDefaultColumnLayout(i);
		worker.doWork( cl );
	    }

	    for ( RowLayout* rw =m_rows.first(); rw; rw = rw->next() )
	    {
		if ( !rw->isDefault() && worker.testCondition( rw ) )
		{
		    for ( int i=m_rctSelection.left(); i<=m_rctSelection.right(); i++ )
		    {
			KSpreadCell *cell = cellAt( i, rw->row());
			// ### this if should be not necessary; cells are created
			//     before the undo object is created, aren't they?
			if ( cell == m_pDefaultCell )
			{
			    cell = new KSpreadCell( this, i, rw->row() );
			    m_cells.insert( cell, i, rw->row() );
			}
			worker.doWork( cell, false, i, rw->row() );
		    }
		}
	    }
            if ( worker.emit_signal )
		emit sig_updateView( this );
	}
	return CompleteColumns;
    }
    // cell region selected
    else
    {
	for ( int x = r.left(); x <= r.right(); x++ )
	    for ( int y = r.top(); y <= r.bottom(); y++ )
	    {
		KSpreadCell *cell = cellAt( x, y );
                if ( worker.testCondition( cell ) )
		{
		    if ( worker.create_if_default && cell == m_pDefaultCell )
		    {
			cell = new KSpreadCell( this, x, y );
			m_cells.insert( cell, x, y );
		    }
                    if ( cell != m_pDefaultCell )
			worker.doWork( cell, true, x, y );
		}
	    }
        if ( worker.emit_signal )
	    emit sig_updateView( this, r );
        return CellRegion;
    }
}

struct SetSelectionFontWorker : public KSpreadTable::CellWorkerTypeA {
    const char *_font;
    int _size;
    signed char _bold;
    signed char _italic;
    signed char _underline;
    signed char _strike;
    SetSelectionFontWorker( const char *font, int size, signed char bold, signed char italic,signed char underline, signed char strike )
	: _font( font ), _size( size ), _bold( bold ), _italic( italic ), _underline( underline ), _strike( strike ) { }

    QString getUndoTitle() { return i18n("Change font"); }
    bool testCondition( RowLayout* rw ) {
        return ( rw->hasProperty( KSpreadCell::PFont ) );
    }
    void doWork( RowLayout* rw ) {
	if ( _font )
	    rw->setTextFontFamily( _font );
	if ( _size > 0 )
	    rw->setTextFontSize( _size );
	if ( _italic >= 0 )
	    rw->setTextFontItalic( (bool)_italic );
	if ( _bold >= 0 )
	    rw->setTextFontBold( (bool)_bold );
	if ( _underline >= 0 )
	    rw->setTextFontUnderline( (bool)_underline );
	if ( _strike >= 0 )
	    rw->setTextFontStrike( (bool)_strike );
    }
    void doWork( ColumnLayout* cl ) {
	if ( _font )
	    cl->setTextFontFamily( _font );
	if ( _size > 0 )
	    cl->setTextFontSize( _size );
	if ( _italic >= 0 )
	    cl->setTextFontItalic( (bool)_italic );
	if ( _bold >= 0 )
	    cl->setTextFontBold( (bool)_bold );
	if ( _underline >= 0 )
	    cl->setTextFontUnderline( (bool)_underline );
	if ( _strike >= 0 )
	    cl->setTextFontStrike( (bool)_strike );
    }
    void prepareCell( KSpreadCell* cell ) {
	cell->clearProperty( KSpreadCell::PFont );
	cell->clearNoFallBackProperties( KSpreadCell::PFont );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	if ( _font )
	    cell->setTextFontFamily( _font );
	if ( _size > 0 )
	    cell->setTextFontSize( _size );
	if ( _italic >= 0 )
	    cell->setTextFontItalic( (bool)_italic );
	if ( _bold >= 0 )
	    cell->setTextFontBold( (bool)_bold );
	if ( _underline >= 0 )
	    cell->setTextFontUnderline( (bool)_underline );
	if ( _strike >= 0 )
	    cell->setTextFontStrike( (bool)_strike );
        if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionFont( const QPoint &_marker, const char *_font, int _size,
                                     signed char _bold, signed char _italic,signed char _underline,
                                     signed char _strike )
{
    SetSelectionFontWorker w( _font, _size, _bold, _italic, _underline, _strike );
    workOnCells( _marker, w );
}

struct SetSelectionSizeWorker : public KSpreadTable::CellWorkerTypeA {
    int _size, size;
    SetSelectionSizeWorker( int __size, int size2 ) : _size( __size ), size( size2 ) { }

    QString getUndoTitle() { return i18n("Change font"); }
    bool testCondition( RowLayout* rw ) {
        return ( rw->hasProperty( KSpreadCell::PFont ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setTextFontSize( size + _size) ;
    }
    void doWork( ColumnLayout* cl ) {
	cl->setTextFontSize( size + _size );
    }
    void prepareCell( KSpreadCell* cell ) {
	cell->clearProperty( KSpreadCell::PFont );
	cell->clearNoFallBackProperties( KSpreadCell::PFont );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setTextFontSize( size + _size );
        if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionSize( const QPoint &_marker, int _size )
{
    int size;
    KSpreadCell* c;
    c=cellAt(_marker.x(), _marker.y());
    size=c->textFontSize(_marker.x(), _marker.y());

    SetSelectionSizeWorker w( _size, size );
    workOnCells( _marker, w );
}


struct SetSelectionUpperLowerWorker : public KSpreadTable::CellWorker {
    int _type;
    SetSelectionUpperLowerWorker( int type ) : KSpreadTable::CellWorker( false ), _type( type ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoChangeAreaTextCell( doc, table, r );
    }
    bool testCondition( KSpreadCell* c ) {
	return ( !c->isValue() && !c->isBool() &&!c->isFormular() && !c->isDefault()
		 && !c->text().isEmpty() && c->text()[0] != '*' && c->text()[0] != '!'
		 && !c->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	if ( _type == -1 )
	    cell->setCellText( (cell->text().lower()));
	else if ( _type == 1 )
	    cell->setCellText( (cell->text().upper()));
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionUpperLower( const QPoint &_marker, int _type )
{
    SetSelectionUpperLowerWorker w( _type );
    workOnCells( _marker, w );
}


struct SetSelectionFirstLetterUpperWorker : public KSpreadTable::CellWorker {
    SetSelectionFirstLetterUpperWorker( ) : KSpreadTable::CellWorker( false ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return   new KSpreadUndoChangeAreaTextCell( doc, table, r );
    }
    bool testCondition( KSpreadCell* c ) {
	return ( !c->isValue() && !c->isBool() &&!c->isFormular() && !c->isDefault()
		 && !c->text().isEmpty() && c->text()[0] != '*' && c->text()[0] != '!'
		 && !c->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	QString tmp = cell->text();
	int len = tmp.length();
	cell->setCellText( (tmp.at(0).upper()+tmp.right(len-1)) );
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionfirstLetterUpper( const QPoint &_marker)
{
    SetSelectionFirstLetterUpperWorker w;
    workOnCells( _marker, w );
}


struct SetSelectionVerticalTextWorker : public KSpreadTable::CellWorker {
    bool _b;
    SetSelectionVerticalTextWorker( bool b ) : KSpreadTable::CellWorker( ), _b( b ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Vertical Text");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	cell->setVerticalText( _b );
	cell->setMultiRow( false );
	cell->setAngle( 0 );
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionVerticalText( const QPoint &_marker, bool _b )
{
    SetSelectionVerticalTextWorker w( _b );
    workOnCells( _marker, w );
}


struct SetSelectionCommentWorker : public KSpreadTable::CellWorker {
    QString _comment;
    SetSelectionCommentWorker( QString comment ) : KSpreadTable::CellWorker( ), _comment( comment ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Add comment");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	cell->setComment( _comment );
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionComment( const QPoint &_marker, QString _comment)
{
    SetSelectionCommentWorker w( _comment );
    workOnCells( _marker, w );
}


struct SetSelectionAngleWorker : public KSpreadTable::CellWorkerTypeA {
    int _value;
    SetSelectionAngleWorker( int value ) : _value( value ) { }

    QString getUndoTitle() { return i18n("Change angle"); }
    bool testCondition( RowLayout* rw ) {
        return ( rw->hasProperty( KSpreadCell::PAngle ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setAngle( _value );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setAngle( _value );
    }
    void prepareCell( KSpreadCell* cell ) {
	cell->clearProperty( KSpreadCell::PAngle );
	cell->clearNoFallBackProperties( KSpreadCell::PAngle );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setAngle( _value );
	if ( cellRegion ) {
	    cell->setVerticalText(false);
	    cell->setMultiRow( false );
	    cell->clearDisplayDirtyFlag();
	}
    }
};

void KSpreadTable::setSelectionAngle( const QPoint &_marker, int _value )
{
    SetSelectionAngleWorker w( _value );
    workOnCells( _marker, w );
}

struct SetSelectionRemoveCommentWorker : public KSpreadTable::CellWorker {
    SetSelectionRemoveCommentWorker( ) : KSpreadTable::CellWorker( false ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Remove comment");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	cell->setComment( "" );
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionRemoveComment( const QPoint &_marker)
{
    SetSelectionRemoveCommentWorker w;
    workOnCells( _marker, w );
}


struct SetSelectionTextColorWorker : public KSpreadTable::CellWorkerTypeA {
    const QColor& tb_Color;
    SetSelectionTextColorWorker( const QColor& _tb_Color ) : tb_Color( _tb_Color ) { }

    QString getUndoTitle() { return i18n("Change text color"); }
    bool testCondition( RowLayout* rw ) {
        return ( rw->hasProperty( KSpreadCell::PTextPen ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setTextColor( tb_Color );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setTextColor( tb_Color );
    }
    void prepareCell( KSpreadCell* cell ) {
	cell->clearProperty( KSpreadCell::PTextPen );
	cell->clearNoFallBackProperties( KSpreadCell::PTextPen );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setTextColor( tb_Color );
	if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionTextColor( const QPoint &_marker, const QColor &tb_Color )
{
    SetSelectionTextColorWorker w( tb_Color );
    workOnCells( _marker, w );
}


struct SetSelectionBgColorWorker : public KSpreadTable::CellWorkerTypeA {
    const QColor& bg_Color;
    SetSelectionBgColorWorker( const QColor& _bg_Color ) : bg_Color( _bg_Color ) { }

    QString getUndoTitle() { return i18n("Change background color"); }
    bool testCondition( RowLayout* rw ) {
        return ( rw->hasProperty( KSpreadCell::PBackgroundColor ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setBgColor( bg_Color );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setBgColor( bg_Color );
    }
    void prepareCell( KSpreadCell* cell ) {
	cell->clearProperty( KSpreadCell::PBackgroundColor );
	cell->clearNoFallBackProperties( KSpreadCell::PBackgroundColor );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setBgColor( bg_Color );
	if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionbgColor( const QPoint &_marker, const QColor &bg_Color )
{
    SetSelectionBgColorWorker w( bg_Color );
    workOnCells( _marker, w );
}


struct SetSelectionBorderColorWorker : public KSpreadTable::CellWorker {
    const QColor& bd_Color;
    SetSelectionBorderColorWorker( const QColor& _bd_Color ) : KSpreadTable::CellWorker( false ), bd_Color( _bd_Color ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Change border color");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	int it_Row = cell->row();
	int it_Col = cell->column();
	if ( cell->topBorderStyle( it_Row, it_Col )!=Qt::NoPen )
	    cell->setTopBorderColor( bd_Color );
	if ( cell->leftBorderStyle( it_Row, it_Col )!=Qt::NoPen )
	    cell->setLeftBorderColor( bd_Color );
	if ( cell->fallDiagonalStyle( it_Row, it_Col )!=Qt::NoPen )
	    cell->setFallDiagonalColor( bd_Color );
	if ( cell->goUpDiagonalStyle( it_Row, it_Col )!=Qt::NoPen )
	    cell->setGoUpDiagonalColor( bd_Color );
	if ( cell->bottomBorderStyle( it_Row, it_Col )!=Qt::NoPen )
	    cell->setBottomBorderColor( bd_Color );
	if ( cell->rightBorderStyle( it_Row, it_Col )!=Qt::NoPen )
	    cell->setRightBorderColor( bd_Color );
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionBorderColor( const QPoint &_marker, const QColor &bd_Color )
{
    SetSelectionBorderColorWorker w( bd_Color );
    workOnCells( _marker, w );
}


void KSpreadTable::setSeries( const QPoint &_marker,int start,int end,int step,Series mode,Series type)
{
    QRect r(_marker.x(), _marker.y(), _marker.x(), _marker.y() );

    int y = r.top();
    int x = r.left();
    int posx=0;
    int posy=0;
    int numberOfCell=0;
    for ( int incr=start;incr<=end; )
    {
        if(type==Linear)
            incr=incr+step;
        else if(type==Geometric)
            incr=incr*step;
        numberOfCell++;
    }
    int extraX=_marker.x();
    int extraY=_marker.y();
    if(mode==Column)
    {
        for ( int y = _marker.y(); y <=(_marker.y()+numberOfCell); y++ )
        {
            KSpreadCell *cell = cellAt( _marker.x(), y );
            if( cell->isObscuringForced())
            {
                numberOfCell+=cell->extraYCells()+1;
                extraX=QMIN(extraX,cell->obscuringCellsColumn());
            }
        }
    }
    else if(mode==Row)
    {
        for ( int x = _marker.x(); x <=(_marker.x()+numberOfCell); x++ )
        {
            KSpreadCell *cell = cellAt( x,_marker.y() );
            if( cell->isObscuringForced())
            {
                numberOfCell+=cell->extraXCells()+1;
                extraY=QMIN(extraY,cell->obscuringCellsRow());
            }
        }
    }
    QRect rect;
    if(mode==Column)
    {
        rect.setCoords( extraX,_marker.y(),_marker.x(),_marker.y()+numberOfCell);
    }
    else if(mode==Row)
    {
        rect.setCoords(_marker.x(),extraY,_marker.x()+numberOfCell,_marker.y());
    }

    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
        KSpreadUndoChangeAreaTextCell *undo = new KSpreadUndoChangeAreaTextCell( m_pDoc, this, rect );
        m_pDoc->undoBuffer()->appendUndo( undo );
    }

    for ( int incr=start;incr<=end; )
    {
        KSpreadCell *cell = cellAt( x+posx, y+posy );
        if(cell->isObscuringForced())
        {
            cell = cellAt( cell->obscuringCellsColumn(), cell->obscuringCellsRow());
        }
        if ( cell == m_pDefaultCell )
        {
            cell = new KSpreadCell( this, x+posx, y+posy );
            m_cells.insert( cell, x + posx, y + posy );
        }

        QString tmp;
        cell->setCellText(tmp.setNum(incr));
        if(mode==Column)
            if(cell->isForceExtraCells())
                posy+=cell->extraYCells()+1;
            else
                posy++;
        else if(mode==Row)
            if(cell->isForceExtraCells())
                posx+=cell->extraXCells()+1;
            else
                posx++;
        else
            kdDebug(36001) << "Error in Series::mode" << endl;
        if(type==Linear)
            incr=incr+step;
        else if(type==Geometric)
            incr=incr*step;
        else
            kdDebug(36001) << "Error in Series::type" << endl;
    }
}


struct SetSelectionPercentWorker : public KSpreadTable::CellWorkerTypeA {
    bool b;
    SetSelectionPercentWorker( bool _b ) : b( _b ) { }

    QString getUndoTitle() { return i18n("Format percent"); }
    bool testCondition( RowLayout* rw ) {
        return ( rw->hasProperty( KSpreadCell::PFaktor ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setFaktor( b ? 100.0 : 1.0 );
	rw->setPrecision( 0 );
	rw->setFormatNumber( b ? KSpreadCell::Percentage : KSpreadCell::Number);
    }
    void doWork( ColumnLayout* cl ) {
	cl->setFaktor( b ? 100.0 : 1.0 );
	cl->setPrecision( 0 );
	cl->setFormatNumber( b ? KSpreadCell::Percentage : KSpreadCell::Number);
    }
    void prepareCell( KSpreadCell* cell ) {
	cell->clearProperty(KSpreadCell::PFaktor);
	cell->clearNoFallBackProperties( KSpreadCell::PFaktor );
	cell->clearProperty(KSpreadCell::PPrecision);
	cell->clearNoFallBackProperties( KSpreadCell::PPrecision );
	cell->clearProperty(KSpreadCell::PFormatNumber);
	cell->clearNoFallBackProperties( KSpreadCell::PFormatNumber );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setFaktor( b ? 100.0 : 1.0 );
	cell->setPrecision( 0 );
	cell->setFormatNumber( b ? KSpreadCell::Percentage : KSpreadCell::Number);
	if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionPercent( const QPoint &_marker, bool b )
{
    SetSelectionPercentWorker w( b );
    workOnCells( _marker, w );
}


void KSpreadTable::refreshRemoveAreaName(const QString &_areaName)
{
 KSpreadCell* c = m_cells.firstCell();
 QString tmp="'"+_areaName+"'";
    for( ;c; c = c->nextCell() )
    {
        if(c->isFormular() )
        {

	  if(c->text().find(tmp)!=-1)
	    {
	      if ( !c->makeFormular() )
		kdError(36002) << "ERROR: Syntax ERROR" << endl;
	    }
	}
    }
}

void KSpreadTable::changeCellTabName(QString old_name,QString new_name)
{
    KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
    {
        if(c->isFormular() || c->content()==KSpreadCell::RichText)
        {
            if(c->text().find(old_name)!=-1)
            {
                int nb = c->text().contains(old_name+"!");
                QString tmp=old_name+"!";
                int len=tmp.length();
                tmp=c->text();

                for(int i=0;i<nb;i++)
                {
                    int pos= tmp.find(old_name+"!");
                    tmp.replace(pos,len,new_name+"!");
                }
                c->setCellText(tmp);
            }
        }
    }
}

bool KSpreadTable::shiftRow( const QRect &rect,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked()  &&makeUndo)
    {
            KSpreadUndoInsertCellRow *undo = new KSpreadUndoInsertCellRow( m_pDoc, this,rect );
            m_pDoc->undoBuffer()->appendUndo( undo );
    }

    bool res=true;
    for(int i =rect.top();i<=rect.bottom();i++)
        for(int j=0;j<=(rect.right()-rect.left());j++)
        {
        bool result=m_cells.shiftRow( QPoint(rect.left(),i) );
        if(!result)
                res=false;
         }

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        {
        for(int i=rect.top();i<=rect.bottom();i++)
                it.current()->changeNameCellRef( QPoint(rect.left(),i), false, KSpreadTable::ColumnInsert, name() ,(rect.right()-rect.left()+1));
        }
    refreshChart(QPoint(rect.left(),rect.top()), false, KSpreadTable::ColumnInsert);
    recalc(true);
    refreshMergedCell();
    emit sig_updateView( this );

    return res;
}

bool KSpreadTable::shiftColumn( const QRect& rect,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked()  &&makeUndo)
    {
            KSpreadUndoInsertCellCol *undo = new KSpreadUndoInsertCellCol( m_pDoc, this,rect);
            m_pDoc->undoBuffer()->appendUndo( undo );
    }

    bool res=true;
    for(int i =rect.left();i<=rect.right();i++)
        for(int j=0;j<=(rect.bottom()-rect.top());j++)
        {
        bool result=m_cells.shiftColumn( QPoint(i,rect.top()) );
        if(!result)
                res=false;
         }


    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        for(int i=rect.left();i<=rect.right();i++)
                it.current()->changeNameCellRef( QPoint(i,rect.top()), false, KSpreadTable::RowInsert, name() ,(rect.bottom()-rect.top()+1));
    refreshChart(/*marker*/QPoint(rect.left(),rect.top()), false, KSpreadTable::RowInsert);
    recalc(true);
    refreshMergedCell();
    emit sig_updateView( this );

    return res;
}

void KSpreadTable::unshiftColumn( const QRect & rect,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked()  &&makeUndo)
    {
            KSpreadUndoRemoveCellCol *undo = new KSpreadUndoRemoveCellCol( m_pDoc, this,rect);
            m_pDoc->undoBuffer()->appendUndo( undo );
    }

    for(int i =rect.top();i<=rect.bottom();i++)
        for(int j=rect.left();j<=rect.right();j++)
               m_cells.remove(j,i);

    for(int i =rect.left();i<=rect.right();i++)
        for(int j=0;j<=(rect.bottom()-rect.top());j++)
                m_cells.unshiftColumn( QPoint(i,rect.top()) );

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        for(int i=rect.left();i<=rect.right();i++)
                it.current()->changeNameCellRef( QPoint(i,rect.top()), false, KSpreadTable::RowRemove, name(),(rect.bottom()-rect.top()+1) );

    refreshChart( QPoint(rect.left(),rect.top()), false, KSpreadTable::RowRemove );
    refreshMergedCell();
    recalc(true);
    emit sig_updateView( this );
}

void KSpreadTable::unshiftRow( const QRect & rect,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked() &&makeUndo)
    {
            KSpreadUndoRemoveCellRow *undo = new KSpreadUndoRemoveCellRow( m_pDoc, this,rect );
            m_pDoc->undoBuffer()->appendUndo( undo );
    }
    for(int i =rect.top();i<=rect.bottom();i++)
        for(int j=rect.left();j<=rect.right();j++)
                m_cells.remove(j,i);

    for(int i =rect.top();i<=rect.bottom();i++)
        for(int j=0;j<=(rect.right()-rect.left());j++)
                m_cells.unshiftRow( QPoint(rect.left(),i) );

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        for(int i=rect.top();i<=rect.bottom();i++)
                it.current()->changeNameCellRef( QPoint(rect.left(),i), false, KSpreadTable::ColumnRemove, name(),(rect.right()-rect.left()+1) );

    refreshChart(QPoint(rect.left(),rect.top()), false, KSpreadTable::ColumnRemove );
    refreshMergedCell();
    recalc(true);
    emit sig_updateView( this );
}

bool KSpreadTable::insertColumn( int col, int nbCol,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked() && makeUndo)
    {
        KSpreadUndoInsertColumn *undo = new KSpreadUndoInsertColumn( m_pDoc, this, col,nbCol );
        m_pDoc->undoBuffer()->appendUndo( undo  );
    }

    bool res=true;
    for(int i=0;i<=nbCol;i++)
        {
        bool result = m_cells.insertColumn( col+i );
        m_columns.insertColumn( col+i);
        if(!result)
                res=false;
        }

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        it.current()->changeNameCellRef( QPoint( col, 1 ), true, KSpreadTable::ColumnInsert, name() ,nbCol+1);
    refreshChart( QPoint( col, 1 ), true, KSpreadTable::ColumnInsert );
    refreshMergedCell();
    recalc(true);
    emit sig_updateHBorder( this );
    emit sig_updateView( this );

    return res;
}

bool KSpreadTable::insertRow( int row,int nbRow,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked() && makeUndo)
    {
        KSpreadUndoInsertRow *undo = new KSpreadUndoInsertRow( m_pDoc, this, row,nbRow );
        m_pDoc->undoBuffer()->appendUndo( undo  );
    }

    bool res=true;
    for(int i=0;i<=nbRow;i++)
        {
        bool result = m_cells.insertRow( row+i );
        m_rows.insertRow( row );
        if(!result)
                res=false;
        }

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        it.current()->changeNameCellRef( QPoint( 1, row ), true, KSpreadTable::RowInsert, name() ,nbRow+1);
    refreshChart( QPoint( 1, row ), true, KSpreadTable::RowInsert );
    refreshMergedCell();
    recalc(true);
    emit sig_updateVBorder( this );
    emit sig_updateView( this );

    return res;
}

void KSpreadTable::removeColumn( int col,int nbCol,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked() && makeUndo)
    {
        KSpreadUndoRemoveColumn *undo = new KSpreadUndoRemoveColumn( m_pDoc, this, col, nbCol );
        m_pDoc->undoBuffer()->appendUndo( undo  );
    }

    for(int i=0;i<=nbCol;i++)
        {
        m_cells.removeColumn( col );
        m_columns.removeColumn( col );
        }

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        it.current()->changeNameCellRef( QPoint( col, 1 ), true, KSpreadTable::ColumnRemove, name(),nbCol+1 );
    refreshChart( QPoint( col, 1 ), true, KSpreadTable::ColumnRemove );
    recalc(true);
    refreshMergedCell();
    emit sig_updateHBorder( this );
    emit sig_updateView( this );
}

void KSpreadTable::removeRow( int row,int nbRow,bool makeUndo )
{
    if ( !m_pDoc->undoBuffer()->isLocked() && makeUndo)
    {
        KSpreadUndoRemoveRow *undo = new KSpreadUndoRemoveRow( m_pDoc, this, row,nbRow );
        m_pDoc->undoBuffer()->appendUndo( undo  );
    }

    for(int i=0;i<=nbRow;i++)
        {
        m_cells.removeRow( row );
        m_rows.removeRow( row );
        }
    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        it.current()->changeNameCellRef( QPoint( 1, row ), true, KSpreadTable::RowRemove, name(),nbRow+1 );
    refreshChart(QPoint( 1, row ), true, KSpreadTable::RowRemove);
    recalc(true);
    refreshMergedCell();
    emit sig_updateVBorder( this );
    emit sig_updateView( this );
}

void KSpreadTable::hideRow( int _row,int nbRow, QValueList<int>_list )
{
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
      KSpreadUndoHideRow *undo ;
      if(nbRow!=-1)
	undo= new KSpreadUndoHideRow( m_pDoc, this, _row,nbRow );
      else
	undo= new KSpreadUndoHideRow( m_pDoc, this, _row,nbRow,_list );
      m_pDoc->undoBuffer()->appendUndo( undo  );
    }

    if(nbRow!=-1)
      {
	for(int i=0;i<=nbRow;i++)
	  {
	    RowLayout *rl=nonDefaultRowLayout(_row+i);
	    rl->setHide(true);
	  }
      }
    else
      {
	QValueList<int>::Iterator it;
	for( it = _list.begin(); it != _list.end(); ++it )
	  {
	    RowLayout *rl=nonDefaultRowLayout(*it);
	    rl->setHide(true);
	  }
      }
    emitHideRow();
}

void KSpreadTable::emitHideRow()
{
    emit sig_updateVBorder( this );
    emit sig_updateView( this );
}

void KSpreadTable::showRow( int _row,int nbRow,QValueList<int>_list )
{
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
      KSpreadUndoShowRow *undo;
      if(nbRow!=-1)
        undo = new KSpreadUndoShowRow( m_pDoc, this, _row,nbRow );
      else
	undo = new KSpreadUndoShowRow( m_pDoc, this, _row,nbRow,_list );
      m_pDoc->undoBuffer()->appendUndo( undo  );
    }
    if(nbRow!=-1)
      {
	for(int i=0;i<=nbRow;i++)
	  {
	    RowLayout *rl=nonDefaultRowLayout(_row+i);
	    rl->setHide(false);
	  }
      }
    else
      {
	QValueList<int>::Iterator it;
	for( it = _list.begin(); it != _list.end(); ++it )
	  {
	    RowLayout *rl=nonDefaultRowLayout(*it);
	    rl->setHide(false);
	  }
      }
    emit sig_updateVBorder( this );
    emit sig_updateView( this );
}


void KSpreadTable::hideColumn( int _col,int nbCol,QValueList<int>_list )
{
   if ( !m_pDoc->undoBuffer()->isLocked() )
    {
        KSpreadUndoHideColumn *undo;
	if(nbCol!=-1)
	  undo= new KSpreadUndoHideColumn( m_pDoc, this, _col,nbCol );
	else
	  undo= new KSpreadUndoHideColumn( m_pDoc, this, _col,nbCol,_list );
        m_pDoc->undoBuffer()->appendUndo( undo  );
    }
   if(nbCol!=-1)
     {
       for(int i=0;i<=nbCol;i++)
	 {
	   ColumnLayout *cl=nonDefaultColumnLayout(_col+i);
	   cl->setHide(true);
	 }
     }
   else
     {
       QValueList<int>::Iterator it;
       for( it = _list.begin(); it != _list.end(); ++it )
	 {
	   ColumnLayout *cl=nonDefaultColumnLayout(*it);
	   cl->setHide(true);
	 }
      }
   emitHideColumn();
}

void KSpreadTable::emitHideColumn()
{
    emit sig_updateHBorder( this );
    emit sig_updateView( this );
}


void KSpreadTable::showColumn( int _col,int nbCol,QValueList<int>_list )
{
   if ( !m_pDoc->undoBuffer()->isLocked() )
     {
       KSpreadUndoShowColumn *undo;
      if(nbCol!=-1)
        undo = new KSpreadUndoShowColumn( m_pDoc, this, _col,nbCol );
      else
	undo = new KSpreadUndoShowColumn( m_pDoc, this, _col,nbCol,_list );
      m_pDoc->undoBuffer()->appendUndo( undo  );
    }

   if(nbCol!=-1)
     {
       for(int i=0;i<=nbCol;i++)
	 {
	   ColumnLayout *cl=nonDefaultColumnLayout(_col+i);
	   cl->setHide(false);
	 }
     }
   else
     {
       QValueList<int>::Iterator it;
       for( it = _list.begin(); it != _list.end(); ++it )
	 {
	   ColumnLayout *cl=nonDefaultColumnLayout(*it);
	   cl->setHide(false);
	 }

     }
   emit sig_updateHBorder( this );
   emit sig_updateView( this );
}


void KSpreadTable::refreshChart(const QPoint & pos, bool fullRowOrColumn, ChangeRef ref)
{
KSpreadCell* c = m_cells.firstCell();
  for( ;c; c = c->nextCell() )
  {
  if((ref==ColumnInsert || ref==ColumnRemove) && fullRowOrColumn
        && c->column()>=(pos.x()-1))
        {
        if(c->updateChart())
                return;
        }
  else if((ref==ColumnInsert || ref==ColumnRemove)&& !fullRowOrColumn
        && c->column()>=(pos.x()-1) && c->row()==pos.y())
        {
        if(c->updateChart())
                return;
        }
  else if((ref==RowInsert|| ref==RowRemove) && fullRowOrColumn
        && c->row()>=(pos.y()-1))
        {
        if(c->updateChart())
                return;
        }
  else if((ref==RowInsert || ref==RowRemove)&& !fullRowOrColumn
        && c->column()==pos.x() && c->row()>=(pos.y()-1))
        {
        if(c->updateChart())
                return;
        }
  }
  //refresh chart when there is a chart and you remove
  //all cells
  if(c==0L)
        {
        CellBinding *bind=firstCellBinding();
        if(bind!=0L)
                bind->cellChanged( 0 );
        }
}

void KSpreadTable::refreshMergedCell()
{
KSpreadCell* c = m_cells.firstCell();
  for( ;c; c = c->nextCell() )
  {
  if(c->isForceExtraCells())
        c->forceExtraCells( c->column(), c->row(), c->extraXCells(), c->extraYCells() );
  }

}

QRect KSpreadTable::selectionCellMerged(const QRect &_sel)
{
QRect selection(_sel);
if(selection.bottom()==0x7FFF ||selection.right()==0x7FFF)
        return selection;
else
  {
  int top=selection.top();
  int left=selection.left();
  int bottom=selection.bottom();
  int right=selection.right();
  for ( int x = selection.left(); x <= selection.right(); x++ )
        for ( int y = selection.top(); y <= selection.bottom(); y++ )
        {
                KSpreadCell *cell = cellAt( x, y );
                if( cell->isForceExtraCells())
                {
                        right=QMAX(right,cell->extraXCells()+x);
                        bottom=QMAX(bottom,cell->extraYCells()+y);
                }
                else if ( cell->isObscured() && cell->isObscuringForced() )
                {
                        int moveX=cell->obscuringCellsColumn();
                        int moveY=cell->obscuringCellsRow();
                        cell = cellAt( moveX, moveY );
                        left=QMIN(left,moveX);
                        top=QMIN(top,moveY);
                        bottom=QMAX(bottom,moveY+cell->extraYCells());
                        right=QMAX(right,moveX+cell->extraXCells());
                }
        }

  selection.setCoords(left,top,right,bottom);
  }
  return selection;
}

void KSpreadTable::changeNameCellRef(const QPoint & pos, bool fullRowOrColumn, ChangeRef ref, QString tabname,int nbCol)
{
  bool correctDefaultTableName = (tabname == name()); // for cells without table ref (eg "A1")
  KSpreadCell* c = m_cells.firstCell();
  for( ;c; c = c->nextCell() )
  {
    if(c->isFormular())
    {
      QString origText = c->text();
      unsigned int i = 0;
      QString newText;

      bool correctTableName = correctDefaultTableName;
      //bool previousCorrectTableName = false;
      for ( ; i < origText.length(); ++i )
      {
        QChar origCh = origText[i];
        if ( origCh != ':' && origCh != '$' && !origCh.isLetter() )
        {
          newText += origCh;
          // Reset the "correct table indicator"
          correctTableName = correctDefaultTableName;
        }
        else // Letter or dollar : maybe start of cell name/range
          // (or even ':', like in a range - note that correctTable is kept in this case)
        {
          // Collect everything that forms a name (cell name or table name)
          QString str;
          for( ; i < origText.length() &&
                  (origText[i].isLetter() || origText[i].isDigit()
                   || origText[i].isSpace() || origText[i] == '$')
                   ; ++i )
            str += origText[i];

          // Was it a table name ?
          if ( origText[i] == '!' )
          {
            newText += str + '!'; // Copy it (and the '!')
            // Look for the table name right before that '!'
            correctTableName = ( newText.right( tabname.length()+1 ) == tabname+"!" );
          }
          else // It must be a cell identifier
          {
            // Parse it
            KSpreadPoint point( str );
            if (point.isValid())
            {
              int col = point.pos.x();
              int row = point.pos.y();
              // Update column
              if ( point.columnFixed )
                newText += '$' + util_columnLabel( col );
              else
              {
                if(ref==ColumnInsert
                   && correctTableName
                   && col>=pos.x()     // Column after the new one : +1
                   && ( fullRowOrColumn || row == pos.y() ) ) // All rows or just one
                {
                  newText += util_columnLabel(col+nbCol);
                }
                else if(ref==ColumnRemove
                        && correctTableName
                        && col > pos.x() // Column after the deleted one : -1
                        && ( fullRowOrColumn || row == pos.y() ) ) // All rows or just one
                {
                  newText += util_columnLabel(col-nbCol);
                }
                else
                  newText += util_columnLabel(col);
              }
              // Update row
              if ( point.rowFixed )
                newText += '$' + QString::number( row );
              else
              {
                if(ref==RowInsert
                   && correctTableName
                   && row >= pos.y() // Row after the new one : +1
                   && ( fullRowOrColumn || col == pos.x() ) ) // All columns or just one
                {
                  newText += QString::number( row+nbCol );
                }
                else if(ref==RowRemove
                        && correctTableName
                        && row > pos.y() // Column after the deleted one : -1
                        && ( fullRowOrColumn || col == pos.x() ) ) // All columns or just one
                {
                  newText += QString::number( row-nbCol );
                }
                else
                  newText += QString::number( row );
              }
            }
            else // Not a cell ref
            {
              //kdDebug(36001) << "Copying (unchanged) : " << str << endl;
              newText += str;
            }
            // Copy the char that got us to stop
            newText += origText[i];
          }
        }
      }
      c->setCellText(newText, false /* no recalc deps for each, done independently */ );
    }
  }
}

void KSpreadTable::find( const QPoint &_marker, QString _find, long options, KSpreadCanvas *canvas )
{
    // Identify the region of interest.
    QRect region( m_rctSelection );
    if (options & KoFindDialog::SelectedText)
    {
        bool selectionValid = ( m_rctSelection.left() != 0 );

        // Complete rows selected ?
        if ( selectionValid && m_rctSelection.right() == 0x7FFF )
        {
        }
        // Complete columns selected ?
        else if ( selectionValid && m_rctSelection.bottom() == 0x7FFF )
        {
        }
        else
        {
            if ( !selectionValid )
                region.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
        }
    }
    else
    {
        // All cells.
        region.setCoords( 0, 0, m_iMaxRow, m_iMaxColumn );
    }

    // Create the class that handles all the actual Find stuff, and connect it to its
    // local slots.
    KoFind dialog( _find, options );
    QObject::connect(
        &dialog, SIGNAL( highlight( const QString &, int, int, const QRect & ) ),
        canvas, SLOT( highlight( const QString &, int, int, const QRect & ) ) );

    // Now do the finding...
    QRect cellRegion( 0, 0, 0, 0 );
    bool bck = options & KoFindDialog::FindBackwards;

    int colStart = !bck ? region.left() : region.right();
    int colEnd = !bck ? region.right() : region.left();
    int rowStart = !bck ? region.top() :region.bottom();
    int rowEnd = !bck ? region.bottom() : region.top();
    if ( options & KoFindDialog::FromCursor ) {
        colStart = _marker.x();
        rowStart =  _marker.y();
    }
    for (int row = rowStart ; !bck ? row < rowEnd : row > rowEnd ; !bck ? ++row : --row )
    {
        for(int col = colStart ; !bck ? col < colEnd : col > colEnd ; !bck ? ++col : --col )
        {
            KSpreadCell *cell = cellAt( col, row );
            if ( !cell->isDefault() && !cell->isObscured() && !cell->isFormular() )
            {
                QString text = cell->text();
                cellRegion.setTop( row );
                cellRegion.setLeft( col );
                if ( !dialog.find( text, cellRegion ) )
                    return;
            }
        }
    }
}

void KSpreadTable::replace( const QPoint &_marker, QString _find, QString _replace, long options, KSpreadCanvas *canvas )
{
    // Identify the region of interest.
    QRect region( m_rctSelection );
    if (options & KoReplaceDialog::SelectedText)
    {
        bool selectionValid = ( m_rctSelection.left() != 0 );

        // Complete rows selected ?
        if ( selectionValid && m_rctSelection.right() == 0x7FFF )
        {
        }
        // Complete columns selected ?
        else if ( selectionValid && m_rctSelection.bottom() == 0x7FFF )
        {
        }
        else
        {
            if ( !selectionValid )
                region.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
        }
    }
    else
    {
        // All cells.
        region.setCoords( 0, 0, m_iMaxRow, m_iMaxColumn );
    }

    // Create the class that handles all the actual replace stuff, and connect it to its
    // local slots.
    KoReplace dialog( _find, _replace, options );
    QObject::connect(
        &dialog, SIGNAL( highlight( const QString &, int, int, const QRect & ) ),
        canvas, SLOT( highlight( const QString &, int, int, const QRect & ) ) );
    QObject::connect(
        &dialog, SIGNAL( replace( const QString &, int, int,int, const QRect & ) ),
        canvas, SLOT( replace( const QString &, int, int,int, const QRect & ) ) );

    // Now do the replacing...
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
        KSpreadUndoChangeAreaTextCell *undo = new KSpreadUndoChangeAreaTextCell( m_pDoc, this, region );
        m_pDoc->undoBuffer()->appendUndo( undo );
    }
    QRect cellRegion( 0, 0, 0, 0 );
    bool bck = options & KoFindDialog::FindBackwards;

    int colStart = !bck ? region.left() : region.right();
    int colEnd = !bck ? region.right() : region.left();
    int rowStart = !bck ? region.top() :region.bottom();
    int rowEnd = !bck ? region.bottom() : region.top();
    if ( options & KoFindDialog::FromCursor ) {
        colStart = _marker.x();
        rowStart =  _marker.y();
    }
    for (int row = rowStart ; !bck ? row < rowEnd : row > rowEnd ; !bck ? ++row : --row )
    {
        for(int col = colStart ; !bck ? col < colEnd : col > colEnd ; !bck ? ++col : --col )
        {
            KSpreadCell *cell = cellAt( col, row );
            if ( !cell->isDefault() && !cell->isObscured() && !cell->isFormular() )
            {
                QString text = cell->text();
                cellRegion.setTop( row );
                cellRegion.setLeft( col );
                if (!dialog.replace( text, cellRegion ))
                    return;
            }
        }
    }
}

void KSpreadTable::borderBottom( const QPoint &_marker,const QColor &_color )
{
    bool selected = ( m_rctSelection.left() != 0 );
    QRect r( m_rctSelection );
    if ( m_rctSelection.left()==0 )
        r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
    QPen pen( _color,1,SolidLine);

        // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            QString title=i18n("Change border");
            KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r, title );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int row = c->row();
        if ( m_rctSelection.bottom() == row &&!c->isObscuringForced())
        {
          c->clearProperty( KSpreadCell::PBottomBorder );
          c->clearNoFallBackProperties( KSpreadCell::PBottomBorder );
        }
      }

      RowLayout *rw=nonDefaultRowLayout(m_rctSelection.bottom());
      rw->setBottomBorderPen(pen);

      emit sig_updateView( this );
      return;
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      //nothing
      return;
    }
    else
    {
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            QString title=i18n("Change border");
            KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

    for ( int x = r.left(); x <= r.right(); x++ )
        {
        int y = r.bottom();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setBottomBorderPen(pen);
        }
    emit sig_updateView( this, r );
    }
}

void KSpreadTable::borderRight( const QPoint &_marker,const QColor &_color )
{
    bool selected = ( m_rctSelection.left() != 0 );
    QRect r( m_rctSelection );
    if ( m_rctSelection.left() == 0 )
        r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );


    QPen pen( _color,1,SolidLine);
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      //nothing
      return;
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {

    RowLayout* rw =m_rows.first();
      for( ; rw; rw = rw->next() )
        {
        if ( !rw->isDefault() && (rw->hasProperty(KSpreadCell::PRightBorder)))
                {
                for(int i=m_rctSelection.left();i<=m_rctSelection.right();i++)
                        {
                        KSpreadCell *cell = cellAt( i,  rw->row());
                        if ( cell == m_pDefaultCell )
                                {
                                cell = new KSpreadCell( this, i,  rw->row() );
                                m_cells.insert( cell, i,  rw->row() );
                                }
                        }
                }
        }

    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
        QString title=i18n("Change border");
        KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
        m_pDoc->undoBuffer()->appendUndo( undo );
        }

      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int col = c->column();
        if ( m_rctSelection.right() == col &&!c->isObscuringForced())
        {
          c->clearProperty( KSpreadCell::PRightBorder );
          c->clearNoFallBackProperties( KSpreadCell::PRightBorder );
        }
      }

      ColumnLayout *cl=nonDefaultColumnLayout(m_rctSelection.right());
      cl->setRightBorderPen(pen);

      rw =m_rows.first();
      for( ; rw; rw = rw->next() )
        {
        if ( !rw->isDefault() && (rw->hasProperty(KSpreadCell::PRightBorder)))
                {
                for(int i=m_rctSelection.left();i<=m_rctSelection.right();i++)
                        {
                        KSpreadCell *cell = cellAt( i,  rw->row());
                        if ( cell == m_pDefaultCell )
                                {
                                cell = new KSpreadCell( this, i,  rw->row() );
                                m_cells.insert( cell, i,  rw->row() );
                                }
                        cell->setRightBorderPen(pen);
                        }
                }
        }

      emit sig_updateView( this );
      return;
    }
    else
    {
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
        QString title=i18n("Change border");
        KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
        m_pDoc->undoBuffer()->appendUndo( undo );
        }

    for ( int y = r.top(); y <= r.bottom(); y++ )
        {
        int x = r.right();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setRightBorderPen(pen);
        }
    emit sig_updateView( this, r );
    }
}

void KSpreadTable::borderLeft( const QPoint &_marker, const QColor &_color )
{
    bool selected = ( m_rctSelection.left() != 0 );
    QString title=i18n("Change border");
    QRect r( m_rctSelection );
    if ( m_rctSelection.left()==0 )
        r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );


    QPen pen( _color,1,SolidLine);
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
    QRect rect;
    rect.setCoords(r.left(),r.top(),1,r.bottom());
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
        KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, rect,title );
        m_pDoc->undoBuffer()->appendUndo( undo );
        }

    for ( int y = r.top(); y <= r.bottom(); y++ )
        {
        int x = r.left();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setLeftBorderPen(pen);
        }
    emit sig_updateView( this, rect );
    return;
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
    RowLayout* rw =m_rows.first();
    for( ; rw; rw = rw->next() )
        {
        if ( !rw->isDefault() && (rw->hasProperty(KSpreadCell::PLeftBorder)))
                {
                for(int i=m_rctSelection.left();i<=m_rctSelection.right();i++)
                        {
                        KSpreadCell *cell = cellAt( i,  rw->row());
                        if ( cell == m_pDefaultCell )
                                {
                                cell = new KSpreadCell( this, i,  rw->row() );
                                m_cells.insert( cell, i,  rw->row() );
                                }
                        }
                }
        }

    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
        KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r, title );
        m_pDoc->undoBuffer()->appendUndo( undo );
        }

      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int col = c->column();
        if ( col==m_rctSelection.left() &&!c->isObscuringForced())
        {
          c->clearProperty( KSpreadCell::PLeftBorder );
          c->clearNoFallBackProperties( KSpreadCell::PLeftBorder );
        }
      }
      ColumnLayout *cl=nonDefaultColumnLayout(m_rctSelection.left());
      cl->setLeftBorderPen(pen);

      rw =m_rows.first();
      for( ; rw; rw = rw->next() )
        {
        if ( !rw->isDefault() && (rw->hasProperty(KSpreadCell::PLeftBorder)))
                {
                for(int i=m_rctSelection.left();i<=m_rctSelection.right();i++)
                        {
                        KSpreadCell *cell = cellAt( i,  rw->row());
                        if ( cell == m_pDefaultCell )
                                {
                                cell = new KSpreadCell( this, i,  rw->row() );
                                m_cells.insert( cell, i,  rw->row() );
                                }
                        cell->setLeftBorderPen(pen);
                        }
                }
        }

      emit sig_updateView( this );
      return;
    }
    else
    {
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
        KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
        m_pDoc->undoBuffer()->appendUndo( undo );
        }

    for ( int y = r.top(); y <= r.bottom(); y++ )
        {
        int x = r.left();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setLeftBorderPen(pen);
        }
    emit sig_updateView( this, r );
    }
}

void KSpreadTable::borderTop( const QPoint &_marker,const QColor &_color )
{
    bool selected = ( m_rctSelection.left() != 0 );
    QRect r( m_rctSelection );
    if ( m_rctSelection.left()==0 )
        r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
    QString title=i18n("Change border");
    QPen pen( _color,1,SolidLine);
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int row = c->row();
        if ( m_rctSelection.top() == row &&!c->isObscuringForced())
        {
          c->clearProperty( KSpreadCell::PTopBorder );
          c->clearNoFallBackProperties( KSpreadCell::PTopBorder );
        }
      }

      RowLayout *rw=nonDefaultRowLayout(m_rctSelection.top());
      rw->setTopBorderPen(pen);

      emit sig_updateView( this );
      return;
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
    QRect rect;
    rect.setCoords(r.left(),r.top(),r.right(),1);

    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, rect,title );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

    for ( int x = r.left(); x <= r.right(); x++ )
        {
        int y = r.top();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setTopBorderPen(pen);
        }
    emit sig_updateView( this,rect );
    return;
    }
    else
    {
    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

    for ( int x = r.left(); x <= r.right(); x++ )
        {
        int y = r.top();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setTopBorderPen(pen);
        }
    emit sig_updateView( this, r );
    }
}

void KSpreadTable::borderOutline( const QPoint &_marker,const QColor &_color )
{
    bool selected = ( m_rctSelection.left() != 0 );
    QRect r( m_rctSelection );
    if ( m_rctSelection.left()==0 )
        r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );

    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
        QString title=i18n("Change border");
        KSpreadUndoCellLayout *undo = new KSpreadUndoCellLayout( m_pDoc, this, r,title );
        m_pDoc->undoBuffer()->appendUndo( undo );
        }
    QPen pen( _color,1,SolidLine);
    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int row = c->row();
        if ( m_rctSelection.top() <= row && m_rctSelection.bottom() >= row
        &&!c->isObscuringForced())
        {
          c->clearProperty( KSpreadCell::PTopBorder );
          c->clearNoFallBackProperties( KSpreadCell::PTopBorder );
          c->clearProperty( KSpreadCell::PRightBorder );
          c->clearNoFallBackProperties( KSpreadCell::PRightBorder );
          c->clearProperty( KSpreadCell::PLeftBorder );
          c->clearNoFallBackProperties( KSpreadCell::PLeftBorder );
          c->clearProperty( KSpreadCell::PBottomBorder );
          c->clearNoFallBackProperties( KSpreadCell::PBottomBorder );
        }
      }

      RowLayout *rw=nonDefaultRowLayout(m_rctSelection.top());
      rw->setTopBorderPen(pen);
      rw=nonDefaultRowLayout(m_rctSelection.bottom());
      rw->setBottomBorderPen(pen);
      for ( int y = r.top(); y <= r.bottom(); y++ )
      {
        int x = r.left();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setLeftBorderPen(pen);
      }
      emit sig_updateView( this );
      return;
    }
    // Complete columns selected ?
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int row = c->row();
        if ( m_rctSelection.top() <= row && m_rctSelection.bottom() >= row
        &&!c->isObscuringForced())
        {
          c->clearProperty( KSpreadCell::PTopBorder );
          c->clearNoFallBackProperties( KSpreadCell::PTopBorder );
          c->clearProperty( KSpreadCell::PRightBorder );
          c->clearNoFallBackProperties( KSpreadCell::PRightBorder );
          c->clearProperty( KSpreadCell::PLeftBorder );
          c->clearNoFallBackProperties( KSpreadCell::PLeftBorder );
          c->clearProperty( KSpreadCell::PBottomBorder );
          c->clearNoFallBackProperties( KSpreadCell::PBottomBorder );
        }
      }

      ColumnLayout *cl=nonDefaultColumnLayout(m_rctSelection.left());
      cl->setLeftBorderPen(pen);
      cl=nonDefaultColumnLayout(m_rctSelection.right());
      cl->setRightBorderPen(pen);
      for ( int x = r.left(); x <= r.right(); x++ )
      {
        int y = r.top();
        KSpreadCell *cell = cellAt( x, y );
        if(!cell->isObscuringForced())
        {
         if ( cell == m_pDefaultCell )
         {
            cell = new KSpreadCell( this, x, y );
            m_cells.insert( cell, x, y );
         }
         cell->setTopBorderPen(pen);
        }
      }
    emit sig_updateView( this );
    return;
    }
    else
    {
    for ( int x = r.left(); x <= r.right(); x++ )
    {
        int y = r.top();
        KSpreadCell *cell = cellAt( x, y );
        if(!cell->isObscuringForced())
        {
         if ( cell == m_pDefaultCell )
         {
            cell = new KSpreadCell( this, x, y );
            m_cells.insert( cell, x, y );
         }
         cell->setTopBorderPen(pen);
        }
    }
    for ( int y = r.top(); y <= r.bottom(); y++ )
    {
        int x = r.left();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setLeftBorderPen(pen);
    }
    for ( int y = r.top(); y <= r.bottom(); y++ )
    {
        int x = r.right();
        KSpreadCell *cell = nonDefaultCell( x, y );
        if(!cell->isObscuringForced())
                cell->setRightBorderPen(pen);
    }
    for ( int x = r.left(); x <= r.right(); x++ )
    {
        int y = r.bottom();
        KSpreadCell *cell = cellAt( x, y );
        if(!cell->isObscuringForced())
        {
        if ( cell == m_pDefaultCell )
            {
            cell = new KSpreadCell( this, x, y );
            m_cells.insert( cell, x, y );
            }
        cell->setBottomBorderPen(pen);
        }
    }
    emit sig_updateView( this, r );
    }
}

struct SetSelectionBorderAllWorker : public KSpreadTable::CellWorkerTypeA {
    QPen pen;
    SetSelectionBorderAllWorker( const QColor& color ) : pen( color, 1, QPen::SolidLine ) { }

    QString getUndoTitle() { return i18n("Change border"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PRightBorder )
		 || rw->hasProperty( KSpreadCell::PLeftBorder )
		 || rw->hasProperty( KSpreadCell::PTopBorder )
		 || rw->hasProperty( KSpreadCell::PBottomBorder ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setTopBorderPen( pen );
        rw->setRightBorderPen( pen );
        rw->setLeftBorderPen( pen );
        rw->setBottomBorderPen( pen );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setTopBorderPen( pen );
        cl->setRightBorderPen( pen );
        cl->setLeftBorderPen( pen );
        cl->setBottomBorderPen( pen );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PTopBorder );
	c->clearNoFallBackProperties( KSpreadCell::PTopBorder );
	c->clearProperty( KSpreadCell::PBottomBorder );
	c->clearNoFallBackProperties( KSpreadCell::PBottomBorder );
	c->clearProperty( KSpreadCell::PLeftBorder );
	c->clearNoFallBackProperties( KSpreadCell::PLeftBorder );
	c->clearProperty( KSpreadCell::PRightBorder );
	c->clearNoFallBackProperties( KSpreadCell::PRightBorder );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	//if ( cellRegion )
	//    cell->setDisplayDirtyFlag();
	cell->setTopBorderPen( pen );
        cell->setRightBorderPen( pen );
        cell->setLeftBorderPen( pen );
        cell->setBottomBorderPen( pen );
	//if ( cellRegion )
	//    cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::borderAll( const QPoint &_marker,const QColor &_color )
{
    SetSelectionBorderAllWorker w( _color );
    workOnCells( _marker, w );
}

struct SetSelectionBorderRemoveWorker : public KSpreadTable::CellWorkerTypeA {
    QPen pen;
    SetSelectionBorderRemoveWorker() : pen( Qt::black, 1, Qt::NoPen  ) { }
    QString getUndoTitle() { return i18n("Change border"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PRightBorder )
		 || rw->hasProperty( KSpreadCell::PLeftBorder )
		 || rw->hasProperty( KSpreadCell::PTopBorder )
		 || rw->hasProperty( KSpreadCell::PBottomBorder )
		 || rw->hasProperty( KSpreadCell::PFallDiagonal )
		 || rw->hasProperty( KSpreadCell::PGoUpDiagonal ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setTopBorderPen( pen );
        rw->setRightBorderPen( pen );
        rw->setLeftBorderPen( pen );
        rw->setBottomBorderPen( pen);
        rw->setFallDiagonalPen( pen );
        rw->setGoUpDiagonalPen (pen );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setTopBorderPen( pen );
        cl->setRightBorderPen( pen );
        cl->setLeftBorderPen( pen );
        cl->setBottomBorderPen( pen);
        cl->setFallDiagonalPen( pen );
        cl->setGoUpDiagonalPen (pen );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PTopBorder );
	c->clearNoFallBackProperties( KSpreadCell::PTopBorder );
	c->clearProperty( KSpreadCell::PLeftBorder );
	c->clearNoFallBackProperties( KSpreadCell::PLeftBorder );
	c->clearProperty( KSpreadCell::PRightBorder );
	c->clearNoFallBackProperties( KSpreadCell::PRightBorder );
	c->clearProperty( KSpreadCell::PBottomBorder );
	c->clearNoFallBackProperties( KSpreadCell::PBottomBorder );
	c->clearProperty( KSpreadCell::PFallDiagonal );
	c->clearNoFallBackProperties( KSpreadCell::PFallDiagonal );
	c->clearProperty( KSpreadCell::PGoUpDiagonal );
	c->clearNoFallBackProperties( KSpreadCell::PGoUpDiagonal );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	//if ( cellRegion )
	//    cell->setDisplayDirtyFlag();
	cell->setTopBorderPen( pen );
        cell->setRightBorderPen( pen );
        cell->setLeftBorderPen( pen );
        cell->setBottomBorderPen( pen);
        cell->setFallDiagonalPen( pen );
        cell->setGoUpDiagonalPen (pen );
	//if ( cellRegion )
	//    cell->clearDisplayDirtyFlag();
    }
};


void KSpreadTable::borderRemove( const QPoint &_marker )
{
    SetSelectionBorderRemoveWorker w;
    workOnCells( _marker, w );
}


void KSpreadTable::sortByRow( int ref_row, SortingOrder mode )
{
    QRect r( selectionRect() );
    ASSERT( mode == Increase || mode == Decrease );

    // It may not happen that entire columns are selected.
    ASSERT( r.right() != 0x7fff );

    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
        KSpreadUndoChangeAreaTextCell *undo = new KSpreadUndoChangeAreaTextCell( m_pDoc, this, r );
        m_pDoc->undoBuffer()->appendUndo( undo );
    }
    // Are entire rows selected ?
    if ( r.right() == 0x7FFF )
    {
        r.setLeft( 0x7fff );
        r.setRight( 0 );

        // Determine a correct left and right.
        // Iterate over all cells to find out which cells are
        // located in the selected rows.
        KSpreadCell* c = m_cells.firstCell();
        for( ; c; c = c->nextCell() )
        {
            int row = c->row();
            int col = c->column();

            // Is the cell in the selected columns ?
            if ( !c->isEmpty() && row >= r.top() && row <= r.bottom())
            {
                if ( col > r.right() )
                    r.rRight() = col;
                if ( col < r.left() )
                    r.rLeft() = col;
            }
        }

        // Any cells to sort here ?
        if ( r.right() < r.left() )
            return;
    }

    // Sorting algorithm: David's :). Well, I guess it's called minmax or so.
    // For each column, we look for all cells right hand of it and we find the one to swap with it.
    // Much faster than the awful bubbleSort...
    for ( int d = r.left();  d <= r.right(); d++ )
    {
        KSpreadCell *cell1 = cellAt( d, ref_row  );
        if ( cell1->isObscured() && cell1->isObscuringForced() )
        {
            int moveX=cell1->obscuringCellsColumn();
            KSpreadCell* cell = cellAt(moveX,ref_row);
            cell1 = cellAt( moveX+cell->extraXCells()+1,moveX );
            d=moveX+cell->extraXCells()+1;
        }

        // Look for which column we want to swap with the one number d
        KSpreadCell * bestCell = cell1;
        int bestX = d;

        for ( int x = d + 1 ; x <= r.right(); x++ )
        {
            KSpreadCell *cell2 = cellAt( x, ref_row );

            if ( cell2->isEmpty() )
            { /* No need to swap */ }
            else if ( cell2->isObscured() && cell2->isObscuringForced() )
            { /* No need to swap */}
            else if ( bestCell->isEmpty() )
            {
                // empty cells are always shifted to the end
                bestCell = cell2;
                bestX = x;
            }
            // Here we use the operators < and > for cells, which do it all.
            else if ( (mode == Increase && *cell2 < *bestCell) ||
                      (mode == Decrease && *cell2 > *bestCell) )
            {
                bestCell = cell2;
                bestX = x;
            }
        }

        // Swap columns cell1 and bestCell (i.e. d and bestX)
        if ( d != bestX )
        {
            for( int y = r.top(); y <= r.bottom(); y++ )
                swapCells( d, y, bestX, y );
        }

    }

    emit sig_updateView( this, r );
}

void KSpreadTable::sortByColumn(int ref_column,SortingOrder mode)
{
    ASSERT( mode == Increase || mode == Decrease );

    QRect r( selectionRect() );
    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
        KSpreadUndoChangeAreaTextCell *undo = new KSpreadUndoChangeAreaTextCell( m_pDoc, this, r );
        m_pDoc->undoBuffer()->appendUndo( undo );
    }
    // It may not happen that entire rows are selected.
    ASSERT( r.right() != 0x7fff );

    // Are entire columns selected ?
    if ( r.bottom() == 0x7FFF )
    {
        r.setTop( 0x7fff );
        r.setBottom( 0 );

        // Determine a correct top and bottom.
        // Iterate over all cells to find out which cells are
        // located in the selected columns.
        KSpreadCell* c = m_cells.firstCell();
        for( ; c; c = c->nextCell() )
        {
            int row = c->row();
            int col = c->column();

            // Is the cell in the selected columns ?
            if ( !c->isEmpty() && col >= r.left() && col <= r.right())
            {
                if ( row > r.bottom() )
                    r.rBottom() = row;
                if ( row < r.top() )
                    r.rTop() = row;
            }
        }

        // Any cells to sort here ?
        if ( r.bottom() < r.top() )
            return;
    }

    // Sorting algorithm: David's :). Well, I guess it's called minmax or so.
    // For each row, we look for all rows under it and we find the one to swap with it.
    // Much faster than the awful bubbleSort...
    // Torben: Asymptotically it is alltogether O(n^2) :-)
    for ( int d = r.top(); d <= r.bottom(); d++ )
    {
        // Look for which row we want to swap with the one number d
        KSpreadCell *cell1 = cellAt( ref_column, d );
        //kdDebug() << "New ref row " << d << endl;
        if ( cell1->isObscured() && cell1->isObscuringForced() )
        {
            int moveY=cell1->obscuringCellsRow();
            KSpreadCell* cell = cellAt(ref_column, moveY);
            cell1 = cellAt( ref_column, moveY+cell->extraYCells()+1 );
            d=moveY+cell->extraYCells()+1;
        }
        KSpreadCell * bestCell = cell1;
        int bestY = d;

        for ( int y = d + 1 ; y <= r.bottom(); y++ )
        {
            KSpreadCell *cell2 = cellAt( ref_column, y );

            if ( cell2->isEmpty() )
            { /* No need to swap */ }
            else if ( cell2->isObscured() && cell2->isObscuringForced() )
            { /* No need to swap */}
            else if ( bestCell->isEmpty() )
            {
                // empty cells are always shifted to the end
                bestCell = cell2;
                bestY = y;
            }
            // Here we use the operators < and > for cells, which do it all.
            else if ( (mode==Increase && *cell2 < *bestCell) ||
                 (mode==Decrease && *cell2 > *bestCell) )
            {
                bestCell = cell2;
                bestY = y;
            }
        }

        // Swap rows cell1 and bestCell (i.e. d and bestY)
        if ( d != bestY )
        {
            for(int x=r.left();x<=r.right();x++)
                swapCells( x, d, x, bestY );
        }
    }

    emit sig_updateView( this, r );
}

void KSpreadTable::swapCells( int x1, int y1, int x2, int y2 )
{
  KSpreadCell *ref1 = cellAt( x1, y1 );
  KSpreadCell *ref2 = cellAt( x2, y2 );
  if ( ref1->isDefault() )
  {
    if ( !ref2->isDefault() )
    {
      ref1 = nonDefaultCell( x1, y1 );
      // TODO : make ref2 default instead of copying a default cell into it
    }
    else
      return; // nothing to do
  }
  else
    if ( ref2->isDefault() )
    {
      ref2 = nonDefaultCell( x2, y2 );
      // TODO : make ref1 default instead of copying a default cell into it
    }

  // Dummy cell used for swapping cells.
  // In fact we copy only content and no layout
  // information. Imagine sortting in a table. Swapping
  // the layout while sorting is not what you would expect
  // as a user.
  KSpreadCell *tmp = new KSpreadCell( this, -1, -1 );
  tmp->copyContent( ref1 );
  ref1->copyContent( ref2 );
  ref2->copyContent( tmp );
  delete tmp;
}

void KSpreadTable::refreshPreference()
{
  if(getAutoCalc())
        recalc();

  emit sig_updateHBorder( this );
  emit sig_updateView( this );
}


bool KSpreadTable::areaIsEmpty()
{
    bool selected = ( m_rctSelection.left() != 0 );

    // Complete rows selected ?
    if ( selected && m_rctSelection.right() == 0x7FFF )
    {
      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
      int row = c->row();
      if ( m_rctSelection.top() <= row && m_rctSelection.bottom() >= row
      &&!c->isObscuringForced() && !c->text().isEmpty())
        {
        return false;
        }
      }
    }
    else if ( selected && m_rctSelection.bottom() == 0x7FFF )
    {
      KSpreadCell* c = m_cells.firstCell();
      for( ;c; c = c->nextCell() )
      {
        int col = c->column();
        if ( m_rctSelection.left() <= col && m_rctSelection.right() >= col
        &&!c->isObscuringForced() && !c->text().isEmpty())
        {
        return false;
        }
      }
    }
    else
    {
        QRect r( m_rctSelection );
        if ( !selected )
            r.setCoords( marker().x(), marker().y(), marker().x(), marker().y() );

        for ( int x = r.left(); x <= r.right(); x++ )
            for ( int y = r.top(); y <= r.bottom(); y++ )
            {
                KSpreadCell *cell = cellAt( x, y );
                if(!cell->isObscuringForced() && !cell->text().isEmpty())
                {
                return false;
                }
            }
    }
    return true;
}

struct SetSelectionMultiRowWorker : public KSpreadTable::CellWorker {
    bool enable;
    SetSelectionMultiRowWorker( bool _enable ) : KSpreadTable::CellWorker( ), enable( _enable ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Multirow");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	cell->setMultiRow( enable );
	cell->setVerticalText( false );
	cell->setAngle( 0 );
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionMultiRow( const QPoint &_marker, bool enable )
{
    SetSelectionMultiRowWorker w( enable );
    workOnCells( _marker, w );
}


struct SetSelectionAlignWorker : public KSpreadTable::CellWorkerTypeA {
    KSpreadLayout::Align _align;
    SetSelectionAlignWorker( KSpreadLayout::Align align ) : _align( align ) { }
    QString getUndoTitle() { return i18n("Change horizontal alignment"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PAlign ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setAlign( _align );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setAlign( _align );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PAlign );
	c->clearNoFallBackProperties( KSpreadCell::PAlign );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setAlign( _align );
	if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};


void KSpreadTable::setSelectionAlign( const QPoint &_marker, KSpreadLayout::Align _align )
{
    SetSelectionAlignWorker w( _align );
    workOnCells( _marker, w );
}


struct SetSelectionAlignYWorker : public KSpreadTable::CellWorkerTypeA {
    KSpreadLayout::AlignY _alignY;
    SetSelectionAlignYWorker( KSpreadLayout::AlignY alignY ) : _alignY( alignY ) { }
    QString getUndoTitle() { return i18n("Change vertical alignment"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PAlignY ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setAlignY( _alignY );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setAlignY( _alignY );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PAlignY );
	c->clearNoFallBackProperties( KSpreadCell::PAlignY );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setAlignY( _alignY );
	if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};


void KSpreadTable::setSelectionAlignY( const QPoint &_marker, KSpreadLayout::AlignY _alignY )
{
    SetSelectionAlignYWorker w( _alignY );
    workOnCells( _marker, w );
}


struct SetSelectionPrecisionWorker : public KSpreadTable::CellWorker {
    int _delta;
    SetSelectionPrecisionWorker( int delta ) : KSpreadTable::CellWorker( ), _delta( delta ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Change precision");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setDisplayDirtyFlag();
	if ( _delta == 1 )
	    cell->incPrecision();
	else
	    cell->decPrecision();
	cell->clearDisplayDirtyFlag();
    }
};

void KSpreadTable::setSelectionPrecision( const QPoint &_marker, int _delta )
{
    SetSelectionPrecisionWorker w( _delta );
    workOnCells( _marker, w );
}


struct SetSelectionMoneyFormatWorker : public KSpreadTable::CellWorkerTypeA {
    bool b;
    KSpreadDoc *m_pDoc;
    SetSelectionMoneyFormatWorker( bool _b,KSpreadDoc* _doc ) : b( _b ), m_pDoc(_doc) { }
    QString getUndoTitle() { return i18n("Format money"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PFormatNumber )
		 || rw->hasProperty( KSpreadCell::PPrecision )
		 || rw->hasProperty( KSpreadCell::PFaktor ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setFormatNumber( b ? KSpreadCell::Money : KSpreadCell::Number );
	rw->setFaktor( 1.0 );
	rw->setPrecision( b ? m_pDoc->locale()->fracDigits() : 0 );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setFormatNumber( b ? KSpreadCell::Money : KSpreadCell::Number );
	cl->setFaktor( 1.0 );
	cl->setPrecision( b ? m_pDoc->locale()->fracDigits() : 0 );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PFaktor );
	c->clearNoFallBackProperties( KSpreadCell::PFaktor );
	c->clearProperty( KSpreadCell::PPrecision );
	c->clearNoFallBackProperties( KSpreadCell::PPrecision );
	c->clearProperty( KSpreadCell::PFormatNumber );
	c->clearNoFallBackProperties( KSpreadCell::PFormatNumber );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int, int ) {
	if ( cellRegion )
	    cell->setDisplayDirtyFlag();
	cell->setFormatNumber( b ? KSpreadCell::Money : KSpreadCell::Number );
	cell->setFaktor( 1.0 );
	cell->setPrecision( b ?  m_pDoc->locale()->fracDigits() : 0 );
	if ( cellRegion )
	    cell->clearDisplayDirtyFlag();
    }
};


void KSpreadTable::setSelectionMoneyFormat( const QPoint &_marker, bool b )
{
    SetSelectionMoneyFormatWorker w( b,doc() );
    workOnCells( _marker, w );
}


struct IncreaseIndentWorker : public KSpreadTable::CellWorkerTypeA {
    int tmpIndent, valIndent;
    IncreaseIndentWorker( int _tmpIndent, int _valIndent ) : tmpIndent( _tmpIndent ), valIndent( _valIndent ) { }
    QString getUndoTitle() { return i18n("Increase indent"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PIndent ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setIndent( tmpIndent+valIndent );
	rw->setAlign( KSpreadCell::Left );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setIndent( tmpIndent+valIndent );
	cl->setAlign( KSpreadCell::Left );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PIndent );
	c->clearNoFallBackProperties( KSpreadCell::PIndent );
	c->clearProperty( KSpreadCell::PAlign );
	c->clearNoFallBackProperties( KSpreadCell::PAlign );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int x, int y ) {
	if ( cellRegion ) {
	    if(cell->align(x,y)!=KSpreadCell::Left)
	    {
		cell->setAlign(KSpreadCell::Left);
		cell->setIndent(0);
	    }
	    cell->setDisplayDirtyFlag();
	    cell->setIndent( /* ### ??? --> */ cell->getIndent(x,y) /* <-- */ +valIndent );
	    cell->clearDisplayDirtyFlag();
	} else {
	    cell->setIndent( tmpIndent+valIndent);
	    cell->setAlign( KSpreadCell::Left);
	}
    }
};


void KSpreadTable::increaseIndent( const QPoint &_marker )
{
    int valIndent = doc()->getIndentValue();
    KSpreadCell* c = cellAt( _marker.x(), _marker.y() );
    int tmpIndent = c->getIndent( _marker.x(), _marker.y() );

    IncreaseIndentWorker w( tmpIndent, valIndent );
    workOnCells( _marker, w );
}


struct DecreaseIndentWorker : public KSpreadTable::CellWorkerTypeA {
    int tmpIndent, valIndent;
    DecreaseIndentWorker( int _tmpIndent, int _valIndent ) : tmpIndent( _tmpIndent ), valIndent( _valIndent ) { }
    QString getUndoTitle() { return i18n("Decrease indent"); }
    bool testCondition( RowLayout* rw ) {
	return ( rw->hasProperty( KSpreadCell::PIndent ) );
    }
    void doWork( RowLayout* rw ) {
	rw->setIndent( (tmpIndent-valIndent)>=0 ? tmpIndent-valIndent : 0 );
    }
    void doWork( ColumnLayout* cl ) {
	cl->setIndent( (tmpIndent-valIndent)>=0 ? tmpIndent-valIndent : 0 );
    }
    void prepareCell( KSpreadCell* c ) {
	c->clearProperty( KSpreadCell::PIndent );
	c->clearNoFallBackProperties( KSpreadCell::PIndent );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscuringForced() );
    }
    void doWork( KSpreadCell* cell, bool cellRegion, int x, int y ) {
	if ( cellRegion ) {
	    cell->setDisplayDirtyFlag();
	    cell->setIndent( (cell->getIndent(x,y)-valIndent)>=0 ? /* ### ??? --> */ cell->getIndent(x,y) /* <-- */ -valIndent : 0 );
	    cell->clearDisplayDirtyFlag();
	} else {
	    cell->setIndent( (tmpIndent-valIndent)>=0 ? tmpIndent-valIndent : 0 );
	}
    }
};


void KSpreadTable::decreaseIndent( const QPoint &_marker )
{
    int valIndent = doc()->getIndentValue();
    KSpreadCell* c = cellAt( _marker.x(), _marker.y() );
    int tmpIndent = c->getIndent( _marker.x(), _marker.y() );

    DecreaseIndentWorker w( tmpIndent, valIndent );
    workOnCells( _marker, w );
}


int KSpreadTable::adjustColumn( const QPoint& _marker, int _col )
{
    int long_max=0;
    if( _col == -1 )
    {
        if ( m_rctSelection.left() != 0 && m_rctSelection.bottom() == 0x7FFF )
        {
            KSpreadCell* c = m_cells.firstCell();
            for( ;c; c = c->nextCell() )
            {
                int col = c->column();
                if ( m_rctSelection.left() <= col && m_rctSelection.right() >= col )
                {
                    if( !c->isEmpty() && !c->isObscured())
                    {
                        c->conditionAlign(painter(),col,c->row());
                        if( c->textWidth() > long_max )
                                {
                                int indent=0;
                                int a = c->align(c->column(),c->row());
                                if ( a == KSpreadCell::Undefined )
                                        {
                                        if ( c->isValue() || c->isDate() || c->isTime())
                                                a = KSpreadCell::Right;
                                        else
                                                a = KSpreadCell::Left;
                                        }

                                if(  a==KSpreadCell::Left)
                                        indent=c->getIndent(c->column(),c->row() );
                                long_max = indent+c->textWidth() +
                                       c->leftBorderWidth(c->column(),c->row() ) +
                                       c->rightBorderWidth(c->column(),c->row() );
                                }

                    }
                }
            }
        }

    }
    else
    {
        QRect r( m_rctSelection );
        if( r.left() == 0 || r.right() == 0 || r.top() == 0 || r.bottom() == 0 )
        {
            r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
        }

        if ( m_rctSelection.left() != 0 && m_rctSelection.bottom() == 0x7FFF )
        {
            KSpreadCell* c = m_cells.firstCell();
            for( ;c; c = c->nextCell() )
            {
                int col = c->column();
                if ( m_rctSelection.left() <= col && m_rctSelection.right() >= col )
                {
                    if( !c->isEmpty() && !c->isObscured())
                    {
                        c->conditionAlign(painter(),col,c->row());
                        if( c->textWidth() > long_max )
                                {
                                int indent=0;
                                int a = c->align(c->column(),c->row());
                                if ( a == KSpreadCell::Undefined )
                                        {
                                        if ( c->isValue() || c->isDate() || c->isTime())
                                                a = KSpreadCell::Right;
                                        else
                                                a = KSpreadCell::Left;
                                        }

                                if(  a==KSpreadCell::Left)
                                        indent=c->getIndent(c->column(),c->row() );
                                long_max = indent+c->textWidth() +
                                       c->leftBorderWidth(c->column(),c->row() ) +
                                       c->rightBorderWidth(c->column(),c->row() );
                                }

                    }
                }
            }
        }
        else
        {
        int x = _col;
        for ( int y = r.top(); y <= r.bottom(); y++ )
        {
            KSpreadCell *cell = cellAt( x, y );
            if( cell != m_pDefaultCell && !cell->isEmpty()
            && !cell->isObscured())
            {
                   cell->conditionAlign(painter(),x,y);
		   if(cell->textWidth() > long_max )
                                {
                                int indent=0;

                                int a = cell->align(x,y);
                                if ( a == KSpreadCell::Undefined )
                                        {
                                        if ( cell->isValue() || cell->isDate() || cell->isTime())
                                                a = KSpreadCell::Right;
                                        else
                                                a = KSpreadCell::Left;
                                        }

                                if(  a==KSpreadCell::Left)
                                        indent=cell->getIndent(x,y );

                                long_max = indent+cell->textWidth() +
                                cell->leftBorderWidth(cell->column(),cell->row() ) +
                                cell->rightBorderWidth(cell->column(),cell->row() );
                                }

                        }
                }
        }
    }
    //add 4 because long_max is the long of the text
    //but column has borders
    if( long_max == 0 )
        return -1;
    else
        return ( long_max + 4 );
}

int KSpreadTable::adjustRow(const QPoint &_marker,int _row)
{
    int long_max=0;
    if(_row==-1)
    {
        if ( m_rctSelection.left() != 0 && m_rctSelection.right() == 0x7FFF )
        {
            KSpreadCell* c = m_cells.firstCell();
            for( ;c; c = c->nextCell() )
            {
                int row = c->row();
                if ( m_rctSelection.top() <= row && m_rctSelection.bottom() >= row )
                {
                    if(!c->isEmpty() && !c->isObscured())
                    {
                        c->conditionAlign(painter(),c->column(),row);
                        if(c->textHeight()>long_max)
                            long_max = c->textHeight() +
                                       c->topBorderWidth(c->column(),c->row() ) +
                                       c->bottomBorderWidth(c->column(),c->row() );

                    }
                }
            }
        }
    }
    else
    {
        QRect r( m_rctSelection );
        if( r.left() == 0 || r.right() == 0 || r.top() == 0 || r.bottom() == 0 )
        {
            r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
        }

        if ( m_rctSelection.left() != 0 && m_rctSelection.right() == 0x7FFF )
        {
            KSpreadCell* c = m_cells.firstCell();
            for( ;c; c = c->nextCell() )
            {
                int row = c->row();
                if ( m_rctSelection.top() <= row && m_rctSelection.bottom() >= row )
                {
                    if(!c->isEmpty() && !c->isObscured())
                    {
                        c->conditionAlign(painter(),c->column(),row);
                        if(c->textHeight()>long_max)
                            long_max = c->textHeight() +
                                       c->topBorderWidth(c->column(),c->row() ) +
                                       c->bottomBorderWidth(c->column(),c->row() );

                    }
                }
            }
        }
        else
        {
        int y=_row;
        for ( int x = r.left(); x <= r.right(); x++ )
                {
                KSpreadCell *cell = cellAt( x, y );
                if(cell != m_pDefaultCell && !cell->isEmpty()
                        && !cell->isObscured())
                        {
                        cell->conditionAlign(painter(),x,y);
                        if(cell->textHeight()>long_max )
                                long_max = cell->textHeight() +
                                        cell->topBorderWidth(cell->column(),cell->row() ) +
                                        cell->bottomBorderWidth(cell->column(),cell->row() );
                        }
                }
        }
    }
    //add 4 because long_max is the long of the text
    //but column has borders
    if( long_max == 0 )
        return -1;
    else
        return ( long_max + 4 );
}

struct ClearTextSelectionWorker : public KSpreadTable::CellWorker {
    ClearTextSelectionWorker( ) : KSpreadTable::CellWorker( ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoChangeAreaTextCell( doc, table, r );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscured() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->setCellText( "" );
    }
};

void KSpreadTable::clearTextSelection( const QPoint &_marker )
{
    if(areaIsEmpty())
        return;
    ClearTextSelectionWorker w;
    workOnCells( _marker, w );
}


struct ClearValiditySelectionWorker : public KSpreadTable::CellWorker {
    ClearValiditySelectionWorker( ) : KSpreadTable::CellWorker( ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoConditional( doc, table, r );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscured() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->removeValidity();
    }
};

void KSpreadTable::clearValiditySelection( const QPoint &_marker )
{
    if(areaIsEmpty())
        return;
    ClearValiditySelectionWorker w;
    workOnCells( _marker, w );
}


struct ClearConditionalSelectionWorker : public KSpreadTable::CellWorker {
    ClearConditionalSelectionWorker( ) : KSpreadTable::CellWorker( ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoConditional( doc, table, r );
    }
    bool testCondition( KSpreadCell* cell ) {
	return ( !cell->isObscured() );
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->removeFirstCondition();
	cell->removeSecondCondition();
	cell->removeThirdCondition();
    }
};

void KSpreadTable::clearConditionalSelection( const QPoint &_marker )
{
    if(areaIsEmpty())
        return;
    ClearConditionalSelectionWorker w;
    workOnCells( _marker, w );
}


struct DefaultSelectionWorker : public KSpreadTable::CellWorker {
    DefaultSelectionWorker( ) : KSpreadTable::CellWorker( true, false, true ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
        QString title=i18n("Default parameters");
	return new KSpreadUndoCellLayout( doc, table, r, title );
    }
    bool testCondition( KSpreadCell* ) {
	return true;
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	cell->defaultStyle();
    }
};

void KSpreadTable::defaultSelection( const QPoint &_marker )
{
    DefaultSelectionWorker w;
    SelectionType st = workOnCells( _marker, w );
    switch ( st ) {
    case CompleteRows:
	for ( int i=m_rctSelection.top(); i<=m_rctSelection.bottom(); i++ ) {
	    RowLayout *rw=nonDefaultRowLayout( i );
	    rw->defaultStyleLayout();
	}
	emit sig_updateView( this, m_rctSelection );
	return;
    case CompleteColumns:
	for ( int i=m_rctSelection.left(); i<=m_rctSelection.right(); i++ ) {
	    ColumnLayout *cl=nonDefaultColumnLayout( i );
	    cl->defaultStyleLayout();
	}
	emit sig_updateView( this, m_rctSelection );
	return;
    case CellRegion:
	QRect r( m_rctSelection );
	if ( m_rctSelection.left() == 0 )
	    r.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
	emit sig_updateView( this, r );
	return;
    }
}


struct SetConditionalWorker : public KSpreadTable::CellWorker {
    KSpreadConditional* tmp;
    SetConditionalWorker( KSpreadConditional* _tmp ) : KSpreadTable::CellWorker( ), tmp( _tmp ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoConditional( doc, table, r );
    }
    bool testCondition( KSpreadCell* ) {
        return true;
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	if ( !cell->isObscured() ) {
	    KSpreadConditional *tmpCondition = 0L;
	    cell->setDisplayDirtyFlag();
	    for(int i=0;i<3;i++)
	    {
		if(tmp[i].m_cond==None)
		    switch ( i ) {
                    case 0: cell->removeFirstCondition(); break;
		    case 1: cell->removeSecondCondition(); break;
		    case 2: cell->removeThirdCondition(); break;
		    }
		else
		{
		    switch ( i ) {
		    case 0: tmpCondition=cell->getFirstCondition(); break;
		    case 1: tmpCondition=cell->getSecondCondition(); break;
		    case 2: tmpCondition=cell->getThirdCondition(); break;
		    }
		    tmpCondition->val1=tmp[i].val1;
		    tmpCondition->val2=tmp[i].val2;
		    tmpCondition->colorcond=tmp[i].colorcond;
		    tmpCondition->fontcond=tmp[i].fontcond;
		    tmpCondition->m_cond=tmp[i].m_cond;
		}
	    }
	    cell->clearDisplayDirtyFlag();
	}
    }
};

void KSpreadTable::setConditional( const QPoint &_marker,KSpreadConditional tmp[3] )
{
    SetConditionalWorker w( tmp );
    workOnCells( _marker, w );
}


struct SetValidityWorker : public KSpreadTable::CellWorker {
    KSpreadValidity tmp;
    SetValidityWorker( KSpreadValidity _tmp ) : KSpreadTable::CellWorker( ), tmp( _tmp ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoConditional( doc, table, r );
    }
    bool testCondition( KSpreadCell* ) {
        return true;
    }
    void doWork( KSpreadCell* cell, bool, int, int ) {
	if ( !cell->isObscured() ) {
	    cell->setDisplayDirtyFlag();
	    if ( tmp.m_allow==Allow_All )
		cell->removeValidity();
	    else
	    {
		KSpreadValidity *tmpValidity = cell->getValidity();
		tmpValidity->message=tmp.message;
		tmpValidity->title=tmp.title;
		tmpValidity->valMin=tmp.valMin;
		tmpValidity->valMax=tmp.valMax;
		tmpValidity->m_cond=tmp.m_cond;
		tmpValidity->m_action=tmp.m_action;
		tmpValidity->m_allow=tmp.m_allow;
		tmpValidity->timeMin=tmp.timeMin;
		tmpValidity->timeMax=tmp.timeMax;
		tmpValidity->dateMin=tmp.dateMin;
		tmpValidity->dateMax=tmp.dateMax;
	    }
	    cell->clearDisplayDirtyFlag();
	}
    }
};

void KSpreadTable::setValidity(const QPoint &_marker,KSpreadValidity tmp )
{
    SetValidityWorker w( tmp );
    workOnCells( _marker, w );
}


struct GetWordSpellingWorker : public KSpreadTable::CellWorker {
    QString& listWord;
    GetWordSpellingWorker( QString& _listWord ) : KSpreadTable::CellWorker( false, false, true ), listWord( _listWord ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc*, KSpreadTable*, QRect& ) {
	return 0L;
    }
    bool testCondition( KSpreadCell* ) {
        return true;
    }
    void doWork( KSpreadCell* c, bool cellRegion, int, int ) {
	if ( !c->isObscured() || cellRegion /* ### ??? */ ) {
	    if ( !c->isFormular() && !c->isValue() && !c->valueString().isEmpty() && !c->isTime()
		 && !c->isDate() && c->content() != KSpreadCell::VisualFormula
		 && !c->text().isEmpty())
	    {
		listWord+=c->text()+'\n';
	    }
	}
    }
};

QString KSpreadTable::getWordSpelling(const QPoint &_marker )
{
    QString listWord;
    GetWordSpellingWorker w( listWord );
    workOnCells( _marker, w );
    return listWord;
}


struct SetWordSpellingWorker : public KSpreadTable::CellWorker {
    QStringList& list;
    int pos;
    SetWordSpellingWorker( QStringList& _list ) : KSpreadTable::CellWorker( false, false, true ), list( _list ), pos( 0 ) { }

    class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadTable* table, QRect& r ) {
	return new KSpreadUndoChangeAreaTextCell( doc, table, r );
    }
    bool testCondition( KSpreadCell* ) {
        return true;
    }
    void doWork( KSpreadCell* c, bool cellRegion, int, int ) {
	if ( !c->isObscured() || cellRegion /* ### ??? */ ) {
	    if ( !c->isFormular() && !c->isValue() && !c->valueString().isEmpty() && !c->isTime()
		 && !c->isDate() && c->content() != KSpreadCell::VisualFormula
		 && !c->text().isEmpty())
	    {
		c->setCellText( list[pos] );
		pos++;
	    }
	}
    }
};

void KSpreadTable::setWordSpelling(const QPoint &_marker, const QString _listWord )
{
    QStringList list = QStringList::split ( '\n', _listWord );
    SetWordSpellingWorker w( list );
    workOnCells( _marker, w );
}


void KSpreadTable::copyAsText( const QPoint &_marker )
{
    KSpreadCell* cell = cellAt( _marker.x(), _marker.y() );
    if( !cell->isDefault() )
        QApplication::clipboard()->setText( cell->text() );
}

void KSpreadTable::copySelection( const QPoint &_marker )
{
    QRect rct;

    // No selection ? => copy active cell
    if ( m_rctSelection.left() == 0 )
        rct.setCoords( _marker.x(), _marker.y(), _marker.x(), _marker.y() );
    else
        rct = selectionRect();

    QDomDocument doc = saveCellRect( rct );

    // Save to buffer
    QBuffer buffer;
    buffer.open( IO_WriteOnly );
    QTextStream str( &buffer );
    str.setEncoding( QTextStream::UnicodeUTF8 );
    str << doc;
    buffer.close();

    QStoredDrag* data = new QStoredDrag( "application/x-kspread-snippet" );
    data->setEncodedData( buffer.buffer() );

    QApplication::clipboard()->setData( data );
}

void KSpreadTable::cutSelection( const QPoint &_marker )
{
    copySelection( _marker );
    deleteSelection( _marker );
}

void KSpreadTable::paste( const QPoint &_marker,bool makeUndo, PasteMode sp, Operation op,bool insert,int insertTo )
{
    QMimeSource* mime = QApplication::clipboard()->data();
    if ( !mime )
        return;

    QByteArray b;

    if ( mime->provides( "application/x-kspread-snippet" ) )
        b = mime->encodedData( "application/x-kspread-snippet" );
    else if( mime->provides( "text/plain" ) )
      {
	pasteTextPlain( mime, _marker);
	return;
      }
    else
        return;

    paste( b, _marker,makeUndo, sp, op,insert, insertTo );
}

void KSpreadTable::pasteTextPlain( QMimeSource * _mime, const QPoint &_marker)
{
  QString tmp;
  tmp= QString::fromLocal8Bit(_mime->encodedData( "text/plain" ));
  if(tmp.isEmpty())
    return;
  KSpreadCell* cell = cellAt( _marker.x(), _marker.y() );
  if ( !m_pDoc->undoBuffer()->isLocked() )
    {
      KSpreadUndoSetText *undo =new KSpreadUndoSetText( m_pDoc, this , cell->text(), _marker.x(), _marker.y(),cell->getFormatNumber( _marker.x(), _marker.y() ) );
      m_pDoc->undoBuffer()->appendUndo( undo );
    }
  if ( cell->isDefault() )
    {
      cell = new KSpreadCell( this, _marker.x(), _marker.y() );
      insertCell( cell );
    }
  cell->setCellText( tmp );
  cell->updateChart();

  if(!isLoading())
    refreshMergedCell();

  emit sig_updateView( this );
  emit sig_updateHBorder( this );
  emit sig_updateVBorder( this );
}

void KSpreadTable::paste( const QByteArray& b, const QPoint &_marker,bool makeUndo, PasteMode sp, Operation op,bool insert, int insertTo )
{
    kdDebug(36001) << "Parsing " << b.size() << " bytes" << endl;

    QBuffer buffer( b );
    buffer.open( IO_ReadOnly );
    QDomDocument doc;
    doc.setContent( &buffer );
    buffer.close();

    // ##### TODO: Test for parsing errors

    loadSelection( doc, _marker.x() - 1, _marker.y() - 1,makeUndo, sp, op, insert, insertTo);

}

bool KSpreadTable::loadSelection( const QDomDocument& doc, int _xshift, int _yshift,bool makeUndo, PasteMode sp, Operation op,bool insert,int insertTo )
{
    QDomElement e = doc.documentElement();
    if(!isLoading()&&makeUndo)
        loadSelectionUndo( doc, _xshift,_yshift,insert,insertTo);

    int rowsInClpbrd =  e.attribute( "rows" ).toInt();
    int columnsInClpbrd =  e.attribute( "columns" ).toInt();
    // find size of rectangle that we want to paste to (either clipboard size or current selection)
    const int pasteWidth = ( selectionRect().left() != 0 && selectionRect().width() >= columnsInClpbrd &&
                             selectionRect().right() != 0x7fff && e.namedItem( "rows" ).toElement().isNull() )
        ? selectionRect().width() : columnsInClpbrd;
    const int pasteHeight = ( selectionRect().left() != 0 && selectionRect().height() >= rowsInClpbrd &&
                              selectionRect().bottom() != 0x7fff && e.namedItem( "columns" ).toElement().isNull() )
        ? selectionRect().height() : rowsInClpbrd;
    //kdDebug(36001) << "loadSelection: paste area has size " << pasteHeight << " rows * "
    //               << pasteWidth << " columns " << endl;
    //kdDebug(36001) << "loadSelection: " << rowsInClpbrd << " rows and "
    //               << columnsInClpbrd << " columns in clipboard." << endl;

    if ( !e.namedItem( "columns" ).toElement().isNull() )
    {
        _yshift = 0;

        // Clear the existing columns
        for( int i = 1; i <= pasteWidth; ++i )
        {
            if(!insert)
            {
                m_cells.clearColumn( _xshift + i );
                m_columns.removeElement( _xshift + i );
            }
        }

        // Insert column layouts
        QDomElement c = e.firstChild().toElement();
        for( ; !c.isNull(); c = c.nextSibling().toElement() )
        {
            if ( c.tagName() == "column" )
            {
                ColumnLayout *cl = new ColumnLayout( this, 0 );
                if ( cl->load( c, _xshift,sp ) )
                    insertColumnLayout( cl );
                else
                    delete cl;
            }
        }

    }

    if ( !e.namedItem( "rows" ).toElement().isNull() )
    {
        _xshift = 0;

        // Clear the existing rows
        for( int i = 1; i <= pasteHeight; ++i )
        {
            m_cells.clearRow( _yshift + i );
            m_rows.removeElement( _yshift + i );
        }

        // Insert row layouts
        QDomElement c = e.firstChild().toElement();
        for( ; !c.isNull(); c = c.nextSibling().toElement() )
        {
            if ( c.tagName() == "row" )
            {
                RowLayout *cl = new RowLayout( this, 0 );
                if ( cl->load( c, _yshift,sp ) )
                    insertRowLayout( cl );
                else
                    delete cl;
            }
        }
    }

    KSpreadCell* refreshCell = 0;
    QDomElement c = e.firstChild().toElement();
    for( ; !c.isNull(); c = c.nextSibling().toElement() )
    {
        if ( c.tagName() == "cell" )
        {
            int row = c.attribute( "row" ).toInt() + _yshift;
            int col = c.attribute( "column" ).toInt() + _xshift;
            // tile the selection with the clipboard contents
            for(int roff=0; row-_yshift+roff<=pasteHeight; roff += rowsInClpbrd)
            {
                for(int coff=0; col-_xshift+coff<=pasteWidth; coff += columnsInClpbrd)
                {
                    //kdDebug(36001) << "loadSelection: cell at " << (row+roff) << "," << (col+coff) << " with roff,coff= "
                    //               << roff << "," << coff << endl;

                    bool needInsert = FALSE;
                    KSpreadCell* cell = cellAt( col + coff, row + roff );
                    if ( ( op == OverWrite && sp == Normal ) || cell->isDefault() )
                    {
                        cell = new KSpreadCell( this, 0, 0 );
                        needInsert = TRUE;
                    }
                    if ( !cell->load( c, _xshift + coff, _yshift + roff, sp, op ) )
                    {
                        if ( needInsert )
                            delete cell;
                    }
                    else {
                        if ( needInsert )
                            insertCell( cell );
                        if( !refreshCell && cell->updateChart( false ) )
                            refreshCell = cell;
                    }
                }
            }
        }
    }
    //refresh chart after that you paste all cells
    if ( refreshCell )
        refreshCell->updateChart();
    m_pDoc->setModified( true );

    if(!isLoading())
        refreshMergedCell();

    emit sig_updateView( this );
    emit sig_updateHBorder( this );
    emit sig_updateVBorder( this );

    return true;
}

void KSpreadTable::loadSelectionUndo( const QDomDocument & doc,int _xshift, int _yshift,bool insert,int insertTo)
{
    QDomElement e = doc.documentElement();
    QDomElement c = e.firstChild().toElement();
    int rowsInClpbrd =  e.attribute( "rows" ).toInt();
    int columnsInClpbrd =  e.attribute( "columns" ).toInt();
    // find rect that we paste to
    const int pasteWidth = ( selectionRect().left() != 0 && selectionRect().width() >= columnsInClpbrd &&
                             selectionRect().right() != 0x7fff && e.namedItem( "rows" ).toElement().isNull() )
        ? selectionRect().width() : columnsInClpbrd;
    const int pasteHeight = ( selectionRect().left() != 0 && selectionRect().height() >= rowsInClpbrd &&
                              selectionRect().bottom() != 0x7fff && e.namedItem( "columns" ).toElement().isNull() )
        ? selectionRect().height() : rowsInClpbrd;
    QRect rect;
    if ( !e.namedItem( "columns" ).toElement().isNull() )
    {
        if ( !m_pDoc->undoBuffer()->isLocked() )
        {
                KSpreadUndoCellPaste *undo = new KSpreadUndoCellPaste( m_pDoc, this, pasteWidth, 0, _xshift,_yshift,rect,insert );
                m_pDoc->undoBuffer()->appendUndo( undo );
        }
        if(insert)
                 insertColumn(  _xshift+1,pasteWidth-1,false);
	return;
    }

    if ( !e.namedItem( "rows" ).toElement().isNull() )
    {
        if ( !m_pDoc->undoBuffer()->isLocked() )
        {
                KSpreadUndoCellPaste *undo = new KSpreadUndoCellPaste( m_pDoc, this, 0,pasteHeight, _xshift,_yshift,rect,insert );
                m_pDoc->undoBuffer()->appendUndo( undo );
        }
	if(insert)
	    insertRow(  _yshift+1,pasteHeight-1,false);
	return;
    }

    rect.setRect( _xshift+1, _yshift+1, pasteWidth, pasteHeight );

    if(!c.isNull())
    {
        if ( !m_pDoc->undoBuffer()->isLocked() )
        {
                KSpreadUndoCellPaste *undo = new KSpreadUndoCellPaste( m_pDoc, this, 0,0,_xshift,_yshift,rect,insert,insertTo );
                m_pDoc->undoBuffer()->appendUndo( undo );
        }
    if(insert)
        {
        if(insertTo==-1)
                shiftRow(rect,false);
        else if(insertTo==1)
                shiftColumn(rect,false);
        }
    }
}

bool KSpreadTable::testAreaPasteInsert()
{
   QMimeSource* mime = QApplication::clipboard()->data();
    if ( !mime )
        return false;

    QByteArray b;

    if ( mime->provides( "application/x-kspread-snippet" ) )
        b = mime->encodedData( "application/x-kspread-snippet" );
    else
        return false;

    QBuffer buffer( b );
    buffer.open( IO_ReadOnly );
    QDomDocument doc;
    doc.setContent( &buffer );
    buffer.close();

    QDomElement e = doc.documentElement();
    if ( !e.namedItem( "columns" ).toElement().isNull() )
        return false;

    if ( !e.namedItem( "rows" ).toElement().isNull() )
        return false;

    QDomElement c = e.firstChild().toElement();
    for( ; !c.isNull(); c = c.nextSibling().toElement() )
    {
        if ( c.tagName() == "cell" )
                return true;
    }
    return false;
}

void KSpreadTable::deleteCells( const QRect& rect )
{
    // A list of all cells we want to delete.
    QStack<KSpreadCell> cellStack;

    QRect tmpRect;
    bool extraCell=false;
    if(rect.width()==1 && rect.height()==1)
        {
        KSpreadCell *cell = nonDefaultCell( rect.x(), rect.y() );
        if(cell->isForceExtraCells())
                {
                extraCell=true;
                tmpRect=rect;
                }
        }
    else if(rect.contains(m_marker.x(),m_marker.y())
    &&m_rctSelection.left()==0)
        {
        KSpreadCell *cell = nonDefaultCell( m_marker.x(),m_marker.y() );
        if(cell->isForceExtraCells())
                {
                extraCell=true;
                tmpRect=QRect(m_marker.x(),m_marker.y(),1,1);
                }
        }

    KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
    {
        if ( !c->isDefault() && c->row() >= rect.top() &&
             c->row() <= rect.bottom() && c->column() >= rect.left() &&
             c->column() <= rect.right() )
          cellStack.push( c );
    }

    m_cells.setAutoDelete( false );

    // Remove the cells from the table
    while ( !cellStack.isEmpty() )
    {
        KSpreadCell *cell = cellStack.pop();

        m_cells.remove( cell->column(), cell->row() );
        cell->updateDepending();

        delete cell;
    }

    m_cells.setAutoDelete( true );

    setLayoutDirtyFlag();

    // Since obscured cells might have been deleted we
    // have to reenforce it.
    c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
        if ( c->isForceExtraCells() && !c->isDefault() )
            c->forceExtraCells( c->column(), c->row(), c->extraXCells(), c->extraYCells() );

    if(extraCell)
        {
        setSelection(tmpRect);
        unselect();
        }

    m_pDoc->setModified( true );

}

void KSpreadTable::deleteSelection( const QPoint& _marker )
{
    QRect r( m_rctSelection );

    if ( r.left() == 0 )
        r = QRect( _marker.x(), _marker.y(), 1, 1 );

    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            KSpreadUndoDelete *undo = new KSpreadUndoDelete( m_pDoc, this, r );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

    if ( !m_pDoc->undoBuffer()->isLocked() )
        {
            KSpreadUndoDelete *undo = new KSpreadUndoDelete( m_pDoc, this, r );
            m_pDoc->undoBuffer()->appendUndo( undo );
        }

    // Entire rows selected ?
    if ( r.right() == 0x7fff )
    {
        for( int i = r.top(); i <= r.bottom(); ++i )
        {
            m_cells.clearRow( i );
            m_rows.removeElement( i );
        }

        emit sig_updateVBorder( this );
    }
    // Entire columns selected ?
    else if ( r.bottom() == 0x7fff )
    {
        for( int i = r.left(); i <= r.right(); ++i )
        {
            m_cells.clearColumn( i );
            m_columns.removeElement( i );
        }

        emit sig_updateHBorder( this );
    }
    else
    {

        deleteCells( r );
    }
    refreshMergedCell();
    emit sig_updateView( this );
}

void KSpreadTable::refreshView(const QRect& rect)
{
    QRect tmp(rect);
    KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
    {
        if ( !c->isDefault() && c->row() >= rect.top() &&
             c->row() <= rect.bottom() && c->column() >= rect.left() &&
             c->column() <= rect.right() )
                if(c->isForceExtraCells())
                {
                int right=QMAX(tmp.right(),c->column()+c->extraXCells());
                int bottom=QMAX(tmp.bottom(),c->row()+c->extraYCells());

                tmp.setRight(right);
                tmp.setBottom(bottom);
                }
    }
    deleteCells( rect );
    emit sig_updateView( this, tmp );
}

void KSpreadTable::updateView(const QRect& rect)
{
    emit sig_updateView( this, rect );
}

void KSpreadTable::changeMergedCell( int m_iCol, int m_iRow, int m_iExtraX, int m_iExtraY)
{
   if( m_iExtraX==0 && m_iExtraY==0)
        {
        dissociateCell( QPoint( m_iCol,m_iRow),false);
        return;
        }
    KSpreadCell *cell = nonDefaultCell( m_iCol,m_iRow  );
    if(cell->isForceExtraCells())
        dissociateCell( QPoint( m_iCol,m_iRow),false);

    cell->forceExtraCells( m_iCol,m_iRow,
                           m_iExtraX,m_iExtraY);

    setMarker(QPoint(m_iCol,m_iRow));
    refreshMergedCell();
    QRect rect;
    rect.setCoords(m_iCol,m_iRow,m_iCol+m_iExtraX,m_iRow+m_iExtraY);
    emit sig_updateView( this, rect );

}

void KSpreadTable::mergeCell( const QPoint &_marker, bool makeUndo)
{
    if(m_rctSelection.left() == 0)
        return;
    int x=_marker.x();
    int y=_marker.y();
    if( _marker.x() > m_rctSelection.left() )
        x = m_rctSelection.left();
    if( _marker.y() > m_rctSelection.top() )
        y = m_rctSelection.top();
    KSpreadCell *cell = nonDefaultCell( x ,y  );

    if ( !m_pDoc->undoBuffer()->isLocked() && makeUndo)
    {
        KSpreadUndoMergedCell *undo = new KSpreadUndoMergedCell( m_pDoc, this, x ,y,cell->extraXCells() ,cell->extraYCells());
        m_pDoc->undoBuffer()->appendUndo( undo );
    }

    cell->forceExtraCells( x ,y,
                           abs(m_rctSelection.right() -m_rctSelection.left()),
                           abs(m_rctSelection.bottom() - m_rctSelection.top()));

    setMarker(QPoint(x,y));
    if(getAutoCalc())
        recalc(true);
    emit sig_updateView( this, m_rctSelection );
}

void KSpreadTable::dissociateCell( const QPoint &_marker,bool makeUndo)
{
    KSpreadCell *cell = nonDefaultCell(_marker.x() ,_marker.y()  );
    if(!cell->isForceExtraCells())
        return;

    if ( !m_pDoc->undoBuffer()->isLocked() && makeUndo)
    {
        KSpreadUndoMergedCell *undo = new KSpreadUndoMergedCell( m_pDoc, this, _marker.x() ,_marker.y(),cell->extraXCells() ,cell->extraYCells());
        m_pDoc->undoBuffer()->appendUndo( undo );
    }
    int x=cell->extraXCells();
    if( x == 0 )
        x=1;
    int y = cell->extraYCells();
    if( y == 0 )
        y=1;

    cell->forceExtraCells( _marker.x() ,_marker.y(), 0, 0 );
    QRect selection( _marker.x(), _marker.y(), x, y );
    setSelection(selection);
    unselect();
    refreshMergedCell();
    emit sig_updateView( this, selection );
}

bool KSpreadTable::testListChoose(const QPoint &_marker)
{
   QRect selection( selectionRect() );
   if(selection.left()==0)
     selection.setCoords(_marker.x(),_marker.y(),_marker.x(),_marker.y());

   KSpreadCell *cell = cellAt( _marker.x(), _marker.y() );
   QString tmp=cell->text();

   KSpreadCell* c = firstCell();
   bool different=false;
   for( ;c  ; c = c->nextCell() )
     {
       int col = c->column();
       if ( selection.left() <= col && selection.right() >= col
	    &&!c->isObscuringForced()&& !(col==_marker.x()&& c->row()==_marker.y()))
	 {
	   if(!c->isFormular() && !c->isValue() && !c->valueString().isEmpty()
	      && !c->isTime() &&!c->isDate()
	      && c->content() != KSpreadCell::VisualFormula)
	     {
                 if(c->text()!=tmp)
                     different=true;
	     }

	 }
     }
   return different;
}




void KSpreadTable::print( QPainter &painter, KPrinter *_printer )
{
    kdDebug(36001)<<"PRINTING ...."<<endl;

    // Override the current grid pen setting
    QPen gridPen = m_pDoc->defaultGridPen();
    QPen nopen;
    nopen.setStyle( NoPen );
    m_pDoc->setDefaultGridPen( nopen );

    //
    // Find maximum right/bottom cell with content
    //
    QRect cell_range;
    cell_range.setCoords( 1, 1, 1, 1 );

    KSpreadCell* c = m_cells.firstCell();
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
    QListIterator<KoDocumentChild> cit( m_pDoc->children() );
    for( ; cit.current(); ++cit )
    {
        QRect bound = cit.current()->boundingRect();
        int dummy;
        int i = leftColumn( bound.right(), dummy );
        if ( i > cell_range.right() )
            cell_range.setRight( i );
        i = topRow( bound.bottom(), dummy );
        if ( i > cell_range.bottom() )
            cell_range.setBottom( i );
    }


    //
    // Find out how many pages need printing
    // and which cells to print on which page.
    //
    QValueList<QRect> page_list;
    QValueList<QRect> page_frame_list;

    // How much space is on every page for table content ?
    QRect rect;
    rect.setCoords( 0, 0, (int)( MM_TO_POINT ( m_pDoc->printableWidth() )),
                    (int)( MM_TO_POINT ( m_pDoc->printableHeight() )) );

    // Up to this row everything is already handled
    int bottom = 0;
    // Start of the next page
    int top = 1;
    // Calculate all pages, but if we are embedded, print only the first one
    while ( bottom < cell_range.bottom() /* && page_list.count() == 0 */ )
    {
        kdDebug(36001) << "bottom=" << bottom << " bottom_range=" << cell_range.bottom() << endl;

        // Up to this column everything is already printed
        int right = 0;
        // Start of the next page
        int left = 1;
        while ( right < cell_range.right() )
        {
            kdDebug(36001) << "right=" << right << " right_range=" << cell_range.right() << endl;

            QRect page_range;
            page_range.setLeft( left );
            page_range.setTop( top );

            int col = left;
            int x = columnLayout( col )->width();
            while ( x < rect.width() )
            {
                col++;
                x += columnLayout( col )->width();
            }
            // We want to print at least one column
            if ( col == left )
                col = left + 1;
            page_range.setRight( col - 1 );

            int row = top;
            int y = rowLayout( row )->height();
            while ( y < rect.height() )
            {
                row++;
                y += rowLayout( row )->height();
            }
            // We want to print at least one row
            if ( row == top )
                row = top + 1;
            page_range.setBottom( row - 1 );

            right = page_range.right();
            left = page_range.right() + 1;
            bottom = page_range.bottom();

            //
            // Test wether there is anything on the page at all.
            //

            // Look at the cells
            bool empty = TRUE;
            for( int r = page_range.top(); r <= page_range.bottom(); ++r )
                for( int c = page_range.left(); c <= page_range.right(); ++c )
                    if ( cellAt( c, r )->needsPrinting() )
                        empty = FALSE;

            // Look for children
            QRect view( columnPos( page_range.left() ), rowPos( page_range.top() ),
                        rect.width(), rect.height() );

            QListIterator<KoDocumentChild> it( m_pDoc->children() );
            for( ; it.current(); ++it )
            {
                QRect bound = it.current()->boundingRect();
                if ( bound.intersects( view ) )
                    empty = FALSE;
            }

            if ( !empty )
            {
                page_list.append( page_range );
                page_frame_list.append( view );
            }
        }

        top = bottom + 1;
    }

     kdDebug(36001)<<"PRINTING "<< page_list.count()<<" pages"<<endl;

    int pagenr = 1;

    //
    // Print all pages in the list
    //

    QValueList<QRect>::Iterator it = page_list.begin();
    QValueList<QRect>::Iterator fit = page_frame_list.begin();
    for( ; it != page_list.end(); ++it, ++fit, ++pagenr )
    {
        // print head line
        QFont font( "Times", 10 );
        painter.setFont( font );
        QFontMetrics fm = painter.fontMetrics();
        int w = fm.width( m_pDoc->headLeft( pagenr, m_strName ) );
        if ( w > 0 )
            painter.drawText( (int)( MM_TO_POINT ( m_pDoc->leftBorder() )),
                              (int)( MM_TO_POINT ( 10.0 )), m_pDoc->headLeft( pagenr, m_strName ) );
        w = fm.width( m_pDoc->headMid( pagenr, m_strName.latin1() ) );
        if ( w > 0 )
            painter.drawText( (int)( MM_TO_POINT ( m_pDoc->leftBorder()) +
                                     ( MM_TO_POINT ( m_pDoc->printableWidth()) - (float)w ) / 2.0 ),
                              (int)( MM_TO_POINT ( 10.0 )), m_pDoc->headMid( pagenr, m_strName ) );
        w = fm.width( m_pDoc->headRight( pagenr, m_strName ) );
        if ( w > 0 )
            painter.drawText( (int)( MM_TO_POINT ( m_pDoc->leftBorder()) +
                                     MM_TO_POINT ( m_pDoc->printableWidth()) - (float)w ),
                              (int)( MM_TO_POINT ( 10.0 )), m_pDoc->headRight( pagenr, m_strName) );

        // print foot line
        w = fm.width( m_pDoc->footLeft( pagenr, m_strName ) );
        if ( w > 0 )
            painter.drawText( (int)( MM_TO_POINT ( m_pDoc->leftBorder() )),
                              (int)( MM_TO_POINT ( m_pDoc->paperHeight() - 10.0 )),
                              m_pDoc->footLeft( pagenr, m_strName ) );
        w = fm.width( m_pDoc->footMid( pagenr, m_strName ) );
        if ( w > 0 )
            painter.drawText( (int)( MM_TO_POINT ( m_pDoc->leftBorder() )+
                                     ( MM_TO_POINT ( m_pDoc->printableWidth()) - (float)w ) / 2.0 ),
                              (int)( MM_TO_POINT  ( m_pDoc->paperHeight() - 10.0 ) ),
                              m_pDoc->footMid( pagenr, m_strName ) );
        w = fm.width( m_pDoc->footRight( pagenr, m_strName ) );
        if ( w > 0 )
            painter.drawText( (int)( MM_TO_POINT ( m_pDoc->leftBorder()) +
                                     MM_TO_POINT ( m_pDoc->printableWidth()) - (float)w ),
                              (int)( MM_TO_POINT ( m_pDoc->paperHeight() - 10.0 ) ),
                              m_pDoc->footRight( pagenr, m_strName ) );

        painter.translate( MM_TO_POINT ( m_pDoc->leftBorder()),
                           MM_TO_POINT ( m_pDoc->topBorder() ));
        // Print the page
        printPage( painter, *it, *fit );

        painter.translate( - MM_TO_POINT ( m_pDoc->leftBorder()),
                           - MM_TO_POINT ( m_pDoc->topBorder() ));

        if ( pagenr < (int)page_list.count() )
            _printer->newPage();
    }

    // Restore the grid pen
    m_pDoc->setDefaultGridPen( gridPen );
}

void KSpreadTable::printPage( QPainter &_painter, const QRect& page_range, const QRect& view )
{
    // kdDebug(36001) << "Rect x=" << page_range->left() << " y=" << page_range->top() << ", w="
    // << page_range->width() << " h="  << page_range->height() << endl;

    //
    // Draw the cells.
    //
    int ypos = 0;
    for ( int y = page_range.top(); y <= page_range.bottom(); y++ )
    {
        RowLayout *row_lay = rowLayout( y );
        int xpos = 0;

        for ( int x = page_range.left(); x <= page_range.right(); x++ )
        {
            ColumnLayout *col_lay = columnLayout( x );

            KSpreadCell *cell = cellAt( x, y );
            QRect r( 0, 0, view.width(), view.height() );
            cell->paintCell( r, _painter, xpos, ypos, x, y, col_lay, row_lay );

            xpos += col_lay->width();
        }

        ypos += row_lay->height();
    }

    //
    // Draw the children
    //
    QListIterator<KoDocumentChild> it( m_pDoc->children() );
    for( ; it.current(); ++it )
    {
        QString tmp=QString("Testing child %1/%2 %3/%4 against view %5/%6 %7/%8")
        .arg(it.current()->contentRect().left())
        .arg(it.current()->contentRect().top())
        .arg(it.current()->contentRect().right())
        .arg(it.current()->contentRect().bottom())
        .arg(view.left()).arg(view.top()).arg(view.right()).arg(view.bottom() );
        kdDebug(36001)<<tmp<<endl;

        QRect bound = it.current()->boundingRect();
        if ( ((KSpreadChild*)it.current())->table() == this && bound.intersects( view ) )
        {
            _painter.save();
            _painter.translate( -view.left(), -view.top() );

            it.current()->transform( _painter );
            it.current()->document()->paintEverything( _painter,
                                                       it.current()->contentRect(),
                                                       it.current()->isTransparent() );
            _painter.restore();
        }
    }
}

QDomDocument KSpreadTable::saveCellRect( const QRect &_rect )
{
    QDomDocument doc( "spreadsheet-snippet" );
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
    QDomElement spread = doc.createElement( "spreadsheet-snippet" );
    spread.setAttribute( "rows", _rect.bottom() - _rect.top() + 1 );
    spread.setAttribute( "columns", _rect.right() - _rect.left() + 1 );
    doc.appendChild( spread );

    //
    // Entire rows selected ?
    //
    if ( _rect.right() == 0x7fff )
    {
        QDomElement rows = doc.createElement("rows");
        rows.setAttribute( "count", _rect.bottom() - _rect.top() + 1 );
        spread.appendChild( rows );

        // Save all cells.
        KSpreadCell* c = m_cells.firstCell();
        for( ;c; c = c->nextCell() )
        {
            if ( !c->isDefault()&&!c->isObscuringForced() )
            {
                QPoint p( c->column(), c->row() );
                if ( _rect.contains( p ) )
                    spread.appendChild( c->save( doc, 0, _rect.top() - 1 ) );
            }
        }

        // ##### Inefficient
        // Save the row layouts if there are any
        for( int y = _rect.top(); y <= _rect.bottom(); ++y )
        {
            RowLayout* lay = rowLayout( y );
            if ( lay && !lay->isDefault() )
            {
                QDomElement e = lay->save( doc, _rect.top() - 1 );
                if ( !e.isNull() )
                    spread.appendChild( e );
            }
        }

        return doc;
    }

    //
    // Entire columns selected ?
    //
    if ( _rect.bottom() == 0x7fff )
    {
        QDomElement columns = doc.createElement("columns");
        columns.setAttribute( "count", _rect.right() - _rect.left() + 1 );
        spread.appendChild( columns );

        // Save all cells.
        KSpreadCell* c = m_cells.firstCell();
        for( ;c; c = c->nextCell() )
        {
            if ( !c->isDefault()&&!c->isObscuringForced())
            {
                QPoint p( c->column(), c->row() );
                if ( _rect.contains( p ) )
                    spread.appendChild( c->save( doc, _rect.left() - 1, 0 ) );
            }
        }

        // ##### Inefficient
        // Save the column layouts if there are any
        for( int x = _rect.left(); x <= _rect.right(); ++x )
        {
            ColumnLayout* lay = columnLayout( x );
            if ( lay && !lay->isDefault() )
            {
                QDomElement e = lay->save( doc, _rect.left() - 1 );
                if ( !e.isNull() )
                    spread.appendChild( e );
            }
        }

        return doc;
    }

    // Save all cells.
    /*KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
    {
        if ( !c->isDefault() && !c->isObscuringForced())
        {
            QPoint p( c->column(), c->row() );
            if ( _rect.contains( p ) )
                spread.appendChild( c->save( doc, _rect.left() - 1, _rect.top() - 1  ,true));
        }
    } */
    //store all cell
    //when they don't exist we created them
    //because it's necessary when there is a  layout on a column/row
    //but I remove cell which is inserted.
    for (int i=_rect.left();i<=_rect.right();i++)
        for(int j=_rect.top();j<=_rect.bottom();j++)
                {
                bool insert=false;
                KSpreadCell *cell = cellAt( i, j );
                if ( cell == m_pDefaultCell )
               {
                  cell = new KSpreadCell( this, i, j );
                  m_cells.insert( cell, i, j );
                  insert=true;
               }
                spread.appendChild( cell->save( doc, _rect.left() - 1, _rect.top() - 1  ,true));
                if(insert)
                        m_cells.remove(i,j);
                }

    return doc;
}

QDomElement KSpreadTable::save( QDomDocument& doc )
{
    QDomElement table = doc.createElement( "table" );
    table.setAttribute( "name", m_strName );
    table.setAttribute( "grid", (int)m_bShowGrid);
    table.setAttribute( "hide", (int)m_bTableHide);
    table.setAttribute( "formular", (int)m_bShowFormular);
    table.setAttribute( "borders", (int)m_bShowPageBorders);
    table.setAttribute( "lcmode", (int)m_bLcMode);
    table.setAttribute( "columnnumber", (int)m_bShowColumnNumber);
    table.setAttribute( "hidezero", (int)m_bHideZero);
    table.setAttribute( "firstletterupper", (int)m_bFirstLetterUpper);
    // Save all cells.
    KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
    {
        if ( !c->isDefault() )
        {
            QDomElement e = c->save( doc );
            if ( e.isNull() )
                return QDomElement();
            table.appendChild( e );
        }
    }

    // Save all RowLayout objects.
    RowLayout* rl = m_rows.first();
    for( ; rl; rl = rl->next() )
    {
        if ( !rl->isDefault() )
        {
            QDomElement e = rl->save( doc );
            if ( e.isNull() )
                return QDomElement();
            table.appendChild( e );
        }
    }

    // Save all ColumnLayout objects.
    ColumnLayout* cl = m_columns.first();
    for( ; cl; cl = cl->next() )
    {
        if ( !cl->isDefault() )
        {
            QDomElement e = cl->save( doc );
            if ( e.isNull() )
                return QDomElement();
            table.appendChild( e );
        }
    }

    QListIterator<KoDocumentChild> chl( m_pDoc->children() );
    for( ; chl.current(); ++chl )
    {
        if ( ((KSpreadChild*)chl.current())->table() == this )
        {
            QDomElement e = chl.current()->save( doc );
            if ( e.isNull() )
                return QDomElement();
            table.appendChild( e );
        }
    }

    return table;
}

bool KSpreadTable::isLoading()
{
    return m_pDoc->isLoading();
}

bool KSpreadTable::loadXML( const QDomElement& table )
{
    bool ok = false;
    m_strName = table.attribute( "name" );
    if ( m_strName.isEmpty() )
        return false;

    if( table.hasAttribute( "grid" ) )
    {
        m_bShowGrid = (int)table.attribute("grid").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "hide" ) )
    {
        m_bTableHide = (int)table.attribute("hide").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "formular" ) )
    {
        m_bShowFormular = (int)table.attribute("formular").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "borders" ) )
    {
        m_bShowPageBorders = (int)table.attribute("borders").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "lcmode" ) )
    {
        m_bLcMode = (int)table.attribute("lcmode").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "columnnumber" ) )
    {
        m_bShowColumnNumber = (int)table.attribute("columnnumber").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "hidezero" ) )
    {
        m_bHideZero = (int)table.attribute("hidezero").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    if( table.hasAttribute( "firstletterupper" ) )
    {
        m_bFirstLetterUpper = (int)table.attribute("firstletterupper").toInt( &ok );
        // we just ignore 'ok' - if it didn't work, go on
    }
    // Load the cells
    QDomNode n = table.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if ( !e.isNull() && e.tagName() == "cell" )
        {
            KSpreadCell *cell = new KSpreadCell( this, 0, 0 );
            if ( cell->load( e, 0, 0 ) )
                insertCell( cell );
            else
                delete cell; // Allow error handling: just skip invalid cells
        }
        else if ( !e.isNull() && e.tagName() == "row" )
        {
            RowLayout *rl = new RowLayout( this, 0 );
            if ( rl->load( e ) )
                insertRowLayout( rl );
            else
                delete rl;
        }
        else if ( !e.isNull() && e.tagName() == "column" )
        {
            ColumnLayout *cl = new ColumnLayout( this, 0 );
            if ( cl->load( e ) )
                insertColumnLayout( cl );
            else
                delete cl;
        }
        else if ( !e.isNull() && e.tagName() == "object" )
        {
            KSpreadChild *ch = new KSpreadChild( m_pDoc, this );
            if ( ch->load( e ) )
                insertChild( ch );
            else
                delete ch;
        }
        else if ( !e.isNull() && e.tagName() == "chart" )
        {
            ChartChild *ch = new ChartChild( m_pDoc, this );
            if ( ch->load( e ) )
                insertChild( ch );
            else
                delete ch;
        }

    n = n.nextSibling();
  }

  return true;
}

void KSpreadTable::update()
{
  kdDebug(36001) << "KSpreadTable::update()" << endl;
  KSpreadCell* c = m_cells.firstCell();
  for( ;c; c = c->nextCell() )
  {
      if ( c->isFormular() )
          c->makeFormular();
      if ( c->calcDirtyFlag() )
          c->update();
  }
}

bool KSpreadTable::loadChildren( KoStore* _store )
{
    QListIterator<KoDocumentChild> it( m_pDoc->children() );
    for( ; it.current(); ++it )
    {
        if ( ((KSpreadChild*)it.current())->table() == this )
        {
            if ( !it.current()->loadDocument( _store ) )
                return false;
        }
    }

    return true;
}

void KSpreadTable::setShowPageBorders( bool b )
{
    if ( b == m_bShowPageBorders )
        return;

    m_bShowPageBorders = b;
    emit sig_updateView( this );
}

bool KSpreadTable::isOnNewPageX( int _column )
{
    int col = 1;
    float x = columnLayout( col )->mmWidth();
    while ( col <= _column )
    {
        // Should never happen
        if ( col == 0x10000 )
            return FALSE;

        if ( x > m_pDoc->printableWidth() )
        {
            if ( col == _column )
                return TRUE;
            else
                x = columnLayout( col )->mmWidth();
        }

        col++;
        x += columnLayout( col )->mmWidth();
    }

    return FALSE;
}

bool KSpreadTable::isOnNewPageY( int _row )
{
    int row = 1;
    float y = rowLayout( row )->mmHeight();
    while ( row <= _row )
    {
        // Should never happen
        if ( row == 0x10000 )
            return FALSE;

        if ( y > m_pDoc->printableHeight() )
        {
            if ( row == _row )
                return TRUE;
            else
                y = rowLayout( row )->mmHeight();
        }
        row++;
        y += rowLayout( row )->mmHeight();
    }

    return FALSE;
}

void KSpreadTable::addCellBinding( CellBinding *_bind )
{
  m_lstCellBindings.append( _bind );

  m_pDoc->setModified( true );
}

void KSpreadTable::removeCellBinding( CellBinding *_bind )
{
  m_lstCellBindings.removeRef( _bind );

  m_pDoc->setModified( true );
}

KSpreadTable* KSpreadTable::findTable( const QString & _name )
{
  if ( !m_pMap )
    return 0L;

  return m_pMap->findTable( _name );
}

// ###### Torben: Use this one instead of m_cells.insert()
void KSpreadTable::insertCell( KSpreadCell *_cell )
{
    m_cells.insert( _cell, _cell->column(), _cell->row() );

  if ( m_bScrollbarUpdates )
  {
    if ( _cell->column() > m_iMaxColumn )
    {
      m_iMaxColumn = _cell->column();
      emit sig_maxColumn( _cell->column() );
    }
    if ( _cell->row() > m_iMaxRow )
    {
      m_iMaxRow = _cell->row();
      emit sig_maxRow( _cell->row() );
    }
  }
}

void KSpreadTable::insertColumnLayout( ColumnLayout *l )
{
  m_columns.insertElement( l, l->column() );
}

void KSpreadTable::insertRowLayout( RowLayout *l )
{
  m_rows.insertElement( l, l->row() );
}

void KSpreadTable::updateCell( KSpreadCell *cell, int _column, int _row )
{
    if ( doc()->isLoading() )
        return;

    // Get the size
    int left = columnPos( _column );
    int top = rowPos( _row );
    int right = left + cell->extraWidth();
    int bottom = top + cell->extraHeight();

    // Need to calculate ?
    if ( cell->calcDirtyFlag() )
        cell->calc();

    // Need to make layout ?
    if ( cell->layoutDirtyFlag() )
        cell->makeLayout( painter(), _column, _row );

    // Perhaps the size changed now ?
    right = QMAX( right, left + cell->extraWidth() );
    bottom = QMAX( bottom, top + cell->extraHeight() );

    // Force redraw
    QPointArray arr( 4 );
    arr.setPoint( 0, left, top );
    arr.setPoint( 1, right, top );
    arr.setPoint( 2, right, bottom );
    arr.setPoint( 3, left, bottom );

    // ##### Hmmmm, why not draw the cell directly ?
    // That will be faster.
    emit sig_polygonInvalidated( arr );

    cell->clearDisplayDirtyFlag();
}

void KSpreadTable::emit_polygonInvalidated( const QPointArray& arr )
{
    emit sig_polygonInvalidated( arr );
}

void KSpreadTable::emit_updateRow( RowLayout *_layout, int _row )
{
    if ( doc()->isLoading() )
        return;

    KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
      if ( c->row() == _row )
          c->setLayoutDirtyFlag();

    emit sig_updateVBorder( this );
    emit sig_updateView( this );
    emit sig_maxRow(maxRow());
    _layout->clearDisplayDirtyFlag();
}

void KSpreadTable::emit_updateColumn( ColumnLayout *_layout, int _column )
{
    if ( doc()->isLoading() )
        return;

    KSpreadCell* c = m_cells.firstCell();
    for( ;c; c = c->nextCell() )
        if ( c->column() == _column )
            c->setLayoutDirtyFlag();

    emit sig_updateHBorder( this );
    emit sig_updateView( this );
    emit sig_maxColumn(maxColumn());
    _layout->clearDisplayDirtyFlag();
}

void KSpreadTable::insertChart( const QRect& _rect, KoDocumentEntry& _e, const QRect& _data )
{
    kdDebug(36001) << "Creating document" << endl;
    KoDocument* doc = _e.createDoc();
    kdDebug(36001) << "Created" << endl;
    if ( !doc )
        // Error message is already displayed, so just return
        return;

    kdDebug(36001) << "NOW FETCHING INTERFACE" << endl;

    if ( !doc->initDoc() )
        return;

    ChartChild* ch = new ChartChild( m_pDoc, this, doc, _rect );
    ch->setDataArea( _data );
    ch->update();

    // m_pDoc->insertChild( ch );
    insertChild( ch );

    KoChart::WizardExtension *wiz = ch->chart()->wizardExtension();
    if ( wiz )
        wiz->show();
}

void KSpreadTable::insertChild( const QRect& _rect, KoDocumentEntry& _e )
{
    KoDocument* doc = _e.createDoc( m_pDoc );
    doc->initDoc();

    KSpreadChild* ch = new KSpreadChild( m_pDoc, this, doc, _rect );

    insertChild( ch );
}

void KSpreadTable::insertChild( KSpreadChild *_child )
{
    // m_lstChildren.append( _child );
    m_pDoc->insertChild( _child );

    emit sig_polygonInvalidated( _child->framePointArray() );
}

void KSpreadTable::deleteChild( KSpreadChild* child )
{
    QPointArray polygon = child->framePointArray();

    emit sig_removeChild( child );

    delete child;

    emit sig_polygonInvalidated( polygon );
}

void KSpreadTable::changeChildGeometry( KSpreadChild *_child, const QRect& _rect )
{
    _child->setGeometry( _rect );

    emit sig_updateChildGeometry( _child );
}

/*
QListIterator<KSpreadChild> KSpreadTable::childIterator()
{
  return QListIterator<KSpreadChild> ( m_lstChildren );
}
*/

bool KSpreadTable::saveChildren( KoStore* _store, const QString &_path )
{
    int i = 0;

    QListIterator<KoDocumentChild> it( m_pDoc->children() );
    for( ; it.current(); ++it )
    {
        if ( ((KSpreadChild*)it.current())->table() == this )
        {
            QString path = QString( "%1/%2" ).arg( _path ).arg( i++ );
            if ( !it.current()->document()->saveToStore( _store, path ) )
                return false;
        }
    }
    return true;
}

KSpreadTable::~KSpreadTable()
{
    s_mapTables->remove( m_id );

    //when you remove all table (close file)
    //you must reinit s_id otherwise there is not
    //the good name between map and table
    if( s_mapTables->count()==0)
      s_id=0L;

    KSpreadCell* c = m_cells.firstCell();
    for( ; c; c = c->nextCell() )
        c->tableDies();

    m_cells.clear(); // cells destructor needs table to still exist

    m_pPainter->end();
    delete m_pPainter;
    delete m_pWidget;

    delete m_defaultLayout;
    delete m_pDefaultCell;
    delete m_pDefaultRowLayout;
    delete m_pDefaultColumnLayout;

}

void KSpreadTable::enableScrollBarUpdates( bool _enable )
{
  m_bScrollbarUpdates = _enable;
}

DCOPObject* KSpreadTable::dcopObject()
{
    if ( !m_dcop )
        m_dcop = new KSpreadTableIface( this );

    return m_dcop;
}

void KSpreadTable::hideTable(bool _hide)
{
    setHidden(_hide);
    if(_hide)
        emit sig_TableHidden(this);
    else
        emit sig_TableShown(this);
}

void KSpreadTable::removeTable()
{
    emit sig_TableRemoved(this);
}

void KSpreadTable::setActiveTable()
{
    emit sig_maxColumn( maxColumn());
    emit sig_maxRow(maxRow() );
    emit sig_TableActivated(this);
    emit sig_updateVBorder( this );
    emit sig_updateHBorder( this );
    emit sig_updateView( this );
}

bool KSpreadTable::setTableName( const QString& name, bool init, bool makeUndo )
{
    if ( map()->findTable( name ) )
        return FALSE;

    if ( m_strName == name )
        return TRUE;

    QString old_name = m_strName;
    m_strName = name;

    if ( init )
        return TRUE;

    QListIterator<KSpreadTable> it( map()->tableList() );
    for( ; it.current(); ++it )
        it.current()->changeCellTabName( old_name, name );
    if(makeUndo)
    {
        if ( !m_pDoc->undoBuffer()->isLocked() )
        {
                KSpreadUndoAction* undo = new KSpreadUndoSetTableName( doc(), this, old_name );
                m_pDoc->undoBuffer()->appendUndo( undo );
        }
    }

    m_pDoc->changeAreaTableName(old_name,name);
    emit sig_nameChanged( this, old_name );

    return TRUE;
}

/**********************************************************
 *
 * KSpreadChild
 *
 **********************************************************/

KSpreadChild::KSpreadChild( KSpreadDoc *parent, KSpreadTable *_table, KoDocument* doc, const QRect& geometry )
  : KoDocumentChild( parent, doc, geometry )
{
  m_pTable = _table;
}

KSpreadChild::KSpreadChild( KSpreadDoc *parent, KSpreadTable *_table ) : KoDocumentChild( parent )
{
  m_pTable = _table;
}


KSpreadChild::~KSpreadChild()
{
}

/**********************************************************
 *
 * ChartChild
 *
 **********************************************************/

ChartChild::ChartChild( KSpreadDoc *_spread, KSpreadTable *_table, KoDocument* doc, const QRect& geometry )
  : KSpreadChild( _spread, _table, doc, geometry )
{
    m_pBinding = 0;
}

ChartChild::ChartChild( KSpreadDoc *_spread, KSpreadTable *_table )
  : KSpreadChild( _spread, _table )
{
    m_pBinding = 0;
}

ChartChild::~ChartChild()
{
    if ( m_pBinding )
        delete m_pBinding;
}

void ChartChild::setDataArea( const QRect& _data )
{
    if ( m_pBinding == 0L )
        m_pBinding = new ChartBinding( m_pTable, _data, this );
    else
        m_pBinding->setDataArea( _data );
}

void ChartChild::update()
{
    if ( m_pBinding )
        m_pBinding->cellChanged( 0 );
}

bool ChartChild::load( const QDomElement& element )
{
    if ( !KSpreadChild::load( element ) )
        return false;

    if ( element.hasAttribute( "left-cell" ) &&
         element.hasAttribute( "top-cell" ) &&
         element.hasAttribute( "right-cell" ) &&
         element.hasAttribute( "bottom-cell" ) )
    {
        QRect r;
        r.setCoords( element.attribute( "left-cell" ).toInt(),
                     element.attribute( "top-cell" ).toInt(),
                     element.attribute( "right-cell" ).toInt(),
                     element.attribute( "bottom-cell" ).toInt() );

        setDataArea( r );
    }

    return true;
}

QDomElement ChartChild::save( QDomDocument& doc )
{
    QDomElement element = KSpreadChild::save( doc );
    element.setTagName( "chart" );

    element.setAttribute( "left-cell", m_pBinding->dataArea().left() );
    element.setAttribute( "right-cell", m_pBinding->dataArea().right() );
    element.setAttribute( "top-cell", m_pBinding->dataArea().top() );
    element.setAttribute( "bottom-cell", m_pBinding->dataArea().bottom() );

    return element;
}

bool ChartChild::loadDocument( KoStore* _store )
{
    bool res = KSpreadChild::loadDocument( _store );
    if ( !res )
        return res;

    // Did we see a cell rectangle ?
    if ( !m_pBinding )
        return true;

    update();

    return true;
}

KoChart::Part* ChartChild::chart()
{
    assert( document()->inherits( "KoChart::Part" ) );
    return static_cast<KoChart::Part *>( document() );
}

#include "kspread_table.moc"
