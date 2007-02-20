/* This file is part of the KDE project
   Copyright (C) 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
             (C) 2002-2004 Ariya Hidayat <ariya@kde.org>
             (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2000-2005 Laurent Montel <montel@kde.org>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2001-2002 David Faure <faure@kde.org>
             (C) 2001 Werner Trobin <trobin@kde.org>
             (C) 2000 Bernd Johannes Wuebben <wuebben@kde.org>

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

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollBar>

#include <kconfig.h>
#include <kicon.h>
#include <kstatusbar.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kdeversion.h>
#include <kcolorbutton.h>

#include <KoTabBar.h>

#include "Sheet.h"
#include "SheetPrint.h"
#include "Doc.h"
#include "Canvas.h"
#include "Border.h"
#include "View.h"
#include "Localization.h"
#include "Editors.h"

#include "PreferenceDialog.h"

using namespace KSpread;

PreferenceDialog::PreferenceDialog( View* parent, const char* /*name*/)
  : KPageDialog( )

{
  setFaceType( List );
  setCaption( i18n("Configure KSpread") );
  setButtons( Ok|Cancel|Default );
  setDefaultButton( Ok );

  m_pView=parent;

  connect(this, SIGNAL(okClicked()),this,SLOT(slotApply()));
  connect(this, SIGNAL(defaultClicked()),this,SLOT(slotDefault()));
  KVBox *page2 = new KVBox();
  p2 = addPage(page2, i18n("Locale Settings"));
  p2->setIcon( KIcon( BarIcon("gohome",K3Icon::SizeMedium) ) );
 _localePage=new parameterLocale(parent,page2 );

  KVBox *page3 = new KVBox();
  p3 = addPage(page3, i18n("Interface"));
  p3->setIcon( KIcon( BarIcon("signature",K3Icon::SizeMedium) ) );
  _configure = new  configure(parent,page3 );

  KVBox *page4 = new KVBox();
  p4 = addPage(page4, i18n("Misc"));
  p4->setIcon( KIcon( BarIcon("misc",K3Icon::SizeMedium) ) );
  _miscParameter = new  miscParameters(parent,page4 );

  KVBox *page5 = new KVBox();
  p5 = addPage(page5, i18n("Color"));
  p5->setIcon( KIcon( BarIcon("colorize",K3Icon::SizeMedium) ) );
  _colorParameter=new colorParameters(parent,page5 );

  KVBox *page6 = new KVBox();
  p6 = addPage(page6, i18n("Page Layout"));
  p6->setIcon( KIcon( BarIcon("edit",K3Icon::SizeMedium) ) );
  _layoutPage=new configureLayoutPage(parent,page6 );

  KVBox *page7 = new KVBox();
  p7 = addPage(page7,  i18n("Spelling") );
  p7->setIcon( KIcon( BarIcon("spellcheck",K3Icon::SizeMedium) ) );
  p7->setHeader( i18n("Spell Checker Behavior") );
  _spellPage=new configureSpellPage(parent,page7);

}

void PreferenceDialog::openPage(int flags)
{
    if(flags & KS_LOCALE)
        setCurrentPage( p2 );
    else if(flags & KS_INTERFACE)
        setCurrentPage( p3 );
    else if(flags & KS_MISC)
        setCurrentPage( p4 );
    else if(flags & KS_COLOR)
        setCurrentPage( p5 );
    else if(flags & KS_LAYOUT)
        setCurrentPage( p6 );
    else if(flags & KS_SPELLING)
        setCurrentPage( p7 );
}

void PreferenceDialog::slotApply()
{
  m_pView->doc()->emitBeginOperation( false );
  _configure->apply();
  _miscParameter->apply();
  _colorParameter->apply();
  _layoutPage->apply();
  _spellPage->apply();
  _localePage->apply();
  m_pView->doc()->refreshInterface();
  m_pView->slotUpdateView( m_pView->activeSheet() );
}

void PreferenceDialog::slotDefault()
{
    if ( currentPage() == p2 )
      _configure->slotDefault();
    else if ( currentPage() == p3 )
      _miscParameter->slotDefault();
    else if ( currentPage() == p4 )
      _colorParameter->slotDefault();
    else if ( currentPage() == p5 )
      _layoutPage->slotDefault();
    else if ( currentPage() == p6 )
      _spellPage->slotDefault();
}


parameterLocale::parameterLocale( View* _view, KVBox *box , char * /*name*/ )
 :QObject ( box->parent() )
{
    m_pView = _view;
    m_bUpdateLocale=false;
//   QGroupBox* tmpQGroupBox = new QGroupBox( i18n("Settings"), box );
  KVBox* tmpQGroupBox = box;

  KLocale* locale=_view->doc()->locale();

  m_language=new QLabel( tmpQGroupBox );
  m_number=new QLabel( tmpQGroupBox );
  m_date=new QLabel( tmpQGroupBox );
  m_shortDate=new QLabel( tmpQGroupBox );
  m_time=new QLabel( tmpQGroupBox );
  m_money=new QLabel( tmpQGroupBox );

  updateToMatchLocale(locale);

  m_updateButton=new QPushButton ( i18n("&Update Locale Settings"), tmpQGroupBox);
  connect(m_updateButton, SIGNAL(clicked()),this,SLOT(updateDefaultSystemConfig()));

  box->layout()->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}

void parameterLocale::apply()
{
    if (m_bUpdateLocale)
    {
        m_pView->doc()->emitBeginOperation( false );
        m_pView->doc()->refreshLocale();
        m_pView->slotUpdateView( m_pView->activeSheet() );
    }
}

void parameterLocale::updateDefaultSystemConfig()
{
    m_bUpdateLocale=true;
    static_cast<Localization*>(m_pView->doc()->locale())->defaultSystemConfig( );
    KLocale* locale=m_pView->doc()->locale();
    updateToMatchLocale( locale );
}

void parameterLocale::updateToMatchLocale(KLocale* locale)
{
  m_language->setText( i18n("Language: %1", locale->language() ));
  m_number->setText( i18n("Default number format: %1", locale->formatNumber(12.55) ));
  m_date->setText( i18n("Long date format: %1", locale->formatDate( QDate::currentDate() )));
  m_shortDate->setText( i18n("Short date format: %1", locale->formatDate( QDate::currentDate() ,true) ));
  m_time->setText( i18n("Time format: %1", locale->formatTime( QTime::currentTime() ) ));
  m_money->setText( i18n("Currency format: %1", locale->formatMoney(12.55) ));
}

configure::configure( View* _view, KVBox *box , char * /*name*/ )
 :QObject ( box->parent() )
 {
  m_pView = _view;

  bool vertical=true;
  bool horizontal=true;
  bool rowHeader=true;
  bool colHeader=true;
  bool tabbar=true;
  bool formulaBar=true;
  bool statusBar=true;
  m_oldBackupFile = true;

//   QGroupBox* tmpQGroupBox = new QGroupBox( i18n("Settings"), box );
  KVBox* tmpQGroupBox = box;

  config = Factory::global().config();
  int _page=1;

  oldRecent=10;
  oldAutoSaveValue=KoDocument::defaultAutoSave()/60;

    const KConfigGroup parameterGroup = config->group( "Parameters" );
    _page = parameterGroup.readEntry( "NbPage" ,1) ;
    horizontal = parameterGroup.readEntry("Horiz ScrollBar",true);
    vertical = parameterGroup.readEntry("Vert ScrollBar",true);
    colHeader = parameterGroup.readEntry("Column Header",true);
    rowHeader = parameterGroup.readEntry("Row Header",true);
    tabbar = parameterGroup.readEntry("Tabbar",true);
    formulaBar = parameterGroup.readEntry("Formula bar",true);
    statusBar = parameterGroup.readEntry("Status bar",true);
    oldRecent = parameterGroup.readEntry( "NbRecentFile" ,10);
    oldAutoSaveValue = parameterGroup.readEntry("AutoSave",KoDocument::defaultAutoSave()/60);
    m_oldBackupFile = parameterGroup.readEntry("BackupFile",m_oldBackupFile);

  nbPage=new KIntNumInput(_page, tmpQGroupBox , 10);
  nbPage->setRange(1, 10, 1);
  nbPage->setLabel(i18n("Number of sheets open at the &beginning:"));
  nbPage->setWhatsThis( i18n( "Controls how many worksheets will be created if the option Start with an empty document is chosen when KSpread is started." ) );

  nbRecentFile=new KIntNumInput(oldRecent, tmpQGroupBox , 10);
  nbRecentFile->setRange(1, 20, 1);
  nbRecentFile->setLabel(i18n("&Number of files to show in Recent Files list:"));
  nbRecentFile->setWhatsThis( i18n( "Controls the maximum number of filenames that are shown when you select File-> Open Recent." ) );

  autoSaveDelay=new KIntNumInput(oldAutoSaveValue, tmpQGroupBox , 10);
  autoSaveDelay->setRange(0, 60, 1);
  autoSaveDelay->setLabel(i18n("Au&tosave delay (minutes):"));
  autoSaveDelay->setSpecialValueText(i18n("Do not save automatically"));
  autoSaveDelay->setSuffix(i18n("min"));
  autoSaveDelay->setWhatsThis( i18n( "Here you can select the time between autosaves, or disable this feature altogether by choosing Do not save automatically (drag the slider to the far left)." ) );

  m_createBackupFile = new QCheckBox( i18n("Create backup files"), tmpQGroupBox );
  m_createBackupFile->setChecked( m_oldBackupFile );
  m_createBackupFile->setWhatsThis( i18n( "Check this box if you want some backup files created. This is checked per default." ) );

  showVScrollBar=new QCheckBox(i18n("Show &vertical scrollbar"),tmpQGroupBox);
  showVScrollBar->setChecked(vertical);
  showVScrollBar->setWhatsThis( i18n( "Check or uncheck this box to show or hide the vertical scrollbar in all sheets." ) );

  showHScrollBar=new QCheckBox(i18n("Show &horizontal scrollbar"),tmpQGroupBox);
  showHScrollBar->setChecked(horizontal);
  showHScrollBar->setWhatsThis( i18n( "Check or uncheck this box to show or hide the horizontal scrollbar in all sheets." ) );

  showColHeader=new QCheckBox(i18n("Show c&olumn header"),tmpQGroupBox);
  showColHeader->setChecked(colHeader);
  showColHeader->setWhatsThis( i18n( "Check this box to show the column letters across the top of each worksheet." ) );

  showRowHeader=new QCheckBox(i18n("Show &row header"),tmpQGroupBox);
  showRowHeader->setChecked(rowHeader);
  showRowHeader->setWhatsThis( i18n( "Check this box to show the row numbers down the left side." ) );

  showTabBar =new QCheckBox(i18n("Show ta&bs"),tmpQGroupBox);
  showTabBar->setChecked(tabbar);
  showTabBar->setWhatsThis( i18n( "This check box controls whether the sheet tabs are shown at the bottom of the worksheet." ) );

  showFormulaBar =new QCheckBox(i18n("Sho&w formula toolbar"),tmpQGroupBox);
  showFormulaBar->setChecked(formulaBar);
  showFormulaBar->setWhatsThis( i18n( "Here is where you can choose to show or hide the Formula bar." ) );

  showStatusBar =new QCheckBox(i18n("Show stat&us bar"),tmpQGroupBox);
  showStatusBar->setChecked(statusBar);
  showStatusBar->setWhatsThis( i18n( "Uncheck this box if you want to hide the status bar." ) );

  box->layout()->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}


void configure::slotDefault()
{
  showHScrollBar->setChecked(true);
  showRowHeader->setChecked(true);
  showVScrollBar->setChecked(true);
  showColHeader->setChecked(true);
  showTabBar->setChecked(true);
  showFormulaBar->setChecked(true);
  showStatusBar->setChecked(true);
  nbPage->setValue(1);
  nbRecentFile->setValue(10);
  autoSaveDelay->setValue(KoDocument::defaultAutoSave()/60);
  m_createBackupFile->setChecked( true );
}


void configure::apply()
{
    m_pView->doc()->emitBeginOperation( false );
    KConfigGroup parameterGroup = config->group( "Parameters" );
    parameterGroup.writeEntry( "NbPage", nbPage->value());
    Doc *doc =m_pView->doc();
    bool active=true;
    active=showHScrollBar->isChecked();
    if( m_pView->horzScrollBar()->isVisible()!=active)
    {
        parameterGroup.writeEntry( "Horiz ScrollBar",active);
        if( active)
            m_pView->horzScrollBar()->show();
        else
            m_pView->horzScrollBar()->hide();
        doc->setShowHorizontalScrollBar(active);
    }
    active=showVScrollBar->isChecked();
    if( m_pView->vertScrollBar()->isVisible()!=active)
    {
        parameterGroup.writeEntry( "Vert ScrollBar", active);
        if(active)
            m_pView->vertScrollBar()->show();
        else
            m_pView->vertScrollBar()->hide();
        doc->setShowVerticalScrollBar(active);

    }
    active=showColHeader->isChecked();
    if( m_pView->hBorderWidget()->isVisible()!=active)
    {
        parameterGroup.writeEntry( "Column Header", active);
        if( active)
            m_pView->hBorderWidget()->show();
        else
            m_pView->hBorderWidget()->hide();
        doc->setShowColumnHeader(active);
    }

    active=showRowHeader->isChecked();
    if( m_pView->vBorderWidget()->isVisible()!=active)
    {
        parameterGroup.writeEntry( "Row Header", active);
        if( active)
            m_pView->vBorderWidget()->show();
        else
            m_pView->vBorderWidget()->hide();
        doc->setShowRowHeader(active);
    }

    active=showTabBar->isChecked();
    if(m_pView->tabBar()->isVisible()!=active)
    {
        parameterGroup.writeEntry( "Tabbar", active);
        if(active)
            m_pView->tabBar()->show();
        else
            m_pView->tabBar()->hide();
        doc->setShowTabBar(active);
    }

    active=showFormulaBar->isChecked();
    if(m_pView->posWidget()->isVisible()!=active)
    {
        parameterGroup.writeEntry( "Formula bar",active);
        m_pView->editWidget()->showEditWidget(active);
        if(active)
            m_pView->posWidget()->show();
        else
            m_pView->posWidget()->hide();
        doc->setShowFormulaBar(active);
    }

    active=showStatusBar->isChecked();
    parameterGroup.writeEntry( "Status bar",active);
    m_pView->showStatusBar( active );

    int val=nbRecentFile->value();
    if( oldRecent!= val)
    {
       parameterGroup.writeEntry( "NbRecentFile",val);
       m_pView->changeNbOfRecentFiles(val);
    }
    val=autoSaveDelay->value();
    if(val!=oldAutoSaveValue)
    {
        parameterGroup.writeEntry( "AutoSave", val );
        doc->setAutoSave(val*60);
    }

    bool state =m_createBackupFile->isChecked();
    if(state!=m_oldBackupFile)
    {
        parameterGroup.writeEntry( "BackupFile", state );
        doc->setBackupFile( state);
        m_oldBackupFile=state;
    }

    m_pView->slotUpdateView( m_pView->activeSheet() );
}


miscParameters::miscParameters( View* _view,KVBox *box, char * /*name*/ )
 :QObject ( box->parent() )
 {
  m_pView = _view;


//   QGroupBox* tmpQGroupBox = new QGroupBox( i18n("Misc"), box );
  KVBox* tmpQGroupBox = box;

  config = Factory::global().config();
  indentUnit = _view->doc()->unit();
    const KConfigGroup parameterGroup = config->group( "Parameters" );
    double _indent = parameterGroup.readEntry( "Indent" , KoUnit::toUserValue( 10.0, indentUnit ) );
    bool bMsgError = parameterGroup.readEntry( "Msg error" ,false );

    m_oldNbRedo = config->group( "Misc" ).readEntry( "UndoRedo", m_oldNbRedo );

  m_undoRedoLimit=new KIntNumInput( m_oldNbRedo, tmpQGroupBox );
  m_undoRedoLimit->setLabel(i18n("Undo/redo limit:"));
  m_undoRedoLimit->setRange(10, 60, 1);


  QLabel *label=new QLabel(i18n("&Completion mode:"), tmpQGroupBox);

  typeCompletion=new QComboBox(tmpQGroupBox);
  label->setBuddy(typeCompletion);
  typeCompletion->setWhatsThis( i18n( "Lets you choose the (auto) text completion mode from a range of options in the drop down selection box." ) );
  QStringList listType;
  listType+=i18n("None");
  listType+=i18n("Manual");
  listType+=i18n("Popup");
  listType+=i18n("Automatic");
  listType+=i18n("Semi-Automatic");
  typeCompletion->insertItems( 0,listType);
  typeCompletion->setCurrentIndex(0);
  comboChanged=false;
  connect(typeCompletion,SIGNAL(activated( const QString & )),this,SLOT(slotTextComboChanged(const QString &)));

  label=new QLabel(i18n("&Pressing enter moves cell cursor:"), tmpQGroupBox);
  typeOfMove=new QComboBox( tmpQGroupBox);
  label->setBuddy(typeOfMove);
  listType.clear();
  listType+=i18n("Down");
  listType+=i18n("Up");
  listType+=i18n("Right");
  listType+=i18n("Left");
  listType+=i18n("Down, First Column");
  typeOfMove->insertItems( 0,listType);
  typeOfMove->setCurrentIndex(0);
  typeOfMove->setWhatsThis( i18n( "When you have selected a cell, pressing the Enter key will move the cell cursor one cell left, right, up or down, as determined by this setting." ) );

  label=new QLabel(i18n("&Method of calc:"), tmpQGroupBox);

  typeCalc=new QComboBox( tmpQGroupBox);
  label->setBuddy(typeCalc);
  QStringList listTypeCalc;
  listTypeCalc+=i18n("Sum");
  listTypeCalc+=i18n("Min");
  listTypeCalc+=i18n("Max");
  listTypeCalc+=i18n("Average");
  listTypeCalc+=i18n("Count");
  listTypeCalc+=i18n("CountA");
  listTypeCalc+=i18n("None");
  typeCalc->insertItems( 0,listTypeCalc);
  typeCalc->setCurrentIndex(0);
  typeCalc->setWhatsThis( i18n( "This drop down selection box can be used to choose the calculation performed by the Statusbar Summary  function." ) );

//   valIndent = new KDoubleNumInput( _indent, tmpQGroupBox , 10.0 );
  valIndent = new KDoubleNumInput( tmpQGroupBox );
  valIndent->setRange( KoUnit::toUserValue( 0.0, indentUnit ),
                       KoUnit::toUserValue( 400.0, indentUnit ),
                       KoUnit::toUserValue( 10.0, indentUnit) );
//   valIndent->setRange( 0.0, 100.0, 10.0 );
  valIndent->setValue ( KoUnit::toUserValue( _indent, indentUnit ) );
  valIndent->setWhatsThis( i18n( "Lets you define the amount of indenting used by the Increase Indent and Decrease Indent option in the Format menu." ) );
  valIndent->setLabel(i18n("&Indentation step (%1):", KoUnit::unitName(indentUnit)));

  msgError= new QCheckBox(i18n("&Show error message for invalid formulae"),tmpQGroupBox);
  msgError->setChecked( bMsgError );
  msgError->setWhatsThis( i18n( "If this box is checked a message box will pop up when what you have entered into a cell cannot be understood by KSpread." ) );

  box->layout()->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

  initComboBox();
}

void miscParameters::slotTextComboChanged(const QString &)
{
  comboChanged=true;
}

void miscParameters::initComboBox()
{
    KGlobalSettings::Completion tmpCompletion = KGlobalSettings::CompletionAuto;
    tmpCompletion = ( KGlobalSettings::Completion )config->group( "Parameters" ).readEntry( "Completion Mode" ,int(KGlobalSettings::CompletionAuto)) ;
    config->group( "Parameters" ).writeEntry( "Completion Mode", (int)tmpCompletion);

switch(tmpCompletion )
        {
        case  KGlobalSettings::CompletionNone:
                typeCompletion->setCurrentIndex(0);
                break;
        case  KGlobalSettings::CompletionAuto:
                typeCompletion->setCurrentIndex(3);
                break;
        case  KGlobalSettings::CompletionMan:
                typeCompletion->setCurrentIndex(4);
                break;
        case  KGlobalSettings::CompletionShell:
                typeCompletion->setCurrentIndex(1);
                break;
        case  KGlobalSettings::CompletionPopup:
                typeCompletion->setCurrentIndex(2);
                break;
        default :
                typeCompletion->setCurrentIndex(0);
                break;
        }
        switch( m_pView->doc()->moveToValue( ))
        {
        case  Bottom:
                typeOfMove->setCurrentIndex(0);
                break;
        case  Left:
                typeOfMove->setCurrentIndex(3);
                break;
        case  Top:
                typeOfMove->setCurrentIndex(1);
                break;
        case  Right:
                typeOfMove->setCurrentIndex(2);
                break;
        case  BottomFirst:
                typeOfMove->setCurrentIndex(4);
                break;
        default :
                typeOfMove->setCurrentIndex(0);
                break;
        }

switch( m_pView->doc()->getTypeOfCalc())
        {
        case  SumOfNumber:
                typeCalc->setCurrentIndex(0);
                break;
        case  Min:
                typeCalc->setCurrentIndex(1);
                break;
        case  Max:
                typeCalc->setCurrentIndex(2);
                break;
        case  Average:
                typeCalc->setCurrentIndex(3);
                break;
        case  Count:
	        typeCalc->setCurrentIndex(4);
                break;
        case  CountA:
	        typeCalc->setCurrentIndex(5);
                break;
        case  NoneCalc:
	        typeCalc->setCurrentIndex(6);
                break;
        default :
                typeCalc->setCurrentIndex(0);
                break;
        }

}

void miscParameters::slotDefault()
{
  m_undoRedoLimit->setValue(30);
  valIndent->setValue( KoUnit::toUserValue( 10.0, indentUnit) );
  typeCompletion->setCurrentIndex(3);
  typeOfMove->setCurrentIndex(0);
  msgError->setChecked(false);
  typeCalc->setCurrentIndex(0);
}


void miscParameters::apply()
{
    kDebug() << "Applying misc preferences" << endl;

    int const newUndo=m_undoRedoLimit->value();
    if( newUndo!=m_oldNbRedo )
    {
        config->group( "Misc" ).writeEntry( "UndoRedo", newUndo );
        m_pView->doc()->setUndoRedoLimit(newUndo);
        m_oldNbRedo=newUndo;
    }

    KConfigGroup parameterGroup = config->group( "Parameters" );
    KGlobalSettings::Completion tmpCompletion=KGlobalSettings::CompletionNone;
    switch(typeCompletion->currentIndex())
    {
        case 0:
            tmpCompletion=KGlobalSettings::CompletionNone;
            break;
        case 1:
            tmpCompletion=KGlobalSettings::CompletionShell;
            break;
        case 2:
            tmpCompletion=KGlobalSettings::CompletionPopup;
            break;
        case 3:
            tmpCompletion=KGlobalSettings::CompletionAuto;
            break;
        case 4:
            tmpCompletion=KGlobalSettings::CompletionMan;
            break;
    }


    if(comboChanged)
    {
        m_pView->doc()->setCompletionMode(tmpCompletion);
        parameterGroup.writeEntry( "Completion Mode", (int)tmpCompletion);
    }

    KSpread::MoveTo tmpMoveTo=Bottom;
    switch(typeOfMove->currentIndex())
    {
        case 0:
            tmpMoveTo=Bottom;
            break;
        case 1:
            tmpMoveTo=Top;
            break;
        case 2:
            tmpMoveTo=Right;
            break;
        case 3:
            tmpMoveTo=Left;
            break;
        case 4:
            tmpMoveTo=BottomFirst;
            break;
    }
    if(tmpMoveTo!=m_pView->doc()->moveToValue())
    {
        m_pView->doc()->setMoveToValue(tmpMoveTo);
        parameterGroup.writeEntry( "Move", (int)tmpMoveTo);
    }

    MethodOfCalc tmpMethodCalc=SumOfNumber;
    switch(typeCalc->currentIndex())
    {
        case 0:
            tmpMethodCalc =SumOfNumber;
            break;
        case 1:
            tmpMethodCalc=Min;
            break;
        case 2:
            tmpMethodCalc=Max;
            break;
        case 3:
            tmpMethodCalc=Average;
            break;
        case 4:
            tmpMethodCalc=Count;
            break;
        case 5:
            tmpMethodCalc=CountA;
            break;
        case 6:
            tmpMethodCalc=NoneCalc;
            break;

    }
    if(tmpMethodCalc!=m_pView->doc()->getTypeOfCalc())
    {
        m_pView->doc()->setTypeOfCalc(tmpMethodCalc);
        parameterGroup.writeEntry( "Method of Calc", (int)tmpMethodCalc);
        m_pView->calcStatusBarOp();
        m_pView->initCalcMenu();
    }

    double val = valIndent->value();
    if( val != m_pView->doc()->indentValue() )
    {
        KoUnit oldUnit = m_pView->doc()->unit();
        m_pView->doc()->setUnit(indentUnit);
        m_pView->doc()->setIndentValue( val );
        m_pView->doc()->setUnit(oldUnit);
        parameterGroup.writeEntry( "Indent", KoUnit::fromUserValue( val, indentUnit ) );
    }

    bool active=msgError->isChecked();
    if(active!=m_pView->doc()->showMessageError())
    {
        m_pView->doc()->setShowMessageError( active);
        parameterGroup.writeEntry( "Msg error" ,(int)active);
    }
}



colorParameters::colorParameters( View* _view,KVBox *box , char * /*name*/ )
 :QObject ( box->parent() )
{
  m_pView = _view;
  config = Factory::global().config();

    QColor _gridColor = config->group( "KSpread Color" ).readEntry( "GridColor", Qt::lightGray );

//   QGroupBox* tmpQGroupBox = new QGroupBox( i18n("Color"), box );
  KVBox* tmpQGroupBox = box;

  QLabel *label = new QLabel(i18n("&Grid color:"), tmpQGroupBox );

  gridColor = new KColorButton( _gridColor,
                                Qt::lightGray,
                                tmpQGroupBox );
  gridColor->setWhatsThis( i18n( "Click here to change the grid color ie the color of the borders of each cell." ) );
  label->setBuddy(gridColor);

    QColor _pbColor = config->group( "KSpread Color" ).readEntry( "PageBorderColor", Qt::red );

  QLabel * label2 = new QLabel( i18n("&Page borders:"), tmpQGroupBox );

  pageBorderColor = new KColorButton( _pbColor,
                                Qt::red,
                                tmpQGroupBox );
  pageBorderColor->setWhatsThis( i18n( "When the View ->Show Page Borders menu item is checked, the page borders are displayed. Click here to choose another color for the borders than the default red." ) );

  label2->setBuddy(pageBorderColor);

  box->layout()->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding ) );
}

void colorParameters::apply()
{
  QColor _col = gridColor->color();
  if ( m_pView->doc()->gridColor() != _col )
  {
    m_pView->doc()->setGridColor( _col );
    config->group( "KSpread Color" ).writeEntry( "GridColor", _col );
  }

  QColor _pbColor = pageBorderColor->color();
  if ( m_pView->doc()->pageBorderColor() != _pbColor )
  {
    m_pView->doc()->changePageBorderColor( _pbColor );
    config->group( "KSpread Color" ).writeEntry( "PageBorderColor", _pbColor );
  }
}

void colorParameters::slotDefault()
{
  gridColor->setColor( Qt::lightGray );
  pageBorderColor->setColor( Qt::red );
}



configureLayoutPage::configureLayoutPage( View* _view,KVBox *box , char * /*name*/ )
 :QObject ( box->parent() )
{
  m_pView = _view;

//   QGroupBox* tmpQGroupBox = new QGroupBox( i18n("Default Parameters"), box );
//   tmpQGroupBox->layout()->setSpacing(KDialog::spacingHint());
//   tmpQGroupBox->layout()->setMargin(KDialog::marginHint());
  QWidget* tmpQGroupBox = new QWidget( box );

  QGridLayout *grid1 = new QGridLayout(tmpQGroupBox);
//   grid1->addItem(new QSpacerItem( 0, KDialog::marginHint() ), 0, 0 );
  grid1->setRowStretch( 7, 10 );

  config = Factory::global().config();

  QLabel *label=new QLabel(i18n("Default page &size:"), tmpQGroupBox);

  grid1->addWidget(label,0,0);

  defaultSizePage=new QComboBox( tmpQGroupBox);
  label->setBuddy(defaultSizePage);
  defaultSizePage->insertItems( 0, KoPageFormat::allFormats() );
  defaultSizePage->setCurrentIndex(1);
  defaultSizePage->setWhatsThis( i18n( "Choose the default page size for your worksheet among all the most common page sizes.\nNote that you can overwrite the page size for the current sheet using the Format -> Page Layout... dialog." ) );
  grid1->addWidget(defaultSizePage,1,0);

  label=new QLabel(i18n("Default page &orientation:"), tmpQGroupBox);
  grid1->addWidget(label,2,0);

  defaultOrientationPage=new QComboBox( tmpQGroupBox);
  label->setBuddy(defaultOrientationPage);

  QStringList listType;
  listType+=i18n( "Portrait" );
  listType+=i18n( "Landscape" );
  defaultOrientationPage->insertItems( 0,listType);
  defaultOrientationPage->setCurrentIndex(0);
  defaultOrientationPage->setWhatsThis( i18n( "Choose the sheet orientation: portrait or lanscape.\nNote that you can overwrite the orientation for the current sheet using the Format -> Page Layout... dialog." ) );
  grid1->addWidget(defaultOrientationPage,3,0);

  label=new QLabel(tmpQGroupBox);
  label->setText(i18n("Default page &unit:"));
  grid1->addWidget(label,4,0);
  defaultUnit=new QComboBox( tmpQGroupBox);
  label->setBuddy(defaultUnit);

  defaultUnit->insertItems( 0,KoUnit::listOfUnitName());
  defaultUnit->setCurrentIndex(0);
  defaultUnit->setWhatsThis( i18n( "Choose the default unit that will be used in your sheet.\nNote that you can overwrite the unit for the current sheet using the Format -> Page Layout... dialog." ) );
  grid1->addWidget(defaultUnit,5,0);

  box->layout()->addItem( new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

  initCombo();
}

void configureLayoutPage::slotDefault()
{
  defaultSizePage->setCurrentIndex(1);
  defaultOrientationPage->setCurrentIndex(0);
  defaultUnit->setCurrentIndex(0);
}

void configureLayoutPage::initCombo()
{
    const KConfigGroup pageLayoutGroup = config->group( "KSpread Page Layout" );
    paper = pageLayoutGroup.readEntry( "Default size page", 1 );
    orientation = pageLayoutGroup.readEntry( "Default orientation page", 0 );
    unit = pageLayoutGroup.readEntry( "Default unit page", 0 );

    defaultUnit->setCurrentIndex(m_pView->doc()->unit().indexInList());
    defaultSizePage->setCurrentIndex(paper);
    defaultOrientationPage->setCurrentIndex(orientation);
}


void configureLayoutPage::apply()
{
    m_pView->doc()->emitBeginOperation( false );
    KConfigGroup pageLayoutGroup = config->group( "KSpread Page Layout" );

  if( paper != defaultSizePage->currentIndex() )
  {
     unsigned int sizePage = defaultSizePage->currentIndex();
     pageLayoutGroup.writeEntry( "Default size page", sizePage );
     m_pView->activeSheet()->print()->setPaperFormat( (KoFormat)sizePage );
  }
  if( orientation != defaultOrientationPage->currentIndex() )
  {
     unsigned int orientationPage = defaultOrientationPage->currentIndex();
     pageLayoutGroup.writeEntry( "Default orientation page", orientationPage );
     m_pView->activeSheet()->print()->setPaperOrientation( (KoOrientation)orientationPage );
  }
  if( unit != defaultUnit->currentIndex() )
  {
     unsigned int unitPage = defaultUnit->currentIndex();
     pageLayoutGroup.writeEntry( "Default unit page", unitPage );
     m_pView->doc()->setUnit( KoUnit((KoUnit::Unit)unitPage) );
  }
  m_pView->slotUpdateView( m_pView->activeSheet() );
}

configureSpellPage::configureSpellPage( View* _view,KVBox *box , char * /*name*/ )
 :QObject ( box->parent() )
{
  m_pView = _view;

  config = Factory::global().config();


#ifdef __GNUC__
#warning TODO KDE4 port to sonnet
#endif
#if 0
  m_spellConfigWidget = new KSpellConfig( box, m_pView->doc()->getKSpellConfig()/*, false*/);
#endif
  dontCheckUpperWord = new QCheckBox( i18n("Skip all uppercase words"),box);
  dontCheckUpperWord->setWhatsThis( i18n( "If checked, the words written in uppercase letters are not spell checked. This might be useful if you have a lot of acronyms such as KDE for example." ) );
  dontCheckTitleCase = new QCheckBox( i18n("Do not check title case"),box);
  dontCheckTitleCase->setWhatsThis( i18n( "Check this box if you want the spellchecker to ignore the title case, for example My Own Spreadsheet or My own spreadsheet. If this is unchecked, the spell checker will ask for a uppercase letter in the title nouns." ) );

  QWidget* spacer = new QWidget( box );
  spacer->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    const KConfigGroup spellGroup = config->group( "KSpell kspread" );
    dontCheckUpperWord->setChecked(spellGroup.readEntry("KSpell_dont_check_upper_word",false));
    dontCheckTitleCase->setChecked(spellGroup.readEntry("KSpell_dont_check_title_case",false));
    //m_spellConfigWidget->addIgnoreList( m_pView->doc()->spellListIgnoreAll() );
}


void configureSpellPage::apply()
{
#ifdef __GNUC__
#warning TODO KDE4 port to sonnet
#endif
#if 0
    m_pView->doc()->emitBeginOperation( false );
    KSpellConfig *_spellConfig = m_spellConfigWidget;
    const KConfigGroup spellGroup = config->group( "KSpell kspread" );
    spellGroup.writeEntry("KSpell_NoRootAffix",(int) _spellConfig->noRootAffix ());
    spellGroup.writeEntry("KSpell_RunTogether", (int) _spellConfig->runTogether ());
    spellGroup.writeEntry("KSpell_Dictionary", _spellConfig->dictionary ());
    spellGroup.writeEntry("KSpell_DictFromList",(int)  _spellConfig->dictFromList());
    spellGroup.writeEntry("KSpell_Encoding", (int)  _spellConfig->encoding());
    spellGroup.writeEntry("KSpell_Client",  _spellConfig->client());
    //m_spellConfigWidget->saveDictionary();
    Doc* doc = m_pView->doc();
    doc->setKSpellConfig(*_spellConfig);

    bool state=dontCheckUpperWord->isChecked();
    spellGroup.writeEntry("KSpell_dont_check_upper_word",(int)state);
    doc->setDontCheckUpperWord(state);

    state=dontCheckTitleCase->isChecked();
    config->writeEntry("KSpell_dont_check_title_case",(int)state);
    doc->setDontCheckTitleCase(state);

    //m_pView->doc()->addIgnoreWordAllList( m_spellConfigWidget->ignoreList() );

    m_pView->slotUpdateView( m_pView->activeSheet() );
#endif
}

void configureSpellPage::slotDefault()
{
    //FIXME
    //m_spellConfigWidget->setDefault();
}

////

#include "PreferenceDialog.moc"
