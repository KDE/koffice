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

#ifndef FRACTIONELEMENT_H
#define FRACTIONELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"
#include <QLineF>

/**
 * @short Implementation of the MathML mfrac element
 *
 * The mfrac element is specified in the MathML spec section 3.3.2. The
 * FractionElement holds two child elements that are the numerator and the
 * denominator.
 */
class KOFORMULA_EXPORT FractionElement : public BasicElement {
public:
    /// The standard constructor
    FractionElement( BasicElement* parent = 0 );
   
    /// The standard destructor 
    ~FractionElement();

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
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements() const;

    /**
     * Insert a new child at the cursor position - reimplemented from BasicElement
     * @param cursor The cursor holding the position where to inser
     * @param child A BasicElement to insert
     */
    void insertChild( FormulaCursor* cursor, BasicElement* child );
    
    /// inherited from BasicElement
    virtual bool setCursorTo(FormulaCursor* cursor, QPointF point);
    
    /**
     * Remove a child element
     * @param cursor The cursor holding the position where to remove
     * @param element The BasicElement to remove
     */ 
    void removeChild( FormulaCursor* cursor, BasicElement* element );

    /// inherited from BasicElement
    bool acceptCursor( const FormulaCursor* cursor );
    
    /// inherited from BasicElement
    virtual bool moveCursor(FormulaCursor* newcursor, FormulaCursor* oldcursor);
    
    /// inherited from BasicElement
    virtual int length() const;
    
    /// inherited from BasicElement
    virtual int positionOfChild(BasicElement* child) const;
    
    /// inherited from BasicElement
    virtual QLineF cursorLine(int position) const;
    
    /// inherited from BasicElement
    virtual QPainterPath selectionRegion(const int pos1, const int pos2) const;
    
    /// @return The default value of the attribute for this element
    QString attributesDefaultValue( const QString& attribute ) const;
    
    /// @return The element's ElementType
    ElementType elementType() const;

protected:
    /// Read all content from the node - reimplemented by child elements
    bool readMathMLContent( const KoXmlElement& parent );

    /// Write all content to the KoXmlWriter - reimplemented by the child elements
    void writeMathMLContent( KoXmlWriter* writer ) const;   
    
    void fixSelection(FormulaCursor* cursor);

private:
    /// Layout the fraction in a bevelled way
    void layoutBevelledFraction( const AttributeManager* am );

    /// The element representing the fraction's numerator
    BasicElement* m_numerator;

    /// The element representing the fraction's denominator 
    BasicElement* m_denominator;

    /// The line that separates the denominator and the numerator
    QLineF m_fractionLine;

    /// Hold the thickness of the fraction line
    double m_lineThickness;
};

#endif // FRACTIONELEMENT_H
