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
#include <KoPathShape.h>
#include <KoUnit.h>

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QList>
#include <QPainterPath>

class KoShapeConnectionPrivate;

namespace {
    class ConnectStrategy {
      public:
        ConnectStrategy(KoShapeConnection::ConnectionType type)
            : m_type(type)
        {
        }
        virtual ~ConnectStrategy() { }
        KoShapeConnection::ConnectionType type() const { return m_type; }

        virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapeConnectionPrivate *d) = 0;
        virtual void setSkew(const QStringList &values) {
            Q_UNUSED(values);
        }

      private:
        KoShapeConnection::ConnectionType m_type;
    };

    class ConnectLines : public ConnectStrategy {
      public:
        ConnectLines(KoShapeConnection::ConnectionType type)
            : ConnectStrategy(type) { }
        virtual ~ConnectLines() { }

        virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapeConnectionPrivate *d);
        virtual void setSkew(const QStringList &values);

      private:
        // store stop points
    };

    class ConnectCurve : public ConnectStrategy {
      public:
        ConnectCurve()
            : ConnectStrategy(KoShapeConnection::Curve),
            m_shape(new KoPathShape) { }
        ~ConnectCurve() {
            delete m_shape;
        }

        virtual void paint(QPainter &painter, const KoViewConverter &converter, KoShapeConnectionPrivate *d);

      private:
        KoPathShape *m_shape;
    };
};

class KoShapeConnectionPrivate
{
public:
    KoShapeConnectionPrivate(KoShape *from, int gp1, KoShape *to, int gp2)
        : shape1(from),
        shape2(to),
        gluePointIndex1(gp1),
        gluePointIndex2(gp2),
        connectionStrategy(0)
    {
        Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);
        Q_ASSERT(shape2 == 0 || shape2->connectionPoints().count() > gp2);

        zIndex = shape1->zIndex() + 1;
        if (shape2)
            zIndex = qMax(zIndex, shape2->zIndex() + 1);
    }

    KoShapeConnectionPrivate(KoShape *from, int gp1, const QPointF& ep)
        : shape1(from),
        shape2(0),
        gluePointIndex1(gp1),
        gluePointIndex2(0),
        endPoint(ep),
        connectionStrategy(0)
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
    QPointF startPoint; // used if there is no shape1
    QPointF endPoint; // used if there is no shape2
    int zIndex;

    ConnectStrategy *connectionStrategy;
};

void ConnectLines::paint(QPainter &painter, const KoViewConverter &converter, KoShapeConnectionPrivate *d)
{
    qreal x, y;
    converter.zoom(&x, &y);

    QPointF a(d->startPoint);
    QPointF b(d->endPoint);
    if (d->shape1) {
        QList<QPointF> points(d->shape1->connectionPoints());
        int index = qMin(d->gluePointIndex1, points.count()-1);
        a = d->shape1->absoluteTransformation(&converter).map(points[index]);
    }
    if (d->shape2) {
        QList<QPointF> points(d->shape2->connectionPoints());
        int index = qMin(d->gluePointIndex2, points.count()-1);
        b = d->shape2->absoluteTransformation(&converter).map(points[index]);
    }

    QPainterPath path;
    path.moveTo(a);
    path.lineTo(b);

    QPen pen(Qt::black);
    painter.setPen(pen);
    painter.drawPath(path);
}
void ConnectLines::setSkew(const QStringList &values) {
    qDebug() << "skew.." << values;
}

void ConnectCurve::paint(QPainter &painter, const KoViewConverter &converter, KoShapeConnectionPrivate *d)
{
    // TODO
    m_shape->paint(painter, converter);
}

KoShapeConnection::KoShapeConnection()
    : d(new KoShapeConnectionPrivate(0, 0, QPointF(100, 100)))
{
}

KoShapeConnection::KoShapeConnection(KoShape *from, int gp1, KoShape *to, int gp2)
    : d(new KoShapeConnectionPrivate(from, gp1, to, gp2))
{
    Q_ASSERT(from);
    Q_ASSERT(to);
    d->shape1->priv()->addConnection(this);
    d->shape2->priv()->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape* from, int gluePointIndex, const QPointF& endPoint)
: d(new KoShapeConnectionPrivate(from, gluePointIndex, endPoint))
{
    Q_ASSERT(from);
    d->shape1->priv()->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape *from, KoShape *to, int gp2)
    : d(new KoShapeConnectionPrivate(from, 0, to, gp2))
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
    if (d->connectionStrategy)
        d->connectionStrategy->paint(painter, converter, d);
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
    if (d->shape1) {
        QList<QPointF> points(d->shape1->connectionPoints());
        int index = qMin(d->gluePointIndex1, points.count()-1);
        return d->shape1->absoluteTransformation(0).map(points[index]);
    }
    return d->startPoint;
}

QPointF KoShapeConnection::endPoint() const
{
    if (d->shape2) {
        QList<QPointF> points(d->shape2->connectionPoints());
        int index = qMin(d->gluePointIndex2, points.count()-1);
        return d->shape2->absoluteTransformation(0).map(points[index]);
    }
    return d->endPoint;
}

QRectF KoShapeConnection::boundingRect() const
{
    QPointF start = startPoint();
    QPointF end = endPoint();
    QPointF topLeft(qMin(start.x(), end.x()), qMin(start.y(), end.y()));
    QPointF bottomRight(qMax(start.x(), end.x()), qMax(start.y(), end.y()));

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

//static
bool KoShapeConnection::compareConnectionZIndex(KoShapeConnection *c1, KoShapeConnection *c2)
{
    return c1->zIndex() < c2->zIndex();
}

bool KoShapeConnection::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    //loadOdfAttributes(element, context, OdfMandatories | OdfCommonChildElements | OdfAdditionalAttributes);

    QString type = element.attributeNS(KoXmlNS::draw, "type", "standard");
    delete d->connectionStrategy;
    if (type == "lines") {
        d->connectionStrategy = new ConnectLines(EdgedLines);
    } else if (type == "line") {
        d->connectionStrategy = new ConnectLines(Straight);
    } else if (type == "curve") {
        d->connectionStrategy = new ConnectCurve();
    } else {
        d->connectionStrategy = new ConnectLines(EdgedLinesOutside);
    }

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
        } else {
//            context.updateShape(shapeId1, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::First));
        }
    } else {
        qreal x = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x1"));
        qreal y = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y1"));
        d->startPoint = QPointF(x, y);
    }

    if (element.hasAttributeNS(KoXmlNS::draw, "end-shape")) {
        d->gluePointIndex2 = element.attributeNS(KoXmlNS::draw, "end-glue-point", "").toInt();
        QString shapeId2 = element.attributeNS(KoXmlNS::draw, "end-shape", "");
        d->shape2 = context.shapeById(shapeId2);
        if (d->shape2) {
            d->shape2->priv()->addConnection(this);
        } else {
//            context.updateShape(shapeId2, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::Second));
        }
    } else {
        qreal x = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x2"));
        qreal y = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y2"));
        d->endPoint = QPointF(x, y);
    }

    QString skew = element.attributeNS(KoXmlNS::draw, "line-skew", QString());
    QStringList skewValues = skew.simplified().split(' ', QString::SkipEmptyParts);
    d->connectionStrategy->setSkew(skewValues);

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
