/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KoShapeConnection.h"
#include "KoShape.h"
#include "KoViewConverter.h"

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QList>
#include <QPainterPath>

class KoShapeConnection::Private {
public:
    Private(KoShape *from, int gp1, KoShape *to, int gp2)
    : shape1(from),
    shape2(to),
    gluePointIndex1(gp1),
    gluePointIndex2(gp2)
    {
        Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);
        Q_ASSERT(shape2 == 0 || shape2->connectionPoints().count() > gp2);

        zIndex = shape1->zIndex() + 1;
        if(shape2)
            zIndex = qMax(zIndex, shape2->zIndex() + 1);
    }

    Private(KoShape *from, int gp1, const QPointF& ep)
    : shape1(from),
    shape2(0),
    gluePointIndex1(gp1),
    gluePointIndex2(0),
    endPoint (ep)
    {
        Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);

        zIndex = shape1->zIndex() + 1;
    }

    KoShape * const shape1;
    KoShape * shape2;
    int gluePointIndex1;
    int gluePointIndex2;
    QPointF endPoint;
    int zIndex;
    QList<QPointF> controlPoints;
};

KoShapeConnection::KoShapeConnection(KoShape *from, int gp1, KoShape *to, int gp2)
    : d(new Private(from, gp1, to, gp2))
{
    d->shape1->addConnection(this);
    d->shape2->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape* from, int gluePointIndex, const QPointF& endPoint)
: d(new Private(from, gluePointIndex, endPoint))
{
    d->shape1->addConnection(this);
}

KoShapeConnection::~KoShapeConnection() {
    d->shape1->removeConnection(this);
    d->shape2->removeConnection(this);
    delete d;
}

void KoShapeConnection::paint(QPainter &painter, const KoViewConverter &converter) {
    double x, y;
    converter.zoom(&x, &y);
    QTransform matrix = d->shape1->absoluteTransformation(&converter);
    matrix.scale(x, y);
    QPointF a = matrix.map(d->shape1->connectionPoints()[d->gluePointIndex1]);
    QPointF b;
    if(d->shape2) {
        matrix = d->shape2->absoluteTransformation(&converter);
        matrix.scale(x, y);
        b = matrix.map(d->shape2->connectionPoints()[d->gluePointIndex2]);
    }
    else
        b = converter.documentToView(d->endPoint);

    QPainterPath path;
    path.moveTo(a);

    foreach(QPointF point, d->controlPoints)
    {
        path.lineTo(converter.documentToView(point));
    }

    path.lineTo(b);

    QPen pen(Qt::black);
    painter.setPen(pen);
    painter.drawPath(path);
}

KoShape *KoShapeConnection::shape1() const {
    return d->shape1;
}

KoShape *KoShapeConnection::shape2() const {
    return d->shape2;
}

int KoShapeConnection::zIndex() const {
    return d->zIndex;
}

void KoShapeConnection::setZIndex(int index) {
    d->zIndex = index;
}

int KoShapeConnection::gluePointIndex1() const {
    return d->gluePointIndex1;
}

int KoShapeConnection::gluePointIndex2() const {
    return d->gluePointIndex2;
}

QPointF KoShapeConnection::startPoint() const {
    return d->shape1->absoluteTransformation(0).map(d->shape1->connectionPoints()[d->gluePointIndex1]);
}

QPointF KoShapeConnection::endPoint() const {
    if(d->shape2)
        return d->shape2->absoluteTransformation(0).map(d->shape2->connectionPoints()[d->gluePointIndex2]);
    return d->endPoint;
}

QRectF KoShapeConnection::boundingRect() const {
    QPointF start = startPoint();
    QPointF end = endPoint();
    QPointF topLeft(qMin(start.x(), end.x()), qMin(start.y(), end.y()));
    QPointF bottomRight(qMax(start.x(), end.x()), qMax(start.y(), end.y()));

    foreach(QPointF point, d->controlPoints)
    {
        topLeft.setX(qMin(topLeft.x(), point.x()));
        topLeft.setY(qMin(topLeft.y(), point.y()));
        bottomRight.setX(qMax(bottomRight.x(), point.x()));
        bottomRight.setY(qMax(bottomRight.y(), point.y()));
    }

    QRectF br(topLeft.x(), topLeft.y(), bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());
    return br.normalized();
}

void KoShapeConnection::setEndPoint(const QPointF& point)
{
    d->endPoint = point;
}

void KoShapeConnection::setEndPoint(KoShape* shape, int gluePointIndex)
{
    if(!shape)
        return;

    if(d->shape2)
    {
        d->shape2->removeConnection(this);
    }

    d->shape2 = shape;
    d->gluePointIndex2 = gluePointIndex;
    d->zIndex = qMax(d->zIndex, shape->zIndex() + 1);
    d->shape2->addConnection(this);
}

void KoShapeConnection::appendControlPoint(const QPointF& point)
{
    d->controlPoints.append(point);
}

//static
bool KoShapeConnection::compareConnectionZIndex(KoShapeConnection *c1, KoShapeConnection *c2) {
    return c1->zIndex() < c2->zIndex();
}
