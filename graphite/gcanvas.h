/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <wtrobin@mandrakesoft.com>

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

// The main widget (scroll area)

#ifndef gcanvas_h
#define gcanvas_h

#include <qscrollview.h>
#include <graphitepart.h>

class GraphiteView;


class GCanvas : public QScrollView {

    Q_OBJECT
public:
    GCanvas(GraphiteView *view, GraphitePart *doc);
    virtual ~GCanvas() {}

protected:
    virtual void viewportPaintEvent(QPaintEvent *e);
    // TODO: Do we have to override the resizeEvent?
    // virtual void viewportResizeEvent(QResizeEvent *e);
    virtual void contentsMousePressEvent(QMouseEvent *e) { m_doc->mousePressEvent(e, m_view); }
    virtual void contentsMouseReleaseEvent(QMouseEvent *e) { m_doc->mouseReleaseEvent(e, m_view); }
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *e) { m_doc->mouseDoubleClickEvent(e, m_view); }
    virtual void contentsMouseMoveEvent(QMouseEvent *e) { m_doc->mouseMoveEvent(e, m_view); }

    virtual void keyPressEvent(QKeyEvent *e) { m_doc->keyPressEvent(e, m_view); }
    virtual void keyReleaseEvent(QKeyEvent *e) { m_doc->keyReleaseEvent(e, m_view); }

    // Here we should tell the document not to draw the handles
    // when we don't have focus... (TODO)
    virtual void focusInEvent(QFocusEvent */*e*/) {}
    virtual void focusOutEvent(QFocusEvent */*e*/) {}
private:
    GraphiteView *m_view;
    GraphitePart *m_doc;
};
#endif // gcanvas_h
