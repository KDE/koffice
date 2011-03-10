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

#include "SCDoubleSweepWipeEffectFactory.h"
#include <klocale.h>

#include "SCDoubleSweepWipeStrategy.h"

#define DoubleSweepWipeEffectId  "DoubleSweepWipeEffect"

SCDoubleSweepWipeEffectFactory::SCDoubleSweepWipeEffectFactory()
: SCPageEffectFactory(DoubleSweepWipeEffectId, i18n("Double Sweep"))
{
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelVertical, "doubleSweepWipe", "parallelVertical", false));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelDiagonal, "doubleSweepWipe", "parallelDiagonal", false));
    addStrategy(new SCDoubleSweepWipeStrategy(OppositeVertical, "doubleSweepWipe", "oppositeVertical", false));
    addStrategy(new SCDoubleSweepWipeStrategy(OppositeHorizontal, "doubleSweepWipe", "oppositeHorizontal", false));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelDiagonalTopLeft, "doubleSweepWipe", "parallelDiagonalTopLeft", false));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelDiagonalBottomLeft, "doubleSweepWipe", "parallelDiagonalBottomLeft", false));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelVerticalReverse, "doubleSweepWipe", "parallelVertical", true));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelDiagonalReverse, "doubleSweepWipe", "parallelDiagonal", true));
    addStrategy(new SCDoubleSweepWipeStrategy(OppositeVerticalReverse, "doubleSweepWipe", "oppositeVertical", true));
    addStrategy(new SCDoubleSweepWipeStrategy(OppositeHorizontalReverse, "doubleSweepWipe", "oppositeHorizontal", true));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelDiagonalTopLeftReverse, "doubleSweepWipe", "parallelDiagonalTopLeft", true));
    addStrategy(new SCDoubleSweepWipeStrategy(ParallelDiagonalBottomLeftReverse, "doubleSweepWipe", "parallelDiagonalBottomLeft", true));
}

SCDoubleSweepWipeEffectFactory::~SCDoubleSweepWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("Parallel Vertical"),
    I18N_NOOP("Parallel Diagonal"),
    I18N_NOOP("Opposite Vertical"),
    I18N_NOOP("Opposite Horizontal"),
    I18N_NOOP("Parallel Diagonal Top Left"),
    I18N_NOOP("Parallel Diagonal Bottom Left"),
    I18N_NOOP("Parallel Vertical Reverse"),
    I18N_NOOP("Parallel Diagonal Reverse"),
    I18N_NOOP("Opposite Vertical Reverse"),
    I18N_NOOP("Opposite Horizontal Reverse"),
    I18N_NOOP("Parallel Diagonal Top Left Reverse"),
    I18N_NOOP("Parallel Diagonal Bottom Left Reverse")
};

QString SCDoubleSweepWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}
