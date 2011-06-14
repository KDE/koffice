/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoCreateShapesTool.h"
#include "KInteractionTool_p.h"
#include "KoPointerEvent.h"
#include "KInteractionStrategy.h"
#include "KoCreateShapeStrategy_p.h"

#include <QMouseEvent>
#include <QPainter>

class KoCreateShapesToolPrivate : public KoInteractionToolPrivate
{
public:
    KoCreateShapesToolPrivate(KoToolBase *qq, KoCanvasBase *canvas)
        : KoInteractionToolPrivate(qq, canvas),
        newShapeProperties(0)
    {
    }

    QString shapeId;
    KProperties *newShapeProperties;
};

KoCreateShapesTool::KoCreateShapesTool(KoCanvasBase *canvas)
    : KInteractionTool(*(new KoCreateShapesToolPrivate(this, canvas)))
{
}

KoCreateShapesTool::~KoCreateShapesTool()
{
}

void KoCreateShapesTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (currentStrategy())
        currentStrategy()->paint(painter, converter);
}

void KoCreateShapesTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KInteractionTool::mouseReleaseEvent(event);
    emit KoToolBase::done();
}

void KoCreateShapesTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    setCursor(Qt::ArrowCursor);
}

void KoCreateShapesTool::setShapeId(const QString &id)
{
    Q_D(KoCreateShapesTool);
    d->shapeId = id;
}

QString KoCreateShapesTool::shapeId() const
{
    Q_D(const KoCreateShapesTool);
    return d->shapeId;
}

void KoCreateShapesTool::setShapeProperties(KProperties *properties)
{
    Q_D(KoCreateShapesTool);
    d->newShapeProperties = properties;
}

KProperties const * KoCreateShapesTool::shapeProperties()
{
    Q_D(KoCreateShapesTool);
    return d->newShapeProperties;
}

KInteractionStrategy *KoCreateShapesTool::createStrategy(KoPointerEvent *event)
{
    return new KoCreateShapeStrategy(this, event->point);
}

