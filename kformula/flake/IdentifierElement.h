/* This file is part of the KDE project
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef IDENTIFIERELEMENT_H
#define IDENTIFIERELEMENT_H

#include "TokenElement.h"

namespace FormulaShape {

/**
 * @short Implementation of the MathML mi element
 *
 * The mi element represents an identifier and is defined in the section 3.2.3
 * of the MathMl spec.
 */
class IdentifierElement : public TokenElement {
public:
    /// The standart constructor
    IdentifierElement( BasicElement* parent = 0 );

private:
    /// The actual identifier
};

} // namespace FormulaShape

#endif // IDENTIFIERELEMENT_H
