/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
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
 * Boston, MA 02110-1301, USA.
*/

#include <qfontmetrics.h>
#include <qpainter.h>

#include <kdebug.h>
#include <kprinter.h>

#include "contextstyle.h"
#include "elementvisitor.h"
#include "spaceelement.h"


KFORMULA_NAMESPACE_BEGIN


SpaceElement::SpaceElement( SpaceWidth space, bool tab, BasicElement* parent )
    : BasicElement( parent ), 
      spaceWidth( space ), 
      m_tab( tab ),
      m_widthType( NoSize ),
      m_heightType( NoSize ),
      m_depthType( NoSize ),
      m_lineBreak( NoBreakType )
{
}


SpaceElement::SpaceElement( const SpaceElement& other )
    : BasicElement( other ),
      spaceWidth( other.spaceWidth ),
      m_widthType( other.m_widthType ),
      m_width( other.m_width ),
      m_heightType( other.m_heightType ),
      m_height( other.m_height ),
      m_depthType( other.m_depthType ),
      m_depth( other.m_depth ),
      m_lineBreak( other.m_lineBreak )
{
}


bool SpaceElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void SpaceElement::calcSizes( const ContextStyle& context,
                              ContextStyle::TextStyle tstyle,
                              ContextStyle::IndexStyle /*istyle*/,
                              StyleAttributes& style )
{
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );

    QFont font = context.getDefaultFont();
    font.setPointSize( mySize );

    QFontMetrics fm( font );
    QChar ch = 'x';
    LuPixelRect bound = fm.boundingRect( ch );

    setWidth( context.getSpace( tstyle, spaceWidth, factor ) );
    setHeight( bound.height() );
    setBaseline( -bound.top() );
    //setMidline( getBaseline() - fm.strikeOutPos() );

    if ( m_tab ) {
        getParent()->registerTab( this );
    }
}

void SpaceElement::draw( QPainter& painter, const LuPixelRect& /*r*/,
                         const ContextStyle& context,
                         ContextStyle::TextStyle /*tstyle*/,
                         ContextStyle::IndexStyle /*istyle*/,
                         StyleAttributes& /*style*/,
                         const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos(parentOrigin.x()+getX(), parentOrigin.y()+getY());
    // there is such a thing as negative space!
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    if ( context.edit() ) {
        painter.setPen( context.getEmptyColor() );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          context.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight()-getHeight()/5 ) );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight() ),
                          context.layoutUnitToPixelX( myPos.x()+getWidth()-1 ),
                          context.layoutUnitToPixelY( myPos.y()+getHeight()-getHeight()/5 ) );
    }
}


void SpaceElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);
    switch ( spaceWidth ) {
    case NEGTHIN:
        element.setAttribute( "WIDTH", "negthin" );
        break;
    case THIN:
        element.setAttribute( "WIDTH", "thin" );
        break;
    case MEDIUM:
        element.setAttribute( "WIDTH", "medium" );
        break;
    case THICK:
        element.setAttribute( "WIDTH", "thick" );
        break;
    case QUAD:
        element.setAttribute( "WIDTH", "quad" );
        break;
    }
    if ( m_tab ) {
        element.setAttribute( "TAB", "true" );
    }
}

bool SpaceElement::readAttributesFromDom( QDomElement element )
{
    if ( !BasicElement::readAttributesFromDom( element ) ) {
        return false;
    }
    QString widthStr = element.attribute( "WIDTH" );
    if( !widthStr.isNull() ) {
        if ( widthStr.lower() == "quad" ) {
            spaceWidth = QUAD;
        }
        else if ( widthStr.lower() == "thick" ) {
            spaceWidth = THICK;
        }
        else if ( widthStr.lower() == "medium" ) {
            spaceWidth = MEDIUM;
        }
        else if ( widthStr.lower() == "negthin" ) {
            spaceWidth = NEGTHIN;
        }
        else {
            spaceWidth = THIN;
        }
    }
    else {
        return false;
    }
    QString tabStr = element.attribute( "TAB" );
    m_tab = !tabStr.isNull();
    return true;
}

bool SpaceElement::readContentFromDom(QDomNode& node)
{
    return BasicElement::readContentFromDom( node );
}

bool SpaceElement::readAttributesFromMathMLDom(const QDomElement& element)
{
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    QString widthStr = element.attribute( "width" ).stripWhiteSpace().lower();
    if ( ! widthStr.isNull() ) {
        m_width = getSize( widthStr, &m_widthType );
        if ( m_widthType == NoSize ) {
            m_widthType = getSpace( widthStr );
        }
    }
    QString heightStr = element.attribute( "height" ).stripWhiteSpace().lower();
    if ( ! heightStr.isNull() ) {
        m_height = getSize( heightStr, &m_heightType );
    }
    QString depthStr = element.attribute( "depth" ).stripWhiteSpace().lower();
    if ( ! depthStr.isNull() ) {
        m_depth = getSize( depthStr, &m_depthType );
    }
    QString linebreakStr = element.attribute( "linebreak" ).stripWhiteSpace().lower();
    if ( ! linebreakStr.isNull() ) {
        if ( linebreakStr == "auto" ) {
            m_lineBreak = AutoBreak;
        }
        else if ( linebreakStr == "newline" ) {
            m_lineBreak = NewLineBreak;
        }
        else if ( linebreakStr == "indentingnewline" ) {
            m_lineBreak = IndentingNewLineBreak;
        }
        else if ( linebreakStr == "nobreak" ) {
            m_lineBreak = NoBreak;
        }
        else if ( linebreakStr == "goodbreak" ) {
            m_lineBreak = GoodBreak;
        }
        else if ( linebreakStr == "badbreak" ) {
            m_lineBreak = BadBreak;
        }
    }
}

void SpaceElement::writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat )
{

    QDomElement de = doc.createElement( oasisFormat ? "math:mspace" : "mspace" );
    QString width;

    switch ( spaceWidth ) {
    case NEGTHIN:
        width = "-3/18em";
        break;
    case THIN:
        width = "thinmathspace";
        break;
    case MEDIUM:
        width = "mediummathspace";
        break;
    case THICK:
        width = "thickmathspace";
        break;
    case QUAD:
        width = "veryverythickmathspace"; // double 'very' is appropriate.
        break;
    }

    de.setAttribute( "width", width );

    parent.appendChild( de );


    /* // worked, but I redecided.
    switch ( spaceWidth )
    {
    case NEGTHIN:
        return doc.createEntityReference( "NegativeThinSpace" );
    case THIN:
        return doc.createEntityReference( "ThinSpace" );
    case MEDIUM:
        return doc.createEntityReference( "MediumSpace" );
    case THICK:
        return doc.createEntityReference( "ThickSpace" );
    case QUAD:
        //return doc.createEntityReference( "Space" ); // misused &Space;???
        QDomElement de = doc.createElement( "mspace" );
        de.setAttribute( "width", "veryverythickmathspace" );
        return de;
    }*/

}

QString SpaceElement::toLatex()
{
    switch ( spaceWidth ) {
    case NEGTHIN: return "\\!";
    case THIN:    return "\\,";
    case MEDIUM:  return "\\>";
    case THICK:   return "\\;";
    case QUAD:    return "\\quad ";
    }
    return "";
}

KFORMULA_NAMESPACE_END
