/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <kapp.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kspell.h>
#include <kdebug.h>


#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>

#include "kwconfig.h"
#include "kwview.h"
#include "kwunit.h"
#include "kwdoc.h"
#include "kwcanvas.h"
#include "kwviewmode.h"

KWConfig::KWConfig( KWView* parent )
  : KDialogBase(KDialogBase::IconList,i18n("Configure KWord") ,
		KDialogBase::Ok | KDialogBase::Cancel| KDialogBase::Default,
		KDialogBase::Ok)

{
  QVBox *page = addVBoxPage( i18n("Spelling"), i18n("Spell checker behavior"),
                        BarIcon("spellcheck", KIcon::SizeMedium) );
  _spellPage=new configureSpellPage(parent, page);

  QVBox *page2 = addVBoxPage( i18n("Interface"), i18n("Interface"),
                              BarIcon("misc", KIcon::SizeMedium) );
  _interfacePage=new configureInterfacePage(parent, page2);

  connect(this, SIGNAL(okClicked()),this,SLOT(slotApply()));
}

void KWConfig::slotApply()
{
    _spellPage->apply();
    _interfacePage->apply();
}

void KWConfig::slotDefault()
{
    switch(activePageIndex())
    {
        case 0:
            _spellPage->slotDefault();
            break;
        case 1:
            _interfacePage->slotDefault();
            break;
        default:
            break;
    }
}

configureSpellPage::configureSpellPage( KWView *_view, QVBox *box, char *name )
    : QObject( box->parent(), name )
{
  m_pView=_view;
  config = KWFactory::global()->config();
  QGroupBox* tmpQGroupBox = new QGroupBox( box, "GroupBox" );
  tmpQGroupBox->setTitle(i18n("Spelling"));

  QGridLayout *grid1 = new QGridLayout(tmpQGroupBox, 5, 1, KDialog::marginHint(), KDialog::spacingHint());
  grid1->addRowSpacing( 0, KDialog::marginHint() + 5 );
  grid1->setRowStretch( 4, 10 );
  _spellConfig = new KSpellConfig(tmpQGroupBox, 0L, m_pView->kWordDocument()->getKSpellConfig(), false );
  grid1->addWidget(_spellConfig,1,0);
#ifdef KSPELL_HAS_IGNORE_UPPER_WORD
  _dontCheckUpperWord= new QCheckBox(i18n("Ignore uppercase words"),tmpQGroupBox);
  grid1->addWidget(_dontCheckUpperWord,2,0);

  _dontCheckTilteCase= new QCheckBox(i18n("Ignore title case words"),tmpQGroupBox);
  grid1->addWidget(_dontCheckTilteCase,3,0);

  if( config->hasGroup("KSpell kword") )
    {
        config->setGroup( "KSpell kword" );
        _dontCheckUpperWord->setChecked(config->readBoolEntry("KSpell_dont_check_upper_word",false));
        _dontCheckTilteCase->setChecked(config->readBoolEntry("KSpell_dont_check_title_case",false));
    }
#endif
  //box->addWidget( tmpQGroupBox );
}

void configureSpellPage::apply()
{
  config->setGroup( "KSpell kword" );
  config->writeEntry ("KSpell_NoRootAffix",(int) _spellConfig->noRootAffix ());
  config->writeEntry ("KSpell_RunTogether", (int) _spellConfig->runTogether ());
  config->writeEntry ("KSpell_Dictionary", _spellConfig->dictionary ());
  config->writeEntry ("KSpell_DictFromList",(int)  _spellConfig->dictFromList());
  config->writeEntry ("KSpell_Encoding", (int)  _spellConfig->encoding());
  config->writeEntry ("KSpell_Client",  _spellConfig->client());

  m_pView->kWordDocument()->setKSpellConfig(*_spellConfig);

#ifdef KSPELL_HAS_IGNORE_UPPER_WORD
  bool state=_dontCheckUpperWord->isChecked();
  config->writeEntry ("KSpell_dont_check_upper_word",(int)state);
  m_pView->kWordDocument()->setDontCheckUpperWord(state);

  state=_dontCheckTilteCase->isChecked();
  config->writeEntry("KSpell_dont_check_title_case",(int)state);
  m_pView->kWordDocument()->setDontCheckTitleCase(state);
#endif
}

void configureSpellPage::slotDefault()
{
    _spellConfig->setNoRootAffix( 0);
    _spellConfig->setRunTogether(0);
    _spellConfig->setDictionary( "");
    _spellConfig->setDictFromList( FALSE);
    _spellConfig->setEncoding (KS_E_ASCII);
    _spellConfig->setClient (KS_CLIENT_ISPELL);
#ifdef KSPELL_HAS_IGNORE_UPPER_WORD
    _dontCheckUpperWord->setChecked(false);
    _dontCheckTilteCase->setChecked(false);
#endif
}

configureInterfacePage::configureInterfacePage( KWView *_view, QVBox *box, char *name )
 : QObject( box->parent(), name )
{
    m_pView=_view;
    config = KWFactory::global()->config();
    KWUnit::Unit unit = m_pView->kWordDocument()->getUnit();
    /*QVBoxLayout *box = new QVBoxLayout( this );
    box->setMargin( 5 );
    box->setSpacing( 10 );*/
    QGroupBox* tmpQGroupBox = new QGroupBox( box, "GroupBox" );
    tmpQGroupBox->setTitle(i18n("Interface"));

    QVBoxLayout *lay1 = new QVBoxLayout(tmpQGroupBox);
    lay1->addSpacing( 10 );
    lay1->setMargin( KDialog::marginHint() );
    lay1->setSpacing( KDialog::spacingHint() );

    // ### m_ is for member variables !
    int m_iGridX=10;
    int m_iGridY=10;
    double ptIndent = MM_TO_POINT(10.0);
    bool m_bShowRuler=true;

    oldNbRecentFiles=10;
    oldAutoSaveValue=KoDocument::defaultAutoSave() / 60;
    int nbPagePerRow=4;
    if( config->hasGroup("Interface") )
    {
        config->setGroup( "Interface" );
        m_iGridX=config->readNumEntry("GridX",m_iGridX);
        m_iGridY=config->readNumEntry("GridY",m_iGridY);
        ptIndent = config->readDoubleNumEntry("Indent", MM_TO_POINT(10.0));
        oldNbRecentFiles=config->readNumEntry("NbRecentFile",oldNbRecentFiles);
        m_bShowRuler=config->readBoolEntry("Rulers",true);
        oldAutoSaveValue=config->readNumEntry("AutoSave",oldAutoSaveValue);
        nbPagePerRow=config->readNumEntry("nbPagePerRow",nbPagePerRow);
    }


    showRuler= new QCheckBox(i18n("Show rulers"),tmpQGroupBox);
    showRuler->setChecked(m_bShowRuler);
    lay1->addWidget(showRuler);

    autoSave = new KIntNumInput( oldAutoSaveValue, tmpQGroupBox );
    autoSave->setRange(0, 60, 1);
    autoSave->setLabel(i18n("Auto save (min):"));
    autoSave->setSpecialValueText(i18n("No auto save"));
    autoSave->setSuffix(i18n("min"));

    lay1->addWidget(autoSave);

    recentFiles=new KIntNumInput( oldNbRecentFiles, tmpQGroupBox );
    recentFiles->setRange(1, 20, 1);
    recentFiles->setLabel(i18n("Number of recent file:"));

    lay1->addWidget(recentFiles);

    gridX=new KIntNumInput( m_iGridX, tmpQGroupBox );
    gridX->setRange(1, 50, 1);
    gridX->setLabel(i18n("X grid space"));
    lay1->addWidget(gridX);

    gridY=new KIntNumInput(m_iGridY, tmpQGroupBox );
    gridY->setRange(1, 50, 1);
    gridY->setLabel(i18n("Y grid space"));
    lay1->addWidget(gridY);

    // ### move this to KWUnit
    QString unitText;
    switch ( unit )
      {
      case KWUnit::U_MM:
	unitText=i18n("Millimeters (mm)");
	break;
      case KWUnit::U_INCH:
	unitText=i18n("Inches (inch)");
	break;
      case KWUnit::U_PT:
      default:
	unitText=i18n("points (pt)" );
      }

    double val = KWUnit::userValue( ptIndent, unit );
    indent = new KDoubleNumInput( val, tmpQGroupBox );
    indent->setRange(1, 50, 0.1);
    indent->setPrecision(1);
    indent->setLabel(i18n("1 is a unit name", "Indent in %1").arg(unitText));

    lay1->addWidget(indent);

    m_nbPagePerRow=new KIntNumInput(nbPagePerRow, tmpQGroupBox );
    m_nbPagePerRow->setRange(1, 10, 1);
    m_nbPagePerRow->setLabel(i18n("Preview mode - Number of pages per row:"));

    lay1->addWidget(m_nbPagePerRow);

    //box->addWidget( tmpQGroupBox);
}

void configureInterfacePage::apply()
{
    int valX=gridX->value();
    int valY=gridY->value();
    int nbRecent=recentFiles->value();
    bool ruler=showRuler->isChecked();
    KWDocument * doc = m_pView->kWordDocument();

    config->setGroup( "Interface" );
    if(valX!=doc->gridX())
    {
        config->writeEntry( "GridX",valX );
        doc->setGridX(valX);
    }
    if(valY!=doc->gridY())
    {
        config->writeEntry( "GridY",valY );
        doc->setGridY(valY);
    }

    double newIndent = KWUnit::fromUserValue( indent->value(), doc->getUnit() );
    if( newIndent != doc->getIndentValue() )
    {
        config->writeEntry( "Indent", newIndent );
        doc->setIndentValue( newIndent );
    }
    if(nbRecent!=oldNbRecentFiles)
    {
        config->writeEntry( "NbRecentFile", nbRecent);
        m_pView->changeNbOfRecentFiles(nbRecent);
    }

    if(ruler != doc->showRuler())
    {
        config->writeEntry( "Rulers", ruler );
        doc->setShowRuler( ruler );
        doc->reorganizeGUI();
    }
    int autoSaveVal=autoSave->value();
    if(autoSaveVal!=oldAutoSaveValue)
    {
        config->writeEntry( "AutoSave", autoSaveVal );
        doc->setAutoSave(autoSaveVal*60);
    }
    int nbPageByRow=m_nbPagePerRow->value();
    if(nbPageByRow!=doc->getNbPagePerRow())
    {
        config->writeEntry("nbPagePerRow",nbPageByRow);
        m_pView->getGUI()->canvasWidget()->viewMode()->setPagesPerRow(nbPageByRow);
        doc->setNbPagePerRow(nbPageByRow);
        m_pView->getGUI()->canvasWidget()->refreshViewMode();
    }
}

void configureInterfacePage::slotDefault()
{
    gridX->setValue(10);
    gridY->setValue(10);
    m_nbPagePerRow->setValue(4);
    KWDocument * doc = m_pView->kWordDocument();
    double newIndent = KWUnit::userValue( MM_TO_POINT( 10 ), doc->getUnit() );
    indent->setValue( newIndent );
    recentFiles->setValue(10);
    showRuler->setChecked(true);
    autoSave->setValue(KoDocument::defaultAutoSave()/60);
}

#include "kwconfig.moc"
