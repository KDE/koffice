/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2002-2006 David Faure <faure@kde.org>
   Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>

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

#ifndef KWCANVAS_H
#define KWCANVAS_H

#include "KWDocument.h"

#include <KoCanvasBase.h>

#include <QWidget>

class KWGui;
class KWView;
class KWViewMode;
class KoToolProxy;

/**
 * Class: KWCanvas
 * This class is responsible for the rendering of the frames to
 * the screen as well as the interaction with the user via mouse
 * and keyboard. There is one per view.
 */
class KWCanvas : public QWidget, public KoCanvasBase {
    Q_OBJECT

public:
    /**
     * Constructor
     * Creates a new canvas widget that can display pages and frames.
     * @param viewMode the initial KWViewMode this canvas should use
     * @param document as this is one view in the MVC design; the document holds all content
     * @param view the parent KWView object
     * @param parent the parent widget.
     */
    KWCanvas(const QString& viewMode, KWDocument *document, KWView *view, KWGui *parent );
    virtual ~KWCanvas();

    /// ask the widget to set the size this canvas takes to display all content
    void updateSize();

    // KoCanvasBase interface methods.
    /// reimplemented method from superclass
    void gridSize(double *horizontal, double *vertical) const;
    /// reimplemented method from superclass
    bool snapToGrid() const;
    /// reimplemented method from superclass
    void addCommand(KCommand *command, bool execute = true);
    /// reimplemented method from superclass
    KoShapeManager *shapeManager() const { return m_shapeManager; }
    /// reimplemented method from superclass
    void updateCanvas(const QRectF& rc);
    /// reimplemented method from superclass
    KoViewConverter *viewConverter();
    /// reimplemented method from superclass
    QWidget* canvasWidget() { return this; }
    /// reimplemented method from superclass
    KoUnit::Unit unit() { return document()->unit(); }
    /// reimplemented method from superclass
    KoToolProxy * toolProxy() { return m_toolProxy; }
    // getters
    /// return the document that this canvas works on
    KWDocument *document() const { return m_document; }

    /// return the viewMode currently associated with this canvas
    KWViewMode *viewMode() const { return m_viewMode; }

protected:
    /// reimplemented method from superclass
    void keyPressEvent( QKeyEvent *e );
    /// reimplemented method from superclass
    void mouseMoveEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    void mousePressEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    void mouseReleaseEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    void mouseDoubleClickEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    void keyReleaseEvent (QKeyEvent *e);
    /// reimplemented method from superclass
    void paintEvent(QPaintEvent * ev);
    /// reimplemented method from superclass
    void tabletEvent( QTabletEvent *e );
    /// reimplemented method from superclass
    void wheelEvent( QWheelEvent *e );


private slots:
    /// Called whenever there was a page added/removed or simply resized.
    void pageSetupChanged();

private:
    KWDocument *m_document;
    KoShapeManager *m_shapeManager;
    KoToolProxy * m_toolProxy;
    KWView *m_view;
    KWViewMode *m_viewMode;
};

#endif
