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

#ifndef TEXTELEMENT_H
#define TEXTELEMENT_H

#include <qfont.h>
#include <qstring.h>

#include "basicelement.h"

class SymbolTable;


KFORMULA_NAMESPACE_BEGIN

/**
 * A element that represents one char.
 */
class TextElement : public BasicElement {
public:

    TextElement(QChar ch = ' ', bool beSymbol = false, BasicElement* parent = 0);


    /**
     * @returns the type of this element. Used for
     * parsing a sequence.
     */
    virtual TokenType getTokenType() const;

    /**
     * @returns true if we don't want to see the element.
     */
    virtual bool isInvisible() const;

    /**
     * @returns the character that represents this element. Used for
     * parsing a sequence.
     */
    virtual QChar getCharacter() const { return character; }

    // drawing
    //
    // Drawing depends on a context which knows the required properties like
    // fonts, spaces and such.
    // It is essential to calculate elements size with the same context
    // before you draw.

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes(const ContextStyle& context, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle);

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuRect& r,
                       const ContextStyle& context,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPoint& parentOrigin );

    /**
     * Moves the cursor away from the given child. The cursor is
     * guaranteed to be inside this element.
     */
    //virtual void childWillVanish(FormulaCursor*, BasicElement*) {}

    /**
     * @returns whether we are a symbol (greek letter).
     */
    bool isSymbol() const { return symbol; }

    /**
     * @returns the latex representation of the element and
     * of the element's children
     */
    virtual QString toLatex();

protected:

    //Save/load support

    /**
     * @returns the tag name of this element type.
     */
    virtual QString getTagName() const { return "TEXT"; }

    /**
     * Appends our attributes to the dom element.
     */
    virtual void writeDom(QDomElement& element);

    /**
     * Reads our attributes from the element.
     * Returns false if it failed.
     */
    virtual bool readAttributesFromDom(QDomElement& element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read.
     * Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

    /**
     * @returns the char that is used to draw with the given font.
     */
    QChar getRealCharacter();

    /**
     * @returns the font to be used for the element.
     */
    QFont getFont(const ContextStyle& context);

    /**
     * Sets up the painter to be used for drawing.
     */
    void setUpPainter(const ContextStyle& context, QPainter& painter);

    const SymbolTable& getSymbolTable() const;

private:

    /**
     * Our content.
     */
    QChar character;

    /**
     * Whether this character is a symbol.
     */
    bool symbol;
};

KFORMULA_NAMESPACE_END

#endif // TEXTELEMENT_H
