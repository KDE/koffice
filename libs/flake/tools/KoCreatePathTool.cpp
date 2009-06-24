/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoCreatePathTool.h"
#include "KoSnapGuide.h"
#include "SnapGuideConfigWidget.h"

#include "KoPathShape.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "KoPointerEvent.h"
#include "KoLineBorder.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoShapeController.h"
#include "KoCanvasResourceProvider.h"
#include "KoParameterShape.h"
#include "commands/KoPathPointMergeCommand.h"

#include <QtGui/QPainter>

#ifndef NO_PIGMENT
#include <KoColor.h>
#endif

qreal squareDistance( const QPointF &p1, const QPointF &p2)
{
    qreal dx = p1.x()-p2.x();
    qreal dy = p1.y()-p2.y();
    return dx*dx + dy*dy;
}

KoCreatePathTool::KoCreatePathTool(KoCanvasBase * canvas)
        : KoTool(canvas)
        , m_shape(0)
        , m_activePoint(0)
        , m_firstPoint(0)
        , m_handleRadius(3)
        , m_mouseOverFirstPoint(false)
        , m_existingStartPoint(0)
        , m_existingEndPoint(0)
        , m_hoveredPoint(0)
{
}

KoCreatePathTool::~KoCreatePathTool()
{
}

void KoCreatePathTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_shape) {
        painter.save();
        painter.setMatrix(m_shape->absoluteTransformation(&converter) * painter.matrix());

        painter.save();
        m_shape->paint(painter, converter);
        painter.restore();
        if (m_shape->border()) {
            painter.save();
            m_shape->border()->paintBorder(m_shape, painter, converter);
            painter.restore();
        }

        KoShape::applyConversion(painter, converter);

        QRectF handle = handleRect(QPoint(0, 0));
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::white);   //TODO make configurable

        const bool firstPoint = (m_firstPoint == m_activePoint);
        const bool pointIsBeingDragged = m_activePoint->activeControlPoint1();
        if (pointIsBeingDragged || firstPoint) {
            const bool onlyPaintActivePoints = false;
            m_activePoint->paint(painter, m_handleRadius,
                                 KoPathPoint::ControlPoint1 | KoPathPoint::ControlPoint2,
                                 onlyPaintActivePoints);
        }

        // paint the first point

        // check if we have to color the first point
        if (m_mouseOverFirstPoint)
            painter.setBrush(Qt::red);   // //TODO make configurable
        else
            painter.setBrush(Qt::white);   //TODO make configurable

        m_firstPoint->paint(painter, m_handleRadius, KoPathPoint::Node);

        painter.restore();
    }

    if (m_hoveredPoint) {
        painter.save();
        painter.setMatrix(m_hoveredPoint->parent()->absoluteTransformation(&converter), true);
        KoShape::applyConversion(painter, converter);
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::white);   //TODO make configurable
        m_hoveredPoint->paint(painter, m_handleRadius, KoPathPoint::Node);
        painter.restore();
    }
    painter.save();
    KoShape::applyConversion(painter, converter);
    m_canvas->snapGuide()->paint(painter, converter);
    painter.restore();
}

void KoCreatePathTool::mousePressEvent(KoPointerEvent *event)
{
    if ((event->buttons() & Qt::RightButton) && m_shape) {
        // repaint the shape before removing the last point
        m_canvas->updateCanvas(m_shape->boundingRect());
        m_shape->removePoint(m_shape->pathPointIndex(m_activePoint));

        addPathShape();
        return;
    }

    if (m_shape) {
        // the path shape gets closed by clicking on the first point
        if (grabRect(m_firstPoint->point()).contains(event->point)) {
            m_activePoint->setPoint(m_firstPoint->point());
            m_shape->closeMerge();
            // we are closing the path, so reset the existing start path point
            m_existingStartPoint = 0;
            // finish path
            addPathShape();
        } else {
            m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());

            QPointF point = m_canvas->snapGuide()->snap(event->point, event->modifiers());

            // check whether we hit an start/end node of an existing path
            m_existingEndPoint = endPointAtPosition(point);
            if (m_existingEndPoint && m_existingEndPoint != m_existingStartPoint) {
                point = m_existingEndPoint->parent()->shapeToDocument(m_existingEndPoint->point());
                m_activePoint->setPoint(point);
                // finish path
                addPathShape();
            } else {
                m_activePoint->setPoint(point);
                m_canvas->updateCanvas(m_shape->boundingRect());
                m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
            }
        }
    } else {
        m_shape = new KoPathShape();
        m_shape->setShapeId(KoPathShapeId);
        KoLineBorder * border = new KoLineBorder(m_canvas->resourceProvider()->activeBorder());
#ifndef NO_PIGMENT
        border->setColor(m_canvas->resourceProvider()->foregroundColor().toQColor());
#else
        border->setColor(m_canvas->resourceProvider()->foregroundColor());
#endif
        m_shape->setBorder(border);
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
        QPointF point = m_canvas->snapGuide()->snap(event->point, event->modifiers());
        
        // check whether we hit an start/end node of an existing path
        m_existingStartPoint = endPointAtPosition(point);
        if (m_existingStartPoint) {
            point = m_existingStartPoint->parent()->shapeToDocument(m_existingStartPoint->point());
        }
        m_activePoint = m_shape->moveTo(point);
        // set the control points to be different from the default (0, 0)
        // to avoid a unnecessary big area being repainted
        m_activePoint->setControlPoint1(point);
        m_activePoint->setControlPoint2(point);
        // remove them immediately so we do not end up having control points on a line start point
        m_activePoint->removeControlPoint1();
        m_activePoint->removeControlPoint2();
        m_firstPoint = m_activePoint;
        m_canvas->updateCanvas(handleRect(point));
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());

        m_canvas->snapGuide()->setEditedShape(m_shape);
    }
}

void KoCreatePathTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);

    if (m_shape) {
        // the first click of the double click created a new point which has the be removed again
        m_shape->removePoint(m_shape->pathPointIndex(m_activePoint));

        addPathShape();
    }
}

void KoCreatePathTool::mouseMoveEvent(KoPointerEvent *event)
{
    KoPathPoint * endPoint = endPointAtPosition(event->point);
    if (m_hoveredPoint != endPoint) {
        if (m_hoveredPoint) {
            QPointF nodePos = m_hoveredPoint->parent()->shapeToDocument(m_hoveredPoint->point());
            m_canvas->updateCanvas(handleRect(nodePos));
        }
        m_hoveredPoint = endPoint;
        if (m_hoveredPoint) {
            QPointF nodePos = m_hoveredPoint->parent()->shapeToDocument(m_hoveredPoint->point());
            m_canvas->updateCanvas(handleRect(nodePos));
        }
    }
    
    if (! m_shape) {
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
        m_canvas->snapGuide()->snap(event->point, event->modifiers());
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());

        m_mouseOverFirstPoint = false;
        return;
    }

    m_mouseOverFirstPoint = grabRect(m_firstPoint->point()).contains(event->point);

    m_canvas->updateCanvas(m_shape->boundingRect());

    repaintActivePoint();
    if (event->buttons() & Qt::LeftButton) {
        m_activePoint->setControlPoint2(event->point);
        m_activePoint->setControlPoint1(m_activePoint->point() + (m_activePoint->point() - event->point));
        m_canvas->updateCanvas(m_shape->boundingRect());
        repaintActivePoint();
    } else {
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
        m_activePoint->setPoint(m_canvas->snapGuide()->snap(event->point, event->modifiers()));
        m_canvas->updateCanvas(m_shape->boundingRect());
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
    }
}

void KoCreatePathTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (! m_shape || (event->buttons() & Qt::RightButton))
        return;

    repaintActivePoint();
    m_activePoint = m_shape->lineTo(event->point);
    m_canvas->snapGuide()->setIgnoredPathPoints( (QList<KoPathPoint*>()<<m_activePoint) );
}

void KoCreatePathTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        emit done();
    else
        event->ignore();
}

void KoCreatePathTool::activate(bool temporary)
{
    Q_UNUSED(temporary);
    useCursor(Qt::ArrowCursor, true);

    // retrieve the actual global handle radius
    m_handleRadius = m_canvas->resourceProvider()->handleRadius();
    
    // reset snap guide
    m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
    m_canvas->snapGuide()->reset();
}

void KoCreatePathTool::deactivate()
{
    // reset snap guide
    m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
    m_canvas->snapGuide()->reset();
    
    if (m_shape) {
        m_canvas->updateCanvas(handleRect(m_firstPoint->point()));
        m_canvas->updateCanvas(m_shape->boundingRect());
        delete m_shape;
        m_shape = 0;
        m_firstPoint = 0;
        m_activePoint = 0;
        m_existingStartPoint = 0;
        m_existingEndPoint = 0;
        m_hoveredPoint = 0;
    }
}

void KoCreatePathTool::resourceChanged(int key, const QVariant & res)
{
    switch (key) {
    case KoCanvasResource::HandleRadius: {
        m_handleRadius = res.toUInt();
    }
    break;
    default:
        return;
    }
}

void KoCreatePathTool::addPathShape()
{
    m_shape->normalize();

    // reset snap guide
    m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
    m_canvas->snapGuide()->reset();

    // this is done so that nothing happens when the mouseReleaseEvent for the this event is received
    KoPathShape *pathShape = m_shape;
    m_shape = 0;

    KoPathShape * startShape = 0;
    KoPathShape * endShape = 0;
    
    if (connectPaths(pathShape, m_existingStartPoint, m_existingEndPoint)) {
        if (m_existingStartPoint)
            startShape = m_existingStartPoint->parent();
        if (m_existingEndPoint && m_existingEndPoint != m_existingStartPoint)
            endShape = m_existingEndPoint->parent();
    }
    
    QUndoCommand * cmd = m_canvas->shapeController()->addShape(pathShape);
    if (cmd) {
        KoSelection *selection = m_canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select(pathShape);
        if (startShape)
            m_canvas->shapeController()->removeShape(startShape, cmd);
        if (endShape && startShape != endShape)
            m_canvas->shapeController()->removeShape(endShape, cmd);
        m_canvas->addCommand(cmd);
    } else {
        m_canvas->updateCanvas(pathShape->boundingRect());
        delete pathShape;
    }

    m_existingStartPoint = 0;
    m_existingEndPoint = 0;
    m_hoveredPoint = 0;
}

QRectF KoCreatePathTool::handleRect(const QPointF &p)
{
    const KoViewConverter * converter = m_canvas->viewConverter();
    QRectF hr = converter->viewToDocument(QRectF(0, 0, 2*m_handleRadius, 2*m_handleRadius));
    hr.moveCenter( p );
    return hr;
}

QRectF KoCreatePathTool::grabRect(const QPointF &p)
{
    uint grabSize = 2*m_canvas->resourceProvider()->grabSensitivity();
    const KoViewConverter * converter = m_canvas->viewConverter();
    QRectF hr = converter->viewToDocument(QRectF(0, 0, grabSize, grabSize));
    hr.moveCenter( p );
    return hr;
}

void KoCreatePathTool::repaintActivePoint()
{
    const bool isFirstPoint = (m_activePoint == m_firstPoint);
    const bool pointIsBeeingDragged = m_activePoint->activeControlPoint1();

    if (!isFirstPoint && !pointIsBeeingDragged)
        return;

    QRectF rect = m_activePoint->boundingRect(false);

    // make sure that we have the second control point inside our
    // update rect, as KoPathPoint::boundingRect will not include
    // the second control point of the last path point if the path
    // is not closed
    const QPointF &point = m_activePoint->point();
    const QPointF &controlPoint = m_activePoint->controlPoint2();
    rect = rect.united(QRectF(point, controlPoint).normalized());

    // when paiting the fist point we want the
    // first control point to be painted as well
    if (isFirstPoint) {
        const QPointF &controlPoint = m_activePoint->controlPoint1();
        rect = rect.united(QRectF(point, controlPoint).normalized());
    }

    QPointF border = m_canvas->viewConverter()
                     ->viewToDocument(QPointF(m_handleRadius, m_handleRadius));

    rect.adjust(-border.x(), -border.y(), border.x(), border.y());
    m_canvas->updateCanvas(rect);
}

QMap<QString, QWidget *> KoCreatePathTool::createOptionWidgets()
{
    QMap<QString, QWidget *> map;
    SnapGuideConfigWidget *widget = new SnapGuideConfigWidget(m_canvas->snapGuide());
    map.insert(i18n("Snapping"), widget);
    return map;
}

KoPathPoint* KoCreatePathTool::endPointAtPosition( const QPointF &position )
{
    QRectF roi = grabRect(position);
    QList<KoShape *> shapes = m_canvas->shapeManager()->shapesAt(roi);
    
    KoPathPoint * nearestPoint = 0;
    qreal minDistance = HUGE_VAL;
    uint grabSensitivity = m_canvas->resourceProvider()->grabSensitivity();
    qreal maxDistance = m_canvas->viewConverter()->viewToDocumentX(grabSensitivity);
    
    foreach(KoShape *shape, shapes) {
        KoPathShape * path = dynamic_cast<KoPathShape*>(shape);
        if (!path)
            continue;
        KoParameterShape *paramShape = dynamic_cast<KoParameterShape*>(shape);
        if (paramShape && paramShape->isParametricShape())
            continue;
        
        KoPathPoint * p = 0;
        uint subpathCount = path->subpathCount();
        for (uint i = 0; i < subpathCount; ++i) {
            if (path->isClosedSubpath(i))
                continue;
            p = path->pointByIndex(KoPathPointIndex(i, 0));
            // check start of subpath
            qreal d = squareDistance(position, path->shapeToDocument(p->point()));
            if (d < minDistance && d < maxDistance) {
                nearestPoint = p;
                minDistance = d;
            }
            // check end of subpath
            p = path->pointByIndex(KoPathPointIndex(i, path->pointCountSubpath(i)-1));
            d = squareDistance(position, path->shapeToDocument(p->point()));
            if (d < minDistance && d < maxDistance) {
                nearestPoint = p;
                minDistance = d;
            }
        }
    }
    
    return nearestPoint;
}

bool KoCreatePathTool::connectPaths( KoPathShape *pathShape, KoPathPoint *pointAtStart, KoPathPoint *pointAtEnd )
{
    // at least one point must be valid
    if (!pointAtStart && !pointAtEnd)
        return false;
    // do not allow connecting to the same point twice
    if (pointAtStart == pointAtEnd)
        pointAtEnd = 0;
    
    // we have hit an existing path point on start/finish
    // what we now do is:
    // 1. combine the new created path with the ones we hit on start/finish
    // 2. merge the endpoints of the corresponding subpaths
    
    uint newPointCount = pathShape->pointCountSubpath(0);
    KoPathPointIndex newStartPointIndex(0, 0);
    KoPathPointIndex newEndPointIndex(0, newPointCount-1);
    KoPathPoint * newStartPoint = pathShape->pointByIndex(newStartPointIndex);
    KoPathPoint * newEndPoint = pathShape->pointByIndex(newEndPointIndex);
    
    KoPathShape * startShape = pointAtStart ? pointAtStart->parent() : 0;
    KoPathShape * endShape = pointAtEnd ? pointAtEnd->parent() : 0;
    
    // combine with the path we hit on start
    KoPathPointIndex startIndex(-1,-1);
    if (pointAtStart) {
        startIndex = startShape->pathPointIndex(pointAtStart);
        pathShape->combine(startShape);
        pathShape->moveSubpath(0, pathShape->subpathCount()-1);
    }
    // combine with the path we hit on finish
    KoPathPointIndex endIndex(-1,-1);
    if (pointAtEnd) {
        endIndex = endShape->pathPointIndex(pointAtEnd);
        if (endShape != startShape) {
            endIndex.first += pathShape->subpathCount();
            pathShape->combine(endShape);
        }
    }
    // do we connect twice to a single subpath ?
    bool connectToSingleSubpath = (startShape == endShape && startIndex.first == endIndex.first);
    
    if (startIndex.second == 0 && !connectToSingleSubpath) {
        pathShape->reverseSubpath(startIndex.first);
        startIndex.second = pathShape->pointCountSubpath(startIndex.first)-1;
    }
    if (endIndex.second > 0 && !connectToSingleSubpath) {
        pathShape->reverseSubpath(endIndex.first);
        endIndex.second = 0;
    }
    
    // after combining we have a path where with the subpaths in the follwing
    // order:
    // 1. the subpaths of the pathshape we started the new path at
    // 2. the subpath we just created
    // 3. the subpaths of the pathshape we finished the new path at
    
    // get the path points we want to merge, as these are not going to
    // change while merging
    KoPathPoint * existingStartPoint = pathShape->pointByIndex(startIndex);
    KoPathPoint * existingEndPoint = pathShape->pointByIndex(endIndex);
    
    // merge first two points
    if (existingStartPoint) {
        KoPathPointData pd1(pathShape, pathShape->pathPointIndex(existingStartPoint));
        KoPathPointData pd2(pathShape, pathShape->pathPointIndex(newStartPoint));
        KoPathPointMergeCommand cmd1(pd1, pd2);
        cmd1.redo();
    }
    // merge last two points
    if (existingEndPoint) {
        KoPathPointData pd3(pathShape, pathShape->pathPointIndex(newEndPoint));
        KoPathPointData pd4(pathShape, pathShape->pathPointIndex(existingEndPoint));
        KoPathPointMergeCommand cmd2(pd3, pd4);
        cmd2.redo();
    }
    
    return true;
}
