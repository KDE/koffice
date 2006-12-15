/* This file is part of the KDE project
   Copyright (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2002-2003 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 Laurent Montel <montel@kde.org>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2002 Ariya Hidayat <ariya@kde.org>
             (C) 2002 Werner Trobin <trobin@kde.org>
             (C) 2002 Harri Porten <porten@kde.org>

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

#include "GoalSeekDialog.h"

#include "Canvas.h"
#include "Cell.h"
#include "Doc.h"
#include "Editors.h"
#include "Map.h"
#include "Selection.h"
#include "Sheet.h"
#include "Util.h"
#include "View.h"
#include "DataManipulators.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <ktextedit.h>
#include <kpushbutton.h>

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QToolTip>
#include <QVariant>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>
#include <QCloseEvent>

#include <math.h>

using namespace KSpread;

GoalSeekDialog::GoalSeekDialog( View * parent,  QPoint const & marker,
                                        const char * name, bool, Qt::WFlags fl )
  : KDialog( parent, fl ),
    m_pView( parent ),
    m_maxIter( 1000 ),
    m_restored( true ),
    m_anchor( m_pView->selection()->anchor() ),
    m_marker( m_pView->selection()->marker() ),
    m_selection( m_pView->selection()->selection() )
{
  Q_UNUSED(marker)
  //setWFlags( Qt::WDestructiveClose );
  setButtons( 0 );
  setModal( false );

  if ( !name )
    setObjectName( "GoalSeekDialog" );

  resize( 458, 153 );
  setWindowTitle( i18n( "Goal Seek" ) );
  setSizeGripEnabled( true );
  setModal( false );

  QWidget* mainWidget = new QWidget( this );
  setMainWidget( mainWidget );

  GoalSeekDialogLayout = new QGridLayout( mainWidget );
  GoalSeekDialogLayout->setMargin(KDialog::marginHint());
  GoalSeekDialogLayout->setSpacing(KDialog::spacingHint());

  m_startFrame = new QFrame( this );
  m_startFrame->setFrameShape( QFrame::StyledPanel );
  m_startFrame->setFrameShadow( QFrame::Raised );
  m_startFrameLayout = new QGridLayout( m_startFrame );
  m_startFrameLayout->setMargin(KDialog::marginHint());
  m_startFrameLayout->setSpacing(KDialog::spacingHint());

  QLabel* label1 = new QLabel( i18n( "Set cell:" ), m_startFrame );
  m_startFrameLayout->addWidget( label1, 0, 0 );

  m_selector1 = new RegionSelector( m_startFrame );
  m_selector1->setView( m_pView );
  m_selector1->setDialog( this );
  m_selector1->setSelectionMode( RegionSelector::SingleCell );
  m_startFrameLayout->addWidget( m_selector1, 0, 1 );

  QLabel* label2 = new QLabel( i18n( "To value:" ), m_startFrame );
  m_startFrameLayout->addWidget( label2, 1, 0 );

  m_selector2 = new RegionSelector( m_startFrame );
  m_selector2->setView( m_pView );
  m_selector2->setDialog( this );
  m_selector2->setSelectionMode( RegionSelector::SingleCell );
  m_startFrameLayout->addWidget( m_selector2, 1, 1 );

  QLabel* label3 = new QLabel( i18n( "By changing cell:" ), m_startFrame );
  m_startFrameLayout->addWidget( label3, 2, 0 );

  m_selector3 = new RegionSelector( m_startFrame );
  m_selector3->setView( m_pView );
  m_selector3->setDialog( this );
  m_selector3->setSelectionMode( RegionSelector::SingleCell );
  m_startFrameLayout->addWidget( m_selector3, 2, 1 );

  GoalSeekDialogLayout->addWidget( m_startFrame, 0, 0 );

  QVBoxLayout * Layout5 = new QVBoxLayout();
  Layout5->setMargin(0);
  Layout5->setSpacing(6);

  m_buttonOk = new QPushButton( this );
  m_buttonOk->setText( i18n( "&Start" ) );
  m_buttonOk->setAutoDefault( true );
  m_buttonOk->setDefault( true );
  Layout5->addWidget( m_buttonOk );

  m_buttonCancel = new KPushButton( KStdGuiItem::cancel(), this );
  m_buttonCancel->setAutoDefault( true );
  Layout5->addWidget( m_buttonCancel );
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout5->addItem( spacer );

  GoalSeekDialogLayout->addLayout( Layout5, 0, 1 );

  m_resultFrame = new QFrame( this );
  m_resultFrame->setFrameShape( QFrame::StyledPanel );
  m_resultFrame->setFrameShadow( QFrame::Raised );
  m_resultFrame->setMinimumWidth( 350 );
  m_resultFrameLayout = new QGridLayout( m_resultFrame );
  m_resultFrameLayout->setMargin(KDialog::marginHint());
  m_resultFrameLayout->setSpacing(KDialog::spacingHint());

  m_currentValueLabel = new QLabel( m_resultFrame );
  m_currentValueLabel->setText( i18n( "Current value:" ) );

  m_resultFrameLayout->addWidget( m_currentValueLabel, 2, 0 );

  m_newValueDesc = new QLabel( m_resultFrame );
  m_newValueDesc->setText( i18n( "New value:" ) );

  m_resultFrameLayout->addWidget( m_newValueDesc, 1, 0 );

  m_newValue = new QLabel( m_resultFrame );
  m_newValue->setText( "m_targetValueEdit" );

  m_resultFrameLayout->addWidget( m_newValue, 1, 1 );

  m_currentValue = new QLabel( m_resultFrame );
  m_currentValue->setText( "m_currentValue" );

  m_resultFrameLayout->addWidget( m_currentValue, 2, 1 );

  m_resultText = new QLabel( m_resultFrame );
  m_resultText->setText( "Goal seeking with cell <cell> found <a | no> solution:" );
  m_resultText->setAlignment( Qt::AlignVCenter );
  m_resultText->setWordWrap( true );

  m_resultFrameLayout->addWidget( m_resultText, 0, 0, 0, 1 );

  //  GoalSeekDialogLayout->addWidget( m_resultFrame, 1, 0 );

  m_resultFrame->hide();

  m_sheetName = m_pView->activeSheet()->sheetName();

  // Allow the user to select cells on the spreadsheet.
//   m_pView->canvasWidget()->startChoose();

  qApp->installEventFilter( this );

  // signals and slots connections
  connect( m_buttonOk, SIGNAL( clicked() ), this, SLOT( buttonOkClicked() ) );
  connect( m_buttonCancel, SIGNAL( clicked() ), this, SLOT( buttonCancelClicked() ) );

  connect( m_pView->choice(), SIGNAL(changed(const Region&)),
           this, SLOT(slotSelectionChanged()));

  // tab order
  setTabOrder( m_selector1,  m_selector2 );
  setTabOrder( m_selector2,  m_selector3 );
  setTabOrder( m_selector3,  m_buttonOk );
  setTabOrder( m_buttonOk, m_buttonCancel );
}

GoalSeekDialog::~GoalSeekDialog()
{
  kDebug() << "~GoalSeekDialog" << endl;

  chooseCleanup();
  if ( !m_restored )
  {
    m_pView->doc()->emitBeginOperation( false );
    m_sourceCell->setValue(Value(m_oldSource));
    m_pView->slotUpdateView( m_pView->activeSheet() );
  }
}

void GoalSeekDialog::closeEvent ( QCloseEvent * e )
{
  e->accept();
  deleteLater();
}

void GoalSeekDialog::buttonOkClicked()
{
  Doc * pDoc = m_pView->doc();
  pDoc->emitBeginOperation( false );
  if (m_maxIter > 0)
  {
    Sheet * sheet = m_pView->activeSheet();

    Point source( m_selector3->textEdit()->toPlainText(), sheet->workbook(), sheet );
    if (!source.isValid())
    {
      KMessageBox::error( this, i18n("Cell reference is invalid.") );
      m_selector3->textEdit()->selectAll();
      m_selector3->textEdit()->setFocus();

      m_pView->slotUpdateView( m_pView->activeSheet() );
      return;
    }

    Point target( m_selector1->textEdit()->toPlainText(), sheet->workbook(), sheet );
    if (!target.isValid())
    {
      KMessageBox::error( this, i18n("Cell reference is invalid.") );
      m_selector1->textEdit()->selectAll();
      m_selector1->textEdit()->setFocus();

      m_pView->slotUpdateView( m_pView->activeSheet() );
      return;
    }

    bool ok = false;
    double goal = m_pView->doc()->locale()->readNumber(m_selector2->textEdit()->toPlainText(), &ok );
    if ( !ok )
    {
      KMessageBox::error( this, i18n("Target value is invalid.") );
      m_selector2->textEdit()->selectAll();
      m_selector2->textEdit()->setFocus();

      m_pView->slotUpdateView( m_pView->activeSheet() );
      return;
    }

    m_sourceCell = source.cell();
    m_targetCell = target.cell();

    if ( !m_sourceCell->value().isNumber() )
    {
      KMessageBox::error( this, i18n("Source cell must contain a numeric value.") );
      m_selector3->textEdit()->selectAll();
      m_selector3->textEdit()->setFocus();

      m_pView->slotUpdateView( m_pView->activeSheet() );
      return;
    }

    if ( !m_targetCell->isFormula() )
    {
      KMessageBox::error( this, i18n("Target cell must contain a formula.") );
      m_selector1->textEdit()->selectAll();
      m_selector1->textEdit()->setFocus();

      m_pView->slotUpdateView( m_pView->activeSheet() );
      return;
    }

    m_buttonOk->setText( i18n("&OK") );
    m_buttonOk->setEnabled(false);
    m_buttonCancel->setEnabled(false);
    GoalSeekDialogLayout->addWidget( m_resultFrame, 0, 0 );
    m_startFrame->hide();
    m_resultFrame->show();
    if ( m_startFrame->width() > 350 )
      m_resultFrame->setMinimumWidth( m_startFrame->width() );

    m_restored = false;

    startCalc( m_sourceCell->value().asFloat(), goal );
    m_pView->slotUpdateView( m_pView->activeSheet() );

    return;
  }
  else
  {
    m_restored = true;
  }

  m_pView->slotUpdateView( m_pView->activeSheet() );
  accept();
  deleteLater();
}

void GoalSeekDialog::buttonCancelClicked()
{
  if ( !m_restored )
  {
    m_pView->doc()->emitBeginOperation( false );
    m_sourceCell->setValue(Value(m_oldSource));
    m_restored = true;
    m_pView->slotUpdateView( m_pView->activeSheet() );
  }

  reject();
  deleteLater();
}

void GoalSeekDialog::chooseCleanup()
{
//   m_pView->canvasWidget()->endChoose();

  Sheet * sheet = 0;

  // Switch back to the old sheet
  if ( m_pView->activeSheet()->sheetName() !=  m_sheetName )
  {
    sheet = m_pView->doc()->map()->findSheet( m_sheetName );
    if ( sheet )
      m_pView->setActiveSheet( sheet );
  }
  else
    sheet = m_pView->activeSheet();

  // Revert the marker to its original position
  m_pView->selection()->initialize(QRect(m_marker, m_anchor));//, sheet );
}


void GoalSeekDialog::startCalc(double _start, double _goal)
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

    m_sourceCell->setValue(Value(startA));
    resultA = m_targetCell->value().asFloat() - _goal;
    //    kDebug() << "Target A: " << m_targetCell->value().asFloat() << ", " << m_targetCell->text() << " Calc: " << resultA << endl;

    m_sourceCell->setValue(Value(startB));
    resultB = m_targetCell->value().asFloat() - _goal;
    /*
      kDebug() << "Target B: " << m_targetCell->value().asFloat() << ", " << m_targetCell->text() << " Calc: " << resultB << endl;

      kDebug() << "Iteration: " << m_maxIter << ", StartA: " << startA
              << ", ResultA: " << resultA << " (eps: " << eps << "), StartB: "
              << startB << ", ResultB: " << resultB << endl;
    */

    // find zero with secant method (rough implementation was provided by Franz-Xaver Meier):
    // if the function returns the same for two different
    // values we have something like a horizontal line
    // => can't get zero.
    if ( resultB == resultA )
    {
      //      kDebug() << " resultA == resultB" << endl;
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
      //      kDebug() << "fabs(x) > 100000000: " << x << endl;
      ok = false;
      break;
    }

    //    kDebug() << "X: " << x << ", fabs (resultA): " << fabs(resultA) << ", Real start: " << startA << ", Real result: " << resultA << ", Iteration: " << m_maxIter << endl;

    --m_maxIter;
    if ( m_maxIter % 20 == 0 )
      m_newValue->setText( QString::number(m_maxIter) );
  }

  m_newValueDesc->setText( i18n( "New value:" ) );
  if ( ok )
  {
    m_sourceCell->setValue( Value(startA ) );
    m_sourceCell->sheet()->setRegionPaintDirty(Region(m_sourceCell->cellPosition()));

    m_resultText->setText( i18n( "Goal seeking with cell %1 found a solution:",
                                 m_selector3->textEdit()->toPlainText() ) );
    m_newValue->setText( m_pView->doc()->locale()->formatNumber( startA ) );
    m_currentValue->setText( m_pView->doc()->locale()->formatNumber( m_oldSource ) );
    m_restored = false;
  }
  else
  {
    // restore the old value
    m_sourceCell->setValue( Value( m_oldSource ) );
    m_sourceCell->sheet()->setRegionPaintDirty(Region(m_sourceCell->cellPosition()));
    m_resultText->setText( i18n( "Goal seeking with cell %1 has found NO solution.",
                                 m_selector3->textEdit()->toPlainText() ) );
    m_newValue->setText( "" );
    m_currentValue->setText( m_pView->doc()->locale()->formatNumber( m_oldSource ) );
    m_restored = true;
  }

  m_buttonOk->setEnabled( true );
  m_buttonCancel->setEnabled( true );
  m_maxIter = 0;
}

#include "GoalSeekDialog.moc"

