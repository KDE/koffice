/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#include "KShape_p.h"
#include "KoShapeGroup.h"
#include <KRTree.h>

class KoShapeManagerPrivate
{
public:
    KoShapeManagerPrivate(KoShapeManager *shapeManager, KCanvasBase *c);
    ~KoShapeManagerPrivate();

    /**
     * Update the tree when there are shapes in m_aggregate4update. This is done so not all
     * updates to the tree are done when they are asked for but when they are needed.
     */
    void updateTree();

    /**
     * Recursively paints the given group shape to the specified painter
     * This is needed for filter effects on group shapes where the filter effect
     * applies to all the children of the group shape at once
     */
    void paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, bool forPrint);

    /**
     * Add a shape connection to the manager so it can be taken into account for drawing purposes.
     * Note that this is typically called by the shape instance only.
     */
    void addShapeConnection(KoShapeConnection *connection);

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in points (the document coordinates system of KShape) and it is expected to be
     * normalized and based in the global coordinates, not any local coordinates.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param rect the rectangle (in pt) to queue for repaint.
     * @param shape the shape that is going to be redrawn; only needed when selectionHandles=true
     * @param selectionHandles if true; find out if the shape is selected and repaint its
     *   selection handles at the same time.
     */
    void update(const QRectF &rect, const KShape *shape = 0, bool selectionHandles = false);

    QPolygonF routeConnection(KoShapeConnection *connection, const QPointF &from, const QPointF &to);

    class DetectCollision
    {
    public:
        DetectCollision() {}
        void detect(KRTree<KShape *> &tree, KShape *s, int prevZIndex) {
            foreach(KShape *shape, tree.intersects(s->boundingRect())) {
                bool isChild = false;
                KoShapeContainer *parent = s->parent();
                while (parent && !isChild) {
                    if (parent == shape)
                        isChild = true;
                    parent = parent->parent();
                }
                if (isChild)
                    continue;
                if (s->zIndex() <= shape->zIndex() && prevZIndex <= shape->zIndex())
                    // Moving a shape will only make it collide with shapes below it.
                    continue;
                if (shape->collisionDetection() && !shapesWithCollisionDetection.contains(shape))
                    shapesWithCollisionDetection.append(shape);
            }
        }

        void fireSignals() {
            foreach(KShape *shape, shapesWithCollisionDetection)
                shape->priv()->shapeChanged(KShape::CollisionDetected);
        }

    private:
        QList<KShape*> shapesWithCollisionDetection;
    };

    QList<KShape *> shapes;
    QList<KShape *> additionalShapes; // these are shapes that are only handled for updates
    KSelection *selection;
    KCanvasBase *canvas;
    KRTree<KShape *> tree;
    KRTree<KoShapeConnection *> connectionTree;

    QSet<KShape *> aggregate4update;
    QHash<KShape*, int> shapeIndexesBeforeUpdate;
    KoShapeManagerPaintingStrategy *strategy;
    KoShapeManager *q;
};
