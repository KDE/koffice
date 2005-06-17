/* This file is part of the KDE project
 Copyright (C) 2002-2004 Alexander Dymo <adymo@mksat.net>

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
#ifndef CALCFIELD_H
#define CALCFIELD_H

#include "field.h"

namespace Kudesigner
{

class CalculatedField: public Field
{
public:
    CalculatedField( int x, int y, int width, int height, Canvas *canvas );

    virtual int rtti() const
    {
        return Rtti_Calculated;
    }
    virtual QString getXml();
    virtual void draw( QPainter &painter );
    virtual void updateGeomProps()
    {
        Field::updateGeomProps();
    }
};

}

#endif
