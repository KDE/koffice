/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
   Copyright (C) 2001 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "MatrixRowElement.h"
#include "MatrixEntryElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>

#include <klocale.h>

#include <QPainter>
#include <QList>

MatrixRowElement::MatrixRowElement( BasicElement* parent ) : BasicElement( parent )
{
    m_matrixEntryElements.append( new MatrixEntryElement( this ) );
}

MatrixRowElement::~MatrixRowElement()
{
}

int MatrixRowElement::positionOfEntry( BasicElement* entry ) const
{
    for( int i = 0; i < m_matrixEntryElements.count(); i++ )
         if( m_matrixEntryElements[ i ] == entry )
             return i;
    return 0;
}

MatrixEntryElement* MatrixRowElement::entryAt( int pos )
{
    return m_matrixEntryElements[ pos ];
}

void MatrixRowElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
}

void MatrixRowElement::removeChild( BasicElement* element )
{
}

const QList<BasicElement*> MatrixRowElement::childElements()
{
    QList<BasicElement*> tmp;
    foreach( MatrixEntryElement* element, m_matrixEntryElements )
        tmp.append( element );

    return tmp;
}

void MatrixRowElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )   // coming from the parent go to the very right entry
        m_matrixEntryElements.last()->moveLeft( cursor, this );
    else                            // coming from a child go to the parent
        parentElement()->moveLeft( cursor, from );
}

void MatrixRowElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if( from == parentElement() )
        m_matrixEntryElements.first()->moveRight( cursor, this );
    else
        parentElement()->moveRight( cursor, this );
}

void MatrixRowElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    parentElement()->moveUp( cursor, from );   // just forward the call to MatrixElement   
}

void MatrixRowElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    parentElement()->moveDown( cursor, from ); // just forward the call to MatrixElement
}

bool MatrixRowElement::readMathMLContent( const KoXmlElement& element )
{
    MatrixEntryElement* tmpEntry = 0;
    KoXmlElement tmp;
    forEachElement( tmp, element )
    {
        tmpEntry = new MatrixEntryElement( this );
	m_matrixEntryElements << tmpEntry;
	tmpEntry->readMathML( tmp );
    }

    return true;
}

void MatrixRowElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    foreach( MatrixEntryElement* tmpEntry, m_matrixEntryElements )
        tmpEntry->writeMathML( writer );
}



#if 0
void MatrixRowElement::calcSizes( const ContextStyle& context,
                                  ContextStyle::TextStyle tstyle,
                                  ContextStyle::IndexStyle istyle,
                                  StyleAttributes& style )
{
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QFont font = context.getDefaultFont();
    font.setPointSizeF( context.layoutUnitPtToPt( mySize ) );
    QFontMetrics fm( font );
    luPixel leading = context.ptToLayoutUnitPt( fm.leading() );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );

    int count = m_matrixEntryElements.count();
    luPixel height = -leading;
    luPixel width = 0;
    int tabCount = 0;
    for ( int i = 0; i < count; ++i ) {
        MatrixEntryElement* line = m_matrixEntryElements[i];
        line->calcSizes( context, tstyle, istyle, style );
        tabCount = qMax( tabCount, line->tabCount() );

        height += leading;
        line->setX( 0 );
        line->setY( height );
        height += line->getHeight() + distY;
        width = qMax( line->getWidth(), width );
    }

    // calculate the tab positions
    for ( int t = 0; t < tabCount; ++t ) {
        luPixel pos = 0;
        for ( int i = 0; i < count; ++i ) {
            MatrixEntryElement* line = m_matrixEntryElements[i];
            if ( t < line->tabCount() ) {
                pos = qMax( pos, line->tab( t )->getX() );
            }
            else {
                pos = qMax( pos, line->getWidth() );
            }
        }
        for ( int i = 0; i < count; ++i ) {
            MatrixEntryElement* line = m_matrixEntryElements[i];
            if ( t < line->tabCount() ) {
                line->moveTabTo( t, pos );
                width = qMax( width, line->getWidth() );
            }
        }
    }

    setHeight( height );
    setWidth( width );
    if ( count == 1 ) {
        setBaseline( m_matrixEntryElements.at( 0 )->getBaseline() );
    }
    else {
        // There's always a first line. No formulas without lines.
        setBaseline( height/2 + context.axisHeight( tstyle, factor ) );
    }
}

void MatrixRowElement::draw( QPainter& painter, const LuPixelRect& r,
                             const ContextStyle& context,
                             ContextStyle::TextStyle tstyle,
                             ContextStyle::IndexStyle istyle,
                             StyleAttributes& style,
                             const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );
    int count = m_matrixEntryElements.count();

    if ( context.edit() ) {
        int tabCount = 0;
        painter.setPen( context.getHelpColor() );
        for ( int i = 0; i < count; ++i ) {
            MatrixEntryElement* line = m_matrixEntryElements[i];
            if ( tabCount < line->tabCount() ) {
                for ( int t = tabCount; t < line->tabCount(); ++t ) {
                    BasicElement* marker = line->tab( t );
                    painter.drawLine( context.layoutUnitToPixelX( myPos.x()+marker->getX() ),
                                      context.layoutUnitToPixelY( myPos.y() ),
                                      context.layoutUnitToPixelX( myPos.x()+marker->getX() ),
                                      context.layoutUnitToPixelY( myPos.y()+getHeight() ) );
                }
                tabCount = line->tabCount();
            }
        }
    }

    for ( int i = 0; i < count; ++i ) {
        MatrixEntryElement* line = m_matrixEntryElements[i];
        line->draw( painter, r, context, tstyle, istyle, style, myPos );
    }
}
#endif



