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

#include <qlabel.h>
#include <qgroupbox.h>

#include <klocale.h>
#include <knuminput.h>

#include <karbon_view.h>
#include <karbon_part.h>
#include <shapes/vrectangle.h>
#include "vrectangletool.h"
#include <koUnitWidgets.h>


VRectangleTool::VRectangleOptionsWidget::VRectangleOptionsWidget( KarbonPart*part, QWidget* parent, const char* name )
	: KDialogBase( parent, name, true, i18n( "Insert rectangle" ), Ok | Cancel ), m_part(part)
{
	QGroupBox *group = new QGroupBox( 2, Qt::Horizontal, i18n( "Properties" ), this );
	// add width/height-input:
	m_widthLabel = new QLabel( i18n( "Width:" ), group );
	m_width = new KoUnitDoubleSpinBox( group, 0.0, 1000.0, 0.5, 100.0, KoUnit::U_MM );

	m_heightLabel = new QLabel( i18n( "Height:" ), group );
	m_height = new KoUnitDoubleSpinBox( group, 0.0, 1000.0, 0.5, 100.0, KoUnit::U_MM );

	refreshUnit();

	group->setInsideMargin( 4 );
	group->setInsideSpacing( 2 );

	setMainWidget( group );
	//setFixedSize( baseSize() );
}

double
VRectangleTool::VRectangleOptionsWidget::width() const
{
	return m_width->value();
}

double
VRectangleTool::VRectangleOptionsWidget::height() const
{
	return m_height->value();
}

void
VRectangleTool::VRectangleOptionsWidget::setWidth( double value )
{
	m_width->setValue( value );
}

void
VRectangleTool::VRectangleOptionsWidget::setHeight( double value )
{
	m_height->setValue( value );
}

void
VRectangleTool::VRectangleOptionsWidget::refreshUnit()
{
	m_width->setUnit( m_part->unit() );
	m_height->setUnit( m_part->unit() );
}

VRectangleTool::VRectangleTool( KarbonView* view )
	: VShapeTool( view, i18n( "Insert Rectangle" ) )
{
	// Create config dialog:
	m_optionWidget = new VRectangleOptionsWidget( view->part() );
	registerTool( this );
}

VRectangleTool::~VRectangleTool()
{
	delete( m_optionWidget );
}

void
VRectangleTool::refreshUnit()
{
	m_optionWidget->refreshUnit();
}

VComposite *
VRectangleTool::shape( bool interactive ) const
{
	if( interactive )
	{
		return
			new VRectangle(
				0L,
				m_p,
				KoUnit::ptFromUnit( m_optionWidget->width(), view()->part()->unit() ),
				KoUnit::ptFromUnit( m_optionWidget->height(), view()->part()->unit() ) );
	}
	else
		return
			new VRectangle(
				0L,
				m_p,
				m_d1,
				m_d2 );
}

bool
VRectangleTool::showDialog() const
{
	return m_optionWidget->exec() == QDialog::Accepted;
}

