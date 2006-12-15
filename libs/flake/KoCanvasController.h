/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOCANVASVIEW_H
#define KOCANVASVIEW_H

#include <koffice_export.h>

#include "KoCanvasBase.h"

#include <QWidget>
#include <QScrollArea>

class QGridLayout;
class QPaintEvent;
class QEvent;
class KoShape;
class KoToolDocker;

/**
 * This widget is a wrapper around your canvas providing scrollbars.
 * Flake does not provide a canvas, the application will have to extend a QWidget
 * and implement that themselves; but Flake does make it a lot easier to do so.
 * One of those things is this widget that acts as a decorator around the canvas
 * widget and provides scrollbars and allows the canvas to be centered in the viewArea
 * <p>The using application can intantiate this class and add its canvas using the
 * setCanvas() call. Which is designed so it can be called multiple times for those
 * that wish to exchange one canvas widget for another.
 *
 * Effectively, there is _one_ KoCanvasController per KoView in your application.
 *
 */
class FLAKE_EXPORT KoCanvasController : public QScrollArea {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent the parent this widget will belong to
     */
    explicit KoCanvasController(QWidget *parent);
    virtual ~KoCanvasController() {}

    /**
     * Set the new canvas to be shown as a child
     * Calling this will emit canvasRemoved() if there was a canvas before, and will emit
     * canvasSet() with the new canvas.
     * @param canvas the new canvas. The KoCanvasBase::canvas() will be called to retrieve the
     *        actual widget which will then be added as child of this one.
     */
    void setCanvas(KoCanvasBase *canvas);
    /**
     * Return the curently set canvas
     * @return the curently set canvas
     */
    KoCanvasBase* canvas() const;

    /**
     * return the amount of pixels vertically visible of the child canvas.
     * @return the amount of pixels vertically visible of the child canvas.
     */
    int visibleHeight() const;
    /**
     * return the amount of pixels horizontally visible of the child canvas.
     * @return the amount of pixels horizontally visible of the child canvas.
     */
    int visibleWidth() const;
    /**
     * return the amount of pixels that are not visible on the left side of the canvas.
     * The leftmost pixel that is shown is returned.
     */
    int canvasOffsetX() const;
    /**
     * return the amount of pixels that are not visible on the top side of the canvas.
     * The topmost pixel that is shown is returned.
     */
    int canvasOffsetY() const;

    /**
     * Set the canvas to be displayed centered in this widget.
     * In the case that the canvas widget is smaller then this one the canvas will be centered
     * and a contrasting color used for the background.
     * @param centered center canvas if true, or aligned to the left (LTR) if false.
     *        Centered is the default value.
     */
    void centerCanvas(bool centered);
    /**
     * return the canvas centering value.
     * @return the canvas centering value
     */
    bool isCanvasCentered() const;

    /// Reimplemented from QObject
    virtual bool eventFilter(QObject* watched, QEvent* event);

    /**
     * @brief Scrolls the content of the canvas so that the given rect is visible.
     *
     * The rect is to be specified in document coordinates. The scrollbar positions
     * are changed so that the centerpoint of the rectangle is centered if possible.
     *
     * @param rect the rectangle to make visible
     */
    void ensureVisible( const QRectF &rect );

    /**
     * @brief Scrolls the content of the canvas so that the given shape is visible.
     *
     * This is just a wrapper function of the above function.
     *
     * @param shape the shape to make visible
     */
    void ensureVisible( KoShape *shape );


    /**
     * Return a pointer to the dock widget that will contain the
     * options widget associated with the current tool for this
     * canvas. May be 0.
     */
    KoToolDocker * toolOptionDocker()
        {
            return m_toolOptionDocker;
        }

    /**
     * Set the tool option dock widget.
     */
    void setToolOptionDocker( KoToolDocker * toolOptionDocker )
        {
            m_toolOptionDocker = toolOptionDocker;
        }

signals:
    /**
     * Emitted when a previously added canvas is about to be removed.
     * @param cv this object
     */
    void canvasRemoved(KoCanvasController* cv);
    /**
     * Emitted when a canvas is set on this widget
     * @param cv this object
     */
    void canvasSet(KoCanvasController* cv);

    /**
     * Emitted when canvasOffsetX() changes
     * @param offset the new canvas offset
     */
    void canvasOffsetXChanged(int offset);

    /**
     * Emitted when canvasOffsetY() changes
     * @param offset the new canvas offset
     */
    void canvasOffsetYChanged(int offset);

    /**
     * Emitted when the cursor is moved over the canvas widget
     * @param pos the position in widget pixels.
     */
    void canvasMousePositionChanged(const QPoint & pos );

protected slots:

    /// Called by the horizontal scrollbar when it's value changes
    void updateCanvasOffsetX();

    /// Called by the vertical scrollbar when it's value changes
    void updateCanvasOffsetY();

private:
    class Viewport : public QWidget {
      public:
        Viewport(KoCanvasController *parent);
        ~Viewport() {};
        void setCanvas(QWidget *canvas);
        void removeCanvas(QWidget *canvas);
        void centerCanvas(bool centered);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void dragMoveEvent (QDragMoveEvent *event);
        void dragLeaveEvent(QDragLeaveEvent *event);
        void paintEvent(QPaintEvent *event);

      private:
        QPointF corrrectPosition(const QPoint &point) const;
        void repaint(KoShape *shape);

      private:
        QGridLayout *m_layout;
        KoCanvasController *m_parent;
        KoShape *m_draggedShape;
    };

private:
    KoCanvasBase *m_canvas;
    QWidget *m_canvasWidget;
    Viewport *m_viewport;
    bool m_centerCanvas;
    KoToolDocker * m_toolOptionDocker;


};

#endif
