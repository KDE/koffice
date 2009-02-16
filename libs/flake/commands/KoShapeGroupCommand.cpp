/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006,2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeGroupCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"

#include <klocale.h>

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped,
        QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_shapes(shapes)
        , m_clipped(clipped)
        , m_container(container)
{
    Q_ASSERT(m_clipped.count() == m_shapes.count());
    foreach(KoShape* shape, m_shapes)
        m_oldParents.append(shape->parent());

    setText(i18n("Group shapes"));
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeGroup *container, QList<KoShape *> shapes, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_shapes(shapes)
        , m_container(container)
{
    foreach(KoShape* shape, m_shapes) {
        m_clipped.append(false);
        m_oldParents.append(shape->parent());
    }

    setText(i18n("Group shapes"));
}

KoShapeGroupCommand::KoShapeGroupCommand(QUndoCommand *parent)
        : QUndoCommand(parent)
{
}

void KoShapeGroupCommand::redo()
{
    QUndoCommand::redo();

    if (dynamic_cast<KoShapeGroup*>(m_container)) {
        QRectF bound = containerBoundingRect();
        QPointF oldGroupPosition = m_container->absolutePosition(KoFlake::TopLeftCorner);
        m_container->setAbsolutePosition(bound.topLeft(), KoFlake::TopLeftCorner);
        m_container->setSize(bound.size());

        if (m_container->childCount() > 0) {
            // the group has changed position and so have the group child shapes
            // -> we need compensate the group position change
            QPointF positionOffset = oldGroupPosition - bound.topLeft();
            foreach(KoShape * child, m_container->iterator())
                child->setAbsolutePosition(child->absolutePosition() + positionOffset);
        }
    }

    QMatrix groupTransform = m_container->absoluteTransformation(0).inverted();

    uint shapeCount = m_shapes.count();
    for (uint i = 0; i < shapeCount; ++i) {
        KoShape * shape = m_shapes[i];
        shape->applyAbsoluteTransformation(groupTransform);
        m_container->addChild(shape);
        m_container->setClipping(shape, m_clipped[i]);
    }
}

void KoShapeGroupCommand::undo()
{
    QUndoCommand::undo();

    QMatrix ungroupTransform = m_container->absoluteTransformation(0);
    for (int i = 0; i < m_shapes.count(); i++) {
        KoShape * shape = m_shapes[i];
        m_container->removeChild(shape);
        if (m_oldParents.at(i))
            m_oldParents.at(i)->addChild(shape);
        shape->applyAbsoluteTransformation(ungroupTransform);
    }

    if (dynamic_cast<KoShapeGroup*>(m_container)) {
        QPointF oldGroupPosition = m_container->absolutePosition(KoFlake::TopLeftCorner);
        if (m_container->childCount() > 0) {
            bool boundingRectInitialized = false;
            QRectF bound;
            foreach(KoShape * shape, m_container->iterator()) {
                if (! boundingRectInitialized) {
                    bound = shape->boundingRect();
                    boundingRectInitialized = true;
                } else
                    bound = bound.unite(shape->boundingRect());
            }
            // the group has changed position and so have the group child shapes
            // -> we need compensate the group position change
            QPointF positionOffset = oldGroupPosition - bound.topLeft();
            foreach(KoShape * child, m_container->iterator())
                child->setAbsolutePosition(child->absolutePosition() + positionOffset);

            m_container->setAbsolutePosition(bound.topLeft(), KoFlake::TopLeftCorner);
            m_container->setSize(bound.size());
        }
    }
}

QRectF KoShapeGroupCommand::containerBoundingRect()
{
    bool boundingRectInitialized = true;
    QRectF bound;
    if (m_container->childCount() > 0)
        bound = m_container->boundingRect();
    else
        boundingRectInitialized = false;

    foreach(KoShape *shape, m_shapes) {
        if (boundingRectInitialized)
            bound = bound.unite(shape->boundingRect());
        else {
            bound = shape->boundingRect();
            boundingRectInitialized = true;
        }
    }
    return bound;
}
