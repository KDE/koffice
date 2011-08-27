/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>

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

#ifndef KSHAPEMANAGER_H
#define KSHAPEMANAGER_H

#include <QList>
#include <QObject>
#include <QSet>

#include "KFlake.h"
#include "KShape.h"
#include "flake_export.h"

class KShapeSelection;
class KViewConverter;
class KCanvasBase;
class KPointerEvent;
class KShapeManagerPaintingStrategy;
class KShapeConnection;
class KShapeManagerPrivate;

class QPainter;
class QPointF;
class QRectF;

/**
 * The shape manager hold a list of all shape which are in scope.
 * There is one shape manager per view.
 *
 * The selection in the different views can be different.
 */
class FLAKE_EXPORT KShapeManager : public QObject
{
    Q_OBJECT

public:
    /// enum for add()
    enum Repaint {
        PaintShapeOnAdd,    ///< Causes each shapes 'update()' to be called after being added to the shapeManager
        AddWithoutRepaint   ///< Avoids each shapes 'update()' to be called for faster addition when its possible.
    };

    /**
     * Constructor.
     * @param canvas the canvas this shape manager is working on.
     * @param parent is a QObject based parent for memory management purposes
     */
    explicit KShapeManager(KCanvasBase *canvas, QObject *parent = 0);
    /**
     * Constructor that takes a list of shapes, convenience version.
     * @param shapes the shapes to start out with, see also setShapes()
     * @param canvas the canvas this shape manager is working on.
     * @param parent is a QObject based parent for memory management purposes
     */
    KShapeManager(KCanvasBase *canvas, const QList<KShape *> &shapes, QObject *parent = 0);
    virtual ~KShapeManager();

    /**
     * Remove all previously owned shapes and make the argument list the new shapes
     * to be managed by this manager.
     * @param shapes the new shapes to manage.
     * @param repaint if true it will trigger a repaint of the shapes
     */
    void setShapes(const QList<KShape *> &shapes, Repaint repaint = PaintShapeOnAdd);

    /// returns the list of maintained shapes
    QList<KShape*> shapes() const;

    /**
     * Get a list of all shapes that don't have a parent.
     */
    QList<KShape*> topLevelShapes() const;

    /**
     * Add a KShape to be displayed and managed by this manager.
     * This will trigger a repaint of the shape.
     * @param shape the shape to add
     * @param repaint if true it will trigger a repaint of the shape
     */
    void addShape(KShape *shape, Repaint repaint = PaintShapeOnAdd);

    /**
     * Add an additional shape to the manager.
     *
     * For additional shapes only updates are handled
     */
    void addAdditional(KShape *shape);

    /**
     * Remove a KShape from this manager
     * @param shape the shape to remove
     */
    void remove(KShape *shape);

    /**
     * Remove an additional shape
     *
     * For additional shapes only updates are handled
     */
    void removeAdditional(KShape *shape);

    /// return the selection shapes for this shapeManager
    KShapeSelection *selection() const;

    /**
     * Paint all shapes and their selection handles etc.
     * @param painter the painter to paint to.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     * @param converter to convert between document and view coordinates.
     */
    void paint(QPainter &painter, const KViewConverter &converter, bool forPrint);

    /**
     * Returns the shape located at a specific point in the document.
     * If more than one shape is located at the specific point, the given selection type
     * controls which of them is returned.
     * @param position the position in the document coordinate system.
     * @param selection controls which shape is returned when more than one shape is at the specific point
     * @param omitHiddenShapes if true, only visible shapes are considered
     */
    KShape *shapeAt(const QPointF &position, KFlake::ShapeSelection selection = KFlake::ShapeOnTop, bool omitHiddenShapes = true);

    KShapeConnection *connectionAt(const QPointF &position);

    /**
     * Returns the shapes which intersects the specific rect in the document.
     * @param rect the rectangle in the document coordinate system.
     * @param omitHiddenShapes if true, only visible shapes are considered
     */
    QList<KShape *> shapesAt(const QRectF &rect, bool omitHiddenShapes = true);

    /**
     * Paint a shape
     *
     * @param shape the shape to paint
     * @param painter the painter to paint to.
     * @param converter to convert between document and view coordinates.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     */
    void paintShape(KShape *shape, QPainter &painter, const KViewConverter &converter, bool forPrint);

    /**
     * Set the strategy of the KShapeManager
     *
     * This can be used to change the behaviour of the painting of the shapes.
     * @param strategy the new strategy. The ownership of the argument \p
     *    strategy will be taken by the shape manager.
     */
    void setPaintingStrategy(KShapeManagerPaintingStrategy *strategy);

    QPolygonF routeConnection(KShapeConnection *connection);

    /**
     * \internal
     * Returns the private object for use within the flake lib
     */
    KShapeManagerPrivate *priv();

signals:
    friend class KShapeManagerPrivate;
    /// emitted when the selection is changed
    void selectionChanged();
    /// emitted when an object in the selection is changed (moved/rotated etc)
    void selectionContentChanged();
    /// emitted whenever any shape has any change.
    void notifyShapeChanged(KShape *shape, KShape::ChangeType type);

private:
    KShapeManagerPrivate * const d;
    Q_PRIVATE_SLOT(d, void updateTree())
};

#endif
