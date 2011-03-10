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
 * Boston, MA 02110-1301, USA.
*/

#include "SCArrowHeadWipeEffectFactory.h"

#include <cmath>

#include <klocale.h>

#include "../SCIrisWipeEffectStrategyBase.h"

#define ArrowHeadEffectFactoryId "ArrowHeadEffectFactory"

SCArrowHeadWipeEffectFactory::SCArrowHeadWipeEffectFactory()
: SCPageEffectFactory(ArrowHeadEffectFactoryId, i18n("ArrowHead"))
{
    QPainterPath shape;

    //up
    shape.moveTo(-25*cos(M_PI/2), -25*sin(M_PI/2));
    shape.lineTo(-25*cos(7 * M_PI / 6), -25*sin(7 * M_PI / 6));
    shape.lineTo(0, 7);
    shape.lineTo(-25*cos(11 * M_PI / 6), -25*sin(11 * M_PI / 6));
    shape.closeSubpath();
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, Up, "arrowHeadWipe", "up", false));

    //up reverse
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, UpReverse, "arrowHeadWipe", "up", true));

    //right
    shape = QPainterPath();
    shape.moveTo(25*cos(0.0), 25*sin(0.0));
    shape.lineTo(25*cos(2 * M_PI / 3), 25*sin(2 * M_PI / 3));
    shape.lineTo(-7, 0);
    shape.lineTo(25*cos(4 * M_PI / 3), 25*sin(4 * M_PI / 3));
    shape.closeSubpath();
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, Right, "arrowHeadWipe", "right", false));

    //right reverse
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, RightReverse, "arrowHeadWipe", "right", true));

    //down
    shape = QPainterPath();
    shape.moveTo(25*cos(M_PI/2), 25*sin(M_PI/2));
    shape.lineTo(25*cos(7 * M_PI / 6), 25*sin(7 * M_PI / 6));
    shape.lineTo(0, -7);
    shape.lineTo(25*cos(11 * M_PI / 6), 25*sin(11 * M_PI / 6));
    shape.closeSubpath();
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, Down, "arrowHeadWipe", "down", false));

    //down reverse
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, DownReverse, "arrowHeadWipe", "down", true));

    //left
    shape = QPainterPath();
    shape.moveTo(-25*cos(0.0), 25*sin(0.0));
    shape.lineTo(-25*cos(2 * M_PI / 3), 25*sin(2 * M_PI / 3));
    shape.lineTo(7, 0);
    shape.lineTo(-25*cos(4 * M_PI / 3), 25*sin(4 * M_PI / 3));
    shape.closeSubpath();
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, Left, "arrowHeadWipe", "left", false));

    //left reverse
    addStrategy(new SCIrisWipeEffectStrategyBase(shape, LeftReverse, "arrowHeadWipe", "left", true));
}

SCArrowHeadWipeEffectFactory::~SCArrowHeadWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("Up"),
    I18N_NOOP("Up Reverse"),
    I18N_NOOP("Right"),
    I18N_NOOP("Right Reverse"),
    I18N_NOOP("Down"),
    I18N_NOOP("Down Reverse"),
    I18N_NOOP("Left"),
    I18N_NOOP("Left Reverse")
};

QString SCArrowHeadWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}
