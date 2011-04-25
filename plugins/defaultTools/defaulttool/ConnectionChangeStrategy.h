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

#ifndef CONNECTIONCHANGESTRATEGY_H
#define CONNECTIONCHANGESTRATEGY_H

#include <KoInteractionStrategy.h>

#include "ConnectionChangeCommand.h"

#include <QList>
#include <QPointF>

class KoShapeConnection;
class KoShape;

/**
 * 
 */
class ConnectionChangeStrategy : public KoInteractionStrategy
{
public:
    enum Type {
        StartPointDrag,
        EndPointDrag
    };
    /**
     * Constructor that starts to move the objects.
     * @param tool the parent tool which controls this strategy
     * @param canvas the canvas interface which will supply things like a selection object
     * @param clicked the initial point that the user depressed (in pt).
     */
    ConnectionChangeStrategy(KoToolBase *tool, KoShapeConnection* connection, const QPointF &clicked, Type type);
    virtual ~ConnectionChangeStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    QUndoCommand* createCommand(QUndoCommand *parent = 0);
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

private:
    struct Connection {
        Connection() : shape(0), index(0) { }
        KoShape *shape;
        int index;
    };

    Connection findConnectionForPoint(const QPointF &point) const;


    KoShapeConnection *m_connection;
    Type m_type;
    QPointF m_origPoint;

    ConnectionHook m_start; // for the connection we are editing, this is the state at start
};

#endif
