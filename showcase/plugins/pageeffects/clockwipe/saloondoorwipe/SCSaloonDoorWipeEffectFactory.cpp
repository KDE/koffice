/* This file is part of the KDE project
   Copyright (C) 2008 Sven Langkamp <sven.langkamp@gmail.com>

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

#include "SCSaloonDoorWipeEffectFactory.h"
#include <klocale.h>

#include "SCSaloonDoorWipeStrategy.h"

#define SaloonDoorWipeEffectId  "SaloonDoorWipeEffect"

SCSaloonDoorWipeEffectFactory::SCSaloonDoorWipeEffectFactory()
: SCPageEffectFactory(SaloonDoorWipeEffectId, i18n("Saloon Door"))
{
    addStrategy(new SCSaloonDoorWipeStrategy(FromTop, "saloonDoorWipe", "top", false));
    addStrategy(new SCSaloonDoorWipeStrategy(FromLeft, "saloonDoorWipe", "left", false));
    addStrategy(new SCSaloonDoorWipeStrategy(FromBottom, "saloonDoorWipe", "bottom", false));
    addStrategy(new SCSaloonDoorWipeStrategy(FromRight, "saloonDoorWipe", "right", false));

    addStrategy(new SCSaloonDoorWipeStrategy(ToTop, "saloonDoorWipe", "top", true));
    addStrategy(new SCSaloonDoorWipeStrategy(ToLeft, "saloonDoorWipe", "left", true));
    addStrategy(new SCSaloonDoorWipeStrategy(ToBottom, "saloonDoorWipe", "bottom", true));
    addStrategy(new SCSaloonDoorWipeStrategy(ToRight, "saloonDoorWipe", "right", true));
}

SCSaloonDoorWipeEffectFactory::~SCSaloonDoorWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("From Top"),
    I18N_NOOP("From Left"),
    I18N_NOOP("From Bottom"),
    I18N_NOOP("From Right"),
    I18N_NOOP("To Top"),
    I18N_NOOP("To Left"),
    I18N_NOOP("To Bottom"),
    I18N_NOOP("To Right")
};

QString SCSaloonDoorWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}
