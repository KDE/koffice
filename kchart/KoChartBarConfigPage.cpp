/*
 * $Id$
 *
 * Copyright 1999 by Kalle Dalheimer, released under Artistic License.
 */

#include "KoChartBarConfigPage.h"
#include "KoChartParameters.h"

#include "KoChartBarConfigPage.moc"

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <kapp.h>
#include <klocale.h>


KoChartBarConfigPage::KoChartBarConfigPage( QWidget* parent ) :
	QWidget( parent, "Bar diagramm config page" )
{
	// PENDING(kalle) Use layout management to layout group boxes in dialog


	QGridLayout *grid1 = new QGridLayout(this,2,2,15,7);

	QButtonGroup* gb = new QButtonGroup( i18n( "Overwrite Mode" ), this );

	QGridLayout *grid2 = new QGridLayout(gb,5,1,15,7);

	_sidebysideRB = new QRadioButton( i18n( "&Side by side" ), gb );
	_sidebysideRB->resize( _sidebysideRB->sizeHint() );
	grid2->addWidget( _sidebysideRB,1,0);

	_ontopRB = new QRadioButton( i18n( "On &Top" ), gb );
	_ontopRB->resize( _ontopRB->sizeHint() );
	grid2->addWidget( _ontopRB,2,0 );

	_infrontRB = new QRadioButton( i18n( "In &Front" ), gb );
	_infrontRB->resize( _infrontRB->sizeHint() );
	grid2->addWidget( _infrontRB,3,0 );

	grid2->addRowSpacing(0,7);
	grid2->addRowSpacing(1,_sidebysideRB->height());
	grid2->addRowSpacing(2,_ontopRB->height());
	grid2->addRowSpacing(3,_infrontRB->height());
	grid2->setRowStretch(0,0);
	grid2->setRowStretch(1,0);
	grid2->setRowStretch(2,0);
	grid2->setRowStretch(3,0);
	grid2->setRowStretch(4,1);

	grid2->addColSpacing(0,_sidebysideRB->width());
	grid2->addColSpacing(0,_ontopRB->width());
	grid2->addColSpacing(0,_infrontRB->width());
	grid2->setColStretch(0,1);
	
	grid2->activate();
	grid1->addWidget(gb,0,0);

	QGroupBox* gb2 = new QGroupBox( i18n( "X Axis Layout" ), this );
	QGridLayout *grid3 = new QGridLayout(gb2,3,2,15,7);

	QLabel* label = new QLabel( i18n( "&Distance between bars in percent of bar width" ), gb2 );
	label->resize( label->sizeHint() );
	grid3->addWidget( label,1,0 );

	_xbardist = new QLineEdit( gb2 );
	_xbardist->resize(30, _xbardist->sizeHint().height() );
	grid3->addWidget( _xbardist,1,1 );
	_xbardist->setValidator( new QIntValidator( 0, 200, _xbardist ) );
	label->setBuddy( _xbardist );

	grid3->addRowSpacing(0,7);
	grid3->addRowSpacing(1,label->height());
	grid3->addRowSpacing(1,_xbardist->height());
	grid3->setRowStretch(0,0);
	grid3->setRowStretch(1,0);
	grid3->setRowStretch(2,1);

	grid3->addColSpacing(0,label->width());
	grid3->addColSpacing(1,_xbardist->width());
	grid3->setColStretch(0,0);
	grid3->setColStretch(1,1);
	
	grid3->activate();
	grid1->addWidget(gb2,0,1);
	
	grid1->addRowSpacing(0,gb->height());
	grid1->addRowSpacing(0,gb2->height());
	grid1->setRowStretch(0,0);
	grid1->setRowStretch(1,1);

	grid1->addColSpacing(0,gb->width());
	grid1->addColSpacing(1,gb2->width());
	grid1->setColStretch(0,1);
	grid1->setColStretch(1,1);

	grid1->activate();
}


void KoChartBarConfigPage::setOverwriteMode( OverwriteMode overwrite )
{
	switch( overwrite ) {
	case SideBySide:
		_sidebysideRB->setChecked( true );
		_ontopRB->setChecked( false );
		_infrontRB->setChecked( false );
		break;
	case OnTop:
		_sidebysideRB->setChecked( false );
		_ontopRB->setChecked( true );
		_infrontRB->setChecked( false );
		break;
	case InFront:
		_sidebysideRB->setChecked( false );
		_ontopRB->setChecked( false );
		_infrontRB->setChecked( true );
		break;
	default:
		debug( "Unknown overwrite mode" );
	}
}


OverwriteMode KoChartBarConfigPage::overwriteMode() const
{
	if( _sidebysideRB->isChecked() )
		return SideBySide;
	else if( _infrontRB->isChecked() )
		return InFront;
	else
		return OnTop;
}

