/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <qevent.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qwidget.h>

#include <klocale.h>

#include "vcdlg_star.h"

VCDlgStar::VCDlgStar()
	: KDialog( 0L, 0, true, Qt::WStyle_Customize |
	  WType_Dialog | Qt::WStyle_NormalBorder | Qt::WStyle_Title )
{
	setCaption( i18n( "Insert Star" ) );

	QBoxLayout* outerbox = new QHBoxLayout( this );

	// add input fields on the left:
	QGroupBox* group = new QGroupBox( 2, Qt::Horizontal, i18n( "Properties" ), this );
 	outerbox->addWidget( group );

	// add width/height-input:
	new QLabel( i18n( "Outer Radius:" ), group );
	m_outerR = new QLineEdit( 0, group );
	new QLabel( i18n( "Inner Radius:" ), group );
	m_innerR = new QLineEdit( 0, group );
	new QLabel( i18n( "Edges:" ), group );
	m_edges = new QSpinBox( group );
	m_edges->setMinValue( 3 );

	outerbox->addSpacing( 2 );

	// add buttons on the right side:
	QBoxLayout* innerbox = new QVBoxLayout( outerbox );

	innerbox->addStretch();

	QPushButton* okbutton = new QPushButton( i18n( "&Ok" ), this );
	QPushButton* cancelbutton = new QPushButton( i18n( "&Cancel" ), this );

	okbutton->setMaximumSize( okbutton->sizeHint() );
	cancelbutton->setMaximumSize( cancelbutton->sizeHint() );

	okbutton->setFocus();

	innerbox->addWidget( okbutton );
	innerbox->addSpacing( 2 );
	innerbox->addWidget( cancelbutton );

	// signals and slots:
	connect( okbutton, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( cancelbutton, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

double
VCDlgStar::valueInnerR() const
{
	return m_innerR->text().toDouble();
}

double
VCDlgStar::valueOuterR() const
{
	return m_outerR->text().toDouble();
}

uint
VCDlgStar::valueEdges() const
{
	return m_edges->value();
}

void
VCDlgStar::setValueInnerR( const double value )
{
	QString s;
	s.setNum( value, 'f', 3 );
	m_innerR->setText( s );
}

void
VCDlgStar::setValueOuterR( const double value )
{
	QString s;
	s.setNum( value, 'f', 3 );
	m_outerR->setText( s );
}

void
VCDlgStar::setValueEdges( const uint value )
{
	m_edges->setValue( value );
}

#include "vcdlg_star.moc"
