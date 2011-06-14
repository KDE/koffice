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
#include "KoShapeManager.h"
#include "KoShapeManager_p.h"
#include "KoShapeBorderBase.h"
#include "KoViewConverter.h"
#include "KPathShape.h"
#include "KPathShape_p.h"
#include "KPathPoint.h"
#include "KLoadingShapeUpdater.h"

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KUnit.h>

#include <KDebug>

#include <QPainter>
#include <QPainterPath>


class ConnectLines : public ConnectStrategy
{
  public:
    ConnectLines(KoShapeConnectionPrivate *qq, KoShapeConnection::ConnectionType type)
        : ConnectStrategy(qq, type) { }
    virtual ~ConnectLines() { }

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void setSkew(const QStringList &values);
    virtual void saveOdf(KoShapeSavingContext &context) const;
    virtual QRectF boundingRect() const {
        recalc();
        return m_path.boundingRect();
    }

    /// using the shape-connection policy we calculate a line to get some distance from the shape
    QLineF calculateShape1Fallout() const;
    /// using the shape-connection policy we calculate a line to get some distance from the shape
    QLineF calculateShape2Fallout() const;

  private:
    void recalc() const;

    // store stop points
    mutable QPainterPath m_path;
};

class ConnectCurve : public ConnectStrategy
{
  public:
    ConnectCurve(KoShapeConnectionPrivate *qq)
        : ConnectStrategy(qq, KoShapeConnection::Curve),
        needsResize(true)
    {
    }

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void saveOdf(KoShapeSavingContext &context) const;
    virtual QRectF boundingRect() const {
        QPointF start = q->q->startPoint();
        QPointF end = q->q->endPoint();
        QPointF topLeft(qMin(start.x(), end.x()), qMin(start.y(), end.y()));
        QPointF bottomRight(qMax(start.x(), end.x()), qMax(start.y(), end.y()));

        QRectF br(topLeft.x(), topLeft.y(), bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());
        return br.normalized();
    }
    KPathShape shape;

  private:
    bool needsResize;
    void recalc() const;
};

class ConnectionLoaderUpdater : public KLoadingShapeUpdater
{
public:
    enum Pos {
        First,
        Second
    };

    ConnectionLoaderUpdater(KoShapeConnection *connectionShape, Pos pos)
        : m_connectionShape(connectionShape), m_pos(pos)
    {
    }

    virtual void update(KoShape *shape) {
        if (m_pos == First) {
            m_connectionShape->setStartPoint(shape, m_connectionShape->gluePointIndex1());
        } else {
            m_connectionShape->setEndPoint(shape, m_connectionShape->gluePointIndex2());
        }
    }

private:
    KoShapeConnection *m_connectionShape;
    Pos m_pos;
};

KoShapeConnectionPrivate::KoShapeConnectionPrivate(KoShapeConnection *qq, KoShape *from, int gp1, KoShape *to, int gp2)
    : q(qq),
    shape1(from),
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

KoShapeConnectionPrivate::KoShapeConnectionPrivate(KoShapeConnection *qq, KoShape *from, int gp1, const QPointF &ep)
    : q(qq),
    shape1(from),
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

QPainterPath KoShapeConnectionPrivate::createConnectionPath(const QPointF &from, const QPointF &to) const
{
    KoShape *shape = shape1;
    if (shape == 0)
        shape = shape2;
    QPainterPath path;
    if (shape && !shape->priv()->shapeManagers.isEmpty()) {
        QPointF p1(from);
        if (shape1) {
            QLineF line(calculateShapeFalloutPrivate(p1, true));
            p1 = line.p2();
        }
        QPointF p2(to);
        bool first = true;
        if (shape2) {
            QLineF line(calculateShapeFalloutPrivate(p2, false));
            path.moveTo(line.p2());
            first = false;
            p2 = line.p1();
        }
        QPolygonF lines((*shape->priv()->shapeManagers.begin())->priv()->routeConnection(q, p1, p2));
        foreach (const QPointF &point, lines) {
            if (first)
                path.moveTo(point);
            else
                path. lineTo(point);
            first = false;
        }
        if (shape1)
            path.lineTo(from);
    }

    return path;
}

QLineF KoShapeConnectionPrivate::calculateShapeFalloutPrivate(const QPointF &begin, bool start) const
{
    QPointF a(begin);
    QPointF b(a);

    KoShape *shape = 0;
    KoShapeConnectionPolicy policy;
    if (start && !hasDummyShape && shape1) {
        shape = shape1;
        policy = shape->connectionPolicy(gluePointIndex1);
    } else if (!start && shape2) {
        shape = shape2;
        policy = shape->connectionPolicy(gluePointIndex2);
    }

    // TODO snap the line to the 'grid' (10 pt grid is hardcoded in shape manager now)
    if (shape) {
        switch (policy.escapeDirection()) {
        case KoFlake::EscapeAny: {
            QPointF shapeCenter = shape->absolutePosition();
            QLineF outward(shapeCenter, a);
            outward.setLength(outward.length() + 10);
            b = outward.p2();
            break;
        }
        case KoFlake::EscapeHorizontal: {
            QPointF shapeCenter = shape->absolutePosition();
            b.setX(b.x() + (shapeCenter.x() < b.x() ? 10 : -10));
            break;
        }
        case KoFlake::EscapeLeft:
            b.setX(b.x() - 10);
            break;
        case KoFlake::EscapeRight:
            b.setX(b.x() + 10);
            break;
        case KoFlake::EscapeVertical: {
            QPointF shapeCenter = shape->absolutePosition();
            b.setY(b.y() + (shapeCenter.y() < b.y() ? 10 : -10));
            break;
        }
        case KoFlake::EscapeUp:
            b.setY(b.y() - 10);
            break;
        case KoFlake::EscapeDown:
            b.setY(b.y() + 10);
            break;
        }
    }

    if (start)
        return QLineF(a, b);
    else
        return QLineF(b, a);
}

QLineF ConnectLines::calculateShape1Fallout() const
{
    return q->calculateShapeFalloutPrivate(q->resolveStartPoint(), true);
}

QLineF ConnectLines::calculateShape2Fallout() const
{
    return q->calculateShapeFalloutPrivate(q->resolveEndPoint(), false);
}

void ConnectLines::paint(QPainter &painter, const KoViewConverter &converter)
{
    recalc();
    QPen pen(Qt::black);
    painter.setPen(pen);
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    painter.drawPath(m_path);
}

void ConnectLines::recalc() const
{
    if (!m_dirty)
        return ;
    m_path = QPainterPath();
    QPointF point1(q->resolveStartPoint());
    QPointF point2(q->resolveEndPoint());
    switch (type()) {
    case KoShapeConnection::EdgedLinesOutside:
        m_path = q->createConnectionPath(point1, point2);
        break;
    case KoShapeConnection::EdgedLines: {
        // shoot out a bit and then a direct line.
        QLineF fallOut1(calculateShape1Fallout());
        m_path.moveTo(fallOut1.p1());
        m_path.lineTo(fallOut1.p2());
        QLineF fallOut2(calculateShape2Fallout());
        m_path.lineTo(fallOut2.p1());
        m_path.lineTo(fallOut2.p2());
        break;
    }
    case KoShapeConnection::Straight:
        m_path.moveTo(q->resolveStartPoint());
        m_path.lineTo(q->resolveEndPoint());
        break;
    default:
        Q_ASSERT(0); // parent should have created another strategy
    };
    m_dirty = m_path.isEmpty();
}

void ConnectLines::setSkew(const QStringList &values)
{
    qDebug() << "skew.." << values;
}

void ConnectLines::saveOdf(KoShapeSavingContext &context) const
{
    // We should try to save the svg:d
}

void ConnectCurve::paint(QPainter &painter, const KoViewConverter &converter)
{
    recalc();
    QPointF point1(q->resolveStartPoint());
    QPointF point2(q->resolveEndPoint());
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    painter.translate(QPointF(qMin(point1.x(), point2.x()), qMin(point1.y(), point2.y()))
            - shape.outline().boundingRect().topLeft());

    if (needsResize) {
        KPathPoint *first = shape.pointByIndex(KoPathPointIndex(0, 0));
        KPathPoint *last = shape.pointByIndex(KoPathPointIndex(0, shape.pointCount() - 1));

        if (!first || !last || first == last) {
            foul();
            return;
        }
        const qreal scaleX = (point1.x() - point2.x()) / (first->point().x() - last->point().x());
        const qreal scaleY = (point1.y() - point2.y()) / (first->point().y() - last->point().y());

        QTransform mapper;
        mapper.scale(qMax(scaleX, 1E-4), qMax(scaleY, 1E-4));
        static_cast<KPathShapePrivate*>(shape.priv())->map(mapper);
        needsResize = false;
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

void ConnectCurve::recalc() const
{
    // TODO calculate a curve and fill the shape with it.
    m_dirty = false;
}

void ConnectCurve::saveOdf(KoShapeSavingContext &context) const
{
    // write the path data
    context.xmlWriter().addAttribute("svg:d", shape.toString());
    //shape.saveOdfAttributes(context, KPathShape::OdfViewbox);
    shape.saveOdfCommonChildElements(context);
}


////////

KoShapeConnection::KoShapeConnection()
    : d(new KoShapeConnectionPrivate(this, 0, 0, QPointF(100, 100)))
{
}

KoShapeConnection::KoShapeConnection(KoShape *from, int gp1, KoShape *to, int gp2)
    : d(new KoShapeConnectionPrivate(this, from, gp1, to, gp2))
{
    Q_ASSERT(from);
    Q_ASSERT(to);
    d->shape1->priv()->addConnection(this);
    d->shape2->priv()->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape* from, int gluePointIndex, const QPointF &endPoint)
: d(new KoShapeConnectionPrivate(this, from, gluePointIndex, endPoint))
{
    Q_ASSERT(from);
    d->shape1->priv()->addConnection(this);
}

KoShapeConnection::KoShapeConnection(KoShape *from, KoShape *to, int gp2)
    : d(new KoShapeConnectionPrivate(this, from, 0, to, gp2))
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

    d->connectionStrategy->paint(painter, converter);
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
    d->foul();
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
    return d->connectionStrategy->boundingRect();
}

void KoShapeConnection::setStartPoint(const QPointF &point)
{
    d->startPoint = point;
    d->foul();
}

void KoShapeConnection::setEndPoint(const QPointF &point)
{
    d->endPoint = point;
    d->foul();
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
    d->foul();
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
    d->foul();
}

bool KoShapeConnection::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
{
    QString type = element.attributeNS(KOdfXmlNS::draw, "type", "standard");
    delete d->connectionStrategy;
    if (type == "lines") {
        d->connectionStrategy = new ConnectLines(d, EdgedLines);
    } else if (type == "line") {
        d->connectionStrategy = new ConnectLines(d, Straight);
    } else if (type == "curve") {
        ConnectCurve *curve = new ConnectCurve(d);
        curve->shape.loadOdf(element, context);
        curve->wipe();
        d->connectionStrategy = curve;
    } else {
        d->connectionStrategy = new ConnectLines(d, EdgedLinesOutside);
    }

    // reset connection point indices
    d->gluePointIndex1 = -1;
    d->gluePointIndex2 = -1;
    // reset connected shapes
    d->shape1 = 0;
    d->shape2 = 0;

    d->hasDummyShape = true;
    if (element.hasAttributeNS(KOdfXmlNS::draw, "start-shape")) {
        d->gluePointIndex1 = element.attributeNS(KOdfXmlNS::draw, "start-glue-point", QString()).toInt();
        QString shapeId1 = element.attributeNS(KOdfXmlNS::draw, "start-shape", QString());
        d->shape1 = context.shapeById(shapeId1);
        if (d->shape1) {
            d->shape1->priv()->addConnection(this);
        } else {
            context.updateShape(shapeId1,
                    new ConnectionLoaderUpdater(this, ConnectionLoaderUpdater::First));
        }
        d->hasDummyShape = false;
    } else {
        qreal x = KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x1"));
        qreal y = KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y1"));
        d->startPoint = QPointF(x, y);
    }

    if (element.hasAttributeNS(KOdfXmlNS::draw, "end-shape")) {
        d->gluePointIndex2 = element.attributeNS(KOdfXmlNS::draw, "end-glue-point", "").toInt();
        QString shapeId2 = element.attributeNS(KOdfXmlNS::draw, "end-shape", "");
        d->shape2 = context.shapeById(shapeId2);
        if (d->shape2) {
            d->shape2->priv()->addConnection(this);
        } else {
            context.updateShape(shapeId2,
                    new ConnectionLoaderUpdater(this, ConnectionLoaderUpdater::Second));
        }
        d->hasDummyShape = false;
    } else {
        qreal x = KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x2"));
        qreal y = KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y2"));
        d->endPoint = QPointF(x, y);
    }

    QString skew = element.attributeNS(KOdfXmlNS::draw, "line-skew");
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
        d->connectionStrategy = new ConnectCurve(d);
    else
        d->connectionStrategy = new ConnectLines(d, newType);
    d->foul();
}

void KoShapeConnection::update() const
{
    KoShape *shape = d->shape1;
    if (shape == 0)
        shape = d->shape2;
    if (shape == 0)
        return;
    QRectF mySize = boundingRect();
    foreach (KoShapeManager *shapeManager, shape->priv()->shapeManagers) {
        shapeManager->priv()->update(mySize, 0, false);
    }
}
