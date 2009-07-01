/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2007 Martin Pfeiffer <hubipete@gmx.net>

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

#include "TableEntryElement.h"
#include "FormulaCursor.h"
#include "TableRowElement.h"
#include "TableElement.h"
#include <KoXmlWriter.h>

TableEntryElement::TableEntryElement( BasicElement* parent ) : RowElement( parent )
{}

// void TableEntryElement::layout( const AttributeManager* am )
// {}

QString TableEntryElement::attributesDefaultValue( const QString& attribute ) const
{
    if( attribute == "rowspan" || attribute == "columnspan" )
        return "1";
    else
        return QString();
} 


bool TableEntryElement::moveCursor ( FormulaCursor* newcursor, FormulaCursor* oldcursor )
{
    if (newcursor->isSelecting() || 
        newcursor->direction()==MoveLeft || newcursor->direction()==MoveRight) {
        return RowElement::moveCursor(newcursor,oldcursor);
    } else {
        TableRowElement* tr= static_cast<TableRowElement*>(parentElement());
        TableElement* te = static_cast<TableElement*>(tr->parentElement());
        int rn=te->positionOfChild(tr)/2; //table elements have a cursor 
        int cn=tr->positionOfChild(this);
        //positions before and after each element
        if (newcursor->direction()==MoveUp) {
            if (rn>1) {
                return newcursor->moveCloseTo(te->childElements()[rn-1]->childElements()[cn],oldcursor);
            } else {
                return false;
            }
        } else {
            if (rn < te->length()/2) {
                return newcursor->moveCloseTo(te->childElements()[rn+1]->childElements()[cn],oldcursor);
            } else {
                return false;
            }
        }
    }
}


ElementType TableEntryElement::elementType() const
{
    return TableEntry;
}

