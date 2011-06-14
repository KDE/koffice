/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "ConnectionChangeStrategy.h"

#include <KShapeConnection.h>
#include <KShapeManager.h>
#include <KShape.h>
#include <KoToolBase.h>
#include <KCanvasBase.h>
#include <KoViewConverter.h>
#include <KoSnapGuide.h>

#include <QLineF>
#include <KDebug>

ConnectionChangeStrategy::ConnectionChangeStrategy(KoToolBase *tool, KShapeConnection *connection, const QPointF &clicked, Type type)
    : KInteractionStrategy(tool),
    m_connection(connection),
    m_type(type)
{
    Q_UNUSED(clicked);
    Q_ASSERT(connection);
    if (type == StartPointDrag) {
        m_origPoint = connection->startPoint();
        m_start.shape = connection->shape1();
        if (m_start.shape)
            m_start.index = connection->gluePointIndex1();
        else
            m_start.point = connection->startPoint();
    } else if (type == EndPointDrag) {
        m_origPoint = connection->endPoint();
        m_start.shape = connection->shape2();
        if (m_start.shape)
            m_start.index = connection->gluePointIndex2();
        else
            m_start.point = connection->endPoint();
    }
}

void ConnectionChangeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF newLocation = tool()->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    switch (m_type) {
    case StartPointDrag: {
        tool()->repaintDecorations();
        if (m_connection->shape1()) { // disconnect
            m_connection->setStartPoint(0, 0);
        }
        Connection connection = findConnectionForPoint(newLocation);
        if (connection.shape != 0)
            m_connection->setStartPoint(connection.shape, connection.index);
        else
            m_connection->setStartPoint(newLocation);
        break;
    }
    case EndPointDrag: {
        tool()->repaintDecorations();
        if (m_connection->shape2()) { // disconnect
            m_connection->setEndPoint(0, 0);
        }
        Connection connection = findConnectionForPoint(newLocation);
        if (connection.shape != 0)
            m_connection->setEndPoint(connection.shape, connection.index);
        else
            m_connection->setEndPoint(newLocation);
        break;
    }
    default:
        return;
    };
    tool()->repaintDecorations();
}

QUndoCommand* ConnectionChangeStrategy::createCommand(QUndoCommand *parent)
{
    if (m_type == StartPointDrag) {
        ConnectionHook end;
        end.shape = m_connection->shape1();
        if (end.shape)
            end.index = m_connection->gluePointIndex1();
        else
            end.point = m_connection->startPoint();
        return new ConnectionChangeCommand(m_connection, ConnectionChangeCommand::StartOnly,
                m_start, end, parent);
    } else {
        ConnectionHook end;
        end.shape = m_connection->shape2();
        if (end.shape)
            end.index = m_connection->gluePointIndex2();
        else
            end.point = m_connection->endPoint();
        return new ConnectionChangeCommand(m_connection, ConnectionChangeCommand::EndOnly,
                m_start, end, parent);
    }
}

void ConnectionChangeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    //m_connection->paintDecorations(painter, converter);
    // TODO implement
}

ConnectionChangeStrategy::Connection ConnectionChangeStrategy::findConnectionForPoint(const QPointF &point) const
{
    const int distance = tool()->canvas()->snapGuide()->snapDistance();
    QRectF rect(point.x() - distance, point.y() - distance, distance * 2, distance * 2);
    Connection answer;
    foreach (KShape *shape, tool()->canvas()->shapeManager()->shapesAt(rect)) {
        QTransform st = shape->absoluteTransformation(0);
        QList<QPointF> points = shape->connectionPoints();
        for (int i = 0; i < points.size(); ++i) {
            QLineF line(point, st.map(points[i]));
            if (line.length() <= distance) {
                answer.shape = shape;
                answer.index = i;
                return answer;
            }
        }
    }
    return answer;
}
