/* This file is part of the KDE project
 * Copyright (C)  2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoText.h"

#include <klocale.h>

using namespace KoText;

QStringList KoText::underlineTypeList() {
    QStringList lst;
    lst <<i18nc("Underline Style", "None");
    lst <<i18nc("Underline Style", "Single");
    lst <<i18nc("Underline Style", "Double");
    return lst;
}

QStringList KoText::underlineStyleList() {
    QStringList lst;
    lst <<"_________";   // solid
    lst <<"___ ___ __";  // dash
    lst <<"_ _ _ _ _ _"; // dot
    lst <<"___ _ ___ _"; // dash_dot
    lst <<"___ _ _ ___"; // dash_dot_dot
    lst <<"~~~~~~~"; // wavy lines
    return lst;
}



KoText::Tab::Tab()
    : position(0.),
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    type(QTextOption::LeftTab),
#else
    type(LeftTab), // remove when 4.4 is mandatory
#endif
    leaderType(KoCharacterStyle::NoLineType),
    leaderStyle(KoCharacterStyle::NoLineStyle),
    leaderWeight(KoCharacterStyle::AutoLineWeight),
    leaderWidth(0),
    textStyleId(0)
{
}

bool KoText::Tab::operator==(const Tab &other) const {
    return other.position == position &&
        other.type == type &&
        other.delimiter == delimiter &&
        other.leaderStyle == leaderStyle &&
        other.leaderColor == leaderColor &&
        other.leaderText == leaderText &&
        other.textStyleId == textStyleId;
}

