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

#include <math.h>

#include <qdom.h>

#include "vglobal.h"
#include "vpainter.h"
#include "vpath.h"
#include "vsegment.h"

#include <kdebug.h>


// Calculate height of p above line AB.
static double
height(
	const KoPoint& a,
	const KoPoint& p,
	const KoPoint& b )
{
	// Calculate determinant of AP and AB to obtain projection of vector AP to
	// the orthogonal vector of AB.
	const double det =
		p.x() * a.y() + b.x() * p.y() - p.x() * b.y() -
		a.x() * p.y() + a.x() * b.y() - b.x() * a.y();

	// Calculate norm = length(AB).
	const KoPoint ab = b - a;
	const double norm = sqrt( ab * ab );

	// If norm is very small, simply use distance AP.
	if( norm < VGlobal::verySmallNumber )
		return
			sqrt(
				( p.x() - a.x() ) * ( p.x() - a.x() ) +
				( p.y() - a.y() ) * ( p.y() - a.y() ) );

	// Normalize.
	return QABS( det ) / norm;
}


VSegment::VSegment( int degree )
{
	m_degree = degree;

	m_nodes = new VNodeData[ m_degree ];
	for( int i = 0; i < m_degree; ++i )
		selectPoint( i );

	m_type = begin;
	m_state = normal;

	m_prev = 0L;
	m_next = 0L;
}

VSegment::VSegment( const VSegment& segment )
{
	m_degree = segment.m_degree;

	m_nodes = new VNodeData[ m_degree ];

	m_type = segment.m_type;
	m_state = segment.m_state;

	// Copying the pointers m_prev/m_next has some advantages (see VSegment::length()).
	// Inserting a segment into a path overwrites these anyway.
	m_prev = segment.m_prev;
	m_next = segment.m_next;

	// Copy points.
	for( int i = 0; i < degree(); ++i )
	{
		setPoint( i, segment.point( i ) );
		selectPoint( i, segment.pointIsSelected( i ) );
	}
}

VSegment::~VSegment()
{
	delete[]( m_nodes );
}

void
VSegment::setDegree( int degree )
{
	// Do nothing if old and new degrees are identical.
	if( m_degree == degree )
		return;


	// Delete old node data.
	delete[]( m_nodes );

	// Allocate new node data.
	m_nodes = new VNodeData[ degree ];

	m_degree = degree;
}

void
VSegment::draw( VPainter* painter ) const
{
	if( state() == deleted )
		return;


	if( type() == curve )
	{
		painter->curveTo( point( 0 ), point( 1 ), point( 2 ) );
	}
	else if( type() == line )
	{
		painter->lineTo( knot() );
	}
	else
	{
		painter->moveTo( knot() );
	}
}

bool
VSegment::isFlat( double flatness ) const
{
	if(
		!prev() ||
		m_type == begin ||
		m_type == line )
	{
		return true;
	}

	if( m_type == curve )
	{
		bool flat = false;

		for( int i = 0; i < degree() - 1; ++i )
		{
			flat =
				height( prev()->knot(), point( i ), knot() ) / chordLength()
				< flatness;

			if( !flat )
				break;
		}

		return flat;
	}

	return false;
}

KoPoint
VSegment::pointAt( double t ) const
{
	KoPoint p;

	pointDerivativesAt( t, &p );

	return p;
}

void
VSegment::pointDerivativesAt( double t, KoPoint* p,
							  KoPoint* d1, KoPoint* d2 ) const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return;
	}


	// Lines.
	if( m_type == line )
	{
		const KoPoint diff = knot() - prev()->knot();

		if( p )
			*p = prev()->knot() + diff * t;

		if( d1 )
			*d1 = diff;

		if( d2 )
			*d2 = KoPoint( 0.0, 0.0 );

		return;
	}


	// Beziers.

	// Copy points.
	KoPoint* q = new KoPoint[ degree() + 1 ];

	q[ 0 ] = prev()->knot();

	for( int i = 0; i < degree(); ++i )
	{
		q[ i + 1 ] = point( i );
	}


	// The De Casteljau algorithm.
	for( int j = 1; j <= degree(); ++j )
	{
		for( int i = 0; i <= degree() - j; ++i )
		{
			q[ i ] = ( 1.0 - t ) * q[ i ] + t * q[ i + 1 ];
		}

		// Save second derivative now that we have it.
		if( j == degree() - 2 )
		{
			if( d2 )
				*d2 = degree() * ( degree() - 1 )
					  * ( q[ 2 ] - 2 * q[ 1 ] + q[ 0 ] );
		}

		// Save first derivative now that we have it.
		else if( j == degree() - 1 )
		{
			if( d1 )
				*d1 = degree() * ( q[ 1 ] - q[ 0 ] );
		}
	}

	// Save point.
	if( p )
		*p = q[ 0 ];

	delete[]( q );


	return;
}

KoPoint
VSegment::tangentAt( double t ) const
{
	KoPoint tangent;

	pointTangentNormalAt( t, 0L, &tangent );

	return tangent;
}

void
VSegment::pointTangentNormalAt( double t, KoPoint* p,
								KoPoint* tn, KoPoint* n ) const
{
	// Calculate derivative if necessary.
	KoPoint d;

	pointDerivativesAt( t, p, tn || n ? &d : 0L );


	// Normalize derivative.
	if( tn || n )
	{
		const double norm =
			sqrt( d.x() * d.x() + d.y() * d.y() );

		d = norm ? d * ( 1.0 / norm ) : KoPoint( 0.0, 0.0 );
	}

	// Assign tangent vector.
	if( tn )
		*tn = d;

	// Calculate normal vector.
	if( n )
	{
		// Calculate vector product of "binormal" x tangent
		// (0,0,1) x (dx,dy,0), which is simply (dy,-dx,0).
		n->setX( d.y() );
		n->setY( -d.x() );
	}
}

double
VSegment::length( double t ) const
{
	if(
		!prev() ||
		m_type == begin ||
		t == 0.0 )
	{
		return 0.0;
	}


	// Length of a line.
	if( m_type == line )
	{
		return
			t * chordLength();
	}

	// Length of a bezier.
	else if( m_type == curve )
	{
		/* The idea for this algortihm is by Jens Gravesen <gravesen@mat.dth.dk>.
		 * We calculate the chord length "chord"=|P0P3| and the length of the control point
		 * polygon "poly"=|P0P1|+|P1P2|+|P2P3|. The approximation for the bezier length is
		 * 0.5 * poly + 0.5 * chord. "poly - chord" is a measure for the error.
		 * We subdivide each segment until the error is smaller than a given tolerance
		 * and add up the subresults.
		 */

		// "Copy segment" splitted at t into a path.
		VPath path( 0L );
		path.moveTo( prev()->knot() );

		// Optimize a bit: most of the time we'll need the
		// length of the whole segment.
		if( t == 1.0 )
			path.append( this->clone() );
		else
		{
			VSegment* copy = this->clone();
			path.append( copy->splitAt( t ) );
			delete copy;
		}


		double chord;
		double poly;

		double length = 0.0;

		while( path.current() )
		{
			chord = path.current()->chordLength();
			poly = path.current()->polyLength();

			if(
				poly &&
				( poly - chord ) / poly > VGlobal::lengthTolerance )
			{
				// Split at midpoint.
				path.insert(
					path.current()->splitAt( 0.5 ) );
			}
			else
			{
				length += 0.5 * poly + 0.5 * chord;
				path.next();
			}
		}

		return length;
	}
	else
		return 0.0;
}

double
VSegment::chordLength() const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return 0.0;
	}


	KoPoint d = knot() - prev()->knot();

	return sqrt( d * d );
}

double
VSegment::polyLength() const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return 0.0;
	}


	// Start with distance |first point - previous knot|.
	KoPoint d = point( 0 ) - prev()->knot();

	double length = sqrt( d * d );

	// Iterate over remaining points.
	for( int i = 1; i < degree(); ++i )
	{
		d = point( i ) - point( i - 1 );
		length += sqrt( d * d );
	}


	return length;
}

double
VSegment::lengthParam( double len ) const
{
	if(
		len == 0.0 ||		// We divide by len below.
		!prev() ||
		m_type == begin )
	{
		return 0.0;
	}


	// Line.
	if( m_type == line )
	{
		return
			len / chordLength();
	}
	// Bezier.
	else if( m_type == curve )
	{
		// Perform a successive interval bisection.
		double param1 = 0.0;
		double paramMid = 0.5;
		double param2 = 1.0;

		double lengthMid = length( paramMid );

		while( QABS( lengthMid - len ) / len > VGlobal::paramLengthTolerance )
		{
			if( lengthMid < len )
				param1 = paramMid;
			else
				param2 = paramMid;

			paramMid = 0.5 * ( param2 + param1 );

			lengthMid = length( paramMid );
		}

		return paramMid;
	}

	return 0.0;
}

double
VSegment::nearestPointParam( const KoPoint& p ) const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return 1.0;
	}


	/* This function solves the "nearest point on curve" problem. That means, it
	 * calculates the point q (to be precise: it's paramter t) on this segment, which
	 * is located nearest to the input point p.
	 * The basic idea is best described (because it is freely available) in "Phoenix:
	 * An Interactive Curve Design System Based on the Automatic Fitting of
	 * Hand-Sketched Curves", Philip J. Schneider (master thesis, university of
	 * washington).
	 *
	 * For the nearest point q = C(t) on this segment, the first derivative is
	 * orthogonal to the distance vector "C(t) - p". In other words we are looking for
	 * solutions of f(t) = ( C(t) - p ) * C'(t) = 0.
	 * ( C(t) - p ) is a nth degree curve, C'(t) a n-1th degree curve => f(t) is a
	 * (2n - 1)th degree curve and thus has up to 2n - 1 distinct solutions.
	 * To solve it, we apply the Newton Raphson iteration: a parameter approximation t_i leads
	 * to the next approximation t_{i+1}
	 *
	 *                  f(t_i)
	 * t_{i+1} = t_i - -------
	 *                 f'(t_i)
	 *
	 * Convergence criteria are then
	 *
	 * 1) Point coincidence: | C(t_i) - p | <= tolerance1
	 *
	 * 2) Zero cosine: | C'(t_i) * ( C(t_i) - p ) |
	 *                 ---------------------------- <= tolerance2
	 *                 | C'(t_i) | * | C(t_i) - p |
	 *
	 * But first we need to find a first guess t_0 to start with. The solution for this
	 * problem is called "Approximate Inversion Method". Let's write f(t) explicitely:
	 *
	 *         n                     n-1
	 * f(t) = SUM c_i * B^n_i(t)  *  SUM d_j * B^{n-1}_j(t)
	 *        i=0                    j=0
	 *
	 *         n  n-1
	 *      = SUM SUM w_{ij} * B^{2n-1}_{i+j}(t)
	 *        i=0 j=0
	 *
	 * with w_{ij} = c_i * d_j * z_{ij} and
	 *
	 *          BinomialCoeff( n, i ) * BinomialCoeff( n - i ,j )
	 * z_{ij} = -----------------------------------------------
	 *                   BinomialCoeff( 2n - 1, i + j )
	 *
	 * This Bernstein-Bezier polynom representation can now be solved for it's roots.
	 */


	// Calculate the c_i = point( i ) - p.
	KoPoint* c = new KoPoint[ degree() + 1 ];

	c[ 0 ] = prev()->knot() - p;

	for( int i = 0; i < degree(); ++i )
	{
		c[ i + 1 ] = point( i ) - p;
	}


	// Calculate the d_j = point( j + 1 ) - point( j ).
	KoPoint* d = new KoPoint[ degree() ];

	d[ 0 ] = point( 0 ) - prev()->knot();

	for( int j = 0; j < degree() - 1; ++j )
	{
		d[ j + 1 ] = point( j + 1 ) - point( j );
	}


	// Calculate the z_{ij}.
	double* z = new double[ degree() * ( degree() + 1 ) ];

	for( int j = 0; j < degree(); ++j )
	{
		for( int i = 0; i <= degree(); ++i )
		{
			z[ j * ( degree() + 1 ) + i ] =
				VGlobal::binomialCoeff( degree(), i ) *
				VGlobal::binomialCoeff( degree() - i, j ) /
				VGlobal::binomialCoeff( 2 * degree() - 1, i + j );
		}
	}


	// Calculate the dot products of c_i and d_i.
	double* products = new double[ degree() * ( degree() + 1 ) ];

	for( int j = 0; j < degree(); ++j )
	{
		for( int i = 0; i <= degree(); ++i )
		{
			products[ j * ( degree() + 1 ) + i ] =
				d[ j ] * c[ i ];
		}
	}

	// We don't need the c_i and d_i anymore.
	delete[]( d );
	delete[]( c );


	// Calculate the control points of the new 2n-1th degree curve.
	VPath newCurve( 0L );

	// Set up control points in the ( u, f(u) )-plane.
	newCurve.append( new VSegment( 2 * degree() ) );

	for( int u = 1; u <= 2 * degree(); ++u )
	{
		newCurve.current()->setPoint(
			u - 1,
			KoPoint(
				static_cast<double>( u ) / static_cast<double>( 2 * degree() ),
				0.0 ) );
	}

	// Set f(u)-values.
	for( int k = 0; k < 2 * degree() - 1; ++k )
	{
		int min = QMIN( k, degree() );

		for(
			int i = QMAX( 0, k - degree() + 1 );
			i <= min;
			++i )
		{
			int j = k - i;

			newCurve.getLast()->m_nodes[ i + j  - 1 ].m_vector.setY(
				newCurve.getLast()->m_nodes[ i + j  - 1 ].m_vector.y() +
					products[ j * ( degree() + 1 ) + i ] *
					z[ j * ( degree() + 1 ) + i ] );
		}
	}

	// We don't need the c_i/d_i dot products and the z_{ij} anymore.
	delete[]( products );
	delete[]( z );


	// Find roots.
	QValueList<double> params;

	newCurve.current()->roots( params );


// TODO
	return 0.0;
}

void
VSegment::roots( QValueList<double>& params ) const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return;
	}


	// Evaluate the number of crossing the y=0 axis (sign changes)
	// which is >= number of roots.
	switch( signChanges() )
	{
		// No solutions.
		case 0:
			return;
		// Exactly one solution.
		case 1:
			if( isFlat() )
			{
				// TODO
				return;
			}
			// TODO
			break;
	}

	// Many solutions. Do recursive subdivision.
	QValueList<double> params1;
	QValueList<double> params2;

	// TODO

	// Add params of the sub segments (if any).
	params += params1;
	params += params2;
}

int
VSegment::signChanges() const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return 0;
	}


	int changes = 0;

	int sign = VGlobal::sign( prev()->knot().y() );
	int oldSign;

	// Check how many times the control polygon crosses the
	// y=0 axis.
	for( int i = 1; i <= degree(); ++i )
	{
		oldSign = sign;
		sign = VGlobal::sign( point( i - 1 ).y() );

		if( sign != oldSign )
		{
			++changes;
		}
	}

	return changes;
}

bool
VSegment::isSmooth( const VSegment& next ) const
{
	// Return false if this segment is a "begin".
	if( type() == begin )
		return false;


	// Calculate tangents.
	KoPoint t1;
	KoPoint t2;

	pointTangentNormalAt( 1.0, 0L, &t1 );

	next.pointTangentNormalAt( 0.0, 0L, &t2 );


	// Dot product.
	if( t1 * t2 >= VGlobal::parallelTolerance )
		return true;

	return false;
}

KoRect
VSegment::boundingBox() const
{
	// Initialize with knot.
	KoRect rect( knot(), knot() );


	// Add p0, if it exists.
	if( prev() )
	{
		if( prev()->knot().x() < rect.left() )
			rect.setLeft( prev()->knot().x() );

		if( prev()->knot().x() > rect.right() )
			rect.setRight( prev()->knot().x() );

		if( prev()->knot().y() < rect.top() )
			rect.setTop( prev()->knot().y() );

		if( prev()->knot().y() > rect.bottom() )
			rect.setBottom( prev()->knot().y() );
	}


	for( int i = 0; i < degree() - 1; ++i )
	{
		if( point( i ).x() < rect.left() )
			rect.setLeft( point( i ).x() );

		if( point( i ).x() > rect.right() )
			rect.setRight( point( i ).x() );

		if( point( i ).y() < rect.top() )
			rect.setTop( point( i ).y() );

		if( point( i ).y() > rect.bottom() )
			rect.setBottom( point( i ).y() );
	}


	return rect;
}

VSegment*
VSegment::splitAt( double t )
{
	if(
		!prev() ||
		m_type == begin )
	{
		return 0L;
	}


	// Create new segment.
	VSegment* segment = new VSegment( m_degree );

	// Set segment type.
	segment->m_state = m_state;


	// Lines are easy: no need to modify the current segment.
	if( m_type == line )
	{
		segment->setKnot(
			prev()->knot() +
			( knot() - prev()->knot() ) * t );

		segment->m_type = line;

		return segment;
	}


	// Beziers.

	// Set segment type.
	segment->m_type = curve;


	// Copy points.
	KoPoint* q = new KoPoint[ degree() + 1 ];

	q[ 0 ] = prev()->knot();

	for( int i = 0; i < degree(); ++i )
	{
		q[ i + 1 ] = point( i );
	}


	// The De Casteljau algorithm.
	for( int j = 1; j <= degree(); ++j )
	{
		for( int i = 0; i <= degree() - j; ++i )
		{
			q[ i ] = ( 1.0 - t ) * q[ i ] + t * q[ i + 1 ];
		}

		// Modify the new segment.
		segment->setPoint( j - 1, q[ 0 ] );
	}

	// Modify the current segment (no need to modify the knot though).
	for( int i = 1; i < degree(); ++i )
	{
		setPoint( i - 1, q[ i ] );
	}


	delete[]( q );


	return segment;
}
bool
VSegment::intersects( const KoPoint& a0, const KoPoint& a1 ) const
{
	return linesIntersect( a0, a1, point( 0 ), next() ? next()->point( 0 ) : knot() );
}

bool
VSegment::linesIntersect(
	const KoPoint& a0,
	const KoPoint& a1,
	const KoPoint& b0,
	const KoPoint& b1 )
{
	const KoPoint delta_a = a1 - a0;
	const double det_a = a1.x() * a0.y() - a1.y() * a0.x();

	const double r_b0 = delta_a.y() * b0.x() - delta_a.x() * b0.y() + det_a;
	const double r_b1 = delta_a.y() * b1.x() - delta_a.x() * b1.y() + det_a;

	if( r_b0 != 0.0 && r_b1 != 0.0 && r_b0 * r_b1 > 0.0 )
		return false;

	const KoPoint delta_b = b1 - b0;

	const double det_b = b1.x() * b0.y() - b1.y() * b0.x();

	const double r_a0 = delta_b.y() * a0.x() - delta_b.x() * a0.y() + det_b;
	const double r_a1 = delta_b.y() * a1.x() - delta_b.x() * a1.y() + det_b;

	if( r_a0 != 0.0 && r_a1 != 0.0 && r_a0 * r_a1 > 0.0 )
		return false;

	return true;
}

// TODO: Move this function into "userland"
uint
VSegment::nodeNear( const KoPoint& p, double isNearRange ) const
{
	int index = 0;

	for( int i = 0; i < degree(); ++i )
	{
		if( point( 0 ).isNear( p, isNearRange ) )
		{
			index = i + 1;
			break;
		}
	}

	return index;
}

VSegment*
VSegment::revert() const
{
	if(
		!prev() ||
		m_type == begin )
	{
		return 0L;
	}


	// Create new segment.
	VSegment* segment = new VSegment( m_degree );

	segment->m_type = m_type;
	segment->m_state = m_state;


	// Swap points.
	for( int i = 0; i < degree() - 1; ++i )
	{
		segment->setPoint( i, point( degree() - 2 - i ) );
	}

	segment->setKnot( prev()->knot() );


	// TODO swap node attributes (selected)

	return segment;
}

VSegment*
VSegment::prev() const
{
	VSegment* segment = m_prev;

	while( segment && segment->state() == deleted )
	{
		segment = segment->m_prev;
	}

	return segment;
}

VSegment*
VSegment::next() const
{
	VSegment* segment = m_next;

	while( segment && segment->state() == deleted )
	{
		segment = segment->m_next;
	}

	return segment;
}

void
VSegment::transform( const QWMatrix& m )
{
	for( int i = 0; i < degree(); ++i )
		if( pointIsSelected( i ) )
			setPoint( i, point( i ).transform( m ) );
}

void
VSegment::load( const QDomElement& element )
{
	// TOD save control point fixing?

	if( element.tagName() == "CURVE" )
	{
		m_type = curve;

		setDegree( 3 );

		setPoint(
			0,
			KoPoint(
				element.attribute( "x1" ).toDouble(),
				element.attribute( "y1" ).toDouble() ) );

		setPoint(
			1,
			KoPoint(
				element.attribute( "x2" ).toDouble(),
				element.attribute( "y2" ).toDouble() ) );

		setKnot(
			KoPoint(
				element.attribute( "x3" ).toDouble(),
				element.attribute( "y3" ).toDouble() ) );
	}
	else if( element.tagName() == "LINE" )
	{
		m_type = line;

		setDegree( 1 );

		setKnot(
			KoPoint(
				element.attribute( "x" ).toDouble(),
				element.attribute( "y" ).toDouble() ) );
	}
	else if( element.tagName() == "MOVE" )
	{
		m_type = begin;

		setDegree( 1 );

		setKnot(
			KoPoint(
				element.attribute( "x" ).toDouble(),
				element.attribute( "y" ).toDouble() ) );
	}
}

VSegment*
VSegment::clone() const
{
	return new VSegment( *this );
}

