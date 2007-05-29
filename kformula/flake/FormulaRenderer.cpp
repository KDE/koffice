/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "FormulaRenderer.h"
#include "AttributeManager.h"
#include "BasicElement.h"
#include <kdebug.h>

FormulaRenderer::FormulaRenderer()
{
    m_dirtyElement = 0;
    m_attributeManager = new AttributeManager();
}

FormulaRenderer::~FormulaRenderer()
{
    delete m_attributeManager;
}

void FormulaRenderer::paintElement( QPainter& p, BasicElement* element )
{
      // TODO add more general painting code like: background, scriptlevel and so on!
      // p.setBrush( QBrush( m_attributeManager->valueOf( "mathbackground" ) ) );
      // p.setPen( m_attributeManager->valueOf( "mathcolor" ) );
      // p.setFont();
    
    p.save();
    p.translate( element->origin() );
    kDebug() << element->origin() << endl;
    element->paint( p, m_attributeManager );

    if( element->childElements().isEmpty() || element->elementType() == Phantom )
    {
        p.restore();
        return;
    }
    else
        foreach( BasicElement* tmpElement, element->childElements() )
            paintElement( p, tmpElement );

    p.restore();
}

void FormulaRenderer::layoutElement( BasicElement* element )
{
    foreach( BasicElement* tmp, element->childElements() )
        layoutElement( tmp );              // first layout all children

    kDebug() << "layoutElement" << endl;
    element->layout( m_attributeManager );      // actually layout the element
}

void FormulaRenderer::update( QPainter& p, BasicElement* element )
{
    updateElementLayout( element );              // relayout the changed element
    paintElement( p, m_dirtyElement );     // and then repaint as much as needed
}

void FormulaRenderer::updateElementLayout( BasicElement* element )
{
    QRectF tmpBoundingRect;
    bool parentLayoutAffected = true;
    BasicElement* tmpElement = element;
    while( parentLayoutAffected )
    {
        tmpBoundingRect = tmpElement->boundingRect();   // cache the former boundingRect
        tmpElement->layout( m_attributeManager );       // layout the element

        // check whether the new layout affects the parent element's layout
        if( tmpBoundingRect == tmpElement->boundingRect() )
        {
            parentLayoutAffected = false;               // stop the layouting
            m_dirtyElement = tmpElement;
        }
        else
            tmpElement = tmpElement->parentElement();   // prepare layouting the parent
    }
}
