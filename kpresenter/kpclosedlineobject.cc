/* This file is part of the KDE project
   Copyright (C) 2002 Toshitaka Fujioka <fujioka@kde.org>

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

#include <kpclosedlineobject.h>
#include <kpgradient.h>
#include <kozoomhandler.h>
#include <kdebug.h>
#include <qbitmap.h>
#include <qregion.h>
#include <qdom.h>
#include <qpicture.h>
#include <qpainter.h>

#include <math.h>

using namespace std;

/******************************************************************/
/* Class: KPClosedLineObject                                      */
/******************************************************************/

/*================ default constructor ===========================*/
KPClosedLineObject::KPClosedLineObject()
    : KP2DObject()
{
    redrawPix = false;
}

/*================== overloaded constructor ======================*/
KPClosedLineObject::KPClosedLineObject( const KoPointArray &_points, const KoSize &_size, const QPen &_pen, const QBrush &_brush,
                                        FillType _fillType, const QColor &_gColor1, const QColor &_gColor2, BCType _gType,
                                        bool _unbalanced, int _xfactor, int _yfactor, const QString _typeString )
    : KP2DObject( _pen, _brush, _fillType, _gColor1, _gColor2, _gType, _unbalanced, _xfactor, _yfactor )
{
    points = KoPointArray( _points );
    origPoints = points;
    origSize = _size;
    typeString = _typeString;

    redrawPix = false;

    if ( fillType == FT_GRADIENT ) {
        gradient = new KPGradient( gColor1, gColor2, gType, unbalanced, xfactor, yfactor );
        redrawPix = true;
    }
    else
        gradient = 0;
}

/*================================================================*/
KPClosedLineObject &KPClosedLineObject::operator=( const KPClosedLineObject & )
{
    return *this;
}

#if 0
DCOPObject* KPClosedLineObject::dcopObject()
{
   if ( !dcop )
	dcop = new KPClosedLineObjectIface( this );
    return dcop;
}
#endif

/*========================= save =================================*/
QDomDocumentFragment KPClosedLineObject::save( QDomDocument& doc, double offset )
{
    QDomDocumentFragment fragment = KP2DObject::save( doc, offset );

    QDomElement elemObjectsName = doc.createElement( "OBJECTSNAME" );

    elemObjectsName.setAttribute( "NAME", typeString );

    fragment.appendChild( elemObjectsName );

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

/*========================== load ================================*/
double KPClosedLineObject::load( const QDomElement &element )
{
    double offset = KP2DObject::load( element );

    QDomElement e = element.namedItem( "OBJECTSNAME" ).toElement();
    if ( !e.isNull() ) {
        if ( e.hasAttribute( "NAME" ) )
            typeString = e.attribute( "NAME" );
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
        origPoints = points;
        origSize = ext;
    }
    return offset;
}

/*================================================================*/
void KPClosedLineObject::setSize( double _width, double _height )
{
    KPObject::setSize( _width, _height );

    double fx = ext.width() / origSize.width();
    double fy = ext.height() / origSize.height();

    updatePoints( fx, fy );
}

void KPClosedLineObject::resizeBy( const KoSize &_size )
{
    resizeBy( _size.width(), _size.height() );
}

/*================================================================*/
void KPClosedLineObject::resizeBy( double _dx, double _dy )
{
    KPObject::resizeBy( _dx, _dy );

    double fx = ext.width() / origSize.width();
    double fy = ext.height() / origSize.height();

    updatePoints( fx, fy );
}

void KPClosedLineObject::updatePoints( double _fx, double _fy )
{
    int index = 0;
    KoPointArray tmpPoints;
    KoPointArray::ConstIterator it;
    for ( it = origPoints.begin(); it != origPoints.end(); ++it ) {
        KoPoint point = (*it);
        double tmpX = point.x() * _fx;
        double tmpY = point.y() * _fy;

        tmpPoints.putPoints( index, 1, tmpX,tmpY );
        ++index;
    }
    points = tmpPoints;
}

/*================================================================*/
void KPClosedLineObject::setFillType( FillType _fillType )
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

/*======================== paint =================================*/
void KPClosedLineObject::paint( QPainter* _painter,KoZoomHandler*_zoomHandler,
                                bool drawingShadow, bool drawContour )
{
    int _w = pen.width();

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

