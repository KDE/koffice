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

#ifndef kppointobject_h
#define kppointobject_h

#include "kpobject.h"
#include <koStyleStack.h>

class KPPointObject : public KPShadowObject
{
public:
    KPPointObject();
    KPPointObject( const QPen &_pen, LineEnd _lineBegin, LineEnd _lineEnd );
    KPPointObject( const QPen &_pen, LineEnd _lineBegin, LineEnd _lineEnd, const QBrush &_brush );

    virtual KoSize getRealSize() const;
    virtual KoPoint getRealOrig() const;

    virtual QDomDocumentFragment save( QDomDocument& doc, double offset );
    virtual bool saveOasis( KoXmlWriter &xmlWriter, KoGenStyles& mainStyles );

    virtual double load( const QDomElement &element );
    virtual void loadOasis( const QDomElement &element, KoOasisContext & context, QDomElement *animation );

    virtual void setLineBegin( LineEnd _lineBegin ) { lineBegin = _lineBegin; }
    virtual void setLineEnd( LineEnd _lineEnd ) { lineEnd = _lineEnd; }

    virtual LineEnd getLineBegin() const { return lineBegin; }
    virtual LineEnd getLineEnd() const { return lineEnd; }

    virtual void setSize( double _width, double _height );
    virtual void setSize( const KoSize & _size )
        { setSize( _size.width(), _size.height() ); }

    virtual void flip( bool horizontal );

    virtual void closeObject( bool close );
    virtual bool isClosed() const;

protected:
    void loadOasisMarker( KoOasisContext & context );
    virtual void paint( QPainter *_painter,KoZoomHandler*_zoomHandler,
                        bool drawingShadow, bool drawContour = FALSE );
    virtual void updatePoints( double _fx, double _fy );
    virtual KoPointArray getDrawingPoints();

    KoPointArray points;
    LineEnd lineBegin, lineEnd;
};

#endif
