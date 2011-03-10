/* This file is part of the KDE project
   Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software itation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software itation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SCRoundRectWipeEffectFactory.h"

#include <cmath>

#include <klocale.h>


#include "../SCIrisWipeEffectStrategyBase.h"

#define RoundRectWipeEffectFactoryId "RoundRectWipeEffectFactory"

SCRoundRectWipeEffectFactory::SCRoundRectWipeEffectFactory()
: SCPageEffectFactory(RoundRectWipeEffectFactoryId, i18n("RoundRect"))
{
    QPainterPath shape;

    //horizontal
    shape.addRoundedRect(-25, -12, 50, 24, 10, Qt::AbsoluteSize);
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, Horizontal, "RoundRect", "horizontal", false));

    //horizontal reverse
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, HorizontalReverse, "RoundRect", "horizontal", true));

    //vertical
    shape = QPainterPath();
    shape.addRoundedRect(-12, -25, 24, 50, 10, Qt::AbsoluteSize);
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, Vertical, "RoundRect", "vertical", false));

    //vertical reverse
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, VerticalReverse, "RoundRect", "vertical", true));
}

SCRoundRectWipeEffectFactory::~SCRoundRectWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("Horizontal"),
    I18N_NOOP("Horizontal Reverse"),
    I18N_NOOP("Vertical"),
    I18N_NOOP("Vertical Reverse")
};

QString SCRoundRectWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}
