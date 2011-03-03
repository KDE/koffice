/* This file is part of the KDE project
   Copyright (C) 2008 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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

#ifndef KPRSNAKEWIPEEFFECTFACTORY_H
#define KPRSNAKEWIPEEFFECTFACTORY_H

#include <KPrPageEffectFactory.h>

class KPrSnakeWipeEffectFactory : public KPrPageEffectFactory
{
public:
    KPrSnakeWipeEffectFactory();
    virtual ~KPrSnakeWipeEffectFactory();
    virtual QString subTypeName(int subTypeId) const;

    enum SubType {
        FromLeft,       // the new page is coming from the left
        FromRight,      // the new page is coming from the right
        FromTop,        // the new page is coming from the top
        FromBottom,     // the new page is coming from the bottom
        FromTopLeft,    // the new page is coming from the top-left
        FromTopRight,   // the new page is coming from the top-right
        FromBottomLeft, // the new page is coming from the bottom-left
        FromBottomRight // the new page is coming from the bottom-right
    };
};

#endif /* KPRSNAKEWIPEEFFECTFACTORY_H */
