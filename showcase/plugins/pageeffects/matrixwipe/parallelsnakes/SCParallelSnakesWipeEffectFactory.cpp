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

#include "SCParallelSnakesWipeEffectFactory.h"

#include <klocale.h>

#include "SCParallelSnakesWipeVerticalStrategy.h"
#include "SCParallelSnakesWipeHorizontalStrategy.h"
#include "SCParallelSnakesWipeDiagonalStrategy.h"

#define ParallelSnakesWipeEffectId "ParallelSnakesWipeEffect"

SCParallelSnakesWipeEffectFactory::SCParallelSnakesWipeEffectFactory()
    : SCPageEffectFactory(ParallelSnakesWipeEffectId, i18n("Parallel Snakes"))
{
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(false, false, false));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(true, true, false));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(false, true, false));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(true, false, false));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(false, false, true));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(true, true, true));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(false, true, true));
    addStrategy(new SCParallelSnakesWipeVerticalStrategy(true, false, true));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(false, false, false));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(true, true, false));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(false, true, false));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(true, false, false));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(false, false, true));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(true, true, true));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(false, true, true));
    addStrategy(new SCParallelSnakesWipeHorizontalStrategy(true, false, true));
    addStrategy(new SCParallelSnakesWipeDiagonalStrategy(false, true));
    addStrategy(new SCParallelSnakesWipeDiagonalStrategy(true, true));
    addStrategy(new SCParallelSnakesWipeDiagonalStrategy(false, false));
    addStrategy(new SCParallelSnakesWipeDiagonalStrategy(true, false));
}

SCParallelSnakesWipeEffectFactory::~SCParallelSnakesWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("Vertical Top Same In"),
    I18N_NOOP("Vertical Top Same Out"),
    I18N_NOOP("Vertical Bottom Same In"),
    I18N_NOOP("Vertical Bottom Same Out"),
    I18N_NOOP("Vertical Top Left Opposite In"),
    I18N_NOOP("Vertical Top Left Opposite Out"),
    I18N_NOOP("Vertical Bottom Left Opposite In"),
    I18N_NOOP("Vertical Bottom Left Opposite Out"),
    I18N_NOOP("Horizontal Left Same In"),
    I18N_NOOP("Horizontal Left Same Out"),
    I18N_NOOP("Horizontal Right Same In"),
    I18N_NOOP("Horizontal Right Same Out"),
    I18N_NOOP("Horizontal Top Left Opposite In"),
    I18N_NOOP("Horizontal Top Left Opposite Out"),
    I18N_NOOP("Horizontal Top Right Opposite In"),
    I18N_NOOP("Horizontal Top Right Opposite Out"),
    I18N_NOOP("Diagonal Bottom Left Opposite In"),
    I18N_NOOP("Diagonal Bottom Left Opposite Out"),
    I18N_NOOP("Diagonal Top Left Opposite In"),
    I18N_NOOP("Diagonal Top Left Opposite Out")
};

QString SCParallelSnakesWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}
