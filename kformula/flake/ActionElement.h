/* This file is part of the KDE project
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

#ifndef ACTIONELEMENT_H
#define ACTIONELEMENT_H

#include "SequenceElement.h"

/**
 * Support for action elements in MathML. According to MathML spec 
 * (Section 3.6.1.1), a MathML conformant application is not required to 
 * recognize any single actiontype.
 */
class ActionElement : public SequenceElement {
public:
    ActionElement( BasicElement* parent = 0 );
    virtual int buildChildrenFromMathMLDom(QList<BasicElement*>& list, QDomNode n);

private:
    virtual void readMathMLAttributes(const QDomElement& element);
    virtual QString elementName() const { return "maction"; }
    virtual void writeMathMLAttributes( QDomElement& element ) const ;

    QString m_actionType;
    uint m_selection;
};

#endif // ACTIONELEMENT_H
