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
#include "KoShape_p.h"
#include "KoViewConverter.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QList>
#include <QPainterPath>

class KoShapeConnection::Private
{
public:
    Private(KoShape *from, int gp1, KoShape *to, int gp2)
        : shape1(from),
        shape2(to),
        gluePointIndex1(gp1),
        gluePointIndex2(gp2),
        connectionType(KoShapeConnection::Standard)
    {
        Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);
        Q_ASSERT(shape2 == 0 || shape2->connectionPoints().count() > gp2);

        zIndex = shape1->zIndex() + 1;
        if (shape2)
            zIndex = qMax(zIndex, shape2->zIndex() + 1);
    }

    Private(KoShape *from, int gp1, const QPointF& ep)
        : shape1(from),
        shape2(0),
        gluePointIndex1(gp1),
        gluePointIndex2(0),
        endPoint(ep),
        connectionType(KoShapeConnection::Standard)
    {
        Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);

        zIndex = 0;
        if (shape1)
            zIndex = shape1->zIndex() + 1;
    }

    KoShape *shape1;
    KoShape *shape2;
    int gluePointIndex1;
    int gluePointIndex2;
    QPointF endPoint;
    int zIndex;
    QList<QPointF> controlPoints;
    KoShapeConnection::ConnectionType connectionType;
};

KoShapeConnection::KoShapeConnection()
    : d(new Private(0, 0, QPointF(100, 100)))
{
}

KoShapeConnection::KoShapeConnection(KoShape *from, int gp1, KoShape *to, int gp2)
    : d(new Private(from, gp1, to, gp2))
{
    Q_ASSERT(from);
    Q_ASSERT(to);
    d->shape1->priv()->addConnection(this);
    d->shape2->priv()->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape* from, int gluePointIndex, const QPointF& endPoint)
: d(new Private(from, gluePointIndex, endPoint))
{
    Q_ASSERT(from);
    d->shape1->priv()->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape *from, KoShape *to, int gp2)
    : d(new Private(from, 0, to, gp2))
{
    Q_ASSERT(from);
    Q_ASSERT(to);
    d->shape1->priv()->addConnection(this);
    d->shape2->priv()->addConnection(this);
}

KoShapeConnection::~KoShapeConnection()
{
    if (d->shape1)
        d->shape1->priv()->removeConnection(this);
    if (d->shape2)
        d->shape2->priv()->removeConnection(this);
    delete d;
}

void KoShapeConnection::paint(QPainter &painter, const KoViewConverter &converter)
{
    qreal x, y;
    converter.zoom(&x, &y);
    QTransform matrix = d->shape1->absoluteTransformation(&converter);
    matrix.scale(x, y);
    QPointF a = matrix.map(d->shape1->connectionPoints()[d->gluePointIndex1]);
    QPointF b;
    if (d->shape2) {
        matrix = d->shape2->absoluteTransformation(&converter);
        matrix.scale(x, y);
        b = matrix.map(d->shape2->connectionPoints()[d->gluePointIndex2]);
    } else {
        b = converter.documentToView(d->endPoint);
    }

    QPainterPath path;
    path.moveTo(a);

    foreach (QPointF point, d->controlPoints) {
        path.lineTo(converter.documentToView(point));
    }

    path.lineTo(b);

    QPen pen(Qt::black);
    painter.setPen(pen);
    painter.drawPath(path);
}

KoShape *KoShapeConnection::shape1() const
{
    return d->shape1;
}

KoShape *KoShapeConnection::shape2() const
{
    return d->shape2;
}

int KoShapeConnection::zIndex() const
{
    return d->zIndex;
}

void KoShapeConnection::setZIndex(int index)
{
    d->zIndex = index;
}

int KoShapeConnection::gluePointIndex1() const
{
    return d->gluePointIndex1;
}

int KoShapeConnection::gluePointIndex2() const
{
    return d->gluePointIndex2;
}

QPointF KoShapeConnection::startPoint() const
{
    return d->shape1->absoluteTransformation(0).map(d->shape1->connectionPoints()[d->gluePointIndex1]);
}

QPointF KoShapeConnection::endPoint() const
{
    if (d->shape2)
        return d->shape2->absoluteTransformation(0).map(d->shape2->connectionPoints()[d->gluePointIndex2]);
    return d->endPoint;
}

QRectF KoShapeConnection::boundingRect() const
{
    QPointF start = startPoint();
    QPointF end = endPoint();
    QPointF topLeft(qMin(start.x(), end.x()), qMin(start.y(), end.y()));
    QPointF bottomRight(qMax(start.x(), end.x()), qMax(start.y(), end.y()));

    foreach (QPointF point, d->controlPoints) {
        topLeft.setX(qMin(topLeft.x(), point.x()));
        topLeft.setY(qMin(topLeft.y(), point.y()));
        bottomRight.setX(qMax(bottomRight.x(), point.x()));
        bottomRight.setY(qMax(bottomRight.y(), point.y()));
    }

    QRectF br(topLeft.x(), topLeft.y(), bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());
    return br.normalized();
}

void KoShapeConnection::setEndPoint(const QPointF &point)
{
    d->endPoint = point;
}

void KoShapeConnection::setEndPoint(KoShape *shape, int gluePointIndex)
{
    if (!shape)
        return;

    if (d->shape2) {
        d->shape2->priv()->removeConnection(this);
    }

    d->shape2 = shape;
    d->gluePointIndex2 = gluePointIndex;
    d->zIndex = qMax(d->zIndex, shape->zIndex() + 1);
    d->shape2->priv()->addConnection(this);
}

void KoShapeConnection::appendControlPoint(const QPointF &point)
{
    d->controlPoints.append(point);
}

//static
bool KoShapeConnection::compareConnectionZIndex(KoShapeConnection *c1, KoShapeConnection *c2)
{
    return c1->zIndex() < c2->zIndex();
}

bool KoShapeConnection::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    //loadOdfAttributes(element, context, OdfMandatories | OdfCommonChildElements | OdfAdditionalAttributes);

    QString type = element.attributeNS(KoXmlNS::draw, "type", "standard");
    if (type == "lines")
        d->connectionType = Lines;
    else if (type == "line")
        d->connectionType = Straight;
    else if (type == "curve")
        d->connectionType = Curve;
    else
        d->connectionType = Standard;

    // reset connection point indices
    d->gluePointIndex1 = -1;
    d->gluePointIndex2 = -1;
    // reset connected shapes
    d->shape1 = 0;
    d->shape2 = 0;

    if (element.hasAttributeNS(KoXmlNS::draw, "start-shape")) {
        d->gluePointIndex1 = element.attributeNS(KoXmlNS::draw, "start-glue-point", QString()).toInt();
        QString shapeId1 = element.attributeNS(KoXmlNS::draw, "start-shape", QString());
        d->shape1 = context.shapeById(shapeId1);
        if (d->shape1) {
            d->shape1->priv()->addConnection(this);
            QList<QPointF> connectionPoints = d->shape1->connectionPoints();
//           if (d->gluePointIndex1 < connectionPoints.count()) {
//               d->handles[StartHandle] = d->shape1->absoluteTransformation(0).map(connectionPoints[d->gluePointIndex1]);
//           }
        } else {
//            context.updateShape(shapeId1, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::First));
        }
    } else {
//       d->handles[StartHandle].setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x1", QString())));
//       d->handles[StartHandle].setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y1", QString())));
    }

    if (element.hasAttributeNS(KoXmlNS::draw, "end-shape")) {
        d->gluePointIndex2 = element.attributeNS(KoXmlNS::draw, "end-glue-point", "").toInt();
        QString shapeId2 = element.attributeNS(KoXmlNS::draw, "end-shape", "");
        d->shape2 = context.shapeById(shapeId2);
        if (d->shape2) {
            d->shape2->priv()->addConnection(this);
            QList<QPointF> connectionPoints = d->shape2->connectionPoints();
//           if (d->gluePointIndex2 < connectionPoints.count()) {
//               d->handles[EndHandle] = d->shape2->absoluteTransformation(0).map(connectionPoints[d->gluePointIndex2]);
//           }
        } else {
//            context.updateShape(shapeId2, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::Second));
        }
    } else {
//       d->handles[EndHandle].setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x2", QString())));
//       d->handles[EndHandle].setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y2", QString())));
    }

    QString skew = element.attributeNS(KoXmlNS::draw, "line-skew", QString());
    QStringList skewValues = skew.simplified().split(' ', QString::SkipEmptyParts);
    // TODO apply skew values once we support them

#if 0
    // load the path data if there is any
    d->hasCustomPath = element.hasAttributeNS(KoXmlNS::svg, "d");
    if (d->hasCustomPath) {
        KoPathShapeLoader loader(this);
        loader.parseSvg(element.attributeNS(KoXmlNS::svg, "d"), true);
        QRectF viewBox = loadOdfViewbox(element);
        if (viewBox.isEmpty()) {
            // there should be a viewBox to transform the path data
            // if there is none, use the bounding rectangle of the parsed path
            viewBox = outline().boundingRect();
        }
        // convert path to viewbox coordinates to have a bounding rect of (0,0 1x1)
        // which can later be fitted back into the target rect once we have all
        // the required information
        QTransform viewMatrix;
        viewMatrix.scale(viewBox.width() ? static_cast<qreal>(1.0) / viewBox.width() : 1.0,
                         viewBox.height() ? static_cast<qreal>(1.0) / viewBox.height() : 1.0);
        viewMatrix.translate(-viewBox.left(), -viewBox.top());
        d->map(viewMatrix);

        // trigger finishing the connections in case we have all data
        // otherwise it gets called again once the shapes we are
        // connected to are loaded
        finishLoadingConnection();
    } else {
        d->forceUpdate = true;
        updateConnections();
    }
#endif

    //KoTextOnShapeContainer::tryWrapShape(this, element, context);

    return true;
}
