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
#include <kdebug.h>

#include "vfill.h"

VFill::VFill()
	: m_type( none )
{
	/*m_gradient.addStop( VColor( Qt::red.rgb() ), 0.0 );
	m_gradient.addStop( VColor( Qt::yellow.rgb() ), 1.0 );
	m_gradient.setOrigin( KoPoint( 0, 0 ) );
	m_gradient.setVector( KoPoint( 0, 50 ) );
	m_gradient.setSpreadMethod( gradient_spread_reflect );*/
	//kdDebug() << "Size of VFill : " << sizeof(*this) << endl;
}

VFill::VFill( const VColor &c )
	: m_type( solid )
{
	m_color = c;
	//kdDebug() << "Size of VFill : " << sizeof(*this) << endl;
}

VFill::VFill( const VFill& fill )
{
	// doesn't copy parent:
	*this = fill;
}

void
VFill::save( QDomElement& element ) const
{
	QDomElement me = element.ownerDocument().createElement( "FILL" );
	element.appendChild( me );

	if( !( m_type == none ) )
	{
		// save color:
		m_color.save( me );
	}
	if( m_type == grad )
	{
		// save gradient:
		m_gradient.save( me );
	}
	else if( m_type == patt )
	{
		// save pattern:
		m_pattern.save( me );
	}
}

void
VFill::load( const QDomElement& element )
{
	m_type = none;

	// load color:
	QDomNodeList list = element.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();
			if( e.tagName() == "COLOR" )
			{
				m_type = solid;
				m_color.load( e );
			}
			if( e.tagName() == "GRADIENT" )
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

VFill&
VFill::operator=( const VFill& fill )
{
	if( this != &fill )
	{
		// dont copy the parent!
		m_type = fill.m_type;
		m_color = fill.m_color;
		m_gradient = fill.m_gradient;
		m_pattern = fill.m_pattern;
	}

	return *this;
}


