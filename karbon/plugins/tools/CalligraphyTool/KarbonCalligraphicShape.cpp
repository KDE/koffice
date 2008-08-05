/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KarbonCalligraphicShape.h"

#include <KoPathPoint.h>

#include "KarbonSimplifyPath.h"
#include <KarbonCurveFit.h>
#include <KoColorBackground.h>

#include <KDebug>

#include <cmath>
#include <cstdlib>

#undef M_PI
const double M_PI = 3.1415927;

KarbonCalligraphicShape::KarbonCalligraphicShape( double caps )
    : m_caps( caps )
{
    setShapeId( KoPathShapeId );
    setFillRule( Qt::WindingFill );
    setBackground( new KoColorBackground( Qt::black ) );
    setBorder( 0 );
}

KarbonCalligraphicShape::~KarbonCalligraphicShape()
{
}

void KarbonCalligraphicShape::appendPoint( const QPointF &point,
                                           double angle,
                                           double width )
{
    // convert the point from canvas to shape coordinates
    QPointF p = point - position();
    KarbonCalligraphicPoint *calligraphicPoint =
            new KarbonCalligraphicPoint( p, angle, width );

    m_handles.append( p );
    m_points.append(calligraphicPoint);
    appendPointToPath(*calligraphicPoint);
}

void KarbonCalligraphicShape::
        appendPointToPath( const KarbonCalligraphicPoint &p )
{
    double dx = std::cos( p.angle() ) * p.width();
    double dy = std::sin( p.angle() ) * p.width();

    // find the outline points
    QPointF p1 = p.point() - QPointF( dx/2, dy/2 );
    QPointF p2 = p.point() + QPointF( dx/2, dy/2 );

    if ( pointCount() == 0 ) 
    {
        moveTo( p1 );
        lineTo( p2 );
        normalize();
        return;
    }
    // pointCount > 0

    bool flip = (pointCount() >= 2) ? flipDetected(p1, p2) : false;

    // if there was a flip add additional points
    if ( flip ) {
        appendPointsToPathAux( p2, p1 );
        if ( pointCount() > 4 )
            smoothLastPoints();
    }

    appendPointsToPathAux( p1, p2 );

    if ( pointCount() > 4 )
    {
        smoothLastPoints();

        if ( flip )
        {
            int index = pointCount() / 2;
            // find the last two points
            KoPathPoint *last1 = pointByIndex( KoPathPointIndex(0, index-1) );
            KoPathPoint *last2 = pointByIndex( KoPathPointIndex(0, index) );

            last1->removeControlPoint1();
            last1->removeControlPoint2();
            last2->removeControlPoint1();
            last2->removeControlPoint2();
            m_lastWasFlip = true;
        }

        if ( m_lastWasFlip )
        {
            int index = pointCount() / 2;
            // find the previous two points
            KoPathPoint *prev1 = pointByIndex( KoPathPointIndex(0, index-2) );
            KoPathPoint *prev2 = pointByIndex( KoPathPointIndex(0, index+1) );

            prev1->removeControlPoint1();
            prev1->removeControlPoint2();
            prev2->removeControlPoint1();
            prev2->removeControlPoint2();

            if ( ! flip )
                m_lastWasFlip = false;
        }
    }
    normalize();
}

void KarbonCalligraphicShape::appendPointsToPathAux( const QPointF &p1,
                                                     const QPointF &p2 )
{
    KoPathPoint *pathPoint1 = new KoPathPoint(this, p1);
    KoPathPoint *pathPoint2 = new KoPathPoint(this, p2);

    // calculate the index of the insertion position
    int index = pointCount() / 2;

    insertPoint( pathPoint2, KoPathPointIndex(0, index) );
    insertPoint( pathPoint1, KoPathPointIndex(0, index) );
}

void KarbonCalligraphicShape::smoothLastPoints()
{
    int index = pointCount() / 2;
    smoothPoint( index - 2 );
    smoothPoint( index + 1 );
}

void KarbonCalligraphicShape::smoothPoint( const int index )
{
    if ( pointCount() < index + 2 )
    {
        kDebug() << "index to high";
        return;
    }
    else if ( index < 1 )
    {
        kDebug() << "index to low";
        return;
    }

    const KoPathPointIndex PREV( 0, index-1 );
    const KoPathPointIndex INDEX( 0, index );
    const KoPathPointIndex NEXT( 0, index+1 );

    QPointF prev = pointByIndex( PREV )->point();
    QPointF point = pointByIndex( INDEX )->point();
    QPointF next = pointByIndex( NEXT )->point();

    QPointF vector = next - prev;
    double dist = ( QLineF( prev, next ) ).length();
    // normalize the vector (make it's size equal to 1)
    if ( ! qFuzzyCompare(dist + 1, 1) )
        vector /= dist;
    double mult = 0.35; // found by trial and error, might not be perfect...
    // distance of the control points from the point
    double dist1 = ( QLineF( point, prev ) ).length() * mult;
    double dist2 = ( QLineF( point, next ) ).length() * mult;
    QPointF vector1 = vector * dist1;
    QPointF vector2 = vector * dist2;
    QPointF controlPoint1 = point - vector1;
    QPointF controlPoint2 = point + vector2;

    pointByIndex( INDEX )->setControlPoint1( controlPoint1 );
    pointByIndex( INDEX )->setControlPoint2( controlPoint2 );
}


const QRectF KarbonCalligraphicShape::lastPieceBoundingRect()
{
    if ( pointCount() < 6 )
        return QRectF();

    int index = pointCount() / 2;

    QPointF p1 = pointByIndex( KoPathPointIndex(0, index-3) )->point();
    QPointF p2 = pointByIndex( KoPathPointIndex(0, index-2) )->point();
    QPointF p3 = pointByIndex( KoPathPointIndex(0, index-1) )->point();
    QPointF p4 = pointByIndex( KoPathPointIndex(0, index  ) )->point();
    QPointF p5 = pointByIndex( KoPathPointIndex(0, index+1) )->point();
    QPointF p6 = pointByIndex( KoPathPointIndex(0, index+2) )->point();

    // TODO: also take the control points into account
    QPainterPath p;
    p.moveTo( p1 );
    p.lineTo( p2 );
    p.lineTo( p3 );
    p.lineTo( p4 );
    p.lineTo( p5 );
    p.lineTo( p6 );

    return p.boundingRect().translated( position() );
}


bool KarbonCalligraphicShape::flipDetected( const QPointF &p1, const QPointF &p2 )
{
    // detect the flip caused by the angle changing 180 degrees
    // thus detect the boundary crossing
    int index = pointCount() / 2;
    QPointF last1 = pointByIndex( KoPathPointIndex(0, index-1) )->point();
    QPointF last2 = pointByIndex( KoPathPointIndex(0, index) )->point();

    int sum1 = std::abs( ccw(p1, p2, last1) + ccw(p1, last2, last1) );
    int sum2 = std::abs( ccw(p2, p1, last2) + ccw(p2, last1, last2) );
    // if there was a flip
    return sum1 < 2 && sum2 < 2;
}

int KarbonCalligraphicShape::ccw( const QPointF &p1,
                                 const QPointF &p2,
                                 const QPointF &p3 )
{
    // calculate two times the area of the triangle fomed by the points given
    double area2 = ( p2.x() - p1.x() ) * ( p3.y() - p1.y() ) -
                   ( p2.y() - p1.y() ) * ( p3.x() - p1.x() );
    if ( area2 > 0 ) 
    {
        return +1; // the points are given in counterclockwise order
    }
    else if ( area2 < 0 ) 
    {
        return -1; // the points are given in clockwise order
    }
    else 
    {
        return 0; // the points form a degenerate triangle
    }
}



void KarbonCalligraphicShape::setSize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    // TODO: check
    KoParameterShape::setSize( newSize );
}

QPointF KarbonCalligraphicShape::normalize()
{
    QPointF offset( KoParameterShape::normalize() );
    QMatrix matrix;
    matrix.translate( -offset.x(), -offset.y() );

    for( int i = 0; i < m_handles.size(); ++i )
    {
        m_points[i]->setPoint( matrix.map( m_points[i]->point() ) );
    }

    return offset;
}

void KarbonCalligraphicShape::moveHandleAction( int handleId,
                                                const QPointF & point,
                                               Qt::KeyboardModifiers modifiers )
{
    Q_UNUSED(modifiers);
    m_points[handleId]->setPoint( point );
}

void KarbonCalligraphicShape::updatePath( const QSizeF &size )
{
    kDebug() << "updatePath";
    Q_UNUSED(size);

    QPointF pos = position();

    // remove all points
    clear();
    setPosition( QPoint(0, 0) );

    foreach(KarbonCalligraphicPoint *p, m_points)
        appendPointToPath(*p);

    simplifyPath();

    for (int i = 0; i < m_points.size(); ++i)
        m_handles[i] = m_points[i]->point();

    setPosition( pos );
}

void KarbonCalligraphicShape::simplifyPath()
{
    if ( m_points.count() < 2 )
        return;

    close();

    // add final cap
    addCap( m_points.count()-2, m_points.count()-1, pointCount()/2 );

    // add initial cap
    addCap( 1, 0, pointCount(), true );

    // TODO: the error should be proportional to the width
    //       and it shouldn't be a magic number
    //karbonSimplifyPath( this, 0.3 );
}

void KarbonCalligraphicShape::addCap( int index1, int index2, int pointIndex,
                                      bool inverted )
{
    QPointF p1 = m_points[index1]->point();
    QPointF p2 = m_points[index2]->point();
    double width = m_points[index2]->width();

    QPointF direction = QLineF( QPointF(0, 0), p2 - p1 ).unitVector().p2();
    QPointF p = p2 + direction * m_caps * width;

    KoPathPoint * newPoint = new KoPathPoint(this, p);
    
    double angle = m_points[index2]->angle();
    if ( inverted )
        angle += M_PI;
    
    double dx = std::cos( angle ) * width;
    double dy = std::sin( angle ) * width;
    newPoint->setControlPoint1( QPointF(p.x()-dx/2, p.y()-dy/2) );
    newPoint->setControlPoint2( QPointF(p.x()+dx/2, p.y()+dy/2) );

    insertPoint( newPoint, KoPathPointIndex(0, pointIndex) );
}

QString KarbonCalligraphicShape::pathShapeId() const
{
    return KarbonCalligraphicShapeId;
}
