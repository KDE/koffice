/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres, nandres@web.de

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

#include "kspread_dlg_goalseek.h"

#include "kspread_canvas.h"
#include "kspread_cell.h"
#include "kspread_doc.h"
#include "kspread_map.h"
#include "kspread_sheet.h"
#include "kspread_undo.h"
#include "kspread_util.h"
#include "kspread_view.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>

#include <math.h>
KSpreadGoalSeekDlg::KSpreadGoalSeekDlg( KSpreadView * parent,  QPoint const & marker,
                                        const char * name, bool, WFlags fl )
  : KDialog( parent, name, false, fl ),
    m_pView( parent ),
    m_maxIter( 1000 ),
    m_restored( true ),
    m_focus(0),
    m_anchor( m_pView->canvasWidget()->selectionInfo()->selectionAnchor() ),
    m_marker( m_pView->canvasWidget()->marker() ),
    m_selection( m_pView->canvasWidget()->selection() )
{
  setWFlags( Qt::WDestructiveClose );

  if ( !name )
    setName( "KSpreadGoalSeekDlg" );

  resize( 458, 153 );
  setCaption( i18n( "Goal Seek" ) );
  setSizeGripEnabled( true );

  KSpreadGoalSeekDlgLayout = new QGridLayout( this, 1, 1, 11, 6, "KSpreadGoalSeekDlgLayout");

  m_startFrame = new QFrame( this, "m_startFrame" );
  m_startFrame->setFrameShape( QFrame::StyledPanel );
  m_startFrame->setFrameShadow( QFrame::Raised );
  m_startFrameLayout = new QGridLayout( m_startFrame, 1, 1, 11, 6, "m_startFrameLayout");

  QLabel * TextLabel4 = new QLabel( m_startFrame, "TextLabel4" );
  TextLabel4->setText( i18n( "To value:" ) );
  m_startFrameLayout->addWidget( TextLabel4, 1, 0 );

  m_targetValueEdit = new QLineEdit( m_startFrame, "m_targetValueEdit" );
  m_startFrameLayout->addWidget( m_targetValueEdit, 1, 1 );

  m_targetEdit = new QLineEdit( m_startFrame, "m_targetEdit" );
  m_startFrameLayout->addWidget( m_targetEdit, 0, 1 );
  m_targetEdit->setText( KSpreadCell::name( marker.x(), marker.y() ) );

  m_sourceEdit = new QLineEdit( m_startFrame, "m_sourceEdit" );
  m_startFrameLayout->addWidget( m_sourceEdit, 2, 1 );

  QLabel * TextLabel5 = new QLabel( m_startFrame, "TextLabel5" );
  TextLabel5->setText( i18n( "By changing cell:" ) );

  m_startFrameLayout->addWidget( TextLabel5, 2, 0 );

  QLabel * TextLabel3 = new QLabel( m_startFrame, "TextLabel3" );
  TextLabel3->setText( i18n( "Set cell:" ) );

  m_startFrameLayout->addWidget( TextLabel3, 0, 0 );
  KSpreadGoalSeekDlgLayout->addWidget( m_startFrame, 0, 0 );

  QVBoxLayout * Layout5 = new QVBoxLayout( 0, 0, 6, "Layout5");

  m_buttonOk = new QPushButton( this, "m_buttonOk" );
  m_buttonOk->setText( i18n( "&Start" ) );
  m_buttonOk->setAccel( 276824143 );
  m_buttonOk->setAutoDefault( TRUE );
  m_buttonOk->setDefault( TRUE );
  Layout5->addWidget( m_buttonOk );

  m_buttonCancel = new KPushButton( KStdGuiItem::cancel(), this, "m_buttonCancel" );
  m_buttonCancel->setAccel( 276824131 );
  m_buttonCancel->setAutoDefault( TRUE );
  Layout5->addWidget( m_buttonCancel );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout5->addItem( spacer );

  KSpreadGoalSeekDlgLayout->addMultiCellLayout( Layout5, 0, 1, 1, 1 );

  m_resultFrame = new QFrame( this, "m_resultFrame" );
  m_resultFrame->setFrameShape( QFrame::StyledPanel );
  m_resultFrame->setFrameShadow( QFrame::Raised );
  m_resultFrame->setMinimumWidth( 350 );
  m_resultFrameLayout = new QGridLayout( m_resultFrame, 1, 1, 11, 6, "m_resultFrameLayout");

  m_currentValueLabel = new QLabel( m_resultFrame, "m_currentValueLabel" );
  m_currentValueLabel->setText( i18n( "Current value:" ) );

  m_resultFrameLayout->addWidget( m_currentValueLabel, 2, 0 );

  m_newValueDesc = new QLabel( m_resultFrame, "m_newValueDesc" );
  m_newValueDesc->setText( i18n( "New value:" ) );

  m_resultFrameLayout->addWidget( m_newValueDesc, 1, 0 );

  m_newValue = new QLabel( m_resultFrame, "m_newValue" );
  m_newValue->setText( "m_targetValueEdit" );

  m_resultFrameLayout->addWidget( m_newValue, 1, 1 );

  m_currentValue = new QLabel( m_resultFrame, "m_currentValue" );
  m_currentValue->setText( "m_currentValue" );

  m_resultFrameLayout->addWidget( m_currentValue, 2, 1 );

  m_resultText = new QLabel( m_resultFrame, "m_resultText" );
  m_resultText->setText( "Goal seeking with cell <cell> found <a | no> solution:" );
  m_resultText->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );

  m_resultFrameLayout->addMultiCellWidget( m_resultText, 0, 0, 0, 1 );

  //  KSpreadGoalSeekDlgLayout->addWidget( m_resultFrame, 1, 0 );

  m_resultFrame->hide();

  m_tableName = m_pView->activeTable()->tableName();

  // Allow the user to select cells on the spreadsheet.
  m_pView->canvasWidget()->startChoose();

  qApp->installEventFilter( this );

  // signals and slots connections
  connect( m_buttonOk, SIGNAL( clicked() ), this, SLOT( buttonOkClicked() ) );
  connect( m_buttonCancel, SIGNAL( clicked() ), this, SLOT( buttonCancelClicked() ) );

  connect( m_pView, SIGNAL( sig_chooseSelectionChanged( KSpreadSheet*, const QRect& ) ),
           this, SLOT( slotSelectionChanged( KSpreadSheet *, const QRect & ) ) );

  // tab order
  setTabOrder( m_targetEdit,      m_targetValueEdit );
  setTabOrder( m_targetValueEdit, m_sourceEdit );
  setTabOrder( m_sourceEdit,      m_buttonOk );
  setTabOrder( m_buttonOk,        m_buttonCancel );
}

KSpreadGoalSeekDlg::~KSpreadGoalSeekDlg()
{
  kdDebug() << "~KSpreadGoalSeekDlg" << endl;

  if ( !m_restored )
  {
    m_pView->doc()->emitBeginOperation( false );
    m_sourceCell->setValue(m_oldSource);
    m_targetCell->setCalcDirtyFlag();
    m_targetCell->calc();
    m_pView->slotUpdateView( m_pView->activeTable() );
  }
}

bool KSpreadGoalSeekDlg::eventFilter( QObject* obj, QEvent* ev )
{
  if ( obj == m_targetValueEdit && ev->type() == QEvent::FocusIn )
    m_focus = m_targetValueEdit;
  else if ( obj == m_targetEdit && ev->type() == QEvent::FocusIn )
    m_focus = m_targetEdit;
  else if ( obj == m_sourceEdit && ev->type() == QEvent::FocusIn )
    m_focus = m_sourceEdit;
  else
    return FALSE;

  if ( m_focus )
    m_pView->canvasWidget()->startChoose();

  return FALSE;
}

void KSpreadGoalSeekDlg::closeEvent ( QCloseEvent * e )
{
  e->accept();
}

void KSpreadGoalSeekDlg::slotSelectionChanged( KSpreadSheet * _table, const QRect & _selection )
{
  if ( !m_focus )
    return;

  if ( _selection.left() <= 0 )
    return;

  if ( _selection.left() >= _selection.right() && _selection.top() >= _selection.bottom() )
  {
    int dx = _selection.right();
    int dy = _selection.bottom();
    QString tmp;

    tmp.setNum( dy );
    tmp = _table->tableName() + "!" + util_encodeColumnLabelText( dx ) + tmp;
    m_focus->setText( tmp );
  }
  else
  {
    QString area = util_rangeName( _table, _selection );
    m_focus->setText( area );
  }
}

void KSpreadGoalSeekDlg::buttonOkClicked()
{
  KSpreadDoc * pDoc = m_pView->doc();
  pDoc->emitBeginOperation( false );
  if (m_maxIter > 0)
  {
    KSpreadSheet * table = m_pView->activeTable();

    KSpreadPoint source( m_sourceEdit->text(), table->map(), table );
    if (!source.isValid())
    {
      KMessageBox::error( this, i18n("Cell reference is invalid!") );
      m_sourceEdit->selectAll();
      m_sourceEdit->setFocus();

      m_pView->slotUpdateView( m_pView->activeTable() );
      return;
    }

    KSpreadPoint target( m_targetEdit->text(), table->map(), table );
    if (!target.isValid())
    {
      KMessageBox::error( this, i18n("Cell reference is invalid!") );
      m_targetEdit->selectAll();
      m_targetEdit->setFocus();

      m_pView->slotUpdateView( m_pView->activeTable() );
      return;
    }

    bool ok = false;
    double goal = m_targetValueEdit->text().toDouble( &ok );
    if ( !ok )
    {
      KMessageBox::error( this, i18n("Target value is invalid!") );
      m_targetValueEdit->selectAll();
      m_targetValueEdit->setFocus();

      m_pView->slotUpdateView( m_pView->activeTable() );
      return;
    }

    m_sourceCell = source.cell();
    m_targetCell = target.cell();

    if ( !m_sourceCell->value().isNumber() )
    {
      KMessageBox::error( this, i18n("Source cell must contain a numeric value!") );
      m_sourceEdit->selectAll();
      m_sourceEdit->setFocus();

      m_pView->slotUpdateView( m_pView->activeTable() );
      return;
    }

    if ( !m_targetCell->isFormula() )
    {
      KMessageBox::error( this, i18n("Target cell must contain a formula!") );
      m_targetEdit->selectAll();
      m_targetEdit->setFocus();

      m_pView->slotUpdateView( m_pView->activeTable() );
      return;
    }

    m_buttonOk->setText( i18n("&OK") );
    m_buttonOk->setEnabled(false);
    m_buttonCancel->setEnabled(false);
    KSpreadGoalSeekDlgLayout->addWidget( m_resultFrame, 0, 0 );
    m_startFrame->hide();
    m_resultFrame->show();
    if ( m_startFrame->width() > 350 )
      m_resultFrame->setMinimumWidth( m_startFrame->width() );

    m_restored = false;

    startCalc( m_sourceCell->value().asFloat(), goal );
    m_pView->slotUpdateView( m_pView->activeTable() );

    return;
  }
  else
  {
    if ( !pDoc->undoBuffer()->isLocked() )
    {
      KSpreadUndoSetText * undo
        = new KSpreadUndoSetText( pDoc, m_pView->activeTable(), QString::number(m_oldSource),
                                  m_sourceCell->column(), m_sourceCell->row(),
                                  m_sourceCell->formatType() );

      pDoc->undoBuffer()->appendUndo( undo );
    }

    m_restored = true;
  }
  chooseCleanup();

  m_pView->slotUpdateView( m_pView->activeTable() );
  accept();
}

void KSpreadGoalSeekDlg::buttonCancelClicked()
{
  if ( !m_restored )
  {
    m_pView->doc()->emitBeginOperation( false );
    m_sourceCell->setValue(m_oldSource);
    m_targetCell->setCalcDirtyFlag();
    m_targetCell->calc();
    m_restored = true;
    m_pView->slotUpdateView( m_pView->activeTable() );
  }

  chooseCleanup();
  reject();
}

void KSpreadGoalSeekDlg::chooseCleanup()
{
  m_pView->canvasWidget()->endChoose();

  KSpreadSheet * table = 0;

  // Switch back to the old table
  if ( m_pView->activeTable()->tableName() !=  m_tableName )
  {
    table = m_pView->doc()->map()->findTable( m_tableName );
    if ( table )
      m_pView->setActiveTable( table );
  }
  else
    table = m_pView->activeTable();

  // Revert the marker to its original position
  m_pView->selectionInfo()->setSelection( m_marker, m_anchor, table );
}


void KSpreadGoalSeekDlg::startCalc(double _start, double _goal)
{
  m_resultText->setText( i18n( "Starting..." ) );
  m_newValueDesc->setText( i18n( "Iteration:" ) );

  // lets be optimistic
  bool ok = true;

  // TODO: make this configurable
  double eps = 0.0000001;

  double startA = 0.0, startB;
  double resultA, resultB;

  // save old value
  m_oldSource = m_sourceCell->value().asFloat();
  resultA = _goal;

  // initialize start value
  startB = _start;
  double x = startB + 0.5;

  // while the result is not close enough to zero
  // or while the max number of iterations is not reached...
  while ( fabs( resultA ) > eps && ( m_maxIter >= 0 ) )
  {
    startA = startB;
    startB = x;

    m_sourceCell->setValue(startA);
    //    m_sourceCell->updateDepending();
    m_sourceCell->setCalcDirtyFlag();
    m_targetCell->calc( false );
    resultA = m_targetCell->value().asFloat() - _goal;
    //    kdDebug() << "Target A: " << m_targetCell->value().asFloat() << ", " << m_targetCell->text() << " Calc: " << resultA << endl;

    m_sourceCell->setValue(startB);
    //    m_sourceCell->updateDepending();
    m_sourceCell->setCalcDirtyFlag();
    m_targetCell->calc( false );
    resultB = m_targetCell->value().asFloat() - _goal;
    /*
      kdDebug() << "Target B: " << m_targetCell->value().asFloat() << ", " << m_targetCell->text() << " Calc: " << resultB << endl;

      kdDebug() << "Iteration: " << m_maxIter << ", StartA: " << startA
              << ", ResultA: " << resultA << " (eps: " << eps << "), StartB: "
              << startB << ", ResultB: " << resultB << endl;
    */

    // find zero with secant method (rough implementation was provided by Franz-Xaver Meier):
    // if the function returns the same for two different
    // values we have something like a horizontal line
    // => can't get zero.
    if ( resultB == resultA )
    {
      //      kdDebug() << " resultA == resultB" << endl;
      if ( fabs( resultA ) < eps )
      {
        ok = true;
        break;
      }

      ok = false;
      break;
    }

    // Point of intersection of secant with x-axis
    x = ( startA * resultB - startB * resultA ) / ( resultB - resultA );

    if ( fabs(x) > 100000000 )
    {
      //      kdDebug() << "fabs(x) > 100000000: " << x << endl;
      ok = false;
      break;
    }

    //    kdDebug() << "X: " << x << ", fabs (resultA): " << fabs(resultA) << ", Real start: " << startA << ", Real result: " << resultA << ", Iteration: " << m_maxIter << endl;

    --m_maxIter;
    if ( m_maxIter % 20 == 0 )
      m_newValue->setText( QString::number(m_maxIter) );
  }

  m_newValueDesc->setText( i18n( "New value:" ) );
  if ( ok )
  {
    m_sourceCell->setValue( startA );
    m_sourceCell->setCalcDirtyFlag();
    m_sourceCell->sheet()->setRegionPaintDirty(m_sourceCell->cellRect());
    //    m_targetCell->setCalcDirtyFlag();
    m_targetCell->calc( false );

    m_resultText->setText( i18n( "Goal seeking with cell %1 found a solution:" ).arg( m_sourceEdit->text() ) );
    m_newValue->setText( QString::number( startA ) );
    m_currentValue->setText( QString::number( m_oldSource ) );
    m_restored = false;
  }
  else
  {
    // restore the old value
    m_sourceCell->setValue( m_oldSource );
    m_targetCell->setCalcDirtyFlag();
    m_sourceCell->sheet()->setRegionPaintDirty(m_sourceCell->cellRect());
    m_targetCell->calc( false );
    m_resultText->setText( i18n( "Goal seeking with cell %1 has found NO solution." ).arg( m_sourceEdit->text() ) );
    m_newValue->setText( "" );
    m_currentValue->setText( QString::number( m_oldSource ) );
    m_restored = true;
  }

  m_buttonOk->setEnabled( true );
  m_buttonCancel->setEnabled( true );
  m_maxIter = 0;
}

#include "kspread_dlg_goalseek.moc"

