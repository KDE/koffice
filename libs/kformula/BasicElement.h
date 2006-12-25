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

#ifndef BASICELEMENT_H
#define BASICELEMENT_H

#include <QHash>
#include <QString>
#include <QVariant>


#include <QList>
#include <QDomElement>
#include <QDomDocument>

#include "contextstyle.h"
#include "kformuladefs.h"

class KoXmlWriter;

namespace KFormula {

class FontCommand;
class FormulaCursor;
class FormulaElement;
class SequenceElement;

enum ElementType {
    Basic,
    Sequence,
    Fraction,
    Matrix,
    MatrixRow,
    MatrixEntry,
    UnderOver,
    MultiScript
};


/**
 * @short The base class for all elements of a formula
 *
 * The BasicElement class is constructed with a parent and normally an element in a
 * formula has a parent. The only exception is the @see FormulaElement which is the
 * root of the element tree and has no parent element.
 * Most of the elements have children but the number of it can be fixed or variable
 * and the type of child element is not certain. So with the childElements() method you
 * can obtain a list of all direct children of an element. Note that the returned list
 * can be empty when the element is eg a token. This is also the reason why each class
 * inheriting BasicElement has to implement the childElements() method on its own.
 * With the childElementAt method you can test if the given point is in the element.
 * This method is generically implemented for all element types only once in
 * BasicElement.
 * The BasicElement knows its size and position in the formula. This data is normally
 * only used for drawing and stored in the m_boundingRect attribute.
 * To adapt both variables, size and coordinates, to fit in the formula each and every
 * BasicElement derived class has to implement layoutElement() and calculateSize()
 * methods. The former adaptes the position, means the coordinates, when the formula
 * changes and the latter calculates the size of the element. After a formula change
 * first calculateSize is called for all elements then layoutElement().
 */
class BasicElement {
public:
    /*
     * The standard constructor
     * @param parent pointer to the BasicElement's parent
     */
    BasicElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~BasicElement();

    /**
     * Get the element of the formula at the given point
     * @param p the point to look for 
     * @return a pointer to a BasicElement
     */
    BasicElement* childElementAt( const QPointF& p );

    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    virtual const QList<BasicElement*> childElements();

    /**
     * Insert a new child at the cursor position
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
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    virtual void paint( QPainter& painter ) const;

    /// Calculate the element's sizes and the size of its children
    virtual void calculateSize();
    
    /**
     * Move the FormulaCursor left
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor right 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveRight( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor up 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveUp( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor down 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    virtual void moveDown( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor at the first position of the line 
     * @param cursor The FormulaCursor to be moved
     */
    virtual void moveHome( FormulaCursor* cursor );

    /**
     * Move the FormulaCursor at the end of the line
     * @param cursor The FormulaCursor to be moved 
     */
    virtual void moveEnd( FormulaCursor* cursor );

    /// @return The element's ElementType
    ElementType elementType() const;
    
    /// @return The height of the element
    double height() const;

    /// @return The width of the element
    double width() const;

    /// @return The baseline of the element
    double baseLine() const;

    /// @return The element's origin 
    QPointF origin() const;

    /// @return The bounding rectangle of the element
    const QRectF& boundingRect() const;

    /// Set the element's width to @p width
    void setWidth( double width );

    /// Set the element's height to @p height
    void setHeight( double height );
    
    /// Set the element's baseline to @p baseLine
    void setBaseLine( double baseLine );

    /// Set the element's origin inside the m_parentElement to @p origin
    void setOrigin( QPointF origin );

    /// Set the element's m_parentElement to @p parent
    void setParentElement( BasicElement* parent );

    /// @return The parent element of this BasicElement
    BasicElement* parentElement() const;

    /// @return The value of the attribute if it is inherited
    QString inheritAttribute( const QString& attribute ) const;
    
    /// Read the element from MathML
    virtual void readMathML( const QDomElement& element );

    /// Save the element to MathML 
    virtual void writeMathML( KoXmlWriter* writer, bool oasisFormat = false ) const ;





    /**
     * @returns whether the child should be read-only. The idea is
     * that a read-only parent has read-only children.
     */
    virtual bool readOnly( const BasicElement* child ) const;

    /**
     * @returns the character that represents this element. Used for
     * parsing a sequence.
     * This is guaranteed to be QChar::null for all non-text elements.
     */
    virtual QChar getCharacter() const { return QChar::Null; }

    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const { return ELEMENT; }


    /**
     * Returns our position inside the widget.
     */
    LuPixelPoint widgetPos();

    /// Calculates our width and height and our children's parentPosition
    virtual void calcSizes(const ContextStyle& context, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle);

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin );


    /**
     * Dispatch this FontCommand to all our TextElement children.
     */
    virtual void dispatchFontCommand( FontCommand* /*cmd*/ ) {}
    
    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    virtual SequenceElement* getMainChild() { return 0; }

    /**
     * Inserts all new children at the cursor position. Places the
     * cursor according to the direction.
     *
     * The list will be emptied but stays the property of the caller.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement*>&, Direction) {}

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction) {}

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    virtual void normalize(FormulaCursor*, Direction);

    /**
     * Returns wether the element has no more useful
     * children (except its main child) and should therefore
     * be replaced by its main child's content.
     */
    virtual bool isSenseless() { return false; }

    /**
     * Returns the child at the cursor.
     */
    virtual BasicElement* getChild(FormulaCursor*, Direction = beforeCursor) { return 0; }

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor*, BasicElement*) {}

    /**
     * Moves the cursor away from the given child. The cursor is
     * guaranteed to be inside this element.
     */
    virtual void childWillVanish(FormulaCursor*, BasicElement*) {}

    /**
     * Callback for the tabs among our children. Needed for alignment.
     */
    virtual void registerTab( BasicElement* /*tab*/ ) {}

    // basic support

    const BasicElement* getParent() const { return m_parentElement; }
    BasicElement* getParent() { return m_parentElement; }
    void setParent(BasicElement* p) { m_parentElement = p; }
    double getX() const;
    double getY() const;
    void setX( double x );
    void setY( double y );
    double getWidth() const;
    double getHeight() const;
    luPixel getBaseline() const { return m_baseLine; }
    void setBaseline( luPixel line ) { m_baseLine = line; }

    luPixel axis( const ContextStyle& style, 
                  ContextStyle::TextStyle tstyle,
                  double factor ) const {
        return getBaseline() - style.axisHeight( tstyle, factor ); }

    /**
     * @return a QDomElement that contain as DomChildren the
     * children, and as attribute the attribute of this
     * element.
     */
    QDomElement getElementDom( QDomDocument& doc);

    /**
     * Set this element attribute, build children and
     * call their buildFromDom.
     */
    bool buildFromDom(QDomElement element);



protected:
    /// Read all attributes loaded and add them to the m_attributes map 
    void readMathMLAttributes( const QDomElement& element );

    virtual int readMathMLContent( QDomNode& node );

    /// Write all attributes of m_attributes to @p writer
    void writeMathMLAttributes( KoXmlWriter* writer ) const ;

    virtual void writeMathMLContent( KoXmlWriter* , bool ) const {};

    /**
     * Returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "BASIC"; }

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
     * Returns if the SequenceElement could be constructed from the nodes first child.
     * The node name must match the given name.
     *
     * This is a service for all subclasses that contain children.
     */
    bool buildChild( SequenceElement* child, QDomNode node, const QString & name );

private:
    /// The element's parent element - might not be null except of FormulaElement
    BasicElement* m_parentElement;

    /// A hash map of all attributes where attribute name is assigned to a value
    QHash<QString,QString> m_attributes;

    /// The element's type, for example a Sequence, a Fraction etc
    ElementType m_elementType;
    
    /// The boundingRect storing the element's width, height, x and y
    QRectF m_boundingRect;
   
    /// The position of our base line from the upper border
    double m_baseLine;
};

} // namespace KFormula

#endif // BASICELEMENT_H
