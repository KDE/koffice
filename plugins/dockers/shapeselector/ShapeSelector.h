/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SHAPESELECTOR_H
#define SHAPESELECTOR_H

#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoShapeControllerBase.h>

#include <QDockWidget>
#include <QRectF>

class KoTool;
class KoShape;
class KoViewConverter;
class QUndoCommand;
class KoCanvasController;
class KoShapeManager;
class QKeyEvent;
class QPainter;

/**
 * The shape selector shows a widget that holds templates and clipboard data
 * for the user to easilly move that between apps and maintain functionality.
 */
class ShapeSelector : public QDockWidget {
    Q_OBJECT
public:
    explicit ShapeSelector(QWidget *parent = 0);
    ~ShapeSelector();

private slots:
    void loadShapeTypes();

private:
    void itemSelected();
    void add(KoShape *item);

private:
    /// \internal
    class DummyViewConverter : public KoViewConverter {
        QPointF documentToView (const QPointF &documentPoint) const;
        QPointF viewToDocument (const QPointF &viewPoint) const;
        QRectF documentToView (const QRectF &documentRect) const;
        QRectF viewToDocument (const QRectF &viewRect) const;
        QSizeF documentToView (const QSizeF &documentSize) const;
        QSizeF viewToDocument (const QSizeF &viewSize) const;
        void zoom (double *zoomX, double *zoomY) const;
        double documentToViewX (double documentX) const;
        double documentToViewY (double documentY) const;
        double viewToDocumentX (double viewX) const;
        double viewToDocumentY (double viewY) const;
    };

    class DummyShapeController : public KoShapeControllerBase {
    public:
        void addShape( KoShape* ) {}
        void removeShape( KoShape* ) {}
    };

    /// \internal
    class Canvas : public QWidget, public KoCanvasBase {
        public:
            explicit Canvas(ShapeSelector *parent);
            void gridSize (double *horizontal, double *vertical) const;
            bool snapToGrid() const { return false; }
            void addCommand (QUndoCommand *command);
            KoShapeManager * shapeManager () const { return m_parent->m_shapeManager; }
            void updateCanvas (const QRectF &rc);
            KoToolProxy *toolProxy () const { return 0; }
            const KoViewConverter * viewConverter() const { return &m_converter; }
            QWidget *canvasWidget () { return m_parent; }
            KoUnit unit() const { return KoUnit(KoUnit::Millimeter); }
            void updateInputMethodInfo() {}

        protected: // event handlers
            void mouseMoveEvent(QMouseEvent *e);
            void mousePressEvent(QMouseEvent *e);
            void mouseReleaseEvent(QMouseEvent *e);
            void tabletEvent(QTabletEvent *e);
            void paintEvent(QPaintEvent * e);
            void dragEnterEvent(QDragEnterEvent *e);
            void dropEvent(QDropEvent *e);
            bool event(QEvent *e);

        private:
            DummyShapeController m_shapeController;
            DummyViewConverter m_converter;
            ShapeSelector *m_parent;
            bool m_emitItemSelected;
    };

    friend class Canvas;

    KoShapeManager *m_shapeManager;
    Canvas *m_canvas;
};

#endif
