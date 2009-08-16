/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "RootElement.h"
#include "AttributeManager.h"
#include "FormulaCursor.h"
#include "RowElement.h"
#include <KoXmlReader.h>
#include <QPainter>
#include <QPen>
#include <kdebug.h>

RootElement::RootElement( BasicElement* parent ) : FixedElement( parent )
{
    m_radicand = new RowElement( this );
    m_exponent = new RowElement( this );
}

RootElement::~RootElement()
{
    delete m_radicand;
    delete m_exponent;
}

void RootElement::paint( QPainter& painter, AttributeManager* am )
{
    Q_UNUSED( am )
    QPen pen;
    pen.setWidth( m_lineThickness );
    painter.setPen( pen );
    painter.drawPath( m_rootSymbol );
}

void RootElement::layout( const AttributeManager* am )
{
    // Calculate values to layout the root symbol
    double thinSpace = am->layoutSpacing( this );
    double symbolHeight  = m_radicand->baseLine();
    if( m_radicand->height() > symbolHeight*1.3 ) symbolHeight = m_radicand->height();
    symbolHeight += thinSpace;
    double tickWidth = symbolHeight / 3.0;  // The width of the root symbol's tick part

    m_lineThickness = am->lineThickness(this);

    // The root symbol an xOffset and yOffset due to the exponent.
    double xOffset = m_exponent->width() - tickWidth/2;
    xOffset = xOffset < 0 ? 0 : xOffset; // no negative offset for the root symbol
    double yOffset =  m_exponent->height() - 2.0*symbolHeight/5.0;
    yOffset = yOffset < 0 ? 0 : yOffset;

    // Set the roots dimensions
    setBaseLine( yOffset + thinSpace + m_radicand->baseLine() );
    setHeight( yOffset + thinSpace + m_radicand->height() );
    setWidth( xOffset + tickWidth + m_radicand->width() + thinSpace );

    // Place the children in the correct place
    m_radicand->setOrigin( QPointF( xOffset+tickWidth+thinSpace, yOffset+thinSpace ) );
    m_exponent->setOrigin( QPointF( 0.0, 0.0 ) );

    // Draw the actual root symbol to a path as buffer
    m_rootSymbol = QPainterPath();
    m_rootSymbol.moveTo( xOffset+m_lineThickness, yOffset +  2.0 * symbolHeight / 3.0 );
    m_rootSymbol.lineTo( m_rootSymbol.currentPosition().x()+tickWidth*0.5, yOffset + symbolHeight - m_lineThickness/2 );
    m_rootSymbol.lineTo( m_rootSymbol.currentPosition().x()+tickWidth*0.5, yOffset + m_lineThickness/2 );
    m_rootSymbol.lineTo( width()-m_lineThickness/2, yOffset + m_lineThickness/2);
}

const QList<BasicElement*> RootElement::childElements() const
{
    QList<BasicElement*> tmp;
    tmp << m_exponent << m_radicand;
    return tmp;
}


// QList< BasicElement* > RootElement::elementsBetween(int pos1, int pos2) const
// { 
//     QList<BasicElement*> tmp;
//     if (pos1==0 && pos2 >0) { 
//         tmp.append(m_exponent);
//     }
//     if (pos1<3 && pos2==3) {
//         tmp.append(m_radicand);
//     }
//     return tmp;
// }

// int RootElement::positionOfChild(BasicElement* child) const 
// {
//     if (child==m_exponent) {
//         return 0;
//     } else if (child==m_radicand) {
//         return 2;
//     }
//     return -1;
// }

bool RootElement::setCursorTo(FormulaCursor& cursor, QPointF point)
{
    if (cursor.isSelecting()) {
        return false;
    }
    if (m_exponent->boundingRect().contains(point)) {
        return m_exponent->setCursorTo(cursor, point-m_exponent->origin());
    } else {
        return m_radicand->setCursorTo(cursor, point-m_radicand->origin());
    }
}

bool RootElement::moveCursor(FormulaCursor& newcursor, FormulaCursor& oldcursor)
{
    if (newcursor.isSelecting()) {
        return false;
    } else {
        return moveHorSituation(newcursor,oldcursor,0,1);
    }
}


int RootElement::length() const
{
    return 3;
}


bool RootElement::replaceChild ( BasicElement* oldelement, BasicElement* newelement )
{
    if (oldelement==m_exponent) {
        m_exponent=newelement;
        return true;
    } else if (oldelement==m_radicand) {
        m_radicand=newelement;
        return true;
    }
    return false;
}

ElementType RootElement::elementType() const
{
    return Root;
}

bool RootElement::readMathMLContent( const KoXmlElement& element )
{
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    bool radicand = true;
    bool exponent = true;

    forEachElement( tmp, element ) {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        if( !tmpElement->readMathML( tmp ) ) {
            return false;
        }
        if( radicand ) {
            delete m_radicand;
            m_radicand = tmpElement;
            radicand = false;
        } else if( exponent ) {
            delete m_exponent;
            m_exponent = tmpElement;
            exponent= false;
        } else {
            kDebug(39001) << "Too many arguments to mroot";
            return false;
        }
    }
    return true;
}

void RootElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    Q_ASSERT( m_radicand );
    Q_ASSERT( m_exponent );
    m_radicand->writeMathML( writer );
    m_exponent->writeMathML( writer );
}

