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

#ifndef __KFORMULAWIDGET_H
#define __KFORMULAWIDGET_H

#include <qdom.h>
#include <qpoint.h>
#include <qwidget.h>

//#include <kaccel.h>
//#include <kaction.h>

#include "formuladefs.h"


class BasicElement;
class FormulaCursor;
class FormulaElement;
class KFormulaContainer;


/**
 * The view. A widget that shows the formula. There are methods
 * to move the cursor around. To edit the formula use the document.
 */
class KFormulaWidget : public QWidget {
    Q_OBJECT

public:
    KFormulaWidget(KFormulaContainer*, QWidget* parent=0, const char* name=0, WFlags f=0);
    ~KFormulaWidget();

    /**
     * Sets the char that is to be used as default
     * left bracket.
     */
    void setLeftBracket(char left) { leftBracket = left; }

    /**
     * Sets the char that is to be used as default
     * right bracket.
     */
    void setRightBracket(char right) { rightBracket = right; }

    /**
     * @returns the point inside the formula widget where the cursor is.
     */
    QPoint getCursorPoint() const;

    /**
     * Puts the widget in read only mode.
     */
    void setReadOnly(bool ro) { readOnly = ro; }

    /**
     * Gets called from the cursor just before it changes.
     */
    void tellCursorChanged(FormulaCursor* c);
    
signals:

    /**
     * Is emitted everytime the cursor might have changed.
     */
    void cursorChanged(bool visible, bool selecting);
    
public slots:
    
    void slotSelectAll();

    void slotMoveLeft(MoveFlag flag);
    void slotMoveRight(MoveFlag flag);
    void slotMoveUp(MoveFlag flag);
    void slotMoveDown(MoveFlag flag);
    void slotMoveHome(MoveFlag flag);
    void slotMoveEnd(MoveFlag flag);

protected slots:

    void slotFormulaChanged(int width, int height);
    void slotFormulaLoaded(FormulaElement*);
    void slotElementWillVanish(BasicElement*);

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

    virtual void paintEvent(QPaintEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);

    MoveFlag movementFlag(int state);

    void hideCursor();
    void showCursor();

    KFormulaContainer* getDocument() { return document; }

    FormulaCursor* getCursor() { return cursor; }
    
private:

    void emitCursorChanged();

    /**
     * Whether you can see the cursor.
     */
    bool cursorVisible;

    /**
     * Whether the cursor changed since the last time
     * we emitted a cursorChanged signal.
     */
    bool cursorHasChanged;

    /**
     * Whether we are only allowed to read.
     */
    bool readOnly;

    char leftBracket;
    char rightBracket;
    
    KFormulaContainer* document;
    FormulaCursor* cursor;
};

#endif // __KFORMULAWIDGET_H
