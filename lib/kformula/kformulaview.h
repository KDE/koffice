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

#ifndef KFORMULAVIEW_H
#define KFORMULAVIEW_H

#include <qevent.h>
#include <qobject.h>
#include <qrect.h>

#include "kformuladefs.h"

class QColorGroup;

KFORMULA_NAMESPACE_BEGIN

class BasicElement;
class FormulaCursor;
class FormulaElement;
class Container;


/**
 * The view that shows the formula. Its main purpose is to handle
 * the cursor. There are methods
 * to move the cursor around. To edit the formula use the document.
 *
 * The view is meant to be easy embeddable into a widget or
 * to be used alone if there is a bigger widget the formula
 * is to be drawn into.
 */
class View : public QObject {
    Q_OBJECT

public:
    View(Container*);
    virtual ~View();

    /**
     * @returns the point inside the formula view where the cursor is.
     */
    QPoint getCursorPoint() const;

    /**
     * @returns the area the cursor is currently on.
     */
    QRect getDirtyArea() const;

    /**
     * Puts the widget in read only mode.
     */
    void setReadOnly(bool ro);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

    // the mouse event happened at a certain point
    void mousePressEvent( QMouseEvent* event, const PtPoint& pos );
    void mouseReleaseEvent( QMouseEvent* event, const PtPoint& pos );
    void mouseDoubleClickEvent( QMouseEvent* event, const PtPoint& pos );
    void mouseMoveEvent( QMouseEvent* event, const PtPoint& pos );
    void wheelEvent( QWheelEvent* event, const PtPoint& pos );

    // the mouse event happened at a certain point
    void mousePressEvent( QMouseEvent* event, const LuPixelPoint& pos );
    void mouseReleaseEvent( QMouseEvent* event, const LuPixelPoint& pos );
    void mouseDoubleClickEvent( QMouseEvent* event, const LuPixelPoint& pos );
    void mouseMoveEvent( QMouseEvent* event, const LuPixelPoint& pos );
    void wheelEvent( QWheelEvent* event, const LuPixelPoint& pos );

    void keyPressEvent(QKeyEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);

    void draw(QPainter& painter, const QRect& rect, const QColorGroup& cg);

    /**
     * The document we show.
     */
    Container* getDocument() const { return container(); }

    /**
     * Our cursor.
     */
    FormulaCursor* getCursor() const { return cursor(); }

    void setSmallCursor(bool small);

    // simple cursor movement.

    void moveLeft( int flag = NormalMovement );
    void moveRight( int flag = NormalMovement );
    void moveUp( int flag = NormalMovement );
    void moveDown( int flag = NormalMovement );

    void moveHome( int flag = NormalMovement );
    void moveEnd( int flag = NormalMovement );

    /** @returns whether the cursor is at the first position. */
    bool isHome() const;

    /** @returns whether the cursor is at the last position. */
    bool isEnd() const;

    void eraseSelection( Direction direction = beforeCursor );
    void addText( QString str );

    /** Gets called if the cursor ties to leave the formula at its begin. */
    virtual void exitLeft();

    /** Gets called if the cursor ties to leave the formula at its end. */
    virtual void exitRight();

signals:

    /**
     * Is emitted everytime the cursor might have changed.
     */
    void cursorChanged(bool visible, bool selecting);

public slots:

    void slotSelectAll();

protected slots:

    /**
     * The cursor has been moved by the container.
     * We need to repaint if it was ours.
     */
    void slotCursorMoved(FormulaCursor* cursor);

    /**
     * A new formula has been loaded.
     */
    void slotFormulaLoaded(FormulaElement*);

    /**
     * There is an element that will disappear from the tree.
     * our cursor must not be inside it.
     */
    void slotElementWillVanish(BasicElement*);

    void slotCursorExitLeft( FormulaCursor* cursor );
    void slotCursorExitRight( FormulaCursor* cursor );

private:

    /**
     * Tell everybody that our cursor has changed if so.
     */
    void emitCursorChanged();

    FormulaCursor* cursor() const;
    bool& cursorHasChanged();
    bool cursorVisible();
    bool& smallCursor();
    Container* container() const;
    const ContextStyle& contextStyle() const;

    struct View_Impl;
    View_Impl* impl;
};

KFORMULA_NAMESPACE_END

#endif // KFORMULAVIEW_H
