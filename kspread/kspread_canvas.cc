// -*- mode: c++; c-basic-offset: 4 -*-
#include "kspread_canvas.h"
#include "kspread_table.h"
#include "kspread_cell.h"
#include "kspread_util.h"
#include "kspread_editors.h"
#include "kspread_map.h"

#include "kspread_view.h"
#include "kspread_doc.h"

#include "kspread_tabbar.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kdebug.h>

#include <cassert>
#include <stdio.h>
#include <qlabel.h>
#include <qpainter.h>

KSpreadLocationEditWidget::KSpreadLocationEditWidget( QWidget * _parent, KSpreadView * _view )
  : QLineEdit( _parent, "KSpreadLocationEditWidget" )
{
    m_pView = _view;
}

void KSpreadLocationEditWidget::keyPressEvent( QKeyEvent * _ev )
{
    // Do not handle special keys and accelerators. This is
    // done by QLineEdit.
    if ( _ev->state() & ( Qt::AltButton | Qt::ControlButton ) )
    {
        QLineEdit::keyPressEvent( _ev );
        // Never allow that keys are passed on to the parent.
        _ev->accept();

        return;
    }

    // Handle some special keys here. Eve
    switch( _ev->key() )
    {
    case Key_Return:
    case Key_Enter:
        {
            QString tmp;
            int pos;

            // Set the cell component to uppercase:
            // Table1!a1 -> Table1!A2
            pos = text().find('!');
            if( pos !=- 1 )
                tmp = text().left(pos)+text().mid(pos).upper();
            else
                tmp=text().upper();

            // Selection entered in location widget
            if ( text().contains( ':' ) )
                m_pView->canvasWidget()->gotoLocation( KSpreadRange( tmp, m_pView->doc()->map() ) );
            // Location entered in location widget
            else
                m_pView->canvasWidget()->gotoLocation( KSpreadPoint( tmp, m_pView->doc()->map() ));

            // Set the focus back on the canvas.
            m_pView->canvasWidget()->setFocus();
            _ev->accept();
        }
        break;
    // Escape pressed, restore original value
    case Key_Escape:
        // #### Torben says: This is duplicated code. Bad.
        if ( m_pView->activeTable()->selectionRect().left() == 0 ) {
            setText( util_columnLabel( m_pView->canvasWidget()->markerColumn() )
                     + QString::number( m_pView->canvasWidget()->markerRow() ) );
        } else {
            setText( util_columnLabel( m_pView->activeTable()->selectionRect().left() )
                     + QString::number( m_pView->activeTable()->selectionRect().top() )
                     + ":"
                     + util_columnLabel( m_pView->activeTable()->selectionRect().right() )
                     + QString::number( m_pView->activeTable()->selectionRect().bottom() ) );
        }
        m_pView->canvasWidget()->setFocus();
        _ev->accept();
        break;
    default:
        QLineEdit::keyPressEvent( _ev );
        // Never allow that keys are passed on to the parent.
        _ev->accept();
    }
}

/****************************************************************
 *
 * KSpreadEditWidget
 * The line-editor that appears above the table and allows to
 * edit the cells content.
 *
 ****************************************************************/

KSpreadEditWidget::KSpreadEditWidget( QWidget *_parent, KSpreadCanvas *_canvas,
                                      QButton *cancelButton, QButton *okButton )
  : QLineEdit( _parent, "KSpreadEditWidget" )
{
  m_pCanvas = _canvas;
  // Those buttons are created by the caller, so that they are inserted
  // properly in the layout - but they are then managed here.
  m_pCancelButton = cancelButton;
  m_pOkButton = okButton;
  if ( !m_pCanvas->doc()->isReadWrite() )
    setEnabled( false );
  else
  {
    QObject::connect( m_pCancelButton, SIGNAL( clicked() ),
                      this, SLOT( slotAbortEdit() ) );
    QObject::connect( m_pOkButton, SIGNAL( clicked() ),
                      this, SLOT( slotDoneEdit() ) );
  }
  setEditMode( false ); // disable buttons
}

void KSpreadEditWidget::slotAbortEdit()
{
    m_pCanvas->deleteEditor( false /*discard changes*/ );
    // will take care of the buttons
}

void KSpreadEditWidget::slotDoneEdit()
{
    m_pCanvas->deleteEditor( true /*keep changes*/ );
    // will take care of the buttons
}

void KSpreadEditWidget::keyPressEvent ( QKeyEvent* _ev )
{
    // Dont handle special keys and accelerators
    if ( _ev->state() & ( Qt::AltButton | Qt::ControlButton ) )
    {
        QLineEdit::keyPressEvent( _ev );
        return;
    }

  if ( !m_pCanvas->doc()->isReadWrite() )
    return;

  switch ( _ev->key() )
  {
    case Key_Down:
    case Key_Up:
    case Key_Return:
    case Key_Enter:

      // Send to the canvas, which will handle it.
      QApplication::sendEvent( m_pCanvas, _ev );

      _ev->accept();
      break;

    default:

      QLineEdit::keyPressEvent( _ev );

      if ( !m_pCanvas->editor() )
      {
        // Start editing the current cell
        m_pCanvas->createEditor( KSpreadCanvas::CellEditor );
      }
      setFocus();
      KSpreadTextEditor* cellEditor = (KSpreadTextEditor*) m_pCanvas->editor();
      cellEditor->blockCheckChoose( TRUE );
      cellEditor->setText( text() );
      cellEditor->blockCheckChoose( FALSE );
      cellEditor->setCursorPosition( cursorPosition() );
  }
}

void KSpreadEditWidget::setEditMode( bool mode )
{
  m_pCancelButton->setEnabled(mode);
  m_pOkButton->setEnabled(mode);
}

void KSpreadEditWidget::focusOutEvent( QFocusEvent* ev )
{
  kdDebug(36001) << "EditWidget lost focus" << endl;
  // See comment about setLastEditorWithFocus
  m_pCanvas->setLastEditorWithFocus( KSpreadCanvas::EditWidget );

  QLineEdit::focusOutEvent( ev );
}

void KSpreadEditWidget::setText( const QString& t )
{
  if ( t == text() ) // Why this? (David)
    return;

  QLineEdit::setText( t );
}

/****************************************************************
 *
 * KSpreadCanvas
 *
 ****************************************************************/

KSpreadCanvas::KSpreadCanvas( QWidget *_parent, KSpreadView *_view, KSpreadDoc* _doc )
  : QWidget( _parent, "", WNorthWestGravity | WResizeNoErase | WRepaintNoErase )
{
  length_namecell = 0;
  m_chooseStartTable = 0;
  m_pEditor = 0;
  m_bChoose = FALSE;

  QWidget::setFocusPolicy( QWidget::StrongFocus );

  m_defaultGridPen.setColor( lightGray );
  m_defaultGridPen.setWidth( 1 );
  m_defaultGridPen.setStyle( SolidLine );

  m_iXOffset = 0;
  m_iYOffset = 0;
  m_pView = _view;
  m_pDoc = _doc;
  // m_eAction = DefaultAction;
  m_eMouseAction = NoAction;
  m_bGeometryStarted = false;
  // m_bEditDirtyFlag = false;

  //Now built afterwards(David)
  //m_pEditWidget = m_pView->editWidget();
  m_pPosWidget = m_pView->posWidget();

  setBackgroundMode( PaletteBase );

  setMouseTracking( TRUE );
  m_bMousePressed = false;

  choose_visible = false;

  setFocus();

  (void)new KSpreadToolTip( this );
}

QPoint KSpreadCanvas::marker() const
{
    return activeTable()->marker().topLeft();
}

int KSpreadCanvas::markerColumn() const
{
    return activeTable()->marker().left();
}

int KSpreadCanvas::markerRow() const
{
    return activeTable()->marker().top();
}

void KSpreadCanvas::startChoose()
{
  if ( m_bChoose )
    return;

  // Clear a selection if there is any
  m_i_chooseMarkerColumn = markerColumn();
  m_i_chooseMarkerRow = markerRow();
  activeTable()->setChooseRect( QRect( 0, 0, 0, 0 ) );

  // It is important to enable this AFTER we set the rect!
  m_bChoose = TRUE;
  m_chooseStartTable = activeTable();
}

void KSpreadCanvas::startChoose( const QRect& rect )
{
  activeTable()->setChooseRect( rect );

  m_i_chooseMarkerColumn = rect.right();
  m_i_chooseMarkerRow = rect.bottom();

  // It is important to enable this AFTER we set the rect!
  m_bChoose = TRUE;
  m_chooseStartTable = activeTable();
}

void KSpreadCanvas::endChoose()
{
  if ( !m_bChoose )
    return;

  activeTable()->setChooseRect( QRect( 0, 0, 0, 0 ) );
  m_pView->setActiveTable( m_chooseStartTable );
  kdDebug(36001) << "endChoose len=0" << endl;
  length_namecell = 0;
  m_bChoose = FALSE;
  m_chooseStartTable = 0;
}

KSpreadHBorder* KSpreadCanvas::hBorderWidget()
{
  return m_pView->hBorderWidget();
}

KSpreadVBorder* KSpreadCanvas::vBorderWidget()
{
  return m_pView->vBorderWidget();
}

QScrollBar* KSpreadCanvas::horzScrollBar()
{
  return m_pView->horzScrollBar();
}

QScrollBar* KSpreadCanvas::vertScrollBar()
{
  return m_pView->vertScrollBar();
}

KSpreadTable* KSpreadCanvas::findTable( const QString& _name )
{
  return m_pDoc->map()->findTable( _name );
}

void KSpreadCanvas::insertFormulaChar(int c)
{
  m_pEditor->insertFormulaChar(c);
}

const KSpreadTable* KSpreadCanvas::activeTable() const
{
  return m_pView->activeTable();
}

KSpreadTable* KSpreadCanvas::activeTable()
{
  return m_pView->activeTable();
}

void KSpreadCanvas::gotoLocation( const KSpreadRange & _range )
{
        if ( !_range.isValid() )
        {
                KMessageBox::error( this, i18n( "Invalid cell reference" ) );
                return;
        }
        KSpreadTable * table = activeTable();
        if ( _range.isTableKnown() )
                table = _range.table;
        if ( !table )
        {
                KMessageBox::error( this, i18n("Unknown table name %1" ).arg( _range.tableName ) );
                return;
        }

        gotoLocation( _range.range.left(), _range.range.top(), table, false );
        gotoLocation( _range.range.right(), _range.range.bottom(), table, true );
}


void KSpreadCanvas::gotoLocation( const KSpreadPoint& _cell )
{
  if ( !_cell.isValid() )
  {
    KMessageBox::error( this, i18n("Invalid cell reference") );
    return;
  }

  KSpreadTable* table = activeTable();
  if ( _cell.isTableKnown() )
    table = _cell.table;
  if ( !table )
  {
    KMessageBox::error( this, i18n("Unknown table name %1").arg( _cell.tableName ) );
    return;
  }

  gotoLocation( _cell.pos.x(), _cell.pos.y(), table );
}

void KSpreadCanvas::gotoLocation( int x, int y, KSpreadTable* table, bool make_select,bool /*move_into_area*/ )
{
  //kdDebug(36001) << "KSpreadCanvas::gotoLocation" << " x=" << x << " y=" << y <<
  //  " table=" << table << " make_select=" << (make_select ? "true" : "false" ) << endl;
  if ( table )
    m_pView->setActiveTable( table );
  else
    table = activeTable();

  KSpreadCell* cell = table->cellAt( x, y );
  if ( cell->isObscured() && cell->isObscuringForced() )
  {
    int moveX=cell->obscuringCellsColumn();
    int moveY=cell->obscuringCellsRow();
    cell = table->cellAt( moveX, moveY );
    QRect extraCell;
    extraCell.setCoords(moveX,moveY,moveX+cell->extraXCells(),moveY+cell->extraYCells());
    if( (x-markerColumn())!=0 && extraCell.contains(QPoint(markerColumn(),markerRow())))
        x=cell->extraXCells()+x;
    else if((y-markerRow())!=0 && extraCell.contains(QPoint(markerColumn(),markerRow())))
        y=cell->extraYCells()+y;
    else
        {
        y = moveY;
        x = moveX;
        }
  }

  int xpos = table->columnPos( x, this );
  int ypos = table->rowPos( y, this );

  //kdDebug(36001) << "KSpreadCanvas::gotoLocation : zoom=" << zoom() << endl;
  int minX = (int) (100 * zoom()); // less than that, we scroll
  int minY = (int) (50 * zoom());
  int maxX = (int) (width() - 100 * zoom()); // more than that, we scroll
  int maxY = (int) (height() - 50 * zoom());
  //kdDebug(36001) << "KSpreadCanvas::gotoLocation : height=" << height() << endl;
  //kdDebug(36001) << "KSpreadCanvas::gotoLocation : width=" << width() << endl;

  if ( xpos < minX )
    horzScrollBar()->setValue( xOffset() + xpos - minX );
  else if ( xpos > maxX )
    horzScrollBar()->setValue( xOffset() + xpos - maxX );

  if ( ypos < minY )
    vertScrollBar()->setValue( yOffset() + ypos - minY );
  else if ( ypos > maxY )
    vertScrollBar()->setValue( yOffset() + ypos - maxY );

  QRect selection = activeTable()->selectionRect();

  if ( !make_select )
  {
      // if ( selection.left() != 0 && !move_into_area)
      // activeTable()->unselect();
    activeTable()->setMarker( QPoint( x, y ) );
  }
  else
  {
    if ( selection.left() == 0 )
      selection.setCoords( markerColumn(), markerRow(), markerColumn(), markerRow() );

    if ( markerColumn() == selection.left() )
      selection.setLeft( x );
    else
      selection.setRight( x );

    if ( markerRow() == selection.top() )
      selection.setTop( y );
    else
      selection.setBottom( y );
    selection = selection.normalize();

    // m_iMarkerColumn = x;
    // m_iMarkerRow = y;
    activeTable()->setSelection( selection, QPoint( x, y ) );
  }

  // Perhaps the user is entering a value in the cell.
  // In this case we may not touch the EditWidget
  if ( !m_pEditor )
    m_pView->updateEditWidget();
  updatePosWidget();
}

void KSpreadCanvas::chooseGotoLocation( int x, int y, KSpreadTable* table, bool make_select )
{
  if ( table )
    m_pView->setActiveTable( table );
  else
    table = activeTable();

  KSpreadCell* cell = table->cellAt( x, y );
  if ( cell->isObscured() && cell->isObscuringForced() )
  {
    x = cell->obscuringCellsRow();
    y = cell->obscuringCellsColumn();
  }

  int xpos = table->columnPos( x, this );
  int ypos = table->rowPos( y, this );

  int minX = (int) (100 * zoom()); // less than that, we scroll
  int minY = (int) (50 * zoom());
  int maxX = (int) (width() - 100 * zoom()); // more than that, we scroll
  int maxY = (int) (height() - 50 * zoom());

  if ( xpos < minX )
    horzScrollBar()->setValue( xOffset() + xpos - minX);
  else if ( xpos > maxX )
    horzScrollBar()->setValue( xOffset() + xpos - maxX );

  if ( ypos < minY )
    vertScrollBar()->setValue( yOffset() + ypos - minY );
  else if ( ypos > maxY )
    vertScrollBar()->setValue( yOffset() + ypos - maxY );

  if ( !make_select )
    setChooseMarker( QPoint( x, y ) );
  else
  {
    QRect selection = activeTable()->chooseRect();
    if ( chooseMarkerColumn() == selection.left() )
      selection.setLeft( x );
    else
      selection.setRight( x );

    if ( chooseMarkerRow() == selection.top() )
      selection.setTop( y );
    else
      selection.setBottom( y );
    selection = selection.normalize();
    m_i_chooseMarkerColumn = x;
    m_i_chooseMarkerRow = y;
    activeTable()->setChooseRect( selection );
  }
}

void KSpreadCanvas::slotScrollHorz( int _value )
{
  if ( activeTable() == 0L )
    return;

  if ( _value < 0 )
    _value = 0;

  activeTable()->enableScrollBarUpdates( false );

  // Relative movement
  int dx = m_iXOffset - _value;
  // New absolute position
  m_iXOffset = _value;

  scroll( dx, 0 );

  hBorderWidget()->scroll( dx, 0 );

  activeTable()->enableScrollBarUpdates( true );
}

void KSpreadCanvas::slotScrollVert( int _value )
{
  if ( activeTable() == 0L )
    return;

  if ( _value < 0 )
    _value = 0;

  activeTable()->enableScrollBarUpdates( false );

  // Relative movement
  int dy = m_iYOffset - _value;
  // New absolute position
  m_iYOffset = _value;
  scroll( 0, dy );
  vBorderWidget()->scroll( 0, dy );

  activeTable()->enableScrollBarUpdates( true );
}

void KSpreadCanvas::slotMaxColumn( int _max_column )
{
  int xpos = activeTable()->columnPos( _max_column + 10, this );

  horzScrollBar()->setRange( 0, xpos + xOffset() );
}

void KSpreadCanvas::slotMaxRow( int _max_row )
{
  int ypos = activeTable()->rowPos( _max_row + 10, this );

  vertScrollBar()->setRange( 0, ypos + yOffset() );
}

void KSpreadCanvas::mouseMoveEvent( QMouseEvent * _ev )
{
    // Dont allow modifications if document is readonly.
    if ( !m_pView->koDocument()->isReadWrite() )
        return;

    // Special handling for choose mode.
    if( m_bChoose )
    {
        chooseMouseMoveEvent( _ev );
        return;
    }

    // Working on this table ?
    KSpreadTable *table = activeTable();
    if ( !table )
        return;

    // Get the current selected rectangle
    QRect selection( table->selectionRect() );

    int xpos;
    int ypos;
    int row = table->topRow( _ev->pos().y(), ypos, this );
    int col = table->leftColumn( _ev->pos().x(), xpos, this );

    // Find out where the little "corner" (in lower right direction) is.
    QRect corner;
    // No selection or just complete rows/columns ?
    if ( selection.left() == 0 || selection.right() == 0x7fff || selection.bottom() == 0x7fff )
    {
        int x = table->columnPos( markerColumn(), this );
        int y = table->rowPos( markerRow(), this );
        KSpreadCell *cell = table->cellAt( markerColumn(), markerRow() );
        int w = cell->width( markerColumn(), this );
        int h = cell->height( markerRow(), this );

        corner = QRect( x + w - 2, y + h -1, 5, 5 );
    }
    else // if we have a rectangular selection ( not complete rows or columns )
    {
        int x = table->columnPos( selection.left(), this );
        int y = table->rowPos( selection.top(),  this );
        int x2 = table->columnPos( selection.right(), this );
        KSpreadCell *cell = table->cellAt( selection.right(), selection.top() );
        int tw = cell->width( selection.right(), this );
        int w = ( x2 - x ) + tw;
        cell = table->cellAt( selection.left(), selection.bottom() );
        int y2 = table->rowPos( selection.bottom(), this );
        int th = cell->height( selection.bottom(), this );
        int h = ( y2 - y ) + th;

        corner = QRect( x + w - 2, y + h -1, 5, 5 );
    }

    //
    // Now set the cursor correctly.
    //
    if ( corner.contains( _ev->pos() ) )
      setCursor( sizeFDiagCursor );
    else if ( !m_strAnchor.isEmpty() )
      setCursor( KCursor::handCursor() );
    else
      setCursor( arrowCursor );

    // No marking, selecting etc. in progess? Then quit here.
    if ( m_eMouseAction == NoAction )
        return;

    // Set the new lower right corner of the selection
    if ( col <= m_iMouseStartColumn )
    {
        selection.setLeft( col );
        selection.setRight( m_iMouseStartColumn );
    }
    else
        selection.setRight( col );
    if ( row <= m_iMouseStartRow )
    {
        selection.setTop( row );
        selection.setBottom( m_iMouseStartRow);
    }
    else
        selection.setBottom( row );

    // If nothing changed, then quit
    if ( selection == table->selectionRect() )
        return;

    // Set the new selection
    table->setSelection( selection, QPoint( col, row ), this );

    // Scroll the table if necessary
    if ( _ev->pos().x() < 0 )
        horzScrollBar()->setValue( xOffset() + xpos );
    else if ( _ev->pos().x() > width() )
    {
        ColumnLayout *cl = table->columnLayout( col + 1 );
        xpos = table->columnPos( col + 1, this );
        horzScrollBar()->setValue( xOffset() + ( xpos + cl->width( this ) - width() ) );
    }
    if ( _ev->pos().y() < 0 )
        vertScrollBar()->setValue( yOffset() + ypos );
    else if ( _ev->pos().y() > height() )
    {
        RowLayout *rl = table->rowLayout( row + 1 );
        ypos = table->rowPos( row + 1, this );
        vertScrollBar()->setValue( yOffset() + ( ypos + rl->height( this ) - height() ) );
    }

    // Show where we are now.
    updatePosWidget();

    m_bMouseMadeSelection = true;
}

void KSpreadCanvas::mouseReleaseEvent( QMouseEvent* _ev )
{
  if( m_bChoose )
  {
    chooseMouseReleaseEvent( _ev );
    return;
  }

  m_bMousePressed = false;

  KSpreadTable *table = activeTable();
  if ( !table )
    return;

  QRect selection( table->selectionRect() );

  // The user started the drag in the lower right corner of the marker ?
  if ( m_eMouseAction == ResizeCell )
  {
    KSpreadCell *cell = table->nonDefaultCell( m_iMouseStartColumn, m_iMouseStartRow );
    cell->forceExtraCells( m_iMouseStartColumn, m_iMouseStartRow,
                           selection.right() - selection.left(),
                           selection.bottom() - selection.top() );

    selection.setCoords( 0, 0, 0, 0 );
    table->setSelection( selection, this );
    // m_pView->doc()->setModified( TRUE );
  }
  else if ( m_eMouseAction == AutoFill )
  {
    QRect dest = table->selectionRect();
    table->autofill( m_rctAutoFillSrc, dest );

    // m_pView->doc()->setModified( TRUE );
    selection.setCoords( 0, 0, 0, 0 );
    table->setSelection( selection, this );
  }
  // The user started the drag in the middle of a cell ?
  else if ( m_eMouseAction == Mark )
  {
    // Get the object in the lower right corner
    KSpreadCell *cell = table->cellAt( m_iMouseStartColumn, m_iMouseStartRow );
    // Did we mark only a single cell ?
    // Take care: One cell may obscure other cells ( extra size! ).
    if ( selection.left() + cell->extraXCells() == selection.right() &&
         selection.top() + cell->extraYCells() == selection.bottom() )
    {
      // Delete the selection
      selection.setCoords( 0, 0, 0, 0 );
      table->setSelection( selection, this );
    }
  }

  m_eMouseAction = NoAction;

  m_bMouseMadeSelection = FALSE;
}

void KSpreadCanvas::mousePressEvent( QMouseEvent * _ev )
{
    // If in choose mode, we handle the mouse differently.
    if( m_bChoose )
    {
        chooseMousePressEvent( _ev );
        return;
    }

    KSpreadTable *table = activeTable();
    m_bMousePressed = true;
    if ( !table )
        return;

    // We were editing a cell -> save value and get out of editing mode
    if ( m_pEditor )
    {
        deleteEditor( true ); // save changes
    }

    // Remember current values.
    QRect selection( table->selectionRect() );
    int old_column = markerColumn();
    int old_row = markerRow();

    // Check whether we clicked in the little marker in the lower right
    // corner of a cell or a marked area.
    {
        // Get the position and size of the marker/marked-area
        int xpos;
        int ypos;
        int w, h;

        // No selection or complete rows/columns are selected
        if ( selection.left() == 0 ||
             selection.right() == 0x7fff || selection.bottom() == 0x7fff )
        {
            xpos = table->columnPos( markerColumn(), this );
            ypos = table->rowPos( markerRow(), this );
            KSpreadCell *cell = table->cellAt( markerColumn(), markerRow() );
            w = cell->width( markerColumn() );
            h = cell->height( markerRow() );
        }
        else // if we have a rectangular selection ( not complete rows or columns )
        {
            xpos = table->columnPos( selection.left(), this );
            ypos = table->rowPos( selection.top(), this );
            int x = table->columnPos( selection.right(), this );
            KSpreadCell *cell = table->cellAt( selection.right(), selection.top() );
            int tw = cell->width( selection.right(), this );
            w = ( x - xpos ) + tw;
            cell = table->cellAt( selection.left(), selection.bottom() );
            int y = table->rowPos( selection.bottom(), this );
            int th = cell->height( selection.bottom(), this );
            h = ( y - ypos ) + th;
        }

        // Did we click in the lower right corner of the marker/marked-area ?
        if ( _ev->pos().x() >= xpos + w - 2 && _ev->pos().x() <= xpos + w + 3 &&
             _ev->pos().y() >= ypos + h - 1 && _ev->pos().y() <= ypos + h + 4 )
        {
            // Auto fill ? That is done using the left mouse button.
            if ( _ev->button() == LeftButton )
            {
                m_eMouseAction = AutoFill;
                // Do we have a selection already ?
                if ( selection.left() != 0 && selection.right() != 0x7fff && selection.bottom() != 0x7fff )
                { /* Selection is ok */
                    m_rctAutoFillSrc = selection;
                }
                else // Select the current cell
                {
                    KSpreadCell *cell = table->cellAt( markerColumn(), markerRow() );
                    selection.setCoords( markerColumn(), markerRow(),
                                         markerColumn() + cell->extraXCells(),
                                         markerRow() + cell->extraYCells() );
                    m_rctAutoFillSrc.setCoords( markerColumn(), markerRow(),
                                                markerColumn(), markerRow() );
                }

                m_iMouseStartColumn = markerColumn();
                m_iMouseStartRow = markerRow();
            }
            // Resize a cell (dont with the right mouse button) ?
            // But for that to work there must not be a selection.
            else if ( _ev->button() == MidButton && selection.left() == 0 )
            {
                m_eMouseAction = ResizeCell;
                KSpreadCell *cell = table->cellAt( markerColumn(), markerRow() );
                selection.setCoords( markerColumn(), markerRow(),
                                     markerColumn() + cell->extraXCells(),
                                     markerRow() + cell->extraYCells() );
                m_iMouseStartColumn = markerColumn();
                m_iMouseStartRow = markerRow();
            }

            table->setSelection( selection, this );
            return;
        }
    }

    // In which cell did the user click ?
    int xpos, ypos;
    int row = table->topRow( _ev->pos().y(), ypos, this );
    int col = table->leftColumn( _ev->pos().x(), xpos, this );

    // Unselect a selection ?
    if ( _ev->button() == LeftButton || !selection.contains( QPoint( col, row ) ) )
        table->unselect();

    // Extending an existing selection with the shift button ?
    if ( selection.right() != 0x7fff && selection.bottom() != 0x7fff && _ev->state() & ShiftButton )
    {
        if( col != old_column || row != old_row )
        {
            if(old_column>col)
            {
                int tmp=col;
                col=old_column;
                old_column=tmp;
            }
            if(old_row>row)
            {
                int tmp=row;
                row=old_row;
                old_row=tmp;
            }
            if( selection.left()!=0 && selection.right()!=0
                && selection.top()!=0 && selection.bottom()!=0 )
            {
                //if you have a selection and you go up in the table
                //so bottom is the bottom of the selection
                //not the markercolumn
                if(selection.bottom()>row)
                    row = selection.bottom();
                if(selection.right()>col)
                    col = selection.right();
            }
            selection.setLeft(old_column);
            selection.setRight( col );
            selection.setTop(old_row);
            selection.setBottom( row );
            table->setSelection( selection, QPoint( old_column, old_row ), this );
            //always put Marker in top and left
            //otherwise all functions don't work
            // table->setMarker( QPoint( old_column, old_row ) );

            return;
        }
    }

    // activeTable()->setMarker( QPoint( col, row ) );

    KSpreadCell *cell = table->cellAt( col, row );

    // Go to the upper left corner of the obscuring object
    if ( cell->isObscured() )
    {
        // activeTable()->setSelection( QPoint( cell->obscuringCellsColumn(), cell->obscuringCellsRow() ) );
        col = cell->obscuringCellsColumn();
        row = cell->obscuringCellsRow();
        cell = table->cellAt( col, row );
    }

    // Test whether the mouse is over some anchor
    {
        KSpreadCell *cell = table->visibleCellAt( col, row );
        QString anchor = cell->testAnchor( _ev->pos().x() - xpos,
                                           _ev->pos().y() - ypos, this );
        if ( !anchor.isEmpty() && anchor != m_strAnchor )
            setCursor( KCursor::handCursor() );
        m_strAnchor = anchor;
    }

    // Start a marking action ?
    if ( !m_strAnchor.isEmpty() && _ev->button() == LeftButton )
    {
        gotoLocation( KSpreadPoint( m_strAnchor, m_pDoc->map() ) );
        return;
    }
    else if ( _ev->button() == LeftButton )
    {
        m_eMouseAction = Mark;
        selection.setCoords( col, row,
                             col + cell->extraXCells(),
                             row + cell->extraYCells() );

        table->setSelection( selection, this );
        m_iMouseStartColumn = col;
        m_iMouseStartRow = row;
    }
    else if ( _ev->button() == RightButton )
    {
        // No selection or the mouse press was outside of an existing selection ?
        if ( selection.left() == 0 || !selection.contains( QPoint(col, row )) )
            table->setMarker( QPoint( col, row ) );
    }

    // Paste operation with the middle button ?
    if( _ev->button() == MidButton )
        table->paste( QPoint( markerColumn(), markerRow() ) );

    // Update the edit box
    m_pView->updateEditWidget();

    updatePosWidget();

    // Context menu ?
    if ( _ev->button() == RightButton )
    {
        // TODO: Handle anchor
        QPoint p = mapToGlobal( _ev->pos() );
        m_pView->openPopupMenu( p );
    }
}

void KSpreadCanvas::chooseMouseMoveEvent( QMouseEvent * _ev )
{
  if ( !m_bMousePressed )
    return;

  KSpreadTable *table = activeTable();
  if ( !table )
    return;

  int ypos, xpos;
  int row = table->topRow( _ev->pos().y(), ypos, this );
  int col = table->leftColumn( _ev->pos().x(), xpos, this );

  /*if ( col < m_iMouseStartColumn )
    col = m_iMouseStartColumn;
  if ( row < m_iMouseStartRow )
    row = m_iMouseStartRow;*/

  // Noting changed ?
  QRect selection( table->chooseRect() );
  /*if ( row == selection.bottom() && col == selection.right() )
    return;
  */
  // Set the new lower right corner of the selection
  /*selection.setRight( col );
  selection.setBottom( row );*/
  if ( col <= m_iMouseStartColumn )
     {
     selection.setLeft( col );
     selection.setRight( m_iMouseStartColumn );
     }
  else
     selection.setRight( col );
  if ( row <= m_iMouseStartRow )
     {
     selection.setTop( row );
     selection.setBottom( m_iMouseStartRow);
     }
  else
     selection.setBottom( row );

  table->setChooseRect( selection );

  // Scroll the table if neccessary
  if ( _ev->pos().x() < 0 )
    horzScrollBar()->setValue( xOffset() + xpos );
  else if ( _ev->pos().x() > width() )
  {
    ColumnLayout *cl = table->columnLayout( col + 1 );
    xpos = table->columnPos( col + 1, this );
    horzScrollBar()->setValue( xOffset() + ( xpos + cl->width( this ) - width() ) );
  }

  if ( _ev->pos().y() < 0 )
    vertScrollBar()->setValue( yOffset() + ypos );
  else if ( _ev->pos().y() > height() )
  {
    RowLayout *rl = table->rowLayout( row + 1 );
    ypos = table->rowPos( row + 1, this );
    vertScrollBar()->setValue( yOffset() + ( ypos + rl->height( this ) - height() ) );
  }
}

void KSpreadCanvas::chooseMouseReleaseEvent( QMouseEvent* )
{
  m_bMousePressed = FALSE;
}

void KSpreadCanvas::chooseMousePressEvent( QMouseEvent * _ev )
{
  m_bMousePressed = TRUE;

  KSpreadTable *table = activeTable();
  if ( !table )
    return;
  QRect selection = table->chooseRect();

  int ypos, xpos;
  int row = table->topRow( _ev->pos().y(), ypos, this );
  int col = table->leftColumn( _ev->pos().x(), xpos, this );

  if ( (selection.right() != 0x7fff && selection.bottom() != 0x7fff) &&
      ( _ev->state() & ShiftButton ) )
  {
    if ( col != m_iMouseStartColumn || row != m_iMouseStartRow )
    {
      if ( selection.left() !=0 && selection.right() !=0
          && selection.top() !=0 && selection.bottom() !=0 )
      {
        if ( col < m_iMouseStartColumn )
          col = m_iMouseStartColumn;
        if ( row < m_iMouseStartRow )
          row = m_iMouseStartRow;

        if ( row == selection.bottom() && col == selection.right() )
          return;
      }
      selection.setLeft(m_iMouseStartColumn);
      selection.setRight( col );
      selection.setTop(m_iMouseStartRow);
      selection.setBottom( row );
      table->setChooseRect( selection );
      return;
    }
  }

  setChooseMarkerColumn( col );
  setChooseMarkerRow( row );
  KSpreadCell *cell = table->cellAt( chooseMarkerColumn(), chooseMarkerRow() );

  // Go to the upper left corner of the obscuring object
  if ( cell->isObscured() )
  {
    setChooseMarkerRow( cell->obscuringCellsRow() );
    setChooseMarkerColumn( cell->obscuringCellsColumn() );
    cell = table->cellAt( chooseMarkerColumn(), chooseMarkerRow() );
  }

  selection.setCoords( chooseMarkerColumn(), chooseMarkerRow(),
                       chooseMarkerColumn() + cell->extraXCells(),
                       chooseMarkerRow() + cell->extraYCells() );

  table->setChooseRect( selection );
  m_iMouseStartColumn = chooseMarkerColumn();
  m_iMouseStartRow = chooseMarkerRow();
}

void KSpreadCanvas::mouseDoubleClickEvent( QMouseEvent*  )
{
  if ( m_pView->koDocument()->isReadWrite() )
    createEditor();
}

void KSpreadCanvas::wheelEvent( QWheelEvent* _ev )
{
  if ( vertScrollBar() )
    QApplication::sendEvent( vertScrollBar(), _ev );
}

void KSpreadCanvas::paintEvent( QPaintEvent* _ev )
{
  if ( m_pDoc->isLoading() )
    return;

  if ( !activeTable() )
    return;

  // printf("PAINT EVENT %i %i %i %i\n", _ev->rect().x(), _ev->rect().y(), _ev->rect().width(), _ev->rect().height() );

  QRect rect( _ev->rect() );
  if ( rect.left() < 0 )
      rect.rLeft() = 0;
  if ( rect.right() > width() )
      rect.rRight() = width();
  if ( rect.top() < 0 )
      rect.rTop() = 0;
  if ( rect.bottom() > height() )
      rect.rBottom() = height();

  // printf("PAINT EVENT %i %i %i %i\n", rect.x(), rect.y(), rect.width(), rect.height() );

  QPainter painter;
  painter.begin( this );

  QWMatrix m = m_pView->matrix();
  painter.setWorldMatrix( m );
  m = m.invert();
  QPoint tl = m.map( rect.topLeft() );
  QPoint br = m.map( rect.bottomRight() );

  //kdDebug(36001) << "Mapped topleft to " << tl.x() << ":" << tl.y() << endl;

  painter.save();

  // Clip away children
  QRegion rgn = painter.clipRegion();
  if ( rgn.isEmpty() )
    rgn = QRegion( rect );
  QListIterator<KoDocumentChild> it( m_pDoc->children() );
  for( ; it.current(); ++it )
  {
    if ( ((KSpreadChild*)it.current())->table() == activeTable() &&
         !m_pView->hasDocumentInWindow( it.current()->document() ) )
    {
        rgn -= it.current()->region( painter.worldMatrix() );
    }
  }
  painter.setClipRegion( rgn );

  // Draw content
  m_pDoc->paintContent( painter, QRect( tl, br ), FALSE, activeTable() );

  painter.restore();

  // Draw children
  // QListIterator<PartChild> it( m_pDoc->children() );
  it.toFirst();
  for( ; it.current(); ++it )
  {
      if ( ((KSpreadChild*)it.current())->table() == activeTable() &&
           !m_pView->hasDocumentInWindow( it.current()->document() ) )
    {
      // #### todo: paint only if child is visible inside rect
      painter.save();
      m_pDoc->paintChild( it.current(), painter, m_pView );
      painter.restore();
    }
  }

  painter.end();

  if( choose_visible )
    drawChooseMarker( );
}

void KSpreadCanvas::focusInEvent( QFocusEvent* )
{
  if ( !m_pEditor )
    return;

  kdDebug(36001) << "m_bChoose : " << ( m_bChoose ? "true" : "false" ) << endl;
  // If we are in editing mode, we redirect the
  // focus to the CellEditor or EditWidget
  // And we know which, using lastEditorWithFocus.
  // This screws up <Tab> though (David)
  if ( lastEditorWithFocus() == EditWidget )
  {
    m_pView->editWidget()->setFocus();
    kdDebug(36001) << "Focus to EditWidget" << endl;
    return;
  }

  kdDebug(36001) << "Redirecting focus to editor" << endl;
  m_pEditor->setFocus();
}

void KSpreadCanvas::focusOutEvent( QFocusEvent* )
{
}

void KSpreadCanvas::keyPressEvent ( QKeyEvent * _ev )
{
  KSpreadTable *table = activeTable();
  QString tmp;
  if ( !table )
    return;

  // Dont handle special keys.
  if ( _ev->state() & ( Qt::AltButton | Qt::ControlButton ) )
  {
    QWidget::keyPressEvent( _ev );
    return;
  }

  // Always accept so that events are not
  // passed to the parent.
  _ev->accept();

  // Find out about the current selection/choose.
  QRect selection;
  if ( m_bChoose )
    selection = activeTable()->chooseRect();
  else
    selection = activeTable()->selectionRect();

  // Is this a key that involves goint to another cell ? If yes, do the
  // "end of editing" stuff here, instead of for each case - this bool is
  // also used for make_select (extending selection), so it doesn't include Return
  bool bChangingCells = ( _ev->key() == Key_Down || _ev->key() == Key_Up ||
                          _ev->key() == Key_Left || _ev->key() == Key_Right ||
                          _ev->key() == Key_Prior || _ev->key() == Key_Next );

  // End of editing a cell
  if ( ( bChangingCells || _ev->key() == Key_Return || _ev->key() == Key_Enter )
       && m_pEditor && !m_bChoose )
  {
    deleteEditor( true /*save changes*/ );
  }

  // Are we making a selection right now ? Go thru this only if no selection is made
  // or if we neither selected complete rows nor columns.
  bool make_select = ( _ev->state() & ShiftButton ) == ShiftButton &&
                     ( bChangingCells || _ev->key() == Key_Home || _ev->key() == Key_End );

  switch( _ev->key() )
  {
    case Key_Return:
    case Key_Enter:
      if ( !m_bChoose && markerRow() == 0xFFFF )
        return;
      if ( m_bChoose && chooseMarkerRow() == 0xFFFF )
        return;

      if ( m_bChoose )
        chooseGotoLocation( chooseMarkerColumn(), QMIN( 0x7FFF, chooseMarkerRow() + 1 ), 0, make_select );
      else
      {
        QRect selection = activeTable()->selectionRect();
        if( selection.left() == 0 )
            gotoLocation( markerColumn(), QMIN( 0x7FFF, markerRow() + 1 ), 0, make_select );
        else
        {
            if(markerColumn()<selection.right()&&markerRow()<selection.bottom() )
                gotoLocation( markerColumn(), QMIN( 0x7FFF, markerRow() + 1 ), 0, make_select,true );
            else if( markerRow()==selection.bottom() && markerColumn()<selection.right())
                gotoLocation( markerColumn()+1, QMIN( 0x7FFF, selection.top() ), 0, make_select,true );
            else if( markerRow()==selection.bottom() && markerColumn()==selection.right())
                gotoLocation( selection.left(), QMIN( 0x7FFF, selection.top() ), 0, make_select,true );
            else if(markerColumn()==selection.right() && markerRow()<selection.bottom())
                gotoLocation( markerColumn(), QMIN( 0x7FFF, markerRow() + 1 ), 0, make_select,true );
        }
      }
      return;
    case Key_Down:

      if ( !m_bChoose && markerRow() == 0xFFFF )
        return;
      if ( m_bChoose && chooseMarkerRow() == 0xFFFF )
        return;

      if ( m_bChoose )
        chooseGotoLocation( chooseMarkerColumn(), QMIN( 0x7FFF, chooseMarkerRow() + 1 ), 0, make_select );
      else
        gotoLocation( markerColumn(), QMIN( 0x7FFF, markerRow() + 1 ), 0, make_select );

      return;

    case Key_Up:

      if ( !m_bChoose && markerRow() == 1 )
        return;
      if ( m_bChoose && chooseMarkerRow() == 1 )
        return;

      if ( m_bChoose )
        chooseGotoLocation( chooseMarkerColumn(), QMAX( 1, chooseMarkerRow() - 1 ), 0, make_select );
      else
        gotoLocation( markerColumn(), QMAX( 1, markerRow() - 1 ), 0, make_select );

      return;

    case Key_Right:

      if ( !m_bChoose && markerColumn() >= 26*26)//0xFFFF )
        return;
      if ( m_bChoose && chooseMarkerColumn() >= 26*26)//0xFFFF )
        return;

      if ( m_bChoose )
        chooseGotoLocation( QMIN( 26*26/*0x7FFF*/, chooseMarkerColumn() + 1 ), chooseMarkerRow(), 0, make_select );
      else
        gotoLocation( QMIN( /*26*26*/0x7FFF, markerColumn() + 1 ), markerRow(), 0, make_select );

      return;

    case Key_Left:

      if ( !m_bChoose && markerColumn() == 1 )
        return;
      if ( m_bChoose && chooseMarkerColumn() == 1 )
        return;

      if ( m_bChoose )
        chooseGotoLocation( QMAX( 1, chooseMarkerColumn() - 1 ), chooseMarkerRow(), 0, make_select );
      else
        gotoLocation( QMAX( 1, markerColumn() - 1 ), markerRow(), 0, make_select );

      return;

    case Key_Escape:

      if ( m_pEditor )
        deleteEditor( false );

      _ev->accept(); // ?
      return;

    case Key_Home:

      // We are in edit mode -> go beginning of line
      if ( m_pEditor )
      {
        // (David) Do this for text editor only, not formula editor...
        // Don't know how to avoid this hack (member var for editor type ?)
        if ( m_pEditor->inherits("KSpreadTextEditor") )
          QApplication::sendEvent( m_pEditWidget, _ev );
        // What to do for a formula editor ?
      }
      else
      {
        if ( !m_bChoose && markerColumn() == 1 )
          return;
        if ( m_bChoose && chooseMarkerColumn() == 1 )
          return;
        if ( m_bChoose )
          chooseGotoLocation( 1, markerRow(), 0, make_select );
        else
          gotoLocation( 1, markerRow(), 0, make_select );
      }
      return;

    case Key_Prior:

      if( !m_bChoose && markerRow() == 1 )
        return;
      if( m_bChoose && chooseMarkerRow() == 1 )
        return;

      if ( m_bChoose )
        chooseGotoLocation( chooseMarkerColumn(), QMAX( 1, chooseMarkerRow() - 10 ), 0, make_select );
      else
        gotoLocation( markerColumn(), QMAX( 1, markerRow() - 10 ), 0, make_select );

      return;

    case Key_Next:

      if( !m_bChoose && markerRow() == 0x7FFF )
        return;
      if( m_bChoose && chooseMarkerRow() == 0x7FFF )
        return;

      if ( m_bChoose )
        chooseGotoLocation( chooseMarkerColumn(), QMIN( 0x7FFF, chooseMarkerRow() + 10 ), 0, make_select );
      else
        gotoLocation( markerColumn(), QMIN( 0x7FFF, markerRow() + 10 ), 0, make_select );

      return;

    case Key_Delete:

       activeTable()->clearSelection( QPoint( markerColumn(), markerRow() ) );

       return;

    default:

      // No null character ...
      if ( _ev->text().isEmpty() || !m_pView->koDocument()->isReadWrite() )
      {
        _ev->accept();
        return;
      }

      if ( !m_pEditor && !m_bChoose )
      {
        if ( _ev->text() == QString::fromLatin1("*") )
          createEditor( FormulaEditor );
        else
        {
          // Switch to editing mode
          createEditor( CellEditor );
          m_pEditor->handleKeyPressEvent( _ev );
        }
      }
      else if ( m_pEditor )
        m_pEditor->handleKeyPressEvent( _ev );

      return;
  }

  /**
   * Tell the KSpreadView event handler and enable
   * makro recording by the way.
   */
  // _ev->accept();

  // m_pView->eventKeyPressed( _ev, m_bChoose );
}

void KSpreadCanvas::deleteEditor( bool saveChanges )
{
  if( !m_pEditor )
    return;
  // We need to set the line-edit out of edit mode,
  // but only if we are using it (text editor)
  // A bit of a hack - perhaps we should store the editor mode ?
  if ( m_pEditor->inherits("KSpreadTextEditor") )
    m_pEditWidget->setEditMode( false );

  QString t = m_pEditor->text();
  // Delete the cell editor first and after that update the document.
  // That means we get a synchronous repaint after the cell editor
  // widget is gone. Otherwise we may get painting errors.
  delete m_pEditor;
  m_pEditor = 0;

  if (saveChanges)
    m_pView->setText( t );
  else
    m_pView->updateEditWidget();

  setFocus();

  m_pView->enableFormulaToolBar( TRUE );
}

void KSpreadCanvas::createEditor()
{
  KSpreadCell* cell = activeTable()->cellAt( markerColumn(), markerRow() );

  if ( cell && cell->content() == KSpreadCell::VisualFormula )
  {
    QString tmp = cell->text();
    createEditor( FormulaEditor );
    m_pEditor->setText( tmp.right( tmp.length() - 1 ) );
  }
  else
  {
    createEditor( CellEditor );
    if ( cell )
      m_pEditor->setText(cell->text());
  }
}

void KSpreadCanvas::createEditor( EditorType ed )
{
  KSpreadTable *table = activeTable();
  if ( !m_pEditor )
  {
    KSpreadCell* cell = activeTable()->cellAt( marker() );
    if ( ed == CellEditor )
    {
      m_pView->enableFormulaToolBar( FALSE );
      m_pEditWidget->setEditMode( true );

      m_pEditor = new KSpreadTextEditor( cell, this );
    }
    else if( ed == FormulaEditor )
    {
      m_pView->enableFormulaToolBar( TRUE );

      m_pEditor = new KSpreadFormulaEditor( cell, this );
    }

    int w, h;
    int min_w = cell->width( markerColumn(), this );
    int min_h = cell->height( markerRow(), this );
    if ( cell->isDefault() )
    {
      w = min_w;
      h = min_h;
      kdDebug(36001) << "DEFAULT" << endl;
    }
    else
    {
      w = cell->extraWidth() + 1;
      h = cell->extraHeight() + 1;
      kdDebug(36001) << "HEIGHT=" << min_h << " EXTRA=" << h << endl;
    }
    int xpos = table->columnPos( markerColumn(), this );
    int ypos = table->rowPos( markerRow(), this );
    QPalette p = m_pEditor->palette();
    QColorGroup g( p.normal() );
    g.setColor( QColorGroup::Text, cell->textColor( markerColumn(), markerRow() ) );
    g.setColor( QColorGroup::Background, cell->bgColor( markerColumn(), markerRow() ) );
    m_pEditor->setPalette( QPalette( g, p.disabled(), g ) );
    m_pEditor->setFont( cell->textFont( markerColumn(), markerRow() ) );
    m_pEditor->setGeometry( xpos, ypos, w, h );
    m_pEditor->setMinimumSize( QSize( min_w, min_h ) );
    m_pEditor->show();
    kdDebug(36001) << "FOCUS1" << endl;
    m_pEditor->setFocus();
    kdDebug(36001) << "FOCUS2" << endl;
  }
}

//---------------------------------------------
//
// Drawing Engine
//
//---------------------------------------------

void KSpreadCanvas::updateCellRect( const QRect &_rect )
{
    updateSelection( _rect, activeTable()->markerRect() );
}

void KSpreadCanvas::updateSelection( const QRect &_old_sel, const QRect& old_marker )
{
    KSpreadTable *table = activeTable();
    if ( !table )
        return;

    // Calculate some value which are needed later
    QRect new_sel = table->selectionRect();
    QRect new_marker = table->marker();

    QRect old_sel = _old_sel;
    QRect old_marker_outer;
    old_marker_outer.setCoords( QMAX( 1, old_marker.left() - 1 ), QMAX( 1, old_marker.top() - 1 ),
                                old_marker.right() + 1, old_marker.bottom() + 1 );
    // QRect old_marker_rect = _old_sel;
    // if ( old_marker_rect.left() == 0 )
    // old_marker_rect =  QRect( m_marker, m_marker );

    // Old selection was empty -> Just use the marker
    if ( old_sel.left() == 0 )
        old_sel = old_marker;
    // New selection was empty -> Just use the marker
    if ( new_sel.left() == 0 )
        new_sel = table->markerRect();

    // Which cells were not marked, but were located next to some
    // selection? They may need repainting, too.
    QRect old_outer;
    old_outer.setCoords( QMAX( 1, old_sel.left() - 1 ), QMAX( 1, old_sel.top() - 1 ),
                         old_sel.right() + 1, old_sel.bottom() + 1 );

    // Which cells are located next to the new selection ?
    // QRect new_outer;
    // outer.setCoords( new_sel.left() - 1, new_sel.top() - 1, new_sel.right() + 1, new_sel.bottom() + 1 );

    // Determine which area might be subject of repainting.
    QRect uni = old_sel.unite( new_sel ).unite( old_outer ).unite( old_marker ).unite( new_marker );

    // Limit the number of cells
    uni.rBottom() = QMIN( 9999, uni.bottom() );
    uni.rRight() = QMIN( 9999, uni.right() );

    // qDebug("UNI %i/%i %i/%i", uni.left(), uni.top(), uni.right(), uni.bottom() );

    // Determine the position of "uni" rect on the screen.
    int left = table->columnPos( uni.left() );
    int top = table->rowPos( uni.top() );
    // int right = table->columnPos( uni.right() + 1 );
    // int bottom = table->rowPos( uni.bottom() + 1 );

    QPainter painter;
    painter.begin( this );

    // Do the view transformation.
    QWMatrix m = m_pView->matrix();
    painter.setWorldMatrix( m );

    // Which part of the document is visible ? To determine this
    // just transform the viewport rectangle with the inverse
    // matrix, since this matrix usually transforms from document
    // coordinates to view coordinates.
    m = m.invert();
    QPoint tl = m.map( QPoint( 0, 0 ) );
    QPoint br = m.map( QPoint( width(), height() ) );
    QRect view( tl, br );

    //
    // Clip away children
    //

    QRegion rgn = painter.clipRegion();
    if ( rgn.isEmpty() )
        rgn = QRegion( QRect( 0, 0, width(), height() ) );
    QListIterator<KoDocumentChild> it( m_pDoc->children() );
    for( ; it.current(); ++it )
    {
        if ( ((KSpreadChild*)it.current())->table() == activeTable() &&
             !m_pView->hasDocumentInWindow( it.current()->document() ) )
            rgn -= it.current()->region( painter.worldMatrix() );
    }
    painter.setClipRegion( rgn );

    QPen pen;
    pen.setWidth( 1 );
    painter.setPen( pen );

    int top_row = uni.top();
    int bottom_row = uni.bottom();
    int left_col = uni.left();
    int right_col = uni.right();

    QRect r;

    int ypos = top;
    for ( int y = top_row; y <= bottom_row && ypos <= view.bottom(); y++ )
    {
        RowLayout *row_lay = table->rowLayout( y );
        int xpos = left;

        for ( int x = left_col; x <= right_col && xpos <= view.right(); x++ )
        {
            ColumnLayout *col_lay = table->columnLayout( x );
            KSpreadCell *cell = table->cellAt( x, y, TRUE );

            QPoint p( x, y );

            //
            // Determine which parts of the marker this cell used to
            // display.
            //
            // Parts of this code are copied from KSpreadCell::paintCell.
            //
            QRect larger;
            larger.setCoords( old_sel.left() - 1, old_sel.top() - 1,
                              old_sel.right() + 1, old_sel.bottom() + 1 );

            QPoint lr = old_sel.bottomRight();

            int old_border = 0;

            if ( old_sel.left() == x && old_sel.right() == x &&
                 old_sel.top() == y && old_sel.bottom() == y &&
                 ( cell->extraXCells() || cell->extraYCells() ) )
                old_border = 1 | 2 | 4 | 8;
            else if ( old_sel.contains( QPoint(x, y) ) )
            {
                // Upper border ?
                if ( y == old_sel.top() )
                    old_border |= 1;
                // Left border ?
                if ( x == old_sel.left() )
                    old_border |= 2;
                // Lower border ?
                if ( y == old_sel.bottom() )
                    old_border |= 4;
                // Right border ?
                if ( x == old_sel.right() )
                    old_border |= 8;
                // The little rect in the lower right corner ?
                if ( p == lr )
                    old_border |= 256;
            }
            else if ( larger.contains( QPoint(x, y) ) )
            {
                // Upper border ?
                if ( x >= old_sel.left() && x <= old_sel.right() && y - 1 == old_sel.bottom() )
                    old_border |= 1;
                // Left border ?
                if ( y >= old_sel.top() && y <= old_sel.bottom() && x - 1 == old_sel.right() )
                    old_border |= 2;
                // Lower border ?
                if ( x >= old_sel.left() && x <= old_sel.right() && y + 1 == old_sel.top() )
                    old_border |= 4;
                // Right border ?
                if ( y >= old_sel.top() && y <= old_sel.bottom() && x + 1 == old_sel.left() )
                    old_border |= 8;
                // Upper left corner ?
                if ( p == larger.topLeft() )
                    old_border |= 16;
                // Lower left corner ?
                if ( p == larger.bottomLeft() )
                    old_border |= 32;
                // Lower right corner ?
                if ( p == larger.bottomRight() )
                    old_border |= 64;
                // Upper right corner ?
                if ( p == larger.topRight() )
                    old_border |= 128;
                // The little rect in the upper left corner ?
                if ( p == larger.bottomRight() )
                    old_border |= 512;
                // The little rect in the upper right corner ?
                if ( x == lr.x() && y == lr.y() + 1 )
                    old_border |= 1024;
                // The little rect in the upper left corner ?
                if ( x == lr.x() + 1 && y == lr.y() )
                    old_border |= 2048;
            }

            //
            // Determine which parts of the marker this cell displays now.
            //
            // Parts of this code are copied from KSpreadCell::paintCell.
            //
            larger.setCoords( new_sel.left() - 1, new_sel.top() - 1,
                              new_sel.right() + 1, new_sel.bottom() + 1 );
            lr = new_sel.bottomRight();

            int new_border = 0;

            if ( new_sel.left() == x && new_sel.right() == x &&
                 new_sel.top() == y && new_sel.bottom() == y &&
                 ( cell->extraXCells() || cell->extraYCells() ) )
                new_border = 1 | 2 | 4 | 8;
            else if ( new_sel.contains( QPoint(x, y) ) )
            {
                // Upper border ?
                if ( y == new_sel.top() )
                    new_border |= 1;
                // Left border ?
                if ( x == new_sel.left() )
                    new_border |= 2;
                // Lower border ?
                if ( y == new_sel.bottom() )
                    new_border |= 4;
                // Right border ?
                if ( x == new_sel.right() )
                    new_border |= 8;
                // The little rect in the lower right corner ?
                if ( p == lr )
                    old_border |= 256;
            }
            else if ( larger.contains( QPoint(x, y) ) )
            {
                // Upper border ?
                if ( x >= new_sel.left() && x <= new_sel.right() && y - 1 == new_sel.bottom() )
                    new_border |= 1;
                // Left border ?
                if ( y >= new_sel.top() && y <= new_sel.bottom() && x - 1 == new_sel.right() )
                    new_border |= 2;
                // Lower border ?
                if ( x >= new_sel.left() && x <= new_sel.right() && y + 1 == new_sel.top() )
                    new_border |= 4;
                // Right border ?
                if ( y >= new_sel.top() && y <= new_sel.bottom() && x + 1 == new_sel.left() )
                    new_border |= 8;
                // Upper left corner ?
                if ( p == larger.topLeft() )
                    new_border |= 16;
                // Lower left corner ?
                if ( p == larger.bottomLeft() )
                    new_border |= 32;
                // Lower right corner ?
                if ( p == larger.bottomRight() )
                    new_border |= 64;
                // Upper right corner ?
                if ( p == larger.topRight() )
                    new_border |= 128;
                // The little rect in the upper left corner ?
                if ( p == larger.bottomRight() )
                    old_border |= 512;
                // The little rect in the upper right corner ?
                if ( x == lr.x() && y == lr.y() + 1 )
                    old_border |= 1024;
                // The little rect in the upper left corner ?
                if ( x == lr.x() + 1 && y == lr.y() )
                    old_border |= 2048;
            }

            // Draw if the cell
            // a) was part of the selection, but is no longer.
            // b) the exact opposite of a)
            // c) One of its edges used to show the marker but does no longer
            if ( ( new_sel.contains( p ) ^ old_sel.contains( p ) ) ||   // a) and b)
                 ( ( old_border & new_border ) != old_border ) )        // c)
            {
                cell->paintCell( view, painter, xpos, ypos, x, y, col_lay, row_lay, &r );
            }

            xpos += col_lay->width();
        }

        ypos += row_lay->height();
    }

    painter.end();
}

//---------------------------------------------
//
// Choose Marker
//
//---------------------------------------------

void KSpreadCanvas::updateChooseMarker( const QRect& _old, const QRect& _new )
{
  if ( isChooseMarkerVisible() )
  {
    drawChooseMarker( _old );
    drawChooseMarker( _new );
  }
  else
  {
    choose_visible = TRUE;
    drawChooseMarker( _new );
  }

  if ( _new.left() == 0 || !m_bChoose || !m_pEditor )
  {
    kdDebug(36001) << "updateChooseMarker len=0" << endl;
    length_namecell = 0;
    return;
  }

  KSpreadTable* table = activeTable();

  // ##### Torben: Clean up here!
  QString name_cell;

  kdDebug(36001) << m_chooseStartTable->tableName().local8Bit() << ", "
                 << table->tableName().local8Bit() << endl;
  if( m_chooseStartTable != table )
  {
    if ( _new.left() >= _new.right() && _new.top() >= _new.bottom() )
      name_cell = util_cellName( table, _new.left(), _new.top() );
    else
      name_cell = util_rangeName( table, _new );
  }
  else
  {
    if ( _new.left() >= _new.right() && _new.top() >= _new.bottom() )
      name_cell = util_cellName( _new.left(), _new.top() );
    else
      name_cell = util_rangeName( _new );
  }

  int old = length_namecell;
  length_namecell= name_cell.length();
  length_text = m_pEditor->text().length();
  kdDebug(36001) << "updateChooseMarker2 len=" << length_namecell << endl;

  QString text = m_pEditor->text();
  QString res = text.left( m_pEditor->cursorPosition() - old ) + name_cell + text.right( text.length() - m_pEditor->cursorPosition() );
  int pos = m_pEditor->cursorPosition() - old;

  ((KSpreadTextEditor*)m_pEditor)->blockCheckChoose( TRUE );
  m_pEditor->setText( res );
  ((KSpreadTextEditor*)m_pEditor)->blockCheckChoose( FALSE );
  m_pEditor->setCursorPosition( pos + length_namecell );
  kdDebug(36001) << "old=" << old << " len=" << length_namecell << " pos=" << pos << endl;
}

void KSpreadCanvas::updatePosWidget()
{
    QRect selection = m_pView->activeTable()->selectionRect();
    QString buffer;
    QString tmp;
    if ( selection.left() == 0 )
    {
        if(activeTable()->getLcMode())
        {
            buffer="L"+tmp.setNum( markerRow() );
            buffer+="C"+tmp.setNum( markerColumn() );
        }
        else
        {
            buffer=util_columnLabel( markerColumn() );
            buffer+=tmp.setNum( markerRow() );
        }
    }
  /*else if(selection.right() != 0x7fff && selection.bottom() != 0x7fff )
        {
        if(activeTable()->getLcMode())
                {
                buffer=tmp.setNum( (selection.bottom()-m_iMarkerRow+1) )+"Lx";
                buffer+=tmp.setNum((selection.right()-m_iMarkerColumn+1))+"C";
                }
        else
                {
                buffer=util_columnLabel( m_iMarkerColumn );
                buffer+=tmp.setNum(m_iMarkerRow);
                buffer+=":";
                buffer+=util_columnLabel( selection.right() );
                buffer+=tmp.setNum(selection.bottom());
                }
        }*/
  else
  {
        if(activeTable()->getLcMode())
        {
            buffer=tmp.setNum( (selection.bottom()-selection.top()+1) )+"Lx";
            if(selection.right()==0x7FFF)
                buffer+=tmp.setNum((26*26-selection.left()+1))+"C";
            else
                buffer+=tmp.setNum((selection.right()-selection.left()+1))+"C";
        }
        else
        {
                //Problem columnLabel return @@@@ when column >26*26
                //=> it's not a good display
                //=> for the moment I display pos of marker
                buffer=util_columnLabel( selection.left() );
                buffer+=tmp.setNum(selection.top());
                buffer+=":";
                buffer+=util_columnLabel( selection.right() );
                buffer+=tmp.setNum(selection.bottom());
                //buffer=activeTable()->columnLabel( m_iMarkerColumn );
                //buffer+=tmp.setNum(m_iMarkerRow);
        }
  }

  m_pPosWidget->setText(buffer);
}


void KSpreadCanvas::drawChooseMarker()
{
  drawChooseMarker( activeTable()->chooseRect() );
}

void KSpreadCanvas::drawChooseMarker( const QRect& selection )
{
  // Draw nothing if the selection is empty
  if ( selection.left() == 0 )
    return;

  QPainter painter;
  painter.begin( this );

  int xpos;
  int ypos;
  int w, h;

  if ( selection.left() == 0 || selection.right() == 0x7fff || selection.bottom() == 0x7fff )
  {
    xpos = activeTable()->columnPos( chooseMarkerColumn(), this );
    ypos = activeTable()->rowPos( chooseMarkerRow(), this );
    KSpreadCell *cell = activeTable()->cellAt( chooseMarkerColumn(), chooseMarkerRow() );
    w = cell->width( chooseMarkerColumn(), this );
    h = cell->height( chooseMarkerRow(), this );
  }
  else
  {
    xpos = activeTable()->columnPos( selection.left(), this );
    ypos = activeTable()->rowPos( selection.top(), this );
    int x = activeTable()->columnPos( selection.right(), this );
    KSpreadCell *cell = activeTable()->cellAt( selection.right(), selection.top() );
    int tw = cell->width( selection.right(), this );
    w = ( x - xpos ) + tw;
    cell = activeTable()->cellAt( selection.left(), selection.bottom() );
    int y = activeTable()->rowPos( selection.bottom(), this );
    int th = cell->height( selection.bottom(), this );
    h = ( y - ypos ) + th;
  }

  RasterOp rop = painter.rasterOp();

  painter.setRasterOp( NotROP );
  QPen pen;
  pen.setWidth( 2 );
  pen.setStyle(DashLine);
  painter.setPen( pen );

  painter.drawLine( xpos - 2, ypos - 1, xpos + w + 2, ypos - 1 );
  painter.drawLine( xpos - 1, ypos + 1, xpos - 1, ypos + h + 3 );
  painter.drawLine( xpos + 1, ypos + h + 1, xpos + w - 3, ypos + h + 1 );
  painter.drawLine( xpos + w, ypos + 1, xpos + w, ypos + h - 2 );
  // painter.fillRect( xpos + w - 2, ypos + h - 1, 5, 5, black );
  painter.setRasterOp( rop );

  painter.end();
}

void KSpreadCanvas::setChooseMarker( const QPoint& p )
{
  if ( p.x() == m_i_chooseMarkerColumn && p.y() == m_i_chooseMarkerRow )
    return;

  m_i_chooseMarkerRow = p.y();
  m_i_chooseMarkerColumn = p.x();
  // This will trigger a redraw
  activeTable()->setChooseRect( QRect( p.x(), p.y(), 1, 1 ) );
}

void KSpreadCanvas::adjustArea()
{
  QRect selection( activeTable()->selectionRect() );
  // Columns selected
  if( selection.left() != 0 && selection.bottom() == 0x7FFF )
  {
    for (int x=selection.left(); x <= selection.right(); x++ )
    {
      hBorderWidget()->adjustColumn(x);
    }
  }
  // Rows selected
  else if(selection.left() != 0 && selection.right() == 0x7FFF )
  {
    for(int y = selection.top(); y <= selection.bottom(); y++ )
    {
      vBorderWidget()->adjustRow(y);
    }
  }
  // No selection
  else if( selection.left() == 0 || selection.top() == 0 ||
           selection.bottom() == 0 || selection.right() == 0 )
  {
    vBorderWidget()->adjustRow(markerRow());
    hBorderWidget()->adjustColumn(markerColumn());
  }
  // Selection of a rectangular area
  else
  {
    for (int x=selection.left(); x <= selection.right(); x++ )
    {
      hBorderWidget()->adjustColumn(x);
    }
    for(int y = selection.top(); y <= selection.bottom(); y++ )
    {
      vBorderWidget()->adjustRow(y);
    }
  }
}

void KSpreadCanvas::equalizeRow()
{
  QRect selection( activeTable()->selectionRect() );
  bool selected = ( selection.left() != 0 );
  RowLayout *rl;
  int size;
  if(selected)
  {
    rl = m_pView->activeTable()->rowLayout(selection.top());
    size=rl->height(this);
    for(int i=selection.top()+1;i<=selection.bottom();i++)
      size=QMAX(m_pView->activeTable()->rowLayout(i)->height(this),size);
    for(int i=selection.top()+1;i<=selection.bottom();i++)
      m_pView->vBorderWidget()->resizeRow(size,i );
  }

}

void KSpreadCanvas::equalizeColumn()
{
  ColumnLayout *cl;
  QRect selection( activeTable()->selectionRect() );
  bool selected = ( selection.left() != 0 );
  int size;
  if(selected)
  {
    cl = m_pView->activeTable()->columnLayout(selection.left());
    size=cl->width(this);
    for(int i=selection.left()+1;i<=selection.right();i++)
      size=QMAX(m_pView->activeTable()->columnLayout(i)->width(this),size);
    for(int i=selection.left()+1;i<=selection.right();i++)
      m_pView->hBorderWidget()->resizeColumn(size,i );
  }

}

/****************************************************************
 *
 * KSpreadVBorder
 *
 ****************************************************************/

KSpreadVBorder::KSpreadVBorder( QWidget *_parent, KSpreadCanvas *_canvas, KSpreadView *_view)
  : QWidget( _parent, "", WNorthWestGravity | WResizeNoErase | WRepaintNoErase )
{
  m_pView = _view;
  m_pCanvas = _canvas;
  m_lSize = 0L;

  setBackgroundMode( PaletteButton );
  setMouseTracking( TRUE );
  m_bResize = FALSE;
  m_bSelection = FALSE;
}

void KSpreadVBorder::mousePressEvent( QMouseEvent * _ev )
{
  m_bResize = FALSE;
  m_bSelection = FALSE;
  if(!m_pView->koDocument()->isReadWrite())
    return;

  KSpreadTable *table = m_pCanvas->activeTable();
  assert( table );

  // Find the first visible row and the y position of this row.
  int y = 0;
  int row = table->topRow( 0, y, m_pCanvas );

  // Did the user click between two rows ?
  while ( y < height() )
  {
    int h = table->rowLayout( row )->height( m_pCanvas );
    row++;
    if ( _ev->pos().y() >= y + h - 1 && _ev->pos().y() <= y + h + 1 )
      m_bResize = TRUE;
    y += h;
  }

  // So he clicked between two rows ?
  if ( m_bResize )
  {
    // Determine row to resize
    int tmp;
    m_iResizedRow = table->topRow( _ev->pos().y() - 3, tmp, m_pCanvas );

    paintSizeIndicator( _ev->pos().y(), true );
  }
  else
  {
    m_bSelection = TRUE;

    int tmp;
    int hit_row = table->topRow( _ev->pos().y(), tmp, m_pCanvas );
    m_iSelectionAnchor = hit_row;

    QRect selection;
    selection.setCoords( 1, hit_row, 0x7FFF, hit_row );

    table->setSelection( selection, m_pCanvas );

    if ( _ev->button() == RightButton )
    {
      QPoint p = mapToGlobal( _ev->pos() );
      m_pView->popupRowMenu( p );
      m_bSelection=FALSE;
    }
  }

}

void KSpreadVBorder::mouseReleaseEvent( QMouseEvent * _ev )
{
    KSpreadTable *table = m_pCanvas->activeTable();
    assert( table );
    if( !m_pView->koDocument()->isReadWrite() )
        return;

    if ( m_bResize )
    {
        // Remove size indicator painted by paintSizeIndicator
        QPainter painter;
        painter.begin( m_pCanvas );
        painter.setRasterOp( NotROP );
        painter.drawLine( 0, m_iResizePos, m_pCanvas->width(), m_iResizePos );
        painter.end();

        int start = m_iResizedRow;
        int end = m_iResizedRow;
        QRect selection = m_pCanvas->activeTable()->selectionRect();
        if( selection.left() != 0 && selection.right() == 0x7FFF )
        {
            if( selection.contains( QPoint( 1, m_iResizedRow ) ) )
            {
                start=selection.top();
                end=selection.bottom();
            }
        }
        int height = 0;
        int y = table->rowPos( m_iResizedRow, m_pCanvas );
        if (( m_pCanvas->zoom() * (float)( _ev->pos().y() - y ) ) < (20.0* m_pCanvas->zoom()) )
            height = (int)( 20.0 * m_pCanvas->zoom() );
        else
            height = _ev->pos().y() - y;

        for(int i = start; i <= end; i++ )
        {
            RowLayout *rl = table->nonDefaultRowLayout( i );
            rl->setHeight( height, m_pCanvas );
        }

        delete m_lSize;
        m_lSize = 0;
        m_pView->koDocument()->setModified(true);
    }

    m_bSelection = FALSE;
    m_bResize = FALSE;
}

void KSpreadVBorder::adjustRow(int _row)
{
  int adjust;
  int select;
  if(_row==-1)
  {
    adjust=m_pCanvas->activeTable()->adjustRow(QPoint( m_pCanvas->markerColumn(), m_pCanvas->markerRow() ));
    select=m_iSelectionAnchor;
  }
  else
  {
    adjust=m_pCanvas->activeTable()->adjustRow(QPoint( m_pCanvas->markerColumn(), m_pCanvas->markerRow() ),_row);
    select=_row;
  }
  if(adjust!=-1)
  {
    KSpreadTable *table = m_pCanvas->activeTable();
    assert( table );
    RowLayout *rl = table->nonDefaultRowLayout( select );
    adjust=QMAX((int)(20* m_pCanvas->zoom()),adjust);
    rl->setHeight(adjust,m_pCanvas);
    m_pView->koDocument()->setModified(true);
  }
}

void KSpreadVBorder::resizeRow(int resize,int nb  )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  ASSERT( table );
  if(nb==-1)
  {
    RowLayout *rl = table->nonDefaultRowLayout( m_iSelectionAnchor );
    resize = QMAX( (int)(20* m_pCanvas->zoom()), resize );
    rl->setHeight( resize, m_pCanvas );
  }
  else
  {
    QRect selection( table->selectionRect() );
    if(selection.bottom()==0 ||selection.top()==0 || selection.left()==0
       || selection.right()==0)
    {
      RowLayout *rl = table->nonDefaultRowLayout( m_pCanvas->markerRow() );
      resize=QMAX((int)(20* m_pCanvas->zoom()), resize);
      rl->setHeight( resize, m_pCanvas );
    }
    else
    {
      RowLayout *rl;
      for (int i=selection.top();i<=selection.bottom();i++)
      {
        rl= table->nonDefaultRowLayout( i );
        resize=QMAX((int)(20* m_pCanvas->zoom()), resize);
        rl->setHeight( resize, m_pCanvas );
      }
    }
  }
  m_pView->koDocument()->setModified(true);
}

void KSpreadVBorder::mouseMoveEvent( QMouseEvent * _ev )
{
  if(!m_pView->koDocument()->isReadWrite())
    return;

  KSpreadTable *table = m_pCanvas->activeTable();
  assert( table );
  // The button is pressed and we are resizing ?
  if ( m_bResize )
  {
    paintSizeIndicator( _ev->pos().y(), false );
  }
  // The button is pressed and we are selecting ?
  else if ( m_bSelection )
  {
    int y = 0;
    int row = table->topRow( _ev->pos().y(), y, m_pCanvas );
    QRect selection = table->selectionRect();

    if ( row < m_iSelectionAnchor )
    {
      selection.setTop( row );
      selection.setBottom( m_iSelectionAnchor );
    }
    else
    {
      selection.setBottom( row );
      selection.setTop( m_iSelectionAnchor );
    }
    table->setSelection( selection, m_pCanvas );

    if ( _ev->pos().y() < 0 )
      m_pCanvas->vertScrollBar()->setValue( m_pCanvas->yOffset() + y );
    else if ( _ev->pos().y() > m_pCanvas->height() )
    {
      RowLayout *rl = table->rowLayout( row + 1 );
      y = table->rowPos( row + 1, m_pCanvas );
      m_pCanvas->vertScrollBar()->setValue( m_pCanvas->yOffset()
                                            + y + rl->height( m_pCanvas )
                                            - m_pCanvas->height() );
    }
  }
  // No button is pressed and the mouse is just moved
  else
  {
    int y = 0;
    int row = table->topRow( 0, y, m_pCanvas );

    while ( y < height() )
    {
      int h = table->rowLayout( row )->height( m_pCanvas );
      row++;
      if ( _ev->pos().y() >= y + h - 1 && _ev->pos().y() <= y + h + 1 )
      {
        setCursor(splitVCursor);
        return;
      }
      y += h;
    }

    setCursor( arrowCursor );
  }
}

void KSpreadVBorder::wheelEvent( QWheelEvent* _ev )
{
  if ( m_pCanvas->vertScrollBar() )
    QApplication::sendEvent( m_pCanvas->vertScrollBar(), _ev );
}

void KSpreadVBorder::paintSizeIndicator( int mouseY, bool firstTime )
{
    KSpreadTable *table = m_pCanvas->activeTable();
    assert( table );

    QPainter painter;
    painter.begin( m_pCanvas );
    painter.setRasterOp( NotROP );
    if (!firstTime)
      painter.drawLine( 0, m_iResizePos, m_pCanvas->width(), m_iResizePos );

    m_iResizePos = mouseY;
    // Dont make the row have a height < 20 pixel.
    int twenty = (int)( 20.0 * m_pCanvas->zoom() );
    int y = table->rowPos( m_iResizedRow, m_pCanvas );
    if ( m_iResizePos < y + twenty )
      m_iResizePos = y + twenty;
    painter.drawLine( 0, m_iResizePos, m_pCanvas->width(), m_iResizePos );

    painter.end();

    QString tmpSize=i18n("Height: %1").arg((int)((m_iResizePos-y)/m_pCanvas->zoom()));
    painter.begin(this);
    int len = painter.fontMetrics().width(tmpSize );
    int hei = painter.fontMetrics().height( );
    painter.end();
    if(!m_lSize)
    {
          m_lSize=new QLabel(m_pCanvas);
          m_lSize->setGeometry(3,3+y,len+2, hei+2 ) ;
          m_lSize->setAlignment(Qt::AlignVCenter);
          m_lSize->setText(tmpSize);
          m_lSize->show();
    }
    else
    {
          m_lSize->setGeometry(3,3+y,len+2, hei+2 );
          m_lSize->setText(tmpSize);
    }
}

void KSpreadVBorder::paintEvent( QPaintEvent* _ev )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  if ( !table )
    return;

  QPainter painter;
  painter.begin( this );
  QPen pen;
  pen.setWidth( 1 );
  painter.setPen( pen );
  // painter.setBackgroundColor( colorGroup().base() );

  // painter.eraseRect( _ev->rect() );

  //QFontMetrics fm = painter.fontMetrics();
  // Matthias Elter: This causes a SEGFAULT in ~QPainter!
  // Only god and the trolls know why ;-)
  // bah...took me quite some time to track this one down...

  painter.setClipRect( _ev->rect() );

  int ypos;
  int top_row = table->topRow( _ev->rect().y(), ypos, m_pCanvas );
  int bottom_row = table->bottomRow( _ev->rect().bottom(), m_pCanvas );

  QRect selection( table->selectionRect() );

  QFont normalFont = painter.font();
  QFont boldFont = normalFont;
  boldFont.setBold( TRUE );

  for ( int y = top_row; y <= bottom_row; y++ )
  {
    bool highlighted = ( selection.left() != 0 && y >= selection.top() &&
                      y <= selection.bottom() );
    bool selected = ( highlighted && selection.right() == 0x7FFF );

    RowLayout *row_lay = table->rowLayout( y );

    if ( selected )
    {
      QBrush fillSelected( colorGroup().brush( QColorGroup::Highlight ) );
      qDrawShadePanel( &painter, 0, ypos, YBORDER_WIDTH,
                  row_lay->height( m_pCanvas ), colorGroup(), FALSE, 1,
                  &fillSelected );
    }
    else if ( highlighted )
    {
      QBrush fillHighlighted( colorGroup().brush( QColorGroup::Background ) );
      qDrawShadePanel( &painter, 0, ypos, YBORDER_WIDTH,
                  row_lay->height( m_pCanvas ), colorGroup(), true, 1,
                  &fillHighlighted );
    }
    else
    {
      QBrush fill( colorGroup().brush( QColorGroup::Background ) );
      qDrawShadePanel( &painter, 0, ypos, YBORDER_WIDTH, row_lay->height( m_pCanvas ), colorGroup(), FALSE, 1, &fill );
    }

    char buffer[ 20 ];
    sprintf( buffer, "%i", y );

    // Reset painter
    painter.setFont( normalFont );
    painter.setPen( colorGroup().text() );

    if ( selected )
      painter.setPen( colorGroup().highlightedText() );
    else if ( highlighted )
      painter.setFont( boldFont );
    int len = painter.fontMetrics().width(buffer );
    painter.drawText( (YBORDER_WIDTH-len)/2, ypos +
                    ( row_lay->height( m_pCanvas ) + painter.fontMetrics().ascent() - painter.fontMetrics().descent() ) / 2, buffer );

    ypos += row_lay->height( m_pCanvas );
  }
  m_pCanvas->updatePosWidget();
  painter.end();
}

/****************************************************************
 *
 * KSpreadHBorder
 *
 ****************************************************************/

KSpreadHBorder::KSpreadHBorder( QWidget *_parent, KSpreadCanvas *_canvas,KSpreadView *_view )
  : QWidget( _parent, "", WNorthWestGravity | WResizeNoErase | WRepaintNoErase )
{
  m_pView = _view;
  m_pCanvas = _canvas;
  m_lSize = 0L;
  setBackgroundMode( PaletteButton );
  setMouseTracking( TRUE );
  m_bResize = FALSE;
  m_bSelection = FALSE;
}

void KSpreadHBorder::mousePressEvent( QMouseEvent * _ev )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  assert( table );
  if(!m_pView->koDocument()->isReadWrite())
    return;

  m_bResize = FALSE;
  m_bSelection = FALSE;

  int x = 0;
  int col = table->leftColumn( 0, x, m_pCanvas );

  while ( x < width() && !m_bResize )
  {
    int w = table->columnLayout( col )->width( m_pCanvas );
    col++;
    if ( _ev->pos().x() >= x + w - 1 && _ev->pos().x() <= x + w + 1 )
      m_bResize = TRUE;
    x += w;
  }

  if ( m_bResize )
  {
    // Determine the column to resize
    int tmp;
    m_iResizedColumn = table->leftColumn( _ev->pos().x() - 3, tmp, m_pCanvas );
    paintSizeIndicator( _ev->pos().x(), true );
  }
  else
  {
    m_bSelection = TRUE;

    table->unselect();
    int tmp;
    int hit_col = table->leftColumn( _ev->pos().x(), tmp, m_pCanvas );
    m_iSelectionAnchor = hit_col;
    QRect r;
    r.setCoords( hit_col, 1, hit_col, 0x7FFF );
    table->setSelection( r, m_pCanvas );
    if ( _ev->button() == RightButton )
    {
      QPoint p = mapToGlobal( _ev->pos() );
      m_pView->popupColumnMenu( p );
      m_bSelection=FALSE;
    }
  }
}

void KSpreadHBorder::mouseReleaseEvent( QMouseEvent * _ev )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  assert( table );
  if(!m_pView->koDocument()->isReadWrite())
    return;

  if ( m_bResize )
  {
    // Remove size indicator painted by paintSizeIndicator
    QPainter painter;
    painter.begin( m_pCanvas );
    painter.setRasterOp( NotROP );
    painter.drawLine( m_iResizePos, 0, m_iResizePos, m_pCanvas->height() );
    painter.end();

    int start=m_iResizedColumn;
    int end=m_iResizedColumn;
    QRect selection = m_pCanvas->activeTable()->selectionRect();
    if(selection.left()!=0 && selection.bottom()==0x7FFF)
    {
        if(selection.contains(QPoint(m_iResizedColumn,1)))
                {
                start=selection.left();
                end=selection.right();
                }
    }
    int width=0;
    int x = table->columnPos( m_iResizedColumn, m_pCanvas );
    if ( ( m_pCanvas->zoom() * (float)( _ev->pos().x() - x ) ) < (20.0* m_pCanvas->zoom()) )
        width= (int)(20.0* m_pCanvas->zoom());
    else
        width=_ev->pos().x() - x;
    for(int i=start;i<=end;i++)
        {
        ColumnLayout *cl = table->nonDefaultColumnLayout( i );
        cl->setWidth( width, m_pCanvas );
        }
    delete m_lSize;
    m_lSize=0;
    m_pView->koDocument()->setModified(true);
  }

  m_bSelection = FALSE;
  m_bResize = FALSE;
}

void KSpreadHBorder::adjustColumn(int _col)
{
  int adjust;
  int select;
  if( _col==-1 )
  {
    adjust = m_pCanvas->activeTable()->adjustColumn(QPoint( m_pCanvas->markerColumn(), m_pCanvas->markerRow() ));
    select=m_iSelectionAnchor;
  }
  else
  {
    adjust=m_pCanvas->activeTable()->adjustColumn(QPoint( m_pCanvas->markerColumn(), m_pCanvas->markerRow() ),_col);
    select=_col;
  }

  if(adjust!=-1)
  {
    KSpreadTable *table = m_pCanvas->activeTable();
    assert( table );
    ColumnLayout *cl = table->nonDefaultColumnLayout( select );

    adjust = QMAX( (int)(20 * m_pCanvas->zoom()), adjust );
    cl->setWidth( adjust, m_pCanvas );
    m_pView->koDocument()->setModified(true);
  }
}


void KSpreadHBorder::resizeColumn(int resize,int nb  )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  ASSERT( table );
  if( nb == -1 )
  {
    ColumnLayout *cl = table->nonDefaultColumnLayout( m_iSelectionAnchor );
    resize = QMAX( (int)(20* m_pCanvas->zoom()), resize );
    cl->setWidth( resize, m_pCanvas );
  }
  else
  {
    QRect selection( table->selectionRect() );
    if( selection.bottom() == 0 || selection.top() == 0 || selection.left() == 0 ||
        selection.right() == 0 )
    {
      ColumnLayout *cl = table->nonDefaultColumnLayout( m_pCanvas->markerColumn() );

      resize = QMAX( (int)(20* m_pCanvas->zoom()), resize );
      cl->setWidth( resize, m_pCanvas );
    }
    else
    {
      ColumnLayout *cl;
      for (int i=selection.left();i<=selection.right();i++)
      {
        cl= table->nonDefaultColumnLayout( i );

        resize = QMAX( (int)(20* m_pCanvas->zoom()), resize );
        cl->setWidth( resize, m_pCanvas );
      }
    }
  }
  m_pView->koDocument()->setModified(true);
}

void KSpreadHBorder::mouseDoubleClickEvent( QMouseEvent * _ev )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  assert( table );
  if(!m_pView->koDocument()->isReadWrite())
    return;

  int x = 1;
  int col = table->leftColumn( 1, x, m_pCanvas );

  while ( x < width() )
  {
    int w = table->columnLayout( col )->width( m_pCanvas );
    col++;
    if ( _ev->pos().x() >= x + w - 1 && _ev->pos().x() <= x + w + 1 )
    {
      m_bSelection = TRUE;
      table->unselect();
      m_iSelectionAnchor = --col;
      QRect r;
      r.setCoords( col, 1, col, 0x7FFF );
      table->setSelection( r, m_pCanvas );
      m_bSelection = FALSE;
      adjustColumn();
      m_pView->koDocument()->setModified(true);
      return;
    }
    x += w;
  }
}

void KSpreadHBorder::mouseMoveEvent( QMouseEvent * _ev )
{
  KSpreadTable *table = m_pCanvas->activeTable();
  assert( table );
  if(!m_pView->koDocument()->isReadWrite())
    return;

  if ( m_bResize )
  {
    paintSizeIndicator( _ev->pos().x(), false );
  }
  else if ( m_bSelection )
  {
    int x = 0;
    int col = table->leftColumn( _ev->pos().x(), x, m_pCanvas );
    QRect r = table->selectionRect();

    if ( col < m_iSelectionAnchor )
    {
      r.setLeft( col );
      r.setRight( m_iSelectionAnchor );
    }
    else
    {
      r.setRight( col );
      r.setLeft( m_iSelectionAnchor );
    }
    table->setSelection( r, m_pCanvas );

    if ( _ev->pos().x() < 0 )
      m_pCanvas->horzScrollBar()->setValue( m_pCanvas->xOffset() + x );
    else if ( _ev->pos().x() > m_pCanvas->width() )
    {
      ColumnLayout *cl = table->columnLayout( col + 1 );
      x = table->columnPos( col + 1, m_pCanvas );
      m_pCanvas->horzScrollBar()->setValue( m_pCanvas->xOffset() +
                                            ( x + cl->width( m_pCanvas ) - m_pCanvas->width() ) );
    }

  }
  // Perhaps we have to modify the cursor
  else
  {
    int x = 0;
    int col = table->leftColumn( 0, x, m_pCanvas );

    while ( x < width() )
    {
      int w = table->columnLayout( col )->width( m_pCanvas );
      col++;
      if ( _ev->pos().x() >= x + w - 1 && _ev->pos().x() <= x + w + 1 )
      {
        setCursor(splitHCursor);
        return;
      }
      x += w;
    }
    setCursor( arrowCursor );
  }
}

void KSpreadHBorder::wheelEvent( QWheelEvent* _ev )
{
  if ( m_pCanvas->horzScrollBar() )
    QApplication::sendEvent( m_pCanvas->horzScrollBar(), _ev );
}

void KSpreadHBorder::paintSizeIndicator( int mouseX, bool firstTime )
{
    KSpreadTable *table = m_pCanvas->activeTable();
    assert( table );

    QPainter painter;
    painter.begin( m_pCanvas );
    painter.setRasterOp( NotROP );
    if (!firstTime)
      painter.drawLine( m_iResizePos, 0, m_iResizePos, m_pCanvas->height() );

    m_iResizePos = mouseX;
    int twenty = (int)( 20.0 * m_pCanvas->zoom() );
    // Dont make the column have a width < 20 pixels.
    int x = table->columnPos( m_iResizedColumn, m_pCanvas );
    if ( m_iResizePos < x + twenty )
      m_iResizePos = x + twenty;
    painter.drawLine( m_iResizePos, 0, m_iResizePos, m_pCanvas->height() );
    painter.end();

    QString tmpSize=i18n("Width : %1").arg((int)((m_iResizePos-x)/m_pCanvas->zoom()));
    painter.begin(this);
    int len = painter.fontMetrics().width(tmpSize );
    int hei = painter.fontMetrics().height( );
    painter.end();
    if(!m_lSize)
    {
        m_lSize=new QLabel(m_pCanvas);
        m_lSize->setGeometry(x+3,3,len+2, hei+2 ) ;
        m_lSize->setAlignment(Qt::AlignVCenter);
        m_lSize->setText(tmpSize);
        m_lSize->show();
    }
    else
    {
        m_lSize->setGeometry(x+3,3,len+2, hei+2 ) ;
        m_lSize->setText(tmpSize);
    }
}

void KSpreadHBorder::paintEvent( QPaintEvent* _ev )
{
  KSpreadTable *table = m_pCanvas->activeTable();

  if (!table )
    return;

  QPainter painter;
  painter.begin( this );
  QPen pen;
  pen.setWidth( 1 );
  painter.setPen( pen );
  painter.setBackgroundColor( white );

  // painter.eraseRect( _ev->rect() );

  //QFontMetrics fm = painter.fontMetrics();
  // Matthias Elter: This causes a SEGFAULT in ~QPainter!
  // Only god and the trolls know why ;-)
  // bah...took me quite some time to track this one down...

  // Determine which columns need painting
  int xpos;
  int left_col = table->leftColumn( _ev->rect().x(), xpos, m_pCanvas );
  int right_col = table->rightColumn( _ev->rect().right(), m_pCanvas );

  QRect selection( table->selectionRect() );

  QFont normalFont = painter.font();
  QFont boldFont = normalFont;
  boldFont.setBold( TRUE );

  for ( int x = left_col; x <= right_col; x++ )
  {
    bool highlighted = ( selection.left() != 0 && x >= selection.left() &&
                      x <= selection.right() );
    bool selected = ( highlighted && selection.bottom() == 0x7FFF );

    ColumnLayout *col_lay = table->columnLayout( x );

    if ( selected )
    {
      QBrush fillSelected( colorGroup().brush( QColorGroup::Highlight ) );
      qDrawShadePanel( &painter, xpos, 0, col_lay->width( m_pCanvas ),
                  XBORDER_HEIGHT, colorGroup(), FALSE, 1, &fillSelected );
    }
    else if ( highlighted )
    {
      QBrush fillHighlighted( colorGroup().brush( QColorGroup::Background ) );
      qDrawShadePanel( &painter, xpos, 0, col_lay->width( m_pCanvas ),
                  XBORDER_HEIGHT, colorGroup(), true, 1, &fillHighlighted );
    }
    else
    {
      QBrush fill( colorGroup().brush( QColorGroup::Background ) );
      qDrawShadePanel( &painter, xpos, 0, col_lay->width( m_pCanvas ),
                  XBORDER_HEIGHT, colorGroup(), FALSE, 1, &fill );
    }

    // Reset painter
    painter.setFont( normalFont );
    painter.setPen( colorGroup().text() );

    if ( selected )
      painter.setPen( colorGroup().highlightedText() );
    else if ( highlighted )
      painter.setFont( boldFont );
    if(!m_pView->activeTable()->getShowColumnNumber())
        {
        int len = painter.fontMetrics().width( util_columnLabel(x) );
        painter.drawText( xpos + ( col_lay->width( m_pCanvas ) - len ) / 2,
                      ( XBORDER_HEIGHT + painter.fontMetrics().ascent() -
                        painter.fontMetrics().descent() ) / 2,
                      util_columnLabel(x) );
        }
    else
        {
        QString tmp;
        int len = painter.fontMetrics().width( tmp.setNum(x) );
        painter.drawText( xpos + ( col_lay->width( m_pCanvas ) - len ) / 2,
                      ( XBORDER_HEIGHT + painter.fontMetrics().ascent() -
                        painter.fontMetrics().descent() ) / 2,
                      tmp.setNum(x) );
        }
    xpos += col_lay->width( m_pCanvas );
  }
  m_pCanvas->updatePosWidget();
  painter.end();
}

/****************************************************************
 *
 * KSpreadToolTip
 *
 ****************************************************************/

KSpreadToolTip::KSpreadToolTip( KSpreadCanvas* canvas )
    : QToolTip( canvas ), m_canvas( canvas )
{
}

void KSpreadToolTip::maybeTip( const QPoint& p )
{
    KSpreadTable *table = m_canvas->activeTable();
    if ( !table )
        return;

    // Over which cell is the mouse ?
    int ypos, xpos;
    int row = table->topRow( p.y(), ypos, m_canvas );
    int col = table->leftColumn( p.x(), xpos, m_canvas );
    KSpreadCell* cell = table->visibleCellAt( col, row );
    if ( !cell )
        return;

    // Get the comment
    QString comment= cell->comment();

    // Determine position and width of the current cell.
    cell = table->cellAt( col, row );
    int u = cell->width( col, m_canvas );

    // Special treatment for obscured cells.
    if ( cell->isObscured() && cell->isObscuringForced() )
    {
        // Find the obscuring cell
        int moveX = cell->obscuringCellsColumn();
        int moveY = cell->obscuringCellsRow();
        cell = table->cellAt( moveX, moveY );

        // Use the obscuring cells dimensions
        u = cell->width( moveX, m_canvas );
        xpos = table->columnPos( moveX, m_canvas );
        ypos = table->rowPos( moveY, m_canvas );
    }

    // Is the cursor over the comment marker (if there is any) then
    // show the comment.
    QRect marker( xpos + u - 10, ypos, 10, 10 );
    if ( marker.contains( p ) )
    {
        tip( marker, comment );
    }
}

#include "kspread_canvas.moc"
