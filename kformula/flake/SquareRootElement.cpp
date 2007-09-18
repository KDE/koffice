/* This file is part of the KDE project
   Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>

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

#include "SquareRootElement.h"
#include "AttributeManager.h"
#include <QPainter>
#include <QPen>
#include <kdebug.h>

SquareRootElement::SquareRootElement( BasicElement* parent ) : RowElement( parent )
{
}

SquareRootElement::~SquareRootElement()
{
}

void SquareRootElement::paint( QPainter& painter, AttributeManager* am )
{
    BasicElement::paint(painter, am);
    QPen pen;
    pen.setWidth( 1 );
    painter.setPen( pen );
    painter.drawPath( m_rootSymbol );
}

void SquareRootElement::layout( const AttributeManager* am )
{
    RowElement::layout( am );

    kDebug() << "Width: " << width();
    kDebug() << "Height: " << height();

    double thinSpace = am->mathSpaceValue( "thinmathspace" );
    double tickWidth = ( baseLine() + thinSpace ) / 3.0;
    double linethickness = am->doubleOf( "linethickness", this );
    linethickness = 1; // am is broken at the moment - when fixed delete this line
  
    // Set the sqrt dimesions 
    setWidth( tickWidth + width() + thinSpace );
    setHeight( height() + thinSpace );
    setBaseLine( baseLine() + thinSpace );
   
    // Adapt the children's positions to the new offset
    QPointF childOffset( tickWidth + thinSpace, thinSpace );
    foreach( BasicElement* element, childElements() )
        element->setOrigin( element->origin() + childOffset );

    // Draw the sqrt symbol into a QPainterPath as buffer
    m_rootSymbol = QPainterPath();
    m_rootSymbol.moveTo( linethickness, 2.0 * baseLine() / 3.0 );
    m_rootSymbol.lineTo( 0 + tickWidth/2.0, baseLine()-linethickness/2 );
    m_rootSymbol.lineTo( 0 + tickWidth, linethickness/2 );
    m_rootSymbol.lineTo( width() - linethickness/2, linethickness/2 );

}

ElementType SquareRootElement::elementType() const
{
    return SquareRoot;
}

