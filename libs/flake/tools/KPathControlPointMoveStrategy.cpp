/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KPathControlPointMoveStrategy_p.h"
#include "KCanvasBase.h"
#include "KSnapGuide.h"

#include "KPathTool_p.h"
#include "commands/KPathControlPointMoveCommand.h"

KPathControlPointMoveStrategy::KPathControlPointMoveStrategy(KPathTool *tool, const KPathPointData &pointData, KPathPoint::PointType type, const QPointF &pos)
        : KInteractionStrategy(tool)
        , m_lastPosition(pos)
        , m_move(0, 0)
        , m_tool(tool)
        , m_pointData(pointData)
        , m_pointType(type)
{
}

KPathControlPointMoveStrategy::~KPathControlPointMoveStrategy()
{
}

void KPathControlPointMoveStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF docPoint = m_tool->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    QPointF move = docPoint - m_lastPosition;
    // as the last position can change when the top left is changed we have
    // to save it in document pos and not in shape pos
    m_lastPosition = docPoint;

    m_move += move;

    KPathControlPointMoveCommand cmd(m_pointData, move, m_pointType);
    cmd.redo();
}

QUndoCommand* KPathControlPointMoveStrategy::createCommand(QUndoCommand *parent)
{
    QUndoCommand *cmd = 0;
    if (!m_move.isNull()) {
        cmd = new KPathControlPointMoveCommand(m_pointData, m_move, m_pointType, parent);
        cmd->undo();
    }
    return cmd;
}
