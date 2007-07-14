/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "MatrixElement.h"
#include "MatrixRowElement.h"
#include "MatrixEntryElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <QPainter>

MatrixElement::MatrixElement( BasicElement* parent ) : BasicElement( parent )
{
}

MatrixElement::~MatrixElement()
{
}

void MatrixElement::paint( QPainter& painter ) const
{
    // TODO paint the frame, rowlines, columnlines
    // eventually paint the frame
}

void MatrixElement::layout( const AttributeManager* am )
{
    // TODO implement rowspacing
    double tmpHeight = 0.0;
    double tmpWidth = 0.0;
    QPointF tmpOrigin = origin();
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )
    {
        tmpRow->layout( am );
        tmpWidth = qMax( tmpRow->width(), tmpWidth );
	tmpHeight += tmpRow->height();
	tmpRow->setOrigin( tmpOrigin );
	tmpOrigin = origin() + QPointF( 0, tmpHeight ); 
    }
    setHeight( tmpHeight );
    setWidth( tmpWidth );
    setBaseLine( tmpHeight/2 + parentElement()->height()/2 );
}

const QList<BasicElement*> MatrixElement::childElements()
{
    QList<BasicElement*> tmp;
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )
        tmp << tmpRow;
    return tmp;
}

void MatrixElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    /*
    if( from == parentElement() )
    {
        if( parentElement()->elementType() == Row )
            m_matrixRowElements.first()->moveRight( cursor, this ); // enter the matrix
	else
            cursor->setCursorTo( this, 0 );
    }
    else if( cursor->currentElement() == this )
        m_matrixRowElements.first()->moveRight( cursor, this );
    else
        parentElement()->moveRight( cursor, this );
    */
}

void MatrixElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    /*
    if( from == parentElement() )
    {
        if( parentElement()->elementType() == Row )
            m_matrixRowElements.last()->moveLeft( cursor, this ); // enter the matrix
	else
            cursor->setCursorTo( this, 1 );
    }
    else if( cursor->currentElement() == this )
        m_matrixRowElements.last()->moveLeft( cursor, this );
    else
        parentElement()->moveLeft( cursor, this );
    */
}

void MatrixElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    /*
    if( cursor->currentElement() == this )
        parentElement()->moveDown( cursor, this );
    else if( cursor->currentElement()->elementType() == MatrixEntry )
    {
        int row = indexOfRow( from );
        int pos = m_matrixRowElements[row]->positionOfEntry( cursor->currentElement() );
	if( m_matrixRowElements.count() > row++ )
            cursor->setCursorTo( m_matrixRowElements[ row++ ]->entryAt( pos ), 0 );
    }
    */
}

void MatrixElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    /*
    if( cursor->currentElement() == this )
        parentElement()->moveUp( cursor, this );
    else if( cursor->currentElement()->elementType() == MatrixEntry )
    {
        int row = indexOfRow( from );
        int pos = m_matrixRowElements[row]->positionOfEntry( cursor->currentElement() );
	if( 0 < row )
            cursor->setCursorTo( m_matrixRowElements[ row-- ]->entryAt( pos ), 0 );
    }
    */
}

bool MatrixElement::readMathMLContent( const KoXmlElement& element )
{  
    MatrixRowElement* tmpElement = 0;
    KoXmlElement tmp;
    forEachElement( tmp, element )   // iterate over the elements
    {
        tmpElement = new MatrixRowElement( this );
        m_matrixRowElements << tmpElement;
	tmpElement->readMathML( tmp );
    }

    return true;
}

void MatrixElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    foreach( MatrixRowElement* tmpRow, m_matrixRowElements )  // write each mtr element
	tmpRow->writeMathML( writer );
}

int MatrixElement::indexOfRow( BasicElement* row ) const
{
    for( int i = 0; i < m_matrixRowElements.count(); i++ )
        if( m_matrixRowElements[ i ] == row )
            return i;
    return 0;
}
/*
int MatrixElement::rows() const
{
    return m_matrixRowElements.count();
}

int MatrixElement::cols() const
{
    return m_matrixRowElements[ 0 ]->childElements().count();
}

MatrixEntryElement* MatrixElement::matrixEntryAt( int row, int col )
{
    return m_matrixRowElements[ row ]->entryAtPosition( col );
}
*/
/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void MatrixElement::goInside(FormulaCursor* cursor)
{
    //matrixEntryAt(0, 0)->goInside(cursor);
}

void MatrixElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
/*    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
            if (child == matrixEntryAt(r, c))
                cursor->setTo(this, r*cols()+c);
        }
    }*/
}
/*
bool MatrixElement::searchElement(BasicElement* element, int& row, int& column)
{
    for (int r = 0; r < rows(); r++) {
        for (int c = 0; c < cols(); c++) {
            if (element == matrixEntryAt(r, c)) {
                row = r;
                column = c;
                return true;
            }
        }
    }
    return false;
}
*/
