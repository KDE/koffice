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
 * Boston, MA 02110-1301, USA.
*/

#include "basicelement.h"
#include "styleelement.h"

KFORMULA_NAMESPACE_BEGIN

StyleElement::StyleElement( BasicElement* parent ) : TokenStyleElement( parent ),
                                                     m_scriptMinSizeType( NoSize ),
                                                     m_veryVeryThinMathSpaceType( NoSize ),
                                                     m_veryThinMathSpaceType( NoSize ),
                                                     m_thinMathSpaceType( NoSize ),
                                                     m_mediumMathSpaceType( NoSize ),
                                                     m_thickMathSpaceType( NoSize ),
                                                     m_veryThickMathSpaceType( NoSize ),
                                                     m_veryVeryThickMathSpaceType( NoSize ),
                                                     m_customDisplayStyle( false ),
                                                     m_customScriptSizeMultiplier( false ),
                                                     m_customBackground( false )
{
}

bool StyleElement::readAttributesFromMathMLDom( const QDomElement& element )
{
    if ( !BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }
    
    QString scriptlevelStr = element.attribute( "scriptlevel" );
    if ( ! scriptlevelStr.isNull() ) {
        bool ok;
        m_scriptLevel = scriptlevelStr.toUInt( &ok );
        if ( ! ok ) {
            kdWarning( DEBUGID ) << "Invalid scriptlevel attribute value: " 
                                 << scriptlevelStr << endl;
        }
        else {
            m_customScriptLevel = true;
        }
    }
    QString displaystyleStr = element.attribute( "displaystyle" );
    if ( ! displaystyleStr.isNull() ) {
        if ( displaystyleStr.lower() == "true" ) {
            m_displayStyle = true;
        }
        else {
            m_displayStyle = false;
        }
        m_customDisplayStyle = true;
    }
    QString scriptsizemultiplierStr = element.attribute( "scriptsizemultiplier" );
    if ( ! scriptsizemultiplierStr.isNull() ) {
        bool ok;
        m_scriptSizeMultiplier = scriptsizemultiplierStr.toDouble( &ok );
        if ( ! ok ) {
            kdWarning( DEBUGID ) << "Invalid scriptsizemultiplier attribute value: " 
                                 << scriptsizemultiplierStr << endl;
        }
        else {
            m_customScriptSizeMultiplier = true;
        }
    }
    QString scriptminsizeStr = element.attribute( "scriptminsize" );
    if ( ! scriptminsizeStr.isNull() ) {
        readSizeAttribute( scriptminsizeStr, &m_scriptMinSizeType, &m_scriptMinSize );
    }
    QString backgroundStr = element.attribute( "background" );
    if ( ! backgroundStr.isNull() ) {
        // TODO: tranparent background
        m_customBackground = true;
        if ( backgroundStr[0] != '#' ) {
            m_background = QColor( getHtmlColor( backgroundStr ) );
        }
        else {
            m_background = QColor( backgroundStr );
        }
    }
    QString veryverythinmathspaceStr = element.attribute( "veryverythinmathspace" );
    if ( ! veryverythinmathspaceStr.isNull() ) {
        readSizeAttribute( veryverythinmathspaceStr, &m_veryVeryThinMathSpaceType, &m_veryVeryThinMathSpace );
    }
    QString verythinmathspaceStr = element.attribute( "verythinmathspace" );
    if ( ! verythinmathspaceStr.isNull() ) {
        readSizeAttribute( verythinmathspaceStr, &m_veryThinMathSpaceType, &m_veryThinMathSpace );
    }
    QString thinmathspaceStr = element.attribute( "thinmathspace" );
    if ( ! thinmathspaceStr.isNull() ) {
        readSizeAttribute( thinmathspaceStr, &m_thinMathSpaceType, &m_thinMathSpace );
    }
    QString mediummathspaceStr = element.attribute( "mediummathspace" );
    if ( ! mediummathspaceStr.isNull() ) {
        readSizeAttribute( mediummathspaceStr, &m_mediumMathSpaceType, &m_mediumMathSpace );
    }
    QString thickmathspaceStr = element.attribute( "thickmathspace" );
    if ( ! thickmathspaceStr.isNull() ) {
        readSizeAttribute( thickmathspaceStr, &m_thickMathSpaceType, &m_thickMathSpace );
    }
    QString verythickmathspaceStr = element.attribute( "verythickmathspace" );
    if ( ! verythickmathspaceStr.isNull() ) {
        readSizeAttribute( verythickmathspaceStr, &m_veryThickMathSpaceType, &m_veryThickMathSpace );
    }
    QString veryverythickmathspaceStr = element.attribute( "veryverythickmathspace" );
    if ( ! veryverythickmathspaceStr.isNull() ) {
        readSizeAttribute( veryverythickmathspaceStr, &m_veryVeryThickMathSpaceType, &m_veryVeryThickMathSpace );
    }
    return inherited::readAttributesFromMathMLDom( element );
}

void StyleElement::writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat )
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mstyle" : "mstyle" );
    writeMathMLAttributes( de );
    SequenceElement::writeMathML( doc, de, oasisFormat );
    parent.appendChild( de );
}

void StyleElement::writeMathMLAttributes( QDomElement& element )
{
    if ( m_customScriptLevel ) {
        element.setAttribute( "scriptlevel", QString( "%1" ).arg( m_scriptLevel ) );
    }
    if ( m_customDisplayStyle ) {
        element.setAttribute( "displaystyle", m_displayStyle ? "true" : "false" );
    }
    if ( m_customScriptSizeMultiplier ) {
        element.setAttribute( "scriptsizemultiplier", QString( "%1" ).arg( m_scriptSizeMultiplier ) );
    }
    writeSizeAttribute( element, "scriptminsize", m_scriptMinSizeType, m_scriptMinSize );
    if ( m_customBackground ) {
        element.setAttribute( "background", m_background.name() );
    }
    writeSizeAttribute( element, "veryverythinmathspace", m_veryVeryThinMathSpaceType, m_veryVeryThinMathSpace );
    writeSizeAttribute( element, "verythinmathspace", m_veryThinMathSpaceType, m_veryThinMathSpace );
    writeSizeAttribute( element, "thinmathspace", m_thinMathSpaceType, m_thinMathSpace );
    writeSizeAttribute( element, "mediummathspace", m_mediumMathSpaceType, m_mediumMathSpace );
    writeSizeAttribute( element, "thickmathspace", m_thickMathSpaceType, m_thickMathSpace );
    writeSizeAttribute( element, "verythickmathspace", m_veryThickMathSpaceType, m_veryThickMathSpace );
    writeSizeAttribute( element, "veryverythickmathspace", m_veryVeryThickMathSpaceType, m_veryVeryThickMathSpace );

    inherited::writeMathMLAttributes( element );
}

void StyleElement::setStyleBackground( StyleAttributes& style )
{
    if ( customMathBackground() ) {
        style.setBackground( mathBackground() );
    }
    else if ( m_customBackground ) {
        style.setBackground( m_background );
    }
    else {
        style.setBackground( style.background() );
    }
}

void StyleElement::readSizeAttribute( const QString& str, SizeType* st, double* s )
{
    if ( st == 0 || s == 0 ){
        return;
    }
    if ( str == "small" ) {
        *st = RelativeSize;
        *s = 0.8; // ### Arbitrary size
    }
    else if ( str == "normal" ) {
        *st = RelativeSize;
        *s = 1.0;
    }
    else if ( str == "big" ) {
        *st = RelativeSize;
        *s = 1.2; // ### Arbitrary size
    }
    else {
        *s = getSize( str, st );
    }
}

void StyleElement::writeSizeAttribute( QDomElement element, const QString& str, SizeType st, double s )
{
    switch ( st ) {
    case AbsoluteSize:
        element.setAttribute( str, QString( "%1pt" ).arg( s ) );
        break;
    case RelativeSize:
        element.setAttribute( str, QString( "%1%" ).arg( s * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( str, QString( "%3px" ).arg( s ) );
        break;
    }
}


KFORMULA_NAMESPACE_END
