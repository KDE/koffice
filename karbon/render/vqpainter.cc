/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

// qpainter wrapper

#include "vqpainter.h"
#include "vstroke.h"
#include "vcolor.h"

#include <qpainter.h>
#include <qwidget.h>
#include <qpen.h>

#include <koPoint.h>
#include <kdebug.h>

VQPainter::VQPainter( QWidget *target, int w, int h ) : VPainter( target, w, h ), m_painter( 0L ), m_target( target ), m_width( w ), m_height( h )
{
	m_index = 0;
	m_painter = new QPainter( target );
}

VQPainter::~VQPainter()
{
	delete m_painter;
}

void
VQPainter::resize( int w, int h )
{
	m_width = w;
	m_height = h;
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
VQPainter::setWorldMatrix( const QWMatrix &mat )
{
	//m_painter->setWorldMatrix( mat );
}

void 
VQPainter::moveTo( const KoPoint &p )
{
	//m_index = 0;
	if( m_pa.size() <= m_index )
		m_pa.resize( m_index + 10 );

	m_pa.setPoint( m_index, p.x(), p.y() );

	m_index++;
}

void 
VQPainter::lineTo( const KoPoint &p )
{
	if( m_pa.size() <= m_index )
		m_pa.resize( m_index + 10 );

	m_pa.setPoint( m_index, p.x(), p.y() );

	m_index++;
}

void
VQPainter::curveTo( const KoPoint &p1, const KoPoint &p2, const KoPoint &p3 )
{
	// calculate cubic bezier using a temp QPointArray
	QPointArray pa( 4 );
	pa.setPoint( 0, m_pa.point( m_index - 1 ).x(), m_pa.point( m_index - 1 ).y() );
	pa.setPoint( 1, p1.x(), p1.y() );
	pa.setPoint( 2, p2.x(), p2.y() );
	pa.setPoint( 3, p3.x(), p3.y() );

	QPointArray pa2( pa.cubicBezier() );

	m_pa.resize( m_index + pa2.size() );
	m_pa.putPoints( m_index, pa2.size(), pa2 );

	m_index += pa2.size();
}

void
VQPainter::fillPath()
{
	// we probably dont need filling for qpainter
	m_index = 0;
}

void
VQPainter::strokePath()
{
	m_painter->drawPolyline( m_pa, 0, m_index );
	m_index = 0;
}

void
VQPainter::drawRect( double x, double y, double w, double h )
{
	m_painter->drawRect( int(x), int(y), int(w), int(h) );
}

void
VQPainter::setPen( const VStroke &stroke )
{
	QPen pen;

	// color + linewidth
	int r;
	int g;
	int b;

	stroke.color().pseudoValues( r, g, b );
	pen.setColor( QColor( r, g, b ) );
	pen.setWidth( stroke.lineWidth() );

	// caps
	if( stroke.lineCap() == VStroke::cap_butt )
		pen.setCapStyle( Qt::FlatCap );
	else if( stroke.lineCap() == VStroke::cap_round )
		pen.setCapStyle( Qt::RoundCap );
	else if( stroke.lineCap() == VStroke::cap_square )
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

