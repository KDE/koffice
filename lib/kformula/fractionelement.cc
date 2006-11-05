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

#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>

#include "elementvisitor.h"
#include "formulaelement.h"
#include "formulacursor.h"
#include "fractionelement.h"
#include "sequenceelement.h"

KFORMULA_NAMESPACE_BEGIN
using namespace std;

FractionElement::FractionElement(BasicElement* parent) : BasicElement(parent),
                                                         m_lineThicknessType( NoSize ),
                                                         m_numAlign( NoHorizontalAlign ),
                                                         m_denomAlign( NoHorizontalAlign ),
                                                         m_customBevelled( false )
{
    numerator = new SequenceElement(this);
    denominator = new SequenceElement(this);
}

FractionElement::~FractionElement()
{
    delete denominator;
    delete numerator;
}

FractionElement::FractionElement( const FractionElement& other )
    : BasicElement( other ),
      m_lineThicknessType( other.m_lineThicknessType ),
      m_lineThickness( other.m_lineThickness ),
      m_numAlign( other.m_numAlign ),
      m_denomAlign( other.m_denomAlign ),
      m_customBevelled( other.m_customBevelled ),
      m_bevelled( other.m_bevelled )
{
    numerator = new SequenceElement( *( other.numerator ) );
    denominator = new SequenceElement( *( other.denominator ) );
    numerator->setParent( this );
    denominator->setParent( this );
}


bool FractionElement::accept( ElementVisitor* visitor )
{
    return visitor->visit( this );
}

void FractionElement::entered( SequenceElement* child )
{
    if ( child == numerator ) {
        formula()->tell( i18n( "Numerator" ) );
    }
    else {
        formula()->tell( i18n( "Denominator" ) );
    }
}


BasicElement* FractionElement::goToPos( FormulaCursor* cursor, bool& handled,
                                        const LuPixelPoint& point, const LuPixelPoint& parentOrigin )
{
    BasicElement* e = BasicElement::goToPos(cursor, handled, point, parentOrigin);
    if (e != 0) {
        LuPixelPoint myPos(parentOrigin.x() + getX(),
                           parentOrigin.y() + getY());
        e = numerator->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }
        e = denominator->goToPos(cursor, handled, point, myPos);
        if (e != 0) {
            return e;
        }

        luPixel dx = point.x() - myPos.x();
        luPixel dy = point.y() - myPos.y();

        // the positions after the numerator / denominator
        if ((dx > numerator->getX()) &&
            (dy < numerator->getHeight())) {
            numerator->moveLeft(cursor, this);
            handled = true;
            return numerator;
        }
        else if ((dx > denominator->getX()) &&
                 (dy > denominator->getY())) {
            denominator->moveLeft(cursor, this);
            handled = true;
            return denominator;
        }

        return this;
    }
    return 0;
}


/**
 * Calculates our width and height and
 * our children's parentPosition.
 */
void FractionElement::calcSizes( const ContextStyle& context, 
                                 ContextStyle::TextStyle tstyle,
                                 ContextStyle::IndexStyle istyle,
                                 StyleAttributes& style )
{
    ContextStyle::TextStyle i_tstyle = context.convertTextStyleFraction( tstyle );
    ContextStyle::IndexStyle u_istyle = context.convertIndexStyleUpper( istyle );
    ContextStyle::IndexStyle l_istyle = context.convertIndexStyleLower( istyle );
    double factor = style.sizeFactor();

    numerator->calcSizes( context, i_tstyle, u_istyle, style );
    denominator->calcSizes( context, i_tstyle, l_istyle, style );

    luPixel distY = context.ptToPixelY( context.getThinSpace( tstyle, factor ) );

    double linethickness = lineThickness( context, factor );

    setWidth( QMAX( numerator->getWidth(), denominator->getWidth() ) );
    setHeight( numerator->getHeight() + denominator->getHeight() +
               2*distY + linethickness );
    setBaseline( qRound( numerator->getHeight() + distY + .5*linethickness 
                         + context.axisHeight( tstyle, factor ) ) );

    numerator->setX( ( getWidth() - numerator->getWidth() ) / 2 );
    denominator->setX( ( getWidth() - denominator->getWidth() ) / 2 );

    numerator->setY( 0 );
    denominator->setY( getHeight() - denominator->getHeight() );
}


/**
 * Draws the whole element including its children.
 * The `parentOrigin' is the point this element's parent starts.
 * We can use our parentPosition to get our own origin then.
 */
void FractionElement::draw( QPainter& painter, const LuPixelRect& r,
                            const ContextStyle& context,
                            ContextStyle::TextStyle tstyle,
                            ContextStyle::IndexStyle istyle,
                            StyleAttributes& style,
                            const LuPixelPoint& parentOrigin )
{
    LuPixelPoint myPos( parentOrigin.x()+getX(), parentOrigin.y()+getY() );
    //if ( !LuPixelRect( myPos.x(), myPos.y(), getWidth(), getHeight() ).intersects( r ) )
    //    return;

    numerator->draw( painter, r, context,
                     context.convertTextStyleFraction( tstyle ),
                     context.convertIndexStyleUpper( istyle ), style, myPos);
    if (denominator) { // Can be temporarily 0 see FractionElement::remove
        denominator->draw( painter, r, context,
                           context.convertTextStyleFraction( tstyle ),
                           context.convertIndexStyleLower( istyle ), style,
                           myPos);
    }

    if ( withLine() ) {
        // TODO: thickness
        double factor = style.sizeFactor();
        double linethickness = lineThickness( context, factor );
        painter.setPen( QPen( style.color(),
                              context.layoutUnitToPixelY( linethickness ) ) );
        painter.drawLine( context.layoutUnitToPixelX( myPos.x() ),
                          context.layoutUnitToPixelY( myPos.y() + axis( context, tstyle, factor ) ),
                          context.layoutUnitToPixelX( myPos.x() + getWidth() ),
                          context.layoutUnitToPixelY( myPos.y() + axis( context, tstyle, factor ) ) );
    }
}


void FractionElement::dispatchFontCommand( FontCommand* cmd )
{
    numerator->dispatchFontCommand( cmd );
    denominator->dispatchFontCommand( cmd );
}

/**
 * Enters this element while moving to the left starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the left of it.
 */
void FractionElement::moveLeft(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveLeft(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            if (linear) {
                denominator->moveLeft(cursor, this);
            }
            else {
                numerator->moveLeft(cursor, this);
            }
        }
        else if (from == denominator) {
            numerator->moveLeft(cursor, this);
        }
        else {
            getParent()->moveLeft(cursor, this);
        }
    }
}


/**
 * Enters this element while moving to the right starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or to the right of it.
 */
void FractionElement::moveRight(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveRight(cursor, this);
    }
    else {
        bool linear = cursor->getLinearMovement();
        if (from == getParent()) {
            numerator->moveRight(cursor, this);
        }
        else if (from == numerator) {
            if (linear) {
                denominator->moveRight(cursor, this);
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


/**
 * Enters this element while moving up starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or above it.
 */
void FractionElement::moveUp(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveUp(cursor, this);
    }
    else {
        if (from == getParent()) {
            denominator->moveRight(cursor, this);
        }
        else if (from == denominator) {
            numerator->moveRight(cursor, this);
        }
        else {
            getParent()->moveUp(cursor, this);
        }
    }
}


/**
 * Enters this element while moving down starting inside
 * the element `from'. Searches for a cursor position inside
 * this element or below it.
 */
void FractionElement::moveDown(FormulaCursor* cursor, BasicElement* from)
{
    if (cursor->isSelectionMode()) {
        getParent()->moveDown(cursor, this);
    }
    else {
        if (from == getParent()) {
            numerator->moveRight(cursor, this);
        }
        else if (from == numerator) {
            denominator->moveRight(cursor, this);
        }
        else {
            getParent()->moveDown(cursor, this);
        }
    }
}


/**
 * Reinserts the denominator if it has been removed.
 */
void FractionElement::insert(FormulaCursor* cursor,
                             QPtrList<BasicElement>& newChildren,
                             Direction direction)
{
    if (cursor->getPos() == denominatorPos) {
        denominator = static_cast<SequenceElement*>(newChildren.take(0));
        denominator->setParent(this);

        if (direction == beforeCursor) {
            denominator->moveLeft(cursor, this);
        }
        else {
            denominator->moveRight(cursor, this);
        }
        cursor->setSelection(false);
        formula()->changed();
    }
}


/**
 * Removes all selected children and returns them. Places the
 * cursor to where the children have been.
 *
 * We remove ourselve if we are requested to remove our numerator.
 *
 * It is possible to remove the denominator. But after this we
 * are senseless and the caller is required to replace us.
 */
void FractionElement::remove(FormulaCursor* cursor,
                             QPtrList<BasicElement>& removedChildren,
                             Direction direction)
{
    switch (cursor->getPos()) {
    case numeratorPos:
        getParent()->selectChild(cursor, this);
        getParent()->remove(cursor, removedChildren, direction);
        break;
    case denominatorPos:
        removedChildren.append(denominator);
        formula()->elementRemoval(denominator);
        denominator = 0;
        cursor->setTo(this, denominatorPos);
        formula()->changed();
        break;
    }
}


/**
 * Returns wether the element has no more useful
 * children (except its main child) and should therefore
 * be replaced by its main child's content.
 */
bool FractionElement::isSenseless()
{
    return denominator == 0;
}


// main child
//
// If an element has children one has to become the main one.

SequenceElement* FractionElement::getMainChild()
{
    return numerator;
}

// void FractionElement::setMainChild(SequenceElement* child)
// {
//     formula()->elementRemoval(numerator);
//     numerator = child;
//     numerator->setParent(this);
//     formula()->changed();
// }


/**
 * Sets the cursor to select the child. The mark is placed before,
 * the position behind it.
 */
void FractionElement::selectChild(FormulaCursor* cursor, BasicElement* child)
{
    if (child == numerator) {
        cursor->setTo(this, numeratorPos);
    }
    else if (child == denominator) {
        cursor->setTo(this, denominatorPos);
    }
}


/**
 * Appends our attributes to the dom element.
 */
void FractionElement::writeDom(QDomElement element)
{
    BasicElement::writeDom(element);

    QDomDocument doc = element.ownerDocument();
    if (!withLine()) element.setAttribute("NOLINE", 1);

    QDomElement num = doc.createElement("NUMERATOR");
    num.appendChild(numerator->getElementDom(doc));
    element.appendChild(num);

    QDomElement den = doc.createElement("DENOMINATOR");
    den.appendChild(denominator->getElementDom(doc));
    element.appendChild(den);
}

/**
 * Reads our attributes from the element.
 * Returns false if it failed.
 */
bool FractionElement::readAttributesFromDom(QDomElement element)
{
    if (!BasicElement::readAttributesFromDom(element)) {
        return false;
    }
    QString lineStr = element.attribute("NOLINE");
    if(!lineStr.isNull()) {
        m_lineThicknessType = RelativeSize;
        m_lineThickness = lineStr.toInt();
    }
    return true;
}

/**
 * Reads our attributes from the MathML element.
 * Returns false if it failed.
 */
bool FractionElement::readAttributesFromMathMLDom(const QDomElement& element)
{
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }
    QString linethicknessStr = element.attribute( "linethickness" ).lower();
    if ( ! linethicknessStr.isNull() ) {
        if ( linethicknessStr == "thin" ) {
            m_lineThicknessType = RelativeSize;
            m_lineThickness = 0.5; // ### Arbitrary size
        }
        else if ( linethicknessStr == "medium" ) {
            m_lineThicknessType = RelativeSize;
            m_lineThickness = 1.0;
        }
        else if ( linethicknessStr == "thick" ) {
            m_lineThicknessType = RelativeSize;
            m_lineThickness = 2.0; // ### Arbitrary size
        }
        else {
            m_lineThickness = getSize( linethicknessStr, &m_lineThicknessType );
        }
    }
    QString numalignStr = element.attribute( "numalign" ).lower();
    if ( ! numalignStr.isNull() ) {
        if ( numalignStr == "left" ) {
            m_numAlign = LeftHorizontalAlign;
        }
        else if ( numalignStr == "center" ) {
            m_numAlign = CenterHorizontalAlign;
        }
        else if ( numalignStr == "right" ) {
            m_numAlign = RightHorizontalAlign;
        }
    }
    QString denomalignStr = element.attribute( "denomalign" ).lower();
    if ( ! denomalignStr.isNull() ) {
        if ( denomalignStr == "left" ) {
            m_denomAlign = LeftHorizontalAlign;
        }
        else if ( denomalignStr == "center" ) {
            m_denomAlign = CenterHorizontalAlign;
        }
        else if ( denomalignStr == "right" ) {
            m_denomAlign = RightHorizontalAlign;
        }
    }
    QString bevelledStr = element.attribute( "bevelled" ).lower();
    if ( ! bevelledStr.isNull() ) {
        m_customBevelled = true;
        if ( bevelledStr == "true" ) {
            m_bevelled = true;
        }
        else {
            m_bevelled = false;
        }
    }

    return true;
}

/**
 * Reads our content from the node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
bool FractionElement::readContentFromDom(QDomNode& node)
{
    if (!BasicElement::readContentFromDom(node)) {
        return false;
    }

    if ( !buildChild( numerator, node, "NUMERATOR" ) ) {
        kdWarning( DEBUGID ) << "Empty numerator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    if ( !buildChild( denominator, node, "DENOMINATOR" ) ) {
        kdWarning( DEBUGID ) << "Empty denominator in FractionElement." << endl;
        return false;
    }
    node = node.nextSibling();

    return true;
}

/**
 * Reads our content from the MathML node. Sets the node to the next node
 * that needs to be read.
 * Returns false if it failed.
 */
int FractionElement::readContentFromMathMLDom(QDomNode& node)
{
    if ( BasicElement::readContentFromMathMLDom( node ) == -1 ) {
        return -1;
    }

    int numeratorNumber = numerator->buildMathMLChild( node );
    if ( numeratorNumber == -1 ) {
        kdWarning( DEBUGID ) << "Empty numerator in FractionElement." << endl;
        return -1;
    }
    for (int i = 0; i < numeratorNumber; i++ ) {
        if ( node.isNull() ) {
            return -1;
        }
        node = node.nextSibling();
    }

    if ( denominator->buildMathMLChild( node ) == -1 ) {
        kdWarning( DEBUGID ) << "Empty denominator in FractionElement." << endl;
        return -1;
    }

    return 1;
}

QString FractionElement::toLatex()
{
    if ( withLine() ) {
        return "\\frac{" + numerator->toLatex() +"}{" + denominator->toLatex() + "}";
    }
    else {
        return "{" + numerator->toLatex() + "\\atop " + denominator->toLatex() + "}";
    }
}

QString FractionElement::formulaString()
{
    return "(" + numerator->formulaString() + ")/(" + denominator->formulaString() + ")";
}

void FractionElement::writeMathMLAttributes( QDomElement& element ) const
{
    switch ( m_lineThicknessType ) {
    case AbsoluteSize:
        element.setAttribute( "linethickness", QString( "%1pt" ).arg( m_lineThickness ) );
        break;
    case RelativeSize:
        element.setAttribute( "linethickness", QString( "%1%" ).arg( m_lineThickness * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( "linethickness", QString( "%1px" ).arg( m_lineThickness ) );
        break;
    default:
        break;
    }

    switch ( m_numAlign ) {
    case LeftHorizontalAlign:
        element.setAttribute( "numalign", "left" );
        break;
    case CenterHorizontalAlign:
        element.setAttribute( "numalign", "center" );
        break;
    case RightHorizontalAlign:
        element.setAttribute( "numalign", "right" );
        break;
    default:
        break;
    }

    switch ( m_denomAlign ) {
    case LeftHorizontalAlign:
        element.setAttribute( "denomalign", "left" );
        break;
    case CenterHorizontalAlign:
        element.setAttribute( "denomalign", "center" );
        break;
    case RightHorizontalAlign:
        element.setAttribute( "denomalign", "right" );
        break;
    default:
        break;
    }

    if ( m_customBevelled ) {
        element.setAttribute( "bevelled", m_bevelled ? "true" : "false" );
    }
}

void FractionElement::writeMathMLContent( QDomDocument& doc, 
                                          QDomElement& element,
                                          bool oasisFormat ) const
{
    numerator->writeMathML( doc, element, oasisFormat );
    denominator->writeMathML( doc, element, oasisFormat );
}

double FractionElement::lineThickness( const ContextStyle& context, double factor )
{
    double linethickness = context.getLineWidth( factor );
    switch ( m_lineThicknessType ) {
    case AbsoluteSize:
        linethickness = context.ptToLayoutUnitPixX( m_lineThickness );
        break;
    case RelativeSize:
        linethickness *= m_lineThickness;
        break;
    case PixelSize:
        linethickness = m_lineThickness;
        break;
    default:
        break;
    }
    return linethickness;
}
        

KFORMULA_NAMESPACE_END
