/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008 Inge Wallin     <inge@lysator.liu.se>

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

#include "CellRegion.h"

#include <kdebug.h>

#include <QPoint>
#include <QRect>
#include <QVector>
#include <QDebug>
#include <QStringList>

#include <cmath>

using std::pow;
using namespace KChart;


CellRegion::CellRegion()
{
}

CellRegion::CellRegion( const QPoint &point )
{
    add( point );
}

CellRegion::CellRegion( const QRect &rect )
{
    add( rect );
}

CellRegion::CellRegion( const QPoint &point, const QSize &size )
{
    add( QRect( point, size ) );
}

CellRegion::CellRegion( const QVector<QRect> &rects )
{
    add( rects );
}

CellRegion::~CellRegion()
{
}

QVector<QRect> CellRegion::rects() const
{
    return m_rects;
}

int CellRegion::rectCount() const
{
    return m_rects.size();
}

bool CellRegion::isValid() const
{
    return m_rects.size() > 0;
}

bool CellRegion::contains( const QPoint &point, bool proper ) const
{
    foreach ( const QRect &rect, m_rects ) {
        if ( rect.contains( point, proper ) )
            return true;
    }
    
    return false;
}

bool CellRegion::contains( const QRect &rect, bool proper ) const
{
    foreach ( const QRect &r, m_rects ) {
        if ( r.contains( rect, proper ) )
            return true;
    }
    
    return false;
}

bool CellRegion::intersects( const QRect &rect ) const
{
    foreach ( const QRect &r, m_rects ) {
        if ( r.intersects( rect ) )
            return true;
    }
    
    return false;
}

CellRegion CellRegion::intersected( const QRect &rect ) const
{
    CellRegion intersections;
    
    foreach ( const QRect &r, m_rects ) {
        if ( r.intersects( rect ) )
            intersections.add( r.intersected( rect ) );
    }
    
    return intersections;
}

Qt::Orientation CellRegion::orientation() const
{
    foreach ( const QRect &rect, m_rects ) {
    	if ( rect.width() > 1 )
    		return Qt::Horizontal;
    	if ( rect.height() > 1 )
    		return Qt::Vertical;
    }
    
    // Default if region is only one cell
    return Qt::Vertical;
}

int CellRegion::cellCount() const
{
    int count = 0;
    if ( orientation() == Qt::Horizontal ) {
        foreach ( const QRect &rect, m_rects )
            count += rect.width();
    }
    else {
        foreach( const QRect &rect, m_rects )
            count += rect.height();
    }
    
    return count;
}

void CellRegion::add( const CellRegion &other )
{
    add( other.rects() );
}

void CellRegion::add( const QPoint &point )
{
    add( QRect( point, QSize( 1, 1 ) ) );
}

void CellRegion::add( const QRect &rect )
{
    if ( !rect.isValid() ) {
        qWarning() << "CellRegion::add() Attempt to add invalid rectangle";
        qWarning() << "CellRegion::add():" << rect;
        return;
    }
    
    if ( rect.width() > 1 && rect.height() > 1 ) {
        qWarning() << "CellRegion::add() Attempt to add rectangle with height AND width > 1";
        qWarning() << "CellRegion::add():" << rect;
        return;
    }
    
    m_rects.append( rect );
    m_boundingRect |= rect;
}

void CellRegion::add( const QVector<QRect> &rects )
{
    foreach ( const QRect &rect, rects )
        add( rect );
}

void CellRegion::subtract( const QPoint &point )
{
    if ( orientation() == Qt::Horizontal ) {
        for ( int i = 0; i < m_rects.size(); i++ ) {
            if ( m_rects[ i ].contains( point ) ) {

                if ( m_rects[ i ].topLeft().x() == point.x() ) {
                    m_rects[ i ].translate( 1, 0 );
                    m_rects[ i ].setWidth( m_rects[ i ].width() - 1 );
                }
                else if ( m_rects[ i ].topRight().x() == point.x() ) {
                    m_rects[ i ].setWidth( m_rects[ i ].width() - 1 );
                }

                return;
            }
        }
    }
    else {
        for ( int i = 0; i < m_rects.size(); i++ ) {
            if ( m_rects[ i ].contains( point ) ) {

                if ( m_rects[ i ].topLeft().y() == point.y() ) {
                    m_rects[ i ].translate( 0, 1 );
                    m_rects[ i ].setHeight( m_rects[ i ].height() - 1 );
                }
                else if ( m_rects[ i ].bottomLeft().y() == point.y() ) {
                    m_rects[ i ].setHeight( m_rects[ i ].height() - 1 );
                }

                return;
            }
        }
    }

    // Recalculate bounding rectangle
    m_boundingRect = QRect();
    foreach ( const QRect &rect, m_rects )
        m_boundingRect |= rect;
}

QRect CellRegion::boundingRect() const
{
    return m_boundingRect;
}

QPoint CellRegion::pointAtIndex( int index ) const
{
    // sum of all previous rectangle indices
    int i = 0;
    
    foreach ( const QRect &rect, m_rects ) {
        // Rectangle is horizontal
        if ( rect.width() > 1 ) {
            // Found it!
            // Index refers to point in current rectangle
            if ( i + rect.width() > index ) {
                // Local index of point in this rectangle
                int j = index - i;
                return QPoint( rect.x() + j, rect.y() );
            }

            // add number of indices in current rectangle to total index count
            i += rect.width();
        }
        else {
            // Found it!
            // Index refers to point in current rectangle
            if ( i + rect.height() > index ) {
                // Local index of point in this rectangle
                int j = index - i;
                return QPoint( rect.x(), rect.y() + j );
            }

            // add number of indices in current rectangle to total index count
            i += rect.height();
        }
    }
    
    // Invalid index!
    return QPoint( -1, -1 );
}

int CellRegion::indexAtPoint( const QPoint &point ) const
{
    int indicesLeftToPoint = 0;
    bool found = false;
    
    foreach ( const QRect &rect, m_rects ) {
        if ( !rect.contains( point ) ) {
            indicesLeftToPoint += rect.width() > 1 ? rect.width() : rect.height();
            continue;
        }

        found = true;
        if ( rect.width() > 1 )
            indicesLeftToPoint += point.x() - rect.topLeft().x();
        else
            indicesLeftToPoint += point.y() - rect.topLeft().y();
    }
    
    return found ? indicesLeftToPoint : -1;
}

static int rangeCharToInt( char c )
{
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;
}

static int rangeStringToInt( const QString &string )
{
    int result = 0;
    const int size = string.size();
    for ( int i = 0; i < size; i++ ) {
        kDebug(350001) << "---" << float( rangeCharToInt( string[i].toAscii() ) * pow( 10.0, ( size - i - 1 ) ) );
        result += rangeCharToInt( string[i].toAscii() ) * pow( 10.0, ( size - i - 1 ) );
    }
    kDebug(350001) << "+++++ result=" << result;
    return result;
}

static QString rangeIntToString( int i )
{
    QString tmp = QString::number( i );
    for( int j = 0; j < tmp.size(); j++ ) {
        tmp[j] = 'A' + tmp[j].toAscii() - '1';
    }

    kDebug(350001) << "tmp=" << tmp;
    return tmp;
}

// static
QVector<QRect> CellRegion::stringToRegion( const QString &string )
{
    if ( string.isEmpty() )
        return QVector<QRect>();
    
    const bool isPoint = !string.contains( ':' );
    kDebug(350001) << "CellRegion::stringToRegion():" << string;
    QString s = string;
    QStringList regionStrings = isPoint ? s.split( '.' ) : s.remove( ':' ).split( '.' );
    QPoint topLeftPoint;
    QPoint bottomRightPoint;

    if ( isPoint && regionStrings.size() < 2 || !isPoint && regionStrings.size() < 3 )
    {
        qWarning() << "1) CellRegion::stringToRegion(): Invalid region string \"" << string << "\"";
        return QVector<QRect>();
    }

    const QString tableName = regionStrings[0];

    const QString topLeft = regionStrings[1];
    QStringList l = topLeft.split( '$' );
    if ( l.size() < 3 ) {
        kDebug(350001) << topLeft;
        qWarning() << "2) CellRegion::stringToRegion(): Invalid region string \"" << string << "\"";
        return QVector<QRect>();
    }

    int column = rangeStringToInt( l[1] );
    int row = l[2].toInt() - 1;
    topLeftPoint = QPoint( column, row );

    if ( isPoint ) {
        kDebug(350001) << "Returning" << QVector<QRect>( 1, QRect( topLeftPoint, QSize( 1, 1 ) ) );
        return QVector<QRect>( 1, QRect( topLeftPoint, QSize( 1, 1 ) ) );
    }

    const QString bottomRight = regionStrings[2];
    l = bottomRight.split( '$' );
    if ( l.size() < 3 ) {
        qWarning() << "CellRegion::stringToRegion(): Invalid region string \"" << string << "\"";
        return QVector<QRect>();
    }
    column = rangeStringToInt( l[1] );
    row = l[2].toInt() - 1;
    bottomRightPoint = QPoint( column, row );

    kDebug(350001) << "///" << QRect( topLeftPoint, bottomRightPoint );

    return QVector<QRect>( 1, QRect( topLeftPoint, bottomRightPoint ) );
}

int CellRegion::rangeCharToInt( char c )     
{   
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 1) : -1;     
}   
     
int CellRegion::rangeStringToInt( const QString &string )   
{   
    int result = 0;     
    const int size = string.size();     
    for ( int i = 0; i < size; i++ ) {     
        result += rangeCharToInt( string[i].toAscii() ) * pow( 10.0, ( size - i - 1 ) );      
    } 

    return result;      
}   
     
QString CellRegion::rangeIntToString( int i )   
{   
    QString tmp = QString::number( i );     
    for( int j = 0; j < tmp.size(); j++ ) {   
        tmp[j] = 'A' + tmp[j].toAscii() - '1';      
    }

    return tmp;     
}

// Return the symbolic name of any column.
static QString columnName( uint column )
{
    if ( column < 1 || column > 32767 )
        return QString( "@@@" );

    QString   str;
    unsigned  digits = 1;
    unsigned  offset = 0;

    column--;

    for( unsigned limit = 26; column >= limit + offset; limit *= 26, ++digits )
        offset += limit;

    for( unsigned col = column - offset; digits; --digits, col /= 26 )
        str.prepend( QChar( 'A' + ( col % 26 ) ) );

    return str;
}

// static
QString CellRegion::regionToString( const QVector<QRect> &region )
{
    if ( region.isEmpty() )
        return QString();
    
    QString result;
    for ( int i = 0; i < region.count(); ++i ) {
        const QRect range = region[i];
        result.append( '$' );
        result.append( columnName( range.left() ) );
            result.append( '$' );
        result.append( QString::number( range.top() ) );
        if ( range.topLeft() != range.bottomRight() ) {
            result.append( ':' );
            result.append( '$' );
            result.append( columnName(range.right() ) );
            result.append( '$' );
            result.append( QString::number( range.bottom() ) );
        }
        if ( i < region.count() - 1 ) {
            result.append( ';' );
        }
    }
    return result;
}

bool CellRegion::operator == ( const CellRegion &other ) const
{
    return m_rects == other.rects();
}
