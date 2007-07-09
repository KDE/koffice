/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "KoFormulaTool.h"
#include "KoFormulaShape.h"
#include "FormulaCursor.h"
#include "FormulaToolOptions.h"
#include "BasicElement.h"
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KFileDialog>
#include <kiconloader.h>
#include <klocale.h>
#include <QKeyEvent>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include "KoFormulaTool.moc"

KoFormulaTool::KoFormulaTool( KoCanvasBase* canvas ) : KoTool( canvas ),
                                                       m_formulaShape( 0 ),
                                                       m_formulaCursor( 0 )
{
}

KoFormulaTool::~KoFormulaTool()
{
    if( m_formulaCursor )
        delete m_formulaCursor;
}

void KoFormulaTool::activate( bool temporary )
{
    Q_UNUSED(temporary);
    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach( KoShape* shape, selection->selectedShapes() )
    {
        m_formulaShape = dynamic_cast<KoFormulaShape*>( shape );
        if( m_formulaShape )
            break;
    }
    if( m_formulaShape == 0 )  // none found
    {
        emit done();
        return;
    }

    m_formulaCursor = new FormulaCursor( m_formulaShape->formulaElement() );
}

void KoFormulaTool::deactivate()
{
    m_formulaShape = 0;
    delete m_formulaCursor;
    m_formulaCursor = 0;
}

void KoFormulaTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED( converter )
    // TODO do view conversions with converter
    m_formulaCursor->paint( painter );
}

void KoFormulaTool::mousePressEvent( KoPointerEvent *event )
{
    Q_UNUSED( event )

// TODO implement the action and the elementAt method in FormulaShape
//   m_formulaCursor->setCursorTo( m_formulaShape->elementAt( ) );
//
//
//   from the old FormulaCursor implementation
/*
    FormulaElement* formula = getElement()->formula();
    formula->goToPos( this, pos );

    setCursorToElement( m_container->childElementAt( pos ) );
    if (flag & SelectMovement) {
        setSelection(true);
        if (getMark() == -1) {
            setMark(getPos());
        }
    }
    else {
        setSelection(false);
        setMark(getPos());
    }
*/
}

void KoFormulaTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    Q_UNUSED( event )

    // TODO select whole element
}

void KoFormulaTool::mouseMoveEvent( KoPointerEvent *event )
{
    Q_UNUSED( event )

    // TODO find the old implementation and use it
    //
    // the old implementation
/*    setSelection(true);
    BasicElement* element = getElement();
    int mark = getMark();

   FormulaElement* formula = getElement()->formula();
    formula->goToPos( this, point );
    BasicElement* newElement = getElement();
    int pos = getPos();

    BasicElement* posChild = 0;
    BasicElement* markChild = 0;
    while (element != newElement) {
        posChild = newElement;
        newElement = newElement->getParent();
        if (newElement == 0) {
            posChild = 0;
            newElement = getElement();
            markChild = element;
            element = element->getParent();
        }
    }

    if (dynamic_cast<SequenceElement*>(element) == 0) {
        element = element->getParent();
        element->selectChild(this, newElement);
    }
    else {
        if (posChild != 0) {
            element->selectChild(this, posChild);
            pos = getPos();
        }
        if (markChild != 0) {
            element->selectChild(this, markChild);
            mark = getMark();
        }
        if (pos == mark) {
            if ((posChild == 0) && (markChild != 0)) {
                mark++;
            }
            else if ((posChild != 0) && (markChild == 0)) {
                mark--;
            }
        }
        else if (pos < mark) {
            if (posChild != 0) {
                pos--;
            }
        }
        setTo(element, pos, mark);
    }*/
}

void KoFormulaTool::mouseReleaseEvent( KoPointerEvent *event )
{
    Q_UNUSED( event )

    // TODO what should happen here ?
}

void KoFormulaTool::keyPressEvent( QKeyEvent *event )
{
    if( !m_formulaCursor )
        return;

    m_formulaCursor->setWordMovement( event->modifiers() & Qt::ControlModifier );
    m_formulaCursor->setSelecting( event->modifiers() & Qt::ShiftModifier );

    switch( event->key() )                           // map key to movement or action
    {
        case Qt::Key_Backspace:
            remove( true );
            break;
        case Qt::Key_Delete:
	    remove( false );
            break;
        case Qt::Key_Left:
	    m_formulaCursor->moveLeft();
            break;
        case Qt::Key_Up:
            m_formulaCursor->moveUp();
            break;
        case Qt::Key_Right:
	    m_formulaCursor->moveRight();
            break;
        case Qt::Key_Down:
	    m_formulaCursor->moveDown();
            break;
        case Qt::Key_End:
	    m_formulaCursor->moveEnd();
            break;
        case Qt::Key_Home:
	    m_formulaCursor->moveHome();
            break;
/*        default:
            if( event->text().length() == 0 )
                return;
            insertText( event->text() );*/
    }

    event->accept();
}

void KoFormulaTool::keyReleaseEvent( QKeyEvent *event )
{
    event->accept();
}

void KoFormulaTool::remove( bool backSpace )
{
    Q_UNUSED( backSpace )

    if( m_formulaCursor->hasSelection() )  // remove the selection
    {
	 // TODO set the cursor according to backSpace
//        m_formulaCursor->setCursorTo( );
    }
    else                                  // remove only the current element
    {
//        m_formulaCursor->currentElement()->parentElement()->removeChild( m_currentElement );
    }
}

QWidget* KoFormulaTool::createOptionWidget()
{
    FormulaToolOptions* options = new FormulaToolOptions();
 //   options->setTool();
    return options;
/*
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout( optionWidget );

    QToolButton *button = 0;

    QLabel * lbl = new QLabel( i18n( "Import formula" ), optionWidget );
    layout->addWidget( lbl, 0, 0 );

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("open") );
    button->setToolTip( i18n( "Open" ) );
    layout->addWidget( button, 0, 1 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( slotChangeUrl() ) );

    return optionWidget; */
}

void KoFormulaTool::slotChangeUrl()
{
    KUrl url = KFileDialog::getOpenUrl();
    if(!url.isEmpty() && m_formulaShape)
        m_formulaShape->importFormula(url);
}


