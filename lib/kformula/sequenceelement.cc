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

#include <stdlib.h>
#include <math.h>

#include <qpainter.h>
#include <qpaintdevice.h>

#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "MatrixDialog.h"
#include "bracketelement.h"
#include "elementtype.h"
#include "formulacursor.h"
#include "formulaelement.h"
#include "fractionelement.h"
#include "indexelement.h"
#include "kformulacommand.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "matrixelement.h"
#include "rootelement.h"
#include "sequenceelement.h"
#include "sequenceparser.h"
#include "spaceelement.h"
#include "symbolelement.h"
#include "symboltable.h"
#include "textelement.h"


KFORMULA_NAMESPACE_BEGIN
using namespace std;


SequenceElement::SequenceElement(BasicElement* parent)
        : BasicElement(parent), parseTree(0), textSequence(true)
{
    children.setAutoDelete(true);
}


SequenceElement::~SequenceElement()
{
    delete parseTree;
}

/**
 * Returns the element the point is in.
 */
BasicElement* SequenceElement::goToPos( FormulaCursor* cursor, bool& handled,
                                        const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());

        uint count = children.count();
        for (uint i = 0; i < count; i++) {
            BasicElement* child = children.at(i);
            e = child->goToPos(cursor, handled, point, myPos);
            if (e != 0) {
                if (!handled) {
                    handled = true;
                    if ((point.x() - myPos.x()) < (e->getX() + e->getWidth()*2/3)) {
                        cursor->setTo(this, children.find(e));
                    }
                    else {
                        cursor->setTo(this, children.find(e)+1);
                    }
                }
                return e;
            }
        }

        luPixel dx = point.x() - myPos.x();
        //int dy = point.y() - myPos.y();

        for (uint i = 0; i < count; i++) {
            BasicElement* child = children.at(i);
            if (dx < child->getX()) {
                cursor->setTo( this, i );
                handled = true;
                return children.at( i );
            }
        }

        cursor->setTo(this, countChildren());
        handled = true;
        return this;
    }
    return 0;
}


bool SequenceElement::isEmpty()
{
    uint count = children.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = children.at(i);
        if (!child->isInvisible()) {
            return false;
        }
    }
    return true;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void SequenceElement::calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle)
{
    if (!isEmpty()) {
        luPixel width = 0;
        luPixel toBaseline = 0;
        luPixel fromBaseline = 0;

        // Let's do all normal elements that have a base line.
        QPtrListIterator<BasicElement> it( children );
        for ( ; it.current(); ++it ) {
            BasicElement* child = it.current();

            luPixel spaceBefore = 0;
            if ( isFirstOfToken( child ) ) {
                spaceBefore = style.ptToPixelX( child->getElementType()->getSpaceBefore( style, tstyle ) );
            }

            if ( !child->isInvisible() ) {
                child->calcSizes( style, tstyle, istyle );
                child->setX( width + spaceBefore );
                width += child->getWidth() + spaceBefore;

                toBaseline = QMAX( toBaseline, child->getBaseline() );
                fromBaseline = QMAX( fromBaseline, child->getHeight()-child->getBaseline() );
            }
            else {
                width += spaceBefore;
                child->setX( width );
            }
        }

        setWidth(width);
        setHeight(toBaseline+fromBaseline);
        setBaseline(toBaseline);

        setChildrenPositions();
    }
    else {
        luPixel w = style.getEmptyRectWidth();
        luPixel h = style.getEmptyRectHeight();
        setWidth( w );
        setHeight( h );
        setBaseline( h );
        //setMidline( h*.5 );
    }
}


void SequenceElement::setChildrenPositions()
{
    QPtrListIterator<BasicElement> it( children );
    for ( ; it.current(); ++it ) {
        BasicElement* child = it.current();
        child->setY(getBaseline() - child->getBaseline());
    }
}


/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void SequenceElement::draw( QPainter& painter, const LuPixelRect& r,
                            const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x() + getX(), parentOrigin.y() + getY() );
    if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
        return;

    if (!isEmpty()) {
        QPtrListIterator<BasicElement> it( children );
        for ( ; it.current(); ++it ) {
            BasicElement* child = it.current();
	    if (!child->isInvisible()) {
                child->draw(painter, r, context, tstyle, istyle, myPos);
            }
        }
    }
    else {
        drawEmptyRect( painter, context, myPos );
    }
    // Debug
    //painter.setPen(Qt::green);
    //painter.drawRect(parentOrigin.x() + getX(), parentOrigin.y() + getY(),
    //                 getWidth(), getHeight());
//     painter.drawLine( context.layoutUnitToPixelX( parentOrigin.x() + getX() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + axis( context, tstyle ) ),
//                       context.layoutUnitToPixelX( parentOrigin.x() + getX() + getWidth() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + axis( context, tstyle ) ) );
//     painter.setPen(Qt::red);
//     painter.drawLine( context.layoutUnitToPixelX( parentOrigin.x() + getX() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + getBaseline() ),
//                       context.layoutUnitToPixelX( parentOrigin.x() + getX() + getWidth() ),
//                       context.layoutUnitToPixelY( parentOrigin.y() + getY() + getBaseline() ) );
}

void SequenceElement::drawEmptyRect( QPainter& painter, const ContextStyle& context,
                                     const LuPixelPoint& upperLeft )
{
    if ( context.edit() ) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen( QPen( context.getEmptyColor(),
                              context.layoutUnitToPixelX( context.getLineWidth() ) ) );
        painter.drawRect( context.layoutUnitToPixelX( upperLeft.x() ),
                          context.layoutUnitToPixelY( upperLeft.y() ),
                          context.layoutUnitToPixelX( getWidth() ),
                          context.layoutUnitToPixelY( getHeight() ) );
    }
}

void SequenceElement::calcCursorSize( const ContextStyle& context,
                                      FormulaCursor* cursor, bool smallCursor )
{
    LuPixelPoint point = widgetPos();
    uint pos = cursor->getPos();

    luPixel posX = getChildPosition( context, pos );
    luPixel height = getHeight();

    luPixel unitX = context.ptToLayoutUnitPixX( 1 );
    luPixel unitY = context.ptToLayoutUnitPixY( 1 );

    // Here are those evil constants that describe the cursor size.

    if ( cursor->isSelection() ) {
        uint mark = cursor->getMark();
        luPixel markX = getChildPosition( context, mark );
        luPixel x = QMIN(posX, markX);
        luPixel width = abs(posX - markX);

        if ( smallCursor ) {
            cursor->cursorSize.setRect( point.x()+x, point.y(), width, height );
        }
        else {
            cursor->cursorSize.setRect( point.x()+x, point.y() - 2*unitY,
                                        width + unitX, height + 4*unitY );
        }
        cursor->selectionArea = cursor->cursorSize;
    }
    else {
        if ( smallCursor ) {
            cursor->cursorSize.setRect( point.x()+posX, point.y(),
                                        unitX, height );
        }
        else {
            cursor->cursorSize.setRect( point.x(), point.y() - 2*unitY,
                                        getWidth() + unitX, height + 4*unitY );
        }
    }

    cursor->cursorPoint.setX( point.x()+posX );
    cursor->cursorPoint.setY( point.y()+getHeight()/2 );
}


/**
 * If the cursor is inside a sequence it needs to be drawn.
 */
void SequenceElement::drawCursor( QPainter& painter, const ContextStyle& context,
                                  FormulaCursor* cursor, bool smallCursor )
{
    painter.setRasterOp( Qt::XorROP );
    if ( cursor->isSelection() ) {
        const LuPixelRect& r = cursor->selectionArea;
        painter.fillRect( context.layoutUnitToPixelX( r.x() ),
                          context.layoutUnitToPixelY( r.y() ),
                          context.layoutUnitToPixelX( r.width() ),
                          context.layoutUnitToPixelY( r.height() ),
                          Qt::white );
    }
    else {
        painter.setPen( QPen( Qt::white,
                              context.layoutUnitToPixelX( context.getLineWidth()/2 ) ) );
        const LuPixelPoint& point = cursor->getCursorPoint();
        const LuPixelRect& size = cursor->getCursorSize();
        if ( smallCursor ) {
            painter.drawLine( context.layoutUnitToPixelX( point.x() ),
                              context.layoutUnitToPixelY( size.top() ),
                              context.layoutUnitToPixelX( point.x() ),
                              context.layoutUnitToPixelY( size.bottom() ) );
        }
        else {
            painter.drawLine( context.layoutUnitToPixelX( point.x() ),
                              context.layoutUnitToPixelY( size.top() ),
                              context.layoutUnitToPixelX( point.x() ),
                              context.layoutUnitToPixelY( size.bottom()-1 ) );
            painter.drawLine( context.layoutUnitToPixelX( size.left() ),
                              context.layoutUnitToPixelY( size.bottom() ),
                              context.layoutUnitToPixelX( size.right() ),
                              context.layoutUnitToPixelY( size.bottom() ) );
        }
    }
    // This might be wrong but probably isn't.
    painter.setRasterOp( Qt::CopyROP );
}


luPixel SequenceElement::getChildPosition( const ContextStyle& context, uint child )
{
    if (child < children.count()) {
        return children.at(child)->getX();
    }
    else {
        if (children.count() > 0) {
            return children.at(child-1)->getX() + children.at(child-1)->getWidth();
        }
        else {
            return context.ptToLayoutUnitPixX( 2 );
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
void SequenceElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    // Our parent asks us for a cursor position. Found.
    if (from == getParent()) {
        cursor->setTo(this, children.count());
    }

    // We already owned the cursor. Ask next child then.
    else if (from == this) {
        if (cursor->getPos() > 0) {
            if (cursor->isSelectionMode()) {
                cursor->setTo(this, cursor->getPos()-1);

                // invisible elements are not visible so we move on.
                if (children.at(cursor->getPos())->isInvisible()) {
                    moveLeft(cursor, this);
                }
            }
            else {
                children.at(cursor->getPos()-1)->moveLeft(cursor, this);
            }
        }
        else {
            // Needed because FormulaElement derives this.
            if (getParent() != 0) {
                getParent()->moveLeft(cursor, this);
            }
            else {
                formula()->moveOutLeft( cursor );
            }
        }
    }

    // The cursor came from one of our children or
    // something is wrong.
    else {
        int fromPos = children.find(from);
        cursor->setTo(this, fromPos);
        if (cursor->isSelectionMode()) {
            cursor->setMark(fromPos+1);
        }

        // invisible elements are not visible so we move on.
        if (from->isInvisible()) {
            moveLeft(cursor, this);
        }
    }
}

/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void SequenceElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    // Our parent asks us for a cursor position. Found.
    if (from == getParent()) {
        cursor->setTo(this, 0);
    }

    // We already owned the cursor. Ask next child then.
    else if (from == this) {
        uint pos = cursor->getPos();
        if (pos < children.count()) {
            if (cursor->isSelectionMode()) {
                cursor->setTo(this, pos+1);

                // invisible elements are not visible so we move on.
                if (children.at(pos)->isInvisible()) {
                    moveRight(cursor, this);
                }
            }
            else {
                children.at(pos)->moveRight(cursor, this);
            }
        }
        else {
            // Needed because FormulaElement derives this.
            if (getParent() != 0) {
                getParent()->moveRight(cursor, this);
            }
            else {
                formula()->moveOutRight( cursor );
            }
        }
    }

    // The cursor came from one of our children or
    // something is wrong.
    else {
        int fromPos = children.find(from);
        cursor->setTo(this, fromPos+1);
        if (cursor->isSelectionMode()) {
            cursor->setMark(fromPos);
        }

        // invisible elements are not visible so we move on.
        if (from->isInvisible()) {
            moveRight(cursor, this);
        }
    }
}


void SequenceElement::moveWordLeft(FormulaCursor* cursor)
{
    uint pos = cursor->getPos();
    if (pos > 0) {
        ElementType* type = children.at(pos-1)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->start());
        }
    }
    else {
        moveLeft(cursor, this);
    }
}


void SequenceElement::moveWordRight(FormulaCursor* cursor)
{
    uint pos = cursor->getPos();
    if (pos < children.count()) {
        ElementType* type = children.at(pos)->getElementType();
        if (type != 0) {
            cursor->setTo(this, type->end());
        }
    }
    else {
        moveRight(cursor, this);
    }
}


/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void SequenceElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (from == getParent()) {
        moveRight(cursor, this);
    }
    else {
        if (getParent() != 0) {
            getParent()->moveUp(cursor, this);
        }
    }
}

/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void SequenceElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (from == getParent()) {
        moveRight(cursor, this);
    }
    else {
        if (getParent() != 0) {
            getParent()->moveDown(cursor, this);
        }
    }
}

/**
 * Moves the cursor to the first position in this sequence.
 * (That is before the first child.)
 */
void SequenceElement::moveHome(FormulaCursor* cursor)
{
    if (cursor->isSelectionMode()) {
        BasicElement* element = cursor->getElement();
        if (element != this) {
            while (element->getParent() != this) {
                element = element->getParent();
            }
            cursor->setMark(children.find(element)+1);
        }
    }
    cursor->setTo(this, 0);
}

/**
 * Moves the cursor to the last position in this sequence.
 * (That is behind the last child.)
 */
void SequenceElement::moveEnd(FormulaCursor* cursor)
{
    if (cursor->isSelectionMode()) {
        BasicElement* element = cursor->getElement();
        if (element != this) {
            while (element->getParent() != this) {
                element = element->getParent();
                if (element == 0) {
                    cursor->setMark(children.count());
                    break;
                }
            }
            if (element != 0) {
                cursor->setMark(children.find(element));
            }
        }
    }
    cursor->setTo(this, children.count());
}

/**
 * Sets the cursor inside this element to its start position.
 * For most elements that is the main child.
 */
void SequenceElement::goInside(FormulaCursor* cursor)
{
    cursor->setSelection(false);
    cursor->setTo(this, 0);
}


// children

/**
 * Removes the child. If this was the main child this element might
 * request its own removal.
 * The cursor is the one that caused the removal. It has to be moved
 * to the place any user expects the cursor after that particular
 * element has been removed.
 */
// void SequenceElement::removeChild(FormulaCursor* cursor, BasicElement* child)
// {
//     int pos = children.find(child);
//     formula()->elementRemoval(child, pos);
//     cursor->setTo(this, pos);
//     children.remove(pos);
//     /*
//         if len(self.children) == 0:
//             if self.parent != None:
//                 self.parent.removeChild(cursor, self)
//                 return
//     */
//     formula()->changed();
// }


/**
 * Inserts all new children at the cursor position. Places the
 * cursor according to the direction. The inserted elements will
 * be selected.
 *
 * The list will be emptied but stays the property of the caller.
 */
void SequenceElement::insert(FormulaCursor* cursor,
                             QPtrList<BasicElement>& newChildren,
                             Direction direction)
{
    int pos = cursor->getPos();
    uint count = newChildren.count();
    for (uint i = 0; i < count; i++) {
        BasicElement* child = newChildren.take(0);
        child->setParent(this);
        children.insert(pos+i, child);
    }
    if (direction == beforeCursor) {
        cursor->setTo(this, pos+count, pos);
    }
    else {
        cursor->setTo(this, pos, pos+count);
    }
    formula()->changed();
    parse();
}


/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * The ownership of the list is passed to the caller.
 */
void SequenceElement::remove(FormulaCursor* cursor,
                             QPtrList<BasicElement>& removedChildren,
                             Direction direction)
{
    if (cursor->isSelection()) {
        int from = cursor->getSelectionStart();
        int to = cursor->getSelectionEnd();
        for (int i = from; i < to; i++) {
            removeChild(removedChildren, from);
        }
        cursor->setTo(this, from);
        cursor->setSelection(false);
    }
    else {
        if (direction == beforeCursor) {
            int pos = cursor->getPos() - 1;
            if (pos >= 0) {
                while (pos >= 0) {
                    BasicElement* child = children.at(pos);
                    formula()->elementRemoval(child);
                    children.take(pos);
                    removedChildren.prepend(child);
                    if (!child->isInvisible()) {
                        break;
                    }
                    pos--;
                }
                cursor->setTo(this, pos);
                formula()->changed();
            }
        }
        else {
            uint pos = cursor->getPos();
            if (pos < children.count()) {
                while (pos < children.count()) {
                    BasicElement* child = children.at(pos);
                    formula()->elementRemoval(child);
                    children.take(pos);
                    removedChildren.append(child);
                    if (!child->isInvisible()) {
                        break;
                    }
                }
                // It is necessary to set the cursor to its old
                // position because it got a notification and
                // moved to the beginning of this sequence.
                cursor->setTo(this, pos);
                formula()->changed();
            }
        }
    }
    parse();
}


/**
 * Removes the children at pos and appends it to the list.
 */
void SequenceElement::removeChild(QPtrList<BasicElement>& removedChildren, int pos)
{
    BasicElement* child = children.at(pos);
    formula()->elementRemoval(child);
    children.take(pos);
    removedChildren.append(child);
    //cerr << *removedChildren.at(0) << endl;
    formula()->changed();
}


/**
 * Moves the cursor to a normal place where new elements
 * might be inserted.
 */
void SequenceElement::normalize(FormulaCursor* cursor, Direction)
{
    cursor->setSelection(false);
}


/**
 * Returns the child at the cursor.
 * Does not care about the selection.
 */
BasicElement* SequenceElement::getChild( FormulaCursor* cursor, Direction direction )
{
    if ( direction == beforeCursor ) {
        if ( cursor->getPos() > 0 ) {
            return children.at( cursor->getPos() - 1 );
        }
    }
    else {
        if ( cursor->getPos() < qRound( children.count() ) ) {
            return children.at( cursor->getPos() );
        }
    }
    return 0;
}


/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void SequenceElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    int pos = children.find(child);
    if (pos > -1) {
        cursor->setTo(this, pos+1, pos);
    }
}

void SequenceElement::childWillVanish(FormulaCursor* cursor, BasicElement* child)
{
    int childPos = children.find(child);
    if (childPos > -1) {
        int pos = cursor->getPos();
        if (pos > childPos) {
            pos--;
        }
        int mark = cursor->getMark();
        if (mark > childPos) {
            mark--;
        }
        cursor->setTo(this, pos, mark);
    }
}


/**
 * Selects all children. The cursor is put behind, the mark before them.
 */
void SequenceElement::selectAllChildren(FormulaCursor* cursor)
{
    cursor->setTo(this, children.count(), 0);
}

bool SequenceElement::onlyTextSelected( FormulaCursor* cursor )
{
    if ( cursor->isSelection() ) {
        uint from = QMIN( cursor->getPos(), cursor->getMark() );
        uint to = QMAX( cursor->getPos(), cursor->getMark() );
        for ( uint i = from; i < to; i++ ) {
            BasicElement* element = getChild( i );
            if ( element->getCharacter() == QChar::null ) {
                return false;
            }
        }
    }
    return true;
}


KCommand* SequenceElement::buildCommand( Container* container, Request* request )
{
    switch ( *request ) {
    case req_addText: {
        KFCReplace* command = new KFCReplace( i18n("Add Text"), container );
        TextRequest* tr = static_cast<TextRequest*>( request );
        for ( uint i = 0; i < tr->text().length(); i++ ) {
            command->addElement( new TextElement( tr->text()[i] ) );
        }
        return command;
    }
    case req_addTextChar: {
        KFCReplace* command = new KFCReplace( i18n("Add Text"), container );
        TextCharRequest* tr = static_cast<TextCharRequest*>( request );
        TextElement* element = new TextElement( tr->ch(), tr->isSymbol() );
        command->addElement( element );
        return command;
    }
    case req_addNameSequence:
        if ( onlyTextSelected( container->activeCursor() ) ) {
            //kdDebug( DEBUGID ) << "SequenceElement::buildCommand" << endl;
            KFCAddReplacing* command = new KFCAddReplacing( i18n( "Add Name" ), container );
            command->setElement( new NameSequence() );
            return command;
        }
        break;
    case req_addBracket: {
        KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Bracket"), container);
        BracketRequest* br = static_cast<BracketRequest*>( request );
        command->setElement( new BracketElement( br->left(), br->right() ) );
        return command;
    }
    case req_addSpace: {
        KFCReplace* command = new KFCReplace( i18n("Add Space"), container );
        SpaceRequest* sr = static_cast<SpaceRequest*>( request );
        SpaceElement* element = new SpaceElement( sr->space() );
        command->addElement( element );
        return command;
    }
    case req_addFraction: {
        KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Fraction"), container);
        command->setElement(new FractionElement());
        return command;
    }
    case req_addRoot: {
        KFCAddReplacing* command = new KFCAddReplacing(i18n("Add Root"), container);
        command->setElement(new RootElement());
        return command;
    }
    case req_addSymbol: {
        KFCAddReplacing* command = new KFCAddReplacing( i18n( "Add Symbol" ), container );
        SymbolRequest* sr = static_cast<SymbolRequest*>( request );
        command->setElement( new SymbolElement( sr->type() ) );
        return command;
    }
    case req_addOneByTwoMatrix: {
        KFCAddReplacing* command = new KFCAddReplacing( i18n("Add 1x2 Matrix"), container );
        FractionElement* element = new FractionElement();
        element->showLine(false);
        command->setElement(element);
        return command;
    }
    case req_addMatrix: {
        MatrixRequest* mr = static_cast<MatrixRequest*>( request );
        uint rows = mr->rows(), cols = mr->columns();
        if ( ( rows == 0 ) || ( cols == 0 ) ) {
            MatrixDialog* dialog = new MatrixDialog( 0 );
            if ( dialog->exec() ) {
                rows = dialog->h;
                cols = dialog->w;
            }
            delete dialog;
        }
        if ( ( rows != 0 ) && ( cols != 0 ) ) {
            KFCAddReplacing* command = new KFCAddReplacing( i18n( "Add Matrix" ), container );
            command->setElement( new MatrixElement( rows, cols ) );
            return command;
        }
    }
    case req_addIndex: {
        FormulaCursor* cursor = container->activeCursor();
        if ( cursor->getPos() > 0 && !cursor->isSelection() ) {
            IndexElement* element = dynamic_cast<IndexElement*>( children.at( cursor->getPos()-1 ) );
            if ( element != 0 ) {
                element->getMainChild()->goInside( cursor );
                return element->getMainChild()->buildCommand( container, request );
            }
        }
        IndexElement* element = new IndexElement;
        if ( !cursor->isSelection() ) {
            cursor->moveLeft( SelectMovement | WordMovement );
        }
        IndexRequest* ir = static_cast<IndexRequest*>( request );
        KFCAddIndex* command = new KFCAddIndex( container, element, element->getIndex( ir->index() ) );
        return command;
    }
    case req_removeEnclosing: {
        FormulaCursor* cursor = container->activeCursor();
        if ( !cursor->isSelection() ) {
            DirectedRemove* dr = static_cast<DirectedRemove*>( request );
            KFCRemoveEnclosing* command = new KFCRemoveEnclosing( container, dr->direction() );
            return command;
        }
    }
    case req_remove: {
        FormulaCursor* cursor = container->activeCursor();
        SequenceElement* sequence = cursor->normal();
        if ( sequence &&
             ( sequence == sequence->formula() ) &&
             ( sequence->countChildren() == 0 ) ) {
            sequence->formula()->removeFormula( cursor );
            return 0;
        }
        else {
            DirectedRemove* dr = static_cast<DirectedRemove*>( request );

            // empty removes are not legal!
            if ( !cursor->isSelection() ) {
                if ( countChildren() > 0 ) {
                    if ( ( cursor->getPos() == 0 ) && ( dr->direction() == beforeCursor ) ) {
                        return 0;
                    }
                    if ( ( cursor->getPos() == countChildren() ) && ( dr->direction() == afterCursor ) ) {
                        return 0;
                    }
                }
                else if ( getParent() == 0 ) {
                    return 0;
                }
            }

            KFCRemove* command = new KFCRemove( container, dr->direction() );
            return command;
        }
    }
    case req_compactExpression: {
        FormulaCursor* cursor = container->activeCursor();
        cursor->moveEnd();
        cursor->moveRight();
        formula()->cursorHasMoved( cursor );
        break;
    }
    case req_makeGreek: {
        FormulaCursor* cursor = container->activeCursor();
        TextElement* element = cursor->getActiveTextElement();
        if ((element != 0) && !element->isSymbol()) {
            cursor->selectActiveElement();
            const SymbolTable& table = container->document()->getSymbolTable();
            if (table.greekLetters().find(element->getCharacter()) != -1) {
                KFCReplace* command = new KFCReplace( i18n( "Change Char to Symbol" ), container );
                TextElement* symbol = new TextElement( table.unicodeFromSymbolFont( element->getCharacter() ), true );
                command->addElement( symbol );
                return command;
            }
            cursor->setSelection( false );
        }
        break;
    }
    case req_paste:
    case req_copy:
    case req_cut:
        break;
    default:
        break;
    }
    return 0;
}


KCommand* SequenceElement::input( Container* container, QKeyEvent* event )
{
    QChar ch = event->text().at( 0 );
    if ( ch.isPrint() ) {
        return input( container, ch );
    }
    else {
        int action = event->key();
        int state = event->state();
        MoveFlag flag = movementFlag(state);

	switch ( action ) {
        case Qt::Key_BackSpace: {
            DirectedRemove r( req_remove, beforeCursor );
            return buildCommand( container, &r );
        }
        case Qt::Key_Delete: {
            DirectedRemove r( req_remove, afterCursor );
            return buildCommand( container, &r );
        }
	case Qt::Key_Left: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveLeft( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Right: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveRight( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Up: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveUp( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Down: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveDown( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_Home: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveHome( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        case Qt::Key_End: {
            FormulaCursor* cursor = container->activeCursor();
            cursor->moveEnd( flag );
            formula()->cursorHasMoved( cursor );
            break;
        }
        default:
            if ( state & Qt::ControlButton ) {
                switch ( event->key() ) {
                case Qt::Key_AsciiCircum: {
                    IndexRequest r( upperLeftPos );
                    return buildCommand( container, &r );
                }
                case Qt::Key_Underscore: {
                    IndexRequest r( lowerLeftPos );
                    return buildCommand( container, &r );
                }
                default:
                    break;
                }
            }
        }
    }
    return 0;
}


KCommand* SequenceElement::input( Container* container, QChar ch )
{
    int latin1 = ch.latin1();
    switch (latin1) {
    case '(': {
        BracketRequest r( container->document()->leftBracketChar(),
                          container->document()->rightBracketChar() );
        return buildCommand( container, &r );
    }
    case '[': {
        BracketRequest r( LeftSquareBracket, RightSquareBracket );
        return buildCommand( container, &r );
    }
    case '{': {
        BracketRequest r( LeftCurlyBracket, RightCurlyBracket );
        return buildCommand( container, &r );
    }
    case '|': {
        BracketRequest r( LeftLineBracket, RightLineBracket );
        return buildCommand( container, &r );
    }
    case '^': {
        IndexRequest r( upperRightPos );
        return buildCommand( container, &r );
    }
    case '_': {
        IndexRequest r( lowerRightPos );
        return buildCommand( container, &r );
    }
    case ' ': {
        Request r( req_compactExpression );
        return buildCommand( container, &r );
    }
    case '}':
    case ']':
    case ')':
        break;
    case '\\': {
        Request r( req_addNameSequence );
        return buildCommand( container, &r );
    }
    default: {
        TextCharRequest r( ch );
        return buildCommand( container, &r );
    }
    }
    return 0;
}

/**
 * Stores the given childrens dom in the element.
 */
void SequenceElement::getChildrenDom(QDomDocument& doc, QDomElement& elem,
                                     uint from, uint to)
{
    for (uint i = from; i < to; i++) {
        QDomElement tmpEleDom=children.at(i)->getElementDom(doc);
	elem.appendChild(tmpEleDom);
    }
}


/**
 * Builds elements from the given node and its siblings and
 * puts them into the list.
 * Returns false if an error occures.
 */
bool SequenceElement::buildChildrenFromDom(QPtrList<BasicElement>& list, QDomNode n)
{
    while (!n.isNull()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            BasicElement* child = 0;
            QString tag = e.tagName().upper();

            child = createElement(tag);
            if (child != 0) {
                child->setParent(this);
                if (child->buildFromDom(e)) {
                    list.append(child);
                }
                else {
                    delete child;
                    return false;
                }
            }
            else {
                return false;
            }
        }
        n = n.nextSibling();
    }
    parse();
    return true;
}


BasicElement* SequenceElement::createElement( QString type )
{
    if      ( type == "TEXT" )         return new TextElement();
    else if ( type == "SPACE" )        return new SpaceElement();
    else if ( type == "ROOT" )         return new RootElement();
    else if ( type == "BRACKET" )      return new BracketElement();
    else if ( type == "MATRIX" )       return new MatrixElement();
    else if ( type == "INDEX" )        return new IndexElement();
    else if ( type == "FRACTION" )     return new FractionElement();
    else if ( type == "SYMBOL" )       return new SymbolElement();
    else if ( type == "NAMESEQUENCE" ) return new NameSequence();
    else if ( type == "SEQUENCE" ) {
        kdWarning( DEBUGID ) << "malformed data: sequence inside sequence." << endl;
        return 0;
    }
    return 0;
}

/**
 * Appends our attributes to the dom element.
 */
void SequenceElement::writeDom(QDomElement& element)
{
    BasicElement::writeDom(element);

    uint count = children.count();
    QDomDocument doc = element.ownerDocument();
    getChildrenDom(doc, element, 0, count);
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool SequenceElement::readAttributesFromDom(QDomElement& element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool SequenceElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    return buildChildrenFromDom(children, node);
}


void SequenceElement::parse()
{
    delete parseTree;

    textSequence = true;
    for (BasicElement* element = children.first();
         element != 0;
         element = children.next()) {

        // Those types are gone. Make sure they won't
        // be used.
        element->setElementType(0);

        if (element->getCharacter() == QChar::null) {
            textSequence = false;
        }
    }

    const SymbolTable& symbols = formula()->getSymbolTable();
    SequenceParser parser(symbols);
    parseTree = parser.parse(children);

    // debug
    //parseTree->output();
}


bool SequenceElement::isFirstOfToken( BasicElement* child )
{
    return ( child->getElementType() != 0 ) && isChildNumber( child->getElementType()->start(), child );
}


QString SequenceElement::toLatex()
{
    QString content;
    content += "{";
    uint count = children.count();
    for ( uint i = 0; i < count; i++ ) {
        BasicElement* child = children.at( i );
        if ( isFirstOfToken( child ) ) {
            content += " ";
        }
        content += child->toLatex();
    }
    content += "}";
    return content;
}

QString SequenceElement::formulaString()
{
    QString content;
    uint count = children.count();
    for ( uint i = 0; i < count; i++ ) {
        BasicElement* child = children.at( i );
        //if ( isFirstOfToken( child ) ) {
        //    content += " ";
        //}
        content += child->formulaString();
    }
    return content;
}



NameSequence::NameSequence( BasicElement* parent )
    : SequenceElement( parent )
{
}

void NameSequence::calcCursorSize( const ContextStyle& context,
                                   FormulaCursor* cursor, bool smallCursor )
{
    inherited::calcCursorSize( context, cursor, smallCursor );
    LuPixelPoint point = widgetPos();
    luPixel unitX = context.ptToLayoutUnitPixX( 1 );
    luPixel unitY = context.ptToLayoutUnitPixY( 1 );
    cursor->addCursorSize( LuPixelRect( point.x()-unitX, point.y()-unitY,
                                        getWidth()+2*unitX, getHeight()+2*unitY ) );
}

void NameSequence::drawCursor( QPainter& painter, const ContextStyle& context,
                               FormulaCursor* cursor, bool smallCursor )
{
    LuPixelPoint point = widgetPos();
    painter.setPen( QPen( context.getEmptyColor(),
                          context.layoutUnitToPixelX( context.getLineWidth()/2 ) ) );
    luPixel unitX = context.ptToLayoutUnitPixX( 1 );
    luPixel unitY = context.ptToLayoutUnitPixY( 1 );
    painter.drawRect( context.layoutUnitToPixelX( point.x()-unitX ),
                      context.layoutUnitToPixelY( point.y()-unitY ),
                      context.layoutUnitToPixelX( getWidth()+2*unitX ),
                      context.layoutUnitToPixelY( getHeight()+2*unitY ) );

    inherited::drawCursor( painter, context, cursor, smallCursor );
}

void NameSequence::moveWordLeft( FormulaCursor* cursor )
{
    uint pos = cursor->getPos();
    if ( pos > 0 ) {
        cursor->setTo( this, 0 );
    }
    else {
        moveLeft( cursor, this );
    }
}

void NameSequence::moveWordRight( FormulaCursor* cursor )
{
    int pos = cursor->getPos();
    if ( pos < countChildren() ) {
        cursor->setTo( this, countChildren() );
    }
    else {
        moveRight( cursor, this );
    }
}


KCommand* NameSequence::compactExpressionCmd( Container* container )
{
    BasicElement* element = replaceElement( container->document()->getSymbolTable() );
    if ( element != 0 ) {
        getParent()->selectChild( container->activeCursor(), this );

        KFCReplace* command = new KFCReplace( i18n( "Add Element" ), container );
        command->addElement( element );
        return command;
    }
    return 0;
}

KCommand* NameSequence::buildCommand( Container* container, Request* request )
{
    switch ( *request ) {
    case req_compactExpression:
        return compactExpressionCmd( container );
    case req_addSpace:
    case req_addIndex:
    case req_addMatrix:
    case req_addOneByTwoMatrix:
    case req_addSymbol:
    case req_addRoot:
    case req_addFraction:
    case req_addBracket:
    case req_addNameSequence:
        return 0;
    default:
        break;
    }
    return inherited::buildCommand( container, request );
}


KCommand* NameSequence::input( Container* container, QChar ch )
{
    int latin1 = ch.latin1();
    switch (latin1) {
    case '(':
    case '[':
    case '|':
    case '^':
    case '_':
    case '}':
    case ']':
    case ')':
    case '\\': {
//         KCommand* compact = compactExpressionCmd( container );
//         KCommand* cmd = static_cast<SequenceElement*>( getParent() )->input( container, ch );
//         if ( compact != 0 ) {
//             KMacroCommand* macro = new KMacroCommand( cmd->name() );
//             macro->addCommand( compact );
//             macro->addCommand( cmd );
//             return macro;
//         }
//         else {
//             return cmd;
//         }
        break;
    }
    case '{':
    case ' ': {
        Request r( req_compactExpression );
        return buildCommand( container, &r );
    }
    default: {
        TextCharRequest r( ch );
        return buildCommand( container, &r );
    }
    }
    return 0;
}

void NameSequence::setElementType( ElementType* t )
{
    inherited::setElementType( t );
    parse();
}

BasicElement* NameSequence::replaceElement( const SymbolTable& table )
{
    QString name = buildName();
    QChar ch = table.unicode( name );
    if ( !ch.isNull() ) return new TextElement( ch, true );

    if ( name == "," )    return new SpaceElement( THIN );
    if ( name == ">" )    return new SpaceElement( MEDIUM );
    if ( name == ";" )    return new SpaceElement( THICK );
    if ( name == "quad" ) return new SpaceElement( QUAD );

    if ( name == "frac" ) return new FractionElement();
    if ( name == "atop" ) {
        FractionElement* frac = new FractionElement();
        frac->showLine( false );
        return frac;
    }
    if ( name == "sqrt" ) return new RootElement();

    return 0;
}

BasicElement* NameSequence::createElement( QString type )
{
    if      ( type == "TEXT" )         return new TextElement();
    return 0;
}

void NameSequence::parse()
{
    // A name sequence is known as name and so are its children.
    // Caution: this is fake!
    for ( int i = 0; i < countChildren(); i++ ) {
        getChild( i )->setElementType( getElementType() );
    }
}

QString NameSequence::buildName()
{
    QString name;
    for ( int i = 0; i < countChildren(); i++ ) {
        name += getChild( i )->getCharacter();
    }
    return name;
}

bool NameSequence::isValidSelection( FormulaCursor* cursor )
{
    SequenceElement* sequence = cursor->normal();
    if ( sequence == 0 ) {
        return false;
    }
    return sequence->onlyTextSelected( cursor );
}

KFORMULA_NAMESPACE_END
