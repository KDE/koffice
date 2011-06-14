/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoPathToolHandle_p.h"
#include "KoPathTool_p.h"
#include "KoPathPointMoveStrategy_p.h"
#include "KPathControlPointMoveStrategy_p.h"
#include "KoSelection.h"
#include "commands/KoPathPointTypeCommand_p.h"
#include "KParameterChangeStrategy_p.h"
#include "KParameterShape.h"
#include "KCanvasBase.h"
#include "KoResourceManager.h"
#include "KoShapeManager.h"
#include "KoViewConverter.h"
#include "KoPointerEvent.h"
#include <QtGui/QPainter>

KoPathToolHandle::KoPathToolHandle(KoPathTool *tool)
        : m_tool(tool)
{
}

KoPathToolHandle::~KoPathToolHandle()
{
}

PointHandle::PointHandle(KoPathTool *tool, KPathPoint *activePoint, KPathPoint::PointType activePointType)
        : KoPathToolHandle(tool)
        , m_activePoint(activePoint)
        , m_activePointType(activePointType)
{
}

void PointHandle::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();
    painter.setTransform(m_activePoint->parent()->absoluteTransformation(&converter) * painter.transform());
    KoShape::applyConversion(painter, converter);

    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());

    KPathPoint::PointType type = KPathPoint::Node;
    if (selection && selection->contains(m_activePoint))
        type = KPathPoint::All;
    int handleRadius = m_tool->canvas()->resourceManager()->handleRadius();
    m_activePoint->paint(painter, handleRadius, type);
    painter.restore();
}

void PointHandle::repaint() const
{
    bool active = false;
    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
    if (selection && selection->contains(m_activePoint))
        active = true;
    m_tool->repaint(m_activePoint->boundingRect(!active));
}

KInteractionStrategy * PointHandle::handleMousePress(KoPointerEvent *event)
{
    if ((event->button() & Qt::LeftButton) == 0)
        return 0;
    if ((event->modifiers() & Qt::ShiftModifier) == 0) { // no shift pressed.
        KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());

        // control select adds/removes points to/from the selection
        if (event->modifiers() & Qt::ControlModifier) {
            if (selection->contains(m_activePoint)) {
                selection->remove(m_activePoint);
            } else {
                selection->add(m_activePoint, false);
            }
            m_tool->repaint(m_activePoint->boundingRect(false));
        } else {
            // no control modifier, so clear selection and select active point
            if (!selection->contains(m_activePoint)) {
                selection->add(m_activePoint, true);
                m_tool->repaint(m_activePoint->boundingRect(false));
            }
        }
        // TODO remove canvas from call ?
        if (m_activePointType == KPathPoint::Node) {
            QPointF startPoint = m_activePoint->parent()->shapeToDocument(m_activePoint->point());
            return new KoPathPointMoveStrategy(m_tool, startPoint);
        } else {
            KoPathShape * pathShape = m_activePoint->parent();
            KoPathPointData pd(pathShape, pathShape->pathPointIndex(m_activePoint));
            return new KPathControlPointMoveStrategy(m_tool, pd, m_activePointType, event->point);
        }
    } else {
        KPathPoint::PointProperties props = m_activePoint->properties();
        if (! m_activePoint->activeControlPoint1() || ! m_activePoint->activeControlPoint2())
            return 0;

        KoPathPointTypeCommand::PointType pointType = KoPathPointTypeCommand::Smooth;
        // cycle the smooth->symmetric->unsmooth state of the path point
        if (props & KPathPoint::IsSmooth)
            pointType = KoPathPointTypeCommand::Symmetric;
        else if (props & KPathPoint::IsSymmetric)
            pointType = KoPathPointTypeCommand::Corner;

        QList<KoPathPointData> pointData;
        pointData.append(KoPathPointData(m_activePoint->parent(), m_activePoint->parent()->pathPointIndex(m_activePoint)));
        m_tool->canvas()->addCommand(new KoPathPointTypeCommand(pointData, pointType));
    }
    return 0;
}

bool PointHandle::check(const QList<KoPathShape*> &selectedShapes)
{
    if (selectedShapes.contains(m_activePoint->parent())) {
        return m_activePoint->parent()->pathPointIndex(m_activePoint) != KoPathPointIndex(-1, -1);
    }
    return false;
}

KPathPoint * PointHandle::activePoint() const
{
    return m_activePoint;
}

KPathPoint::PointType PointHandle::activePointType() const
{
    return m_activePointType;
}

ParameterHandle::ParameterHandle(KoPathTool *tool, KParameterShape *parameterShape, int handleId)
        : KoPathToolHandle(tool)
        , m_parameterShape(parameterShape)
        , m_handleId(handleId)
{
}

void ParameterHandle::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();
    painter.setTransform(m_parameterShape->absoluteTransformation(&converter) * painter.transform());

    int handleRadius = m_tool->canvas()->resourceManager()->handleRadius();
    m_parameterShape->paintHandle(painter, converter, m_handleId, handleRadius);
    painter.restore();
}

void ParameterHandle::repaint() const
{
    m_tool->repaint(m_parameterShape->shapeToDocument(QRectF(m_parameterShape->handlePosition(m_handleId), QSize(1, 1))));
}

KInteractionStrategy * ParameterHandle::handleMousePress(KoPointerEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
        if (selection)
            selection->clear();
        return new KParameterChangeStrategy(m_tool, m_parameterShape, m_handleId);
    }
    return 0;
}

bool ParameterHandle::check(const QList<KoPathShape*> &selectedShapes)
{
    return selectedShapes.contains(m_parameterShape);
}
