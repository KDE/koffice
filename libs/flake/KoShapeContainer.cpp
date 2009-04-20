/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "KoShapeContainer.h"
#include "KoShapeContainerModel.h"
#include "KoShapeBorderModel.h"

#include <QPointF>
#include <QPainter>
#include <QPainterPath>

class ChildrenData : public KoShapeContainerModel
{
public:
    ChildrenData() {}
    ~ChildrenData() {
        qDeleteAll(m_relations);
    }

    void add(KoShape *child) {
        Relation *r = new Relation(child);
        m_relations.append(r);
    }
    void proposeMove(KoShape *shape, QPointF &move) {
        KoShapeContainer *parent = shape->parent();
        bool allowedToMove = true;
        while (allowedToMove && parent) {
            allowedToMove = parent->isEditable();
            parent = parent->parent();
        }
        if (! allowedToMove) {
            move.setX(0);
            move.setY(0);
        }
    }

    void setClipping(const KoShape *child, bool clipping) {
        Relation *relation = findRelation(child);
        if (relation == 0) // throw exception?
            return;
        if (relation->m_inside == clipping)
            return;
        relation->m_inside = clipping;
        relation->child()->update();
        relation->child()->notifyChanged();
        relation->child()->update();
    }

    bool childClipped(const KoShape *child) const {
        Relation *relation = findRelation(child);
        if (relation == 0) // throw exception?
            return false;
        return relation->m_inside;
    }

    void remove(KoShape *child) {
        Relation *relation = findRelation(child);
        if (relation == 0)
            return;
        m_relations.removeAll(relation);
    }

    int count() const {
        return m_relations.count();
    }

    QList<KoShape*> iterator() const {
        QList<KoShape*> answer;
        foreach(Relation *relation, m_relations)
            answer.append(relation->child());
        return answer;
    }

    bool isChildLocked(const KoShape *child) const {
        return child->isGeometryProtected();
    }

    void containerChanged(KoShapeContainer *) { }
    void childChanged(KoShape *, KoShape::ChangeType) { }

private:
    /**
     * This class is a simple data-storage class for Relation objects.
     */
    class Relation
    {
    public:
        explicit Relation(KoShape *child) : m_inside(false) , m_child(child) { }
        KoShape* child() {
            return m_child;
        }
        bool m_inside; ///< if true, the child will be clipped by the parent.
    private:
        KoShape *m_child;
    };

    Relation* findRelation(const KoShape *child) const {
        foreach(Relation *relation, m_relations) {
            if (relation->child() == child)
                return relation;
        }
        return 0;
    }

private: // members
    QList <Relation *> m_relations;
};

class KoShapeContainer::Private
{
public:
    Private() : children(0) {}
    ~Private() {
        if (children) {
            foreach(KoShape *shape, children->iterator())
                shape->setParent(0);
            delete children;
        }
    }
    KoShapeContainerModel *children;
};

KoShapeContainer::KoShapeContainer() : KoShape(), d(new Private())
{
}

KoShapeContainer::KoShapeContainer(KoShapeContainerModel *model)
        : KoShape(),
        d(new Private())
{
    d->children = model;
}

KoShapeContainer::~KoShapeContainer()
{
    delete d;
}

void KoShapeContainer::addChild(KoShape *shape)
{
    Q_ASSERT(shape);
    if (shape->parent() == this && iterator().contains(shape))
        return;
    if (d->children == 0)
        d->children = new ChildrenData();
    if (shape->parent() && shape->parent() != this)
        shape->parent()->removeChild(shape);
    d->children->add(shape);
    shape->setParent(this);
    childCountChanged();
}

void KoShapeContainer::removeChild(KoShape *shape)
{
    Q_ASSERT(shape);
    if (d->children == 0)
        return;
    d->children->remove(shape);
    shape->setParent(0);
    childCountChanged();
}

int  KoShapeContainer::childCount() const
{
    if (d->children == 0)
        return 0;
    return d->children->count();
}

bool KoShapeContainer::isChildLocked(const KoShape *child) const
{
    if (d->children == 0)
        return false;
    return d->children->isChildLocked(child);
}

void KoShapeContainer::setClipping(const KoShape *child, bool clipping)
{
    if (d->children == 0)
        return;
    d->children->setClipping(child, clipping);
}

void KoShapeContainer::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();
    paintComponent(painter, converter);
    painter.restore();
    if (d->children == 0 || d->children->count() == 0)
        return;

    QList<KoShape*> sortedObjects = d->children->iterator();
    qSort(sortedObjects.begin(), sortedObjects.end(), KoShape::compareShapeZIndex);

    QMatrix baseMatrix = absoluteTransformation(0).inverted() * painter.matrix();

    // clip the children to the parent outline.
    QMatrix m;
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    m.scale(zoomX, zoomY);
    painter.setClipPath(m.map(outline()));

    foreach(KoShape *shape, sortedObjects) {
        //kDebug(30006) <<"KoShapeContainer::painting shape:" << shape->shapeId() <<"," << shape->boundingRect();
        if (! shape->isVisible())
            continue;
        if (! childClipped(shape))  // the shapeManager will have to draw those, or else we can't do clipRects
            continue;
        if (painter.hasClipping()) {
            QRectF rect = converter.viewToDocument(painter.clipRegion().boundingRect());
            rect = matrix().mapRect(rect);
            // don't try to draw a child shape that is not in the clipping rect of the painter.
            if (! rect.intersects(shape->boundingRect()))
                continue;
        }

        painter.save();
        painter.setMatrix(shape->absoluteTransformation(&converter) * baseMatrix);
        shape->paint(painter, converter);
        painter.restore();
        if (shape->border()) {
            painter.save();
            painter.setMatrix(shape->absoluteTransformation(&converter) * baseMatrix);
            shape->border()->paintBorder(shape, painter, converter);
            painter.restore();
        }
    }
}

void KoShapeContainer::shapeChanged(ChangeType type)
{
    if (d->children == 0)
        return;
    if (!(type == RotationChanged || type == ScaleChanged || type == ShearChanged
            || type == SizeChanged || type == PositionChanged))
        return;
    d->children->containerChanged(this);
    foreach(KoShape *shape, d->children->iterator())
        shape->notifyChanged();
}

bool KoShapeContainer::childClipped(const KoShape *child) const
{
    if (d->children == 0) // throw exception??
        return false;
    return d->children->childClipped(child);
}

void KoShapeContainer::update() const
{
    KoShape::update();
    if (d->children)
        foreach(KoShape *shape, d->children->iterator())
            shape->update();
}

QList<KoShape*> KoShapeContainer::iterator() const
{
    if (d->children == 0)
        return QList<KoShape*>();

    return d->children->iterator();
}

KoShapeContainerModel *KoShapeContainer::model() const
{
    return d->children;
}

