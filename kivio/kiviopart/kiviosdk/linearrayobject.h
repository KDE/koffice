/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2005 Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIVIOLINEARRAYOBJECT_H
#define KIVIOLINEARRAYOBJECT_H

#include "polylineobject.h"

namespace Kivio {

/**
 * Draws a line array
 */
class LineArrayObject : public PolylineObject
{
  public:
    LineArrayObject();
    ~LineArrayObject();

    /// Duplicate the object
    virtual Object* duplicate();

    /// Type of object
    virtual ShapeType type();

    /// Draws a line array to the canvas
    virtual void paint(QPainter& painter, KoZoomHandler* zoomHandler, bool paintHandles = true);
};

}

#endif
