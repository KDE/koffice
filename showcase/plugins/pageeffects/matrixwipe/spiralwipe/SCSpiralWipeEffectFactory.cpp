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

#include "SCSpiralWipeEffectFactory.h"

#include <klocale.h>

#include "SCSpiralWipeStrategy.h"

#define SpiralWipeEffectId "SpiralWipeEffect"

SCSpiralWipeEffectFactory::SCSpiralWipeEffectFactory()
: SCPageEffectFactory(SpiralWipeEffectId, i18n("Spiral"))
{
    addStrategy(new SCSpiralWipeStrategy(0, true, false));
    addStrategy(new SCSpiralWipeStrategy(1, true, false));
    addStrategy(new SCSpiralWipeStrategy(2, true, false));
    addStrategy(new SCSpiralWipeStrategy(3, true, false));
    addStrategy(new SCSpiralWipeStrategy(0, false, false));
    addStrategy(new SCSpiralWipeStrategy(3, false, false));
    addStrategy(new SCSpiralWipeStrategy(2, false, false));
    addStrategy(new SCSpiralWipeStrategy(1, false, false));
    addStrategy(new SCSpiralWipeStrategy(0, true, true));
    addStrategy(new SCSpiralWipeStrategy(1, true, true));
    addStrategy(new SCSpiralWipeStrategy(2, true, true));
    addStrategy(new SCSpiralWipeStrategy(3, true, true));
    addStrategy(new SCSpiralWipeStrategy(0, false, true));
    addStrategy(new SCSpiralWipeStrategy(3, false, true));
    addStrategy(new SCSpiralWipeStrategy(2, false, true));
    addStrategy(new SCSpiralWipeStrategy(1, false, true));
}

SCSpiralWipeEffectFactory::~SCSpiralWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("Clockwise Top Left In"),
    I18N_NOOP("Clockwise Top Left Out"),
    I18N_NOOP("Clockwise Top Right In"),
    I18N_NOOP("Clockwise Top Right Out"),
    I18N_NOOP("Clockwise Bottom Left In"),
    I18N_NOOP("Clockwise Bottom Left Out"),
    I18N_NOOP("Clockwise Bottom Right In"),
    I18N_NOOP("Clockwise Bottom Right Out"),
    I18N_NOOP("Counterclockwise Top Left In"),
    I18N_NOOP("Counterclockwise Top Left Out"),
    I18N_NOOP("Counterclockwise Top Right In"),
    I18N_NOOP("Counterclockwise Top Right Out"),
    I18N_NOOP("Counterclockwise Bottom Left In"),
    I18N_NOOP("Counterclockwise Bottom Left Out"),
    I18N_NOOP("Counterclockwise Bottom Right In"),
    I18N_NOOP("Counterclockwise Bottom Right Out")
};

QString SCSpiralWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}

