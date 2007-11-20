/* This file is part of the KDE project
 * Copyright (c) 2003 thierry lorthiois <lorthioist@wanadoo.fr>
 * Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "wmfimportparser.h"

#include <core/vdocument.h>

#include <KoPathShape.h>
#include <KoLineBorder.h>
#include <KoShapeLayer.h>
#include <KoImageData.h>

#include <pathshapes/rectangle/KoRectangleShape.h>
#include <pathshapes/ellipse/KoEllipseShape.h>
#include <pictureshape/PictureShape.h>

#include <kdebug.h>

/*
bug : see motar.wmf
*/

WMFImportParser::WMFImportParser() : KoWmfRead() {
}


bool WMFImportParser::play( VDocument& doc )
{
    mDoc = &doc;
    mScaleX = mScaleY = 1;

    // Play the wmf file
    if( ! KoWmfRead::play( ) )
        return false;

    KoShapeLayer * layer = 0;
    // check if we have to insert a default layer
    if( mDoc->layers().count() == 0 )
    {
        layer = new KoShapeLayer();
        mDoc->insertLayer( layer );
    }
    else
        layer = mDoc->layers().first();

    uint zIndex = 0;
    // add all toplevel shapes to the layer
    foreach( KoShape * shape, mDoc->shapes() )
    {
        shape->setZIndex( zIndex++ );
        if( ! shape->parent() )
            layer->addChild( shape );
    }

    return true;
}


//-----------------------------------------------------------------------------
// Virtual Painter

bool WMFImportParser::begin() {
    QRect bounding = boundingRect();

    mBackgroundMode = Qt::TransparentMode;
    mCurrentOrg.setX( bounding.left() );
    mCurrentOrg.setY( bounding.top() );

    if ( isStandard() ) {
        mDoc->setUnit( KoUnit( KoUnit::Point ) );
        mDoc->setPageSize( bounding.size() );
    }
    else {
        // Placeable Wmf store the boundingRect() in pixel and the default DPI
        // The placeable format doesn't have information on which Unit to use
        // so we choose millimeters by default
        mDoc->setUnit( KoUnit( KoUnit::Millimeter ) );
        mDoc->setPageSize( QSizeF( INCH_TO_POINT( (double)bounding.width() / defaultDpi() ),
                                   INCH_TO_POINT( (double)bounding.height() / defaultDpi() ) ) );
    }
    if ( (bounding.width() != 0) && (bounding.height() != 0) ) {
        mScaleX = mDoc->pageSize().width() / (double)bounding.width();
        mScaleY = mDoc->pageSize().height() / (double)bounding.height();
    }
    return true;
}


bool WMFImportParser::end() {
    return true;
}


void WMFImportParser::save() {
}


void WMFImportParser::restore() {
}


void WMFImportParser::setFont( const QFont & ) {
}


void WMFImportParser::setPen( const QPen &pen ) {
    mPen = pen;
}


const QPen &WMFImportParser::pen() const {
    return mPen;
}


void WMFImportParser::setBrush( const QBrush &brush ) {
    mBrush = brush;
}


void WMFImportParser::setBackgroundColor( const QColor &c ) {
    mBackgroundColor = c;
}


void WMFImportParser::setBackgroundMode( Qt::BGMode mode ) {
    mBackgroundMode = mode;
}



void WMFImportParser::setWindowOrg( int left, int top ) {
    mCurrentOrg.setX( left );
    mCurrentOrg.setY( top );
}


void WMFImportParser::setWindowExt( int width, int height ) {
    // the wmf file can change width/height during the drawing
    if ( (width != 0) && (height != 0) ) {
        mScaleX = mDoc->pageSize().width() / (double)width;
        mScaleY = mDoc->pageSize().height() / (double)height;
    }
}


void WMFImportParser::setMatrix( const QMatrix &, bool  ) {
}


void WMFImportParser::setClipRegion( const QRegion & ) {
}


QRegion WMFImportParser::clipRegion() {
    return mClippingRegion;
}


void WMFImportParser::moveTo( int left, int top ) {
    mCurrentPoint.setX( left );
    mCurrentPoint.setY( top );
}


void WMFImportParser::lineTo( int left, int top ) {
    KoPathShape * line = new KoPathShape();
    line->moveTo( QPointF( coordX(mCurrentPoint.x()), coordY(mCurrentPoint.y()) ) );
    line->lineTo( QPointF( coordX(left), coordY(top) ) );
    appendPen( *line );

    mDoc->add( line );
    mCurrentPoint.setX( left );
    mCurrentPoint.setY( top );
}


void WMFImportParser::drawRect( int left, int top, int width, int height ) {
    QRectF bound = QRectF( QPointF( coordX(left), coordY(top) ), QSizeF( scaleW(width), scaleH(height) ) ).normalized();

    KoRectangleShape * rectangle = new KoRectangleShape();
    rectangle->setPosition( bound.topLeft() );
    rectangle->setSize( bound.size() );

    appendPen( *rectangle );
    appendBrush( *rectangle );

    mDoc->add( rectangle );
}


void WMFImportParser::drawRoundRect( int left, int top, int width, int height, int roundw, int roundh ) {
    QRectF bound = QRectF( QPointF( coordX(left), coordY(top) ), QSizeF( scaleW(width), scaleH(height) ) ).normalized();

    KoRectangleShape * rectangle = new KoRectangleShape();
    rectangle->setPosition( bound.topLeft() );
    rectangle->setSize( bound.size() );
    rectangle->setCornerRadiusX( 2.0 * qAbs( roundw )  );
    rectangle->setCornerRadiusY( 2.0 * qAbs( roundh ) );

    appendPen( *rectangle );
    appendBrush( *rectangle );

    mDoc->add( rectangle );
}


void WMFImportParser::drawEllipse( int left, int top, int width, int height ) {
    QRectF bound = QRectF( QPointF( coordX(left), coordY(top) ), QSizeF( scaleW(width), scaleH(height) ) ).normalized();

    KoEllipseShape *ellipse = new KoEllipseShape();
    ellipse->setPosition( bound.topLeft() );
    ellipse->setSize( bound.size() );

    appendPen( *ellipse );
    appendBrush( *ellipse );

    mDoc->add( ellipse );
}


void WMFImportParser::drawArc( int x, int y, int w, int h, int aStart, int aLen ) {
    double start = (aStart * 180) / 2880.0;
    double end = (aLen * 180) / 2880.0;
    end += start;

    QRectF bound = QRectF( QPointF( coordX(x), coordY(y) ), QSizeF( scaleW(w), scaleH(h) ) ).normalized();

    KoEllipseShape * arc = new KoEllipseShape();
    arc->setType( KoEllipseShape::Arc );
    arc->setStartAngle( start );
    arc->setEndAngle( end );
    arc->setPosition( bound.topLeft() );
    arc->setSize( bound.size() );

    appendPen( *arc );
    //appendBrush( *arc );

    mDoc->add( arc );
}


void WMFImportParser::drawPie( int x, int y, int w, int h, int aStart, int aLen ) {
    double start = (aStart * 180) / 2880.0;
    double end = (aLen * 180) / 2880.0;
    end += start;

    QRectF bound = QRectF( QPointF( coordX(x), coordY(y) ), QSizeF( scaleW(w), scaleH(h) ) ).normalized();

    KoEllipseShape * pie = new KoEllipseShape();
    pie->setType( KoEllipseShape::Pie );
    pie->setStartAngle( start );
    pie->setEndAngle( end );
    pie->setPosition( bound.topLeft() );
    pie->setSize( bound.size() );

    appendPen( *pie );
    appendBrush( *pie );

    mDoc->add( pie );
}


void WMFImportParser::drawChord( int x, int y, int w, int h, int aStart, int aLen ) {
    double start = (aStart * 180) / 2880.0;
    double end = (aLen * 180) / 2880.0;
    end += start;

    QRectF bound = QRectF( QPointF( coordX(x), coordY(y) ), QSizeF( scaleW(w), scaleH(h) ) ).normalized();

    KoEllipseShape * chord = new KoEllipseShape();
    chord->setType( KoEllipseShape::Chord );
    chord->setStartAngle( start );
    chord->setEndAngle( end );
    chord->setPosition( bound.topLeft() );
    chord->setSize( bound.size() );

    appendPen( *chord );
    appendBrush( *chord );

    mDoc->add( chord );
}


void WMFImportParser::drawPolyline( const QPolygon &pa ) {
    KoPathShape *polyline = new KoPathShape();
    appendPen( *polyline );
    appendPoints( *polyline, pa );

    mDoc->add( polyline );
}


void WMFImportParser::drawPolygon( const QPolygon &pa, bool winding ) {
    KoPathShape *polygon = new KoPathShape();
    appendPen( *polygon );
    appendBrush( *polygon );
    appendPoints( *polygon, pa );

    polygon->close();
    polygon->setFillRule( winding ? Qt::WindingFill : Qt::OddEvenFill );
    mDoc->add( polygon );
}


void WMFImportParser::drawPolyPolygon( QList<QPolygon>& listPa, bool winding ) {
    KoPathShape *path = new KoPathShape();

    if ( listPa.count() > 0 ) {
        appendPen( *path );
        appendBrush( *path );
        appendPoints( *path, listPa.first() );
        path->close();
        path->setFillRule( winding ? Qt::WindingFill : Qt::OddEvenFill );
        foreach( QPolygon pa, listPa )
        {
            KoPathShape *newPath = new KoPathShape();
            appendPoints( *newPath, pa );
            newPath->close();
            path->combine( newPath );
        }

        mDoc->add( path );
    }
}


void WMFImportParser::drawImage( int x, int y, const QImage &image, int , int , int sw, int sh ) {
    KoImageData * data = new KoImageData( mDoc->imageCollection() );
    data->setImage( image );

    PictureShape * pic = new PictureShape();
    pic->setUserData( data );
    pic->setPosition( QPointF(x,y) );
    if( sw < 0 )
        sw = image.width();
    if( sh < 0 )
        sh = image.height();
    pic->setSize( QSizeF(sw,sh) );

    kDebug() << "image data size =" << data->pixmap().size();

    mDoc->add( pic );
}


void WMFImportParser::drawText( int , int , int , int , int , const QString& , double ) {
    kDebug() << "importing text is not supported";
}


//-----------------------------------------------------------------------------
// Utilities

void WMFImportParser::appendPen( KoShape& obj )
{
    double width = mPen.width() * mScaleX;

    KoLineBorder * border = new KoLineBorder( ((width < 0.99) ? 1 : width), mPen.color() );
    border->setLineStyle( mPen.style(), mPen.dashPattern() );
    border->setCapStyle( mPen.capStyle() );
    border->setJoinStyle( mPen.joinStyle() );

    obj.setBorder( border );
}


void WMFImportParser::appendBrush( KoShape& obj )
{
    obj.setBackground( mBrush );
}

void  WMFImportParser::setCompositionMode( QPainter::CompositionMode )
{
    //TODO
}

void WMFImportParser::appendPoints(KoPathShape &path, const QPolygon& pa)
{
    // list of point array
    if ( pa.size() > 0 ) {
        path.moveTo( QPointF( coordX(pa.point(0).x()), coordY(pa.point(0).y()) ) );
    }
    for ( int i=1 ; i < pa.size() ; i++ ) {
        path.lineTo( QPointF( coordX(pa.point(i).x()), coordY(pa.point(i).y()) ) );
    }
    path.normalize();
}

double WMFImportParser::coordX( int left )
{
    return ((double)(left - mCurrentOrg.x()) * mScaleX);
}

double WMFImportParser::coordY( int top )
{
    return ((double)(top - mCurrentOrg.y()) * mScaleY);
}

double WMFImportParser::scaleW( int width )
{
    return (width * mScaleX);
}

double WMFImportParser::scaleH( int height )
{
    return (height * mScaleY);
}
