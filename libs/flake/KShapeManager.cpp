/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
   Copyright (C) 2009-2010 Jan Hambrecht <jaham@gmx.net>

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

#include "KShapeManager.h"
#include "KShapeManager_p.h"
#include "KShapeSelection.h"
#include "KToolManager.h"
#include "KPointerEvent.h"
#include "KCanvasBase.h"
#include "KShapeContainer.h"
#include "KShapeBorderBase.h"
#include "KToolProxy.h"
#include "KShapeManagerPaintingStrategy.h"
#include "KShapeShadow.h"
#include "KShapeConnection.h"
#include "KShapeConnection_p.h"
#include "KShapeLayer.h"
#include "KFilterEffect.h"
#include "KFilterEffectStack.h"
#include "KFilterEffectRenderContext.h"
#include "KShapeBackgroundBase.h"
#include <KRTree.h>

#include <QPainter>
#include <QTimer>
#include <QtCore/qmath.h>
#include <kdebug.h>

// #define DEBUG_CONNECTIONS

KShapeManagerPrivate::KShapeManagerPrivate(KShapeManager *shapeManager, KCanvasBase *c)
    : selection(new KShapeSelection(shapeManager)),
    canvas(c),
    tree(4, 2),
    connectionTree(4, 2),
    strategy(new KShapeManagerPaintingStrategy(shapeManager)),
    q(shapeManager)
{
}

KShapeManagerPrivate::~KShapeManagerPrivate() {
    delete strategy;
}

void KShapeManagerPrivate::updateTree()
{
    // for detecting collisions between shapes.
    DetectCollision detector;
    bool selectionModified = false;
    foreach (KShape *shape, aggregate4update) {
        if (shapeIndexesBeforeUpdate.contains(shape))
            detector.detect(tree, shape, shapeIndexesBeforeUpdate[shape]);
        selectionModified = selectionModified || selection->isSelected(shape);
    }

    foreach (KShape *shape, aggregate4update) {
        tree.remove(shape);
        QRectF br(shape->boundingRect());
        strategy->adapt(shape, br);
        tree.insert(br, shape);

        foreach (KShapeConnection *connection, shape->priv()->connections) {
            connectionTree.remove(connection);
            connectionTree.insert(connection->boundingRect(), connection);
        }
    }

    // do it again to see which shapes we intersect with _after_ moving.
    foreach (KShape *shape, aggregate4update)
        detector.detect(tree, shape, shapeIndexesBeforeUpdate[shape]);
    aggregate4update.clear();
    shapeIndexesBeforeUpdate.clear();

    detector.fireSignals();
    if (selectionModified) {
        selection->updateSizeAndPosition();
        emit q->selectionContentChanged();
    }
}

void KShapeManagerPrivate::paintGroup(KShapeGroup *group, QPainter &painter, const KViewConverter &converter, bool forPrint)
{
    QList<KShape*> shapes = group->shapes();
    qSort(shapes.begin(), shapes.end(), KShape::compareShapeZIndex);
    foreach(KShape *child, shapes) {
        // we paint recursively here, so we do not have to check recursively for visibility
        if (!child->isVisible())
            continue;
        KShapeGroup *childGroup = dynamic_cast<KShapeGroup*>(child);
        if (childGroup) {
            paintGroup(childGroup, painter, converter, forPrint);
        } else {
            painter.save();
            strategy->paint(child, painter, converter, forPrint);
            painter.restore();
        }
    }
}

void KShapeManagerPrivate::addShapeConnection(KShapeConnection *connection)
{
    connectionTree.insert(connection->boundingRect(), connection);
}

void KShapeManagerPrivate::update(const QRectF &rect, const KShape *shape, bool selectionHandles)
{
    canvas->updateCanvas(rect);
    if (selectionHandles && selection->isSelected(shape)) {
        if (canvas->toolProxy())
            canvas->toolProxy()->repaintDecorations();
    }

    if (selectionHandles) {
        foreach (KShapeConnection *connection, shape->priv()->connections) {
            canvas->updateCanvas(connection->boundingRect());
            connection->priv()->foul();
        }
    }
}

void KShapeManagerPrivate::shapeGeometryChanged(KShape *shape)
{
    Q_ASSERT(shape);
    if (aggregate4update.contains(shape)) {
        return;
    }
    const bool wasEmpty = aggregate4update.isEmpty();
    aggregate4update.insert(shape);
    shapeIndexesBeforeUpdate.insert(shape, shape->zIndex());

    KShapeContainer *container = dynamic_cast<KShapeContainer*>(shape);
    if (container) {
        foreach(KShape *child, container->shapes())
            shapeGeometryChanged(child);
    }
    if (wasEmpty && !aggregate4update.isEmpty())
        QTimer::singleShot(100, q, SLOT(updateTree()));
}

void KShapeManagerPrivate::shapeChanged(KShape *shape, KShape::ChangeType type)
{
    Q_ASSERT(shape);
    emit q->notifyShapeChanged(shape, type);
    // if type == deleted; call remove?
}

void KShapeManagerPrivate::suggestChangeTool(KPointerEvent *event)
{
    QList<KShape*> shapes;

    KShape *clicked = q->shapeAt(event->point);
    if (clicked) {
        if (! selection->isSelected(clicked)) {
            selection->deselectAll();
            selection->select(clicked);
        }
        shapes.append(clicked);
    }

    QList<KShape*> shapes2;
    foreach (KShape *shape, shapes) {
        QSet<KShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) {
            shapes2.append(shape);
        } else {
            foreach (KShape *delegatedShape, delegates) {
                shapes2.append(delegatedShape);
            }
        }
    }
    KToolManager::instance()->switchToolRequested(
        KToolManager::instance()->preferredToolForSelection(shapes2));
}



KShapeManager::KShapeManager(KCanvasBase *canvas, const QList<KShape *> &shapes, QObject *parent)
        : QObject(parent),
        d(new KShapeManagerPrivate(this, canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect(d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    setShapes(shapes);
}

KShapeManager::KShapeManager(KCanvasBase *canvas, QObject *parent)
        : QObject(parent),
        d(new KShapeManagerPrivate(this, canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect(d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
}

KShapeManager::~KShapeManager()
{
    foreach(KShape *shape, d->shapes) {
        shape->priv()->removeShapeManager(this);
    }
    delete d;
}


void KShapeManager::setShapes(const QList<KShape *> &shapes, Repaint repaint)
{
    //clear selection
    d->selection->deselectAll();
    foreach(KShape *shape, d->shapes) {
        shape->priv()->removeShapeManager(this);
    }
    d->aggregate4update.clear();
    d->tree.clear();
    d->shapes.clear();
    foreach(KShape *shape, shapes) {
        addShape(shape, repaint);
    }
}

void KShapeManager::add(KShape *shape, Repaint repaint)
{
    if (d->shapes.contains(shape))
        return;
    shape->priv()->addShapeManager(this);
    foreach (KShapeConnection *connection, shape->priv()->connections) {
        d->connectionTree.insert(connection->boundingRect(), connection);
    }
    d->shapes.append(shape);
    if (! dynamic_cast<KShapeGroup*>(shape) && ! dynamic_cast<KShapeLayer*>(shape)) {
        QRectF br(shape->boundingRect());
        d->tree.insert(br, shape);
    }
    if (repaint == PaintShapeOnAdd) {
        shape->update();
    }

    // add the children of a KShapeContainer
    KShapeContainer *container = dynamic_cast<KShapeContainer*>(shape);

    if (container) {
        foreach (KShape *containerShape, container->shapes()) {
            addShape(containerShape, repaint);
        }
    }

    KShapeManagerPrivate::DetectCollision detector;
    detector.detect(d->tree, shape, shape->zIndex());
    detector.fireSignals();
}

void KShapeManager::remove(KShape *shape)
{
    KShapeManagerPrivate::DetectCollision detector;
    detector.detect(d->tree, shape, shape->zIndex());
    detector.fireSignals();

    shape->update();
    shape->priv()->removeShapeManager(this);
    d->selection->deselect(shape);
    d->aggregate4update.remove(shape);
    d->tree.remove(shape);
    d->shapes.removeAll(shape);

    // remove the children of a KShapeContainer
    KShapeContainer *container = dynamic_cast<KShapeContainer*>(shape);
    if (container) {
        foreach (KShape *containerShape, container->shapes()) {
            remove(containerShape);
        }
    }
}

void KShapeManager::paint(QPainter &painter, const KViewConverter &converter, bool forPrint)
{
    d->updateTree();
    painter.setPen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setBrush(Qt::NoBrush);

    QList<KShapeConnection*> sortedConnections;
    QList<KShape*> unsortedShapes;
    if (painter.hasClipping()) {
        QRectF rect = converter.viewToDocument(painter.clipRegion().boundingRect());
        unsortedShapes = d->tree.intersects(rect);
        sortedConnections = d->connectionTree.intersects(rect);
    } else {
        unsortedShapes = shapes();
        kWarning() << "KShapeManager::paint  Painting with a painter that has no clipping will lead to too much being painted!";
    }

    // filter all hidden shapes from the list
    // also filter shapes with a parent which has filter effects applied
    QList<KShape*> sortedShapes;
    foreach (KShape *shape, unsortedShapes) {
        if (!shape->isVisible(true))
            continue;
        bool addShapeToList = true;
        // check if one of the shapes ancestors have filter effects
        KShapeContainer *parent = shape->parent();
        while (parent) {
            // parent must be part of the shape manager to be taken into account
            if (!d->shapes.contains(parent))
                break;
            if (parent->filterEffectStack() && !parent->filterEffectStack()->isEmpty()) {
                    addShapeToList = false;
                    break;
            }
            parent = parent->parent();
        }
        if (addShapeToList) {
            sortedShapes.append(shape);
        } else if (parent) {
            sortedShapes.append(parent);
        }
    }

    qSort(sortedShapes.begin(), sortedShapes.end(), KShape::compareShapeZIndex);
    qSort(sortedConnections.begin(), sortedConnections.end(), KShapeConnection::compareConnectionZIndex);
    QList<KShapeConnection*>::iterator connectionIterator = sortedConnections.begin();

    foreach (KShape *shape, sortedShapes) {
        if (shape->parent() != 0 && shape->parent()->isClipped(shape))
            continue;
/*
        while (connectionIterator != sortedConnections.end()
                && (*connectionIterator)->zIndex() < shape->zIndex()) {
            painter.save();
            (*connectionIterator)->paint(painter, converter);
            painter.restore();
            ++connectionIterator;
        }
*/

        painter.save();
        d->strategy->paint(shape, painter, converter, forPrint);
        painter.restore();
    }

    while (connectionIterator != sortedConnections.end()) { // paint connections that are above the rest.
        painter.save();
        (*connectionIterator)->paint(painter, converter);
        painter.restore();
        ++connectionIterator;
    }


#ifdef KOFFICE_RTREE_DEBUG
    // paint tree
    qreal zx = 0;
    qreal zy = 0;
    converter.zoom(&zx, &zy);
    painter.save();
    painter.scale(zx, zy);
    d->tree.paint(painter);
    painter.restore();
#endif
}

void KShapeManager::paintShape(KShape *shape, QPainter &painter, const KViewConverter &converter, bool forPrint)
{
    qreal transparency = shape->transparency(KShape::EffectiveTransparency);
    if (transparency > 0.0) {
        painter.setOpacity(1.0-transparency);
    }

    if (shape->shadow()) {
        painter.save();
        shape->shadow()->paint(shape, painter, converter);
        painter.restore();
    }
    if (!shape->filterEffectStack() || shape->filterEffectStack()->isEmpty()) {
        painter.save();
        shape->paint(painter, converter);
        painter.restore();
        if (shape->border()) {
            painter.save();
            shape->border()->paint(shape, painter, converter);
            painter.restore();
        }
    } else {
        // There are filter effects, then we need to prerender the shape on an image, to filter it
        QRectF shapeBound(QPointF(), shape->size());
        // First step, compute the rectangle used for the image
        QRectF clipRegion = shape->filterEffectStack()->clipRectForBoundingRect(shapeBound);
        // convert clip region to view coordinates
        QRectF zoomedClipRegion = converter.documentToView(clipRegion);
        // determine the offset of the clipping rect from the shapes origin
        QPointF clippingOffset = zoomedClipRegion.topLeft();

        // Initialize the buffer image
        QImage sourceGraphic(zoomedClipRegion.size().toSize(), QImage::Format_ARGB32_Premultiplied);
        sourceGraphic.fill(qRgba(0,0,0,0));

        QHash<QString, QImage> imageBuffers;

        QSet<QString> requiredStdInputs = shape->filterEffectStack()->requiredStandarsInputs();

        if (requiredStdInputs.contains("SourceGraphic") || requiredStdInputs.contains("SourceAlpha")) {
            // Init the buffer painter
            QPainter imagePainter(&sourceGraphic);
            imagePainter.translate(-1.0f*clippingOffset);
            imagePainter.setPen(Qt::NoPen);
            imagePainter.setBrush(Qt::NoBrush);
            imagePainter.setRenderHint(QPainter::Antialiasing, painter.testRenderHint(QPainter::Antialiasing));

            // Paint the shape on the image
            KShapeGroup *group = dynamic_cast<KShapeGroup*>(shape);
            if (group) {
                // the childrens matrix contains the groups matrix as well
                // so we have to compensate for that before painting the children
                imagePainter.setTransform(group->absoluteTransformation(&converter).inverted(), true);
                d->paintGroup(group, imagePainter, converter, forPrint);
            } else {
                imagePainter.save();
                shape->paint(imagePainter, converter);
                imagePainter.restore();
                if (shape->border()) {
                    imagePainter.save();
                    shape->border()->paint(shape, imagePainter, converter);
                    imagePainter.restore();
                }
                imagePainter.end();
            }
        }
        if (requiredStdInputs.contains("SourceAlpha")) {
            QImage sourceAlpha = sourceGraphic;
            sourceAlpha.fill(qRgba(0,0,0,255));
            sourceAlpha.setAlphaChannel(sourceGraphic.alphaChannel());
            imageBuffers.insert("SourceAlpha", sourceAlpha);
        }
        if (requiredStdInputs.contains("FillPaint")) {
            QImage fillPaint = sourceGraphic;
            if (shape->background()) {
                QPainter fillPainter(&fillPaint);
                QPainterPath fillPath;
                fillPath.addRect(fillPaint.rect().adjusted(-1,-1,1,1));
                shape->background()->paint(fillPainter, fillPath);
            } else {
                fillPaint.fill(qRgba(0,0,0,0));
            }
            imageBuffers.insert("FillPaint", fillPaint);
        }

        imageBuffers.insert("SourceGraphic", sourceGraphic);
        imageBuffers.insert(QString(), sourceGraphic);

        KFilterEffectRenderContext renderContext(converter);
        renderContext.setShapeBoundingBox(shapeBound);

        QImage result;
        QList<KFilterEffect*> filterEffects = shape->filterEffectStack()->filterEffects();
        // Filter
        foreach (KFilterEffect *filterEffect, filterEffects) {
            QRectF filterRegion = filterEffect->filterRectForBoundingRect(shapeBound);
            filterRegion = converter.documentToView(filterRegion);
            QRect subRegion = filterRegion.translated(-clippingOffset).toRect();
            // set current filter region
            renderContext.setFilterRegion(subRegion & sourceGraphic.rect());

            if (filterEffect->maximalInputCount() <= 1) {
                QList<QString> inputs = filterEffect->inputs();
                QString input = inputs.count() ? inputs.first() : QString();
                // get input image from image buffers and apply the filter effect
                QImage image = imageBuffers.value(input);
                if (!image.isNull()) {
                    result = filterEffect->processImage(imageBuffers.value(input), renderContext);
                }
            } else {
                QList<QImage> inputImages;
                foreach(const QString &input, filterEffect->inputs()) {
                    QImage image = imageBuffers.value(input);
                    if (!image.isNull())
                        inputImages.append(imageBuffers.value(input));
                }
                // apply the filter effect
                if (filterEffect->inputs().count() == inputImages.count())
                    result = filterEffect->processImages(inputImages, renderContext);
            }
            // store result of effect
            imageBuffers.insert(filterEffect->output(), result);
        }

        KFilterEffect *lastEffect = filterEffects.last();

        // Paint the result
        painter.save();
        painter.drawImage(clippingOffset, imageBuffers.value(lastEffect->output()));
        painter.restore();
    }
    if (! forPrint) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        shape->paintDecorations(painter, converter, d->canvas);
    }
}

KShapeConnection *KShapeManager::connectionAt(const QPointF &position)
{
    d->updateTree();
    QList<KShapeConnection*> sortedConnections(d->connectionTree.contains(position));
    if (sortedConnections.isEmpty())
        return 0;
    qSort(sortedConnections.begin(), sortedConnections.end(), KShapeConnection::compareConnectionZIndex);
    return sortedConnections.first();
}

KShape *KShapeManager::shapeAt(const QPointF &position, KFlake::ShapeSelection selection, bool omitHiddenShapes)
{
    d->updateTree();
    QList<KShape*> sortedShapes(d->tree.contains(position));
    qSort(sortedShapes.begin(), sortedShapes.end(), KShape::compareShapeZIndex);
    KShape *firstUnselectedShape = 0;
    for (int count = sortedShapes.count() - 1; count >= 0; count--) {
        KShape *shape = sortedShapes.at(count);
        if (omitHiddenShapes && ! shape->isVisible(true))
            continue;
        if (! shape->hitTest(position))
            continue;

        switch (selection) {
        case KFlake::ShapeOnTop:
            if (shape->isSelectable())
                return shape;
        case KFlake::Selected:
            if (d->selection->isSelected(shape))
                return shape;
            break;
        case KFlake::Unselected:
            if (! d->selection->isSelected(shape))
                return shape;
            break;
        case KFlake::NextUnselected:
            // we want an unselected shape
            if (d->selection->isSelected(shape))
                continue;
            // memorize the first unselected shape
            if (! firstUnselectedShape)
                firstUnselectedShape = shape;
            // check if the shape above is selected
            if (count + 1 < sortedShapes.count() && d->selection->isSelected(sortedShapes.at(count + 1)))
                return shape;
            break;
        }
    }
    // if we want the next unselected below a selected but there was none selected,
    // return the first found unselected shape
    if (selection == KFlake::NextUnselected && firstUnselectedShape)
        return firstUnselectedShape;

    if (d->selection->hitTest(position))
        return d->selection;

    return 0; // missed everything
}

QList<KShape *> KShapeManager::shapesAt(const QRectF &rect, bool omitHiddenShapes)
{
    d->updateTree();

    QList<KShape*> intersectedShapes(d->tree.intersects(rect));
    for (int count = intersectedShapes.count() - 1; count >= 0; count--) {
        KShape *shape = intersectedShapes.at(count);
        if (omitHiddenShapes && ! shape->isVisible(true)) {
            intersectedShapes.removeAt(count);
        } else {
            const QPainterPath outline = shape->absoluteTransformation(0).map(shape->outline());
            if (! outline.intersects(rect) && ! outline.contains(rect)) {
                intersectedShapes.removeAt(count);
            }
        }
    }
    return intersectedShapes;
}

QList<KShape*> KShapeManager::shapes() const
{
    return d->shapes;
}

QList<KShape*> KShapeManager::topLevelShapes() const
{
    QList<KShape*> shapes;
    // get all toplevel shapes
    foreach(KShape *shape, d->shapes) {
        if (shape->parent() == 0) {
            shapes.append(shape);
        }
    }
    return shapes;
}

KShapeSelection *KShapeManager::selection() const
{
    return d->selection;
}

void KShapeManager::setPaintingStrategy(KShapeManagerPaintingStrategy *strategy)
{
    delete d->strategy;
    d->strategy = strategy;
}

QPolygonF KShapeManager::routeConnection(KShapeConnection *connection)
{
    return d->routeConnection(connection->startPoint(), connection->endPoint());
}

struct ko_Node;

struct ko_NodeIndex {
    inline ko_NodeIndex(const ko_Node &node);
    ko_NodeIndex() {
        x = 0;
        y = 0;
    }

    ko_NodeIndex(uint x_, uint y_)
    {
        x = x_;
        y = y_;
        Q_ASSERT(x == x_); // make sure we don't go out of bounds.
        Q_ASSERT(y == y_); // make sure we don't go out of bounds.
    }

    bool operator==(const ko_NodeIndex &other) const { // qhash needs this explicitly
        return other.x == x && other.y == y;
    }
    bool operator!=(const ko_NodeIndex &other) const { // for completeness sake
        return !operator==(other);
    }

    uint x : 16;
    uint y : 16;
};

inline uint qHash(const ko_NodeIndex &nodeIndex)
{
     return (nodeIndex.x << 16) + nodeIndex.y;
}

struct ko_Node {
    ko_Node()
    {
        init();
    }

    ko_Node(int x_, int y_)
    {
        init();
        x = x_;
        y = y_;
        Q_ASSERT(x == x_); // make sure we don't go out of bounds.
        Q_ASSERT(y == y_); // make sure we don't go out of bounds.
    }

    explicit ko_Node(const QPoint &origin, const QPointF &point)
    {
        init();
        x = qRound((point.x() - origin.x()) / 10); // the * 10 is our resolution
        y = qRound((point.y() - origin.y()) / 10);
    }

    inline void init()
    {
        x = 0;
        y = 0;
        score = 10E8; // lower is better.
    }

    QPointF point(const QPoint &origin) const {
        return QPointF(x * 10 + origin.x(), y * 10 + origin.y());
    }

    uint x : 16;
    uint y : 16;
    uint score;
    ko_NodeIndex directionForShortedPath;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const ko_Node &node)
{
#ifndef NDEBUG
    debug.nospace() << node.x << "," << node.y << "(" << node.score << ")";
#else
    Q_UNUSED(node);
#endif
    return debug.nospace();
}
#endif

inline ko_NodeIndex::ko_NodeIndex(const ko_Node &node)
{
    x = node.x;
    y = node.y;
}

QPolygonF KShapeManagerPrivate::routeConnection(const QPointF &from, const QPointF &to)
{
    QHash<ko_NodeIndex, ko_Node> nodes;

    const int OFFSET = 5000; // the 5000 to give us plenty of space to connect
    const qreal originX = qMin(from.x(), to.x()) - OFFSET;
    const qreal originY = qMin(from.y(), to.y()) - OFFSET;
    const QPoint origin(qRound(originX), qRound(originY));

    ko_Node begin(origin, from);
    begin.score = 0;
    begin.directionForShortedPath.x = originX; // set origin too, so we can determine we have no prev dir
    begin.directionForShortedPath.y = originY;
    nodes.insert(ko_NodeIndex(begin), begin);
    const ko_Node destination(origin, to);
#ifdef DEBUG_CONNECTIONS
     kDebug() << "begin" << begin.x << begin.y << "destination; " << destination.x << destination.y;
#endif
    QList<ko_NodeIndex> leafs;
    leafs << ko_NodeIndex(begin);

    int iterations = 0;
    uint bestScorePrevIteration = 10E5;
    while (true) {
        uint bestScore = 10E5;
        ++iterations;
        // 1) create neighbouring nodes
        QList<ko_NodeIndex> oldLeafs = leafs;
        leafs.clear();
        leafs.reserve(oldLeafs.size() * 4);
        const QPointF roundingError(originX - origin.x(), originY - origin.y());
        foreach (const ko_NodeIndex &index, oldLeafs) {
            enum Direction {
                Up = 0,
                Down,
                Left,
                Right,
                NotMoved
            };
            Q_ASSERT(nodes.contains(index));
            if (index.x == destination.x && index.y == destination.y) {
                // reached our goal.
                // Notice that we probably should keep searching a bit longer to at least
                // finish all the leafs we created. TODO
                ko_NodeIndex i = index;
                QPolygonF answer;
                answer << to;
                const ko_NodeIndex beginIndex(begin);
                while (i != beginIndex) {
                    Q_ASSERT(nodes.contains(i));
                    // TODO avoid inserting too many points on a straight line
#ifdef DEBUG_CONNECTIONS
                    qDebug() << "path goes via; " << nodes[i].x << "," << nodes[i].y << nodes[i].score;
#endif
                    answer << nodes[i].point(origin) + roundingError;
                    i = nodes[i].directionForShortedPath;
                }
                answer << from;
#ifdef DEBUG_CONNECTIONS
                qDebug() << "took" << iterations << "iterations and" << nodes.count() << "nodes";
#endif
                return answer;
            }

            const ko_Node &indexedNode = nodes[index];
            const uint prevScore = indexedNode.score;
            if (prevScore > bestScorePrevIteration + 9) { // skip for this round.
                leafs.append(index);
                bestScore = qMin(bestScore, prevScore);
                continue;
            }

            Direction prevDirection = NotMoved;
            if (indexedNode.x > indexedNode.directionForShortedPath.x)
                prevDirection = Right;
            else if (indexedNode.x < indexedNode.directionForShortedPath.x)
                prevDirection = Left;
            else if (indexedNode.y > indexedNode.directionForShortedPath.y)
                prevDirection = Down;
            else if (indexedNode.y < indexedNode.directionForShortedPath.y)
                prevDirection = Up;

            for (int direction = Up; direction <= Right; ++direction) {
                ko_Node node(index.x + (direction == Left ? -1 : (direction == Right ? 1 : 0)),
                        index.y + (direction == Up ? -1 : (direction == Down ? 1 : 0)));

                const QPointF currentLocation(node.point(origin));
                node.score = prevScore + 10;
                foreach (KShape *shape, tree.contains(currentLocation)) {
                    if (shape->isVisible()) {
                        node.score += 50; // going through a shape is very expensive.
                        break;
                    }
                }
                // punish wrong direction.
                if (direction == Left && begin.x <= destination.x)
                    node.score += 1;
                else if (direction == Right && begin.x >= destination.x)
                    node.score += 1;
                else if (direction == Up && begin.y <= destination.y)
                    node.score += 1;
                else if (direction == Down && begin.y >= destination.y)
                    node.score += 1;
                if (direction != prevDirection && prevDirection != NotMoved)
                    node.score += 1;

                ko_NodeIndex nodeIndex(node);
                if (nodes.contains(nodeIndex) && node.score >= nodes[nodeIndex].score) {
                    // if we are slower, just ignore the new one.
                    continue;
                }
                node.directionForShortedPath = index;
                nodes[nodeIndex] = node;
                leafs.append(nodeIndex);
                bestScore = qMin(bestScore, node.score);

#ifdef DEBUG_CONNECTIONS
                QList<ko_Node> polygon;
                ko_NodeIndex path = nodeIndex;
                const ko_NodeIndex origin(begin);
                while (path != origin) {
                    polygon << nodes[path];
                    path = nodes[path].directionForShortedPath;
                }
                qDebug() << " + " << node.score << polygon;
#endif
            }
        }
        bestScorePrevIteration = bestScore;
    }
}

KShapeManagerPrivate *KShapeManager::priv()
{
    return d;
}

#include <KShapeManager.moc>
