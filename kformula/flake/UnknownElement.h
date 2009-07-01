/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#ifndef UNKNOWNELEMENT_H
#define UNKNOWNELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"

#include <QPainterPath>

class FormulaCursor;

/**
 * @short Implementation of an unknown element. 
 *
 * Used when we see a tag that we do not recognise. This element draws nothing, takes up
 * no space, and ignores any calls to insert children etc. 
 */
class KOFORMULA_EXPORT UnknownElement : public BasicElement {
public:
    /// The standard constructor
    UnknownElement( BasicElement* parent = 0 );

    /// The standard destructor
    ~UnknownElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    /**
     * Insert a new child at the cursor position
     * @param cursor The cursor holding the position where to inser
     * @param child A BasicElement to insert
     */
    void insertChild( FormulaCursor* cursor, BasicElement* child );
   
    /**
     * Remove a child element
     * @param element The BasicElement to remove
     */ 
    //void removeChild( BasicElement* element );

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter, AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /// inherited from BasicElement
    bool acceptCursor( const FormulaCursor* cursor );

    /// @return The element's ElementType
    ElementType elementType() const;

protected:
    /// Read root contents - reimplemented from BasicElement
    bool readMathMLContent( const KoXmlElement& element );

    /// Write root contents - reimplemented from BasicElement
    void writeMathMLContent( KoXmlWriter* writer ) const;

private:
};

#endif // UNKNOWNELEMENT_H
