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

#ifndef kppolygonobject_h
#define kppolygonobject_h

#include <qpointarray.h>
#include "koPointArray.h"
#include <kpobject.h>

class KPGradient;
class QPainter;

/******************************************************************/
/* Class: KPPolygonObject                                         */
/******************************************************************/

class KPPolygonObject : public KP2DObject
{
public:
    KPPolygonObject();
    KPPolygonObject( const KoPointArray &_points, const QSize &_size, const QPen &_pen, const QBrush &_brush,
                     FillType _fillType, const QColor &_gColor1, const QColor &_gColor2, BCType _gType,
                     bool _unbalanced, int _xfactor, int _yfactor,
                     bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue );

    virtual ~KPPolygonObject() {}

    KPPolygonObject &operator=( const KPPolygonObject & );

    virtual void setSize( double _width, double _height );
    virtual void resizeBy( const KoSize &_size );
    virtual void resizeBy( double _dx, double _dy );

    virtual void setFillType( FillType _fillType );
    virtual void setGColor1( const QColor &_gColor1 ) { KP2DObject::setGColor1( _gColor1 ); redrawPix = true; }
    virtual void setGColor2( const QColor &_gColor2 ) { KP2DObject::setGColor2( _gColor2 ); redrawPix = true; }
    virtual void setGType( BCType _gType ) { KP2DObject::setGType( _gType ); redrawPix = true; }

    virtual FillType getFillType() const { return fillType; }
    virtual QColor getGColor1() const { return gColor1; }
    virtual QColor getGColor2() const { return gColor2; }
    virtual BCType getGType() const { return gType; }

    virtual ObjType getType() const { return OT_POLYGON; }
    virtual QString getTypeString() const { return i18n( "Polygon" ); }

    void setPolygonSettings( bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue );
    void getPolygonSettings( bool *_checkConcavePolygon, int *_cornersValue, int *_sharpnessValue );

    virtual QDomDocumentFragment save( QDomDocument& doc, int offset );
    virtual int load( const QDomElement &element );

protected:
    void paint( QPainter *_painter,KoZoomHandler*_zoomHandler );

    void updatePoints( double _fx, double _fy );

    void drawPolygon();

    KoPointArray origPoints, points;
    QSize origSize;
    bool checkConcavePolygon;
    int cornersValue;
    int sharpnessValue;

    QPixmap pix;
    bool redrawPix;
};

#endif
