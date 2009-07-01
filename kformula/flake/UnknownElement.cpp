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

#include "UnknownElement.h"
#include "AttributeManager.h"
#include <KoXmlReader.h>
#include <QPainter>
#include <kdebug.h>

UnknownElement::UnknownElement( BasicElement* parent ) : BasicElement( parent )
{
}

UnknownElement::~UnknownElement()
{
}

void UnknownElement::paint( QPainter& painter, AttributeManager* am)
{
    Q_UNUSED( am )
    Q_UNUSED( painter )
}

void UnknownElement::layout( const AttributeManager* am )
{
    Q_UNUSED( am )
}

const QList<BasicElement*> UnknownElement::childElements()
{
    QList<BasicElement*> tmp;
    return tmp;
}

void UnknownElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
    Q_UNUSED( cursor )
    Q_UNUSED( child )
}

bool UnknownElement::acceptCursor( const FormulaCursor* cursor )
{
    Q_UNUSED( cursor )
    return false;
}

ElementType UnknownElement::elementType() const
{
    return Unknown;
}

bool UnknownElement::readMathMLContent( const KoXmlElement& element )
{
//TODO - save the mathml content in a string
    return true;
}

void UnknownElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    //TODO - write the save mathml content back into writer
}

