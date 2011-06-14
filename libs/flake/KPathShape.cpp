/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>

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

#include "KPathShape.h"
#include "KPathShape_p.h"
#include "KPathPoint.h"
#include "KPointGroup_p.h"
#include "KoShapeBorderBase.h"
#include "KoViewConverter.h"
#include "KPathShapeLoader.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeShadow.h"
#include "KoShapeBackground.h"
#include "KoShapeContainer.h"
#include "KFilterEffectStack.h"
#include "KoTextOnShapeContainer.h"

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KUnit.h>
#include <KOdfGenericStyle.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>

#include <KDebug>
#include <QtGui/QPainter>

#ifndef QT_NO_DEBUG
#include <qnumeric.h> // for qIsNaN
static bool qIsNaNPoint(const QPointF &p) {
    return qIsNaN(p.x()) || qIsNaN(p.y());
}
#endif

KPathShapePrivate::KPathShapePrivate(KPathShape *q)
    : KShapePrivate(q),
    fillRule(Qt::OddEvenFill)
{
}

QRectF KPathShapePrivate::handleRect(const QPointF &p, qreal radius) const
{
    return QRectF(p.x() - radius, p.y() - radius, 2*radius, 2*radius);
}

void KPathShapePrivate::applyViewboxTransformation(const KXmlElement &element)
{
    Q_Q(KPathShape);
    // apply viewbox transformation
    QRectF viewBox = q->loadOdfViewbox(element);
    if (! viewBox.isEmpty()) {
        // load the desired size
        QSizeF size;
        size.setWidth(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "width", QString())));
        size.setHeight(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "height", QString())));

        // load the desired position
        QPointF pos;
        pos.setX(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x", QString())));
        pos.setY(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y", QString())));

        // create matrix to transform original path data into desired size and position
        QTransform viewMatrix;
        viewMatrix.translate(-viewBox.left(), -viewBox.top());
        viewMatrix.scale(size.width() / viewBox.width(), size.height() / viewBox.height());
        viewMatrix.translate(pos.x(), pos.y());

        // transform the path data
        map(viewMatrix);
    }
}


/////////////////////////
KPathShape::KPathShape()
    :KShape(*(new KPathShapePrivate(this)))
{
}

KPathShape::KPathShape(KPathShapePrivate &dd)
    : KShape(dd)
{
}

KPathShape::~KPathShape()
{
    clear();
}

void KPathShape::saveOdf(KoShapeSavingContext & context) const
{
    Q_D(const KPathShape);
    context.xmlWriter().startElement("draw:path");
    saveOdfAttributes(context, OdfAllAttributes | OdfViewbox);

    context.xmlWriter().addAttribute("svg:d", toString());
    context.xmlWriter().addAttribute("koffice:nodeTypes", d->nodeTypes());

    saveOdfCommonChildElements(context);
    if (parent())
        parent()->saveOdfChildElements(context);
    context.xmlWriter().endElement();
}

bool KPathShape::loadOdf(const KXmlElement & element, KoShapeLoadingContext &context)
{
    Q_D(KPathShape);
    loadOdfAttributes(element, context, OdfMandatories | OdfAdditionalAttributes | OdfCommonChildElements);

    // first clear the path data from the default path
    clear();

    if (element.localName() == "line") {
        QPointF start;
        start.setX(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x1", "")));
        start.setY(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y1", "")));
        QPointF end;
        end.setX(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x2", "")));
        end.setY(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y2", "")));
        moveTo(start);
        lineTo(end);
    } else if (element.localName() == "polyline" || element.localName() == "polygon") {
        QString points = element.attributeNS(KOdfXmlNS::draw, "points").simplified();
        points.replace(',', ' ');
        points.remove('\r');
        points.remove('\n');
        bool firstPoint = true;
        const QStringList coordinateList = points.split(' ');
        for (QStringList::ConstIterator it = coordinateList.constBegin(); it != coordinateList.constEnd(); ++it) {
            QPointF point;
            point.setX((*it).toDouble());
            ++it;
            point.setY((*it).toDouble());
            if (firstPoint) {
                moveTo(point);
                firstPoint = false;
            } else
                lineTo(point);
        }
        if (element.localName() == "polygon")
            close();
    } else { // path loading
        KPathShapeLoader loader(this);
        loader.parseSvg(element.attributeNS(KOdfXmlNS::svg, "d"), true);
        d->loadNodeTypes(element);
    }

    d->applyViewboxTransformation(element);
    QPointF pos = normalize();
    setTransformation(QTransform());

    if (element.hasAttributeNS(KOdfXmlNS::svg, "x") || element.hasAttributeNS(KOdfXmlNS::svg, "y")) {
        pos.setX(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x", QString())));
        pos.setY(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y", QString())));
    }

    setPosition(pos);

    loadOdfAttributes(element, context, OdfTransformation);
    KoTextOnShapeContainer::tryWrapShape(this, element, context);

    return true;
}

QString KPathShape::saveStyle(KOdfGenericStyle &style, KoShapeSavingContext &context) const
{
    Q_D(const KPathShape);
    style.addProperty("svg:fill-rule", d->fillRule == Qt::OddEvenFill ? "evenodd" : "nonzero");

    return KShape::saveStyle(style, context);
}

void KPathShape::loadStyle(const KXmlElement & element, KoShapeLoadingContext &context)
{
    Q_D(KPathShape);
    KShape::loadStyle(element, context);

    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    if (styleStack.hasProperty(KOdfXmlNS::svg, "fill-rule")) {
        QString rule = styleStack.property(KOdfXmlNS::svg, "fill-rule");
        d->fillRule = rule == "nonzero" ?  Qt::WindingFill : Qt::OddEvenFill;
    }
}

QRectF KPathShape::loadOdfViewbox(const KXmlElement & element) const
{
    QRectF viewbox;

    QString data = element.attributeNS(KOdfXmlNS::svg, "viewBox");
    if (! data.isEmpty()) {
        data.replace(',', ' ');
        QStringList coordinates = data.simplified().split(' ', QString::SkipEmptyParts);
        if (coordinates.count() == 4) {
            viewbox.setRect(coordinates[0].toDouble(), coordinates[1].toDouble(),
                            coordinates[2].toDouble(), coordinates[3].toDouble());
        }
    }

    return viewbox;
}

void KPathShape::clear()
{
    foreach(KoSubpath *subpath, m_subpaths) {
        foreach(KPathPoint *point, *subpath)
            delete point;
        delete subpath;
    }
    m_subpaths.clear();
}

void KPathShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KPathShape);
    applyConversion(painter, converter);
    QPainterPath path(outline());
    path.setFillRule(d->fillRule);

    if (background())
        background()->paint(painter, path);
    //paintDebug(painter);
}

#ifndef NDEBUG
void KPathShapePrivate::paintDebug(QPainter &painter)
{
    Q_Q(KPathShape);
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    int i = 0;

    QPen pen(Qt::black);
    painter.save();
    painter.setPen(pen);
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            ++i;
            KPathPoint *point = (*it);
            QRectF r(point->point(), QSizeF(5, 5));
            r.translate(-2.5, -2.5);
            QPen pen(Qt::black);
            painter.setPen(pen);
            if (point->group()) {
                QBrush b(Qt::blue);
                painter.setBrush(b);
            } else if (point->activeControlPoint1() && point->activeControlPoint2()) {
                QBrush b(Qt::red);
                painter.setBrush(b);
            } else if (point->activeControlPoint1()) {
                QBrush b(Qt::yellow);
                painter.setBrush(b);
            } else if (point->activeControlPoint2()) {
                QBrush b(Qt::darkYellow);
                painter.setBrush(b);
            }
            painter.drawEllipse(r);
        }
    }
    painter.restore();
    kDebug(30006) << "nop =" << i;
}

void KPathShapePrivate::debugPath() const
{
    Q_Q(const KPathShape);
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            kDebug(30006) << "p:" << (*pathIt) << "," << *it << "," << (*it)->point() << "," << (*it)->properties() << "," << (*it)->group();
        }
    }
}
#endif

void KPathShape::paintPoints(QPainter &painter, const KoViewConverter &converter, int handleRadius)
{
    Q_D(KPathShape);
    applyConversion(painter, converter);

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());

    QRectF handle = converter.viewToDocument(d->handleRect(QPoint(0, 0), handleRadius));

    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it)
            (*it)->paint(painter, handleRadius, KPathPoint::Node);
    }
}

QPainterPath KPathShape::outline() const
{
    QPainterPath path;
    foreach(KoSubpath * subpath, m_subpaths) {
        KPathPoint * lastPoint = subpath->first();
        bool activeCP = false;
        foreach(KPathPoint * currPoint, *subpath) {
            KPathPoint::PointProperties currProperties = currPoint->properties();
            if (currPoint == subpath->first()) {
                if (currProperties & KPathPoint::StartSubpath) {
                    Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                    path.moveTo(currPoint->point());
                }
            } else if (activeCP && currPoint->activeControlPoint1()) {
                Q_ASSERT(!qIsNaNPoint(lastPoint->controlPoint2()));
                Q_ASSERT(!qIsNaNPoint(currPoint->controlPoint1()));
                Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                path.cubicTo(
                    lastPoint->controlPoint2(),
                    currPoint->controlPoint1(),
                    currPoint->point());
            } else if (activeCP || currPoint->activeControlPoint1()) {
                Q_ASSERT(!qIsNaNPoint(lastPoint->controlPoint2()));
                Q_ASSERT(!qIsNaNPoint(currPoint->controlPoint1()));
                path.quadTo(
                    activeCP ? lastPoint->controlPoint2() : currPoint->controlPoint1(),
                    currPoint->point());
            } else {
                Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                path.lineTo(currPoint->point());
            }
            if (currProperties & KPathPoint::CloseSubpath && currProperties & KPathPoint::StopSubpath) {
                // add curve when there is a curve on the way to the first point
                KPathPoint * firstPoint = subpath->first();
                Q_ASSERT(!qIsNaNPoint(firstPoint->point()));
                if (currPoint->activeControlPoint2() && firstPoint->activeControlPoint1()) {
                    path.cubicTo(
                        currPoint->controlPoint2(),
                        firstPoint->controlPoint1(),
                        firstPoint->point());
                }
                else if (currPoint->activeControlPoint2() || firstPoint->activeControlPoint1()) {
                    Q_ASSERT(!qIsNaNPoint(currPoint->point()));
                    Q_ASSERT(!qIsNaNPoint(currPoint->controlPoint1()));
                    path.quadTo(
                        currPoint->activeControlPoint2() ? currPoint->controlPoint2() : firstPoint->controlPoint1(),
                        firstPoint->point());
                }
                path.closeSubpath();
            }

            if (currPoint->activeControlPoint2()) {
                activeCP = true;
            } else {
                activeCP = false;
            }
            lastPoint = currPoint;
        }
    }
    return path;
}

QRectF KPathShape::boundingRect() const
{
    QRectF bb(outline().boundingRect());
    if (border()) {
        KoInsets inset;
        border()->borderInsets(inset);
        bb.adjust(-inset.left, -inset.top, inset.right, inset.bottom);
    }

    QTransform transform = absoluteTransformation(0);
    bb = transform.mapRect(bb);
    if (shadow()) {
        KoInsets insets;
        shadow()->insets(insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (filterEffectStack()) {
        QRectF clipRect = filterEffectStack()->clipRectForBoundingRect(QRectF(QPointF(), size()));
        bb |= transform.mapRect(clipRect);
    }
    return bb;
}

QSizeF KPathShape::size() const
{
    // don't call boundingRect here as it uses absoluteTransformation
    // which itself uses size() -> leads to infinite reccursion
    return outline().boundingRect().size();
}

void KPathShape::setSize(const QSizeF &newSize)
{
    Q_D(KPathShape);
    QTransform matrix(resizeMatrix(newSize));

    KShape::setSize(newSize);
    d->map(matrix);
}

QTransform KPathShape::resizeMatrix(const QSizeF & newSize) const
{
    QSizeF oldSize = size();
    if (oldSize.width() == 0.0) {
        oldSize.setWidth(0.000001);
    }
    if (oldSize.height() == 0.0) {
        oldSize.setHeight(0.000001);
    }

    QSizeF sizeNew(newSize);
    if (sizeNew.width() == 0.0) {
        sizeNew.setWidth(0.000001);
    }
    if (sizeNew.height() == 0.0) {
        sizeNew.setHeight(0.000001);
    }

    return QTransform(sizeNew.width() / oldSize.width(), 0, 0, sizeNew.height() / oldSize.height(), 0, 0);
}

KPathPoint * KPathShape::moveTo(const QPointF &p)
{
    KPathPoint * point = new KPathPoint(this, p, KPathPoint::StartSubpath | KPathPoint::StopSubpath);
    KoSubpath * path = new KoSubpath;
    path->push_back(point);
    m_subpaths.push_back(path);
    return point;
}

KPathPoint * KPathShape::lineTo(const QPointF &p)
{
    Q_D(KPathShape);
    if (m_subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }
    KPathPoint * point = new KPathPoint(this, p, KPathPoint::StopSubpath);
    KPathPoint * lastPoint = m_subpaths.last()->last();
    d->updateLast(&lastPoint);
    m_subpaths.last()->push_back(point);
    return point;
}

KPathPoint * KPathShape::curveTo(const QPointF &c1, const QPointF &c2, const QPointF &p)
{
    Q_D(KPathShape);
    if (m_subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }
    KPathPoint * lastPoint = m_subpaths.last()->last();
    d->updateLast(&lastPoint);
    lastPoint->setControlPoint2(c1);
    KPathPoint * point = new KPathPoint(this, p, KPathPoint::StopSubpath);
    point->setControlPoint1(c2);
    m_subpaths.last()->push_back(point);
    return point;
}

KPathPoint * KPathShape::curveTo(const QPointF &c, const QPointF &p)
{
    Q_D(KPathShape);
    if (m_subpaths.empty())
        moveTo(QPointF(0, 0));

    KPathPoint * lastPoint = m_subpaths.last()->last();
    d->updateLast(&lastPoint);
    lastPoint->setControlPoint2(c);
    KPathPoint * point = new KPathPoint(this, p, KPathPoint::StopSubpath);
    m_subpaths.last()->push_back(point);

    return point;
}

KPathPoint * KPathShape::arcTo(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle)
{
    if (m_subpaths.empty()) {
        moveTo(QPointF(0, 0));
    }

    KPathPoint * lastPoint = m_subpaths.last()->last();
    if (lastPoint->properties() & KPathPoint::CloseSubpath) {
        lastPoint = m_subpaths.last()->first();
    }
    QPointF startpoint(lastPoint->point());

    KPathPoint * newEndPoint = lastPoint;

    QPointF curvePoints[12];
    int pointCnt = arcToCurve(rx, ry, startAngle, sweepAngle, startpoint, curvePoints);
    for (int i = 0; i < pointCnt; i += 3) {
        newEndPoint = curveTo(curvePoints[i], curvePoints[i+1], curvePoints[i+2]);
    }
    return newEndPoint;
}

int KPathShape::arcToCurve(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle, const QPointF & offset, QPointF * curvePoints) const
{
    int pointCnt = 0;

    // check Parameters
    if (sweepAngle == 0)
        return pointCnt;
    if (sweepAngle > 360)
        sweepAngle = 360;
    else if (sweepAngle < -360)
        sweepAngle = - 360;

    if (rx == 0 || ry == 0) {
        //TODO
    }

    // split angles bigger than 90° so that it gives a good aproximation to the circle
    qreal parts = ceil(qAbs(sweepAngle / 90.0));

    qreal sa_rad = startAngle * M_PI / 180.0;
    qreal partangle = sweepAngle / parts;
    qreal endangle = startAngle + partangle;
    qreal se_rad = endangle * M_PI / 180.0;
    qreal sinsa = sin(sa_rad);
    qreal cossa = cos(sa_rad);
    qreal kappa = 4.0 / 3.0 * tan((se_rad - sa_rad) / 4);

    // startpoint is at the last point is the path but when it is closed
    // it is at the first point
    QPointF startpoint(offset);

    //center berechnen
    QPointF center(startpoint - QPointF(cossa * rx, -sinsa * ry));

    //kDebug(30006) <<"kappa" << kappa <<"parts" << parts;;

    for (int part = 0; part < parts; ++part) {
        // start tangent
        curvePoints[pointCnt++] = QPointF(startpoint - QPointF(sinsa * rx * kappa, cossa * ry * kappa));

        qreal sinse = sin(se_rad);
        qreal cosse = cos(se_rad);

        // end point
        QPointF endpoint(center + QPointF(cosse * rx, -sinse * ry));
        // end tangent
        curvePoints[pointCnt++] = QPointF(endpoint - QPointF(-sinse * rx * kappa, -cosse * ry * kappa));
        curvePoints[pointCnt++] = endpoint;

        // set the endpoint as next start point
        startpoint = endpoint;
        sinsa = sinse;
        cossa = cosse;
        endangle += partangle;
        se_rad = endangle * M_PI / 180.0;
    }

    return pointCnt;
}

void KPathShape::close()
{
    Q_D(KPathShape);
    if (m_subpaths.empty()) {
        return;
    }
    d->closeSubpath(m_subpaths.last());
}

void KPathShape::closeMerge()
{
    Q_D(KPathShape);
    if (m_subpaths.empty()) {
        return;
    }
    d->closeMergeSubpath(m_subpaths.last());
}

QPointF KPathShape::normalize()
{
    Q_D(KPathShape);
    QPointF tl(outline().boundingRect().topLeft());
    QTransform matrix;
    matrix.translate(-tl.x(), -tl.y());
    d->map(matrix);

    // keep the top left point of the object
    applyTransformation(matrix.inverted());

    return tl;
}

void KPathShapePrivate::map(const QTransform &matrix)
{
    Q_Q(KPathShape);
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            (*it)->map(matrix);
        }
    }
}

void KPathShapePrivate::updateLast(KPathPoint **lastPoint)
{
    Q_Q(KPathShape);
    // check if we are about to add a new point to a closed subpath
    if ((*lastPoint)->properties() & KPathPoint::StopSubpath
            && (*lastPoint)->properties() & KPathPoint::CloseSubpath) {
        // get the first point of the subpath
        KPathPoint *subpathStart = q->m_subpaths.last()->first();
        // clone the first point of the subpath...
        KPathPoint * newLastPoint = new KPathPoint(*subpathStart);
        // ... and make it a normal point
        newLastPoint->setProperties(KPathPoint::Normal);
        // make a point group of the first point and its clone
        KPointGroup * group = subpathStart->group();
        if (group == 0) {
            group = new KPointGroup();
            group->add(subpathStart);
        }
        group->add(newLastPoint);

        // now start a new subpath with the cloned start point
        KoSubpath *path = new KoSubpath;
        path->push_back(newLastPoint);
        q->m_subpaths.push_back(path);
        *lastPoint = newLastPoint;
    } else {
        // the subpath was not closed so the formerly last point
        // of the subpath is no end point anymore
        (*lastPoint)->unsetProperty(KPathPoint::StopSubpath);
    }
    (*lastPoint)->unsetProperty(KPathPoint::CloseSubpath);
}

QList<KPathPoint*> KPathShape::pointsAt(const QRectF &r)
{
    QList<KPathPoint*> result;

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            if (r.contains((*it)->point()))
                result.append(*it);
            else if ((*it)->activeControlPoint1() && r.contains((*it)->controlPoint1()))
                result.append(*it);
            else if ((*it)->activeControlPoint2() && r.contains((*it)->controlPoint2()))
                result.append(*it);
        }
    }
    return result;
}

QList<KPathSegment> KPathShape::segmentsAt(const QRectF &r)
{
    QList<KPathSegment> segments;
    int subpathCount = m_subpaths.count();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        KoSubpath * subpath = m_subpaths[subpathIndex];
        int pointCount = subpath->count();
        bool subpathClosed = isClosedSubpath(subpathIndex);
        for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
            if (pointIndex == (pointCount - 1) && ! subpathClosed)
                break;
            KPathSegment s(subpath->at(pointIndex), subpath->at((pointIndex + 1) % pointCount));
            QRectF controlRect = s.controlPointRect();
            if (! r.intersects(controlRect) && ! controlRect.contains(r))
                continue;
            QRectF bound = s.boundingRect();
            if (! r.intersects(bound) && ! bound.contains(r))
                continue;

            segments.append(s);
        }
    }
    return segments;
}

KoPathPointIndex KPathShape::pathPointIndex(const KPathPoint *point) const
{
    for (int subpathIndex = 0; subpathIndex < m_subpaths.size(); ++subpathIndex) {
        KoSubpath * subpath = m_subpaths.at(subpathIndex);
        for (int pointPos = 0; pointPos < subpath->size(); ++pointPos) {
            if (subpath->at(pointPos) == point) {
                return KoPathPointIndex(subpathIndex, pointPos);
            }
        }
    }
    return KoPathPointIndex(-1, -1);
}

KPathPoint * KPathShape::pointByIndex(const KoPathPointIndex &pointIndex) const
{
    Q_D(const KPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size())
        return 0;

    return subpath->at(pointIndex.second);
}

KPathSegment KPathShape::segmentByIndex(const KoPathPointIndex &pointIndex) const
{
    Q_D(const KPathShape);
    KPathSegment segment(0, 0);

    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath != 0 && pointIndex.second >= 0 && pointIndex.second < subpath->size()) {
        KPathPoint * point = subpath->at(pointIndex.second);
        int index = pointIndex.second;
        // check if we have a (closing) segment starting from the last point
        if ((index == subpath->size() - 1) && point->properties() & KPathPoint::CloseSubpath)
            index = 0;
        else
            ++index;

        if (index < subpath->size()) {
            segment = KPathSegment(point, subpath->at(index));
        }
    }
    return segment;
}

int KPathShape::pointCount() const
{
    int i = 0;
    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        i += (*pathIt)->size();
    }

    return i;
}

int KPathShape::subpathCount() const
{
    return m_subpaths.count();
}

int KPathShape::subpathPointCount(int subpathIndex) const
{
    Q_D(const KPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return -1;

    return subpath->size();
}

bool KPathShape::isClosedSubpath(int subpathIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return false;

    const bool firstClosed = subpath->first()->properties() & KPathPoint::CloseSubpath;
    const bool lastClosed = subpath->last()->properties() & KPathPoint::CloseSubpath;

    return firstClosed && lastClosed;
}

bool KPathShape::insertPoint(KPathPoint* point, const KoPathPointIndex &pointIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second > subpath->size())
        return false;

    KPathPoint::PointProperties properties = point->properties();
    properties &= ~KPathPoint::StartSubpath;
    properties &= ~KPathPoint::StopSubpath;
    properties &= ~KPathPoint::CloseSubpath;
    // check if new point starts subpath
    if (pointIndex.second == 0) {
        properties |= KPathPoint::StartSubpath;
        // subpath was closed
        if (subpath->last()->properties() & KPathPoint::CloseSubpath) {
            // keep the path closed
            properties |= KPathPoint::CloseSubpath;
        }
        // old first point does not start the subpath anymore
        subpath->first()->unsetProperty(KPathPoint::StartSubpath);
    }
    // check if new point stops subpath
    else if (pointIndex.second == subpath->size()) {
        properties |= KPathPoint::StopSubpath;
        // subpath was closed
        if (subpath->last()->properties() & KPathPoint::CloseSubpath) {
            // keep the path closed
            properties = properties | KPathPoint::CloseSubpath;
        }
        // old last point does not end subpath anymore
        subpath->last()->unsetProperty(KPathPoint::StopSubpath);
    }

    point->setProperties(properties);
    point->setParent(this);
    subpath->insert(pointIndex.second , point);
    return true;
}

KPathPoint * KPathShape::removePoint(const KoPathPointIndex &pointIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size())
        return 0;

    KPathPoint * point = subpath->takeAt(pointIndex.second);

    //don't do anything (not even crash), if there was only one point
    if (pointCount()==0) {
        return point;
    }
    // check if we removed the first point
    else if (pointIndex.second == 0) {
        // first point removed, set new StartSubpath
        subpath->first()->setProperty(KPathPoint::StartSubpath);
        // check if path was closed
        if (subpath->last()->properties() & KPathPoint::CloseSubpath) {
            // keep path closed
            subpath->first()->setProperty(KPathPoint::CloseSubpath);
        }
    }
    // check if we removed the last point
    else if (pointIndex.second == subpath->size()) { // use size as point is already removed
        // last point removed, set new StopSubpath
        subpath->last()->setProperty(KPathPoint::StopSubpath);
        // check if path was closed
        if (point->properties() & KPathPoint::CloseSubpath) {
            // keep path closed
            subpath->last()->setProperty(KPathPoint::CloseSubpath);
        }
    }

    return point;
}

bool KPathShape::breakAfter(const KoPathPointIndex &pointIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (!subpath || pointIndex.second < 0 || pointIndex.second > subpath->size() - 2
        || isClosedSubpath(pointIndex.first))
        return false;

    KoSubpath * newSubpath = new KoSubpath;

    int size = subpath->size();
    for (int i = pointIndex.second + 1; i < size; ++i) {
        newSubpath->append(subpath->takeAt(pointIndex.second + 1));
    }
    // now make the first point of the new subpath a starting node
    newSubpath->first()->setProperty(KPathPoint::StartSubpath);
    // the last point of the old subpath is now an ending node
    subpath->last()->setProperty(KPathPoint::StopSubpath);

    // insert the new subpath after the broken one
    m_subpaths.insert(pointIndex.first + 1, newSubpath);

    return true;
}

bool KPathShape::join(int subpathIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);
    KoSubpath *nextSubpath = d->subPath(subpathIndex + 1);

    if (!subpath || !nextSubpath || isClosedSubpath(subpathIndex)
            || isClosedSubpath(subpathIndex+1))
        return false;

    // the last point of the subpath does not end the subpath anymore
    subpath->last()->unsetProperty(KPathPoint::StopSubpath);
    // the first point of the next subpath does not start a subpath anymore
    nextSubpath->first()->unsetProperty(KPathPoint::StartSubpath);

    // append the second subpath to the first
    foreach(KPathPoint * p, *nextSubpath)
        subpath->append(p);

    // remove the nextSubpath from path
    m_subpaths.removeAt(subpathIndex + 1);

    // delete it as it is no longer possible to use it
    delete nextSubpath;

    return true;
}

bool KPathShape::moveSubpath(int oldSubpathIndex, int newSubpathIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(oldSubpathIndex);

    if (subpath == 0 || newSubpathIndex >= m_subpaths.size())
        return false;

    if (oldSubpathIndex == newSubpathIndex)
        return true;

    m_subpaths.removeAt(oldSubpathIndex);
    m_subpaths.insert(newSubpathIndex, subpath);

    return true;
}

KoPathPointIndex KPathShape::openSubpath(const KoPathPointIndex &pointIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (!subpath || pointIndex.second < 0 || pointIndex.second >= subpath->size()
            || !isClosedSubpath(pointIndex.first))
        return KoPathPointIndex(-1, -1);

    KPathPoint * oldStartPoint = subpath->first();
    // the old starting node no longer starts the subpath
    oldStartPoint->unsetProperty(KPathPoint::StartSubpath);
    // the old end node no longer closes the subpath
    subpath->last()->unsetProperty(KPathPoint::StopSubpath);

    // reorder the subpath
    for (int i = 0; i < pointIndex.second; ++i) {
        subpath->append(subpath->takeFirst());
    }
    // make the first point a start node
    subpath->first()->setProperty(KPathPoint::StartSubpath);
    // make the last point an end node
    subpath->last()->setProperty(KPathPoint::StopSubpath);

    return pathPointIndex(oldStartPoint);
}

KoPathPointIndex KPathShape::closeSubpath(const KoPathPointIndex &pointIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(pointIndex.first);

    if (!subpath || pointIndex.second < 0 || pointIndex.second >= subpath->size()
        || isClosedSubpath(pointIndex.first))
        return KoPathPointIndex(-1, -1);

    KPathPoint * oldStartPoint = subpath->first();
    // the old starting node no longer starts the subpath
    oldStartPoint->unsetProperty(KPathPoint::StartSubpath);
    // the old end node no longer ends the subpath
    subpath->last()->unsetProperty(KPathPoint::StopSubpath);

    // reorder the subpath
    for (int i = 0; i < pointIndex.second; ++i) {
        subpath->append(subpath->takeFirst());
    }
    subpath->first()->setProperty(KPathPoint::StartSubpath);
    subpath->last()->setProperty(KPathPoint::StopSubpath);

    d->closeSubpath(subpath);
    return pathPointIndex(oldStartPoint);
}

bool KPathShape::reverseSubpath(int subpathIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath == 0)
        return false;

    int size = subpath->size();
    for (int i = 0; i < size; ++i) {
        KPathPoint *p = subpath->takeAt(i);
        p->reverse();
        subpath->prepend(p);
    }

    // adjust the position dependent properties
    KPathPoint *first = subpath->first();
    KPathPoint *last = subpath->last();

    KPathPoint::PointProperties firstProps = first->properties();
    KPathPoint::PointProperties lastProps = last->properties();

    firstProps |= KPathPoint::StartSubpath;
    firstProps &= ~KPathPoint::StopSubpath;
    lastProps |= KPathPoint::StopSubpath;
    lastProps &= ~KPathPoint::StartSubpath;
    if (firstProps & KPathPoint::CloseSubpath) {
        firstProps |= KPathPoint::CloseSubpath;
        lastProps |= KPathPoint::CloseSubpath;
    }
    first->setProperties(firstProps);
    last->setProperties(lastProps);

    return true;
}

KoSubpath * KPathShape::removeSubpath(int subpathIndex)
{
    Q_D(KPathShape);
    KoSubpath *subpath = d->subPath(subpathIndex);

    if (subpath != 0)
        m_subpaths.removeAt(subpathIndex);

    return subpath;
}

bool KPathShape::addSubpath(KoSubpath * subpath, int subpathIndex)
{
    if (subpathIndex < 0 || subpathIndex > m_subpaths.size())
        return false;

    m_subpaths.insert(subpathIndex, subpath);

    return true;
}

bool KPathShape::combine(const KPathShape *path)
{
    if (! path)
        return false;

    QTransform pathMatrix = path->absoluteTransformation(0);
    QTransform myMatrix = absoluteTransformation(0).inverted();

    foreach(KoSubpath* subpath, path->m_subpaths) {
        KoSubpath *newSubpath = new KoSubpath();

        foreach(KPathPoint* point, *subpath) {
            KPathPoint *newPoint = new KPathPoint(*point);
            newPoint->map(pathMatrix);
            newPoint->map(myMatrix);
            newPoint->setParent(this);
            newSubpath->append(newPoint);
        }
        m_subpaths.append(newSubpath);
    }
    normalize();
    return true;
}

bool KPathShape::separate(QList<KPathShape*> & separatedPaths)
{
    if (! m_subpaths.size())
        return false;

    QTransform myMatrix = absoluteTransformation(0);

    foreach(KoSubpath* subpath, m_subpaths) {
        KPathShape *shape = new KPathShape();
        if (! shape) continue;

        shape->setBorder(border());
        shape->setShapeId(shapeId());

        KoSubpath *newSubpath = new KoSubpath();

        foreach(KPathPoint* point, *subpath) {
            KPathPoint *newPoint = new KPathPoint(*point);
            newPoint->map(myMatrix);
            newSubpath->append(newPoint);
        }
        shape->m_subpaths.append(newSubpath);
        shape->normalize();
        separatedPaths.append(shape);
    }
    return true;
}

void KPathShapePrivate::closeSubpath(KoSubpath *subpath)
{
    if (! subpath)
        return;

    subpath->last()->setProperty(KPathPoint::CloseSubpath);
    subpath->first()->setProperty(KPathPoint::CloseSubpath);
}

void KPathShapePrivate::closeMergeSubpath(KoSubpath *subpath)
{
    if (! subpath || subpath->size() < 2)
        return;

    KPathPoint * lastPoint = subpath->last();
    KPathPoint * firstPoint = subpath->first();

    // check if first and last points are coincident
    if (lastPoint->point() == firstPoint->point()) {
        // we are removing the current last point and
        // reuse its first control point if active
        firstPoint->setProperty(KPathPoint::StartSubpath);
        firstPoint->setProperty(KPathPoint::CloseSubpath);
        if (lastPoint->activeControlPoint1())
            firstPoint->setControlPoint1(lastPoint->controlPoint1());
        // remove last point
        delete subpath->takeLast();
        // the new last point closes the subpath now
        lastPoint = subpath->last();
        lastPoint->setProperty(KPathPoint::StopSubpath);
        lastPoint->setProperty(KPathPoint::CloseSubpath);
    } else {
        closeSubpath(subpath);
    }
}

KoSubpath *KPathShapePrivate::subPath(int subpathIndex) const
{
    Q_Q(const KPathShape);
    if (subpathIndex < 0 || subpathIndex >= q->m_subpaths.size())
        return 0;

    return q->m_subpaths.at(subpathIndex);
}

QString KPathShape::pathShapeId() const
{
    return KoPathShapeId;
}

QString KPathShape::toString(const QTransform &matrix) const
{
    QString d;

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        KPathPoint * lastPoint(*it);
        bool activeCP = false;
        for (; it != (*pathIt)->constEnd(); ++it) {
            // first point of subpath ?
            if (it == (*pathIt)->constBegin()) {
                if ((*it)->properties() & KPathPoint::StartSubpath) {
                    QPointF p = matrix.map((*it)->point());
                    d += QString("M%1 %2").arg(p.x()).arg(p.y());
                }
            }
            // end point of curve ?
            else if (activeCP || (*it)->activeControlPoint1()) {
                QPointF cp1 = matrix.map(activeCP ? lastPoint->controlPoint2() : lastPoint->point());
                QPointF cp2 = matrix.map((*it)->activeControlPoint1() ? (*it)->controlPoint1() : (*it)->point());
                QPointF p = matrix.map((*it)->point());
                d += QString("C%1 %2 %3 %4 %5 %6")
                     .arg(cp1.x()).arg(cp1.y())
                     .arg(cp2.x()).arg(cp2.y())
                     .arg(p.x()).arg(p.y());
            }
            // end point of line
            else {
                QPointF p = matrix.map((*it)->point());
                d += QString("L%1 %2").arg(p.x()).arg(p.y());
            }
            // last point closes subpath ?
            if ((*it)->properties() & KPathPoint::StopSubpath
                    && (*it)->properties() & KPathPoint::CloseSubpath) {
                // add curve when there is a curve on the way to the first point
                KPathPoint * firstPoint = (*pathIt)->first();
                if ((*it)->activeControlPoint2() || firstPoint->activeControlPoint1()) {
                    QPointF cp1 = matrix.map((*it)->activeControlPoint2() ? (*it)->controlPoint2() : (*it)->point());
                    QPointF cp2 = matrix.map(firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : (firstPoint)->point());
                    QPointF p = matrix.map(firstPoint->point());

                    d += QString("C%1 %2 %3 %4 %5 %6")
                         .arg(cp1.x()).arg(cp1.y())
                         .arg(cp2.x()).arg(cp2.y())
                         .arg(p.x()).arg(p.y());
                }
                d += QString("Z");
            }

            activeCP = (*it)->activeControlPoint2();
            lastPoint = *it;
        }
    }

    return d;
}

char nodeType(const KPathPoint * point)
{
    if (point->properties() & KPathPoint::IsSmooth) {
        return 's';
    }
    else if (point->properties() & KPathPoint::IsSymmetric) {
        return 'z';
    }
    else {
        return 'c';
    }
}

QString KPathShapePrivate::nodeTypes() const
{
    Q_Q(const KPathShape);
    QString types;
    KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
    for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            if (it == (*pathIt)->constBegin()) {
                types.append('c');
            }
            else {
                types.append(nodeType(*it));
            }

            if ((*it)->properties() & KPathPoint::StopSubpath
                && (*it)->properties() & KPathPoint::CloseSubpath) {
                KPathPoint * firstPoint = (*pathIt)->first();
                types.append(nodeType(firstPoint));
            }
        }
    }
    return types;
}

void updateNodeType(KPathPoint * point, const QChar & nodeType)
{
    if (nodeType == 's') {
        point->setProperty(KPathPoint::IsSmooth);
    }
    else if (nodeType == 'z') {
        point->setProperty(KPathPoint::IsSymmetric);
    }
}

void KPathShapePrivate::loadNodeTypes(const KXmlElement &element)
{
    Q_Q(KPathShape);
    if (element.hasAttributeNS(KOdfXmlNS::koffice, "nodeTypes")) {
        QString nodeTypes = element.attributeNS(KOdfXmlNS::koffice, "nodeTypes");
        QString::const_iterator nIt(nodeTypes.constBegin());
        KoSubpathList::const_iterator pathIt(q->m_subpaths.constBegin());
        for (; pathIt != q->m_subpaths.constEnd(); ++pathIt) {
            KoSubpath::const_iterator it((*pathIt)->constBegin());
            for (; it != (*pathIt)->constEnd(); ++it, nIt++) {
                // be sure not to crash if there are not enough nodes in nodeTypes
                if (nIt == nodeTypes.constEnd()) {
                    kWarning(30006) << "not enough nodes in koffice:nodeTypes";
                    return;
                }
                // the first node is always of type 'c'
                if (it != (*pathIt)->constBegin()) {
                    updateNodeType(*it, *nIt);
                }

                if ((*it)->properties() & KPathPoint::StopSubpath
                    && (*it)->properties() & KPathPoint::CloseSubpath) {
                    ++nIt;
                    updateNodeType((*pathIt)->first(), *nIt);
                }
            }
        }
    }
}

Qt::FillRule KPathShape::fillRule() const
{
    Q_D(const KPathShape);
    return d->fillRule;
}

void KPathShape::setFillRule(Qt::FillRule fillRule)
{
    Q_D(KPathShape);
    d->fillRule = fillRule;
}

KPathShape * KPathShape::createShapeFromPainterPath(const QPainterPath &path)
{
    KPathShape * shape = new KPathShape();

    int elementCount = path.elementCount();
    for (int i = 0; i < elementCount; i++) {
        QPainterPath::Element element = path.elementAt(i);
        switch (element.type) {
        case QPainterPath::MoveToElement:
            shape->moveTo(QPointF(element.x, element.y));
            break;
        case QPainterPath::LineToElement:
            shape->lineTo(QPointF(element.x, element.y));
            break;
        case QPainterPath::CurveToElement:
            shape->curveTo(QPointF(element.x, element.y),
                           QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y),
                           QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y));
            break;
        default:
            continue;
        }
    }

    shape->normalize();
    return shape;
}

bool KPathShape::hitTest(const QPointF &position) const
{
    if (parent() && parent()->isClipped(this) && ! parent()->hitTest(position))
        return false;

    QPointF point = absoluteTransformation(0).inverted().map(position);
    const QPainterPath outlinePath = outline();
    if (border()) {
        KoInsets insets;
        border()->borderInsets(insets);
        QRectF roi(QPointF(-insets.left, -insets.top), QPointF(insets.right, insets.bottom));
        roi.moveCenter(point);
        if (outlinePath.intersects(roi) || outlinePath.contains(roi))
            return true;
    } else {
        if (outlinePath.contains(point))
            return true;
    }

    // if there is no shadow we can as well just leave
    if (! shadow())
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation(0).inverted().map(position - shadow()->offset());

    return outlinePath.contains(point);
}
