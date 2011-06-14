/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>

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

#include "KParameterShape.h"
#include "KParameterShape_p.h"

#include <QPainter>
#include <KDebug>

KParameterShape::KParameterShape()
    : KoPathShape(*(new KoParameterShapePrivate(this)))
{
}

KParameterShape::KParameterShape(KoParameterShapePrivate &dd)
    : KoPathShape(dd)
{
}

KParameterShape::~KParameterShape()
{
}

void KParameterShape::moveHandle(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers)
{
    Q_D(KParameterShape);
    if (handleId >= d->handles.size()) {
        kWarning(30006) << "handleId out of bounds";
        return;
    }

    update();
    // function to do special stuff
    moveHandleAction(handleId, documentToShape(point), modifiers);

    updatePath(size());
    update();
    d->shapeChanged(ParameterChanged);
}


int KParameterShape::handleIdAt(const QRectF & rect) const
{
    Q_D(const KParameterShape);
    int handle = -1;

    for (int i = 0; i < d->handles.size(); ++i) {
        if (rect.contains(d->handles.at(i))) {
            handle = i;
            break;
        }
    }
    return handle;
}

QPointF KParameterShape::handlePosition(int handleId)
{
    Q_D(KParameterShape);
    return d->handles.value(handleId);
}

void KParameterShape::paintHandles(QPainter & painter, const KoViewConverter & converter, int handleRadius)
{
    Q_D(KParameterShape);
    applyConversion(painter, converter);

    QTransform worldMatrix = painter.worldTransform();
    painter.setTransform(QTransform());

    QTransform matrix;
    matrix.rotate(45.0);
    QPolygonF poly(d->handleRect(QPointF(0, 0), handleRadius));
    poly = matrix.map(poly);

    QList<QPointF>::const_iterator it(d->handles.constBegin());
    for (; it != d->handles.constEnd(); ++it) {
        QPointF moveVector = worldMatrix.map(*it);
        poly.translate(moveVector.x(), moveVector.y());
        painter.drawPolygon(poly);
        poly.translate(-moveVector.x(), -moveVector.y());
    }
}

void KParameterShape::paintHandle(QPainter & painter, const KoViewConverter & converter, int handleId, int handleRadius)
{
    Q_D(KParameterShape);
    applyConversion(painter, converter);

    QTransform worldMatrix = painter.worldTransform();
    painter.setTransform(QTransform());

    QTransform matrix;
    matrix.rotate(45.0);
    QPolygonF poly(d->handleRect(QPointF(0, 0), handleRadius));
    poly = matrix.map(poly);
    poly.translate(worldMatrix.map(d->handles[handleId]));
    painter.drawPolygon(poly);
}

void KParameterShape::setSize(const QSizeF &newSize)
{
    Q_D(KParameterShape);
    QTransform matrix(resizeMatrix(newSize));

    for (int i = 0; i < d->handles.size(); ++i) {
        d->handles[i] = matrix.map(d->handles[i]);
    }

    KoPathShape::setSize(newSize);
}

QPointF KParameterShape::normalize()
{
    Q_D(KParameterShape);
    QPointF offset(KoPathShape::normalize());
    QTransform matrix;
    matrix.translate(-offset.x(), -offset.y());

    for (int i = 0; i < d->handles.size(); ++i) {
        d->handles[i] = matrix.map(d->handles[i]);
    }

    return offset;
}

bool KParameterShape::isParametricShape() const
{
    Q_D(const KParameterShape);
    return d->parametric;
}

void KParameterShape::setParametricShape(bool parametric)
{
    Q_D(KParameterShape);
    d->parametric = parametric;
    update();
}

QList<QPointF> KParameterShape::handles() const
{
    Q_D(const KParameterShape);
    return d->handles;
}

void KParameterShape::setHandles(const QList<QPointF> &handles)
{
    Q_D(KParameterShape);
    d->handles = handles;
}

int KParameterShape::handleCount() const
{
    Q_D(const KParameterShape);
    return d->handles.count();
}
