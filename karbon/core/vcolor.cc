/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <qdom.h>

#include "vcolor.h"

VColor::VColor()
	: m_colorSpace( rgb )
{
	m_value[0] = 0.0;
	m_value[1] = 0.0;
	m_value[2] = 0.0;
	m_value[3] = 0.0;
}

void
VColor::pseudoValues( int& v1, int& v2, int& v3 ) const
{
	if( m_colorSpace == rgb )
	{
		v1 = qRound( 255 * m_value[0] );
		v2 = qRound( 255 * m_value[1] );
		v3 = qRound( 255 * m_value[2] );
	}
	else
	{
		double copy[3];

		convertToColorSpace( rgb, &copy[0], &copy[1], &copy[2] );

		v1 = qRound( 255 * copy[0] );
		v2 = qRound( 255 * copy[1] );
		v3 = qRound( 255 * copy[2] );
	}
}

void
VColor::values(
	double* v1, double* v2,
	double* v3, double* v4 ) const
{
	if( v1 )
		*v1 = m_value[0];
	if( v2 )
		*v2 = m_value[1];
	if( v3 )
		*v3 = m_value[2];
	if( v4 )
		*v4 = m_value[3];
}

void
VColor::setValues(
	const double* v1 = 0L, const double* v2 = 0L,
	const double* v3 = 0L, const double* v4 = 0L )
{
	if( v1 )
		m_value[0] = *v1;
	if( v2 )
		m_value[1] = *v2;
	if( v3 )
		m_value[2] = *v3;
	if( v4 )
		m_value[3] = *v4;
}

void
VColor::setColorSpace( const VColorSpace colorSpace )
{
	if( colorSpace == m_colorSpace )
		return;

	convertToColorSpace( colorSpace,
		&m_value[0], &m_value[1], &m_value[2], &m_value[3] )
;
	m_colorSpace = colorSpace;
}

bool
VColor::convertToColorSpace( const VColorSpace colorSpace,
	double* v1, double* v2, double* v3, double* v4 ) const
{
	double copy[4];

	if( colorSpace == rgb )
	{
		if( m_colorSpace == rgb )
		{
			copy[0] = m_value[0];
			copy[1] = m_value[1];
			copy[2] = m_value[2];
		}
		else if( m_colorSpace == cmyk )
		{
			copy[0] = 1.0 - m_value[0] - m_value[3];
			copy[1] = 1.0 - m_value[1] - m_value[3];
			copy[2] = 1.0 - m_value[2] - m_value[3];
		}
		else if( m_colorSpace == hsb )
		{
			if( 1.0 + m_value[1] == 1.0 )	// saturation == 0.0
			{
				copy[0] = m_value[2];	// brightness
				copy[1] = m_value[2];
				copy[2] = m_value[2];
			}
			else
			{
// TODO
			}
		}
		else if( m_colorSpace == gray )
		{
			copy[0] = m_value[0];
			copy[1] = m_value[0];
			copy[2] = m_value[0];
		}
	}
	else if( colorSpace == cmyk )
	{
		if( m_colorSpace == rgb )
		{
			copy[0] = 1.0 - m_value[0];
			copy[1] = 1.0 - m_value[1];
			copy[2] = 1.0 - m_value[2];
			copy[3] = 0.0;
// TODO: undercolor removal
		}
		else if( m_colorSpace == cmyk )
		{
			copy[0] = m_value[0];
			copy[1] = m_value[1];
			copy[2] = m_value[2];
			copy[3] = m_value[3];
		}
		else if( m_colorSpace == hsb )
		{
// TODO
		}
		else if( m_colorSpace == gray )
		{
			copy[0] = 0.0;
			copy[1] = 0.0;
			copy[2] = 0.0;
			copy[3] = 1.0 - m_value[0];
		}
	}
	else if( colorSpace == hsb )
	{
		if( m_colorSpace == rgb )
		{
// TODO
		}
		else if( m_colorSpace == cmyk )
		{
// TODO
		}
		else if( m_colorSpace == hsb )
		{
			copy[0] = m_value[0];
			copy[1] = m_value[1];
			copy[2] = m_value[2];
		}
		else if( m_colorSpace == gray )
		{
			copy[0] = 0.0;
			copy[1] = 0.0;
			copy[2] = m_value[0];
		}
	}
	else if( colorSpace == gray )
	{
		if( m_colorSpace == rgb )
		{
			copy[0] =
				0.3  * m_value[0] +
				0.59 * m_value[1] +
				0.11 * m_value[2];
		}
		else if( m_colorSpace == cmyk )
		{
			copy[0] =
				1.0 - QMIN( 1.0,
					0.3  * m_value[0] +
					0.59 * m_value[1] +
					0.11 * m_value[2] +
					m_value[3] );
		}
		else if( m_colorSpace == hsb )
		{
			copy[0] = m_value[2];
		}
		else if( m_colorSpace == gray )
		{
			copy[0] = m_value[0];
		}
	}

	if( v1 )
		*v1 = copy[0];
	if( v2 )
		*v2 = copy[1];
	if( v3 )
		*v3 = copy[2];
	if( v4 )
		*v4 = copy[3];
}

void
VColor::save( QDomElement& element ) const
{
	QDomElement me = element.ownerDocument().createElement( "COLOR" );
	element.appendChild( me );

	me.setAttribute( "colorSpace", m_colorSpace );

	if( m_colorSpace == gray )
		me.setAttribute( "v", m_value[0] );
	else
	{
		me.setAttribute( "v1", m_value[0] );
		me.setAttribute( "v2", m_value[1] );
		me.setAttribute( "v3", m_value[2] );

		if( m_colorSpace == cmyk )
			me.setAttribute( "v4", m_value[3] );
	}
}

void
VColor::load( const QDomElement& element )
{
	switch( element.attribute( "colorSpace" ).toUShort() )
	{
		case 1:
			m_colorSpace = cmyk; break;
		case 2:
			m_colorSpace = hsb; break;
		case 3:
			m_colorSpace = gray; break;
		default:
			m_colorSpace = rgb;
	}

	if( m_colorSpace == gray )
		m_value[0] = element.attribute( "v", "0.0" ).toDouble();
	else
	{
		m_value[0] = element.attribute( "v1", "0.0" ).toDouble();
		m_value[1] = element.attribute( "v2", "0.0" ).toDouble();
		m_value[2] = element.attribute( "v3", "0.0" ).toDouble();

		if( m_colorSpace == cmyk )
			m_value[3] = element.attribute( "v4", "0.0" ).toDouble();
	}

	if( m_value[0] < 0.0 || m_value[0] > 1.0 )
		m_value[0] = 0.0;
	if( m_value[1] < 0.0 || m_value[1] > 1.0 )
		m_value[1] = 0.0;
	if( m_value[2] < 0.0 || m_value[2] > 1.0 )
		m_value[2] = 0.0;
	if( m_value[3] < 0.0 || m_value[3] > 1.0 )
		m_value[3] = 0.0;
}


