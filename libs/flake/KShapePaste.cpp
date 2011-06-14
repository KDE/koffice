/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2010 Jan Hambrecht <jaham@gmx.net>

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

#include "KShapePaste.h"

#include <kdebug.h>
#include <klocale.h>

#include <KOdfLoadingContext.h>
#include <KOdfStoreReader.h>

#include "KCanvasBase.h"
#include "KShapeController.h"
#include "KShape.h"
#include "KShapeLayer.h"
#include "KShapeLoadingContext.h"
#include "KShapeManager.h"
#include "KShapeControllerBase.h"
#include "KShapeRegistry.h"
#include "commands/KShapeCreateCommand.h"

#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

class KShapePaste::Private
{
public:
    Private(KCanvasBase *cb, KShapeLayer *l) : canvas(cb), layer(l) {}

    KCanvasBase *canvas;
    KShapeLayer *layer;
    QList<KShape*> pastedShapes;
};

KShapePaste::KShapePaste(KCanvasBase *canvas, KShapeLayer *layer)
        : d(new Private(canvas, layer))
{
}

KShapePaste::~KShapePaste()
{
    delete d;
}

bool KShapePaste::process(const KXmlElement & body, KOdfStoreReader & odfStore)
{
    d->pastedShapes.clear();
    KOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
    KShapeLoadingContext context(loadingContext, d->canvas->shapeController()->resourceManager());

    QList<KShape*> shapes(d->layer ? d->layer->shapes(): d->canvas->shapeManager()->topLevelShapes());

    int zIndex = 0;
    if (!shapes.isEmpty()) {
        zIndex = shapes.first()->zIndex();
        foreach (KShape * shape, shapes) {
            zIndex = qMax(zIndex, shape->zIndex());
        }
        ++zIndex;
    }
    context.setZIndex(zIndex);

    QUndoCommand *cmd = 0;

    QPointF copyOffset(10.0, 10.0);
    // read copy offset from settings
    KSharedConfigPtr config = KGlobal::config();
    if (config->hasGroup("Misc")) {
        const qreal offset = config->group("Misc").readEntry("CopyOffset", 10.0);
        copyOffset = QPointF(offset, offset);
    }

    // TODO if this is a text create a text shape and load the text inside the new shape.
    KXmlElement element;
    forEachElement(element, body) {
        kDebug(30006) << "loading shape" << element.localName();

        KShape * shape = KShapeRegistry::instance()->createShapeFromOdf(element, context);
        if (shape) {
            if (!cmd)
                cmd = new QUndoCommand(i18n("Paste Shapes"));

            KShapeManager *sm = d->canvas->shapeManager();
            Q_ASSERT(sm);
            bool done = true;
            do {
                // find a nice place for our shape.
                done = true;
                foreach (const KShape *s, sm->shapesAt(shape->boundingRect()) + d->pastedShapes) {
                    if (d->layer && s->parent() != d->layer)
                        continue;
                    if (s->name() != shape->name())
                        continue;
                    if (qAbs(s->position().x() - shape->position().x()) > 0.001)
                        continue;
                    if (qAbs(s->position().y() - shape->position().y()) > 0.001)
                        continue;
                    if (qAbs(s->size().width() - shape->size().width()) > 0.001)
                        continue;
                    if (qAbs(s->size().height() - shape->size().height()) > 0.001)
                        continue;
                    // move it and redo our iteration.
                    QPointF move(copyOffset);
                    d->canvas->clipToDocument(shape, move);
                    if (move.x() != 0 || move.y() != 0) {
                        shape->setPosition(shape->position() + move);
                        done = false;
                        break;
                    }
                }
            } while (!done);

            if (!shape->parent()) {
                shape->setParent(d->layer);
            }
            d->canvas->shapeController()->addShape(shape, cmd);
            d->pastedShapes << shape;
        }
    }
    if (cmd)
        d->canvas->addCommand(cmd);
    return true;
}

QList<KShape*> KShapePaste::pastedShapes() const
{
    return d->pastedShapes;
}
