// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2001 Toshitaka Fujioka <fujioka@kde.org>

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

#include "kppolygonobject.h"
#include "kpgradient.h"
#include <kozoomhandler.h>
#include <kdebug.h>
#include <qbitmap.h>
#include <qregion.h>
#include <qdom.h>
#include <qpicture.h>
#include <qpainter.h>
#include "KPPolygonObjectIface.h"

#include <math.h>

using namespace std;

KPPolygonObject::KPPolygonObject()
    : KP2DObject()
{
    redrawPix = false;
}

KPPolygonObject::KPPolygonObject( const KoPointArray &_points, const KoSize &_size,
                                  const QPen &_pen, const QBrush &_brush,
                                  FillType _fillType, const QColor &_gColor1, const QColor &_gColor2, BCType _gType,
                                  bool _unbalanced, int _xfactor, int _yfactor,
                                  bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue )
    : KP2DObject( _pen, _brush, _fillType, _gColor1, _gColor2, _gType, _unbalanced, _xfactor, _yfactor )
{
    points = KoPointArray( _points );
    ext = _size;

    checkConcavePolygon = _checkConcavePolygon;
    cornersValue = _cornersValue;
    sharpnessValue = _sharpnessValue;

    redrawPix = false;

    if ( fillType == FT_GRADIENT ) {
        gradient = new KPGradient( gColor1, gColor2, gType, unbalanced, xfactor, yfactor );
        redrawPix = true;
    }
    else
        gradient = 0;
}

KPPolygonObject &KPPolygonObject::operator=( const KPPolygonObject & )
{
    return *this;
}

DCOPObject* KPPolygonObject::dcopObject()
{
    if ( !dcop )
        dcop = new KPPolygonObjectIface( this );
    return dcop;
}

QDomDocumentFragment KPPolygonObject::save( QDomDocument& doc, double offset )
{
    QDomDocumentFragment fragment = KP2DObject::save( doc, offset );

    QDomElement elemSettings = doc.createElement( "SETTINGS" );

    elemSettings.setAttribute( "checkConcavePolygon", static_cast<int>( checkConcavePolygon ) );
    elemSettings.setAttribute( "cornersValue", cornersValue );
    elemSettings.setAttribute( "sharpnessValue", sharpnessValue );

    fragment.appendChild( elemSettings );

    if ( !points.isNull() ) {
        QDomElement elemPoints = doc.createElement( "POINTS" );
        KoPointArray::ConstIterator it;
        for ( it = points.begin(); it != points.end(); ++it ) {
            QDomElement elemPoint = doc.createElement( "Point" );
            KoPoint point = (*it);
            elemPoint.setAttribute( "point_x", point.x() );
            elemPoint.setAttribute( "point_y", point.y() );

            elemPoints.appendChild( elemPoint );
        }
        fragment.appendChild( elemPoints );
    }

    return fragment;
}

double KPPolygonObject::load( const QDomElement &element )
{
    double offset=KP2DObject::load( element );

    QDomElement e = element.namedItem( "SETTINGS" ).toElement();
    if ( !e.isNull() ) {
        bool _checkConcavePolygon = false;
        int _cornersValue = 3;
        int _sharpnessValue = 0;

        if ( e.hasAttribute( "checkConcavePolygon" ) )
            _checkConcavePolygon = static_cast<bool>( e.attribute( "checkConcavePolygon" ).toInt() );
        if ( e.hasAttribute( "cornersValue" ) )
            _cornersValue = e.attribute( "cornersValue" ).toInt();
        if ( e.hasAttribute( "sharpnessValue" ) )
            _sharpnessValue = e.attribute( "sharpnessValue" ).toInt();

        checkConcavePolygon = _checkConcavePolygon;
        cornersValue = _cornersValue;
        sharpnessValue = _sharpnessValue;
    }

    e = element.namedItem( "POINTS" ).toElement();
    if ( !e.isNull() ) {
        QDomElement elemPoint = e.firstChild().toElement();
        unsigned int index = 0;
        while ( !elemPoint.isNull() ) {
            if ( elemPoint.tagName() == "Point" ) {
                double tmpX = 0;
                double tmpY = 0;
                if( elemPoint.hasAttribute( "point_x" ) )
                    tmpX = elemPoint.attribute( "point_x" ).toDouble();
                if( elemPoint.hasAttribute( "point_y" ) )
                    tmpY = elemPoint.attribute( "point_y" ).toDouble();

                points.putPoints( index, 1, tmpX,tmpY );
            }
            elemPoint = elemPoint.nextSibling().toElement();
            ++index;
        }
    }
    return offset;
}

void KPPolygonObject::setSize( double _width, double _height )
{
    KoSize origSize( ext );
    KPObject::setSize( _width, _height );

    double fx = ext.width() / origSize.width();
    double fy = ext.height() / origSize.height();

    updatePoints( fx, fy );
}

void KPPolygonObject::updatePoints( double _fx, double _fy )
{
    int index = 0;
    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() * _fx;
        double tmpY = point.y() * _fy;

        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }
    points = tmpPoints;
}

void KPPolygonObject::setFillType( FillType _fillType )
{
    fillType = _fillType;

    if ( fillType == FT_BRUSH && gradient ) {
        delete gradient;
        gradient = 0;
    }

    if ( fillType == FT_GRADIENT && !gradient ) {
        gradient = new KPGradient( gColor1, gColor2, gType, unbalanced, xfactor, yfactor );
        redrawPix = true;
    }
}

void KPPolygonObject::paint( QPainter* _painter,KoZoomHandler*_zoomHandler,
                             bool drawingShadow, bool drawContour )
{
    int _w = ( pen.style() == Qt::NoPen ) ? 1 : pen.width();//pen.width();

    if ( drawContour ) {
        QPointArray pointArray2 = points.zoomPointArray( _zoomHandler );
        QPen pen3( Qt::black, 1, Qt::DotLine );
        _painter->setPen( pen3 );
        _painter->setRasterOp( Qt::NotXorROP );
        _painter->drawPolygon( pointArray2 );
        return;
    }

    QPointArray pointArray = points.zoomPointArray( _zoomHandler, _w );
    QPen pen2( pen );
    pen2.setWidth( _zoomHandler->zoomItX( pen.width() ) );

    if ( drawingShadow || fillType == FT_BRUSH || !gradient ) {
        _painter->setPen( pen2 );
        _painter->setBrush( brush );
        _painter->drawPolygon( pointArray );
    }
    else {
        QSize size( _zoomHandler->zoomSize( ext ) );
        if ( redrawPix || gradient->size() != size )
        {
            redrawPix = false;
            gradient->setSize( size );
            QRegion clipregion( pointArray );
            pix.resize( size );
            pix.fill( Qt::white );

            QPainter p;
            p.begin( &pix );
            p.setClipRegion( clipregion );
            p.drawPixmap( 0, 0, gradient->pixmap() );
            p.end();

            pix.setMask( pix.createHeuristicMask() );
        }

        QRect _rect = pointArray.boundingRect();
        _painter->drawPixmap( 0, 0, pix, 0, 0, _rect.width(), _rect.height() );

        _painter->setPen( pen2 );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawPolygon( pointArray );

    }
}

void KPPolygonObject::drawPolygon()
{
    KoRect _rect = points.boundingRect();
    double angle = 2 * M_PI / cornersValue;
    double diameter = static_cast<double>( QMAX( _rect.width(), _rect.height() ) );
    double radius = diameter * 0.5;

    KoPointArray _points( checkConcavePolygon ? cornersValue * 2 : cornersValue );
    _points.setPoint( 0, 0, qRound( -radius ) );

    double xmin = 0;
    double ymin = qRound( -radius );

    if ( checkConcavePolygon ) {
        angle = angle / 2.0;
        double a = angle;
        double r = radius - ( sharpnessValue / 100.0 * radius );
        for ( int i = 1; i < cornersValue * 2; ++i ) {
            double xp, yp;
            if ( i % 2 ) {
                xp =  r * sin( a );
                yp = -r * cos( a );
            }
            else {
                xp = radius * sin( a );
                yp = -radius * cos( a );
            }
            a += angle;
            _points.setPoint( i, xp, yp );
            if (xp < xmin)
                xmin = xp;
            if (yp < ymin)
                ymin = yp;
        }
    }
    else {
        double a = angle;
        for ( int i = 1; i < cornersValue; ++i ) {
            double xp = radius * sin( a );
            double yp = -radius * cos( a );
            a += angle;
            _points.setPoint( i, xp, yp );
            if (xp < xmin)
                xmin = xp;
            if (yp < ymin)
                ymin = yp;
        }
    }

    // calculate the points as offsets to 0,0
    KoRect _changRect = _points.boundingRect();
    double fx = _rect.width() / _changRect.width();
    double fy = _rect.height() / _changRect.height();

    int _index = 0;
    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = _points.begin(); it != _points.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = ( point.x() - xmin) * fx;
        double tmpY = ( point.y() - ymin) * fy;

        tmpPoints.putPoints( _index, 1, tmpX,tmpY );
        ++_index;
    }

    points = tmpPoints;

    if ( fillType == FT_GRADIENT && gradient )
        redrawPix = true;
}

void KPPolygonObject::flip( bool horizontal ) 
{
    KP2DObject::flip( horizontal );
    // flip the points
    KoPointArray tmpPoints;
    int index = 0;
    if ( horizontal )
    {
        KoPointArray::ConstIterator it;
        double horiz = getSize().height()/2;
        for ( it = points.begin(); it != points.end(); ++it ) {
            KoPoint point = (*it);
            if ( point.y()> horiz )
                tmpPoints.putPoints( index, 1, point.x(),point.y()- 2*(point.y()-horiz) );
            else
                tmpPoints.putPoints( index, 1, point.x(),point.y()+ 2*(horiz - point.y()) );
            ++index;
        }
    }
    else
    {
        KoPointArray::ConstIterator it;
        double vert = getSize().width()/2;
        for ( it = points.begin(); it != points.end(); ++it ) {
            KoPoint point = (*it);
            if ( point.x()> vert )
                tmpPoints.putPoints( index, 1, point.x()- 2*(point.x()-vert), point.y() );
            else
                tmpPoints.putPoints( index, 1, point.x()+ 2*(vert - point.x()),point.y() );
            ++index;
        }
    }
    points = tmpPoints;

    if ( fillType == FT_GRADIENT ) {
        redrawPix = true;
    }
}

KoSize KPPolygonObject::getRealSize() const {
    KoSize size( ext );
    KoPoint realOrig( orig );
    KoPointArray p( points );
    getRealSizeAndOrigFromPoints( p, angle, size, realOrig );
    return size;
}

KoPoint KPPolygonObject::getRealOrig() const {
    KoSize size( ext );
    KoPoint realOrig( orig );
    KoPointArray p( points );
    getRealSizeAndOrigFromPoints( p, angle, size, realOrig );
    return realOrig;
}
