/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#include "KoShapeConnection_p.h"
#include "KoShape.h"
#include "KoShape_p.h"
#include "KoShapeBorderBase.h"
#include "KoViewConverter.h"
#include "KoPathShape.h"
#include "KoPathShape_p.h"
#include "KoPathPoint.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoUnit.h>

#include <KDebug>

#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QList>
#include <QPainterPath>

class ConnectStrategy {
  public:
    ConnectStrategy(KoShapeConnection::ConnectionType type)
        : m_type(type)
    {
    }
    virtual ~ConnectStrategy() { }
    KoShapeConnection::ConnectionType type() const { return m_type; }

    virtual void paint(QPainter &painter, const KoViewConverter &converter, const QPointF &point1, const QPointF &point2) = 0;
    virtual void setSkew(const QStringList &values) {
        Q_UNUSED(values);
    }
    virtual void saveOdf(KoShapeSavingContext &context) const = 0;

  private:
    const KoShapeConnection::ConnectionType m_type;
};

class ConnectLines : public ConnectStrategy {
  public:
    ConnectLines(KoShapeConnection::ConnectionType type)
        : ConnectStrategy(type) { }
    virtual ~ConnectLines() { }

    virtual void paint(QPainter &painter, const KoViewConverter &converter, const QPointF &point1, const QPointF &point2);
    virtual void setSkew(const QStringList &values);
    virtual void saveOdf(KoShapeSavingContext &context) const;

  private:
    // store stop points
};

class ConnectCurve : public ConnectStrategy {
  public:
    ConnectCurve() : ConnectStrategy(KoShapeConnection::Curve), dirty(true) { }

    virtual void paint(QPainter &painter, const KoViewConverter &converter, const QPointF &point1, const QPointF &point2);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    KoPathShape shape;
    bool dirty; // shape needs to be resized
};

KoShapeConnectionPrivate::KoShapeConnectionPrivate(KoShape *from, int gp1, KoShape *to, int gp2)
    : shape1(from),
    shape2(to),
    gluePointIndex1(gp1),
    gluePointIndex2(gp2),
    hasDummyShape(false),
    connectionStrategy(0)
{
    Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);
    Q_ASSERT(shape2 == 0 || shape2->connectionPoints().count() > gp2);

    zIndex = shape1->zIndex() + 1;
    if (shape2)
        zIndex = qMax(zIndex, shape2->zIndex() + 1);
}

KoShapeConnectionPrivate::KoShapeConnectionPrivate(KoShape *from, int gp1, const QPointF& ep)
    : shape1(from),
    shape2(0),
    gluePointIndex1(gp1),
    gluePointIndex2(0),
    endPoint(ep),
    hasDummyShape(false),
    connectionStrategy(0)
{
    Q_ASSERT(shape1 == 0 || shape1->connectionPoints().count() > gp1);

    zIndex = 0;
    if (shape1)
        zIndex = shape1->zIndex() + 1;
}

QPointF KoShapeConnectionPrivate::resolveStartPoint() const
{
    QPointF a(startPoint);
    if (!hasDummyShape && shape1) {
        QList<QPointF> points(shape1->connectionPoints());
        int index = qMin(gluePointIndex1, points.count()-1);
        a = shape1->absoluteTransformation(0).map(points[index]);
    }
    return a;
}

QPointF KoShapeConnectionPrivate::resolveEndPoint() const
{

    QPointF b(endPoint);
    if (shape2) {
        QList<QPointF> points(shape2->connectionPoints());
        int index = qMin(gluePointIndex2, points.count()-1);
        b = shape2->absoluteTransformation(0).map(points[index]);
    }
    return b;
}

void ConnectLines::paint(QPainter &painter, const KoViewConverter &converter, const QPointF &point1, const QPointF &point2)
{
    QPainterPath path;
    switch (type()) {
    case KoShapeConnection::EdgedLinesOutside:
        // TODO avoid bounding rects and find a connection.
    case KoShapeConnection::EdgedLines:
        // TODO shoot out a bit and then a direct line.
    case KoShapeConnection::Straight:
        path.moveTo(point1);
        path.lineTo(point2);
        break;
    default:
        Q_ASSERT(0); // parent should have created another strategy
    };

    QPen pen(Qt::black);
    painter.setPen(pen);
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    painter.drawPath(path);
}

void ConnectLines::setSkew(const QStringList &values)
{
    qDebug() << "skew.." << values;
}

void ConnectLines::saveOdf(KoShapeSavingContext &context) const
{
    // We should try to save the svg:d
}

void ConnectCurve::paint(QPainter &painter, const KoViewConverter &converter, const QPointF &point1, const QPointF &point2)
{
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    painter.translate(QPointF(qMin(point1.x(), point2.x()), qMin(point1.y(), point2.y()))
            - shape.outline().boundingRect().topLeft());

    if (dirty) {
        KoPathPoint *first = shape.pointByIndex(KoPathPointIndex(0, 0));
        KoPathPoint *last = shape.pointByIndex(KoPathPointIndex(0, shape.pointCount() - 1));

        if (!first || !last || first == last) {
            // TODO calculate a curve and fill the shape with it.
            return;
        }
        const qreal scaleX = (point1.x() - point2.x()) / (first->point().x() - last->point().x());
        const qreal scaleY = (point1.y() - point2.y()) / (first->point().y() - last->point().y());

        QTransform mapper;
        mapper.scale(qMax(scaleX, 1E-4), qMax(scaleY, 1E-4));
        static_cast<KoPathShapePrivate*>(shape.priv())->map(mapper);
        dirty = false;
    }

    qreal transparency = shape.transparency(true);
    if (transparency > 0.0) {
        painter.setOpacity(1.0-transparency);
    }

    painter.scale(1/zoomX, 1/zoomY); // reverse pixels->pt scale because shape will do that again
    painter.save();
    shape.paint(painter, converter);
    painter.restore();
    if (shape.border()) {
        painter.save();
        shape.border()->paint(&shape, painter, converter);
        painter.restore();
    }
}

void ConnectCurve::saveOdf(KoShapeSavingContext &context) const
{
    // write the path data
    context.xmlWriter().addAttribute("svg:d", shape.toString());
    //shape.saveOdfAttributes(context, KoPathShape::OdfViewbox);
    shape.saveOdfCommonChildElements(context);
}


////////

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
    if (d->connectionStrategy == 0)
        return;

    d->connectionStrategy->paint(painter, converter, d->resolveStartPoint(), d->resolveEndPoint());
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

void KoShapeConnection::setStartPoint(const QPointF &point)
{
    d->startPoint = point;
}

void KoShapeConnection::setEndPoint(const QPointF &point)
{
    d->endPoint = point;
}

void KoShapeConnection::setStartPoint(KoShape *shape, int gluePointIndex)
{
    if (d->shape1) {
        d->shape1->priv()->removeConnection(this);
    }

    d->shape1 = shape;
    if (shape) {
        d->gluePointIndex1 = gluePointIndex;
        d->zIndex = qMax(d->zIndex, shape->zIndex() + 1);
        d->shape1->priv()->addConnection(this);
    } else {
        d->gluePointIndex1 = -1;
    }
}

void KoShapeConnection::setEndPoint(KoShape *shape, int gluePointIndex)
{
    if (d->shape2) {
        d->shape2->priv()->removeConnection(this);
    }

    d->shape2 = shape;
    if (shape) {
        d->gluePointIndex2 = gluePointIndex;
        d->zIndex = qMax(d->zIndex, shape->zIndex() + 1);
        d->shape2->priv()->addConnection(this);
    } else {
        d->gluePointIndex2 = -1;
    }
}

bool KoShapeConnection::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    QString type = element.attributeNS(KoXmlNS::draw, "type", "standard");
    delete d->connectionStrategy;
    if (type == "lines") {
        d->connectionStrategy = new ConnectLines(EdgedLines);
    } else if (type == "line") {
        d->connectionStrategy = new ConnectLines(Straight);
    } else if (type == "curve") {
        ConnectCurve *curve = new ConnectCurve();
        curve->shape.loadOdf(element, context);
        d->connectionStrategy = curve;
    } else {
        d->connectionStrategy = new ConnectLines(EdgedLinesOutside);
    }

    // reset connection point indices
    d->gluePointIndex1 = -1;
    d->gluePointIndex2 = -1;
    // reset connected shapes
    d->shape1 = 0;
    d->shape2 = 0;

    d->hasDummyShape = true;
    if (element.hasAttributeNS(KoXmlNS::draw, "start-shape")) {
        d->gluePointIndex1 = element.attributeNS(KoXmlNS::draw, "start-glue-point", QString()).toInt();
        QString shapeId1 = element.attributeNS(KoXmlNS::draw, "start-shape", QString());
        d->shape1 = context.shapeById(shapeId1);
        if (d->shape1) {
            d->shape1->priv()->addConnection(this);
        } else {
//            context.updateShape(shapeId1, new KoConnectionShapeLoadingUpdater(this, KoConnectionShapeLoadingUpdater::First));
        }
        d->hasDummyShape = false;
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
        d->hasDummyShape = false;
    } else {
        qreal x = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x2"));
        qreal y = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y2"));
        d->endPoint = QPointF(x, y);
    }

    QString skew = element.attributeNS(KoXmlNS::draw, "line-skew");
    QStringList skewValues = skew.simplified().split(' ', QString::SkipEmptyParts);
    d->connectionStrategy->setSkew(skewValues);

    //KoTextOnShapeContainer::tryWrapShape(this, element, context);

    return true;
}

void KoShapeConnection::saveOdf(KoShapeSavingContext &context) const
{
    if (d->connectionStrategy == 0)
        return;

    context.xmlWriter().startElement("draw:connector");
    switch (d->connectionStrategy->type()) {
    case EdgedLines:
        context.xmlWriter().addAttribute("draw:type", "lines");
        break;
    case Straight:
        context.xmlWriter().addAttribute("draw:type", "line");
        break;
    case Curve:
        context.xmlWriter().addAttribute("draw:type", "curve");
        break;
    default:
        context.xmlWriter().addAttribute("draw:type", "standard");
        break;
    }

    if (!d->hasDummyShape && d->shape1) {
        context.xmlWriter().addAttribute("draw:start-shape", context.drawId(d->shape1));
        context.xmlWriter().addAttribute("draw:start-glue-point", d->gluePointIndex1);
    }
    if (d->shape2) {
        context.xmlWriter().addAttribute("draw:end-shape", context.drawId(d->shape2));
        context.xmlWriter().addAttribute("draw:end-glue-point", d->gluePointIndex2);
    }
    QPointF p(d->resolveStartPoint());
    context.xmlWriter().addAttributePt("svg:x1", p.x());
    context.xmlWriter().addAttributePt("svg:y1", p.y());
    p = d->resolveEndPoint();
    context.xmlWriter().addAttributePt("svg:x2", p.x());
    context.xmlWriter().addAttributePt("svg:y2", p.y());

    d->connectionStrategy->saveOdf(context);
    context.xmlWriter().endElement();
}

KoShapeConnectionPrivate *KoShapeConnection::priv()
{
    return d;
}

//static
bool KoShapeConnection::compareConnectionZIndex(KoShapeConnection *c1, KoShapeConnection *c2)
{
    return c1->zIndex() < c2->zIndex();
}

KoShapeConnection::ConnectionType KoShapeConnection::type() const
{
    if (d->connectionStrategy)
        return d->connectionStrategy->type();
    return EdgedLinesOutside;
}

void KoShapeConnection::setType(ConnectionType newType)
{
    if (type() == newType)
        return;
    delete d->connectionStrategy;
    if (newType == Curve)
        d->connectionStrategy = new ConnectCurve;
    else
        d->connectionStrategy = new ConnectLines(newType);
}
