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

// qpainter wrapper

#include "vqpainter.h"
#include "vstroke.h"
#include "vcolor.h"

#include <qpainter.h>
#include <qpaintdevice.h>
#include <qpen.h>

#include <koPoint.h>
#include <kdebug.h>

VQPainter::VQPainter( QPaintDevice *target, unsigned int w, unsigned int h ) : VPainter( target, w, h ), m_painter( 0L ), m_target( target ), m_width( w ), m_height( h )
{
	m_zoomFactor = 1;
	m_index = 0;
	m_painter = new QPainter( target );
}

VQPainter::~VQPainter()
{
	delete m_painter;
}

void
VQPainter::resize( unsigned int w, unsigned int h )
{
	m_width = w;
	m_height = h;
}

void
VQPainter::blit( const QRect & )
{
	end();
}

void
VQPainter::begin()
{
	if( !m_painter->isActive() )
	{
		m_painter->begin( m_target );
		m_painter->eraseRect( 0, 0, m_width, m_height );
	}
}

void
VQPainter::end()
{
	m_painter->end();
}

void
VQPainter::setWorldMatrix( const QWMatrix& mat )
{
	m_painter->setWorldMatrix( mat );
}

void
VQPainter::setZoomFactor( double zoomFactor )
{
	m_zoomFactor = zoomFactor;
	/*QWMatrix mat;
	mat.scale( zoomFactor, zoomFactor );
	m_painter->setWorldMatrix( mat );*/
}

void 
VQPainter::moveTo( const KoPoint &p )
{
	//m_index = 0;
	if( m_pa.size() <= m_index )
		m_pa.resize( m_index + 10 );

	m_pa.setPoint( m_index, static_cast<int>(p.x() * m_zoomFactor), static_cast<int>(p.y() * m_zoomFactor) );

	m_index++;
}

void 
VQPainter::lineTo( const KoPoint &p )
{
	if( m_pa.size() <= m_index )
		m_pa.resize( m_index + 10 );

	m_pa.setPoint( m_index, static_cast<int>(p.x() * m_zoomFactor), static_cast<int>(p.y() * m_zoomFactor) );

	m_index++;
}

void
VQPainter::curveTo( const KoPoint &p1, const KoPoint &p2, const KoPoint &p3 )
{
	// calculate cubic bezier using a temp QPointArray
	QPointArray pa( 4 );
	pa.setPoint( 0, m_pa.point( m_index - 1 ).x(), m_pa.point( m_index - 1 ).y() );
	pa.setPoint( 1, static_cast<int>(p1.x() * m_zoomFactor), static_cast<int>(p1.y() * m_zoomFactor) );
	pa.setPoint( 2, static_cast<int>(p2.x() * m_zoomFactor), static_cast<int>(p2.y() * m_zoomFactor) );
	pa.setPoint( 3, static_cast<int>(p3.x() * m_zoomFactor), static_cast<int>(p3.y() * m_zoomFactor) );

	QPointArray pa2( pa.cubicBezier() );

	m_pa.resize( m_index + pa2.size() );
	m_pa.putPoints( m_index, pa2.size(), pa2 );

	m_index += pa2.size();
}

void
VQPainter::newPath()
{
	m_index = 0;
}

void
VQPainter::fillPath()
{
	// we probably dont need filling for qpainter
	//m_index = 0;
}

void
VQPainter::strokePath()
{
	m_painter->drawPolyline( m_pa, 0, m_index );
	m_index = 0;
}

void
VQPainter::setPen( const VStroke &stroke )
{
	QPen pen;

	// color + linewidth
	pen.setColor( stroke.color().toQColor() );
	pen.setWidth( static_cast<int>(stroke.lineWidth()) );

	// caps
	if( stroke.lineCap() == VStroke::capButt )
		pen.setCapStyle( Qt::FlatCap );
	else if( stroke.lineCap() == VStroke::capRound )
		pen.setCapStyle( Qt::RoundCap );
	else if( stroke.lineCap() == VStroke::capSquare )
		pen.setCapStyle( Qt::SquareCap );

	m_painter->setPen( pen );
}

void
VQPainter::setBrush( const VFill & )
{
}

void
VQPainter::setPen( const QColor &c )
{
	m_painter->setPen( c );
}

void
VQPainter::setPen( Qt::PenStyle style )
{
	m_painter->setPen( style );
}

void
VQPainter::setBrush( const QColor &c )
{
	m_painter->setBrush( c );
}

void
VQPainter::setBrush( Qt::BrushStyle style )
{
	m_painter->setBrush( style );
}

void
VQPainter::save()
{
	m_painter->save();
}

void
VQPainter::restore()
{
	m_painter->restore();
}

void
VQPainter::setRasterOp( Qt::RasterOp r )
{
	m_painter->setRasterOp( r );
}

