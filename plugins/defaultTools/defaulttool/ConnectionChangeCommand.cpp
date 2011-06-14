/* This file is part of the KDE project
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

#include "ConnectionChangeCommand.h"

#include<KShapeConnection.h>
#include<KShape.h>

ConnectionChangeCommand::ConnectionChangeCommand(KShapeConnection *connection, const ConnectionHook &previousStart, const ConnectionHook &newStart, const ConnectionHook &previousEnd, const ConnectionHook &newEnd, QUndoCommand *parent)
    : QUndoCommand(parent),
    m_type(StartAndEnd),
    m_connection(connection),
    m_previousStart(previousStart),
    m_newStart(newStart),
    m_previousEnd(previousEnd),
    m_newEnd(newEnd)
{
    Q_ASSERT(connection);
}

ConnectionChangeCommand::ConnectionChangeCommand(KShapeConnection *connection, Type type, const ConnectionHook &previousHook, const ConnectionHook &newHook, QUndoCommand *parent)
    : QUndoCommand(parent),
    m_type(type),
    m_connection(connection),
    m_previousStart(previousHook),
    m_newStart(newHook),
    m_previousEnd(previousHook),
    m_newEnd(newHook)
{
    Q_ASSERT(connection);
}

void ConnectionChangeCommand::redo()
{
    QUndoCommand::redo();
    m_connection->update();
    if (m_type == StartAndEnd || m_type == StartOnly) {
        if (m_newStart.shape)
            m_connection->setStartPoint(m_newStart.shape, m_newStart.index);
        else
            m_connection->setStartPoint(m_newStart.point);
    }
    if (m_type == StartAndEnd || m_type == EndOnly) {
        if (m_newEnd.shape)
            m_connection->setEndPoint(m_newEnd.shape, m_newEnd.index);
        else
            m_connection->setEndPoint(m_newEnd.point);
    }
    m_connection->update();
}

void ConnectionChangeCommand::undo()
{
    QUndoCommand::undo();
    m_connection->update();
    if (m_type == StartAndEnd || m_type == StartOnly) {
        if (m_previousStart.shape)
            m_connection->setStartPoint(m_previousStart.shape, m_previousStart.index);
        else
            m_connection->setStartPoint(m_previousStart.point);
    }
    if (m_type == StartAndEnd || m_type == EndOnly) {
        if (m_previousEnd.shape)
            m_connection->setEndPoint(m_previousEnd.shape, m_previousEnd.index);
        else
            m_connection->setEndPoint(m_previousEnd.point);
    }
    m_connection->update();
}
