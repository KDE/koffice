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

#ifndef KFORMULAMATHMLREAD_H
#define KFORMULAMATHMLREAD_H

#include <qobject.h>
#include <qdom.h>
#include "kformulainputfilter.h"
#include "contextstyle.h"

KFORMULA_NAMESPACE_BEGIN

class MathML2KFormulaPrivate;

/**
 * This class converts MathML to KFormula XML.
 * Right now are only parts of the Presentation Markup implemented.
 */
class MathML2KFormula : public KFInputFilter
{
    Q_OBJECT

    friend class MathML2KFormulaPrivate;

public:

    /**
     * Build a MathML 2 KFormula converter.
     * call @startConversion to convert and wait for
     * a @conversionFinished signal, than call
     * @getKFormulaDom to get the converted DOM
     */
    MathML2KFormula( QDomDocument mmldoc, const ContextStyle &contextStyle, bool oasisFormat = false );

    /*
     * Get the just created DOM.
     */
    virtual QDomDocument getKFormulaDom();


public slots:
    virtual void startConversion();

private:

    bool processElement( QDomNode node, QDomDocument doc,
                         QDomNode docnode );

    QDomDocument origdoc;
    QDomDocument formuladoc;
    bool oasisFormat;
    const ContextStyle& context;

    MathML2KFormulaPrivate* impl;

    enum Type {
        // Presentation Markup
        UNKNOWN = 1,
        TOKEN   = 2, // Token Elements
        LAYOUT  = 3, // General Layout Schemata
        SCRIPT  = 4, // Script and Limit Schemata
        TABLE   = 5, // Tables and Matrices
        EE      = 6, // Enlivening Expressions

        // Content Markup
        CONTENT = 12
    };
};

KFORMULA_NAMESPACE_END

#endif // KFORMULAMATHMLREAD_H
