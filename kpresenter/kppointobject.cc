// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2004 Thorsten Zachmann  <zachmann@kde.org>

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

#include "kppointobject.h"
#include "kpresenter_utils.h"
#include "kpresenter_doc.h"
#include <kozoomhandler.h>
#include <koUnit.h>
#include <qdom.h>
#include <qpainter.h>
#include <koStyleStack.h>
#include <kooasiscontext.h>

KPPointObject::KPPointObject()
    : KPShadowObject(), KPStartEndLine( L_NORMAL, L_NORMAL )
{
}


KPPointObject::KPPointObject( const QPen &_pen, LineEnd _lineBegin, LineEnd _lineEnd )
    : KPShadowObject( _pen ), KPStartEndLine(_lineBegin, _lineEnd)
{
}


KPPointObject::KPPointObject( const QPen &_pen, LineEnd _lineBegin, LineEnd _lineEnd, const QBrush &_brush )
    : KPShadowObject( _pen, _brush ), KPStartEndLine( _lineBegin, _lineEnd)
{
}


KoSize KPPointObject::getRealSize() const
{
    KoSize size( ext );
    KoPoint realOrig( orig );
    KoPointArray p( points );
    getRealSizeAndOrigFromPoints( p, angle, size, realOrig );
    return size;
}


KoPoint KPPointObject::getRealOrig() const
{
    KoSize size( ext );
    KoPoint realOrig( orig );
    KoPointArray p( points );
    getRealSizeAndOrigFromPoints( p, angle, size, realOrig );
    return realOrig;
}


QDomDocumentFragment KPPointObject::save( QDomDocument& doc, double offset )
{
    QDomDocumentFragment fragment = KPShadowObject::save( doc, offset );
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

    KPStartEndLine::save( fragment,doc );

    return fragment;
}

QString KPPointObject::saveOasisStrokeElement( KoGenStyles& mainStyles )
{
    KoGenStyle styleobjectauto( KPresenterDoc::STYLE_GRAPHICAUTO, "graphic" );
    saveOasisMarkerElement( mainStyles, styleobjectauto );
    KPShadowObject::saveOasisStrokeElement( mainStyles, styleobjectauto );
    return mainStyles.lookup( styleobjectauto, "gr" );
}


bool KPPointObject::saveOasis( KoXmlWriter &xmlWriter, KoSavingContext& context )
{
    QString listOfPoint;
    int maxX=0;
    int maxY=0;
    KoPointArray::ConstIterator it;
    for ( it = points.begin(); it != points.end(); ++it ) {
        int tmpX = 0;
        int tmpY = 0;
        tmpX = ( int ) ( KoUnit::toMM( ( *it ).x() )*100 );
        tmpY = ( int ) ( KoUnit::toMM( ( *it ).y() )*100 );
        if ( !listOfPoint.isEmpty() )
            listOfPoint += QString( " %1,%2" ).arg( tmpX ).arg( tmpY );
        else
            listOfPoint = QString( "%1,%2" ).arg( tmpX ).arg( tmpY );
        maxX = QMAX( maxX, tmpX );
        maxY = QMAX( maxY, tmpY );
    }
    xmlWriter.addAttribute("draw:points", listOfPoint );
    xmlWriter.addAttribute("svg:viewBox", QString( "0 0 %1 %2" ).arg( maxX ).arg( maxY ) );
    return true;
}

void KPPointObject::loadOasisMarker( KoOasisContext & context )
{
    loadOasisMarkerElement( context, "draw:marker-start", lineBegin );
    loadOasisMarkerElement( context, "draw:marker-end", lineBegin );
}

void KPPointObject::loadOasis( const QDomElement &element, KoOasisContext & context,  QDomElement *animation )
{
    kdDebug()<<"void KPPointObject::loadOasis( const QDomElement &element )*************\n";
    KPShadowObject::loadOasis( element, context, animation );
    //load point.
    QStringList ptList = QStringList::split(' ', element.attribute("draw:points"));
    QString pt_x, pt_y;
    double tmp_x, tmp_y;
    unsigned int index = 0;
    for (QStringList::Iterator it = ptList.begin(); it != ptList.end(); ++it)
    {
        tmp_x = (*it).section(',',0,0).toInt() / 100;
        tmp_y = (*it).section(',',1,1).toInt() / 100;

        pt_x.setNum(tmp_x);
        pt_x+="mm";

        pt_y.setNum(tmp_y);
        pt_y+="mm";

        points.putPoints( index, 1, KoUnit::parseValue(pt_x),KoUnit::parseValue(pt_y) );
        ++index;
    }
    loadOasisMarker( context );
}

double KPPointObject::load( const QDomElement &element )
{
    double offset = KPShadowObject::load( element );

    QDomElement e = element.namedItem( "POINTS" ).toElement();
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
    KPStartEndLine::load( element );
    return offset;
}


void KPPointObject::setSize( double _width, double _height )
{
    KoSize origSize( ext );
    KPObject::setSize( _width, _height );

    double fx = ext.width() / origSize.width();
    double fy = ext.height() / origSize.height();

    updatePoints( fx, fy );
}


void KPPointObject::flip( bool horizontal )
{
    KPObject::flip( horizontal );

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
}


void KPPointObject::closeObject( bool close )
{
    points = getCloseObject( points, close, isClosed() );
}


bool KPPointObject::isClosed() const
{
    return ( points.at(0) == points.at(points.count()-1) );
}


void KPPointObject::paint( QPainter* _painter, KoZoomHandler*_zoomHandler,
                           bool /*drawingShadow*/, bool drawContour )
{
    int _w = pen.width();

    QPen pen2;
    if ( drawContour ) {
        pen2 = QPen( Qt::black, 1, Qt::DotLine );
        _painter->setRasterOp( Qt::NotXorROP );
    }
    else {
        pen2 = pen;
        pen2.setWidth( _zoomHandler->zoomItX( pen.width() ) );
    }
    _painter->setPen( pen2 );

    QPointArray pointArray = getDrawingPoints().zoomPointArray( _zoomHandler, _w );
    _painter->drawPolyline( pointArray );

    if ( lineBegin != L_NORMAL && !drawContour && !isClosed()) {
        QPoint startPoint;
        bool first = true;
        QPointArray::ConstIterator it1;
        for ( it1 = pointArray.begin(); it1 != pointArray.end(); ++it1 ) {
            if ( first ) {
                startPoint = (*it1);
                first = false;
            }

            QPoint point = (*it1);
            if ( startPoint != point ) {
                float angle = KoPoint::getAngle( startPoint, point );
                drawFigureWithOffset( lineBegin, _painter, startPoint, pen2.color(), _w, angle,_zoomHandler );

                break;
            }
        }
    }

    if ( lineEnd != L_NORMAL && !drawContour &&!isClosed()) {
        QPoint endPoint;
        bool last = true;
        QPointArray::ConstIterator it2 = pointArray.end();
        for ( it2 = it2 - 1; it2 != pointArray.begin(); --it2 ) {
            if ( last ) {
                endPoint = (*it2);
                last = false;
            }

            QPoint point = (*it2);
            if ( endPoint != point ) {
                float angle = KoPoint::getAngle( endPoint, point );
                drawFigureWithOffset( lineEnd, _painter, endPoint, pen2.color(), _w, angle,_zoomHandler );

                break;
            }
        }
    }
}


void KPPointObject::updatePoints( double _fx, double _fy )
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


KoPointArray KPPointObject::getDrawingPoints() const
{
  return points;
}
