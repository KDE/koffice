/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>

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

#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <KoUnitWidgets.h>
#include <knuminput.h>
#include <knumvalidator.h>
#include <kfontdialog.h>
#include <kdebug.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlayout.h>

#include "KWConfig.h"
#include "KWView.h"
#include "KWFrameSet.h"
#include "KWDocument.h"
#include "KWCanvas.h"
#include "KWViewMode.h"
#include "KWCommand.h"
#include "KWVariable.h"
#include "KoEditPath.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KoSpeaker.h"

#include <KoVariable.h>
#include <kformulaconfigpage.h>

#include <kspell2/configwidget.h>
#include <kspell2/settings.h>
#include <kspell2/broker.h>
using namespace KSpell2;

#include <float.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <kurlrequesterdlg.h>
#include <kfiledialog.h>
#include <qtabwidget.h>
#include <keditlistbox.h>
#include <KoGlobal.h>

// little helper stolen from kmail
// (Note: KDialogBase should have version of the methods that take a QString for the icon name)
static inline QPixmap loadIcon( const char * name ) {
  return KGlobal::instance()->iconLoader()
    ->loadIcon( QString::fromLatin1(name), KIcon::NoGroup, KIcon::SizeMedium );
}

KWConfig::KWConfig( KWView* parent )
  : KDialogBase(KDialogBase::IconList,i18n("Configure KWord") ,
          KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel| KDialogBase::Default,
          KDialogBase::Ok, parent)

{
  QVBox *page2 = addVBoxPage( i18n("Interface"), i18n("Interface Settings"),
                              loadIcon("configure") );
  m_interfacePage=new ConfigureInterfacePage(parent, page2);

  QVBox *page4 = addVBoxPage( i18n("Document"), i18n("Document Settings"),
                              loadIcon("kword_kwd") );

  m_defaultDocPage=new ConfigureDefaultDocPage(parent, page4);

  QVBox *page = addVBoxPage( i18n("Spelling"), i18n("Spell Checker Behavior"),
                        loadIcon("spellcheck") );
  m_spellPage = new ConfigureSpellPage(parent, page);

  QVBox *page5 = addVBoxPage( i18n("Formula"), i18n("Formula Defaults"),
                              loadIcon("kformula") );
  m_formulaPage=new KFormula::ConfigurePage( parent->kWordDocument()->formulaDocument( false ),
                                             this, KWFactory::instance()->config(), page5 );

  QVBox *page3 = addVBoxPage( i18n("Misc"), i18n("Misc Settings"),
                              loadIcon("misc") );
  m_miscPage=new ConfigureMiscPage(parent, page3);

  QVBox *page6 = addVBoxPage( i18n("Path"), i18n("Path Settings"),
                              loadIcon("path") );
  m_pathPage=new ConfigurePathPage(parent, page6);

  if (KoSpeaker::isKttsdInstalled()) {
      QVBox *page7 = addVBoxPage( i18n("Abbreviation for Text-to-Speech", "TTS"),
          i18n("Text-to-Speech Settings"), loadIcon("access") );
      m_ttsPage=new ConfigureTTSPage(parent, page7);
  } else m_ttsPage = 0;

  m_doc = parent->kWordDocument();
  connect(this, SIGNAL(okClicked()),this,SLOT(slotApply()));

  connect( m_interfacePage, SIGNAL( unitChanged( int ) ), SLOT( unitChanged( int ) ) );
  unitChanged( parent->kWordDocument()->unit() );
}

void KWConfig::unitChanged( int u )
{
    KoUnit::Unit unit = static_cast<KoUnit::Unit>(u);
    //m_spellPage->setUnit( unit );
    m_interfacePage->setUnit( unit );
    m_miscPage->setUnit( unit );
    m_defaultDocPage->setUnit( unit );
    //m_formulaPage->setUnit( unit );
    //m_pathPage->setUnit( unit );
}

void KWConfig::openPage(int flags)
{
    if(flags & KW_KSPELL)
        showPage( 2 );
    else if(flags & KP_INTERFACE)
        showPage(0 );
    else if(flags & KP_MISC)
        showPage(4);
    else if(flags & KP_DOCUMENT)
        showPage(2 );
    else if(flags & KP_FORMULA)
        showPage(3);
    else if ( flags & KP_PATH )
        showPage(4);
}

void KWConfig::slotApply()
{
    KMacroCommand *macro = 0L;
    if (m_spellPage) m_spellPage->apply();
    m_interfacePage->apply();
    m_pathPage->apply();
    KCommand * cmd = m_miscPage->apply();
    if ( cmd )
    {
        if ( !macro )
            macro = new KMacroCommand( i18n("Change Config"));
        macro->addCommand(cmd);
    }

    cmd=m_defaultDocPage->apply();
    if ( cmd )
    {
        if ( !macro )
            macro = new KMacroCommand( i18n("Change Config"));

        macro->addCommand( cmd );
    }
    m_formulaPage->apply();
    if (m_ttsPage) m_ttsPage->apply();
    if (macro)
        m_doc->addCommand( macro );
    KWFactory::instance()->config()->sync();
}

void KWConfig::slotDefault()
{
    switch(activePageIndex())
    {
    case 0:
        m_interfacePage->slotDefault();
        break;
    case 1:
        m_defaultDocPage->slotDefault();
        break;
    case 2:
        if (m_spellPage) m_spellPage->slotDefault();
        break;
    case 3:
        m_formulaPage->slotDefault();
        break;
    case 4:
        m_miscPage->slotDefault();
        break;
    case 5:
        m_pathPage->slotDefault();
        break;
    case 6:
        m_ttsPage->slotDefault();
    default:
        break;
    }
}

////

ConfigureSpellPage::ConfigureSpellPage( KWView *view, QVBox *box, char *name )
    : QObject( box->parent(), name )
{
    m_pView=view;
    config = KWFactory::instance()->config();
    m_spellConfigWidget = new ConfigWidget( view->broker(), box );
    m_spellConfigWidget->setBackgroundCheckingButtonShown( true );
    m_spellConfigWidget->layout()->setMargin( 0 );
}

void ConfigureSpellPage::apply()
{
  KWDocument* doc = m_pView->kWordDocument();

  m_spellConfigWidget->save();

  m_pView->kWordDocument()->setSpellCheckIgnoreList(
      m_pView->broker()->settings()->currentIgnoreList() );
  //FIXME reactivate just if there are changes.
  doc->enableBackgroundSpellCheck( m_pView->broker()->settings()->backgroundCheckerEnabled() );
  doc->reactivateBgSpellChecking();
}

void ConfigureSpellPage::slotDefault()
{
    m_spellConfigWidget->slotDefault();
}

ConfigureInterfacePage::ConfigureInterfacePage( KWView *view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    m_pView=view;
    config = KWFactory::instance()->config();
    QVGroupBox* gbInterfaceGroup = new QVGroupBox( i18n("Interface"), box, "GroupBox" );
    gbInterfaceGroup->setMargin( KDialog::marginHint() );
    gbInterfaceGroup->setInsideSpacing( KDialog::spacingHint() );

    double ptGridX=MM_TO_POINT(5.0 );
    double ptGridY=MM_TO_POINT(5.0 );
    double ptIndent = MM_TO_POINT(10.0);
    bool oldShowStatusBar = true;
    bool oldPgUpDownMovesCaret = false;
    bool oldShowScrollBar = true;
    oldNbRecentFiles=10;
    int nbPagePerRow=4;
    KoUnit::Unit unit = m_pView->kWordDocument()->unit();
    if( config->hasGroup("Interface") )
    {
        config->setGroup( "Interface" );
        ptGridX=config->readDoubleNumEntry("GridX", ptGridX);
        ptGridY=config->readDoubleNumEntry("GridY", ptGridY);
        ptIndent = config->readDoubleNumEntry("Indent", ptIndent);
        oldNbRecentFiles=config->readNumEntry("NbRecentFile", oldNbRecentFiles);
        nbPagePerRow=config->readNumEntry("nbPagePerRow", nbPagePerRow);
        oldShowStatusBar = config->readBoolEntry( "ShowStatusBar", true );
        oldPgUpDownMovesCaret = config->readBoolEntry( "PgUpDownMovesCaret", false );
        oldShowScrollBar = config->readBoolEntry("ShowScrollBar", true);
    }

    QHBox *hbUnit = new QHBox(gbInterfaceGroup);
    hbUnit->setSpacing(KDialog::spacingHint());
    QLabel *unitLabel= new QLabel(i18n("&Units:"),hbUnit);

    m_unitCombo = new QComboBox( hbUnit );
    m_unitCombo->insertStringList( KoUnit::listOfUnitName() );
    connect(m_unitCombo, SIGNAL(activated(int)), this, SIGNAL(unitChanged(int)));
    unitLabel->setBuddy( m_unitCombo );
    QString unitHelp = i18n("Select the unit type used every time a distance or width/height "
                            "is displayed or entered. This one setting is for the whole of KWord: all dialogs, the rulers etc. "
                            "Note that KWord documents specify the unit which was used to create them, so this setting "
                            "only affects this document and all documents that will be created later.");
    QWhatsThis::add( unitLabel, unitHelp );
    QWhatsThis::add( m_unitCombo, unitHelp );

    showStatusBar = new QCheckBox(i18n("Show &status bar"),gbInterfaceGroup);
    showStatusBar->setChecked(oldShowStatusBar);
    QWhatsThis::add( showStatusBar, i18n("Show or hide the status bar. If enabled, the status bar is shown at the bottom, which displays various information."));

    showScrollBar = new QCheckBox( i18n("Show s&crollbar"), gbInterfaceGroup);
    showScrollBar->setChecked(oldShowScrollBar);
    QWhatsThis::add( showScrollBar, i18n("Show or hide the scroll bar. If enabled, the scroll bar is shown on the right and lets you scroll up and down, which is useful for navigating through the document."));

    pgUpDownMovesCaret = new QCheckBox(i18n("PageUp/PageDown &moves the caret"),gbInterfaceGroup);
    pgUpDownMovesCaret->setChecked(oldPgUpDownMovesCaret);
    QWhatsThis::add( pgUpDownMovesCaret, i18n(
                         "If this option is enabled, the PageUp and PageDown keys "
                         "move the text caret, as in other KDE applications. "
                         "If it is disabled, they move the scrollbars, as in most other word processors." ) );

    QHBox* hbRecent = new QHBox( gbInterfaceGroup );
    QString recentHelp = i18n("The number of files remembered in the file open dialog and in the "
                              "recent files menu item.");
    QLabel* labelRecent = new QLabel( i18n("Number of recent &files:"), hbRecent );
    QWhatsThis::add( labelRecent, recentHelp );
    recentFiles=new KIntNumInput( oldNbRecentFiles, hbRecent );
    recentFiles->setRange(1, 20, 1);
    labelRecent->setBuddy( recentFiles );
    QWhatsThis::add( recentFiles, recentHelp );

    QHBox* hbGridX = new QHBox( gbInterfaceGroup );
    QString gridxHelp = i18n("The grid size on which frames, tabs and other content snaps while "
                             "moving and scaling.");
    QLabel* labelGridX = new QLabel( i18n("&Horizontal grid size:"), hbGridX );
    QWhatsThis::add( labelGridX, gridxHelp );
    gridX=new KoUnitDoubleSpinBox( hbGridX,
                                   0.1,
                                   50,
                                   0.1,
                                   ptGridX,
                                   unit );
    labelGridX->setBuddy( gridX );
    QWhatsThis::add( gridX, gridxHelp );

    QHBox* hbGridY = new QHBox( gbInterfaceGroup );
    QString gridyHelp = i18n("The grid size on which frames and other content snaps while "
                             "moving and scaling.");
    QLabel* labelGridY = new QLabel( i18n("&Vertical grid size:"), hbGridY );
    QWhatsThis::add( labelGridY, gridyHelp );
    gridY=new KoUnitDoubleSpinBox( hbGridY,
                                   0.1,
                                   50,
                                   0.1,
                                   ptGridY,
                                   unit );
    labelGridY->setBuddy( gridY );

    QWhatsThis::add( gridY, gridyHelp );

    QHBox* hbIndent = new QHBox( gbInterfaceGroup );
    QString indentHelp = i18n("Configure the indent width used when using the 'Increase' "
                              "or 'Decrease' indentation buttons on a paragraph.<p>The lower the value, "
                              "the more often the buttons will have to be pressed to gain the same "
                              "indentation.");
    QLabel* labelIndent = new QLabel( i18n("&Paragraph indent by toolbar buttons:"), hbIndent );
    QWhatsThis::add( labelIndent, indentHelp );
    indent = new KoUnitDoubleSpinBox( hbIndent,
                                      0.1,
                                      5000,
                                      0.1,
                                      ptIndent,
                                      unit );
    labelIndent->setBuddy( indent );
    QWhatsThis::add( indent, indentHelp );

    QHBox* hbPagePerRow = new QHBox( gbInterfaceGroup );
    QString pagePerRowHelp = i18n("After selecting Preview Mode (from the \"View\" menu,) "
                                  "this is the number of pages KWord will "
                                  "position on one horizontal row.");
    QLabel* labelPagePerRow = new QLabel( i18n("Number of pa&ges per row in preview mode:" ), hbPagePerRow );
    QWhatsThis::add( labelPagePerRow, pagePerRowHelp );
    m_nbPagePerRow=new KIntNumInput( 0, nbPagePerRow, hbPagePerRow );
    m_nbPagePerRow->setRange(1, 10, 1);
    labelPagePerRow->setBuddy( m_nbPagePerRow );
    hbPagePerRow->setStretchFactor( m_nbPagePerRow, 1 );
    QWhatsThis::add(m_nbPagePerRow , pagePerRowHelp );
}

void ConfigureInterfacePage::apply()
{
    KWDocument * doc = m_pView->kWordDocument();
    double valX = QMAX( 0.1, gridX->value() );
    double valY = QMAX( 0.1, gridY->value() );
    int nbRecent=recentFiles->value();

    bool statusBar=showStatusBar->isChecked();
    bool scrollBar = showScrollBar->isChecked();
    config->setGroup( "Interface" );
    bool updateView = false;
    if(valX!=doc->gridX())
    {
        config->writeEntry( "GridX", valX, true, false, 'g', DBL_DIG /* 6 is not enough */ );
        doc->setGridX(valX);
        updateView= true;
    }
    if(valY!=doc->gridY())
    {
        config->writeEntry( "GridY", valY, true, false, 'g', DBL_DIG /* 6 is not enough */ );
        doc->setGridY(valY);
        updateView= true;
    }
    double newIndent = indent->value();
    if( newIndent != doc->indentValue() )
    {
        config->writeEntry( "Indent", newIndent, true, false, 'g', DBL_DIG /* 6 is not enough */ );
        doc->setIndentValue( newIndent );
    }
    if(nbRecent!=oldNbRecentFiles)
    {
        config->writeEntry( "NbRecentFile", nbRecent);
        m_pView->changeNbOfRecentFiles(nbRecent);
    }
    bool refreshGUI= false;

    if( statusBar != doc->showStatusBar() )
    {
        config->writeEntry( "ShowStatusBar", statusBar );
        doc->setShowStatusBar( statusBar );
        refreshGUI=true;
    }

    if( scrollBar != doc->showScrollBar() )
    {
        config->writeEntry( "ShowScrollBar", scrollBar );
        doc->setShowScrollBar( scrollBar );
        refreshGUI=true;
    }

    bool b = pgUpDownMovesCaret->isChecked();
    if ( b != doc->pgUpDownMovesCaret() )
    {
        config->writeEntry( "PgUpDownMovesCaret", b );
        doc->setPgUpDownMovesCaret( b );
    }

    if( refreshGUI )
        doc->reorganizeGUI();


    int nbPageByRow=m_nbPagePerRow->value();
    if(nbPageByRow!=doc->nbPagePerRow())
    {
        config->writeEntry("nbPagePerRow",nbPageByRow);
        m_pView->getGUI()->canvasWidget()->viewMode()->setPagesPerRow(nbPageByRow);
        doc->setNbPagePerRow(nbPageByRow);
        //m_pView->getGUI()->canvasWidget()->refreshViewMode();
        //necessary to recreate new view because in switchViewMode
        //we delete viewmode that we want to apply (LM)
        // This needs to be cleaned up .... (DF)
        doc->switchViewMode( doc->viewModeType() ); // force a refresh
    }

    config->setGroup( "Misc" );
    KoUnit::Unit unit = static_cast<KoUnit::Unit>( m_unitCombo->currentItem() );
    // It's already been set in the document, see unitChanged
    config->writeEntry( "Units", KoUnit::unitName( unit ) );
    if ( updateView )
        doc->repaintAllViews(false);
}

void ConfigureInterfacePage::setUnit( KoUnit::Unit unit )
{
    m_unitCombo->blockSignals( true );
    m_unitCombo->setCurrentItem( unit );
    m_unitCombo->blockSignals( false );
    // We need to set it in the doc immediately, because much code here uses doc->unit()
    m_pView->kWordDocument()->setUnit( unit );

    gridX->setUnit( unit );
    gridY->setUnit( unit );
    indent->setUnit( unit );
}

void ConfigureInterfacePage::slotDefault()
{
    KWDocument * doc = m_pView->kWordDocument();
    m_unitCombo->setCurrentItem( KoUnit::U_CM );
    emit unitChanged( m_unitCombo->currentItem() );
    gridX->setValue( KoUnit::toUserValue( MM_TO_POINT( 5.0 ),doc->unit() ) );
    gridY->setValue( KoUnit::toUserValue( MM_TO_POINT( 5.0 ),doc->unit() ) );
    m_nbPagePerRow->setValue(4);
    double newIndent = KoUnit::toUserValue( MM_TO_POINT( 10 ), doc->unit() );
    indent->setValue( newIndent );
    recentFiles->setValue(10);
    showStatusBar->setChecked(true);
    pgUpDownMovesCaret->setChecked(false);
    showScrollBar->setChecked( true);
}

////

ConfigureMiscPage::ConfigureMiscPage( KWView *view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    m_pView=view;
    config = KWFactory::instance()->config();
    QVGroupBox* gbMiscGroup = new QVGroupBox( i18n("Misc"), box, "GroupBox" );
    gbMiscGroup->setMargin( KDialog::marginHint() );
    gbMiscGroup->setInsideSpacing( KDialog::spacingHint() );

    m_oldNbRedo=30;

    // Don't load the unit from config file because the unit can come from the kword file
    // => unit can be different from config file

    if( config->hasGroup("Misc") )
    {
        config->setGroup( "Misc" );
        m_oldNbRedo=config->readNumEntry("UndoRedo",m_oldNbRedo);
    }

    QHBox* hbUndoRedo = new QHBox( gbMiscGroup );
    QLabel* labelUndoRedo = new QLabel( i18n("Undo/&redo limit:"), hbUndoRedo );
    QString undoHelp = i18n("Limit the number of undo/redo actions remembered. "
                            "A lower value helps to save memory, a higher value allows "
                            "you to undo and redo more editing steps.");
    m_undoRedoLimit=new KIntNumInput( m_oldNbRedo, hbUndoRedo );
    m_undoRedoLimit->setRange(1, 100, 1);
    labelUndoRedo->setBuddy( m_undoRedoLimit );
    QWhatsThis::add( m_undoRedoLimit, undoHelp );
    QWhatsThis::add( labelUndoRedo, undoHelp );

    KWDocument* doc = m_pView->kWordDocument();
    m_displayLink=new QCheckBox(i18n("Display &links"),gbMiscGroup);
    m_displayLink->setChecked(doc->variableCollection()->variableSetting()->displayLink());
    QWhatsThis::add( m_displayLink, i18n("If enabled, a link is highlighted as such and is clickable.\n\n"
                                         "You can insert a link from the Insert menu."));
    m_underlineLink=new QCheckBox(i18n("&Underline all links"),gbMiscGroup);
    m_underlineLink->setChecked(doc->variableCollection()->variableSetting()->underlineLink());
    QWhatsThis::add( m_underlineLink, i18n("If enabled, a link is underlined."));

    m_displayComment=new QCheckBox(i18n("Display c&omments"),gbMiscGroup);
    m_displayComment->setChecked(doc->variableCollection()->variableSetting()->displayComment());
    QWhatsThis::add( m_displayComment, i18n("If enabled, comments are indicated by a small yellow box.\n\n"
                                            "You can show and edit a comment from the context menu."));

    m_displayFieldCode=new QCheckBox(i18n("Display field code"),gbMiscGroup);
    m_displayFieldCode->setChecked(doc->variableCollection()->variableSetting()->displayFieldCode());
    QWhatsThis::add( m_displayFieldCode, i18n("If enabled, the type of link is displayed instead "
                                              "of displaying the link text.\n\n"
                                              "There are various types of link that can be inserted, "
                                              "such as hyperlinks, files, mail, news and bookmarks."));


    QVGroupBox* gbViewFormatting = new QVGroupBox( i18n("View Formatting"), box, "view_formatting" );
    QWhatsThis::add( gbViewFormatting, i18n("These settings can be used to select the formatting "
                                            "characters that should be shown.\n\n"
                                            "Note that the selected formatting characters are only "
                                            "shown if formatting characters are enabled in general, "
                                            "which can be done from the View menu."));
    gbViewFormatting->setMargin( KDialog::marginHint() );
    gbViewFormatting->setInsideSpacing( KDialog::spacingHint() );

    m_oldFormattingEndParag = doc->viewFormattingEndParag();
    m_oldFormattingSpace = doc->viewFormattingSpace();
    m_oldFormattingTabs = doc->viewFormattingTabs();
    m_oldFormattingBreak = doc->viewFormattingBreak();

    m_cbViewFormattingEndParag = new QCheckBox( i18n("View formatting end paragraph"), gbViewFormatting);
    m_cbViewFormattingEndParag->setChecked(m_oldFormattingEndParag);

    m_cbViewFormattingSpace = new QCheckBox( i18n("View formatting space"), gbViewFormatting);
    m_cbViewFormattingSpace->setChecked(m_oldFormattingSpace);

    m_cbViewFormattingTabs = new QCheckBox( i18n("View formatting tabs"), gbViewFormatting);
    m_cbViewFormattingTabs->setChecked(m_oldFormattingTabs);

    m_cbViewFormattingBreak = new QCheckBox( i18n("View formatting break"), gbViewFormatting);
    m_cbViewFormattingBreak->setChecked(m_oldFormattingBreak);
}

ConfigureDefaultDocPage::~ConfigureDefaultDocPage()
{
    delete font;
}

KCommand *ConfigureMiscPage::apply()
{
    KWDocument * doc = m_pView->kWordDocument();
    config->setGroup( "Misc" );
    int newUndo=m_undoRedoLimit->value();
    if(newUndo!=m_oldNbRedo)
    {
        config->writeEntry("UndoRedo",newUndo);
        doc->setUndoRedoLimit(newUndo);
        m_oldNbRedo=newUndo;
    }
    KMacroCommand * macroCmd=0L;
    bool b=m_displayLink->isChecked();
    if(doc->variableCollection()->variableSetting()->displayLink()!=b)
    {
        if(!macroCmd)
        {
            macroCmd=new KMacroCommand(i18n("Change Display Link Command"));
        }
        KWChangeVariableSettingsCommand *cmd=new KWChangeVariableSettingsCommand( i18n("Change Display Link Command"), doc, doc->variableCollection()->variableSetting()->displayLink() ,b, KWChangeVariableSettingsCommand::VS_DISPLAYLINK);

        cmd->execute();
        macroCmd->addCommand(cmd);
    }
    b=m_underlineLink->isChecked();
    if(doc->variableCollection()->variableSetting()->underlineLink()!=b)
    {
        if(!macroCmd)
        {
            macroCmd=new KMacroCommand(i18n("Change Display Link Command"));
        }
        KWChangeVariableSettingsCommand *cmd=new KWChangeVariableSettingsCommand( i18n("Change Display Link Command"), doc, doc->variableCollection()->variableSetting()->underlineLink() ,b, KWChangeVariableSettingsCommand::VS_UNDERLINELINK);
        cmd->execute();
        macroCmd->addCommand(cmd);
    }

    b=m_displayComment->isChecked();
    if(doc->variableCollection()->variableSetting()->displayComment()!=b)
    {
        if(!macroCmd)
        {
            macroCmd=new KMacroCommand(i18n("Change Display Link Command"));
        }
        KWChangeVariableSettingsCommand *cmd=new KWChangeVariableSettingsCommand( i18n("Change Display Link Command"), doc, doc->variableCollection()->variableSetting()->displayComment() ,b, KWChangeVariableSettingsCommand::VS_DISPLAYCOMMENT);
        cmd->execute();
        macroCmd->addCommand(cmd);
    }
    b=m_displayFieldCode->isChecked();
    if(doc->variableCollection()->variableSetting()->displayFieldCode()!=b)
    {
        if(!macroCmd)
        {
            macroCmd=new KMacroCommand(i18n("Change Display Field Code Command"));
        }
        KWChangeVariableSettingsCommand *cmd=new KWChangeVariableSettingsCommand( i18n("Change Display Field Code Command"), doc, doc->variableCollection()->variableSetting()->displayFieldCode() ,b, KWChangeVariableSettingsCommand::VS_DISPLAYFIELDCODE);
        cmd->execute();
        macroCmd->addCommand(cmd);
    }

    bool state =m_cbViewFormattingEndParag->isChecked();
    bool needRepaint = false;
    if ( state != m_oldFormattingEndParag )
    {
        doc->setViewFormattingEndParag(state);
        needRepaint = true;
        m_oldFormattingEndParag = state;
    }
    state = m_cbViewFormattingSpace->isChecked();
    if ( state != m_oldFormattingSpace)
    {
        doc->setViewFormattingSpace(state);
        needRepaint = true;
        m_oldFormattingSpace = state;
    }
    state = m_cbViewFormattingBreak->isChecked();
    if ( state != m_oldFormattingBreak)
    {
        doc->setViewFormattingBreak(state);
        needRepaint = true;
        m_oldFormattingBreak = state;
    }
    state = m_cbViewFormattingTabs->isChecked();
    if ( state != m_oldFormattingTabs )
    {
        doc->setViewFormattingTabs(state);
        needRepaint = true;
        m_oldFormattingTabs= state;
    }
    if ( needRepaint )
    {
        doc->layout();
        doc->repaintAllViews();
    }
    return macroCmd;
}

void ConfigureMiscPage::slotDefault()
{
   m_undoRedoLimit->setValue(30);
   m_displayLink->setChecked(true);
   m_displayComment->setChecked(true);
   m_underlineLink->setChecked(true);
   m_cbViewFormattingEndParag->setChecked(true);
   m_cbViewFormattingSpace->setChecked(true);
   m_cbViewFormattingTabs->setChecked(true);
   m_cbViewFormattingBreak->setChecked(true);
   m_displayFieldCode->setChecked( false );
}

void ConfigureMiscPage::setUnit( KoUnit::Unit )
{
}

////

ConfigureDefaultDocPage::ConfigureDefaultDocPage( KWView *view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    m_pView=view;
    KWDocument * doc = m_pView->kWordDocument();
    config = KWFactory::instance()->config();
    QVGroupBox* gbDocumentDefaults = new QVGroupBox( i18n("Document Defaults"), box, "GroupBox" );
    gbDocumentDefaults->setMargin( KDialog::marginHint() );
    gbDocumentDefaults->setInsideSpacing( KDialog::spacingHint() );

    double ptColumnSpacing=3;
    KoUnit::Unit unit = doc->unit();
    if( config->hasGroup("Document defaults") )
    {
        config->setGroup( "Document defaults" );
        ptColumnSpacing=config->readDoubleNumEntry("ColumnSpacing",ptColumnSpacing);
        // loaded by kwdoc already defaultFont=config->readEntry("DefaultFont",defaultFont);
    }


    QHBox* hbColumnSpacing = new QHBox( gbDocumentDefaults );
    QLabel* columnSpacingLabel = new QLabel( i18n("Default column spacing:"), hbColumnSpacing );
    m_columnSpacing = new KoUnitDoubleSpinBox( hbColumnSpacing,
                                               0.1,
                                               50,
                                               0.1,
                                               ptColumnSpacing,
                                               unit );
    columnSpacingLabel->setBuddy( m_columnSpacing );
    QWhatsThis::add( m_columnSpacing, i18n("When setting a document to use more than one column "
                "this distance will be used to separate the columns. This value is merely a default "
                "setting as the column spacing can be changed per document") );

    QWidget *fontContainer = new QWidget(gbDocumentDefaults);
    QGridLayout * fontLayout = new QGridLayout(fontContainer, 1, 3);

    fontLayout->setSpacing(KDialog::spacingHint());
    fontLayout->setColStretch(0, 0);
    fontLayout->setColStretch(1, 1);
    fontLayout->setColStretch(2, 0);

    QLabel *fontTitle = new QLabel(i18n("Default font:"), fontContainer);

    font= new QFont( doc->defaultFont() );

    QString labelName = font->family() + ' ' + QString::number(font->pointSize());
    fontName = new QLabel(labelName, fontContainer);
    fontName->setFont(*font);
    fontName->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    QPushButton *chooseButton = new QPushButton(i18n("Choose..."), fontContainer);
    connect(chooseButton, SIGNAL(clicked()), this, SLOT(selectNewDefaultFont()));

    fontLayout->addWidget(fontTitle, 0, 0);
    fontLayout->addWidget(fontName, 0, 1);
    fontLayout->addWidget(chooseButton, 0, 2);


    oldAutoSaveValue=KoDocument::defaultAutoSave() / 60;
    m_oldLanguage = doc->globalLanguage();
    m_oldHyphenation = doc->globalHyphenation();
    if( config->hasGroup("Interface") )
    {
        config->setGroup( "Interface" );
        oldAutoSaveValue=config->readNumEntry("AutoSave",oldAutoSaveValue);
        m_oldLanguage = config->readEntry( "language", m_oldLanguage);
        m_oldHyphenation = config->readBoolEntry( "hyphenation", m_oldHyphenation);
    }


    QWidget *languageContainer = new QWidget(gbDocumentDefaults);
    QGridLayout * languageLayout = new QGridLayout(languageContainer, 1, 3);

    languageLayout->setSpacing(KDialog::spacingHint());
    languageLayout->setColStretch(0, 0);
    languageLayout->setColStretch(1, 1);

    QLabel *languageTitle = new QLabel(i18n("Global language:"), languageContainer);

    m_globalLanguage = new QComboBox( languageContainer );
    m_globalLanguage->insertStringList( KoGlobal::listOfLanguages() );
    m_globalLanguage->setCurrentText( KoGlobal::languageFromTag( m_oldLanguage ) );

    languageLayout->addWidget(languageTitle, 0, 0);
    languageLayout->addWidget(m_globalLanguage, 0, 1);

    m_autoHyphenation = new QCheckBox( i18n("Automatic hyphenation"), gbDocumentDefaults);
    m_autoHyphenation->setChecked( m_oldHyphenation );

    QVGroupBox* gbDocumentSettings = new QVGroupBox( i18n("Document Settings"), box );
    gbDocumentSettings->setMargin( KDialog::marginHint() );
    gbDocumentSettings->setInsideSpacing( KDialog::spacingHint() );

    QHBox* hbAutoSave = new QHBox( gbDocumentSettings );
    QLabel* labelAutoSave = new QLabel( i18n("Autosave every (min):"), hbAutoSave );
    autoSave = new KIntNumInput( oldAutoSaveValue, hbAutoSave );
    autoSave->setRange(0, 60, 1);
    labelAutoSave->setBuddy(autoSave);
    QWhatsThis::add( autoSave, i18n("A backup copy of the current document is created when a change "
                    "has been made. The interval used to create backup documents is set here.") );
    autoSave->setSpecialValueText(i18n("No autosave"));
    autoSave->setSuffix(i18n(" min"));

    m_oldBackupFile = true;
    if( config->hasGroup("Interface") )
    {
        config->setGroup( "Interface" );
        m_oldBackupFile=config->readBoolEntry("BackupFile",m_oldBackupFile);
    }

    m_createBackupFile = new QCheckBox( i18n("Create backup file"), gbDocumentSettings);
    m_createBackupFile->setChecked( m_oldBackupFile );

    QHBox* hbStartingPage = new QHBox( gbDocumentSettings );
    QLabel* labelStartingPage = new QLabel(i18n("Starting page number:"), hbStartingPage);

    m_oldStartingPage=doc->variableCollection()->variableSetting()->startingPageNumber();
    m_variableNumberOffset=new KIntNumInput(hbStartingPage);
    m_variableNumberOffset->setRange(-1000, 9999, 1, false);
    m_variableNumberOffset->setValue(m_oldStartingPage);
    labelStartingPage->setBuddy( m_variableNumberOffset );

    QHBox* hbTabStop = new QHBox( gbDocumentSettings );
    tabStop = new QLabel(i18n("Tab stop (%1):").arg(doc->unitName()), hbTabStop);
    m_tabStopWidth = new KoUnitDoubleSpinBox( hbTabStop,
                                              MM_TO_POINT(2),
                                              doc->pageManager()->page(doc->startPage())->width(),
                                              0.1,
                                              doc->tabStopValue(),
                                              unit );
    m_oldTabStopWidth = doc->tabStopValue();

    QVGroupBox* gbDocumentCursor = new QVGroupBox( i18n("Cursor"), box );
    gbDocumentCursor->setMargin( KDialog::marginHint() );
    gbDocumentCursor->setInsideSpacing( KDialog::spacingHint() );

    m_cursorInProtectedArea= new QCheckBox(i18n("Cursor in protected area"),gbDocumentCursor);
    m_cursorInProtectedArea->setChecked(doc->cursorInProtectedArea());

//     m_directInsertCursor= new QCheckBox(i18n("Direct insert cursor"),gbDocumentCursor);
//     m_directInsertCursor->setChecked(doc->insertDirectCursor());
}

KCommand *ConfigureDefaultDocPage::apply()
{
    config->setGroup( "Document defaults" );
    KWDocument * doc = m_pView->kWordDocument();
    double colSpacing = m_columnSpacing->value();
    if ( colSpacing != doc->defaultColumnSpacing() )
    {
        config->writeEntry( "ColumnSpacing", colSpacing , true, false, 'g', DBL_DIG );
        doc->setDefaultColumnSpacing(colSpacing);
    }
    config->writeEntry("DefaultFont",font->toString());

    config->setGroup( "Interface" );
    int autoSaveVal=autoSave->value();
    if(autoSaveVal!=oldAutoSaveValue)
    {
        config->writeEntry( "AutoSave", autoSaveVal );
        doc->setAutoSave(autoSaveVal*60);
        oldAutoSaveValue=autoSaveVal;
    }

    bool state =m_createBackupFile->isChecked();
    if(state!=m_oldBackupFile)
    {
        config->writeEntry( "BackupFile", state );
        doc->setBackupFile( state);
        m_oldBackupFile=state;
    }

    state = m_cursorInProtectedArea->isChecked();
    if ( state != doc->cursorInProtectedArea() )
    {
        config->writeEntry( "cursorInProtectArea", state );
        doc->setCursorInProtectedArea( state );
    }

//     state = m_directInsertCursor->isChecked();
//     if ( state != doc->insertDirectCursor() )
//         doc->setInsertDirectCursor( state );

    //Laurent Todo add a message box to inform user that
    //global language will change after re-launch kword
    QString lang = KoGlobal::tagOfLanguage( m_globalLanguage->currentText() );
    config->writeEntry( "language" , lang);
    m_oldLanguage = lang;
    //don't call this fiunction otherwise we can have a textobject with
    // a default language and other textobject with other default language.
    //doc->setGlobalLanguage( lang );

    state = m_autoHyphenation->isChecked();
    config->writeEntry( "hyphenation", state  );
    m_oldHyphenation = state;

    KMacroCommand * macroCmd=0L;
    int newStartingPage=m_variableNumberOffset->value();
    if(newStartingPage!=m_oldStartingPage)
    {
        macroCmd=new KMacroCommand(i18n("Change Starting Page Number"));
        KWChangeStartingPageCommand *cmd = new KWChangeStartingPageCommand( i18n("Change Starting Page Number"), doc, m_oldStartingPage,newStartingPage );
        cmd->execute();
        macroCmd->addCommand(cmd);
        m_oldStartingPage=newStartingPage;
    }
    double newTabStop = m_tabStopWidth->value();
    if ( newTabStop != m_oldTabStopWidth)
    {
        if ( !macroCmd )
            macroCmd=new KMacroCommand(i18n("Change Tab Stop Value"));


        KWChangeTabStopValueCommand *cmd = new KWChangeTabStopValueCommand( i18n("Change Tab Stop Value"), m_oldTabStopWidth, newTabStop, doc);
        cmd->execute();
        macroCmd->addCommand(cmd);
        m_oldTabStopWidth = newTabStop;
    }

    return macroCmd;
}

void ConfigureDefaultDocPage::slotDefault()
{
   m_columnSpacing->setValue( 3 );
   autoSave->setValue( KoDocument::defaultAutoSave() / 60 );
   m_variableNumberOffset->setValue(1);
   m_cursorInProtectedArea->setChecked(true);
   m_tabStopWidth->setValue( MM_TO_POINT(15) );
   m_createBackupFile->setChecked( true );
//    m_directInsertCursor->setChecked( false );
   m_globalLanguage->setCurrentText( KoGlobal::languageFromTag( KGlobal::locale()->language() ) );
   m_autoHyphenation->setChecked( false );
}

void ConfigureDefaultDocPage::selectNewDefaultFont() {
    QStringList list;
    KFontChooser::getFontList(list, KFontChooser::SmoothScalableFonts);
    KFontDialog dlg( (QWidget *)this->parent(), "Font Selector", false, true, list, true );
    dlg.setFont(*font);
    int result = dlg.exec();
    if (KDialog::Accepted == result) {
        delete font;
        font = new QFont(dlg.font());
        fontName->setText(font->family() + ' ' + QString::number(font->pointSize()));
        fontName->setFont(*font);
        m_pView->kWordDocument()->setDefaultFont( *font );
    }
}

void ConfigureDefaultDocPage::setUnit( KoUnit::Unit unit )
{
    m_columnSpacing->setUnit( unit );
    m_tabStopWidth->setUnit( unit );
    tabStop->setText( i18n( "Tab stop:" ) );
}

////

ConfigurePathPage::ConfigurePathPage( KWView *view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    m_pView=view;
    KWDocument * doc = m_pView->kWordDocument();
    config = KWFactory::instance()->config();
    QVGroupBox* gbPathGroup = new QVGroupBox( i18n("Path"), box, "GroupBox" );
    gbPathGroup->setMargin( KDialog::marginHint() );
    gbPathGroup->setInsideSpacing( KDialog::spacingHint() );

    m_pPathView = new KListView( gbPathGroup );
    m_pPathView->setResizeMode(QListView::NoColumn);
    m_pPathView->addColumn( i18n( "Type" ) );
    m_pPathView->addColumn( i18n( "Path" ), 400 ); // not too big by default
    (void) new QListViewItem( m_pPathView, i18n("Personal Expression"), doc->personalExpressionPath().join(";") );
    (void) new QListViewItem( m_pPathView, i18n("Backup Path"),doc->backupPath() );

    m_modifyPath = new QPushButton( i18n("Modify Path..."), gbPathGroup);
    connect( m_modifyPath, SIGNAL( clicked ()), this, SLOT( slotModifyPath()));
    connect( m_pPathView, SIGNAL( doubleClicked (QListViewItem *, const QPoint &, int  )), this, SLOT( slotModifyPath()));
    connect( m_pPathView, SIGNAL( selectionChanged ( QListViewItem * )), this, SLOT( slotSelectionChanged(QListViewItem * )));
    slotSelectionChanged(m_pPathView->currentItem());
}

void ConfigurePathPage::slotSelectionChanged(QListViewItem * item)
{
    m_modifyPath->setEnabled( item );
}

void ConfigurePathPage::slotModifyPath()
{
    QListViewItem *item = m_pPathView->currentItem ();
    if ( item )
    {
        if ( item->text(0)==i18n("Personal Expression"))
        {
            KoEditPathDia * dlg = new KoEditPathDia( item->text( 1),   0L, "editpath");
            if (dlg->exec() )
                item->setText(1, dlg->newPath());
            delete dlg;
        }
        if ( item->text(0)==i18n("Backup Path"))
        {
            KoChangePathDia *dlg = new KoChangePathDia( item->text(1), 0L, "backup path" );
            if (dlg->exec() )
                item->setText(1, dlg->newPath());
            delete dlg;
        }
    }
}

void ConfigurePathPage::slotDefault()
{
    QListViewItem * item = m_pPathView->findItem(i18n("Personal Expression"), 0);
    if ( item )
        item->setText(1, KWFactory::instance()->dirs()->resourceDirs("expression").join(";"));
    item = m_pPathView->findItem(i18n("Backup Path"), 0);
    if ( item )
        item->setText(1, QString::null );
}

void ConfigurePathPage::apply()
{
    QListViewItem * item = m_pPathView->findItem(i18n("Personal Expression"), 0);
    if ( item )
    {
        QStringList lst = QStringList::split(QString(";"), item->text(1));
        if ( lst != m_pView->kWordDocument()->personalExpressionPath())
        {
            m_pView->kWordDocument()->setPersonalExpressionPath(lst);
            config->setGroup( "Kword Path" );
            config->writePathEntry( "expression path", lst);
        }
    }
    item = m_pPathView->findItem(i18n("Backup Path"), 0);
    if ( item )
    {
        QString res = item->text(1 );
        if ( res != m_pView->kWordDocument()->backupPath())
        {
            config->setGroup( "Kword Path" );
            m_pView->kWordDocument()->setBackupPath( res );
            config->writePathEntry( "backup path",res );
        }
    }

}

////

ConfigureTTSPage::ConfigureTTSPage( KWView *view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    Q_UNUSED(view);
    // m_pView=_view;
    // KWDocument * doc = m_pView->kWordDocument();

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

    config = KWFactory::instance()->config();
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

void ConfigureTTSPage::slotDefault()
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

void ConfigureTTSPage::apply()
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

void ConfigureTTSPage::screenReaderOptionChanged()
{
    m_gbScreenReaderOptions->setEnabled(
        m_cbSpeakPointerWidget->isChecked() | m_cbSpeakFocusWidget->isChecked());
    m_leAcceleratorPrefixWord->setEnabled(m_cbSpeakAccelerators->isChecked());
    m_lblAcceleratorPrefix->setEnabled(m_cbSpeakAccelerators->isChecked());
}

#include "KWConfig.moc"
