/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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

#ifndef ROOTELEMENT_H
#define ROOTELEMENT_H

#include <QPoint>

#include "BasicElement.h"

namespace KFormula {

/**
 * A nice graphical root.
 */
class RootElement : public BasicElement {
public:
    /// The standard constructor
    RootElement( BasicElement* parent = 0 );

    ~RootElement();

    void insertInExponent( int index, BasicElement* element );

    void insertInRadicand( int index, BasicElement* element );
     
    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& style, 
                            ContextStyle::TextStyle tstyle, 
                            ContextStyle::IndexStyle istyle,
                            StyleAttributes& style );

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       StyleAttributes& style,
                       const LuPixelPoint& parentOrigin );

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor* cursor, BasicElement* from);

    /**
     * Reinserts the index if it has been removed.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * We remove ourselve if we are requested to remove our content.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

    // main child
    //
    // If an element has children one has to become the main one.

    virtual SequenceElement* getMainChild();

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

    // Moves the cursor inside the index. The index has to exist.
    void moveToIndex(FormulaCursor*, Direction);

    // Sets the cursor to point to the place where the index normaly
    // is. These functions are only used if there is no such index and
    // we want to insert them.
    void setToIndex(FormulaCursor*);

    bool hasIndex() const { return m_exponent != 0; }

protected:
    //Save/load support

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "ROOT"; }

    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement element);

    /**
     * Reads our attributes from the element.
     * Returns false if it failed.
     */
    virtual bool readAttributesFromDom(QDomElement element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

    /**
     * Reads our attributes from the MathML element.
     * Also checks whether it's a msqrt or mroot.
     */
//    virtual void readMathMLAttributes( const QDomElement& element );

    /**
     * Reads our content from the MathML node. Sets the node to the next node
     * that needs to be read.
     */
//    virtual int readMathML( KoXmlNode& node );

private:
    virtual QString getElementName() const { return hasIndex() ? "mroot" : "msqrt"; }
    virtual void writeMathML( KoXmlWriter* writer, bool oasisFormat = true );
    /*
    virtual void writeMathMLContent( QDomDocument& doc, 
                                     QDomElement& element,
                                     bool oasisFormat ) const ;
    */

    /// The element that is the radicand of the root
    BasicElement* m_radicand;

    /// The element that is the exponent of the root
    BasicElement* m_exponent;

    /// The point the artwork relates to.
    LuPixelPoint rootOffset;

	/**
	 * Whether it is msqrt or mroot element. It is only used while reading
	 * from MathML. When reading element contents we must know which of them
	 * it is. After reading, hasIndex() should be used instead.
	 */
	bool square;
};

} // namespace KFormula

#endif // ROOTELEMENT_H
