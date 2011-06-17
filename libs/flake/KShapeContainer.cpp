/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#include "KShapeContainer.h"
#include "KShapeContainer_p.h"
#include "KShapeContainerModel.h"
#include "KShapeBorderBase.h"
#include "KShapeContainerDefaultModel.h"
#include "KShapeSavingContext.h"

#include <QPointF>
#include <QPainter>
#include <QPainterPath>

KShapeContainerPrivate::KShapeContainerPrivate(KShapeContainer *q)
    : KShapePrivate(q),
    model(0)
{
}

KShapeContainerPrivate::~KShapeContainerPrivate()
{
    delete model;
}

KShapeContainer::KShapeContainer()
    : KShape(*(new KShapeContainerPrivate(this)))
{
}

KShapeContainer::KShapeContainer(KShapeContainerModel *model)
        : KShape(*(new KShapeContainerPrivate(this)))
{
    Q_D(KShapeContainer);
    d->model = model;
}

KShapeContainer::KShapeContainer(KShapeContainerPrivate &dd)
    : KShape(dd)
{
}

KShapeContainer::~KShapeContainer()
{
    Q_D(KShapeContainer);
    if (d->model) {
        foreach(KShape *shape, d->model->shapes())
            shape->setParent(0);
    }
}

void KShapeContainer::addShape(KShape *shape)
{
    Q_D(KShapeContainer);
    Q_ASSERT(shape);
    if (shape->parent() == this && shapes().contains(shape))
        return;
    if (d->model == 0)
        d->model = new KShapeContainerDefaultModel();
    if (shape->parent() && shape->parent() != this)
        shape->parent()->removeShape(shape);
    d->model->add(shape);
    shape->setParent(this);
    shapeCountChanged();
}

void KShapeContainer::removeShape(KShape *shape)
{
    Q_D(KShapeContainer);
    Q_ASSERT(shape);
    if (d->model == 0)
        return;
    d->model->remove(shape);
    shape->setParent(0);
    shapeCountChanged();

    KShapeContainer * grandparent = parent();
    if (grandparent) {
        grandparent->model()->childChanged(this, KShape::ChildChanged);
    }
}

int  KShapeContainer::shapeCount() const
{
    Q_D(const KShapeContainer);
    if (d->model == 0)
        return 0;
    return d->model->count();
}

bool KShapeContainer::isChildLocked(const KShape *child) const
{
    Q_D(const KShapeContainer);
    if (d->model == 0)
        return false;
    return d->model->isChildLocked(child);
}

void KShapeContainer::setClipped(const KShape *child, bool clipping)
{
    Q_D(KShapeContainer);
    if (d->model == 0)
        return;
    d->model->setClipped(child, clipping);
}

void KShapeContainer::setInheritsTransform(const KShape *shape, bool inherit)
{
    Q_D(KShapeContainer);
    if (d->model == 0)
        return;
    d->model->setInheritsTransform(shape, inherit);
}

bool KShapeContainer::inheritsTransform(const KShape *shape) const
{
    Q_D(const KShapeContainer);
    if (d->model == 0)
        return false;
    return d->model->inheritsTransform(shape);
}

void KShapeContainer::paint(QPainter &painter, const KViewConverter &converter)
{
    Q_D(KShapeContainer);
    painter.save();
    paintComponent(painter, converter);
    painter.restore();
    if (d->model == 0 || d->model->count() == 0)
        return;

    QList<KShape*> sortedObjects = d->model->shapes();
    qSort(sortedObjects.begin(), sortedObjects.end(), KShape::compareShapeZIndex);

    // Do the following to revert the absolute transformation of the container
    // that is re-applied in shape->absoluteTransformation() later on. The transformation matrix
    // of the container has already been applied once before this function is called.
    QTransform baseMatrix = absoluteTransformation(&converter).inverted() * painter.transform();

    // clip the children to the parent outline.
    QTransform m;
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    m.scale(zoomX, zoomY);
    painter.setClipPath(m.map(outline()));

    QRectF toPaintRect = converter.viewToDocument(painter.clipRegion().boundingRect());
    toPaintRect = transform().mapRect(toPaintRect);
    // We'll use this clipRect to see if our child shapes lie within it.
    // Because shape->boundingRect() uses absoluteTransformation(0) we'll
    // use that as well to have the same (absolute) reference transformation
    // of our and the child's bounding boxes.
    QTransform absTrans = absoluteTransformation(0);
    QRectF clipRect = absTrans.map(outline()).boundingRect();


    foreach(KShape *shape, sortedObjects) {
        //kDebug(30006) <<"KShapeContainer::painting shape:" << shape->shapeId() <<"," << shape->boundingRect();
        if (!shape->isVisible())
            continue;
        if (!isClipped(shape))  // the shapeManager will have to draw those, or else we can't do clipRects
            continue;
        // don't try to draw a child shape that is not in the clipping rect of the painter.
        if (!clipRect.intersects(shape->boundingRect()))

            continue;

        painter.save();
        painter.setTransform(shape->absoluteTransformation(&converter) * baseMatrix);
        shape->paint(painter, converter);
        painter.restore();
        if (shape->border()) {
            painter.save();
            painter.setTransform(shape->absoluteTransformation(&converter) * baseMatrix);
            shape->border()->paint(shape, painter, converter);
            painter.restore();
        }
    }
}

void KShapeContainer::paintComponent(QPainter &, const KViewConverter &)
{
    // empty
}

void KShapeContainer::shapeChanged(ChangeType type, KShape *shape)
{
    Q_D(KShapeContainer);
    Q_UNUSED(shape);
    if (d->model == 0)
        return;
    if (!(type == RotationChanged || type == ScaleChanged || type == ShearChanged
            || type == SizeChanged || type == PositionChanged || type == GenericMatrixChange))
        return;
    d->model->containerChanged(this, type);
    foreach(KShape *shape, d->model->shapes())
        shape->notifyChanged();
}

bool KShapeContainer::isClipped(const KShape *child) const
{
    Q_D(const KShapeContainer);
    if (d->model == 0) // throw exception??
        return false;
    return d->model->isClipped(child);
}

QList<KShape*> KShapeContainer::shapes() const
{
    Q_D(const KShapeContainer);
    if (d->model == 0)
        return QList<KShape*>();

    return d->model->shapes();
}

KShapeContainerModel *KShapeContainer::model() const
{
    Q_D(const KShapeContainer);
    return d->model;
}

void KShapeContainer::saveOdfChildElements(KShapeSavingContext &context) const
{
    Q_UNUSED(context);
}
