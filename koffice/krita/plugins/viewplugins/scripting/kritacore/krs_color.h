/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_APIKRS_COLOR_H
#define KROSS_APIKRS_COLOR_H

#include <qcolor.h>

#include <api/class.h>

namespace Kross {

namespace KritaCore {

class Color : public Kross::Api::Class<Color>
{
    public:
        Color ( int x, int y, int z, QColor::Spec colorSpec );
        Color ();

        ~Color();
    public:
        inline const QString getClassName() const
        { return "Kross::Krita::Color"; };
        inline QColor toQColor() { return m_color; };
    private:
        QColor m_color;
};

}

}

#endif
