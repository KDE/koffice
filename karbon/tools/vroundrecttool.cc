/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers

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

#include <klocale.h>
#include <knuminput.h>

#include "karbon_view.h"
#include "karbon_part.h"
#include "vroundrect.h"
#include "vroundrecttool.h"
#include "koUnitWidgets.h"


VRoundRectTool::VRoundRectOptionsWidget::VRoundRectOptionsWidget( KarbonPart* part, QWidget* parent, const char* name )
		: QGroupBox( 2, Qt::Horizontal, 0L, parent, name ), m_part( part )
{
	m_widthLabel = new QLabel( i18n( "Width:" ), this );
	m_width = new KoUnitDoubleSpinBox( this, 0.0, 1000.0, 0.5, 100.0, KoUnit::U_MM );

	m_heightLabel = new QLabel( i18n( "Height:" ), this );
	m_height = new KoUnitDoubleSpinBox( this, 0.0, 1000.0, 0.5, 100.0, KoUnit::U_MM );

	new QLabel( i18n( "Edge radius:" ), this );
	m_round = new KoUnitDoubleSpinBox( this, 0.0, 1000.0, 0.5 );
	refreshUnit();

	setInsideMargin( 4 );
	setInsideSpacing( 2 );
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
VRoundRectTool::VRoundRectOptionsWidget::round() const
{
	return m_round->value();
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
VRoundRectTool::VRoundRectOptionsWidget::setRound( double value )
{
	m_round->setValue( value );
}

void
VRoundRectTool::VRoundRectOptionsWidget::refreshUnit ()
{
	m_width->setUnit( m_part->unit() );
	m_height->setUnit( m_part->unit() );
}

VRoundRectTool::VRoundRectTool( KarbonView* view )
		: VShapeTool( view, i18n( "Insert Round Rectangle" ) )
{
	// Create config dialog:
	m_optionsWidget = new VRoundRectOptionsWidget( view->part() );
	m_optionsWidget->setRound( 20.0 );
}

VRoundRectTool::~VRoundRectTool()
{
	delete( m_optionsWidget );
}

void VRoundRectTool::refreshUnit()
{
	m_optionsWidget->refreshUnit();
}

VComposite*
VRoundRectTool::shape( bool interactive ) const
{
	if( interactive )
	{
		return
			new VRoundRect(
				0L,
				m_p,
				KoUnit::ptFromUnit( m_optionsWidget->width(), view()->part()->unit() ),
				KoUnit::ptFromUnit( m_optionsWidget->height(), view()->part()->unit() ),
				m_optionsWidget->round() );
	}
	else
		return
			new VRoundRect(
				0L,
				m_p,
				m_d1,
				m_d2,
				m_optionsWidget->round() );
}

