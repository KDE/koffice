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

#ifndef MATRIXELEMENT_H
#define MATRIXELEMENT_H

#include "BasicElement.h"

class MatrixRowElement;
class MatrixEntryElement;
	
/**
 * @short A matrix or table element in a formula
 *
 * A matrix element contains a list of rows which are of class MatrixRowElement.
 * These rows contain single entries which are of class MatrixEntryElement. The
 * MatrixElement takes care that the different MatrixRowElements are informed how
 * to lay out their children correctly as they need to be synced.
 */
class MatrixElement : public BasicElement {
public:
    /// The standard constructor
    MatrixElement( BasicElement* parent = 0);
    
    /// The standard destructor
    ~MatrixElement();

    /**
     * Render the element to the given QPainter
     * @param painter The QPainter to paint the element to
     */
    void paint( QPainter& painter ) const;

    /// Calculate the element's sizes and the size of its children
    void calculateSize();
    
    /**
     * Obtain a list of all child elements of this element
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    /**
     * Move the FormulaCursor left
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveLeft( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor right 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveRight( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor up 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveUp( FormulaCursor* cursor, BasicElement* from );

    /**
     * Move the FormulaCursor down 
     * @param cursor The FormulaCursor to be moved
     * @param from The BasicElement which was the last owner of the FormulaCursor
     */
    void moveDown( FormulaCursor* cursor, BasicElement* from );

    /// Return the number of the rows of this matrix
/*    int rows() const;

    /// Return the number of the columns of this matrix
    int cols() const;
               
    /// Obtain a pointer to the element at @p row and @p col in the matrix
    MatrixEntryElement* matrixEntryAt( int row, int col );
  */  
    /**
     * Sets the cursor inside this element to its start position.
     * For most elements that is the main child.
     */
    virtual void goInside(FormulaCursor* cursor);

    /// Sets the cursor to select the child. The mark is palced after this element.
    virtual void selectChild( FormulaCursor*, BasicElement* );


protected:
    /// Read all content from the node - reimplemented by child elements
    bool readMathMLContent( const KoXmlElement& element );

    /// Write all content to the KoXmlWriter - reimplemented by the child elements
    void writeMathMLContent( KoXmlWriter* writer ) const;

private:
    /// @return The index of @p row in m_matrixRowElements
    int indexOfRow( BasicElement* row ) const;
    
    /// The rows a matrix contains
    QList<MatrixRowElement*> m_matrixRowElements;
};

#endif // MATRIXELEMENT_H
