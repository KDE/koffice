// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
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

#ifndef kpclosedlineoject_h
#define kpclosedlineoject_h

#include <qpointarray.h>
#include "koPointArray.h"
#include "kpobject.h"

class KPGradient;
class QPainter;
class DCOPObject;

class KPClosedLineObject : public KP2DObject
{
public:
    KPClosedLineObject();
    KPClosedLineObject( const KoPointArray &_points, const KoSize &_size, const QPen &_pen, const QBrush &_brush,
                        FillType _fillType, const QColor &_gColor1, const QColor &_gColor2, BCType _gType,
                        bool _unbalanced, int _xfactor, int _yfactor, const QString _type );

    virtual ~KPClosedLineObject() {}
    //virtual DCOPObject* dcopObject();

    KPClosedLineObject &operator=( const KPClosedLineObject & );

    virtual void setSize( double _width, double _height );
    virtual void setSize( const KoSize & _size ) { setSize( _size.width(), _size.height() ); }

    virtual void setFillType( FillType _fillType );
    virtual void setGColor1( const QColor &_gColor1 ) { KP2DObject::setGColor1( _gColor1 ); redrawPix = true; }
    virtual void setGColor2( const QColor &_gColor2 ) { KP2DObject::setGColor2( _gColor2 ); redrawPix = true; }
    virtual void setGType( BCType _gType ) { KP2DObject::setGType( _gType ); redrawPix = true; }

    virtual FillType getFillType() const { return fillType; }
    virtual QColor getGColor1() const { return gColor1; }
    virtual QColor getGColor2() const { return gColor2; }
    virtual BCType getGType() const { return gType; }

    virtual ObjType getType() const { return OT_CLOSED_LINE; }
    virtual QString getTypeString() const { return typeString; }


    virtual QDomDocumentFragment save( QDomDocument& doc, double offset );
    virtual double load( const QDomElement &element );

    virtual void flip(bool horizontal );

protected:
    virtual void paint( QPainter *_painter,KoZoomHandler*_zoomHandler,
                        bool drawingShadow, bool drawContour );

    void updatePoints( double _fx, double _fy );


    KoPointArray origPoints, points;
    KoSize origSize;

    QString typeString;
    QPixmap pix;
    bool redrawPix;
};

#endif
