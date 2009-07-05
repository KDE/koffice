/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein <jeeree@web.de>

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

#ifndef FIXEDELEMENT_H
#define FIXEDELEMENT_H

#include "BasicElement.h"
#include "kformula_export.h"

class FormulaCursor;
class QPainterPath;

/**
 * @short Abstract Base Class for MathML elements with fixed number of childs
 *
 */

class KOFORMULA_EXPORT FixedElement : public BasicElement {
public:
    /// The standard constructor
    FixedElement( BasicElement* parent = 0 );

    /// The standard destructor
    virtual ~FixedElement();
    
    BasicElement* elementBefore(int position);
    
    BasicElement* elementAfter(int position);
    
    virtual QList<BasicElement*> elementsBetween(int pos1, int pos2) const = 0;
    
    virtual QPainterPath selectionRegion ( const int pos1, const int pos2 ) const;
   
protected:
    
};

#endif // ROWELEMENT_H

