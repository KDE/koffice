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
 * Boston, MA 02110-1301, USA.
*/

#include <qvbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kstatusbar.h>
#include <knuminput.h>
#include <kspell.h>
#include <kmessagebox.h>
#include <kdeversion.h>
#include <kcolorbutton.h>

#include <KoTabBar.h>
#include <KoSpeaker.h>

#include "kspread_sheet.h"
#include "kspread_sheetprint.h"
#include "kspread_doc.h"
#include "kspread_canvas.h"
#include "kspread_view.h"
#include "kspread_locale.h"
#include "kspread_editors.h"

#include "kspread_dlg_preference.h"

using namespace KSpread;

PreferenceDialog::PreferenceDialog( View* parent, const char* /*name*/)
  : KDialogBase(KDialogBase::IconList,i18n("Configure KSpread") ,
		KDialogBase::Ok | KDialogBase::Cancel| KDialogBase::Default,
		KDialogBase::Ok)

{
  m_pView=parent;

  connect(this, SIGNAL(okClicked()),this,SLOT(slotApply()));

  QVBox *page2=addVBoxPage(i18n("Locale Settings"), QString::null,BarIcon("gohome",KIcon::SizeMedium));
 _localePage=new parameterLocale(parent,page2 );

  QVBox *page3=addVBoxPage(i18n("Interface"), QString::null,BarIcon("signature", KIcon::SizeMedium) );
  _configure = new  configure(parent,page3 );

  QVBox * page4=addVBoxPage(i18n("Misc"), QString::null,BarIcon("misc",KIcon::SizeMedium) );
  _miscParameter = new  miscParameters(parent,page4 );

  QVBox *page5=addVBoxPage(i18n("Color"), QString::null,BarIcon("colorize",KIcon::SizeMedium) );
  _colorParameter=new colorParameters(parent,page5 );

  QVBox *page6=addVBoxPage(i18n("Page Layout"), QString::null,BarIcon("edit",KIcon::SizeMedium) );
  _layoutPage=new configureLayoutPage(parent,page6 );

  QVBox *page7 = addVBoxPage( i18n("Spelling"), i18n("Spell Checker Behavior"),
                          BarIcon("spellcheck", KIcon::SizeMedium) );
  _spellPage=new configureSpellPage(parent,page7);

  if (KoSpeaker::isKttsdInstalled()) {
      QVBox *page8 = addVBoxPage( i18n("Abbreviation for Text-to-Speech", "TTS"),
          i18n("Text-to-Speech Settings"), BarIcon("access", KIcon::SizeMedium) );
      _ttsPage=new configureTTSPage(parent, page8);
  } else _ttsPage = 0;

}

void PreferenceDialog::openPage(int flags)
{
    if(flags & KS_LOCALE)
        showPage( 0 );
    else if(flags & KS_INTERFACE)
        showPage( 1 );
    else if(flags & KS_MISC)
        showPage( 2 );
    else if(flags & KS_COLOR)
        showPage( 3 );
    else if(flags & KS_LAYOUT)
        showPage( 4 );
    else if(flags & KS_SPELLING)
        showPage( 5 );
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
  if (_ttsPage) _ttsPage->apply();
  m_pView->doc()->refreshInterface();
  m_pView->slotUpdateView( m_pView->activeSheet() );
}

void PreferenceDialog::slotDefault()
{
    switch(activePageIndex())
    {
        case 1:
            _configure->slotDefault();
            break;
        case 2:
            _miscParameter->slotDefault();
            break;
        case 3:
            _colorParameter->slotDefault();
            break;
        case 4:
            _layoutPage->slotDefault();
            break;
        case 5:
            _spellPage->slotDefault();
            break;
        case 6:
            _ttsPage->slotDefault();
            break;
        default:
            break;
    }
}


parameterLocale::parameterLocale( View* _view, QVBox *box , char *name )
 :QObject ( box->parent(),name)
{
    m_pView = _view;
    m_bUpdateLocale=false;
  QGroupBox* tmpQGroupBox = new QVGroupBox( i18n("Settings"), box, "GroupBox" );

  KLocale* locale=_view->doc()->locale();

  m_language=new QLabel( tmpQGroupBox,"label");
  m_number=new QLabel( tmpQGroupBox,"label6");
  m_date=new QLabel( tmpQGroupBox,"label1");
  m_shortDate=new QLabel( tmpQGroupBox,"label5");
  m_time=new QLabel( tmpQGroupBox,"label2");
  m_money=new QLabel( tmpQGroupBox,"label3");

  updateToMatchLocale(locale);

  m_updateButton=new QPushButton ( i18n("&Update Locale Settings"), tmpQGroupBox);
  connect(m_updateButton, SIGNAL(clicked()),this,SLOT(updateDefaultSystemConfig()));
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
    static_cast<Locale*>(m_pView->doc()->locale())->defaultSystemConfig( );
    KLocale* locale=m_pView->doc()->locale();
    updateToMatchLocale( locale );
}

void parameterLocale::updateToMatchLocale(KLocale* locale)
{
  m_language->setText( i18n("Language: %1").arg( locale->language() ));
  m_number->setText( i18n("Default number format: %1").arg( locale->formatNumber(12.55) ));
  m_date->setText( i18n("Long date format: %1").arg( locale->formatDate( QDate::currentDate() )));
  m_shortDate->setText( i18n("Short date format: %1").arg( locale->formatDate( QDate::currentDate() ,true) ));
  m_time->setText( i18n("Time format: %1").arg( locale->formatTime( QTime::currentTime() ) ));
  m_money->setText( i18n("Currency format: %1").arg( locale->formatMoney(12.55) ));
}

configure::configure( View* _view, QVBox *box , char *name )
 :QObject ( box->parent(),name)
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

  QGroupBox* tmpQGroupBox = new QVGroupBox( i18n("Settings"), box, "GroupBox" );

  config = Factory::global()->config();
  int _page=1;

  oldRecent=10;
  oldAutoSaveValue=KoDocument::defaultAutoSave()/60;

  if( config->hasGroup("Parameters" ))
        {
        config->setGroup( "Parameters" );
        _page=config->readNumEntry( "NbPage" ,1) ;
        horizontal=config->readBoolEntry("Horiz ScrollBar",true);
        vertical=config->readBoolEntry("Vert ScrollBar",true);
        colHeader=config->readBoolEntry("Column Header",true);
        rowHeader=config->readBoolEntry("Row Header",true);
	tabbar=config->readBoolEntry("Tabbar",true);
	formulaBar=config->readBoolEntry("Formula bar",true);
        statusBar=config->readBoolEntry("Status bar",true);
        oldRecent=config->readNumEntry( "NbRecentFile" ,10);
        oldAutoSaveValue=config->readNumEntry("AutoSave",KoDocument::defaultAutoSave()/60);
        m_oldBackupFile=config->readBoolEntry("BackupFile",m_oldBackupFile);
        }
  nbPage=new KIntNumInput(_page, tmpQGroupBox , 10);
  nbPage->setRange(1, 10, 1);
  nbPage->setLabel(i18n("Number of sheets open at the &beginning:"));
  QWhatsThis::add(nbPage, i18n( "Controls how many worksheets will be created if the option Start with an empty document is chosen when KSpread is started." ) );

  nbRecentFile=new KIntNumInput(oldRecent, tmpQGroupBox , 10);
  nbRecentFile->setRange(1, 20, 1);
  nbRecentFile->setLabel(i18n("&Number of files to show in Recent Files list:"));
  QWhatsThis::add(nbRecentFile, i18n( "Controls the maximum number of filenames that are shown when you select File-> Open Recent." ) );

  autoSaveDelay=new KIntNumInput(oldAutoSaveValue, tmpQGroupBox , 10);
  autoSaveDelay->setRange(0, 60, 1);
  autoSaveDelay->setLabel(i18n("Au&tosave delay (minutes):"));
  autoSaveDelay->setSpecialValueText(i18n("Do not save automatically"));
  autoSaveDelay->setSuffix(i18n("min"));
  QWhatsThis::add(autoSaveDelay, i18n( "Here you can select the time between autosaves, or disable this feature altogether by choosing Do not save automatically (drag the slider to the far left)." ) );

  m_createBackupFile = new QCheckBox( i18n("Create backup files"), tmpQGroupBox );
  m_createBackupFile->setChecked( m_oldBackupFile );
  QWhatsThis::add(m_createBackupFile, i18n( "Check this box if you want some backup files created. This is checked per default." ) );

  showVScrollBar=new QCheckBox(i18n("Show &vertical scrollbar"),tmpQGroupBox);
  showVScrollBar->setChecked(vertical);
  QWhatsThis::add(showVScrollBar, i18n( "Check or uncheck this box to show or hide the vertical scrollbar in all sheets." ) );

  showHScrollBar=new QCheckBox(i18n("Show &horizontal scrollbar"),tmpQGroupBox);
  showHScrollBar->setChecked(horizontal);
  QWhatsThis::add(showHScrollBar, i18n( "Check or uncheck this box to show or hide the horizontal scrollbar in all sheets." ) );

  showColHeader=new QCheckBox(i18n("Show c&olumn header"),tmpQGroupBox);
  showColHeader->setChecked(colHeader);
  QWhatsThis::add(showColHeader, i18n( "Check this box to show the column letters across the top of each worksheet." ) );
  showRowHeader=new QCheckBox(i18n("Show &row header"),tmpQGroupBox);
  showRowHeader->setChecked(rowHeader);
  QWhatsThis::add(showRowHeader, i18n( "Check this box to show the row numbers down the left side." ) );

  showTabBar =new QCheckBox(i18n("Show ta&bs"),tmpQGroupBox);
  showTabBar->setChecked(tabbar);
  QWhatsThis::add(showTabBar, i18n( "This check box controls whether the sheet tabs are shown at the bottom of the worksheet." ) );

  showFormulaBar =new QCheckBox(i18n("Sho&w formula toolbar"),tmpQGroupBox);
  showFormulaBar->setChecked(formulaBar);
  QWhatsThis::add(showFormulaBar, i18n( "Here is where you can choose to show or hide the Formula bar." ) );

  showStatusBar =new QCheckBox(i18n("Show stat&us bar"),tmpQGroupBox);
  showStatusBar->setChecked(statusBar);
  QWhatsThis::add(showStatusBar, i18n( "Uncheck this box if you want to hide the status bar." ) );
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
    config->setGroup( "Parameters" );
    config->writeEntry( "NbPage", nbPage->value());
    Doc *doc =m_pView->doc();
    bool active=true;
    active=showHScrollBar->isChecked();
    if( m_pView->horzScrollBar()->isVisible()!=active)
    {
        config->writeEntry( "Horiz ScrollBar",active);
        if( active)
            m_pView->horzScrollBar()->show();
        else
            m_pView->horzScrollBar()->hide();
        doc->setShowHorizontalScrollBar(active);
    }
    active=showVScrollBar->isChecked();
    if( m_pView->vertScrollBar()->isVisible()!=active)
    {
        config->writeEntry( "Vert ScrollBar", active);
        if(active)
            m_pView->vertScrollBar()->show();
        else
            m_pView->vertScrollBar()->hide();
        doc->setShowVerticalScrollBar(active);

    }
    active=showColHeader->isChecked();
    if( m_pView->hBorderWidget()->isVisible()!=active)
    {
        config->writeEntry( "Column Header", active);
        if( active)
            m_pView->hBorderWidget()->show();
        else
            m_pView->hBorderWidget()->hide();
        doc->setShowColHeader(active);
    }

    active=showRowHeader->isChecked();
    if( m_pView->vBorderWidget()->isVisible()!=active)
    {
        config->writeEntry( "Row Header", active);
        if( active)
            m_pView->vBorderWidget()->show();
        else
            m_pView->vBorderWidget()->hide();
        doc->setShowRowHeader(active);
    }

    active=showTabBar->isChecked();
    if(m_pView->tabBar()->isVisible()!=active)
    {
        config->writeEntry( "Tabbar", active);
        if(active)
            m_pView->tabBar()->show();
        else
            m_pView->tabBar()->hide();
        doc->setShowTabBar(active);
    }

    active=showFormulaBar->isChecked();
    if(m_pView->posWidget()->isVisible()!=active)
    {
        config->writeEntry( "Formula bar",active);
        m_pView->editWidget()->showEditWidget(active);
        if(active)
            m_pView->posWidget()->show();
        else
            m_pView->posWidget()->hide();
        doc->setShowFormulaBar(active);
    }

    active=showStatusBar->isChecked();
    config->writeEntry( "Status bar",active);
    m_pView->showStatusBar( active );

    int val=nbRecentFile->value();
    if( oldRecent!= val)
    {
       config->writeEntry( "NbRecentFile",val);
       m_pView->changeNbOfRecentFiles(val);
    }
    val=autoSaveDelay->value();
    if(val!=oldAutoSaveValue)
    {
        config->writeEntry( "AutoSave", val );
        doc->setAutoSave(val*60);
    }

    bool state =m_createBackupFile->isChecked();
    if(state!=m_oldBackupFile)
    {
        config->writeEntry( "BackupFile", state );
        doc->setBackupFile( state);
        m_oldBackupFile=state;
    }

    m_pView->slotUpdateView( m_pView->activeSheet() );
}


miscParameters::miscParameters( View* _view,QVBox *box, char *name )
 :QObject ( box->parent(),name)
 {
  m_pView = _view;


  QGroupBox* tmpQGroupBox = new QVGroupBox( i18n("Misc"), box, "GroupBox" );

  config = Factory::global()->config();
  indentUnit = _view->doc()->unit();
  double _indent = KoUnit::toUserValue( 10.0, indentUnit);
  bool m_bMsgError=false;
  if( config->hasGroup("Parameters" ))
        {
        config->setGroup( "Parameters" );
        _indent = config->readDoubleNumEntry( "Indent" , _indent ) ;
        m_bMsgError=config->readBoolEntry( "Msg error" ,false) ;
        }

  if( config->hasGroup("Misc") )
  {
   config->setGroup( "Misc" );
   m_oldNbRedo=config->readNumEntry("UndoRedo",m_oldNbRedo);
  }

  m_undoRedoLimit=new KIntNumInput( m_oldNbRedo, tmpQGroupBox );
  m_undoRedoLimit->setLabel(i18n("Undo/redo limit:"));
  m_undoRedoLimit->setRange(10, 60, 1);


  QLabel *label=new QLabel(i18n("&Completion mode:"), tmpQGroupBox);

  typeCompletion=new QComboBox(tmpQGroupBox);
  label->setBuddy(typeCompletion);
  QWhatsThis::add(typeCompletion, i18n( "Lets you choose the (auto) text completion mode from a range of options in the drop down selection box." ) );
  QStringList listType;
  listType+=i18n("None");
  listType+=i18n("Manual");
  listType+=i18n("Popup");
  listType+=i18n("Automatic");
  listType+=i18n("Semi-Automatic");
  typeCompletion->insertStringList(listType);
  typeCompletion->setCurrentItem(0);
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
  typeOfMove->insertStringList(listType);
  typeOfMove->setCurrentItem(0);
  QWhatsThis::add(typeOfMove, i18n( "When you have selected a cell, pressing the Enter key will move the cell cursor one cell left, right, up or down, as determined by this setting." ) );

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
  typeCalc->insertStringList(listTypeCalc);
  typeCalc->setCurrentItem(0);
  QWhatsThis::add(typeCalc, i18n( "This drop down selection box can be used to choose the calculation performed by the Statusbar Summary  function." ) );

//   valIndent = new KDoubleNumInput( _indent, tmpQGroupBox , 10.0 );
  valIndent = new KDoubleNumInput( tmpQGroupBox );
  valIndent->setRange( KoUnit::toUserValue( 0.0, indentUnit ),
                       KoUnit::toUserValue( 400.0, indentUnit ),
                       KoUnit::toUserValue( 10.0, indentUnit) );
//   valIndent->setRange( 0.0, 100.0, 10.0 );
  valIndent->setValue ( KoUnit::toUserValue( _indent, indentUnit ) );
  QWhatsThis::add(valIndent, i18n( "Lets you define the amount of indenting used by the Increase Indent and Decrease Indent option in the Format menu." ) );
  valIndent->setLabel(i18n("&Indentation step (%1):").arg(KoUnit::unitName(indentUnit)));

  msgError= new QCheckBox(i18n("&Show error message for invalid formulae"),tmpQGroupBox);
  msgError->setChecked(m_bMsgError);
  QWhatsThis::add(msgError, i18n( "If this box is checked a message box will pop up when what you have entered into a cell cannot be understood by KSpread." ) );

  initComboBox();
}

void miscParameters::slotTextComboChanged(const QString &)
{
  comboChanged=true;
}

void miscParameters::initComboBox()
{
  KGlobalSettings::Completion tmpCompletion=KGlobalSettings::CompletionAuto;
  if( config->hasGroup("Parameters" ))
    {
      config->setGroup( "Parameters" );
      tmpCompletion=( KGlobalSettings::Completion)config->readNumEntry( "Completion Mode" ,KGlobalSettings::CompletionAuto) ;
      config->writeEntry( "Completion Mode", (int)tmpCompletion);
    }
switch(tmpCompletion )
        {
        case  KGlobalSettings::CompletionNone:
                typeCompletion->setCurrentItem(0);
                break;
        case  KGlobalSettings::CompletionAuto:
                typeCompletion->setCurrentItem(3);
                break;
        case  KGlobalSettings::CompletionMan:
                typeCompletion->setCurrentItem(4);
                break;
        case  KGlobalSettings::CompletionShell:
                typeCompletion->setCurrentItem(1);
                break;
        case  KGlobalSettings::CompletionPopup:
                typeCompletion->setCurrentItem(2);
                break;
        default :
                typeCompletion->setCurrentItem(0);
                break;
        }
switch( m_pView->doc()->getMoveToValue( ))
        {
        case  Bottom:
                typeOfMove->setCurrentItem(0);
                break;
        case  Left:
                typeOfMove->setCurrentItem(3);
                break;
        case  Top:
                typeOfMove->setCurrentItem(1);
                break;
        case  Right:
                typeOfMove->setCurrentItem(2);
                break;
        case  BottomFirst:
                typeOfMove->setCurrentItem(4);
                break;
        default :
                typeOfMove->setCurrentItem(0);
                break;
        }

switch( m_pView->doc()->getTypeOfCalc())
        {
        case  SumOfNumber:
                typeCalc->setCurrentItem(0);
                break;
        case  Min:
                typeCalc->setCurrentItem(1);
                break;
        case  Max:
                typeCalc->setCurrentItem(2);
                break;
        case  Average:
                typeCalc->setCurrentItem(3);
                break;
        case  Count:
	        typeCalc->setCurrentItem(4);
                break;
        case  CountA:
	        typeCalc->setCurrentItem(5);
                break;
        case  NoneCalc:
	        typeCalc->setCurrentItem(6);
                break;
        default :
                typeCalc->setCurrentItem(0);
                break;
        }

}

void miscParameters::slotDefault()
{
  m_undoRedoLimit->setValue(30);
  valIndent->setValue( KoUnit::toUserValue( 10.0, indentUnit) );
  typeCompletion->setCurrentItem(3);
  typeOfMove->setCurrentItem(0);
  msgError->setChecked(false);
  typeCalc->setCurrentItem(0);
}


void miscParameters::apply()
{
    kdDebug() << "Applying misc preferences" << endl;

    config->setGroup( "Misc" );
    int const newUndo=m_undoRedoLimit->value();
    if( newUndo!=m_oldNbRedo )
    {
        config->writeEntry("UndoRedo",newUndo);
        m_pView->doc()->setUndoRedoLimit(newUndo);
        m_oldNbRedo=newUndo;
    }

    config->setGroup( "Parameters" );
    KGlobalSettings::Completion tmpCompletion=KGlobalSettings::CompletionNone;

    switch(typeCompletion->currentItem())
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
        config->writeEntry( "Completion Mode", (int)tmpCompletion);
    }

    KSpread::MoveTo tmpMoveTo=Bottom;
    switch(typeOfMove->currentItem())
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
    if(tmpMoveTo!=m_pView->doc()->getMoveToValue())
    {
        m_pView->doc()->setMoveToValue(tmpMoveTo);
        config->writeEntry( "Move", (int)tmpMoveTo);
    }

    MethodOfCalc tmpMethodCalc=SumOfNumber;
    switch(typeCalc->currentItem())
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
        config->writeEntry( "Method of Calc", (int)tmpMethodCalc);
        m_pView->calcStatusBarOp();
        m_pView->initCalcMenu();
    }

    double val = valIndent->value();
    if( val != m_pView->doc()->getIndentValue() )
    {
        KoUnit::Unit oldUnit = m_pView->doc()->unit();
        m_pView->doc()->setUnit(indentUnit);
        m_pView->doc()->setIndentValue( val );
        m_pView->doc()->setUnit(oldUnit);
        config->writeEntry( "Indent", KoUnit::fromUserValue( val, indentUnit ) );
    }

    bool active=msgError->isChecked();
    if(active!=m_pView->doc()->getShowMessageError())
    {
        m_pView->doc()->setShowMessageError( active);
        config->writeEntry( "Msg error" ,(int)active);
    }
}



colorParameters::colorParameters( View* _view,QVBox *box , char *name )
 :QObject ( box->parent(),name)
{
  m_pView = _view;
  config = Factory::global()->config();

  QColor _gridColor(Qt::lightGray);

  if ( config->hasGroup("KSpread Color" ) )
  {
    config->setGroup( "KSpread Color" );
    _gridColor = config->readColorEntry("GridColor",&_gridColor);
  }

  QGroupBox* tmpQGroupBox = new QVGroupBox( i18n("Color"), box, "GroupBox" );

  QLabel *label = new QLabel(i18n("&Grid color:"), tmpQGroupBox,"label20" );

  gridColor = new KColorButton( _gridColor,
                                Qt::lightGray,
                                tmpQGroupBox );
  QWhatsThis::add(gridColor, i18n( "Click here to change the grid color ie the color of the borders of each cell." ) );
  label->setBuddy(gridColor);

  QColor _pbColor(Qt::red);
  if ( config->hasGroup("KSpread Color" ) )
  {
    config->setGroup( "KSpread Color" );
    _pbColor = config->readColorEntry("PageBorderColor", &_pbColor);
  }

  QLabel * label2 = new QLabel( i18n("&Page borders:"), tmpQGroupBox, "label21" );

  pageBorderColor = new KColorButton( _pbColor,
                                Qt::red,
                                tmpQGroupBox );
  QWhatsThis::add(pageBorderColor, i18n( "When the View ->Show Page Borders menu item is checked, the page borders are displayed. Click here to choose another color for the borders than the default red." ) );

  label2->setBuddy(pageBorderColor);
}

void colorParameters::apply()
{
  QColor _col = gridColor->color();
  if ( m_pView->doc()->gridColor() != _col )
  {
    m_pView->doc()->setGridColor( _col );
    config->setGroup( "KSpread Color" );
    config->writeEntry( "GridColor", _col );
  }

  QColor _pbColor = pageBorderColor->color();
  if ( m_pView->doc()->pageBorderColor() != _pbColor )
  {
    m_pView->doc()->changePageBorderColor( _pbColor );
    config->setGroup( "KSpread Color" );
    config->writeEntry( "PageBorderColor", _pbColor );
  }
}

void colorParameters::slotDefault()
{
  gridColor->setColor( lightGray );
  pageBorderColor->setColor( red );
}



configureLayoutPage::configureLayoutPage( View* _view,QVBox *box , char *name )
 :QObject ( box->parent(),name)
{
  m_pView = _view;

  QGroupBox* tmpQGroupBox = new QGroupBox( 0, Qt::Vertical, i18n("Default Parameters"), box, "GroupBox" );
  tmpQGroupBox->layout()->setSpacing(KDialog::spacingHint());
  tmpQGroupBox->layout()->setMargin(KDialog::marginHint());

  QGridLayout *grid1 = new QGridLayout(tmpQGroupBox->layout(),8,1);
  grid1->addRowSpacing( 0, KDialog::marginHint()  );
  grid1->setRowStretch( 7, 10 );

  config = Factory::global()->config();

  QLabel *label=new QLabel(i18n("Default page &size:"), tmpQGroupBox);

  grid1->addWidget(label,0,0);

  defaultSizePage=new QComboBox( tmpQGroupBox);
  label->setBuddy(defaultSizePage);
  defaultSizePage->insertStringList( KoPageFormat::allFormats() );
  defaultSizePage->setCurrentItem(1);
  QWhatsThis::add(defaultSizePage, i18n( "Choose the default page size for your worksheet among all the most common page sizes.\nNote that you can overwrite the page size for the current sheet using the Format -> Page Layout... dialog." ) );
  grid1->addWidget(defaultSizePage,1,0);

  label=new QLabel(i18n("Default page &orientation:"), tmpQGroupBox);
  grid1->addWidget(label,2,0);

  defaultOrientationPage=new QComboBox( tmpQGroupBox);
  label->setBuddy(defaultOrientationPage);

  QStringList listType;
  listType+=i18n( "Portrait" );
  listType+=i18n( "Landscape" );
  defaultOrientationPage->insertStringList(listType);
  defaultOrientationPage->setCurrentItem(0);
  QWhatsThis::add(defaultOrientationPage, i18n( "Choose the sheet orientation: portrait or lanscape.\nNote that you can overwrite the orientation for the current sheet using the Format -> Page Layout... dialog." ) );
  grid1->addWidget(defaultOrientationPage,3,0);

  label=new QLabel(tmpQGroupBox);
  label->setText(i18n("Default page &unit:"));
  grid1->addWidget(label,4,0);
  defaultUnit=new QComboBox( tmpQGroupBox);
  label->setBuddy(defaultUnit);

  defaultUnit->insertStringList(KoUnit::listOfUnitName());
  defaultUnit->setCurrentItem(0);
  QWhatsThis::add(defaultUnit, i18n( "Choose the default unit that will be used in your sheet.\nNote that you can overwrite the unit for the current sheet using the Format -> Page Layout... dialog." ) );
  grid1->addWidget(defaultUnit,5,0);
  initCombo();

}

void configureLayoutPage::slotDefault()
{
  defaultSizePage->setCurrentItem(1);
  defaultOrientationPage->setCurrentItem(0);
  defaultUnit->setCurrentItem(0);
}

void configureLayoutPage::initCombo()
{
    paper=1;
    orientation=0;
    unit=0;
    if( config->hasGroup("KSpread Page Layout" ))
    {
        config->setGroup( "KSpread Page Layout" );
        paper=config->readNumEntry( "Default size page" ,1);
        orientation=config->readNumEntry( "Default orientation page" ,0);
        unit=config->readNumEntry( "Default unit page" ,0);
    }

    defaultUnit->setCurrentItem(m_pView->doc()->unit());
    defaultSizePage->setCurrentItem(paper);
    defaultOrientationPage->setCurrentItem(orientation);
}


void configureLayoutPage::apply()
{
  m_pView->doc()->emitBeginOperation( false );
  config->setGroup( "KSpread Page Layout" );

  if( paper != defaultSizePage->currentItem() )
  {
     unsigned int sizePage = defaultSizePage->currentItem();
     config->writeEntry( "Default size page", sizePage );
     m_pView->activeSheet()->print()->setPaperFormat( (KoFormat)sizePage );
  }
  if( orientation != defaultOrientationPage->currentItem() )
  {
     unsigned int orientationPage = defaultOrientationPage->currentItem();
     config->writeEntry( "Default orientation page", orientationPage );
     m_pView->activeSheet()->print()->setPaperOrientation( (KoOrientation)orientationPage );
  }
  if( unit != defaultUnit->currentItem() )
  {
     unsigned int unitPage = defaultUnit->currentItem();
     config->writeEntry( "Default unit page", unitPage );
     m_pView->doc()->setUnit( (KoUnit::Unit)unitPage );
  }
  m_pView->slotUpdateView( m_pView->activeSheet() );
}

configureSpellPage::configureSpellPage( View* _view,QVBox *box , char *name )
 :QObject ( box->parent(),name)
{
  m_pView = _view;

  config = Factory::global()->config();


  m_spellConfigWidget = new KSpellConfig( box, "spell_check",m_pView->doc()->getKSpellConfig()/*, false*/);
  dontCheckUpperWord = new QCheckBox( i18n("Skip all uppercase words"),box);
  QWhatsThis::add(dontCheckUpperWord, i18n( "If checked, the words written in uppercase letters are not spell checked. This might be useful if you have a lot of acronyms such as KDE for example." ) );
  dontCheckTitleCase = new QCheckBox( i18n("Do not check title case"),box);
  QWhatsThis::add(dontCheckTitleCase, i18n( "Check this box if you want the spellchecker to ignore the title case, for example My Own Spreadsheet or My own spreadsheet. If this is unchecked, the spell checker will ask for a uppercase letter in the title nouns." ) );

  QWidget* spacer = new QWidget( box );
  spacer->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );

    if( config->hasGroup("KSpell kspread") )
    {
        config->setGroup( "KSpell kspread" );

        dontCheckUpperWord->setChecked(config->readBoolEntry("KSpell_dont_check_upper_word",false));
        dontCheckTitleCase->setChecked(config->readBoolEntry("KSpell_dont_check_title_case",false));
    }
    //m_spellConfigWidget->addIgnoreList( m_pView->doc()->spellListIgnoreAll() );
}


void configureSpellPage::apply()
{
  m_pView->doc()->emitBeginOperation( false );

  KSpellConfig *_spellConfig = m_spellConfigWidget;
  config->setGroup( "KSpell kspread" );
  config->writeEntry ("KSpell_NoRootAffix",(int) _spellConfig->noRootAffix ());
  config->writeEntry ("KSpell_RunTogether", (int) _spellConfig->runTogether ());
  config->writeEntry ("KSpell_Dictionary", _spellConfig->dictionary ());
  config->writeEntry ("KSpell_DictFromList",(int)  _spellConfig->dictFromList());
  config->writeEntry ("KSpell_Encoding", (int)  _spellConfig->encoding());
  config->writeEntry ("KSpell_Client",  _spellConfig->client());
//  m_spellConfigWidget->saveDictionary();
  Doc* doc = m_pView->doc();
  doc->setKSpellConfig(*_spellConfig);

    bool state=dontCheckUpperWord->isChecked();
  config->writeEntry ("KSpell_dont_check_upper_word",(int)state);
  doc->setDontCheckUpperWord(state);

  state=dontCheckTitleCase->isChecked();
  config->writeEntry("KSpell_dont_check_title_case",(int)state);
  doc->setDontCheckTitleCase(state);

  //m_pView->doc()->addIgnoreWordAllList( m_spellConfigWidget->ignoreList() );

  m_pView->slotUpdateView( m_pView->activeSheet() );
}

void configureSpellPage::slotDefault()
{
    //FIXME
    //m_spellConfigWidget->setDefault();
}

////

configureTTSPage::configureTTSPage( View *_view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    Q_UNUSED(_view);

    m_cbSpeakPointerWidget = new QCheckBox(i18n("Speak widget under &mouse pointer"), box);
    m_cbSpeakFocusWidget = new QCheckBox(i18n("Speak widget with &focus"), box);
    m_gbScreenReaderOptions = new QVGroupBox("", box);
    m_gbScreenReaderOptions->setMargin( KDialog::marginHint() );
    m_gbScreenReaderOptions->setInsideSpacing( KDialog::spacingHint() );
    m_cbSpeakTooltips = new QCheckBox(i18n("Speak &tool tips"), m_gbScreenReaderOptions);
    m_cbSpeakWhatsThis = new QCheckBox(i18n("Speak &What's This"), m_gbScreenReaderOptions);
    m_cbSpeakDisabled = new QCheckBox(i18n("Verbal indication if widget is disabled (grayed)",
        "&Say whether disabled"), m_gbScreenReaderOptions);
    m_cbSpeakAccelerators = new QCheckBox(i18n("Spea&k accelerators"), m_gbScreenReaderOptions);
    QHBox* hbAcceleratorPrefix = new QHBox(m_gbScreenReaderOptions);
    QWidget* spacer = new QWidget(hbAcceleratorPrefix);
    spacer->setMinimumWidth(2 * KDialog::marginHint());
    m_lblAcceleratorPrefix =
        new QLabel(i18n("A word spoken before another word", "Pr&efaced by the word:"),
        hbAcceleratorPrefix);
    m_leAcceleratorPrefixWord = new QLineEdit(i18n("Keyboard accelerator, such as Alt+F", "Accelerator"),
        hbAcceleratorPrefix);
    m_lblAcceleratorPrefix->setBuddy(m_leAcceleratorPrefixWord);
    QHBox* hbPollingInterval = new QHBox(m_gbScreenReaderOptions);
    hbPollingInterval->setMargin( 0 );
    QLabel* lblPollingInterval = new QLabel(i18n("&Polling interval:"), hbPollingInterval);
    m_iniPollingInterval = new KIntNumInput(hbPollingInterval);
    m_iniPollingInterval->setSuffix(" ms");
    m_iniPollingInterval->setRange(100, 5000, 100, true);
    lblPollingInterval->setBuddy(m_iniPollingInterval);

    config = Factory::global()->config();
    config->setGroup("TTS");
    m_cbSpeakPointerWidget->setChecked(config->readBoolEntry("SpeakPointerWidget", false));
    m_cbSpeakFocusWidget->setChecked(config->readBoolEntry("SpeakFocusWidget", false));
    m_cbSpeakTooltips->setChecked(config->readBoolEntry("SpeakTooltips", true));
    m_cbSpeakWhatsThis->setChecked(config->readBoolEntry("SpeakWhatsThis", false));
    m_cbSpeakDisabled->setChecked(config->readBoolEntry("SpeakDisabled", true));
    m_cbSpeakAccelerators->setChecked(config->readBoolEntry("SpeakAccelerators", true));
    m_leAcceleratorPrefixWord->setText(config->readEntry("AcceleratorPrefixWord",
        i18n("Keyboard accelerator, such as Alt+F", "Accelerator")));
    m_iniPollingInterval->setValue(config->readNumEntry("PollingInterval", 600));

    screenReaderOptionChanged();
    connect(m_cbSpeakPointerWidget, SIGNAL(toggled(bool)), this, SLOT(screenReaderOptionChanged()));
    connect(m_cbSpeakFocusWidget, SIGNAL(toggled(bool)), this, SLOT(screenReaderOptionChanged()));
    connect(m_cbSpeakAccelerators, SIGNAL(toggled(bool)), this, SLOT(screenReaderOptionChanged()));
}

void configureTTSPage::slotDefault()
{
    m_cbSpeakPointerWidget->setChecked(false);
    m_cbSpeakFocusWidget->setChecked(false);
    m_cbSpeakTooltips->setChecked(true);
    m_cbSpeakWhatsThis->setChecked(false);
    m_cbSpeakDisabled->setChecked(true);
    m_cbSpeakAccelerators->setChecked(true);
    m_leAcceleratorPrefixWord->setText(i18n("Keyboard accelerator, such as Alt+F", "Accelerator"));
    m_iniPollingInterval->setValue(600);
}

void configureTTSPage::apply()
{
    config->setGroup("TTS");
    config->writeEntry("SpeakPointerWidget", m_cbSpeakPointerWidget->isChecked());
    config->writeEntry("SpeakFocusWidget", m_cbSpeakFocusWidget->isChecked());
    config->writeEntry("SpeakTooltips", m_cbSpeakTooltips->isChecked());
    config->writeEntry("SpeakWhatsThis", m_cbSpeakWhatsThis->isChecked());
    config->writeEntry("SpeakDisabled", m_cbSpeakDisabled->isChecked());
    config->writeEntry("SpeakAccelerators", m_cbSpeakAccelerators->isChecked());
    config->writeEntry("AcceleratorPrefixWord", m_leAcceleratorPrefixWord->text());
    config->writeEntry("PollingInterval", m_iniPollingInterval->value());
    if (kospeaker) kospeaker->readConfig(config);
}

void configureTTSPage::screenReaderOptionChanged()
{
    m_gbScreenReaderOptions->setEnabled(
        m_cbSpeakPointerWidget->isChecked() || m_cbSpeakFocusWidget->isChecked());
    m_leAcceleratorPrefixWord->setEnabled(m_cbSpeakAccelerators->isChecked());
    m_lblAcceleratorPrefix->setEnabled(m_cbSpeakAccelerators->isChecked());
}

#include "kspread_dlg_preference.moc"
