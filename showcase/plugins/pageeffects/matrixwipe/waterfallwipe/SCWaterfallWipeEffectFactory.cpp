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

#include "SCWaterfallWipeEffectFactory.h"

#include <klocale.h>

#include "SCWaterfallWipeTopLeftStrategy.h"
#include "SCWaterfallWipeTopRightStrategy.h"
#include "SCWaterfallWipeBottomLeftStrategy.h"
#include "SCWaterfallWipeBottomRightStrategy.h"

#define WaterfallWipeEffectId "WaterfallWipeEffect"

SCWaterfallWipeEffectFactory::SCWaterfallWipeEffectFactory()
    : SCPageEffectFactory(WaterfallWipeEffectId, i18n("Waterfall"))
{
    addStrategy(new SCWaterfallWipeTopLeftStrategy(SCMatrixWipeStrategy::TopToBottom));
    addStrategy(new SCWaterfallWipeTopLeftStrategy(SCMatrixWipeStrategy::LeftToRight));
    addStrategy(new SCWaterfallWipeTopRightStrategy(SCMatrixWipeStrategy::TopToBottom));
    addStrategy(new SCWaterfallWipeTopRightStrategy(SCMatrixWipeStrategy::RightToLeft));
    addStrategy(new SCWaterfallWipeBottomLeftStrategy(SCMatrixWipeStrategy::BottomToTop));
    addStrategy(new SCWaterfallWipeBottomLeftStrategy(SCMatrixWipeStrategy::LeftToRight));
    addStrategy(new SCWaterfallWipeBottomRightStrategy(SCMatrixWipeStrategy::BottomToTop));
    addStrategy(new SCWaterfallWipeBottomRightStrategy(SCMatrixWipeStrategy::RightToLeft));
}

SCWaterfallWipeEffectFactory::~SCWaterfallWipeEffectFactory()
{
}

static const char* s_subTypes[] = {
    I18N_NOOP("Top Left Vertical"),
    I18N_NOOP("Top Left Horizontal"),
    I18N_NOOP("Top Right Vertical"),
    I18N_NOOP("Top Right Horizontal"),
    I18N_NOOP("Bottom Left Vertical"),
    I18N_NOOP("Bottom Left Horizontal"),
    I18N_NOOP("Bottom Right Vertical"),
    I18N_NOOP("Bottom Right Horizontal")
};

QString SCWaterfallWipeEffectFactory::subTypeName(int subType) const
{
    if (subType >= 0 && (uint)subType < sizeof s_subTypes / sizeof s_subTypes[0]) {
        return i18n(s_subTypes[subType]);
    } else {
        return i18n("Unknown subtype");
    }
}

