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

#include <qfontmetrics.h>
#include <qstring.h>

#include <kdebug.h>
#include <koGlobal.h>

#include "contextstyle.h"

KFORMULA_NAMESPACE_BEGIN


ContextStyle::ContextStyle()
    : symbolFont( "Symbol" ),
      defaultColor(Qt::black), numberColor(Qt::blue),
      operatorColor(Qt::darkGreen), errorColor(Qt::darkRed),
      emptyColor(Qt::blue)
{
//     kdDebug() << "ContextStyle::ContextStyle" << endl
//               << "defaultFont: " << defaultFont.rawName() << endl
//               << "nameFont: " << nameFont.rawName() << endl
//               << "numberFont: " << numberFont.rawName() << endl
//               << "operatorFont: " << operatorFont.rawName() << endl
//               << "symbolFont: " << symbolFont.rawName() << endl;

    textStyleValues[ displayStyle      ].setup( 1. );
    textStyleValues[ textStyle         ].setup( 1. );
    textStyleValues[ scriptStyle       ].setup( .7 );
    textStyleValues[ scriptScriptStyle ].setup( .49 );

    m_baseTextStyle = displayStyle;

    lineWidth = 1;
    linearMovement = false;
    centerSymbol = false;
    m_syntaxHighlighting = true;
}

void ContextStyle::init()
{
    table.init();
    setup();
}

void ContextStyle::readConfig( KConfig* config )
{
    config->setGroup( "kformula Font" );
    QString fontName = config->readEntry( "defaultFont", "Times,12,-1,5,50,1,0,0,0,0" );
    defaultFont.fromString( fontName );
    fontName = config->readEntry( "nameFont", "Times,12,-1,5,50,0,0,0,0,0" );
    nameFont.fromString( fontName );
    fontName = config->readEntry( "numberFont", "Times,12,-1,5,50,0,0,0,0,0" );
    numberFont.fromString( fontName );
    fontName = config->readEntry( "operatorFont", "Times,12,-1,5,50,0,0,0,0,0" );
    operatorFont.fromString( fontName );
    //fontName = config->readEntry( "symbolFont", "times,12,-1,5,50,0,0,0,0,0" );
    //symbolFont.fromString( fontName );
    QString baseSize = config->readEntry( "baseSize", "20" );
    m_baseSize = baseSize.toDouble();

    config->setGroup( "kformula Color" );
    defaultColor  = config->readColorEntry( "defaultColor",  &defaultColor );
    numberColor   = config->readColorEntry( "numberColor",   &numberColor );
    operatorColor = config->readColorEntry( "operatorColor", &operatorColor );
    emptyColor    = config->readColorEntry( "emptyColor",    &emptyColor );
    errorColor    = config->readColorEntry( "errorColor",    &errorColor );
    m_syntaxHighlighting = config->readBoolEntry( "syntaxHighlighting", true );
}

bool ContextStyle::setZoom( double zoomX, double zoomY, bool, bool )
{
    bool changes = m_zoomedResolutionX != zoomX || m_zoomedResolutionY != zoomY;
    m_zoom = 100;
    m_zoomedResolutionX = zoomX;
    m_zoomedResolutionY = zoomY;
    return changes;
}

QColor ContextStyle::getNumberColor()   const
{
    if ( edit() && syntaxHighlighting() ) {
        return numberColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getOperatorColor() const
{
    if ( edit() && syntaxHighlighting() ) {
        return operatorColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getErrorColor()    const
{
    if ( edit() && syntaxHighlighting() ) {
        return errorColor;
    }
    return getDefaultColor();
}

QColor ContextStyle::getEmptyColor()    const
{
    if ( edit() && syntaxHighlighting() ) {
        return emptyColor;
    }
    return getDefaultColor();
}

void ContextStyle::setDefaultColor( const QColor& color )
{
    defaultColor = color;
}
void ContextStyle::setNumberColor( const QColor& color )
{
    numberColor = color;
}
void ContextStyle::setOperatorColor( const QColor& color )
{
    operatorColor = color;
}
void ContextStyle::setErrorColor( const QColor& color )
{
    errorColor = color;
}
void ContextStyle::setEmptyColor( const QColor& color )
{
    emptyColor = color;
}

double ContextStyle::getReductionFactor( TextStyle tstyle ) const
{
    return textStyleValues[ tstyle ].reductionFactor;
}

luPt ContextStyle::getAdjustedSize( TextStyle tstyle ) const
{
    return static_cast<luPt>( ptToLayoutUnitPt( m_baseSize*getReductionFactor( tstyle ) ) );
}

luPixel ContextStyle::getSpace( TextStyle tstyle, SpaceWidth space ) const
{
    switch ( space ) {
    case THIN:   return getThinSpace( tstyle );
    case MEDIUM: return getMediumSpace( tstyle );
    case THICK:  return getThickSpace( tstyle );
    case QUAD:   return getQuadSpace( tstyle );
    }
    return 0;
}

luPixel ContextStyle::getThinSpace( TextStyle tstyle ) const
{
    return ptToPixelX( textStyleValues[ tstyle ].thinSpace( quad ) );
}

luPixel ContextStyle::getMediumSpace( TextStyle tstyle ) const
{
    return ptToPixelX( textStyleValues[ tstyle ].mediumSpace( quad ) );
}

luPixel ContextStyle::getThickSpace( TextStyle tstyle ) const
{
    return ptToPixelX( textStyleValues[ tstyle ].thickSpace( quad ) );
}

luPixel ContextStyle::getQuadSpace( TextStyle tstyle ) const
{
    return ptToPixelX( textStyleValues[ tstyle ].quadSpace( quad ) );
}

luPixel ContextStyle::axisHeight( TextStyle tstyle ) const
{
    return ptToPixelY( textStyleValues[ tstyle ].axisHeight( m_axisHeight ) );
}

luPt ContextStyle::getBaseSize() const
{
    return static_cast<luPt>( ptToLayoutUnitPt( m_baseSize ) );
}

void ContextStyle::setBaseSize( pt size )
{
    //kdDebug( 40000 ) << "ContextStyle::setBaseSize" << endl;
    if ( size != m_baseSize ) {
        m_baseSize = size;
        setup();
    }
}


luPixel ContextStyle::getLineWidth() const
{
    return ptToLayoutUnitPixX( lineWidth );
}

luPixel ContextStyle::getEmptyRectWidth() const
{
    return ptToLayoutUnitPixX( m_baseSize/1.8 );
}

luPixel ContextStyle::getEmptyRectHeight() const
{
    return ptToLayoutUnitPixX( m_baseSize/1.8 );
}


ContextStyle::TextStyle ContextStyle::convertTextStyleFraction( TextStyle tstyle ) const
{
    TextStyle result;

    switch ( tstyle ){
    case displayStyle:
	result = textStyle;
	break;
    case textStyle:
	result = scriptStyle;
	break;
    default:
	result = scriptScriptStyle;
	break;
    }

    return result;
}


ContextStyle::TextStyle ContextStyle::convertTextStyleIndex( TextStyle tstyle ) const
{
    TextStyle result;

    switch ( tstyle ){
    case displayStyle:
	result = scriptStyle;
	break;
    case textStyle:
	result = scriptStyle;
	break;
    default:
	result = scriptScriptStyle;
	break;
    }

    return result;
}


void ContextStyle::setup()
{
    luPt size = static_cast<luPt>( ptToLayoutUnitPt( m_baseSize ) );
    QFont font = symbolFont;
    font.setPointSize( size );
    QFontMetrics fm( font );

    // Or better the real space required? ( boundingRect )
    quad = fm.width( 'M' );
    m_axisHeight = fm.strikeOutPos();
}

KFORMULA_NAMESPACE_END
