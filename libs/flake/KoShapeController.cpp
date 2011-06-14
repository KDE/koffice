/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeController.h"
#include "KoShapeControllerBase.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KoShapeLayer.h"
#include "KSelection.h"
#include "commands/KoShapeCreateCommand.h"
#include "commands/KoShapeDeleteCommand.h"
#include "KCanvasBase.h"
#include "KoShapeConfigWidgetBase.h"
#include "KoShapeFactoryBase.h"
#include "KShape.h"
#include "KoShapeRegistry.h"

#include <kpagedialog.h>
#include <klocale.h>

class KoShapeController::Private
{
public:
    Private()
        : canvas(0),
        shapeController(0),
        dummyRm(0)
    {
    }

    ~Private()
    {
        delete dummyRm;
    }

    KCanvasBase *canvas;
    KoShapeControllerBase *shapeController;
    KResourceManager *dummyRm; // only used when there is no shapeController

    QUndoCommand* addShape(KShape *shape, QUndoCommand *parent) {
        Q_ASSERT(canvas->shapeManager());
        // set the active layer as parent if there is not yet a parent.
        if (!shape->parent()) {
            shape->setParent(canvas->shapeManager()->selection()->activeLayer());
        }

        return new KoShapeCreateCommand(shapeController, shape, parent);
    }
};

KoShapeController::KoShapeController(KCanvasBase *canvas, KoShapeControllerBase *shapeController)
        : d(new Private())
{
    d->canvas = canvas;
    d->shapeController = shapeController;
}

KoShapeController::~KoShapeController()
{
    delete d;
}

QUndoCommand* KoShapeController::addShape(KShape *shape, QUndoCommand *parent)
{
    return d->addShape(shape, parent);
}

QUndoCommand* KoShapeController::removeShape(KShape *shape, QUndoCommand *parent)
{
    return new KoShapeDeleteCommand(d->shapeController, shape, parent);
}

QUndoCommand* KoShapeController::removeShapes(const QList<KShape*> &shapes, QUndoCommand *parent)
{
    return new KoShapeDeleteCommand(d->shapeController, shapes, parent);
}

void KoShapeController::setShapeControllerBase(KoShapeControllerBase* shapeControllerBase)
{
    d->shapeController = shapeControllerBase;
}

KResourceManager *KoShapeController::resourceManager() const
{
    if (!d->shapeController) {
        if (!d->dummyRm) {
            d->dummyRm = new KResourceManager();
            KoShapeRegistry *registry = KoShapeRegistry::instance();
            foreach (const QString &id, registry->keys()) {
                KoShapeFactoryBase *shapeFactory = registry->value(id);
                shapeFactory->newDocumentResourceManager(d->dummyRm);
            }
        }
        return d->dummyRm;
    }
    return d->shapeController->resourceManager();
}
