/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef FORMULADEFS_H
#define FORMULADEFS_H

#include <memory>

#include <qpoint.h>
#include <qrect.h>

#include <koPoint.h>
#include <koRect.h>


#define KFORMULA_NAMESPACE_BEGIN namespace KFormula {
#define KFORMULA_NAMESPACE_END }

KFORMULA_NAMESPACE_BEGIN

/**
 * The type to be used for points. That's the unit the
 * user thinks with.
 */
typedef double pt;
typedef KoPoint PtPoint;
typedef KoRect PtRect;
//typedef KoSize PtSize;

/**
 * Layout Unit. That's the values we store to get
 * wysiwyg right.
 */
typedef int lu;
typedef QPoint LuPoint;
typedef QRect LuRect;
typedef QSize LuSize;

/**
 * The symbols that are supported by our artwork.
 */
enum SymbolType {
    LeftSquareBracket = '[',
    RightSquareBracket = ']',
    LeftCurlyBracket = '{',
    RightCurlyBracket = '}',
    LineBracket = '|',
    LeftCornerBracket = '<',
    RightCornerBracket = '>',
    LeftRoundBracket = '(',
    RightRoundBracket = ')',
    SlashBracket = '/',
    BackSlashBracket = '\\',

    // symbols that have no ascii character
    Empty = 1000,
    Integral,
    Sum,
    Product
};


/**
 * Flag for cursor movement functions.
 * Select means move selecting the text (usually Shift key)
 * Word means move by whole words  (usually Control key)
 */
enum MoveFlag { NormalMovement = 0, SelectMovement = 1, WordMovement = 2 };


/**
 * TeX like char classes
 */
enum CharClass {
    ORDINARY = 0,
    BINOP = 1,
    RELATION = 2,
    PUNCTUATION = 3,

    NUMBER, NAME, ELEMENT, INNER, BRACKET, SEQUENCE, SEPARATOR, END
};

typedef CharClass TokenType;


class ElementIndex;
typedef std::auto_ptr<ElementIndex> ElementIndexPtr;


/**
 * Wether we want to insert to the left of the cursor
 * or right of it.
 * The same for deletion.
 */
enum Direction { beforeCursor, afterCursor };

/**
 * The types of space we know.
 */
enum SpaceWidths { THIN, MEDIUM, THICK, QUAD };


KFORMULA_NAMESPACE_END

#endif // FORMULADEFS_H
