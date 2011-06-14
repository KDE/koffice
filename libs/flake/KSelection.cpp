/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006-2007,2009 Thomas Zander <zander@kde.org>

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

#include "KSelection.h"
#include "KSelection_p.h"
#include "KShapeContainer.h"
#include "KShapeGroup.h"
#include "KPointerEvent.h"

#include <QTimer>

QRectF KSelectionPrivate::sizeRect()
{
    bool first = true;
    QRectF bb;

    QTransform invSelectionTransform = q->absoluteTransformation(0).inverted();

    QRectF bound;

    if (!selectedShapes.isEmpty()) {
        QList<KShape*>::const_iterator it = selectedShapes.constBegin();
        for (; it != selectedShapes.constEnd(); ++it) {
            if (dynamic_cast<KShapeGroup*>(*it))
                continue;

            const QTransform shapeTransform = (*it)->absoluteTransformation(0);
            const QRectF shapeRect(QRectF(QPointF(), (*it)->size()));

            if (first) {
                bb = (shapeTransform * invSelectionTransform).mapRect(shapeRect);
                bound = shapeTransform.mapRect(shapeRect);
                first = false;
            } else {
                bb = bb.united((shapeTransform * invSelectionTransform).mapRect(shapeRect));
                bound = bound.united(shapeTransform.mapRect(shapeRect));
            }
        }
    }

    globalBound = bound;
    return bb;
}

void KSelectionPrivate::requestSelectionChangedEvent()
{
    if (eventTriggered)
        return;
    eventTriggered = true;
    QTimer::singleShot(0, q, SLOT(selectionChangedEvent()));
}

void KSelectionPrivate::selectionChangedEvent()
{
    eventTriggered = false;
    emit q->selectionChanged();
}

void KSelectionPrivate::selectGroupChildren(KShapeGroup *group)
{
    if (! group)
        return;

    foreach(KShape *shape, group->shapes()) {
        if (selectedShapes.contains(shape))
            continue;
        selectedShapes << shape;

        KShapeGroup *childGroup = dynamic_cast<KShapeGroup*>(shape);
        if (childGroup)
            selectGroupChildren(childGroup);
    }
}

void KSelectionPrivate::deselectGroupChildren(KShapeGroup *group)
{
    if (! group)
        return;

    foreach(KShape *shape, group->shapes()) {
        if (selectedShapes.contains(shape))
            selectedShapes.removeAll(shape);

        KShapeGroup *childGroup = dynamic_cast<KShapeGroup*>(shape);
        if (childGroup)
            deselectGroupChildren(childGroup);
    }
}

////////////

KSelection::KSelection(QObject *parent)
    : QObject(parent),
    KShape(*(new KSelectionPrivate(this)))
{
}

KSelection::~KSelection()
{
}

void KSelection::select(KShape *shape, bool recursive)
{
    Q_D(KSelection);
    Q_ASSERT(shape != this);
    Q_ASSERT(shape);
    if (!shape->isSelectable() || !shape->isVisible(true))
        return;

    // save old number of selected shapes
    int oldSelectionCount = d->selectedShapes.count();

    if (!d->selectedShapes.contains(shape))
        d->selectedShapes << shape;

    // automatically recursively select all child shapes downwards in the hierarchy
    KShapeGroup *group = dynamic_cast<KShapeGroup*>(shape);
    if (group)
        d->selectGroupChildren(group);

    if (recursive) {
        // recursively select all parents and their children upwards the hierarchy
        KShapeContainer *parent = shape->parent();
        while (parent) {
            KShapeGroup *parentGroup = dynamic_cast<KShapeGroup*>(parent);
            if (! parentGroup) break;
            if (! d->selectedShapes.contains(parentGroup)) {
                d->selectedShapes << parentGroup;
                d->selectGroupChildren(parentGroup);
            }
            parent = parentGroup->parent();
        }
    }

    if (d->selectedShapes.count() == 1) {
        setTransformation(shape->absoluteTransformation(0));
        updateSizeAndPosition();
    } else {
        // reset global bound if there were no shapes selected before
        if (!oldSelectionCount)
            d->globalBound = QRectF();

        setTransformation(QTransform());
        // we are resetting the transformation here anyway,
        // so we can just add the newly selected shapes to the bounding box
        // in document coordinates and then use that size and position
        int newSelectionCount = d->selectedShapes.count();
        for (int i = oldSelectionCount; i < newSelectionCount; ++i) {
            KShape *shape = d->selectedShapes[i];

            // don't add the rect of the group rect, as it can be invalid
            if (dynamic_cast<KShapeGroup*>(shape)) {
                continue;
            }
            const QTransform shapeTransform = shape->absoluteTransformation(0);
            const QRectF shapeRect(QRectF(QPointF(), shape->size()));

            d->globalBound = d->globalBound.united(shapeTransform.mapRect(shapeRect));
        }
        setSize(d->globalBound.size());
        setPosition(d->globalBound.topLeft());
    }

    d->requestSelectionChangedEvent();
}

void KSelection::deselect(KShape *shape, bool recursive)
{
    Q_D(KSelection);
    if (! d->selectedShapes.contains(shape))
        return;

    d->selectedShapes.removeAll(shape);

    KShapeGroup *group = dynamic_cast<KShapeGroup*>(shape);
    if (recursive) {
        // recursively find the top group upwards int the hierarchy
        KShapeGroup *parentGroup = dynamic_cast<KShapeGroup*>(shape->parent());
        while (parentGroup) {
            group = parentGroup;
            parentGroup = dynamic_cast<KShapeGroup*>(parentGroup->parent());
        }
    }
    if (group)
        d->deselectGroupChildren(group);

    if (count() == 1)
        setTransformation(firstSelectedShape()->absoluteTransformation(0));

    updateSizeAndPosition();

    d->requestSelectionChangedEvent();
}

void KSelection::deselectAll()
{
    Q_D(KSelection);
    // reset the transformation matrix of the selection
    setTransformation(QTransform());

    if (d->selectedShapes.isEmpty())
        return;
    d->selectedShapes.clear();
    d->requestSelectionChangedEvent();
}

int KSelection::count() const
{
    Q_D(const KSelection);
    int count = 0;
    foreach(KShape *shape, d->selectedShapes)
        if (dynamic_cast<KShapeGroup*>(shape) == 0)
            ++count;
    return count;
}

bool KSelection::hitTest(const QPointF &position) const
{
    Q_D(const KSelection);
    if (count() > 1) {
        QRectF bb(boundingRect());
        return bb.contains(position);
    } else if (count() == 1) {
        return (*d->selectedShapes.begin())->hitTest(position);
    } else { // count == 0
        return false;
    }
}
void KSelection::updateSizeAndPosition()
{
    Q_D(KSelection);
    QRectF bb = d->sizeRect();
    QTransform matrix = absoluteTransformation(0);
    setSize(bb.size());
    QPointF p = matrix.map(bb.topLeft() + matrix.inverted().map(position()));
    setPosition(p);
}

QRectF KSelection::boundingRect() const
{
    return absoluteTransformation(0).mapRect(QRectF(QPointF(), size()));
}

const QList<KShape*> KSelection::selectedShapes(KoFlake::SelectionType strip) const
{
    Q_D(const KSelection);
    QList<KShape*> answer;
    // strip the child objects when there is also a parent included.
    bool doStripping = strip == KoFlake::StrippedSelection;
    foreach(KShape *shape, d->selectedShapes) {
        KShapeContainer *container = shape->parent();
        if (strip != KoFlake::TopLevelSelection && dynamic_cast<KShapeGroup*>(shape))
            // since a KShapeGroup
            // guarentees all its children are selected at the same time as itself
            // is selected we will only return its children.
            continue;
        bool add = true;
        while (doStripping && add && container) {
            if (dynamic_cast<KShapeGroup*>(container) == 0 && d->selectedShapes.contains(container))
                add = false;
            container = container->parent();
        }
        if (strip == KoFlake::TopLevelSelection && container && d->selectedShapes.contains(container))
            add = false;
        if (add)
            answer << shape;
    }
    return answer;
}

bool KSelection::isSelected(const KShape *shape) const
{
    Q_D(const KSelection);
    if (shape == this)
        return true;

    foreach (KShape *s, d->selectedShapes) {
        if (s == shape)
            return true;
    }

    return false;
}

KShape *KSelection::firstSelectedShape(KoFlake::SelectionType strip) const
{
    QList<KShape*> set = selectedShapes(strip);
    if (set.isEmpty())
        return 0;
    return *(set.begin());
}

void KSelection::setActiveLayer(KoShapeLayer *layer)
{
    Q_D(KSelection);
    d->activeLayer = layer;
    emit currentLayerChanged(layer);
}

KoShapeLayer* KSelection::activeLayer() const
{
    Q_D(const KSelection);
    return d->activeLayer;
}

#include <KSelection.moc>
