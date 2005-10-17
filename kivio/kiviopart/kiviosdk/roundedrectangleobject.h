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
#ifndef KIVIOROUNDEDRECTANGLEOBJECT_H
#define KIVIOROUNDEDRECTANGLEOBJECT_H

#include "rectangleobject.h"

namespace Kivio {

/**
 * Draws a rounded rectangle
 */
class RoundedRectangleObject : public RectangleObject
{
  public:
    RoundedRectangleObject();
    ~RoundedRectangleObject();

    /// Duplicate the object
    virtual Object* duplicate();

    /// Type of object
    virtual ShapeType type();

    /// Defines how round the corners are... 0 == square and 99 == maximum roundness
    int xRoundness() const;
    /// Set roundness
    void setXRoundness(int newRoundness);
    /// Defines how round the corners are... 0 == square and 99 == maximum roundness
    int yRoundness() const;
    /// Set roundness
    void setYRoundness(int newRoundness);

    /// Draws a rounded rectangle to the canvas
    virtual void paint(QPainter& painter, KoZoomHandler* zoomHandler, bool paintHandles = true);

  private:
    int m_xRoundness;
    int m_yRoundness;
};

}

#endif
