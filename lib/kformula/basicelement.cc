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

#include <qstring.h>
#include <kdebug.h>

#include "contextstyle.h"
#include "basicelement.h"
#include "formulacursor.h"
#include "sequenceelement.h"

KFORMULA_NAMESPACE_BEGIN
using namespace std;


int BasicElement::evilDestructionCount = 0;

BasicElement::BasicElement( BasicElement* p )
        : parent( p ), m_baseline( 0 ), m_axis( 0 ), elementType( 0 )
{
    setX( 0 );
    setY( 0 );
    setWidth( 0 );
    setHeight( 0 );
    evilDestructionCount++;
}

BasicElement::~BasicElement()
{
    evilDestructionCount--;
}


/**
 * Returns the element the point is in.
 */
BasicElement* BasicElement::goToPos( FormulaCursor*, bool&,
                                     const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    luPixel x = point.x() - (parentOrigin.x() + getX());
    if ((x >= 0) && (x < getWidth())) {
        luPixel y = point.y() - (parentOrigin.y() + getY());
        if ((y >= 0) && (y < getHeight())) {
            return this;
        }
    }
    return 0;
}

/**
 * Returns our position inside the widget.
 */
LuPixelPoint BasicElement::widgetPos()
{
    luPixel x = 0;
    luPixel y = 0;
    for (BasicElement* element = this; element != 0; element = element->parent) {
        x += element->getX();
        y += element->getY();
    }
    return LuPixelPoint(x, y);
}


/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void BasicElement::goInside(FormulaCursor* cursor)
{
    BasicElement* mainChild = getMainChild();
    if (mainChild != 0) {
        mainChild->goInside(cursor);
    }
}


/**
 * Calculates the base line. This is used by all elements
 * those main child determines the position of the elements
 * base line.
 */
void BasicElement::calcBaseline()
{
    BasicElement* content = getMainChild();
    if (content->getBaseline() > -1) {
        //setBaseline(content->getBaseline() - content->getMidline() + getMidline());
        setBaseline(content->getBaseline() + content->getY());
    }
    else {
        setBaseline(-1);
    }
}


/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void BasicElement::moveLeft(FormulaCursor* cursor, BasicElement*)
{
    getParent()->moveLeft(cursor, this);
}


/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void BasicElement::moveRight(FormulaCursor* cursor, BasicElement*)
{
    getParent()->moveRight(cursor, this);
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


QDomElement BasicElement::getElementDom(QDomDocument& doc)
{
    QDomElement de = doc.createElement(getTagName());
    writeDom(de);
    return de;
}

bool BasicElement::buildFromDom(QDomElement& element)
{
    if (element.tagName() != getTagName()) {
        kdDebug( DEBUGID ) << "Wrong tag name " << element.tagName().latin1() << " for " << getTagName().latin1() << ".\n";
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
void BasicElement::writeDom(QDomElement&)
{
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool BasicElement::readAttributesFromDom(QDomElement&)
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
SequenceElement* BasicElement::buildChild( SequenceElement* child, QDomNode node, QString name )
{
    if (node.isElement()) {
        QDomElement e = node.toElement();
        if (e.tagName().upper() == name) {
            QDomNode nodeInner = e.firstChild();
            if (nodeInner.isElement()) {
                QDomElement element = nodeInner.toElement();
                if (!child->buildFromDom(element)) {
                    delete child;
                    child = 0;
                }
            }
        }
    }
    return child;
}

QString BasicElement::toLatex()
{
    return "{}";
}

KFORMULA_NAMESPACE_END
