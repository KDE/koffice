/* This file is part of the KDE project
   Copyright (C) 2001 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef KFORMULACOMPATIBILITY_H
#define KFORMULACOMPATIBILITY_H

#include <qdom.h>
#include <qstring.h>

KFORMULA_NAMESPACE_BEGIN

/**
 * Converter from the other kformula lib string
 * to a loadable dom.
 */
class KFormulaCompatibility {
public:

    KFormulaCompatibility();

    /**
     * Builds a kformula DOM from a old formula string.
     */
    QDomDocument buildDOM(QString text);

private:

    QDomElement readSequence(QDomDocument doc);
    QDomElement readMatrix(QDomDocument doc);

    void appendToSequence(QDomElement sequence, QDomElement element, int leftIndexSeen);

    void appendNextSequence(QDomDocument doc, QDomElement element);
    QDomElement getLastSequence(QDomDocument doc, QDomElement sequence);

    QDomElement findIndexNode(QDomDocument doc, QDomElement sequence);

    ushort nextToken() { return formulaString[pos++].unicode(); }
    ushort lookAhead(uint i) const { return formulaString[pos+i].unicode(); }
    void pushback() { pos--; }

    bool hasNext() const { return pos < formulaString.length(); }
    uint tokenLeft() const { return formulaString.length()-pos; }

    /**
     * the string we read
     */
    QString formulaString;

    /**
     * current pos
     */
    uint pos;
};

KFORMULA_NAMESPACE_END

#endif // KFORMULACOMPATIBILITY_H
