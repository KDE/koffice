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

#include <KoShapeConnection.h>
#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoSnapGuide.h>

#include <KDebug>

ConnectionChangeStrategy::ConnectionChangeStrategy(KoToolBase *tool, KoShapeConnection *connection, const QPointF &clicked, Type type)
    : KoInteractionStrategy(tool),
    m_connection(connection),
    m_type(type)
{
    Q_ASSERT(connection);
    if (type == StartPointDrag)
        m_origPoint = connection->startPoint();
    else if (type == EndPointDrag)
        m_origPoint = connection->endPoint();
}

void ConnectionChangeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF newLocation = tool()->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    switch (m_type) {
    case StartPointDrag:
        tool()->repaintDecorations();
        if (m_connection->shape1()) { // disconnect
            m_connection->setStartPoint(0, 0);
        }
        m_connection->setStartPoint(newLocation);
        // TODO connect to existing shapes
        break;
    case EndPointDrag:
        tool()->repaintDecorations();
        if (m_connection->shape2()) { // disconnect
            m_connection->setEndPoint(0, 0);
        }
        m_connection->setEndPoint(newLocation);
        break;
    default:
        return;
    };
    tool()->repaintDecorations();
}

QUndoCommand* ConnectionChangeStrategy::createCommand(QUndoCommand *parent)
{
    // TODO

    return 0;
}

void ConnectionChangeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    // TODO set values back to orig so the command can do stuff?
}

void ConnectionChangeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    //m_connection->paintDecorations(painter, converter);
}
