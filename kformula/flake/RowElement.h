/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef ROWELEMENT_H
#define ROWELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"

/**
 * @short Implementation of the MathML mrow element
 *
 * The mrow element is specified in the MathML spec section 3.3.1. It primarily
 * serves as container for other elements that are grouped together and aligned
 * in a row.
 * mrow has no own visual representance that is why the paint() method is empty.
 * The handling of background and other global attributes is done generically
 * inside FormulaRenderer.
 * The layout() method implements the layouting and size calculation for the mrow
 * element. The calculations assume that spacing is done per element and therefore
 * do not handle it.
 * At the moment there is no linebreaking implementation in RowElement.
 */
class KOFORMULA_EXPORT RowElement : public BasicElement {
public:
    /// The standard constructor
    RowElement( BasicElement* parent = 0 );

    /// The standard destructor
    ~RowElement();

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    virtual void paint( QPainter& painter, const AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    virtual void layout( AttributeManager* am );

    /**
     * Insert a new child at the cursor position - reimplemented from BasicElement
     * @param cursor The cursor holding the position where to inser
     * @param child A BasicElement to insert
     */
    virtual void insertChild( FormulaCursor* cursor, BasicElement* child );

    /**
     * Remove a child element
     * @param element The BasicElement to remove
     */
    virtual void removeChild( BasicElement* element );

    /**
     * Obtain a list of all child elements of this element
     * reimplementated from @see BasicElement
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*> childElements();

    /// @return The child element at the position @p index - 0 if the row is empty
    BasicElement* childAt( int index );

    /// @return The index of the @p element in the row - -1 if not in row
    int indexOfElement( const BasicElement* element ) const;

    /**
     * Move the FormulaCursor left - reimplemented from BasicElement
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor right - reimplemented from BasicElement
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveRight( FormulaCursor* cursor, BasicElement* from );

    /// @return The element's ElementType
    virtual ElementType elementType() const;

    /**
     * Read the content of a MathML child. This is used for inferred mrows.
     * In such cases, the first element to read is not the parent, but a
     * valid child.
     */
    bool readMathMLChild( const KoXmlElement& element );

    /// Read the element contents from MathML
    virtual bool readMathMLContent( const KoXmlElement& parent );
protected:

    /// Save the element contents to MathML
    virtual void writeMathMLContent( KoXmlWriter* writer ) const;

    void appendChild( BasicElement* child ) { m_rowElements << child; }

    QList<BasicElement*> children() { return m_rowElements; }

private:
    /// The sorted list of all elements in this row
    QList<BasicElement*> m_rowElements;
};

#endif // ROWELEMENT_H

