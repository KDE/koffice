/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <qdom.h>
#include <math.h>

#include "vglobal.h"
#include "vsegment.h"
#include "vsegmentlist.h"
#include "vsegmenttools.h"

#include <kdebug.h>


// Calculate height of p above line AB.
static double
height(
	const KoPoint& a,
	const KoPoint& p,
	const KoPoint& b )
{
	// calculate determinant of AP and AB to obtain projection of vector AP to
	// the orthogonal vector of AB:
	const double det =
		p.x() * a.y() + b.x() * p.y() - p.x() * b.y() -
		a.x() * p.y() + a.x() * b.y() - b.x() * a.y();

	// calculate norm = length(AB):
	const double norm = sqrt(
		( b.x() - a.x() ) * ( b.x() - a.x() ) +
		( b.y() - a.y() ) * ( b.y() - a.y() ) );

	// if norm is very small, simply use distance AP:
	if( norm < 1.0e-6 )
		return
			sqrt(
				( p.x() - a.x() ) * ( p.x() - a.x() ) +
				( p.y() - a.y() ) * ( p.y() - a.y() ) );

	// normalize:
	return det / norm;
}


VSegmentListTraverser::VSegmentListTraverser()
	:  m_previousPoint( 0.0, 0.0 )
{
}

bool
VSegmentListTraverser::traverse( const VSegmentList& list )
{
	VSegmentListIterator itr( list );
	for( ; itr.current(); ++itr )
	{
		if( itr.current()->type() == segment_curve )
		{
			if( !curveTo(
				itr.current()->ctrlPoint1(),
				itr.current()->ctrlPoint2(),
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
		else if( itr.current()->type() == segment_line )
		{
			if( !lineTo(
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
		else if( itr.current()->type() == segment_curve1 )
		{
			if( !curve1To(
				itr.current()->ctrlPoint2(),
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
		else if( itr.current()->type() == segment_curve2 )
		{
			if( !curve2To(
				itr.current()->ctrlPoint1(),
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
		else if( itr.current()->type() == segment_begin )
		{
			if( !begin(
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
		else if( itr.current()->type() == segment_end )
		{
			if( !end(
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
		// fall back:
		else
		{
			if( !lineTo(
				itr.current()->knot2() ) )
			{
				return false;
			}
		}
	}

	return true;
}


VSegment::VSegment()
	: m_prev( 0L ), m_next( 0L )
{
	m_type = segment_begin;
	m_smooth = false;
}

VSegment::VSegment( const VSegment& segment )
	: m_prev( 0L ), m_next( 0L )
{
	m_point[0] = segment.m_point[0];
	m_point[1] = segment.m_point[1];
	m_point[2] = segment.m_point[2];
	m_type = segment.m_type;
	m_smooth = segment.m_smooth;
}

bool
VSegment::isFlat() const
{
	if(
		m_prev == 0L ||
		type() == segment_begin ||
		type() == segment_line ||
		type() == segment_end )
	{
		return true;
	}

	if( type() == segment_curve )
		return
			height( m_prev->m_point[2], m_point[0], m_point[2] )
				< VGlobal::flatnessTolerance &&
			height( m_prev->m_point[2], m_point[1], m_point[2] )
				< VGlobal::flatnessTolerance;
	else if( type() == segment_curve1 )
		return
			height( m_prev->m_point[2], m_point[1], m_point[2] )
				< VGlobal::flatnessTolerance;
	else if( type() == segment_curve2 )
		return
			height( m_prev->m_point[2], m_point[0], m_point[2] )
				< VGlobal::flatnessTolerance;

	return false;
}

KoRect
VSegment::boundingBox() const
{
	KoRect rect( m_point[2], m_point[2] );

	if( m_prev )
	{
		if( m_prev->m_point[2].x() < rect.left() )
			rect.setLeft( m_prev->m_point[2].x() );
		if( m_prev->m_point[2].x() > rect.right() )
			rect.setRight( m_prev->m_point[2].x() );
		if( m_prev->m_point[2].y() < rect.top() )
			rect.setTop( m_prev->m_point[2].y() );
		if( m_prev->m_point[2].y() > rect.bottom() )
			rect.setBottom( m_prev->m_point[2].y() );
	}

	if(
		type() == segment_curve1 ||
		type() == segment_curve )
	{
		if( m_point[1].x() < rect.left() )
			rect.setLeft( m_point[1].x() );
		if( m_point[1].x() > rect.right() )
			rect.setRight( m_point[1].x() );
		if( m_point[1].y() < rect.top() )
			rect.setTop( m_point[1].y() );
		if( m_point[1].y() > rect.bottom() )
			rect.setBottom( m_point[1].y() );
	}

	if(
		type() == segment_curve2 ||
		type() == segment_curve )
	{
		if( m_point[0].x() < rect.left() )
			rect.setLeft( m_point[0].x() );
		if( m_point[0].x() > rect.right() )
			rect.setRight( m_point[0].x() );
		if( m_point[0].y() < rect.top() )
			rect.setTop( m_point[0].y() );
		if( m_point[0].y() > rect.bottom() )
			rect.setBottom( m_point[0].y() );
	}

	return rect;
}

void
VSegment::save( QDomElement& element ) const
{
	QDomElement me;

	if( type() == segment_curve )
	{
		me = element.ownerDocument().createElement( "CURVE" );
		me.setAttribute( "x1", m_point[0].x() );
		me.setAttribute( "y1", m_point[0].y() );
		me.setAttribute( "x2", m_point[1].x() );
		me.setAttribute( "y2", m_point[1].y() );
		me.setAttribute( "x3", m_point[2].x() );
		me.setAttribute( "y3", m_point[2].y() );
	}
	else if( type() == segment_curve1 )
	{
		me = element.ownerDocument().createElement( "CURVE1" );
		me.setAttribute( "x2", m_point[1].x() );
		me.setAttribute( "y2", m_point[1].y() );
		me.setAttribute( "x3", m_point[2].x() );
		me.setAttribute( "y3", m_point[2].y() );
	}
	else if( type() == segment_curve2 )
	{
		me = element.ownerDocument().createElement( "CURVE2" );
		me.setAttribute( "x1", m_point[0].x() );
		me.setAttribute( "y1", m_point[0].y() );
		me.setAttribute( "x3", m_point[2].x() );
		me.setAttribute( "y3", m_point[2].y() );
	}
	else if( type() == segment_line )
	{
		me = element.ownerDocument().createElement( "LINE" );
		me.setAttribute( "x", m_point[2].x() );
		me.setAttribute( "y", m_point[2].y() );
	}
	else if( type() == segment_begin )
	{
		me = element.ownerDocument().createElement( "MOVE" );
		me.setAttribute( "x", m_point[2].x() );
		me.setAttribute( "y", m_point[2].y() );
	}
	else
		return;

	element.appendChild( me );

}

void
VSegment::load( const QDomElement& element )
{
	if( element.tagName() == "CURVE" )
	{
		m_type = segment_curve;
		m_point[0].setX( element.attribute( "x1" ).toDouble() );
		m_point[0].setY( element.attribute( "y1" ).toDouble() );
		m_point[1].setX( element.attribute( "x2" ).toDouble() );
		m_point[1].setY( element.attribute( "y2" ).toDouble() );
		m_point[2].setX( element.attribute( "x3" ).toDouble() );
		m_point[2].setY( element.attribute( "y3" ).toDouble() );
	}
	else if( element.tagName() == "CURVE1" )
	{
		m_type = segment_curve1;
		m_point[1].setX( element.attribute( "x2" ).toDouble() );
		m_point[1].setY( element.attribute( "y2" ).toDouble() );
		m_point[2].setX( element.attribute( "x3" ).toDouble() );
		m_point[2].setY( element.attribute( "y3" ).toDouble() );
	}
	else if( element.tagName() == "CURVE2" )
	{
		m_type = segment_curve2;
		m_point[0].setX( element.attribute( "x1" ).toDouble() );
		m_point[0].setY( element.attribute( "y1" ).toDouble() );
		m_point[2].setX( element.attribute( "x3" ).toDouble() );
		m_point[2].setY( element.attribute( "y3" ).toDouble() );
	}
	else if( element.tagName() == "LINE" )
	{
		m_type = segment_line;
		m_point[2].setX( element.attribute( "x" ).toDouble() );
		m_point[2].setY( element.attribute( "y" ).toDouble() );
	}
	else if( element.tagName() == "MOVE" )
	{
		m_type = segment_begin;
		m_point[2].setX( element.attribute( "x" ).toDouble() );
		m_point[2].setY( element.attribute( "y" ).toDouble() );
	}
}
