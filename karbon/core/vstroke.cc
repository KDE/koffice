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

#include <qdom.h>

#include "vobject.h"
#include "vstroke.h"


VStroke::VStroke( VObject* parent, float width, const VLineCap cap, const VLineJoin join,
			float miterLimit )
{
	m_parent = parent;
	m_type = solid;
	m_lineWidth = width;
	m_lineCap = cap;
	m_lineJoin = join;
	m_miterLimit = miterLimit;
}

VStroke::VStroke( const VColor &c, VObject* parent, float width, const VLineCap cap, const VLineJoin join,
			float miterLimit )
{
	m_parent = parent;
	m_type = solid;
	m_lineWidth = width;
	m_lineCap = cap;
	m_lineJoin = join;
	m_miterLimit = miterLimit;
	m_color = c;
}

VStroke::VStroke( const VStroke& stroke )
{
	// doesnt copy parent:
	*this = stroke;
}

void
VStroke::setLineWidth( float width )
{
	m_lineWidth = width;

	// tell our parent so he can update his bbox:
	if( m_parent )
		m_parent->invalidateBoundingBox();
}

void
VStroke::save( QDomElement& element ) const
{
	QDomElement me = element.ownerDocument().createElement( "STROKE" );
	element.appendChild( me );

	// save stroke parameters:
	if( m_lineWidth != 1.0 )
		me.setAttribute( "lineWidth", m_lineWidth );
	if( m_lineCap != capButt )
		me.setAttribute( "lineCap", m_lineCap );
	if( m_lineJoin != joinMiter )
		me.setAttribute( "lineJoin", m_lineJoin );
	if( m_miterLimit != 10.0 )
		me.setAttribute( "miterLimit", m_miterLimit );

	if( m_type == solid )
	{
		// save color:
		m_color.save( me );
	}
	else if( m_type == grad )
	{
		// save gradient:
		m_gradient.save( me );
	}
	else if( m_type == patt )
	{
		// save pattern:
		m_pattern.save( me );
	}

	// save dashpattern:
	m_dashPattern.save( me );
}

void
VStroke::load( const QDomElement& element )
{
	m_type = none;
	// load stroke parameters:
	m_lineWidth = element.attribute( "lineWidth", "1.0" ).toDouble();
	if( m_lineWidth < 0.0 )
		m_lineWidth = 0.0;

	switch( element.attribute( "lineCap", "0" ).toUShort() )
	{
		case 1:
			m_lineCap = capRound; break;
		case 2:
			m_lineCap = capSquare; break;
		default:
			m_lineCap = capButt;
	}

	switch( element.attribute( "lineJoin", "0" ).toUShort() )
	{
		case 1:
			m_lineJoin = joinRound; break;
		case 2:
			m_lineJoin = joinBevel; break;
		default:
			m_lineJoin = joinMiter;
	}

	m_miterLimit = element.attribute( "miterLimit", "10.0" ).toDouble();
	if( m_miterLimit < 0.0 )
		m_miterLimit = 0.0;


	// load color:
	QDomNodeList list = element.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();
			if( e.tagName() == "COLOR" )
			{
				m_color.load( e );
				m_type = solid;
			}
			else if( e.tagName() == "DASHPATTERN" )
			{
				m_dashPattern.load( e );
			}
			else if( e.tagName() == "GRADIENT" )
			{
				m_type = grad;
				m_gradient.load( e );
			}
			else if( e.tagName() == "PATTERN" )
			{
				m_type = patt;
				m_pattern.load( e );
			}
		}
	}
}


VStroke&
VStroke::operator=( const VStroke& stroke )
{
	if( this != &stroke )
	{
		// dont copy the parent!
		m_type = stroke.m_type;

		m_lineWidth = stroke.m_lineWidth;
		// Tell our parent about the linewidth change, so he can update his bbox:
		//if( m_parent )
		//	m_parent->invalidateBoundingBox();

		m_lineCap = stroke.m_lineCap;
		m_lineJoin = stroke.m_lineJoin;
		m_miterLimit = stroke.m_miterLimit;
		m_color = stroke.m_color;
		m_dashPattern = stroke.m_dashPattern;
		m_gradient = stroke.m_gradient;
		m_pattern = stroke.m_pattern;
	}

	return *this;
}

