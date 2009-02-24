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
#include "KoPointerEvent.h"
#include "KoLineBorder.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoShapeController.h"
#include "KoCanvasResourceProvider.h"

#include <KoColor.h>

#include <QtGui/QPainter>


KoCreatePathTool::KoCreatePathTool(KoCanvasBase * canvas)
        : KoTool(canvas)
        , m_shape(0)
        , m_activePoint(0)
        , m_firstPoint(0)
        , m_handleRadius(3)
        , m_mouseOverFirstPoint(false)
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
        if (handleRect(m_firstPoint->point()).contains(event->point)) {
            m_activePoint->setPoint(m_firstPoint->point());
            m_shape->closeMerge();
            addPathShape();
        } else {
            m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());

            m_activePoint->setPoint(m_canvas->snapGuide()->snap(event->point, event->modifiers()));

            m_canvas->updateCanvas(m_shape->boundingRect());
            m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
        }
    } else {
        m_shape = new KoPathShape();
        m_shape->setShapeId(KoPathShapeId);
        KoLineBorder * border = new KoLineBorder(m_canvas->resourceProvider()->activeBorder());
        border->setColor(m_canvas->resourceProvider()->foregroundColor().toQColor());
        m_shape->setBorder(border);
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
        const QPointF &point = m_canvas->snapGuide()->snap(event->point, event->modifiers());
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
        // the first click of the qreal click created a new point which has the be removed again
        m_shape->removePoint(m_shape->pathPointIndex(m_activePoint));

        addPathShape();
    }
}

void KoCreatePathTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (! m_shape) {
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());
        m_canvas->snapGuide()->snap(event->point, event->modifiers());
        m_canvas->updateCanvas(m_canvas->snapGuide()->boundingRect());

        m_mouseOverFirstPoint = false;
        return;
    }

    m_mouseOverFirstPoint = handleRect(m_firstPoint->point()).contains(event->point);

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
    m_canvas->snapGuide()->reset();
}

void KoCreatePathTool::deactivate()
{
    m_canvas->snapGuide()->reset();
    if (m_shape) {
        m_canvas->updateCanvas(handleRect(m_firstPoint->point()));
        m_canvas->updateCanvas(m_shape->boundingRect());
        delete m_shape;
        m_shape = 0;
        m_firstPoint = 0;
        m_activePoint = 0;
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

    m_canvas->snapGuide()->setEditedShape(0);

    // this is done so that nothing happens when the mouseReleaseEvent for the this event is received
    KoPathShape *pathShape = m_shape;
    m_shape = 0;

    QUndoCommand * cmd = m_canvas->shapeController()->addShape(pathShape);
    if (cmd) {
        KoSelection *selection = m_canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select(pathShape);

        m_canvas->addCommand(cmd);
    } else {
        m_canvas->updateCanvas(pathShape->boundingRect());
        delete pathShape;
    }
}

QRectF KoCreatePathTool::handleRect(const QPointF &p)
{
    QPointF border = m_canvas->viewConverter()
                     ->viewToDocument(QPointF(m_handleRadius, m_handleRadius));

    const qreal x = p.x() - border.x();
    const qreal y = p.y() - border.y();
    const qreal w = border.x() * 2;
    const qreal h = border.x() * 2;
    return QRectF(x, y, w, h);
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
