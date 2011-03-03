/* This file is part of the KDE project
   Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>

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
 * sBoston, MA 02110-1301, USA.
*/

#ifndef KPRHEXAGONWIPEEFFECTFACTORY_H
#define KPRHEXAGONWIPEEFFECTFACTORY_H

#include <KPrPageEffectFactory.h>

class KPrHexagonWipeEffectFactory : public KPrPageEffectFactory
{
public:
    KPrHexagonWipeEffectFactory();
    virtual ~KPrHexagonWipeEffectFactory();
    virtual QString subTypeName(int subType) const;

    enum SubType {
        Horizontal,
        HorizontalReverse,
        Vertical,
        VerticalReverse
    };
};

#endif /* KPRHEXAGONWIPEEFFECTFACTORY_H */
