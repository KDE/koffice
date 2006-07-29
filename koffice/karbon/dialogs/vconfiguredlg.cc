/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Laurent Montel <lmontel@mandrakesoft.com>

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

#include <float.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qcombobox.h>
#include <qgrid.h>

#include <kiconloader.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcolorbutton.h>
#include <KoUnitWidgets.h>

#include "karbon_view.h"
#include "karbon_part.h"
#include "karbon_factory.h"

#include "vconfiguredlg.h"


VConfigureDlg::VConfigureDlg( KarbonView* parent )
		: KDialogBase( KDialogBase::IconList, i18n( "Configure" ),
					   KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Default,
					   KDialogBase::Ok, parent )

{
	QVBox * page = addVBoxPage(
					   i18n( "Interface" ), i18n( "Interface" ),
					   BarIcon( "misc", KIcon::SizeMedium ) );

	m_interfacePage = new VConfigInterfacePage( parent, page );

	page = addVBoxPage(
			   i18n( "Misc" ), i18n( "Misc" ),
			   BarIcon( "misc", KIcon::SizeMedium ) );

	m_miscPage = new VConfigMiscPage( parent, page );

	page = addVBoxPage(
			   i18n( "Grid" ), i18n( "Grid" ),
			   BarIcon( "grid", KIcon::SizeMedium ) );

	m_gridPage = new VConfigGridPage( parent, page );

	connect( m_miscPage, SIGNAL( unitChanged( int ) ), m_gridPage, SLOT( slotUnitChanged( int ) ) );

	page = addVBoxPage(
			   i18n( "Document" ), i18n( "Document Settings" ),
			   BarIcon( "document", KIcon::SizeMedium ) );

	m_defaultDocPage = new VConfigDefaultPage( parent, page );
	connect( this, SIGNAL( okClicked() ), this, SLOT( slotApply() ) );
}

void VConfigureDlg::slotApply()
{
	m_interfacePage->apply();
	m_miscPage->apply();
	m_defaultDocPage->apply();
	m_gridPage->apply();
}

void VConfigureDlg::slotDefault()
{
	switch( activePageIndex() )
	{
		case 0: m_interfacePage->slotDefault();
			break;
		case 1: m_miscPage->slotDefault();
			break;
		case 2: m_gridPage->slotDefault();
			break;
		case 3: m_defaultDocPage->slotDefault();
			break;
		default:
			break;
	}
}


VConfigInterfacePage::VConfigInterfacePage( KarbonView* view,
		QVBox* box, char* name )
		: QObject( box->parent(), name )
{
	m_view = view;
	m_config = KarbonFactory::instance()->config();

	m_oldRecentFiles = 10;
	m_oldCopyOffset = 10;
	m_oldDockerFontSize = 8;
	bool oldShowStatusBar = true;

	QVGroupBox* tmpQGroupBox = new QVGroupBox( i18n( "Interface" ), box );

	m_config->setGroup( "" );

	m_oldDockerFontSize = m_config->readNumEntry( "palettefontsize", m_oldDockerFontSize );

	if( m_config->hasGroup( "Interface" ) )
	{
		m_config->setGroup( "Interface" );

		m_oldRecentFiles = m_config->readNumEntry(
							   "NbRecentFile", m_oldRecentFiles );

		oldShowStatusBar = m_config->readBoolEntry(
							   "ShowStatusBar" , true );

		m_oldCopyOffset = m_config->readNumEntry(
							   "CopyOffset", m_oldCopyOffset );
	}

	m_showStatusBar = new QCheckBox( i18n( "Show status bar" ), tmpQGroupBox );
	m_showStatusBar->setChecked( oldShowStatusBar );

	m_recentFiles = new KIntNumInput( m_oldRecentFiles, tmpQGroupBox );
	m_recentFiles->setRange( 1, 20, 1 );
	m_recentFiles->setLabel( i18n( "Number of recent files:" ) );

	m_copyOffset = new KIntNumInput( m_oldCopyOffset, tmpQGroupBox );
	m_copyOffset->setRange( 1, 50, 1 );
	m_copyOffset->setLabel( i18n( "Copy offset:" ) );

	m_dockerFontSize = new KIntNumInput( m_oldDockerFontSize, tmpQGroupBox );
	m_dockerFontSize->setRange( 5, 20, 1 );
	m_dockerFontSize->setLabel( i18n( "Palette font size:" ) );
}

void VConfigInterfacePage::apply()
{
	bool showStatusBar = m_showStatusBar->isChecked();

	KarbonPart* part = m_view->part();

	m_config->setGroup( "Interface" );

	int recent = m_recentFiles->value();

	if( recent != m_oldRecentFiles )
	{
		m_config->writeEntry( "NbRecentFile", recent );
		m_view->setNumberOfRecentFiles( recent );
		m_oldRecentFiles = recent;
	}

	int copyOffset = m_copyOffset->value();

	if( copyOffset != m_oldCopyOffset )
	{
		m_config->writeEntry( "CopyOffset", copyOffset );
		m_oldCopyOffset = copyOffset;
	}

	bool refreshGUI = false;

	if( showStatusBar != part->showStatusBar() )
	{
		m_config->writeEntry( "ShowStatusBar", showStatusBar );
		part->setShowStatusBar( showStatusBar );
		refreshGUI = true;
	}

	m_config->setGroup( "" );

	int dockerFontSize = m_dockerFontSize->value();

	if( dockerFontSize != m_oldDockerFontSize )
	{
		m_config->writeEntry( "palettefontsize", dockerFontSize );
		m_oldDockerFontSize = dockerFontSize;
		refreshGUI = true;
	}

	if( refreshGUI )
		part->reorganizeGUI();

}

void VConfigInterfacePage::slotDefault()
{
	m_recentFiles->setValue( 10 );
	m_dockerFontSize->setValue( 8 );
	m_showStatusBar->setChecked( true );
}


VConfigMiscPage::VConfigMiscPage( KarbonView* view, QVBox* box, char* name )
		: QObject( box->parent(), name )
{
    m_view = view;
    m_config = KarbonFactory::instance()->config();

    KoUnit::Unit unit = view->part()->unit();

    QGroupBox* tmpQGroupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Misc" ), box, "GroupBox" );
    tmpQGroupBox->layout()->setSpacing(KDialog::spacingHint());
    tmpQGroupBox->layout()->setMargin(KDialog::marginHint());

    QGridLayout* grid = new QGridLayout(tmpQGroupBox->layout(), 4, 2 );

    m_oldUndoRedo = 30;

    QString unitType = KoUnit::unitName( unit );
    //#################"laurent
    //don't load unitType from config file because unit is
    //depend from kword file => unit can be different from config file

    if( m_config->hasGroup( "Misc" ) )
    {
        m_config->setGroup( "Misc" );
        m_oldUndoRedo = m_config->readNumEntry( "UndoRedo", m_oldUndoRedo );
    }

    m_undoRedo = new KIntNumInput( m_oldUndoRedo, tmpQGroupBox );
    m_undoRedo->setLabel( i18n( "Undo/redo limit:" ) );
    m_undoRedo->setRange( 10, 60, 1 );

    grid->addMultiCellWidget( m_undoRedo, 0, 0, 0, 1 );

    grid->addWidget( new QLabel(  i18n(  "Units:" ), tmpQGroupBox ), 1, 0 );

    m_unit = new QComboBox( tmpQGroupBox );
    m_unit->insertStringList( KoUnit::listOfUnitName() );
    grid->addWidget( m_unit, 1, 1 );
    m_oldUnit = KoUnit::unit( unitType );
    m_unit->setCurrentItem( m_oldUnit );
    connect( m_unit, SIGNAL( activated( int ) ), SIGNAL( unitChanged( int ) ) );
}

void VConfigMiscPage::apply()
{
    KarbonPart * part = m_view->part();

    m_config->setGroup( "Misc" );

    if( m_oldUnit != m_unit->currentItem() )
    {
        m_oldUnit = m_unit->currentItem();
	part->setUnit( static_cast<KoUnit::Unit>( m_oldUnit ) );
	part->document().setUnit(part->unit());
        m_config->writeEntry( "Units", KoUnit::unitName( part->unit() ) );
    }

    int newUndo = m_undoRedo->value();

    if( newUndo != m_oldUndoRedo )
    {
        m_config->writeEntry( "UndoRedo", newUndo );
        part->setUndoRedoLimit( newUndo );
        m_oldUndoRedo = newUndo;
    }
}

void VConfigMiscPage::slotDefault()
{
    m_undoRedo->setValue( 30 );
    m_unit->setCurrentItem( 0 );
}

VConfigGridPage::VConfigGridPage( KarbonView* view, QVBox* page, char* name )
		: QObject( page->parent(), name )
{
	m_view = view;
	KoUnit::Unit unit = view->part()->document().unit();
	KarbonGridData &gd = view->part()->document().grid();
	double pgw = view->part()->document().width();
	double pgh = view->part()->document().height();
	double fw = gd.freq.width();
	double fh = gd.freq.height();
	double sw = gd.snap.width();
	double sh = gd.snap.height();

	m_gridChBox = new QCheckBox( i18n( "Show &grid" ), page );
	m_gridChBox->setChecked( gd.isShow );
	m_snapChBox = new QCheckBox( i18n( "Snap to g&rid" ), page );
	m_snapChBox->setChecked( gd.isSnap );
	QLabel* gridColorLbl = new QLabel( i18n( "Grid &color:" ), page);
	m_gridColorBtn = new KColorButton( gd.color, page );
	gridColorLbl->setBuddy( m_gridColorBtn );
	QGroupBox* spacingGrp = new QGroupBox( 2, Qt::Horizontal, i18n( "Spacing" ), page );
	QLabel* spaceHorizLbl = new QLabel( i18n( "&Horizontal:" ), spacingGrp );
	m_spaceHorizUSpin = new KoUnitDoubleSpinBox( spacingGrp, 0.0, pgw, 0.1, fw, unit );
	spaceHorizLbl->setBuddy( m_spaceHorizUSpin );
	QLabel* spaceVertLbl = new QLabel( i18n( "&Vertical:" ), spacingGrp );
	m_spaceVertUSpin = new KoUnitDoubleSpinBox( spacingGrp, 0.0, pgh, 0.1, fh, unit );
	spaceVertLbl->setBuddy( m_spaceVertUSpin );
	QGroupBox* snapGrp = new QGroupBox( 2, Qt::Horizontal, i18n( "Snap Distance" ), page );
	QLabel* snapHorizLbl = new QLabel( i18n( "H&orizontal:" ), snapGrp );
	m_snapHorizUSpin = new KoUnitDoubleSpinBox( snapGrp, 0.0, fw, 0.1, sw, unit );
	snapHorizLbl->setBuddy( m_snapHorizUSpin );
	QLabel* snapVertLbl = new QLabel( i18n( "V&ertical:" ), snapGrp );
	m_snapVertUSpin = new KoUnitDoubleSpinBox( snapGrp, 0.0, fh, 0.1, sh, unit );
	snapVertLbl->setBuddy( m_snapVertUSpin );

	QGridLayout* gl = new QGridLayout();
	gl->setSpacing( KDialog::spacingHint() );
	gl->addMultiCellWidget( m_gridChBox, 0, 0, 0, 2 );
	gl->addMultiCellWidget( m_snapChBox, 1, 1, 0, 2 );
	gl->addWidget( gridColorLbl, 2, 0) ;
	gl->addWidget( m_gridColorBtn, 2, 1 );
	gl->addItem( new QSpacerItem( 0, 0 ), 2, 2 );
	gl->addMultiCellWidget( spacingGrp, 3, 3, 0, 2 );
	gl->addMultiCellWidget( snapGrp, 4, 4, 0, 2 );
	gl->addMultiCell( new QSpacerItem( 0, 0 ), 5, 5, 0, 2 );

	connect( m_spaceHorizUSpin, SIGNAL( valueChanged( double ) ), SLOT( setMaxHorizSnap( double ) ) );
	connect( m_spaceVertUSpin, SIGNAL( valueChanged( double ) ), SLOT( setMaxVertSnap( double ) ) ) ;
}

void VConfigGridPage::setMaxHorizSnap( double v )
{
	m_snapHorizUSpin->setMaxValue( v );
}

void VConfigGridPage::setMaxVertSnap( double v )
{
	m_snapVertUSpin->setMaxValue( v );
}

void VConfigGridPage::slotUnitChanged( int u )
{
	KoUnit::Unit unit = static_cast<KoUnit::Unit>( u );
	m_snapHorizUSpin->setUnit( unit );
	m_snapVertUSpin->setUnit( unit );
	m_spaceHorizUSpin->setUnit( unit );
	m_spaceVertUSpin->setUnit( unit );
}

void VConfigGridPage::apply()
{
	KarbonGridData &gd = m_view->part()->document().grid();
	gd.freq.setWidth( m_spaceHorizUSpin->value() );
	gd.freq.setHeight( m_spaceVertUSpin->value() );
	gd.snap.setWidth( m_snapHorizUSpin->value() );
	gd.snap.setHeight( m_snapVertUSpin->value() );
	gd.isShow = m_gridChBox->isChecked();
	gd.isSnap = m_snapChBox->isChecked();
	gd.color = m_gridColorBtn->color();
	m_view->repaintAll();
}

void VConfigGridPage::slotDefault()
{
	KoUnit::Unit unit = m_view->part()->document().unit();
	m_spaceHorizUSpin->setValue( KoUnit::toUserValue( 20.0, unit ) );
	m_spaceVertUSpin->setValue( KoUnit::toUserValue( 20.0, unit ) );
	m_snapHorizUSpin->setValue( KoUnit::toUserValue( 20.0, unit ) );
	m_snapVertUSpin->setValue( KoUnit::toUserValue( 20.0, unit ) );
	m_gridChBox->setChecked( true );
	m_snapChBox->setChecked( true );
	m_gridColorBtn->setColor( QColor( 228, 228, 228 ) );
}

VConfigDefaultPage::VConfigDefaultPage( KarbonView* view,
                                        QVBox* box, char* name )
    : QObject( box->parent(), name )
{
    m_view = view;

    m_config = KarbonFactory::instance()->config();

    QVGroupBox* gbDocumentSettings = new QVGroupBox(
        i18n( "Document Settings" ), box );
    gbDocumentSettings->setMargin( KDialog::marginHint() );
    gbDocumentSettings->setInsideSpacing( KDialog::spacingHint() );

    m_oldAutoSave = m_view->part()->defaultAutoSave() / 60;

    m_oldBackupFile = true;

    m_oldSaveAsPath = true;

    if( m_config->hasGroup( "Interface" ) )
    {
        m_config->setGroup( "Interface" );
        m_oldAutoSave = m_config->readNumEntry( "AutoSave", m_oldAutoSave );
        m_oldBackupFile = m_config->readBoolEntry( "BackupFile", m_oldBackupFile );
        m_oldSaveAsPath = m_config->readBoolEntry( "SaveAsPath", m_oldSaveAsPath );
    }

    m_autoSave = new KIntNumInput( m_oldAutoSave, gbDocumentSettings );
    m_autoSave->setRange( 0, 60, 1 );
    m_autoSave->setLabel( i18n( "Auto save (min):" ) );
    m_autoSave->setSpecialValueText( i18n( "No auto save" ) );
    m_autoSave->setSuffix( i18n( "min" ) );

    m_createBackupFile = new QCheckBox( i18n( "Create backup file" ), gbDocumentSettings );
    m_createBackupFile->setChecked( m_oldBackupFile );

    m_saveAsPath = new QCheckBox( i18n( "Save as path" ), gbDocumentSettings );
    m_saveAsPath->setChecked( m_oldSaveAsPath );
}

void VConfigDefaultPage::apply()
{
	m_config->setGroup( "Document defaults" );

	m_config->setGroup( "Interface" );

	int autoSave = m_autoSave->value();

	if( autoSave != m_oldAutoSave )
	{
		m_config->writeEntry( "AutoSave", autoSave );
		m_view->part()->setAutoSave( autoSave * 60 );
		m_oldAutoSave = autoSave;
	}

	bool state = m_createBackupFile->isChecked();

	if( state != m_oldBackupFile )
	{
		m_config->writeEntry( "BackupFile", state );
		m_view->part()->setBackupFile( state );
		m_oldBackupFile = state;
	}

	state = m_saveAsPath->isChecked();

	//if( state != m_oldSaveAsPath )
	//{
		m_config->writeEntry( "SaveAsPath", state );
		m_view->part()->document().saveAsPath( state );
		m_oldSaveAsPath = state;
	//}
}

void VConfigDefaultPage::slotDefault()
{
	m_autoSave->setValue( m_view->part()->defaultAutoSave() / 60 );
	m_createBackupFile->setChecked( true );
	m_saveAsPath->setChecked( true );
}

#include "vconfiguredlg.moc"

