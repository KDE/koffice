/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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


#include <qgroupbox.h>
#include <qlabel.h>

#include <knuminput.h>

#include <karbon_view.h>
#include <karbon_part.h>
#include <shapes/vrectangle.h>
#include "vroundrecttool.h"
#include "koUnitWidgets.h"
#include <kgenericfactory.h>

VRoundRectTool::VRoundRectOptionsWidget::VRoundRectOptionsWidget( KarbonPart* part, QWidget* parent, const char* name )
	: KDialogBase( parent, name, true, i18n( "Insert Round Rect" ), Ok | Cancel ), m_part( part )
{
	QGroupBox *group = new QGroupBox( 2, Qt::Horizontal, i18n( "Properties" ), this );
	m_widthLabel = new QLabel( i18n( "Width:" ), group );
	m_width = new KoUnitDoubleSpinBox( group, 0.0, 1000.0, 0.5, 100.0, KoUnit::U_MM );

	m_heightLabel = new QLabel( i18n( "Height:" ), group );
	m_height = new KoUnitDoubleSpinBox( group, 0.0, 1000.0, 0.5, 100.0, KoUnit::U_MM );

	new QLabel( i18n( "Edge radius X:" ), group );
	m_roundx = new KoUnitDoubleSpinBox( group, 0.0, 1000.0, 0.5, 10.0, KoUnit::U_MM );

	new QLabel( i18n( "Edge radius Y:" ), group );
	m_roundy = new KoUnitDoubleSpinBox( group, 0.0, 1000.0, 0.5, 10.0, KoUnit::U_MM );
	refreshUnit();

	group->setInsideMargin( 4 );
	group->setInsideSpacing( 2 );

	setMainWidget( group );
	setFixedSize( baseSize() );
}

double
VRoundRectTool::VRoundRectOptionsWidget::width() const
{
	return m_width->value();
}

double
VRoundRectTool::VRoundRectOptionsWidget::height() const
{
	return m_height->value();
}

double
VRoundRectTool::VRoundRectOptionsWidget::roundx() const
{
	return m_roundx->value();
}

double
VRoundRectTool::VRoundRectOptionsWidget::roundy() const
{
	return m_roundy->value();
}

void
VRoundRectTool::VRoundRectOptionsWidget::setWidth( double value )
{
	m_width->setValue( value );
}

void
VRoundRectTool::VRoundRectOptionsWidget::setHeight( double value )
{
	m_height->setValue( value );
}

void
VRoundRectTool::VRoundRectOptionsWidget::setRoundX( double value )
{
	m_roundx->setValue( value );
}

void
VRoundRectTool::VRoundRectOptionsWidget::setRoundY( double value )
{
	m_roundy->setValue( value );
}

void
VRoundRectTool::VRoundRectOptionsWidget::refreshUnit ()
{
	m_width->setUnit( m_part->unit() );
	m_height->setUnit( m_part->unit() );
	m_roundx->setUnit( m_part->unit() );
	m_roundy->setUnit( m_part->unit() );
}

VRoundRectTool::VRoundRectTool( KarbonPart *part )
		: VShapeTool( part, i18n( "Insert Round Rectangle" ) )
{
	// Create config dialog:
	m_optionsWidget = new VRoundRectOptionsWidget( part );
	registerTool( this );
}

VRoundRectTool::~VRoundRectTool()
{
	delete( m_optionsWidget );
}

void VRoundRectTool::refreshUnit()
{
	m_optionsWidget->refreshUnit();
}

VPath*
VRoundRectTool::shape( bool interactive ) const
{
	if( interactive )
	{
		return
			new VRectangle(
				0L,
				m_p,
				KoUnit::fromUserValue( m_optionsWidget->width(), view()->part()->unit() ),
				KoUnit::fromUserValue( m_optionsWidget->height(), view()->part()->unit() ),
				KoUnit::fromUserValue( m_optionsWidget->roundx(), view()->part()->unit() ),
				KoUnit::fromUserValue( m_optionsWidget->roundy(), view()->part()->unit() ));
	}
	else
		return
			new VRectangle(
				0L,
				m_p,
				m_d1,
				m_d2,
				KoUnit::fromUserValue( m_optionsWidget->roundx(), view()->part()->unit() ),
				KoUnit::fromUserValue( m_optionsWidget->roundy(), view()->part()->unit() ) );
}

bool
VRoundRectTool::showDialog() const
{
	return m_optionsWidget->exec() == QDialog::Accepted;
}
