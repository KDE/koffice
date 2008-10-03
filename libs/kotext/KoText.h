/* This file is part of the KDE project
 * Copyright (C)  2006 Thomas Zander <zander@kde.org>
 * Copyright (C)  2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#ifndef KOTEXT_H
#define KOTEXT_H

#include "kotext_export.h"

#include <kdeversion.h>

#include <QtCore/QStringList>
#include <QtCore/QChar>
#include <QtGui/QTextCharFormat>
#include <QtCore/QMetaType>
#include <QtGui/QTextOption>
#include <styles/KoCharacterStyle.h>


/**
 * Generic namespace of the KOffice Text library for helper methods and data.
 */
namespace KoText
{
KOTEXT_EXPORT QStringList underlineTypeList();
KOTEXT_EXPORT QStringList underlineStyleList();
KOTEXT_EXPORT Qt::Alignment alignmentFromString(const QString &align);
KOTEXT_EXPORT QString alignmentToString(Qt::Alignment align);

/// This enum contains values to be used as keys in the KoCanvasResourceProvider
enum Options {
    ShowTextFrames =  278622039, ///< boolean that enables painting of frame outlines
    ShowSpaces,         ///< boolean that enables painting of spaces
    ShowTabs,           ///< boolean that enables painting of tabs
    ShowEnters,         ///< boolean that enables painting of enters (linefeed chars)
    ShowSpecialCharacters,  ///< boolean that enables painting of special characters (nbsp etc)
    BidiDocument = 493038196,
    CurrentTextDocument = 382490375, ///< set by the text plugin whenever the document is changed
    CurrentTextPosition = 183523,   ///<  used by the text plugin whenever the position is changed
    CurrentTextAnchor = 341899485,   ///<  used by the text plugin whenever the anchor-position is changed
    SelectedTextPosition = 21314576,   ///<  used by the text plugin whenever the alternative selection is changed
    ///  used by the text plugin whenever the alternative selection anchor-position is changed
    SelectedTextAnchor = 3344189
};

/// For paragraphs each tab definition is represented by this struct.
struct KOTEXT_EXPORT Tab {
    Tab();
    qreal position;    ///< distance in point from the start of the text-shape
    QTextOption::TabType type;       ///< Determine which type is used.
    QChar delimiter;    ///< If type is DelimitorTab; tab until this char was found in the text.
    KoCharacterStyle::LineType leaderType; // none/single/double
    KoCharacterStyle::LineStyle leaderStyle; // solid/dotted/dash/...
    KoCharacterStyle::LineWeight leaderWeight; // auto/bold/thin/length/percentage/...
    qreal leaderWidth; // the width value if length/percentage
    QColor leaderColor; ///< if color is valid, then use this instead of the (current) text color
    QString leaderText;   ///< character to print as the leader (filler of the tabbed space)

    bool operator==(const Tab &tab) const;
};

/// Text in this object will be positioned according to the direction.
enum Direction {
    AutoDirection,      ///< Take the direction from the text.
    LeftRightTopBottom, ///< Text layout for most western languages
    RightLeftTopBottom, ///< Text layout for langauges like Hebrew
    TopBottomRightLeft,  ///< Vertical text layout.
    PerhapsLeftRightTopBottom,
    PerhapsRightLeftTopBottom
};

}

Q_DECLARE_METATYPE(KoText::Tab)

#endif
