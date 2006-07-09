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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <qfontmetrics.h>
#include <qpainter.h>

#include <kdebug.h>

#include "basicelement.h"
#include "contextstyle.h"
#include "elementtype.h"
#include "elementvisitor.h"
#include "fontstyle.h"
#include "formulaelement.h"
#include "kformulacommand.h"
#include "sequenceelement.h"
#include "symboltable.h"
#include "textelement.h"


KFORMULA_NAMESPACE_BEGIN

TextElement::TextElement(QChar ch, bool beSymbol, BasicElement* parent)
        : BasicElement(parent), character(ch), symbol(beSymbol)
{
    charStyle( anyChar );
    charFamily( anyFamily );
}


TextElement::TextElement( const TextElement& other )
    : BasicElement( other ),
      character( other.character ),
      symbol( other.symbol ),
      m_format( other.m_format )
{
}


bool TextElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


TokenType TextElement::getTokenType() const
{
    if ( isSymbol() ) {
        return getSymbolTable().charClass( character );
    }

    switch ( character.unicode() ) {
    case '+':
    case '-':
    case '*':
        //case '/':  because it counts as text -- no extra spaces
        return BINOP;
    case '=':
    case '<':
    case '>':
        return RELATION;
    case ',':
    case ';':
    case ':':
        return PUNCTUATION;
    case '\\':
        return SEPARATOR;
    case '\0':
        return ELEMENT;
    default:
        if ( character.isNumber() ) {
            return NUMBER;
        }
        else {
            return ORDINARY;
        }
    }
}


bool TextElement::isInvisible() const
{
    if (getElementType() != 0) {
        return getElementType()->isInvisible(*this);
    }
    return false;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void TextElement::calcSizes(const ContextStyle& context, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle /*istyle*/)
{
    luPt mySize = context.getAdjustedSize( tstyle );
    //kdDebug( DEBUGID ) << "TextElement::calcSizes size=" << mySize << endl;

    QFont font = getFont( context );
    font.setPointSizeFloat( context.layoutUnitPtToPt( mySize ) );

    QFontMetrics fm( font );
    QChar ch = getRealCharacter(context);
    if ( ch != QChar::null ) {
        QRect bound = fm.boundingRect( ch );
        setWidth( context.ptToLayoutUnitPt( fm.width( ch ) ) );
        setHeight( context.ptToLayoutUnitPt( bound.height() ) );
        setBaseline( context.ptToLayoutUnitPt( -bound.top() ) );

        // There are some glyphs in TeX that have
        // baseline==0. (\int, \sum, \prod)
        if ( getBaseline() == 0 ) {
            //setBaseline( getHeight()/2 + context.axisHeight( tstyle ) );
            setBaseline( -1 );
        }
    }
    else {
        setWidth( qRound( context.getEmptyRectWidth() * 2./3. ) );
        setHeight( qRound( context.getEmptyRectHeight() * 2./3. ) );
        setBaseline( getHeight() );
    }

    //kdDebug( DEBUGID ) << "height: " << getHeight() << endl;
    //kdDebug( DEBUGID ) << "width: " << getWidth() << endl;
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void TextElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                        const ContextStyle& context,
                        ContextStyle::TextStyle tstyle,
                        ContextStyle::IndexStyle /*istyle*/,
                        const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    setUpPainter( context, painter );

    luPt mySize = context.getAdjustedSize( tstyle );
    QFont font = getFont( context );
    font.setPointSizeFloat( context.layoutUnitToFontSize( mySize, false ) );
    painter.setFont( font );

    //kdDebug( DEBUGID ) << "TextElement::draw font=" << font.rawName() << endl;
    //kdDebug( DEBUGID ) << "TextElement::draw size=" << mySize << endl;
    //kdDebug( DEBUGID ) << "TextElement::draw size=" << context.layoutUnitToFontSize( mySize, false ) << endl;
    //kdDebug( DEBUGID ) << "TextElement::draw height: " << getHeight() << endl;
    //kdDebug( DEBUGID ) << "TextElement::draw width: " << getWidth() << endl;
    //kdDebug( DEBUGID ) << endl;

    // Each starting element draws the whole token
    ElementType* token = getElementType();
    if ( ( token != 0 ) && !symbol ) {
        QString text = token->text( static_cast<SequenceElement*>( getParent() ) );
//         kdDebug() << "draw text: " << text[0].unicode()
//                   << " " << painter.font().family().latin1()
//                   << endl;
        painter.drawText( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getBaseline() ),
                          text );
    }
    else {
        //kdDebug() << "draw char" << endl;
        QChar ch = getRealCharacter(context);
        if ( ch != QChar::null ) {
            luPixel bl = getBaseline();
            if ( bl == -1 ) {
                // That's quite hacky and actually not the way it's
                // meant to be. You shouldn't calculate a lot in
                // draw. But I don't see how else to deal with
                // baseline==0 glyphs without yet another flag.
                bl = -( getHeight()/2 + context.axisHeight( tstyle ) );
            }
            painter.drawText( context.layoutUnitToPixelX( myPos.x() ),
                              context.layoutUnitToPixelY( myPos.y()+bl ),
                              ch );
        }
        else {
            painter.setPen( QPen( context.getErrorColor(),
                                  context.layoutUnitToPixelX( context.getLineWidth() ) ) );
            painter.drawRect( context.layoutUnitToPixelX( myPos.x() ),
                              context.layoutUnitToPixelY( myPos.y() ),
                              context.layoutUnitToPixelX( getWidth() ),
                              context.layoutUnitToPixelY( getHeight() ) );
        }
    }

    // Debug
    //painter.setBrush(Qt::NoBrush);
//     if ( isSymbol() ) {
//         painter.setPen( Qt::red );
//         painter.drawRect( context.layoutUnitToPixelX( myPos.x() ),
//                           context.layoutUnitToPixelX( myPos.y() ),
//                           context.layoutUnitToPixelX( getWidth() ),
//                           context.layoutUnitToPixelX( getHeight() ) );
//         painter.setPen(Qt::green);
//         painter.drawLine(myPos.x(), myPos.y()+axis( context, tstyle ),
//                          myPos.x()+getWidth(), myPos.y()+axis( context, tstyle ));
//     }
}


void TextElement::dispatchFontCommand( FontCommand* cmd )
{
    cmd->addTextElement( this );
}

void TextElement::setCharStyle( CharStyle cs )
{
    charStyle( cs );
    formula()->changed();
}

void TextElement::setCharFamily( CharFamily cf )
{
    charFamily( cf );
    formula()->changed();
}

QChar TextElement::getRealCharacter(const ContextStyle& context)
{
    if ( !isSymbol() ) {
        const FontStyle& fontStyle = context.fontStyle();
        const AlphaTable* alphaTable = fontStyle.alphaTable();
        if ( alphaTable != 0 ) {
            AlphaTableEntry ate = alphaTable->entry( character,
                                                     charFamily(),
                                                     charStyle() );
            if ( ate.valid() ) {
                return ate.pos;
            }
        }
        return character;
    }
    else {
        return getSymbolTable().character(character, charStyle());
    }
}


QFont TextElement::getFont(const ContextStyle& context)
{
    if ( !isSymbol() ) {
        const FontStyle& fontStyle = context.fontStyle();
        const AlphaTable* alphaTable = fontStyle.alphaTable();
        if ( alphaTable != 0 ) {
            AlphaTableEntry ate = alphaTable->entry( character,
                                                     charFamily(),
                                                     charStyle() );
            if ( ate.valid() ) {
                return ate.font;
            }
        }
        QFont font;
        if (getElementType() != 0) {
            font = getElementType()->getFont(context);
        }
        else {
            font = context.getDefaultFont();
        }
        switch ( charStyle() ) {
        case anyChar:
            break;
        case normalChar:
            font.setItalic( false );
            font.setBold( false );
            break;
        case boldChar:
            font.setItalic( false );
            font.setBold( true );
            break;
        case italicChar:
            font.setItalic( true );
            font.setBold( false );
            break;
        case boldItalicChar:
            font.setItalic( true );
            font.setBold( true );
            break;
        }
        return font;
    }
    return context.symbolTable().font( character, charStyle() );
}


void TextElement::setUpPainter(const ContextStyle& context, QPainter& painter)
{
    if (getElementType() != 0) {
        getElementType()->setUpPainter(context, painter);
    }
    else {
        painter.setPen(Qt::red);
    }
}

const SymbolTable& TextElement::getSymbolTable() const
{
    return formula()->getSymbolTable();
}


/**
 * Appends our attributes to the dom element.
 */
void TextElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);
    element.setAttribute("CHAR", QString(character));
    //QString s;
    //element.setAttribute("CHAR", s.sprintf( "#x%05X", character ) );
    if (symbol) element.setAttribute("SYMBOL", "3");

    switch ( charStyle() ) {
    case anyChar: break;
    case normalChar: element.setAttribute("STYLE", "normal"); break;
    case boldChar: element.setAttribute("STYLE", "bold"); break;
    case italicChar: element.setAttribute("STYLE", "italic"); break;
    case boldItalicChar: element.setAttribute("STYLE", "bolditalic"); break;
    }

    switch ( charFamily() ) {
    case normalFamily: element.setAttribute("FAMILY", "normal"); break;
    case scriptFamily: element.setAttribute("FAMILY", "script"); break;
    case frakturFamily: element.setAttribute("FAMILY", "fraktur"); break;
    case doubleStruckFamily: element.setAttribute("FAMILY", "doublestruck"); break;
    case anyFamily: break;
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool TextElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    QString charStr = element.attribute("CHAR");
    if(!charStr.isNull()) {
        character = charStr.at(0);
    }
    QString symbolStr = element.attribute("SYMBOL");
    if(!symbolStr.isNull()) {
        int symbolInt = symbolStr.toInt();
        if ( symbolInt == 1 ) {
            character = getSymbolTable().unicodeFromSymbolFont(character);
        }
        if ( symbolInt == 2 ) {
            switch ( character.unicode() ) {
            case 0x03D5: character = 0x03C6; break;
            case 0x03C6: character = 0x03D5; break;
            case 0x03Ba: character = 0x03BA; break;
            case 0x00B4: character = 0x2032; break;
            case 0x2215: character = 0x2244; break;
            case 0x00B7: character = 0x2022; break;
            case 0x1D574: character = 0x2111; break;
            case 0x1D579: character = 0x211C; break;
            case 0x2219: character = 0x22C5; break;
            case 0x2662: character = 0x26C4; break;
            case 0x220B: character = 0x220D; break;
            case 0x224C: character = 0x2245; break;
            case 0x03DB: character = 0x03C2; break;
            }
        }
        symbol = symbolInt != 0;
    }

    QString styleStr = element.attribute("STYLE");
    if ( styleStr == "normal" ) {
        charStyle( normalChar );
    }
    else if ( styleStr == "bold" ) {
        charStyle( boldChar );
    }
    else if ( styleStr == "italic" ) {
        charStyle( italicChar );
    }
    else if ( styleStr == "bolditalic" ) {
        charStyle( boldItalicChar );
    }
    else {
        charStyle( anyChar );
    }

    QString familyStr = element.attribute( "FAMILY" );
    if ( familyStr == "normal" ) {
        charFamily( normalFamily );
    }
    else if ( familyStr == "script" ) {
        charFamily( scriptFamily );
    }
    else if ( familyStr == "fraktur" ) {
        charFamily( frakturFamily );
    }
    else if ( familyStr == "doublestruck" ) {
        charFamily( doubleStruckFamily );
    }
    else {
        charFamily( anyFamily );
    }

//     kdDebug() << "charStyle=" << charStyle()
//               << "  charFamily=" << charFamily()
//               << "  format=" << int( format() ) << endl;

    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool TextElement::readContentFromDom(QDomNode& node)
{
    return BasicElement::readContentFromDom(node);
}

QString TextElement::toLatex()
{
    if ( isSymbol() ) {
        QString texName = getSymbolTable().name( character );
        if ( !texName.isNull() )
            return " \\" + texName + " ";
        return  " ? ";
    }
    else {
        return QString(character);
    }
}

QString TextElement::formulaString()
{
    if ( isSymbol() ) {
        QString texName = getSymbolTable().name( character );
        if ( !texName.isNull() )
            return " " + texName + " ";
        return " ? ";
    }
    else {
        return character;
    }
}


EmptyElement::EmptyElement( BasicElement* parent )
    : BasicElement( parent )
{
}

EmptyElement::EmptyElement( const EmptyElement& other )
    : BasicElement( other )
{
}


bool EmptyElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void EmptyElement::calcSizes( const ContextStyle& context,
                              ContextStyle::TextStyle tstyle,
                              ContextStyle::IndexStyle /*istyle*/ )
{
    luPt mySize = context.getAdjustedSize( tstyle );
    //kdDebug( DEBUGID ) << "TextElement::calcSizes size=" << mySize << endl;

    QFont font = context.getDefaultFont();
    font.setPointSizeFloat( context.layoutUnitPtToPt( mySize ) );

    QFontMetrics fm( font );
    QChar ch = 'A';
    QRect bound = fm.boundingRect( ch );
    setWidth( 0 );
    setHeight( context.ptToLayoutUnitPt( bound.height() ) );
    setBaseline( context.ptToLayoutUnitPt( -bound.top() ) );
}

void EmptyElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                         const ContextStyle& context,
                         ContextStyle::TextStyle /*tstyle*/,
                         ContextStyle::IndexStyle /*istyle*/,
                         const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    /*
    if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
        return;
    */

    if ( context.edit() ) {
        painter.setPen( context.getHelpColor() );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y() ),
                          context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
    }
}

QString EmptyElement::toLatex()
{
    return "{}";
}

KFORMULA_NAMESPACE_END
