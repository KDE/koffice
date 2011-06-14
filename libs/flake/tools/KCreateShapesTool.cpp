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

#include "KCreateShapesTool.h"
#include "KInteractionTool_p.h"
#include "KoPointerEvent.h"
#include "KInteractionStrategy.h"
#include "KoCreateShapeStrategy_p.h"

#include <QMouseEvent>
#include <QPainter>

class KCreateShapesToolPrivate : public KInteractionToolPrivate
{
public:
    KCreateShapesToolPrivate(KoToolBase *qq, KCanvasBase *canvas)
        : KInteractionToolPrivate(qq, canvas),
        newShapeProperties(0)
    {
    }

    QString shapeId;
    KProperties *newShapeProperties;
};

KCreateShapesTool::KCreateShapesTool(KCanvasBase *canvas)
    : KInteractionTool(*(new KCreateShapesToolPrivate(this, canvas)))
{
}

KCreateShapesTool::~KCreateShapesTool()
{
}

void KCreateShapesTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (currentStrategy())
        currentStrategy()->paint(painter, converter);
}

void KCreateShapesTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KInteractionTool::mouseReleaseEvent(event);
    emit KoToolBase::done();
}

void KCreateShapesTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    setCursor(Qt::ArrowCursor);
}

void KCreateShapesTool::setShapeId(const QString &id)
{
    Q_D(KCreateShapesTool);
    d->shapeId = id;
}

QString KCreateShapesTool::shapeId() const
{
    Q_D(const KCreateShapesTool);
    return d->shapeId;
}

void KCreateShapesTool::setShapeProperties(KProperties *properties)
{
    Q_D(KCreateShapesTool);
    d->newShapeProperties = properties;
}

KProperties const * KCreateShapesTool::shapeProperties()
{
    Q_D(KCreateShapesTool);
    return d->newShapeProperties;
}

KInteractionStrategy *KCreateShapesTool::createStrategy(KoPointerEvent *event)
{
    return new KoCreateShapeStrategy(this, event->point);
}

