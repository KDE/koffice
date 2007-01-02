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

#include <qmemarray.h>
#include <qpainter.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>

#include "MatrixDialog.h"
#include "elementvisitor.h"
#include "formulaelement.h"
#include "formulacursor.h"
#include "kformulacontainer.h"
#include "kformulacommand.h"
#include "matrixelement.h"
#include "sequenceelement.h"
#include "spaceelement.h"


KFORMULA_NAMESPACE_BEGIN


class MatrixSequenceElement : public SequenceElement {
    typedef SequenceElement inherited;
public:

    MatrixSequenceElement( BasicElement* parent = 0 ) : SequenceElement( parent ) {}
    virtual MatrixSequenceElement* clone() {
        return new MatrixSequenceElement( *this );
    }

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );
};


class KFCRemoveRow : public Command {
public:
    KFCRemoveRow( const QString& name, Container* document, MatrixElement* m, uint r, uint c );
    ~KFCRemoveRow();

    virtual void execute();
    virtual void unexecute();

protected:
    MatrixElement* matrix;
    uint rowPos;
    uint colPos;

    QPtrList<MatrixSequenceElement>* row;
};


class KFCInsertRow : public KFCRemoveRow {
public:
    KFCInsertRow( const QString& name, Container* document, MatrixElement* m, uint r, uint c );

    virtual void execute()   { KFCRemoveRow::unexecute(); }
    virtual void unexecute() { KFCRemoveRow::execute(); }
};


class KFCRemoveColumn : public Command {
public:
    KFCRemoveColumn( const QString& name, Container* document, MatrixElement* m, uint r, uint c );
    ~KFCRemoveColumn();

    virtual void execute();
    virtual void unexecute();

protected:
    MatrixElement* matrix;
    uint rowPos;
    uint colPos;

    QPtrList<MatrixSequenceElement>* column;
};


class KFCInsertColumn : public KFCRemoveColumn {
public:
    KFCInsertColumn( const QString& name, Container* document, MatrixElement* m, uint r, uint c );

    virtual void execute()   { KFCRemoveColumn::unexecute(); }
    virtual void unexecute() { KFCRemoveColumn::execute(); }
};


KCommand* MatrixSequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        return 0;
    }

    switch ( *request ) {
    case req_appendColumn:
    case req_appendRow:
    case req_insertColumn:
    case req_removeColumn:
    case req_insertRow:
    case req_removeRow: {
        MatrixElement* matrix = static_cast<MatrixElement*>( getParent() );
        FormulaCursor* cursor = container->activeCursor();
        for ( uint row = 0; row < matrix->getRows(); row++ ) {
            for ( uint col = 0; col < matrix->getColumns(); col++ ) {
                if ( matrix->getElement( row, col ) == cursor->getElement() ) {
                    switch ( *request ) {
                    case req_appendColumn:
                        return new KFCInsertColumn( i18n( "Append Column" ), container, matrix, row, matrix->getColumns() );
                    case req_appendRow:
                        return new KFCInsertRow( i18n( "Append Row" ), container, matrix, matrix->getRows(), col );
                    case req_insertColumn:
                        return new KFCInsertColumn( i18n( "Insert Column" ), container, matrix, row, col );
                    case req_removeColumn:
                        if ( matrix->getColumns() > 1 ) {
                            return new KFCRemoveColumn( i18n( "Remove Column" ), container, matrix, row, col );
                        }
                        break;
                    case req_insertRow:
                        return new KFCInsertRow( i18n( "Insert Row" ), container, matrix, row, col );
                    case req_removeRow:
                        if ( matrix->getRows() > 1 ) {
                            return new KFCRemoveRow( i18n( "Remove Row" ), container, matrix, row, col );
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        kdWarning( DEBUGID ) << "MatrixSequenceElement::buildCommand: Sequence not found." << endl;
        break;
    }
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}


KFCRemoveRow::KFCRemoveRow( const QString& name, Container* document, MatrixElement* m, uint r, uint c )
    : Command( name, document ), matrix( m ), rowPos( r ), colPos( c ), row( 0 )
{
}

KFCRemoveRow::~KFCRemoveRow()
{
    delete row;
}

void KFCRemoveRow::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    row = matrix->content.at( rowPos );
    FormulaElement* formula = matrix->formula();
    for ( uint i = matrix->getColumns(); i > 0; i-- ) {
        formula->elementRemoval( row->at( i-1 ) );
    }
    matrix->content.take( rowPos );
    formula->changed();
    if ( rowPos < matrix->getRows() ) {
        matrix->getElement( rowPos, colPos )->goInside( cursor );
    }
    else {
        matrix->getElement( rowPos-1, colPos )->goInside( cursor );
    }
    testDirty();
}

void KFCRemoveRow::unexecute()
{
    matrix->content.insert( rowPos, row );
    row = 0;
    FormulaCursor* cursor = getExecuteCursor();
    matrix->getElement( rowPos, colPos )->goInside( cursor );
    matrix->formula()->changed();
    testDirty();
}


KFCInsertRow::KFCInsertRow( const QString& name, Container* document, MatrixElement* m, uint r, uint c )
    : KFCRemoveRow( name, document, m, r, c )
{
    row = new QPtrList< MatrixSequenceElement >;
    row->setAutoDelete( true );
    for ( uint i = 0; i < matrix->getColumns(); i++ ) {
        row->append( new MatrixSequenceElement( matrix ) );
    }
}


KFCRemoveColumn::KFCRemoveColumn( const QString& name, Container* document, MatrixElement* m, uint r, uint c )
    : Command( name, document ), matrix( m ), rowPos( r ), colPos( c )
{
    column = new QPtrList< MatrixSequenceElement >;
    column->setAutoDelete( true );
}

KFCRemoveColumn::~KFCRemoveColumn()
{
    delete column;
}

void KFCRemoveColumn::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    FormulaElement* formula = matrix->formula();
    for ( uint i = 0; i < matrix->getRows(); i++ ) {
        column->append( matrix->getElement( i, colPos ) );
        formula->elementRemoval( column->at( i ) );
        matrix->content.at( i )->take( colPos );
    }
    formula->changed();
    if ( colPos < matrix->getColumns() ) {
        matrix->getElement( rowPos, colPos )->goInside( cursor );
    }
    else {
        matrix->getElement( rowPos, colPos-1 )->goInside( cursor );
    }
    testDirty();
}

void KFCRemoveColumn::unexecute()
{
    for ( uint i = 0; i < matrix->getRows(); i++ ) {
        matrix->content.at( i )->insert( colPos, column->take( 0 ) );
    }
    FormulaCursor* cursor = getExecuteCursor();
    matrix->getElement( rowPos, colPos )->goInside( cursor );
    matrix->formula()->changed();
    testDirty();
}


KFCInsertColumn::KFCInsertColumn( const QString& name, Container* document, MatrixElement* m, uint r, uint c )
    : KFCRemoveColumn( name, document, m, r, c )
{
    for ( uint i = 0; i < matrix->getRows(); i++ ) {
        column->append( new MatrixSequenceElement( matrix ) );
    }
}


MatrixElement::MatrixElement(uint rows, uint columns, BasicElement* parent)
    : BasicElement(parent),
      m_rowNumber( 0 ),
      m_align( NoAlign ),
      m_widthType( NoSize ),
      m_frame( NoLine ),
      m_frameHSpacing( NoSize ),
      m_frameVSpacing( NoSize ),
      m_side( NoSide ),
      m_minLabelSpacingType( NoSize ),
      m_customEqualRows( false ),
      m_customEqualColumns( false ),
      m_customDisplayStyle( false )
{
    for (uint r = 0; r < rows; r++) {
        QPtrList< MatrixSequenceElement >* list = new QPtrList< MatrixSequenceElement >;
        list->setAutoDelete(true);
        for (uint c = 0; c < columns; c++) {
            list->append(new MatrixSequenceElement(this));
        }
        content.append(list);
    }
    content.setAutoDelete(true);
}

MatrixElement::~MatrixElement()
{
}


MatrixElement::MatrixElement( const MatrixElement& other )
    : BasicElement( other )
{
    uint rows = other.getRows();
    uint columns = other.getColumns();

    QPtrListIterator< QPtrList< MatrixSequenceElement > > rowIter( other.content );
    for (uint r = 0; r < rows; r++) {
        ++rowIter;
        QPtrListIterator< MatrixSequenceElement > colIter( *rowIter.current() );

        QPtrList< MatrixSequenceElement >* list = new QPtrList< MatrixSequenceElement >;
        list->setAutoDelete(true);
        for (uint c = 0; c < columns; c++) {
            ++colIter;
            MatrixSequenceElement *mse =
                //new MatrixSequenceElement( *( other.getElement( r, c ) ) );
                new MatrixSequenceElement( *colIter.current() );
            list->append( mse );
            mse->setParent( this );
        }
        content.append(list);
    }
    content.setAutoDelete(true);
}


bool MatrixElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void MatrixElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Matrix element" ) );
}


BasicElement* MatrixElement::goToPos( FormulaCursor* cursor, bool& handled,
                                      const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        uint rows = getRows();
        uint columns = getColumns();

        for (uint r = 0; r < rows; r++) {
            for (uint c = 0; c < columns; c++) {
                BasicElement* element = getElement(r, c);
                e = element->goToPos(cursor, handled, point, myPos);
                if (e != 0) {
                    return e;
                }
            }
        }

        // We are in one of those gaps.
        luPixel dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();

        uint row = rows;
        for (uint r = 0; r < rows; r++) {
            BasicElement* element = getElement(r, 0);
            if (element->getY() > dy) {
                row = r;
                break;
            }
        }
        if (row == 0) {
            BasicElement* element = getParent();
            element->moveLeft(cursor, this);
            handled = true;
            return element;
        }
        row--;

        uint column = columns;
        for (uint c = 0; c < columns; c++) {
            BasicElement* element = getElement(row, c);
            if (element->getX() > dx) {
                column = c;
                break;
            }
        }
        if (column == 0) {
            BasicElement* element = getParent();
            element->moveLeft(cursor, this);
            handled = true;
            return element;
        }
        column--;

        // Rescan the rows with the actual colums required.
        row = rows;
        for (uint r = 0; r < rows; r++) {
            BasicElement* element = getElement(r, column);
            if (element->getY() > dy) {
                row = r;
                break;
            }
        }
        if (row == 0) {
            BasicElement* element = getParent();
            element->moveLeft(cursor, this);
            handled = true;
            return element;
        }
        row--;

        BasicElement* element = getElement(row, column);
        element->moveLeft(cursor, this);
        handled = true;
        return element;
    }
    return 0;
}


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
void MatrixElement::calcSizes( const ContextStyle& context,
                               ContextStyle::TextStyle tstyle,
                               ContextStyle::IndexStyle istyle,
                               StyleAttributes& style )
{
    QMemArray<luPixel> toMidlines(getRows());
    QMemArray<luPixel> fromMidlines(getRows());
    QMemArray<luPixel> widths(getColumns());

    toMidlines.fill(0);
    fromMidlines.fill(0);
    widths.fill(0);

    uint rows = getRows();
    uint columns = getColumns();

    ContextStyle::TextStyle i_tstyle = context.convertTextStyleFraction(tstyle);
    ContextStyle::IndexStyle i_istyle = context.convertIndexStyleUpper(istyle);
    double factor = style.sizeFactor();

    for (uint r = 0; r < rows; r++) {
        QPtrList< MatrixSequenceElement >* list = content.at(r);
        for (uint c = 0; c < columns; c++) {
            SequenceElement* element = list->at(c);
            element->calcSizes( context, i_tstyle, i_istyle, style );
            toMidlines[r] = QMAX(toMidlines[r], element->axis( context, i_tstyle, factor ));
            fromMidlines[r] = QMAX(fromMidlines[r],
                                   element->getHeight()-element->axis( context, i_tstyle, factor ));
            widths[c] = QMAX(widths[c], element->getWidth());
        }
    }

    luPixel distX = context.ptToPixelX( context.getThinSpace( tstyle, factor ) );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );

    luPixel yPos = 0;
    for (uint r = 0; r < rows; r++) {
        QPtrList< MatrixSequenceElement >* list = content.at(r);
        luPixel xPos = 0;
        yPos += toMidlines[r];
        for (uint c = 0; c < columns; c++) {
            SequenceElement* element = list->at(c);
            switch (context.getMatrixAlignment()) {
            case ContextStyle::left:
                element->setX(xPos);
                break;
            case ContextStyle::center:
                element->setX(xPos + (widths[c] - element->getWidth())/2);
                break;
            case ContextStyle::right:
                element->setX(xPos + widths[c] - element->getWidth());
                break;
            }
            element->setY(yPos - element->axis( context, i_tstyle, factor ));
            xPos += widths[c] + distX;
        }
        yPos += fromMidlines[r] + distY;
    }

    luPixel width = distX * (columns - 1);
    luPixel height = distY * (rows - 1);

    for (uint r = 0; r < rows; r++) height += toMidlines[r] + fromMidlines[r];
    for (uint c = 0; c < columns; c++) width += widths[c];

    setWidth(width);
    setHeight(height);
    if ((rows == 2) && (columns == 1)) {
        setBaseline( getMainChild()->getHeight() + distY / 2 + context.axisHeight( tstyle, factor ) );
    }
    else {
        setBaseline( height/2 + context.axisHeight( tstyle, factor ) );
    }
}

/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void MatrixElement::draw( QPainter& painter, const LuPixelRect& rect,
                          const ContextStyle& context,
                          ContextStyle::TextStyle tstyle,
                          ContextStyle::IndexStyle istyle,
                          StyleAttributes& style,
                          const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( rect ) )
    //    return;

    uint rows = getRows();
    uint columns = getColumns();

    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < columns; c++) {
            getElement(r, c)->draw(painter, rect, context,
                                   context.convertTextStyleFraction(tstyle),
                                   context.convertIndexStyleUpper(istyle),
                                   style,
                                   myPos);
        }
    }

    // Debug
    //painter.setPen(Qt::red);
    //painter.drawRect(myPos.x(), myPos.y(), getWidth(), getHeight());
}


void MatrixElement::dispatchFontCommand( FontCommand* cmd )
{
    uint rows = getRows();
    uint columns = getColumns();

    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < columns; c++) {
            getElement(r, c)->dispatchFontCommand( cmd );
        }
    }
}


// navigation
//
// The elements are responsible to handle cursor movement themselves.
// To do this they need to know the direction the cursor moves and
// the element it comes from.
//
// The cursor might be in normal or in selection mode.

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void MatrixElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        if (from == getParent()) {
            getElement(getRows()-1, getColumns()-1)->moveLeft(cursor, this);
        }
        else {
            bool linear = cursor->getLinearMovement();
            uint row = 0;
            uint column = 0;
            if (searchElement(from, row, column)) {
                if (column > 0) {
                    getElement(row, column-1)->moveLeft(cursor, this);
                }
                else if (linear && (row > 0)) {
                    getElement(row-1, getColumns()-1)->moveLeft(cursor, this);
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                getParent()->moveLeft(cursor, this);
            }
        }
    }
}

/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void MatrixElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            getElement(0, 0)->moveRight(cursor, this);
        }
        else {
            bool linear = cursor->getLinearMovement();
            uint row = 0;
            uint column = 0;
            if (searchElement(from, row, column)) {
                if (column < getColumns()-1) {
                    getElement(row, column+1)->moveRight(cursor, this);
                }
                else if (linear && (row < getRows()-1)) {
                    getElement(row+1, 0)->moveRight(cursor, this);
                }
                else {
                    getParent()->moveRight(cursor, this);
                }
            }
            else {
                getParent()->moveRight(cursor, this);
            }
        }
    }
}

/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void MatrixElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            getElement(0, 0)->moveRight(cursor, this);
        }
        else {
            uint row = 0;
            uint column = 0;
            if (searchElement(from, row, column)) {
                if (row > 0) {
                    getElement(row-1, column)->moveRight(cursor, this);
                }
                else {
                    getParent()->moveUp(cursor, this);
                }
            }
            else {
                getParent()->moveUp(cursor, this);
            }
        }
    }
}

/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void MatrixElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent()) {
            getElement(0, 0)->moveRight(cursor, this);
        }
        else {
            uint row = 0;
            uint column = 0;
            if (searchElement(from, row, column)) {
                if (row < getRows()-1) {
                    getElement(row+1, column)->moveRight(cursor, this);
                }
                else {
                    getParent()->moveDown(cursor, this);
                }
            }
            else {
                getParent()->moveDown(cursor, this);
            }
        }
    }
}

/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void MatrixElement::goInside(FormulaCursor* cursor)
{
    getElement(0, 0)->goInside(cursor);
}


// If there is a main child we must provide the insert/remove semantics.
SequenceElement* MatrixElement::getMainChild()
{
    return content.at(0)->at(0);
}

void MatrixElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    uint rows = getRows();
    uint columns = getColumns();
    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < columns; c++) {
            if (child == getElement(r, c)) {
                cursor->setTo(this, r*columns+c);
            }
        }
    }
}

const MatrixSequenceElement* MatrixElement::getElement( uint row, uint column ) const
{
    QPtrListIterator< QPtrList < MatrixSequenceElement > > rows( content );
    rows += row;
    if ( ! rows.current() )
        return 0;

    QPtrListIterator< MatrixSequenceElement > cols ( *rows.current() );
    cols += column;
    return cols.current();
}


bool MatrixElement::searchElement(BasicElement* element, uint& row, uint& column)
{
    uint rows = getRows();
    uint columns = getColumns();
    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < columns; c++) {
            if (element == getElement(r, c)) {
                row = r;
                column = c;
                return true;
            }
        }
    }
    return false;
}


/**
 * Appends our attributes to the dom element.
 */
void MatrixElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    uint rows = getRows();
    uint cols = getColumns();

    element.setAttribute("ROWS", rows);
    element.setAttribute("COLUMNS", cols);

    QDomDocument doc = element.ownerDocument();

    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < cols; c++) {
    	    QDomElement tmp = getElement(r,c)->getElementDom(doc);
            element.appendChild(tmp);
	}
        element.appendChild(doc.createComment("end of row"));
    }
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool MatrixElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    uint rows = 0;
    QString rowStr = element.attribute("ROWS");
    if(!rowStr.isNull()) {
        rows = rowStr.toInt();
    }
    if (rows == 0) {
        kdWarning( DEBUGID ) << "Rows <= 0 in MatrixElement." << endl;
        return false;
    }

    QString columnStr = element.attribute("COLUMNS");
    uint cols = 0;
    if(!columnStr.isNull()) {
        cols = columnStr.toInt();
    }
    if (cols == 0) {
        kdWarning( DEBUGID ) << "Columns <= 0 in MatrixElement." << endl;
        return false;
    }

    content.clear();
    for (uint r = 0; r < rows; r++) {
        QPtrList< MatrixSequenceElement >* list = new QPtrList< MatrixSequenceElement >;
        list->setAutoDelete(true);
        content.append(list);
        for (uint c = 0; c < cols; c++) {
            MatrixSequenceElement* element = new MatrixSequenceElement(this);
            list->append(element);
	}
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool MatrixElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    uint rows = getRows();
    uint cols = getColumns();

    uint r = 0;
    uint c = 0;
    while ( !node.isNull() && r < rows ) {
        if ( node.isElement() ) {
            SequenceElement* element = getElement( r, c );
            QDomElement e = node.toElement();
            if ( !element->buildFromDom( e ) ) {
                return false;
            }
            c++;
            if ( c == cols ) {
                c = 0;
                r++;
            }
        }
        node = node.nextSibling();
    }
    return true;
}

bool MatrixElement::readAttributesFromMathMLDom( const QDomElement& element )
{
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    QString alignStr = element.attribute( "align" ).lower();
    if ( ! alignStr.isNull() ) {
        if ( alignStr.find( "top" ) != -1 ) {
            m_align = TopAlign;
        }
        else if ( alignStr.find( "bottom" ) != -1 ) {
            m_align = BottomAlign;
        }
        else if ( alignStr.find( "center" ) != -1 ) {
            m_align = CenterAlign;
        }
        else if ( alignStr.find( "baseline" ) != -1 ) {
            m_align = BaselineAlign;
        }
        else if ( alignStr.find( "axis" ) != -1 ) {
            m_align = AxisAlign;
        }
        int index = alignStr.findRev( ' ' );
        if ( index != -1 ) {
            m_rowNumber = alignStr.right( index + 1 ).toInt();
        }
    }
    QString rowalignStr = element.attribute( "rowalign" ).lower();
    if ( ! rowalignStr.isNull() ) {
        QStringList list = QStringList::split( ' ', rowalignStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            if ( *it == "top" ) {
                m_rowAlign.append( TopAlign );
            }
            else if ( *it == "bottom" ) {
                m_rowAlign.append( BottomAlign );
            }
            else if ( *it == "center" ) {
                m_rowAlign.append( CenterAlign );
            }
            else if ( *it == "baseline" ) {
                m_rowAlign.append( BaselineAlign );
            }
            else if ( *it == "axis" ) {
                m_rowAlign.append( AxisAlign );
            }
        }
    }
    QString columnalignStr = element.attribute( "columnalign" ).lower();
    if ( ! columnalignStr.isNull() ) {
        QStringList list = QStringList::split( ' ', columnalignStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            if ( *it == "left" ) {
                m_columnAlign.append( LeftHorizontalAlign );
            }
            else if ( *it == "center" ) {
                m_columnAlign.append( CenterHorizontalAlign );
            }
            else if ( *it == "right" ) {
                m_columnAlign.append( RightHorizontalAlign );
            }
        }
    }
    QString alignmentscopeStr = element.attribute( "alignmentscope" ).lower();
    if ( ! alignmentscopeStr.isNull() ) {
        QStringList list = QStringList::split( ' ', alignmentscopeStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            if ( *it == "true" ) {
                m_alignmentScope.append( true );
            }
            else if ( *it == "false" ) {
                m_alignmentScope.append( false );
            }
        }
    }
    QString columnwidthStr = element.attribute( "columnwidth" ).lower();
    if ( columnwidthStr.isNull() ) {
        QStringList list = QStringList::split( ' ', columnwidthStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            SizeType type = NoSize;
            double length;
            if ( *it == "auto" ) {
                type = AutoSize;
            }
            else if ( *it == "fit" ) {
                type = FitSize;
            }
            else {
                length = getSize( columnwidthStr, &type );
                if ( type == NoSize ) {
                    type = getSpace( columnwidthStr );
                }
            }
            if ( type != NoSize ) {
                m_columnWidthType.append( type );
                if ( type == RelativeSize || type == AbsoluteSize || type == PixelSize ) {
                    m_columnWidth.append( length );
                }
            }
        }
    }
    QString widthStr = element.attribute( "width" ).lower();
    if ( ! widthStr.isNull() ) {
        if ( widthStr == "auto" ) {
            m_widthType = AutoSize;
        }
        else {
            m_width = getSize( widthStr, &m_widthType );
        }
    }
    QString rowspacingStr = element.attribute( "rowspacing" ).lower();
    if ( ! rowspacingStr.isNull() ) {
        QStringList list = QStringList::split( ' ', rowspacingStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            SizeType type;
            double length = getSize( *it, &type );
            if ( type != NoSize ) {
                m_rowSpacingType.append( type );
                m_rowSpacing.append( length );
            }
        }
    }
    QString columnspacingStr = element.attribute( "columnspacing" ).lower();
    if ( ! columnspacingStr.isNull() ) {
        QStringList list = QStringList::split( ' ', columnspacingStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            SizeType type;
            double length = getSize( *it, &type );
            if ( type == NoSize ) {
                type = getSpace( columnspacingStr );
            }
            if ( type != NoSize ) {
                m_columnSpacingType.append( type );
                if ( type == RelativeSize || type == AbsoluteSize || type == PixelSize ) {
                    m_columnSpacing.append( length );
                }
            }
        }
    }
    QString rowlinesStr = element.attribute( "rowlines" ).lower();
    if ( ! rowlinesStr.isNull() ) {
        QStringList list = QStringList::split( ' ', rowlinesStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            if ( *it == "none" ) {
                m_rowLines.append( NoneLine );
            }
            else if ( *it == "solid" ) {
                m_rowLines.append( SolidLine );
            }
            else if ( *it == "dashed" ) {
                m_rowLines.append( DashedLine );
            }
        }
    }
    QString columnlinesStr = element.attribute( "columnlines" ).lower();
    if ( ! columnlinesStr.isNull() ) {
        QStringList list = QStringList::split( ' ', columnlinesStr );
        for ( QStringList::iterator it = list.begin(); it != list.end(); it++ ) {
            if ( *it == "none" ) {
                m_columnLines.append( NoneLine );
            }
            else if ( *it == "solid" ) {
                m_columnLines.append( SolidLine );
            }
            else if ( *it == "dashed" ) {
                m_columnLines.append( DashedLine );
            }
        }
    }
    QString frameStr = element.attribute( "frame" ).stripWhiteSpace().lower();
    if ( ! frameStr.isNull() ) {
        if ( frameStr == "none" ) {
            m_frame = NoneLine;
        }
        else if ( frameStr == "solid" ) {
            m_frame = SolidLine;
        }
        else if ( frameStr == "dashed" ) {
            m_frame = DashedLine;
        }
    }
    QString framespacingStr = element.attribute( "framespacing" );
    if ( ! framespacingStr.isNull() ) {
        QStringList list = QStringList::split( ' ', framespacingStr );
        m_frameHSpacing = getSize( list[0], &m_frameHSpacingType );
        if ( m_frameHSpacingType == NoSize ) {
            m_frameHSpacingType = getSpace( list[0] );
        }
        if ( list.count() > 1 ) {
            m_frameVSpacing = getSize( list[1], &m_frameVSpacingType );
            if ( m_frameVSpacingType == NoSize ) {
                m_frameVSpacingType = getSpace( list[1] );
            }
        }
    }
    QString equalrowsStr = element.attribute( "equalrows" ).stripWhiteSpace().lower();
    if ( ! equalrowsStr.isNull() ) {
        m_customEqualRows = true;
        if ( equalrowsStr == "false" ) {
            m_equalRows = false;
        }
        else {
            m_equalRows = true;
        }
    }
    QString equalcolumnsStr = element.attribute( "equalcolumns" ).stripWhiteSpace().lower();
    if ( ! equalcolumnsStr.isNull() ) {
        m_customEqualColumns = true;
        if ( equalcolumnsStr == "false" ) {
            m_equalColumns = false;
        }
        else {
            m_equalColumns = true;
        }
    }
    QString displaystyleStr = element.attribute( "displaystyle" ).stripWhiteSpace().lower();
    if ( ! displaystyleStr.isNull() ) {
        m_customDisplayStyle = true;
        if ( displaystyleStr == "false" ) {
            m_displayStyle = false;
        }
        else {
            m_displayStyle = true;
        }
    }
    QString sideStr = element.attribute( "side" ).stripWhiteSpace().lower();
    if ( ! sideStr.isNull() ) {
        if ( sideStr == "left" ) {
            m_side = LeftSide;
        }
        else if ( sideStr == "right" ) {
            m_side = RightSide;
        }
        else if ( sideStr == "leftoverlap" ) {
            m_side = LeftOverlapSide;
        }
        else if ( sideStr == "rightoverlap" ) {
            m_side = RightOverlapSide;
        }
    }
    QString minlabelspacingStr = element.attribute( "minlabelspacing" ).stripWhiteSpace().lower();
    if ( ! minlabelspacingStr.isNull() ) {
        m_minLabelSpacing = getSize( minlabelspacingStr, &m_minLabelSpacingType );
        if ( m_minLabelSpacingType == NoSize ) {
            m_minLabelSpacingType = getSpace( minlabelspacingStr );
        }
    }
    return true;
}

/**
 * Reads our content from the MathML node. Sets the node to the next node
 * that needs to be read. It is sometimes needed to read more than one node
 * (e. g. for fence operators).
 * Returns the number of nodes processed or -1 if it failed.
 */
int MatrixElement::readContentFromMathMLDom( QDomNode& node )
{
    // We have twice, since there may be empty elements and we need to know how
    // many of them we have. So, first pass, get number of rows and columns

    if ( BasicElement::readContentFromMathMLDom( node ) == -1 ) {
        return -1;
    }

    uint rows = 0;
    uint cols = 0;
    QDomNode n = node;
    while ( !n.isNull() ) {
        if ( n.isElement() ) {
            QDomElement e = n.toElement();
            if ( e.tagName().lower() == "mtr" || e.tagName().lower() == "mlabeledtr" )
            {
                rows++;

                /* Determins the number of columns */
                QDomNode cellnode = e.firstChild();
                int cc = 0;

                while ( !cellnode.isNull() ) {
                    if ( cellnode.isElement() )
                        cc++;
                    cellnode = cellnode.nextSibling();
                }
                if ( cc > 0 && e.tagName().lower() == "mlabeledtr" )
                    cc--;
                if ( cc > cols )
                    cols = cc;
            }
        }
        n = n.nextSibling();
    }

    // Create elements
    content.clear();
    for (uint r = 0; r < rows; r++) {
        QPtrList< MatrixSequenceElement >* list = new QPtrList< MatrixSequenceElement >;
        list->setAutoDelete(true);
        content.append(list);
        for (uint c = 0; c < cols; c++) {
            MatrixSequenceElement* element = new MatrixSequenceElement(this);
            list->append(element);
        }
    }

    // Second pass, read elements now
    uint r = 0;
    uint c = 0;
    while ( !node.isNull() ) {
        if ( node.isElement() ) {
            QDomElement e = node.toElement();
            if ( e.tagName().lower() == "mtr" || e.tagName().lower() == "mlabeledtr" ) {
                QDomNode cellnode = e.firstChild();
                if ( e.tagName().lower() == "mlabeledtr" ) {
                    while ( ! cellnode.isNull() && ! cellnode.isElement() )
                        cellnode = cellnode.nextSibling();
                    if ( ! cellnode.isNull() )
                        cellnode = cellnode.nextSibling();
                }
                while ( !cellnode.isNull() ) {
                    if ( cellnode.isElement() ) {
                        QDomElement cellelement = cellnode.toElement();
                        if ( cellelement.tagName().lower() != "mtd" ) {
                            // TODO: Inferred mtd. Deprecated in MathML 2.0
                            kdWarning( DEBUGID ) << "Unsupported tag " 
                                                 << cellelement.tagName()
                                                 << " inside matrix row\n";
                        }
                        else {
                            SequenceElement* element = getElement(r, c);
                            if ( element->buildFromMathMLDom( cellelement ) == -1 )
                                return -1;
                            c++;
                        }
                    }
                    cellnode = cellnode.nextSibling();
                }
                c = 0;
                r++;
            }
        }
        node = node.nextSibling();
    }
    return 1;
}

QString MatrixElement::toLatex()
{
    //All the border handling must be implemented here too

    QString matrix;
    uint cols=getColumns();
    uint rows=getRows();

    matrix="\\begin{array}{ ";
    for(uint i=0;i<cols;i++)
	matrix+="c ";

    matrix+="} ";

    for (uint r = 0; r < rows; r++) {
        for (uint c = 0; c < cols; c++) {
            matrix+=getElement(r, c)->toLatex();
	    if( c < cols-1)    matrix+=" & ";
        }
    	if(r < rows-1 ) matrix+=" \\\\ ";
    }

    matrix+=" \\end{array}";

    return matrix;
}

QString MatrixElement::formulaString()
{
    QString matrix = "[";
    uint cols=getColumns();
    uint rows=getRows();
    for (uint r = 0; r < rows; r++) {
        matrix += "[";
        for (uint c = 0; c < cols; c++) {
            matrix+=getElement(r, c)->formulaString();
	    if ( c < cols-1 ) matrix+=", ";
        }
        matrix += "]";
    	if ( r < rows-1 ) matrix += ", ";
    }
    matrix += "]";
    return matrix;
}


SequenceElement* MatrixElement::elementAt(uint row, uint column)
{
    return getElement( row, column );
}

void MatrixElement::writeMathMLAttributes( QDomElement& element ) const
{
    QString rownumber;
    if ( m_rowNumber ) {
        rownumber = QString( " %1" ).arg( m_rowNumber );
    }
    switch ( m_align ) {
    case TopAlign:
        element.setAttribute( "align", "top" + rownumber );
        break;
    case BottomAlign:
        element.setAttribute( "align", "bottom" + rownumber );
        break;
    case CenterAlign:
        element.setAttribute( "align", "center" + rownumber );
        break;
    case BaselineAlign:
        element.setAttribute( "align", "baseline" + rownumber );
        break;
    case AxisAlign:
        element.setAttribute( "align", "axis" + rownumber );
        break;
    default:
        break;
    }
    QString rowalign;
    for ( QValueList< VerticalAlign >::const_iterator it = m_rowAlign.begin(); it != m_rowAlign.end(); it++ )
    {
        switch ( *it ) {
        case TopAlign:
            rowalign.append( "top " );
            break;
        case BottomAlign:
            rowalign.append( "bottom " );
            break;
        case CenterAlign:
            rowalign.append( "center " );
            break;
        case BaselineAlign:
            rowalign.append( "baseline " );
            break;
        case AxisAlign:
            rowalign.append( "axis " );
            break;
        default:
            break;
        }
    }
    if ( ! rowalign.isNull() ) {
        element.setAttribute( "rowalign", rowalign.stripWhiteSpace() );
    }
    QString columnalign;
    for ( QValueList< HorizontalAlign >::const_iterator it = m_columnAlign.begin(); it != m_columnAlign.end(); it++ )
    {
        switch ( *it ) {
        case LeftHorizontalAlign:
            rowalign.append( "left " );
            break;
        case CenterHorizontalAlign:
            rowalign.append( "center " );
            break;
        case RightHorizontalAlign:
            rowalign.append( "right " );
            break;
        default:
            break;
        }
    }
    if ( ! columnalign.isNull() ) {
        element.setAttribute( "columnalign", columnalign.stripWhiteSpace() );
    }
    QString alignmentscope;
    for ( QValueList< bool >::const_iterator it = m_alignmentScope.begin(); it != m_alignmentScope.end(); it++ )
    {
        if ( *it ) {
            alignmentscope.append( "true " );
        }
        else {
            alignmentscope.append( "false " );
        }
    }
    if ( ! alignmentscope.isNull() ) {
        element.setAttribute( "alignmentscope", alignmentscope.stripWhiteSpace() );
    }
    QString columnwidth;
    QValueList< double >::const_iterator lengthIt = m_columnWidth.begin();
    for ( QValueList< SizeType >::const_iterator typeIt = m_columnWidthType.begin();
          typeIt != m_columnWidthType.end(); typeIt ++ ) {
        switch ( *typeIt ) {
        case AutoSize:
            columnwidth.append( "auto " );
            break;
        case FitSize:
            columnwidth.append( "fit " );
            break;
        case AbsoluteSize:
            columnwidth.append( QString( "%1pt " ).arg( *lengthIt ) );
            lengthIt++;
            break;
        case RelativeSize:
            columnwidth.append( QString( "%1% " ).arg( *lengthIt * 100.0 ) );
            lengthIt++;
            break;
        case PixelSize:
            columnwidth.append( QString( "%1px " ).arg( *lengthIt ) );
            lengthIt++;
            break;
        case NegativeVeryVeryThinMathSpace:
            columnwidth.append( "negativeveryverythinmathspace " );
            break;
        case NegativeVeryThinMathSpace:
            columnwidth.append( "negativeverythinmathspace " );
            break;
        case NegativeThinMathSpace:
            columnwidth.append( "negativethinmathspace " );
            break;
        case NegativeMediumMathSpace:
            columnwidth.append( "negativemediummathspace " );
            break;
        case NegativeThickMathSpace:
            columnwidth.append( "negativethickmathspace " );
            break;
        case NegativeVeryThickMathSpace:
            columnwidth.append( "negativeverythickmathspace " );
            break;
        case NegativeVeryVeryThickMathSpace:
            columnwidth.append( "negativeveryverythickmathspace " );
            break;
        case VeryVeryThinMathSpace:
            columnwidth.append( "veryverythinmathspace " );
            break;
        case VeryThinMathSpace:
            columnwidth.append( "verythinmathspace " );
            break;
        case ThinMathSpace:
            columnwidth.append( "thinmathspace " );
            break;
        case MediumMathSpace:
            columnwidth.append( "mediummathspace " );
            break;
        case ThickMathSpace:
            columnwidth.append( "thickmathspace " );
            break;
        case VeryThickMathSpace:
            columnwidth.append( "verythickmathspace " );
            break;
        case VeryVeryThickMathSpace:
            columnwidth.append( "veryverythickmathspace " );
            break;
        default:
            break;
        }
    }
    if ( ! columnwidth.isNull() ) {
        element.setAttribute( "columnwidth", columnwidth.stripWhiteSpace() );
    }
    switch ( m_widthType ) {
    case AutoSize:
        element.setAttribute( "width", "auto" );
        break;
    case AbsoluteSize:
        element.setAttribute( "width", QString( "%1pt" ).arg( m_width ) );
        break;
    case RelativeSize:
        element.setAttribute( "width", QString( "%1% " ).arg( m_width * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( "width", QString( "%1px " ).arg( m_width ) );
        break;
    default:
        break;
    }
    QString rowspacing;
    lengthIt = m_rowSpacing.begin();
    for ( QValueList< SizeType >::const_iterator typeIt = m_rowSpacingType.begin();
          typeIt != m_rowSpacingType.end(); typeIt++, lengthIt++ ) {
        switch ( *typeIt ) {
        case AbsoluteSize:
            rowspacing.append( QString( "%1pt " ).arg( *lengthIt ) );
            break;
        case RelativeSize:
            rowspacing.append( QString( "%1% " ).arg( *lengthIt * 100.0 ) );
            break;
        case PixelSize:
            rowspacing.append( QString( "%1px " ).arg( *lengthIt ) );
            break;
        default:
            break;
        }
    }
    if ( ! rowspacing.isNull() ) {
        element.setAttribute( "rowspacing", rowspacing.stripWhiteSpace() );
    }
    QString columnspacing;
    lengthIt = m_columnSpacing.begin(); 
    for ( QValueList< SizeType >::const_iterator typeIt = m_columnSpacingType.begin();
          typeIt != m_columnSpacingType.end(); typeIt++ ) {
        switch ( *typeIt ) {
        case AbsoluteSize:
            columnspacing.append( QString( "%1pt " ).arg( *lengthIt ) );
            lengthIt++;
            break;
        case RelativeSize:
            columnspacing.append( QString( "%1% " ).arg( *lengthIt * 100.0 ) );
            lengthIt++;
            break;
        case PixelSize:
            columnspacing.append( QString( "%1px " ).arg( *lengthIt ) );
            lengthIt++;
            break;
        case NegativeVeryVeryThinMathSpace:
            columnspacing.append( "negativeveryverythinmathspace " );
            break;
        case NegativeVeryThinMathSpace:
            columnspacing.append( "negativeverythinmathspace " );
            break;
        case NegativeThinMathSpace:
            columnspacing.append( "negativethinmathspace " );
            break;
        case NegativeMediumMathSpace:
            columnspacing.append( "negativemediummathspace " );
            break;
        case NegativeThickMathSpace:
            columnspacing.append( "negativethickmathspace " );
            break;
        case NegativeVeryThickMathSpace:
            columnspacing.append( "negativeverythickmathspace " );
            break;
        case NegativeVeryVeryThickMathSpace:
            columnspacing.append( "negativeveryverythickmathspace " );
            break;
        case VeryVeryThinMathSpace:
            columnspacing.append( "veryverythinmathspace " );
            break;
        case VeryThinMathSpace:
            columnspacing.append( "verythinmathspace " );
            break;
        case ThinMathSpace:
            columnspacing.append( "thinmathspace " );
            break;
        case MediumMathSpace:
            columnspacing.append( "mediummathspace " );
            break;
        case ThickMathSpace:
            columnspacing.append( "thickmathspace " );
            break;
        case VeryThickMathSpace:
            columnspacing.append( "verythickmathspace " );
            break;
        case VeryVeryThickMathSpace:
            columnspacing.append( "veryverythickmathspace " );
            break;
        default:
            break;
        }
    }
    if ( ! rowspacing.isNull() ) {
        element.setAttribute( "rowspacing", rowspacing.stripWhiteSpace() );
    }
    QString rowlines;
    for ( QValueList< LineType >::const_iterator it = m_rowLines.begin(); it != m_rowLines.end(); it++ )
    {
        switch ( *it ) {
        case NoneLine:
            rowlines.append( "none " );
            break;
        case SolidLine:
            rowlines.append( "solid " );
            break;
        case DashedLine:
            rowlines.append( "dashed " );
            break;
        default:
            break;
        }
    }
    if ( ! rowlines.isNull() ) {
        element.setAttribute( "rowlines", rowlines.stripWhiteSpace() );
    }
    QString columnlines;
    for ( QValueList< LineType >::const_iterator it = m_columnLines.begin(); it != m_columnLines.end(); it++ )
    {
        switch ( *it ) {
        case NoneLine:
            columnlines.append( "none " );
            break;
        case SolidLine:
            columnlines.append( "solid " );
            break;
        case DashedLine:
            columnlines.append( "dashed " );
            break;
        default:
            break;
        }
    }
    if ( ! columnlines.isNull() ) {
        element.setAttribute( "columnlines", columnlines.stripWhiteSpace() );
    }
    switch ( m_frame ) {
    case NoneLine:
        element.setAttribute( "frame", "none" );
        break;
    case SolidLine:
        element.setAttribute( "frame", "solid" );
        break;
    case DashedLine:
        element.setAttribute( "frame", "dashed" );
        break;
    default:
        break;
    }
    QString framespacing;
    switch ( m_frameHSpacingType ) {
    case AbsoluteSize:
        framespacing.append( QString( "%1pt " ).arg( m_frameHSpacing ) );
        break;
    case RelativeSize:
        framespacing.append( QString( "%1% " ).arg( m_frameHSpacing * 100.0 ) );
        break;
    case PixelSize:
        framespacing.append( QString( "%1px " ).arg( m_frameHSpacing ) );
        break;
    case NegativeVeryVeryThinMathSpace:
        framespacing.append( "negativeveryverythinmathspace " );
        break;
    case NegativeVeryThinMathSpace:
        framespacing.append( "negativeverythinmathspace " );
        break;
    case NegativeThinMathSpace:
        framespacing.append( "negativethinmathspace " );
        break;
    case NegativeMediumMathSpace:
        framespacing.append( "negativemediummathspace " );
        break;
    case NegativeThickMathSpace:
        framespacing.append( "negativethickmathspace " );
        break;
    case NegativeVeryThickMathSpace:
        framespacing.append( "negativeverythickmathspace " );
        break;
    case NegativeVeryVeryThickMathSpace:
        framespacing.append( "negativeveryverythickmathspace " );
        break;
    case VeryVeryThinMathSpace:
        framespacing.append( "veryverythinmathspace " );
        break;
    case VeryThinMathSpace:
        framespacing.append( "verythinmathspace " );
        break;
    case ThinMathSpace:
        framespacing.append( "thinmathspace " );
        break;
    case MediumMathSpace:
        framespacing.append( "mediummathspace " );
        break;
    case ThickMathSpace:
        framespacing.append( "thickmathspace " );
        break;
    case VeryThickMathSpace:
        framespacing.append( "verythickmathspace " );
        break;
    case VeryVeryThickMathSpace:
        framespacing.append( "veryverythickmathspace " );
        break;
    default:
        break;
    }
    switch ( m_frameVSpacingType ) {
    case AbsoluteSize:
        framespacing.append( QString( "%1pt " ).arg( m_frameVSpacing ) );
        break;
    case RelativeSize:
        framespacing.append( QString( "%1% " ).arg( m_frameVSpacing * 100.0 ) );
        break;
    case PixelSize:
        framespacing.append( QString( "%1px " ).arg( m_frameVSpacing ) );
        break;
    case NegativeVeryVeryThinMathSpace:
        framespacing.append( "negativeveryverythinmathspace " );
        break;
    case NegativeVeryThinMathSpace:
        framespacing.append( "negativeverythinmathspace " );
        break;
    case NegativeThinMathSpace:
        framespacing.append( "negativethinmathspace " );
        break;
    case NegativeMediumMathSpace:
        framespacing.append( "negativemediummathspace " );
        break;
    case NegativeThickMathSpace:
        framespacing.append( "negativethickmathspace " );
        break;
    case NegativeVeryThickMathSpace:
        framespacing.append( "negativeverythickmathspace " );
        break;
    case NegativeVeryVeryThickMathSpace:
        framespacing.append( "negativeveryverythickmathspace " );
        break;
    case VeryVeryThinMathSpace:
        framespacing.append( "veryverythinmathspace " );
        break;
    case VeryThinMathSpace:
        framespacing.append( "verythinmathspace " );
        break;
    case ThinMathSpace:
        framespacing.append( "thinmathspace " );
        break;
    case MediumMathSpace:
        framespacing.append( "mediummathspace " );
        break;
    case ThickMathSpace:
        framespacing.append( "thickmathspace " );
        break;
    case VeryThickMathSpace:
        framespacing.append( "verythickmathspace " );
        break;
    case VeryVeryThickMathSpace:
        framespacing.append( "veryverythickmathspace " );
        break;
    default:
        break;
    }
    if ( ! framespacing.isNull() ) {
        element.setAttribute( "framespacing", framespacing.stripWhiteSpace() );
    }
    if ( m_customEqualRows ) {
        element.setAttribute( "equalrows", m_equalRows ? "true" : "false" );
    }
    if ( m_customEqualColumns ) {
        element.setAttribute( "equalcolumns", m_equalColumns ? "true" : "false" );
    }
    if ( m_customDisplayStyle ) {
        element.setAttribute( "displaystyle", m_displayStyle ? "true" : "false" );
    }
    switch ( m_side ) {
    case LeftSide:
        element.setAttribute( "side", "left" );
        break;
    case RightSide:
        element.setAttribute( "side", "right" );
        break;
    case LeftOverlapSide:
        element.setAttribute( "side", "leftoverlap" );
        break;
    case RightOverlapSide:
        element.setAttribute( "side", "rightoverlap" );
        break;
    default:
        break;
    }
    switch ( m_minLabelSpacingType ) {
    case AbsoluteSize:
        element.setAttribute( "minlabelspacing", QString( "%1pt" ).arg( m_minLabelSpacing ) );
        break;
    case RelativeSize:
        element.setAttribute( "minlabelspacing", QString( "%1%" ).arg( m_minLabelSpacing * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( "minlabelspacing", QString( "%1px" ).arg( m_minLabelSpacing ) );
        break;
    case NegativeVeryVeryThinMathSpace:
        element.setAttribute( "minlabelspacing", "negativeveryverythinmathspace" );
        break;
    case NegativeVeryThinMathSpace:
        element.setAttribute( "minlabelspacing", "negativeverythinmathspace" );
        break;
    case NegativeThinMathSpace:
        element.setAttribute( "minlabelspacing", "negativethinmathspace" );
        break;
    case NegativeMediumMathSpace:
        element.setAttribute( "minlabelspacing", "negativemediummathspace" );
        break;
    case NegativeThickMathSpace:
        element.setAttribute( "minlabelspacing", "negativethickmathspace" );
        break;
    case NegativeVeryThickMathSpace:
        element.setAttribute( "minlabelspacing", "negativeverythickmathspace" );
        break;
    case NegativeVeryVeryThickMathSpace:
        element.setAttribute( "minlabelspacing", "negativeveryverythickmathspace" );
        break;
    case VeryVeryThinMathSpace:
        element.setAttribute( "minlabelspacing", "veryverythinmathspace" );
        break;
    case VeryThinMathSpace:
        element.setAttribute( "minlabelspacing", "verythinmathspace" );
        break;
    case ThinMathSpace:
        element.setAttribute( "minlabelspacing", "thinmathspace" );
        break;
    case MediumMathSpace:
        element.setAttribute( "minlabelspacing", "mediummathspace" );
        break;
    case ThickMathSpace:
        element.setAttribute( "minlabelspacing", "thickmathspace" );
        break;
    case VeryThickMathSpace:
        element.setAttribute( "minlabelspacing", "verythickmathspace" );
        break;
    case VeryVeryThickMathSpace:
        element.setAttribute( "minlabelspacing", "veryverythickmathspace" );
        break;
    default:
        break;
    }
}

void MatrixElement::writeMathMLContent( QDomDocument& doc, 
                                        QDomElement& element,
                                        bool oasisFormat ) const
{
    QDomElement row;
    QDomElement cell;

    uint rows = getRows();
    uint cols = getColumns();

    for ( uint r = 0; r < rows; r++ )
    {
        row = doc.createElement( oasisFormat ? "math:mtr" : "mtr" );
        element.appendChild( row );
        for ( uint c = 0; c < cols; c++ )
        {
            cell = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );
            row.appendChild( cell );
    	    getElement(r,c)->writeMathML( doc, cell, oasisFormat );
        }
    }
}


//////////////////////////////////////////////////////////////////////////////


/**
 * The lines behaviour is (a little) different from that
 * of ordinary sequences.
 */
class MultilineSequenceElement : public SequenceElement {
    typedef SequenceElement inherited;
public:

    MultilineSequenceElement( BasicElement* parent = 0 );

    virtual MultilineSequenceElement* clone() {
        return new MultilineSequenceElement( *this );
    }

    virtual BasicElement* goToPos( FormulaCursor*, bool& handled,
                                   const LuPixelPoint& point, const LuPixelPoint& parentOrigin );

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes( const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            StyleAttributes& style );

    virtual void registerTab( BasicElement* tab );

    /**
     * This is called by the container to get a command depending on
     * the current cursor position (this is how the element gets chosen)
     * and the request.
     *
     * @returns the command that performs the requested action with
     * the containers active cursor.
     */
    virtual KCommand* buildCommand( Container*, Request* );

    virtual KCommand* input( Container* container, QKeyEvent* event );

    virtual KCommand* input( Container* container, QChar ch );

    uint tabCount() const { return tabs.count(); }

    BasicElement* tab( uint i ) { return tabs.at( i ); }

    /// Change the width of tab i and move all elements after it.
    void moveTabTo( uint i, luPixel pos );

    /// Return the greatest tab number less than pos.
    int tabBefore( uint pos );

    /// Return the position of tab i.
    int tabPos( uint i );

    virtual void writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat = false ) const ;

private:

    QPtrList<BasicElement> tabs;
};


// Split the line at position pos.
class KFCNewLine : public Command {
public:
    KFCNewLine( const QString& name, Container* document,
                MultilineSequenceElement* line, uint pos );

    virtual ~KFCNewLine();

    virtual void execute();
    virtual void unexecute();

private:
    MultilineSequenceElement* m_line;
    MultilineSequenceElement* m_newline;
    uint m_pos;
};


KFCNewLine::KFCNewLine( const QString& name, Container* document,
                        MultilineSequenceElement* line, uint pos )
    : Command( name, document ),
      m_line( line ), m_pos( pos )
{
    m_newline = new MultilineSequenceElement( m_line->getParent() );
}


KFCNewLine::~KFCNewLine()
{
    delete m_newline;
}


void KFCNewLine::execute()
{
    FormulaCursor* cursor = getExecuteCursor();
    MultilineElement* parent = static_cast<MultilineElement*>( m_line->getParent() );
    int linePos = parent->content.find( m_line );
    parent->content.insert( linePos+1, m_newline );

    // If there are children to be moved.
    if ( m_line->countChildren() > static_cast<int>( m_pos ) ) {

        // Remove anything after position pos from the current line
        m_line->selectAllChildren( cursor );
        cursor->setMark( m_pos );
        QPtrList<BasicElement> elementList;
        m_line->remove( cursor, elementList, beforeCursor );

        // Insert the removed stuff into the new line
        m_newline->goInside( cursor );
        m_newline->insert( cursor, elementList, beforeCursor );
        cursor->setPos( cursor->getMark() );
    }
    else {
        m_newline->goInside( cursor );
    }

    // The command no longer owns the new line.
    m_newline = 0;

    // Tell that something changed
    FormulaElement* formula = m_line->formula();
    formula->changed();
    testDirty();
}


void KFCNewLine::unexecute()
{
    FormulaCursor* cursor = getExecuteCursor();
    MultilineElement* parent = static_cast<MultilineElement*>( m_line->getParent() );
    int linePos = parent->content.find( m_line );

    // Now the command owns the new line again.
    m_newline = parent->content.at( linePos+1 );

    // Tell all cursors to leave this sequence
    FormulaElement* formula = m_line->formula();
    formula->elementRemoval( m_newline );

    // If there are children to be moved.
    if ( m_newline->countChildren() > 0 ) {

        // Remove anything from the line to be deleted
        m_newline->selectAllChildren( cursor );
        QPtrList<BasicElement> elementList;
        m_newline->remove( cursor, elementList, beforeCursor );

        // Insert the removed stuff into the previous line
        m_line->moveEnd( cursor );
        m_line->insert( cursor, elementList, beforeCursor );
        cursor->setPos( cursor->getMark() );
    }
    else {
        m_line->moveEnd( cursor );
    }
    parent->content.take( linePos+1 );

    // Tell that something changed
    formula->changed();
    testDirty();
}


MultilineSequenceElement::MultilineSequenceElement( BasicElement* parent )
    : SequenceElement( parent )
{
    tabs.setAutoDelete( false );
}


BasicElement* MultilineSequenceElement::goToPos( FormulaCursor* cursor, bool& handled,
                                                 const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    //LuPixelPoint myPos(parentOrigin.x() + getX(),
    //                   parentOrigin.y() + getY());
    BasicElement* e = inherited::goToPos(cursor, handled, point, parentOrigin);

    if (e == 0) {
        // If the mouse was behind this line put the cursor to the last position.
        if ( ( point.x() > getX()+getWidth() ) &&
             ( point.y() >= getY() ) &&
             ( point.y() < getY()+getHeight() ) ) {
            cursor->setTo(this, countChildren());
            handled = true;
            return this;
        }
    }
    return e;
}


void MultilineSequenceElement::calcSizes( const ContextStyle& context,
                                          ContextStyle::TextStyle tstyle,
                                          ContextStyle::IndexStyle istyle,
                                          StyleAttributes& style )
{
    tabs.clear();
    inherited::calcSizes( context, tstyle, istyle, style );
}


void MultilineSequenceElement::registerTab( BasicElement* tab )
{
    tabs.append( tab );
}


KCommand* MultilineSequenceElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        return 0;
    }

    switch ( *request ) {
    case req_remove: {
        // Remove this line if its empty.
        // Remove the formula if this line was the only one.
        break;
    }
    case req_addNewline: {
        FormulaCursor* cursor = container->activeCursor();
        return new KFCNewLine( i18n( "Add Newline" ), container, this, cursor->getPos() );
    }
    case req_addTabMark: {
        KFCReplace* command = new KFCReplace( i18n("Add Tabmark"), container );
        SpaceElement* element = new SpaceElement( THIN, true );
        command->addElement( element );
        return command;
    }
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}


KCommand* MultilineSequenceElement::input( Container* container, QKeyEvent* event )
{
    int action = event->key();
    //int state = event->state();
    //MoveFlag flag = movementFlag(state);

    switch ( action ) {
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        Request newline( req_addNewline );
        return buildCommand( container, &newline );
    }
    case Qt::Key_Tab: {
        Request r( req_addTabMark );
        return buildCommand( container, &r );
    }
    }
    return inherited::input( container, event );
}


KCommand* MultilineSequenceElement::input( Container* container, QChar ch )
{
    int latin1 = ch.latin1();
    switch (latin1) {
    case '&': {
        Request r( req_addTabMark );
        return buildCommand( container, &r );
    }
    }
    return inherited::input( container, ch );
}


void MultilineSequenceElement::moveTabTo( uint i, luPixel pos )
{
    BasicElement* marker = tab( i );
    luPixel diff = pos - marker->getX();
    marker->setWidth( marker->getWidth() + diff );

    for ( int p = childPos( marker )+1; p < countChildren(); ++p ) {
        BasicElement* child = getChild( p );
        child->setX( child->getX() + diff );
    }

    setWidth( getWidth()+diff );
}


int MultilineSequenceElement::tabBefore( uint pos )
{
    if ( tabs.isEmpty() ) {
        return -1;
    }
    uint tabNum = 0;
    for ( uint i=0; i<pos; ++i ) {
        BasicElement* child = getChild( i );
        if ( tabs.at( tabNum ) == child ) {
            if ( tabNum+1 == tabs.count() ) {
                return tabNum;
            }
            ++tabNum;
        }
    }
    return static_cast<int>( tabNum )-1;
}


int MultilineSequenceElement::tabPos( uint i )
{
    if ( i < tabs.count() ) {
        return childPos( tabs.at( i ) );
    }
    return -1;
}


void MultilineSequenceElement::writeMathML( QDomDocument& doc,
                                            QDomNode& parent, bool oasisFormat ) const
{
    // parent is required to be a <mtr> tag

    QDomElement tmp = doc.createElement( "TMP" );

    inherited::writeMathML( doc, tmp, oasisFormat );

    /* Now we re-parse the Dom tree, because of the TabMarkers
     * that have no direct representation in MathML but mark the
     * end of a <mtd> tag.
     */

    QDomElement mtd = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );

    // The mrow, if it exists.
    QDomNode n = tmp.firstChild().firstChild();
    while ( !n.isNull() ) {
        // the illegal TabMarkers are children of the mrow, child of tmp.
        if ( n.isElement() && n.toElement().tagName() == "TAB" ) {
            parent.appendChild( mtd );
            mtd = doc.createElement( oasisFormat ? "math:mtd" : "mtd" );
        }
        else {
            mtd.appendChild( n.cloneNode() ); // cloneNode needed?
        }
        n = n.nextSibling();
    }

    parent.appendChild( mtd );
}


MultilineElement::MultilineElement( BasicElement* parent )
    : BasicElement( parent )
{
    content.setAutoDelete( true );
    content.append( new MultilineSequenceElement( this ) );
}

MultilineElement::~MultilineElement()
{
}

MultilineElement::MultilineElement( const MultilineElement& other )
    : BasicElement( other )
{
    content.setAutoDelete( true );
    uint count = other.content.count();
    for (uint i = 0; i < count; i++) {
        MultilineSequenceElement* line = content.at(i)->clone();
        line->setParent( this );
        content.append( line );
    }
}


bool MultilineElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}


void MultilineElement::entered( SequenceElement* /*child*/ )
{
    formula()->tell( i18n( "Multi line element" ) );
}


/**
 * Returns the element the point is in.
 */
BasicElement* MultilineElement::goToPos( FormulaCursor* cursor, bool& handled,
                                         const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = inherited::goToPos(cursor, handled, point, parentOrigin);
    if ( e != 0 ) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        uint count = content.count();
        for ( uint i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            e = line->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                return e;
            }
        }
        return this;
    }
    return 0;
}

void MultilineElement::goInside( FormulaCursor* cursor )
{
    content.at( 0 )->goInside( cursor );
}

void MultilineElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    // If you want to select more than one line you'll have to
    // select the whole element.
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        // Coming from the parent (sequence) we go to
        // the very last position
        if (from == getParent()) {
            content.at( content.count()-1 )->moveLeft(cursor, this);
        }
        else {
            // Coming from one of the lines we go to the previous line
            // or to the parent if there is none.
            int pos = content.find( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                if ( pos > 0 ) {
                    content.at( pos-1 )->moveLeft( cursor, this );
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                kdDebug( DEBUGID ) << k_funcinfo << endl;
                kdDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
            }
        }
    }
}

void MultilineElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            content.at( 0 )->moveRight(cursor, this);
        }
        else {
            int pos = content.find( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                uint upos = pos;
                if ( upos < content.count() ) {
                    if ( upos < content.count()-1 ) {
                        content.at( upos+1 )->moveRight( cursor, this );
                    }
                    else {
                        getParent()->moveRight(cursor, this);
                    }
                    return;
                }
            }
            kdDebug( DEBUGID ) << k_funcinfo << endl;
            kdDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
        }
    }
}

void MultilineElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    // If you want to select more than one line you'll have to
    // select the whole element.
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        // Coming from the parent (sequence) we go to
        // the very last position
        if (from == getParent()) {
            content.at( content.count()-1 )->moveLeft(cursor, this);
        }
        else {
            // Coming from one of the lines we go to the previous line
            // or to the parent if there is none.
            int pos = content.find( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                if ( pos > 0 ) {
                    //content.at( pos-1 )->moveLeft( cursor, this );
                    // This is rather hackish.
                    // But we know what elements we have here.
                    int cursorPos = cursor->getPos();
                    MultilineSequenceElement* current = content.at( pos );
                    MultilineSequenceElement* newLine = content.at( pos-1 );
                    int tabNum = current->tabBefore( cursorPos );
                    if ( tabNum > -1 ) {
                        int oldTabPos = current->tabPos( tabNum );
                        int newTabPos = newLine->tabPos( tabNum );
                        if ( newTabPos > -1 ) {
                            cursorPos += newTabPos-oldTabPos;
                            int nextNewTabPos = newLine->tabPos( tabNum+1 );
                            if ( nextNewTabPos > -1 ) {
                                cursorPos = QMIN( cursorPos, nextNewTabPos );
                            }
                        }
                        else {
                            cursorPos = newLine->countChildren();
                        }
                    }
                    else {
                        int nextNewTabPos = newLine->tabPos( 0 );
                        if ( nextNewTabPos > -1 ) {
                            cursorPos = QMIN( cursorPos, nextNewTabPos );
                        }
                    }
                    cursor->setTo( newLine,
                                   QMIN( cursorPos,
                                         newLine->countChildren() ) );
                }
                else {
                    getParent()->moveLeft(cursor, this);
                }
            }
            else {
                kdDebug( DEBUGID ) << k_funcinfo << endl;
                kdDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
            }
        }
    }
}

void MultilineElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        if (from == getParent()) {
            content.at( 0 )->moveRight(cursor, this);
        }
        else {
            int pos = content.find( static_cast<MultilineSequenceElement*>( from ) );
            if ( pos > -1 ) {
                uint upos = pos;
                if ( upos < content.count() ) {
                    if ( upos < content.count()-1 ) {
                        //content.at( upos+1 )->moveRight( cursor, this );
                        // This is rather hackish.
                        // But we know what elements we have here.
                        int cursorPos = cursor->getPos();
                        MultilineSequenceElement* current = content.at( upos );
                        MultilineSequenceElement* newLine = content.at( upos+1 );
                        int tabNum = current->tabBefore( cursorPos );
                        if ( tabNum > -1 ) {
                            int oldTabPos = current->tabPos( tabNum );
                            int newTabPos = newLine->tabPos( tabNum );
                            if ( newTabPos > -1 ) {
                                cursorPos += newTabPos-oldTabPos;
                                int nextNewTabPos = newLine->tabPos( tabNum+1 );
                                if ( nextNewTabPos > -1 ) {
                                    cursorPos = QMIN( cursorPos, nextNewTabPos );
                                }
                            }
                            else {
                                cursorPos = newLine->countChildren();
                            }
                        }
                        else {
                            int nextNewTabPos = newLine->tabPos( 0 );
                            if ( nextNewTabPos > -1 ) {
                                cursorPos = QMIN( cursorPos, nextNewTabPos );
                            }
                        }
                        cursor->setTo( newLine,
                                       QMIN( cursorPos,
                                             newLine->countChildren() ) );
                    }
                    else {
                        getParent()->moveRight(cursor, this);
                    }
                    return;
                }
            }
            kdDebug( DEBUGID ) << k_funcinfo << endl;
            kdDebug( DEBUGID ) << "Serious confusion. Must never happen." << endl;
        }
    }
}


void MultilineElement::calcSizes( const ContextStyle& context,
                                  ContextStyle::TextStyle tstyle,
                                  ContextStyle::IndexStyle istyle,
                                  StyleAttributes& style )
{
    double factor = style.sizeFactor();
    luPt mySize = context.getAdjustedSize( tstyle, factor );
    QFont font = context.getDefaultFont();
    font.setPointSizeFloat( context.layoutUnitPtToPt( mySize ) );
    QFontMetrics fm( font );
    luPixel leading = context.ptToLayoutUnitPt( fm.leading() );
    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );

    uint count = content.count();
    luPixel height = -leading;
    luPixel width = 0;
    uint tabCount = 0;
    for ( uint i = 0; i < count; ++i ) {
        MultilineSequenceElement* line = content.at(i);
        line->calcSizes( context, tstyle, istyle, style );
        tabCount = QMAX( tabCount, line->tabCount() );

        height += leading;
        line->setX( 0 );
        line->setY( height );
        height += line->getHeight() + distY;
        width = QMAX( line->getWidth(), width );
    }

    // calculate the tab positions
    for ( uint t = 0; t < tabCount; ++t ) {
        luPixel pos = 0;
        for ( uint i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            if ( t < line->tabCount() ) {
                pos = QMAX( pos, line->tab( t )->getX() );
            }
            else {
                pos = QMAX( pos, line->getWidth() );
            }
        }
        for ( uint i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            if ( t < line->tabCount() ) {
                line->moveTabTo( t, pos );
                width = QMAX( width, line->getWidth() );
            }
        }
    }

    setHeight( height );
    setWidth( width );
    if ( count == 1 ) {
        setBaseline( content.at( 0 )->getBaseline() );
    }
    else {
        // There's always a first line. No formulas without lines.
        setBaseline( height/2 + context.axisHeight( tstyle, factor ) );
    }
}

void MultilineElement::draw( QPainter& painter, const LuPixelRect& r,
                             const ContextStyle& context,
                             ContextStyle::TextStyle tstyle,
                             ContextStyle::IndexStyle istyle,
                             StyleAttributes& style,
                             const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );
    uint count = content.count();

    if ( context.edit() ) {
        uint tabCount = 0;
        painter.setPen( context.getHelpColor() );
        for ( uint i = 0; i < count; ++i ) {
            MultilineSequenceElement* line = content.at(i);
            if ( tabCount < line->tabCount() ) {
                for ( uint t = tabCount; t < line->tabCount(); ++t ) {
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

    for ( uint i = 0; i < count; ++i ) {
        MultilineSequenceElement* line = content.at(i);
        line->draw( painter, r, context, tstyle, istyle, style, myPos );
    }
}


void MultilineElement::dispatchFontCommand( FontCommand* cmd )
{
    uint count = content.count();
    for ( uint i = 0; i < count; ++i ) {
        MultilineSequenceElement* line = content.at(i);
        line->dispatchFontCommand( cmd );
    }
}

void MultilineElement::insert( FormulaCursor* cursor,
                               QPtrList<BasicElement>& newChildren,
                               Direction direction )
{
    MultilineSequenceElement* e = static_cast<MultilineSequenceElement*>(newChildren.take(0));
    e->setParent(this);
    content.insert( cursor->getPos(), e );

    if (direction == beforeCursor) {
        e->moveLeft(cursor, this);
    }
    else {
        e->moveRight(cursor, this);
    }
    cursor->setSelection(false);
    formula()->changed();
}

void MultilineElement::remove( FormulaCursor* cursor,
                               QPtrList<BasicElement>& removedChildren,
                               Direction direction )
{
    if ( content.count() == 1 ) { //&& ( cursor->getPos() == 0 ) ) {
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
    }
    else {
        MultilineSequenceElement* e = content.take( cursor->getPos() );
        removedChildren.append( e );
        formula()->elementRemoval( e );
        //cursor->setTo( this, denominatorPos );
        formula()->changed();
    }
}

void MultilineElement::normalize( FormulaCursor* cursor, Direction direction )
{
    int pos = cursor->getPos();
    if ( ( cursor->getElement() == this ) &&
         ( pos > -1 ) && ( static_cast<unsigned>( pos ) <= content.count() ) ) {
        switch ( direction ) {
        case beforeCursor:
            if ( pos > 0 ) {
                content.at( pos-1 )->moveLeft( cursor, this );
                break;
            }
            // no break! intended!
        case afterCursor:
            if ( static_cast<unsigned>( pos ) < content.count() ) {
                content.at( pos )->moveRight( cursor, this );
            }
            else {
                content.at( pos-1 )->moveLeft( cursor, this );
            }
            break;
        }
    }
    else {
        inherited::normalize( cursor, direction );
    }
}

SequenceElement* MultilineElement::getMainChild()
{
    return content.at( 0 );
}

void MultilineElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    int pos = content.find( dynamic_cast<MultilineSequenceElement*>( child ) );
    if ( pos > -1 ) {
        cursor->setTo( this, pos );
        //content.at( pos )->moveRight( cursor, this );
    }
}


/**
 * Appends our attributes to the dom element.
 */
void MultilineElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    uint lineCount = content.count();
    element.setAttribute( "LINES", lineCount );

    QDomDocument doc = element.ownerDocument();
    for ( uint i = 0; i < lineCount; ++i ) {
        QDomElement tmp = content.at( i )->getElementDom(doc);
        element.appendChild(tmp);
    }
}

void MultilineElement::writeMathML( QDomDocument& doc, QDomNode& parent, bool oasisFormat ) const
{
    QDomElement de = doc.createElement( oasisFormat ? "math:mtable" : "mtable" );
    QDomElement row; QDomElement cell;

    for ( QPtrListIterator < MultilineSequenceElement > it( content ); it.current(); ++it ) {
        row = doc.createElement( oasisFormat ? "math:mtr" : "mtr" );
        de.appendChild( row );
        //cell = doc.createElement( "mtd" );
        //row.appendChild( cell );

        //content.at( i )->writeMathML( doc, cell );
        it.current()->writeMathML( doc, row, oasisFormat );
    }

    parent.appendChild( de );
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool MultilineElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    uint lineCount = 0;
    QString lineCountStr = element.attribute("LINES");
    if(!lineCountStr.isNull()) {
        lineCount = lineCountStr.toInt();
    }
    if (lineCount == 0) {
        kdWarning( DEBUGID ) << "lineCount <= 0 in MultilineElement." << endl;
        return false;
    }

    content.clear();
    for ( uint i = 0; i < lineCount; ++i ) {
        MultilineSequenceElement* element = new MultilineSequenceElement(this);
        content.append(element);
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool MultilineElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    uint lineCount = content.count();
    uint i = 0;
    while ( !node.isNull() && i < lineCount ) {
        if ( node.isElement() ) {
            SequenceElement* element = content.at( i );
            QDomElement e = node.toElement();
            if ( !element->buildFromDom( e ) ) {
                return false;
            }
            ++i;
        }
        node = node.nextSibling();
    }
    return true;
}

QString MultilineElement::toLatex()
{
    uint lineCount = content.count();
    QString muliline = "\\begin{split} ";
    for ( uint i = 0; i < lineCount; ++i ) {
        muliline += content.at( i )->toLatex();
    	muliline += " \\\\ ";
    }
    muliline += "\\end{split}";
    return muliline;
}

// Does this make any sense at all?
QString MultilineElement::formulaString()
{
    uint lineCount = content.count();
    QString muliline = "";
    for ( uint i = 0; i < lineCount; ++i ) {
        muliline += content.at( i )->formulaString();
    	muliline += "\n";
    }
    //muliline += "";
    return muliline;
}


KFORMULA_NAMESPACE_END
