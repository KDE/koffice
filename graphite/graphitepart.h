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

#ifndef GRAPHITE_PART_H
#define GRAPHITE_PART_H

#include <koDocument.h>

class QPainter;
class QMouseEvent;
class QKeyEvent;
class KAction;

class GraphiteView;
class GCommandHistory;

class GraphitePart : public KoDocument {

    Q_OBJECT

public:
    GraphitePart(QObject *parent=0, const char *name=0, bool singleViewMode=false);
    virtual ~GraphitePart();

    virtual KoView *createView(QWidget *parent=0, const char *name=0);
    virtual KoMainWindow *createShell();

    virtual void paintContent(QPainter &painter, const QRect &rect, bool transparent=false);

    virtual bool initDoc();

    virtual QCString mimeType() const;

    // The canvas forwards the Events to us. We test if any
    // object has been hit (z-order!) and handle the event.
    // (Normally we create a Manipulator (M9r) class for the
    // object which has been hit. This class is the first
    // one which gets the Events before testing the whole tree.
    void mouseMoveEvent(QMouseEvent *e, GraphiteView *view);
    void mousePressEvent(QMouseEvent *e, GraphiteView *view);
    void mouseReleaseEvent(QMouseEvent *e, GraphiteView *view);
    void mouseDoubleClickEvent(QMouseEvent *e, GraphiteView *view);

    void keyPressEvent(QKeyEvent *e, GraphiteView *view);
    void keyReleaseEvent(QKeyEvent *e, GraphiteView *view);

protected slots:
    void edit_undo();
    void edit_redo();
    void edit_cut();

private:
    GraphitePart &operator=(const GraphitePart &rhs);

    GCommandHistory *history;
    // TODO: Do we need isLoading() like in KSpread?
    // TODO: If an object is selected, store a pointer
    //       to it here and draw the handles after
    //       drawing the whole tree. This object is
    //       the first one which is hit-tested...
};
#endif
