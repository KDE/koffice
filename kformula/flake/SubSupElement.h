/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#ifndef SUBSUPELEMENT_H
#define SUBSUPELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"

/**
 * @short Implementation of the msub, msup, msubsup elements
 */
class KOFORMULA_EXPORT MultiscriptElement : public BasicElement {
public:
    /// The standard constructor
    MultiscriptElement( BasicElement* parent = 0 );

    /// The destructor
    ~MultiscriptElement();

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     * @param am AttributeManager containing style info
     */
    void paint( QPainter& painter, AttributeManager* am );

    /**
     * Calculate the size of the element and the positions of its children
     * @param am The AttributeManager providing information about attributes values
     */
    void layout( const AttributeManager* am );

    /**
     * Implement the cursor behaviour for the element
     * @param direction Indicates whether the cursor moves up, down, right or left
     * @return A this pointer if the element accepts if not the element to asked instead
     */
    BasicElement* acceptCursor( CursorDirection direction );

    /// @return The default value of the attribute for this element
    QString attributesDefaultValue( const QString& attribute ) const; 

    /// @return The element's ElementType
    ElementType elementType() const;

protected:
    /// Read all content from the node
    bool readMathMLContent( const KoXmlElement& element );

    /// Write all content to the KoXmlWriter
    void writeMathMLContent( KoXmlWriter* writer ) const;

private:
    /// The BasicElement representing the base element of the multiscript
    BasicElement* m_baseElement;

    //For now, we will only worry about msub, msup and msubsup
    //All of these will need to become QList<BasicElement*> for mmultiscript support
    
    /// The BasicElement representing the subscript left to the base element
    //BasicElement* m_preSubscript;

    /// The BasicElement representing the superscript left to the base element
    //BasicElement* m_preSuperscript;

    /// The BasicElement representing the subscript right to the base element
    BasicElement* m_postSubscript;

    /// The BasicElement representing the superscript right to the base element
    BasicElement* m_postSuperscript;
};

#endif // SUBSUPELEMENT_H
