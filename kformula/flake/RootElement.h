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

#ifndef ROOTELEMENT_H
#define ROOTELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"

#include <QPainterPath>

/**
 * @short Implementation of the MathML mroot and msqrt elements 
 */
class KOFORMULA_EXPORT RootElement : public BasicElement {
public:
    /// The standard constructor
    RootElement( BasicElement* parent = 0 );

    /// The standard destructor
    ~RootElement();

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
     * @param cursor The cursor holding the position where to remove
     * @param element The BasicElement to remove
     */ 
    void removeChild( FormulaCursor* cursor, BasicElement* element );

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

    /**
     * Implement the cursor behaviour for the element
     * @param cursor The FormulaCursor that is moved around
     * @return A this pointer if the element accepts if not the element to asked instead
     */
    BasicElement* acceptCursor( const FormulaCursor* cursor );

    /// @return The element's ElementType
    ElementType elementType() const;

protected:
    /// Read root contents - reimplemented from BasicElement
    bool readMathMLContent( const KoXmlElement& element );

    /// Write root contents - reimplemented from BasicElement
    void writeMathMLContent( KoXmlWriter* writer ) const;

private:
    /// The element that is the radicand of the root
    BasicElement* m_radicand;

    /// The element that is the exponent of the root
    BasicElement* m_exponent;

    /// The point the artwork relates to.
    QPointF m_rootOffset;

    /// The QPainterPath that holds the lines for the root sign   
    QPainterPath m_rootSymbol;
};

#endif // ROOTELEMENT_H
