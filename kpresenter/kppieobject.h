/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kppieobject_h
#define kppieobject_h

#include "kpobject.h"
#include <koStyleStack.h>

class KPGradient;
class DCOPObject;

class KPPieObject : public KP2DObject
{
public:
    KPPieObject();
    KPPieObject( const QPen &_pen, const QBrush &_brush, FillType _fillType,
                 const QColor &_gColor1, const QColor &_gColor2, BCType _gType, PieType _pieType,
                 int _p_angle, int _p_len, LineEnd _lineBegin, LineEnd _lineEnd,
                 bool _unbalanced, int _xfactor, int _yfactor );
    virtual ~KPPieObject() {}
    virtual DCOPObject* dcopObject();
    KPPieObject &operator=( const KPPieObject & );

    virtual void setPieType( PieType _pieType )
        { pieType = _pieType; }
    virtual void setPieAngle( int _p_angle )
        { p_angle = _p_angle; }
    virtual void setPieLength( int _p_len )
        { p_len = _p_len; }
    virtual void setLineBegin( LineEnd _lineBegin )
        { lineBegin = _lineBegin; }
    virtual void setLineEnd( LineEnd _lineEnd )
        { lineEnd = _lineEnd; }

    virtual ObjType getType() const
        { return OT_PIE; }
    virtual QString getTypeString() const
        {
            switch ( pieType ) {
            case PT_PIE:
                return i18n("Pie");
                break;
            case PT_ARC:
                return i18n("Arc");
                break;
            case PT_CHORD:
                return i18n("Chord");
                break;
            }
            return QString::null;
        }

    virtual PieType getPieType() const
        { return pieType; }
    virtual int getPieAngle() const
        { return p_angle; }
    virtual int getPieLength() const
        { return p_len; }
    virtual LineEnd getLineBegin() const
        { return lineBegin; }
    virtual LineEnd getLineEnd() const
        { return lineEnd; }

    virtual QDomDocumentFragment save( QDomDocument& doc, double offset );
    virtual double load(const QDomElement &element);
    virtual void flip(bool horizontal );
    virtual void loadOasis(const QDomElement &element, KoOasisContext & context, QDomElement *animation);

    virtual KoSize getRealSize() const;
    virtual KoPoint getRealOrig() const;

protected:
    virtual void paint( QPainter *_painter, KoZoomHandler*_zoomHandler,
                        bool drawingShadow, bool drawContour );

    void setMinMax( double &min_x, double &min_y, double &max_x, double &max_y, KoPoint point ) const;
    void getRealSizeAndOrig( KoSize &size, KoPoint &realOrig ) const;

    PieType pieType;
    int p_angle, p_len;
    LineEnd lineBegin, lineEnd;
};

#endif
