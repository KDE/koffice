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

#include <QUndoCommand>
#include <QPointF>

class KoShapeConnection;
class KoShape;

/**
 * All information for hooking up a connection object to either a shape or just a point.
 * If the shape is null then the point is used.
 */
struct ConnectionHook {
    KoShape *shape;
    int index;
    QPointF point;
};

/// command to undo/redo changes to a KoShapeConnection;
class ConnectionChangeCommand : public QUndoCommand
{
public:
    enum Type {
        StartAndEnd,
        StartOnly,
        EndOnly
    };
    ConnectionChangeCommand(KoShapeConnection *connection,
        const ConnectionHook &previousStart, const ConnectionHook &newStart,
        const ConnectionHook &previousEnd, const ConnectionHook &newEnd, QUndoCommand *parent);

    ConnectionChangeCommand(KoShapeConnection *connection, Type type,
        const ConnectionHook &previousHook, const ConnectionHook &newHook, QUndoCommand *parent);

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    Type m_type;
    KoShapeConnection *m_connection;
    ConnectionHook m_previousStart, m_newStart, m_previousEnd, m_newEnd;
};
