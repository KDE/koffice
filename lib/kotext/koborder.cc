/* This file is part of the KDE project
   Copyright (C) 2000, 2001 Thomas Zander <zander@kde.org>

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

#include "koborder.h"
#include <qdom.h>
#include <kdebug.h>
#include "kozoomhandler.h"
//#include "kotextformat.h"
#include "qrichtext_p.h" // for KoTextFormat

KoBorder::KoBorder()
    : color(), style( SOLID )
{
    setPenWidth( 1 );
}

KoBorder::KoBorder( const QColor & c, BorderStyle s, double width )
    : color( c ), style( s )
{
    setPenWidth( width );
}

bool KoBorder::operator==( const KoBorder _brd ) const {
    return ( style == _brd.style && color == _brd.color && ptPenWidth == _brd.ptPenWidth );
}

bool KoBorder::operator!=( const KoBorder _brd ) const {
    return ( style != _brd.style || color != _brd.color || ptPenWidth != _brd.ptPenWidth );
}

void KoBorder::setStyle(BorderStyle _style)
{
    style = _style;
    setPenWidth(ptPenWidth);
}

void KoBorder::setPenWidth(double _w)
{
    ptPenWidth = _w;
    if ( style==KoBorder::DOUBLE_LINE)
    {
        ptWidth = 2 * ptPenWidth + 1;
    }
    else
        ptWidth = _w;
}

QPen KoBorder::borderPen( const KoBorder & _brd, int width, QColor defaultColor )
{
    QPen pen( _brd.color, width );
    if ( !_brd.color.isValid() )
        pen.setColor( defaultColor );

    switch ( _brd.style ) {
    case KoBorder::SOLID:
    case KoBorder::DOUBLE_LINE:
        pen.setStyle( SolidLine );
        break;
    case KoBorder::DASH:
        pen.setStyle( DashLine );
        break;
    case KoBorder::DOT:
        pen.setStyle( DotLine );
        break;
    case KoBorder::DASH_DOT:
        pen.setStyle( DashDotLine );
        break;
    case KoBorder::DASH_DOT_DOT:
        pen.setStyle( DashDotDotLine );
        break;
    }

    return pen;
}

KoBorder KoBorder::loadBorder( const QDomElement & elem )
{
    KoBorder bd;
    if ( elem.hasAttribute("red") )
    {
        int r = elem.attribute("red").toInt();
        int g = elem.attribute("green").toInt();
        int b = elem.attribute("blue").toInt();
        bd.color.setRgb( r, g, b );
    }
    bd.style = static_cast<BorderStyle>( elem.attribute("style").toInt() );
    bd.setPenWidth( elem.attribute("width").toDouble() );
    return bd;
}

void KoBorder::save( QDomElement & elem ) const
{
    if (color.isValid()) {
        elem.setAttribute("red", color.red());
        elem.setAttribute("green", color.green());
        elem.setAttribute("blue", color.blue());
    }
    elem.setAttribute("style", static_cast<int>( style ));
    elem.setAttribute("width", ptPenWidth);
}

KoBorder::BorderStyle KoBorder::getStyle( const QString &style )
{
    if ( style == "___ ___ __" )
        return KoBorder::DASH;
    if ( style == "_ _ _ _ _ _" )
        return KoBorder::DOT;
    if ( style == "___ _ ___ _" )
        return KoBorder::DASH_DOT;
    if ( style == "___ _ _ ___" )
        return KoBorder::DASH_DOT_DOT;
    if ( style == "===========" )
        return KoBorder::DOUBLE_LINE;
    // default
    return KoBorder::SOLID;
}

QString KoBorder::getStyle( const BorderStyle &style )
{
    switch ( style )
    {
    case KoBorder::SOLID:
        return "_________";
    case KoBorder::DASH:
        return "___ ___ __";
    case KoBorder::DOT:
        return "_ _ _ _ _ _";
    case KoBorder::DASH_DOT:
        return "___ _ ___ _";
    case KoBorder::DASH_DOT_DOT:
        return "___ _ _ ___";
    case KoBorder::DOUBLE_LINE:
        return "===========";
    }

    // Keep compiler happy.
    return "_________";
}

int KoBorder::zoomWidthX( double ptWidth, KoZoomHandler * zoomHandler, int minborder )
{
    // If a border was set, then zoom it and apply a minimum of 1, so that it's always visible.
    // If no border was set, apply minborder ( 0 for paragraphs, 1 for frames )
    return ptWidth > 0 ? QMAX( 1, zoomHandler->zoomItX( ptWidth ) /*applies qRound*/ ) : minborder;
}

int KoBorder::zoomWidthY( double ptWidth, KoZoomHandler * zoomHandler, int minborder )
{
    // If a border was set, then zoom it and apply a minimum of 1, so that it's always visible.
    // If no border was set, apply minborder ( 0 for paragraphs, 1 for frames )
    return ptWidth > 0 ? QMAX( 1, zoomHandler->zoomItY( ptWidth ) /*applies qRound*/ ) : minborder;
}

void KoBorder::drawBorders( QPainter& painter, KoZoomHandler * zoomHandler, QRect rect, KoBorder leftBorder, KoBorder rightBorder, KoBorder topBorder, KoBorder bottomBorder, int minborder, QPen defaultPen )
{
    int topBorderWidth = zoomWidthY( topBorder.width(), zoomHandler, minborder );
    int bottomBorderWidth = zoomWidthY( bottomBorder.width(), zoomHandler, minborder );
    int leftBorderWidth = zoomWidthX( leftBorder.width(), zoomHandler, minborder );
    int rightBorderWidth = zoomWidthX( rightBorder.width(), zoomHandler, minborder );

    int topBorderPenWidth = zoomWidthY( topBorder.penWidth(), zoomHandler, minborder );
    int bottomBorderPenWidth = zoomWidthY( bottomBorder.penWidth(), zoomHandler, minborder );
    int leftBorderPenWidth = zoomWidthX( leftBorder.penWidth(), zoomHandler, minborder );
    int rightBorderPenWidth = zoomWidthX( rightBorder.penWidth(), zoomHandler, minborder );

    //kdDebug(32500) << "KoBorder::drawBorders top=" << topBorderWidth << " bottom=" << bottomBorderWidth
    //          << " left=" << leftBorderWidth << " right=" << rightBorderWidth << endl;

    QColor defaultColor = KoTextFormat::defaultTextColor( &painter );

    if ( topBorderWidth > 0 )
    {
        if ( topBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( topBorder, topBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
        int y = rect.top() - topBorderWidth + topBorderWidth/2;
        if ( topBorder.style==KoBorder::DOUBLE_LINE)
        {
            y = rect.top() - topBorderWidth + topBorderPenWidth/2;
            painter.drawLine( rect.left()- 2*leftBorderPenWidth-1, y, rect.right()+4*rightBorderPenWidth, y );
            y += topBorderPenWidth + 1;
            painter.drawLine( rect.left()-leftBorderPenWidth, y, rect.right()+rightBorderPenWidth, y );
        }
        else
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+rightBorderWidth, y );
    }
    if ( bottomBorderWidth > 0 )
    {
        if ( bottomBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( bottomBorder, bottomBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
	//kdDebug(32500) << "bottomBorderWidth=" << bottomBorderWidth << " bottomBorderWidth/2=" << (int)bottomBorderWidth/2 << endl;
        int y = rect.bottom() + bottomBorderWidth - (bottomBorderWidth-1)/2;
	//kdDebug(32500) << "   -> bottom=" << rect.bottom() << " y=" << y << endl;
        if ( bottomBorder.style==KoBorder::DOUBLE_LINE)
        {
            y = rect.bottom() + bottomBorderWidth - (bottomBorderPenWidth-1)/2;
            painter.drawLine( rect.left()-2*leftBorderPenWidth-1, y, rect.right()+4*rightBorderPenWidth, y );
            y-= bottomBorderPenWidth + 1;
            painter.drawLine( rect.left()-leftBorderPenWidth, y, rect.right()+rightBorderPenWidth, y );

        }
        else
            painter.drawLine( rect.left()-leftBorderWidth, y, rect.right()+rightBorderWidth, y );
    }
    if ( leftBorderWidth > 0 )
    {
        if ( leftBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( leftBorder, leftBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
        int x = rect.left() - leftBorderWidth + leftBorderWidth/2;
        if ( leftBorder.style==KoBorder::DOUBLE_LINE)
        {
            x = rect.left() - leftBorderWidth + leftBorderPenWidth/2;
            painter.drawLine( x, rect.top()-2*topBorderPenWidth, x, rect.bottom()+2*bottomBorderPenWidth );
            x+= leftBorderPenWidth + 1;
            painter.drawLine( x, rect.top()-topBorderPenWidth, x, rect.bottom()+bottomBorderPenWidth );
        }
        else
            painter.drawLine( x, rect.top()-topBorderWidth, x, rect.bottom()+bottomBorderWidth );
    }
    if ( rightBorderWidth > 0 )
    {
        if ( rightBorder.penWidth() > 0 )
            painter.setPen( KoBorder::borderPen( rightBorder, rightBorderPenWidth, defaultColor ) );
        else
            painter.setPen( defaultPen );
        int x = rect.right() + rightBorderWidth - (rightBorderWidth-1)/2;
        if ( rightBorder.style==KoBorder::DOUBLE_LINE)
        {
            x = rect.right() + rightBorderWidth - (rightBorderPenWidth-1)/2;
            painter.drawLine( x, rect.top()-2*topBorderPenWidth, x, rect.bottom()+2*bottomBorderPenWidth );
            x-= leftBorderPenWidth + 1;
            painter.drawLine( x, rect.top()-topBorderPenWidth, x, rect.bottom()+bottomBorderPenWidth );

        }
        else
            painter.drawLine( x, rect.top()-topBorderWidth, x, rect.bottom()+bottomBorderWidth );
    }
}
