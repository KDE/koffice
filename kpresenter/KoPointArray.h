// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2001 Laurent MONTEL <lmontel@mandrakesoft.com>

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
#ifndef KOPOINTARRAY_H
#define KOPOINTARRAY_H

#include <q3memarray.h>
//Added by qt3to4:
#include <Q3PointArray>

class KoZoomHandler;
class KoPointArray : public Q3MemArray<QPointF>
{
public:
    KoPointArray() {}
    ~KoPointArray() {}
    explicit KoPointArray( int size ) : Q3MemArray<QPointF>( size ) {}
    KoPointArray( const KoPointArray &a ) : Q3MemArray<QPointF>( a ) {}

    KoPointArray &operator=( const KoPointArray &a )
        { return (KoPointArray&)assign( a ); }

    KoPointArray copy() const
        { KoPointArray tmp; return *((KoPointArray*)&tmp.duplicate(*this)); }

    void    translate( double dx, double dy );
    QRectF   boundingRect() const;

    void    point( uint i, double *x, double *y ) const;
    QPointF  point( uint i ) const;
    void    setPoint( uint i, double x, double y );
    void    setPoint( uint i, const QPointF &p );
    bool    putPoints( int index, int nPoints, double firstx, double firsty, ... );

    KoPointArray cubicBezier() const;
    static void cleanBuffers();

    Q3PointArray zoomPointArray( const KoZoomHandler* zoomHandler ) const;
    // Zoom the point array, taking into account the width of the pen
    // (reducing the figure as necessary)
    Q3PointArray zoomPointArray( const KoZoomHandler* zoomHandler, int penWidth ) const;

protected:
    static uint splen;
    static void* sp;
};


#endif // KOPOINTARRAY_H
