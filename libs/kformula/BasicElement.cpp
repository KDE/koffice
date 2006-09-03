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

#include <kdebug.h>

#include "contextstyle.h"
#include "BasicElement.h"
#include "FormulaCursor.h"
#include "FormulaElement.h"
#include "SequenceElement.h"

namespace KFormula {

BasicElement::BasicElement( BasicElement* p ) : m_baseline( 0 )
{
    m_parentElement = p;
    m_boundingRect = QRectF( 0, 0, 0, 0 );
}

BasicElement::~BasicElement()
{
}

const QList<BasicElement*>& BasicElement::childElements() 
{
    return QList<BasicElement*>();
}

const QRectF& BasicElement::boundingRect() const
{
    return m_boundingRect;
}

BasicElement* BasicElement::childElementAt( const QPointF& p )
{
    if( !m_boundingRect.contains( p ) )
        return 0;
	  	
    if( childElements().isEmpty() ) 
        return this;
	      
    BasicElement* ownerElement = 0;
    foreach( BasicElement* tmpElement, childElements() )  
    {
        ownerElement = tmpElement->childElementAt( p );
	
        if( ownerElement )
            return ownerElement;
    }
    
    return this;    // if no child contains the point, it's the FormulaElement itsself
}

BasicElement* BasicElement::parentElement() const
{
    return m_parentElement;
}

void BasicElement::drawInternal()
{
}

void BasicElement::moveLeft( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveLeft( cursor, this );
    else
        cursor->setTo( this, 1 );
}

void BasicElement::moveRight( FormulaCursor* cursor, BasicElement* )
{
    if( cursor->currentElement() == this )
        parentElement()->moveRight( cursor, this );
    else
        cursor->setTo( this, 1 );
}

void BasicElement::moveUp( FormulaCursor* cursor, BasicElement* )
{
    parentElement()->moveUp( cursor, this );
}

void BasicElement::moveDown( FormulaCursor* cursor, BasicElement* )
{
    parentElement()->moveDown( cursor, this );
}

void BasicElement::moveHome( FormulaCursor* cursor )
{
    parentElement()->moveHome( cursor );
}

void BasicElement::moveEnd( FormulaCursor* cursor )
{
    parentElement()->moveEnd( cursor );
}

void BasicElement::readMathML( const QDomElement& element )
{
}

void BasicElement::readMathMLAttributes( const QDomElement& element )
{
}

void BasicElement::writeMathML( const KoXmlWriter* writer, bool oasisFormat )
{
/*    parent.appendChild( doc.createComment( QString( "MathML Error in %1" )
                                           .arg( getTagName() ) ) );*/
}





bool BasicElement::readOnly( const BasicElement* /*child*/ ) const
{
    return m_parentElement->readOnly( this );
}

/*FormulaElement* BasicElement::formula()
{
  return m_parentElement->formula();
}*/

/**
 * Returns our position inside the widget.
 */
LuPixelPoint BasicElement::widgetPos()
{
    luPixel x = 0;
    luPixel y = 0;
    for (BasicElement* element = this; element != 0; element = element->getParent()) {
        x += element->getX();
        y += element->getY();
    }
    return LuPixelPoint(x, y);
}




 /**
  * - * Sets the cursor inside this element to its start position
  *   - * For most elements that is the main child.
  *   - */
void BasicElement::goInside(FormulaCursor* cursor)
{
    BasicElement* mainChild = getMainChild();
    if (mainChild != 0) {
      mainChild->goInside(cursor);
    }
}




/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void BasicElement::normalize(FormulaCursor* cursor, Direction direction)
{
    BasicElement* element = getMainChild();
    if (element != 0) {
        if (direction == beforeCursor) {
            element->moveLeft(cursor, this);
        }
        else {
            element->moveRight(cursor, this);
        }
    }
}


QDomElement BasicElement::getElementDom( QDomDocument& doc)
{
    QDomElement de = doc.createElement(getTagName());
    writeDom(de);
    return de;
}



bool BasicElement::buildFromDom(QDomElement element)
{
    if (element.tagName() != getTagName()) {
        kWarning( DEBUGID ) << "Wrong tag name " << element.tagName().toLatin1() << " for " << getTagName().toLatin1() << ".\n";
        return false;
    }
    if (!readAttributesFromDom(element)) {
        return false;
    }
    QDomNode node = element.firstChild();
    return readContentFromDom(node);
}

/**
 * Appends our attributes to the dom element.
 */
void BasicElement::writeDom(QDomElement)
{
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool BasicElement::readAttributesFromDom(QDomElement)
{
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool BasicElement::readContentFromDom(QDomNode&)
{
    return true;
}


/**
 * Returns a SequenceElement constructed from the nodes first child
 * if the nodes name matches the given name.
 */
bool BasicElement::buildChild( SequenceElement* child, QDomNode node, QString name )
{
    if (node.isElement()) {
        QDomElement e = node.toElement();
        if (e.tagName().toUpper() == name) {
            QDomNode nodeInner = e.firstChild();
            if (nodeInner.isElement()) {
                QDomElement element = nodeInner.toElement();
                return child->buildFromDom( element );
            }
        }
    }
    return false;
}

void BasicElement::setWidth( double width )
{
    m_boundingRect.setWidth( width );
}

void BasicElement::setHeight( double height )
{
    m_boundingRect.setHeight( height );
}

void BasicElement::setX( double x )
{
    m_boundingRect.setX( x );
}

void BasicElement::setY( double y )
{
    m_boundingRect.setY( y );
}

double BasicElement::getHeight() const
{
    return m_boundingRect.height();
}

double BasicElement::getWidth() const
{
    return m_boundingRect.width();
}

double BasicElement::getY() const
{
    return m_boundingRect.y();
}

double BasicElement::getX() const
{
    return m_boundingRect.x();
}



} // namespace KFormula
