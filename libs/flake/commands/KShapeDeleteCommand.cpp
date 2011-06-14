/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KShapeDeleteCommand.h"
#include "KShapeContainer.h"
#include "KShapeControllerBase.h"

#include <klocale.h>

class KShapeDeleteCommand::Private
{
public:
    Private(KShapeControllerBase *c)
            : controller(c),
            deleteShapes(false) {
    }

    ~Private() {
        if (! deleteShapes)
            return;

        foreach(KShape *shape, shapes)
            delete shape;
    }

    KShapeControllerBase *controller; ///< the shape controller to use for removing/readding
    QList<KShape*> shapes; ///< the list of shapes to delete
    QList<KShapeContainer*> oldParents; ///< the old parents of the shapes
    bool deleteShapes;  ///< shows if shapes should be deleted when deleting the command
};

KShapeDeleteCommand::KShapeDeleteCommand(KShapeControllerBase *controller, KShape *shape, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private(controller))
{
    d->shapes.append(shape);
    d->oldParents.append(shape->parent());

    setText(i18n("Delete Shape"));
}

KShapeDeleteCommand::KShapeDeleteCommand(KShapeControllerBase *controller, const QList<KShape*> &shapes,
        QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private(controller))
{
    d->shapes = shapes;
    foreach(KShape *shape, d->shapes) {
        d->oldParents.append(shape->parent());
    }

    setText(i18n("Delete Shapes"));
}

KShapeDeleteCommand::~KShapeDeleteCommand()
{
    delete d;
}

void KShapeDeleteCommand::redo()
{
    QUndoCommand::redo();
    if (! d->controller)
        return;

    for (int i = 0; i < d->shapes.count(); i++) {
        // the parent has to be there when it is removed from the KShapeControllerBase
        d->controller->removeShape(d->shapes[i]);
        if (d->oldParents.at(i))
            d->oldParents.at(i)->removeShape(d->shapes[i]);
    }
    d->deleteShapes = true;
}

void KShapeDeleteCommand::undo()
{
    QUndoCommand::undo();
    if (! d->controller)
        return;

    for (int i = 0; i < d->shapes.count(); i++) {
        if (d->oldParents.at(i))
            d->oldParents.at(i)->addShape(d->shapes[i]);
        // the parent has to be there when it is added to the KShapeControllerBase
        d->controller->addShape(d->shapes[i]);
    }
    d->deleteShapes = false;
}
