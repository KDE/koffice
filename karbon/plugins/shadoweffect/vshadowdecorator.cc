/* This file is part of the KDE project
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

#include "vshadowdecorator.h"
#include <core/vfill.h>
#include <core/vstroke.h>
#include <render/vpainter.h>

VShadowDecorator::VShadowDecorator( VObject *object, VObject* parent, int distance, int angle, float opacity )
	: VObject( parent ), m_object( object ), m_distance( distance ), m_angle( angle ), m_opacity( opacity )
{
}

VShadowDecorator::VShadowDecorator( const VShadowDecorator& other ) : VObject( other )
{
	m_object	= other.m_object->clone();
	m_opacity	= other.m_opacity;
	m_distance	= other.m_distance;
	m_angle		= other.m_angle;
}

VShadowDecorator::~VShadowDecorator()
{
	delete m_object;
}

void
VShadowDecorator::draw( VPainter* painter, const KoRect* rect ) const
{
	if( state() == deleted ||
		state() == hidden ||
		state() == hidden_locked )
	{
		return;
	}

	if( state() != VObject::edit )
	{
	int shadowDx = int( m_distance * cos( m_angle / 360. * 6.2832 ) );
	int shadowDy = int( m_distance * sin( m_angle / 360. * 6.2832 ) );

	VFill *fill = new VFill( *m_object->fill() );
	VStroke *stroke = new VStroke( *m_object->stroke() );
	VColor black( Qt::black );
	black.setOpacity( m_opacity );
	if( m_object->fill()->type() != VFill::none )
		m_object->fill()->setColor( black );
	m_object->stroke()->setColor( black );
	QWMatrix mat = painter->worldMatrix();
	painter->setWorldMatrix( mat.translate( shadowDx * painter->zoomFactor(), -shadowDy * painter->zoomFactor()) );
	m_object->draw( painter, rect );
	m_object->setFill( *fill );
	m_object->setStroke( *stroke );
	painter->setWorldMatrix( mat.translate( -shadowDx* painter->zoomFactor() , shadowDy * painter->zoomFactor() ) );
	}
	m_object->draw( painter, rect );
}

VObject *
VShadowDecorator::clone() const
{
	return new VShadowDecorator( *this );
}

void
VShadowDecorator::accept( VVisitor& visitor )
{
	m_object->accept( visitor );
}
