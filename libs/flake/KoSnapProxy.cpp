/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoSnapProxy_p.h"
#include "KoSnapGuide.h"
#include "KCanvasBase.h"
#include "KoShapeManager.h"
#include "KPathShape.h"
#include "KPathPoint.h"

KoSnapProxy::KoSnapProxy(KoSnapGuide *snapGuide)
        : m_snapGuide(snapGuide)
{
}

QList<QPointF> KoSnapProxy::pointsInRect(const QRectF &rect)
{
    QList<QPointF> points;
    QList<KShape*> shapes = shapesInRect(rect);
    foreach(KShape *shape, shapes) {
        foreach(const QPointF &point, pointsFromShape(shape)) {
            if (rect.contains(point))
                points.append(point);
        }
    }

    return points;
}

QList<KShape*> KoSnapProxy::shapesInRect(const QRectF &rect, bool omitEditedShape)
{
    QList<KShape*> shapes = m_snapGuide->canvas()->shapeManager()->shapesAt(rect);
    foreach(KShape * shape, m_snapGuide->ignoredShapes()) {
        int index = shapes.indexOf(shape);
        if (index >= 0)
            shapes.removeAt(index);
    }
    if (! omitEditedShape && m_snapGuide->editedShape()) {
        QRectF bound = m_snapGuide->editedShape()->boundingRect();
        if (rect.intersects(bound) || rect.contains(bound))
            shapes.append(m_snapGuide->editedShape());
    }
    return shapes;
}

QList<QPointF> KoSnapProxy::pointsFromShape(KShape *shape)
{
    QList<QPointF> snapPoints;
    // no snapping to hidden shapes
    if (! shape->isVisible(true))
        return snapPoints;

    KPathShape *path = dynamic_cast<KPathShape*>(shape);
    if (path) {
        QTransform m = path->absoluteTransformation(0);

        QList<KPathPoint*> ignoredPoints = m_snapGuide->ignoredPathPoints();

        int subpathCount = path->subpathCount();
        for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
            int pointCount = path->subpathPointCount(subpathIndex);
            for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
                KPathPoint *p = path->pointByIndex(KoPathPointIndex(subpathIndex, pointIndex));
                if (! p || ignoredPoints.contains(p))
                    continue;

                snapPoints.append(m.map(p->point()));
            }
        }
    } else {
        // add the bounding box corners as default snap points
        QRectF bbox = shape->boundingRect();
        snapPoints.append(bbox.topLeft());
        snapPoints.append(bbox.topRight());
        snapPoints.append(bbox.bottomRight());
        snapPoints.append(bbox.bottomLeft());
    }

    return snapPoints;
}

QList<KPathSegment> KoSnapProxy::segmentsInRect(const QRectF &rect)
{
    QList<KShape*> shapes = shapesInRect(rect, true);
    QList<KPathPoint*> ignoredPoints = m_snapGuide->ignoredPathPoints();

    QList<KPathSegment> segments;
    foreach (KShape *shape, shapes) {
        QList<KPathSegment> shapeSegments;
        QRectF rectOnShape = shape->documentToShape(rect);
        KPathShape *path = dynamic_cast<KPathShape*>(shape);
        if (path) {
            shapeSegments = path->segmentsAt(rectOnShape);
        }

        QTransform m = shape->absoluteTransformation(0);
        // transform segments to document coordinates
        foreach (const KPathSegment &s, shapeSegments) {
            if (ignoredPoints.contains(s.first()) || ignoredPoints.contains(s.second()))
                continue;
            segments.append(s.mapped(m));
        }
    }
    return segments;
}

QList<KShape*> KoSnapProxy::shapes(bool omitEditedShape)
{
    QList<KShape*> allShapes = m_snapGuide->canvas()->shapeManager()->shapes();
    QList<KShape*> filteredShapes;
    QList<KShape*> ignoredShapes = m_snapGuide->ignoredShapes();

    // filter all hidden and ignored shapes
    foreach (KShape *shape, allShapes) {
        if (!shape->isVisible(true))
            continue;
        if (ignoredShapes.contains(shape))
            continue;

        filteredShapes.append(shape);
    }
    if (!omitEditedShape && m_snapGuide->editedShape())
        filteredShapes.append(m_snapGuide->editedShape());

    return filteredShapes;
}

KCanvasBase *KoSnapProxy::canvas()
{
    return m_snapGuide->canvas();
}
