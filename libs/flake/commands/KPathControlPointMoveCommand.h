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

#ifndef KPATHCONTROLPOINTMOVECOMMAND_H
#define KPATHCONTROLPOINTMOVECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include "KPathShape.h"
#include "KPathPointData.h"
#include "KPathPoint.h"
#include "flake_export.h"


/// The undo / redo command for path point moving.
class FLAKE_TEST_EXPORT KPathControlPointMoveCommand : public QUndoCommand
{
public:
    /**
     * Command to move one control path point.
     * @param offset the offset by which the point is moved in document coordinates
     * @param pointType the type of the point to move
     * @param parent the parent command used for macro commands
     */
    KPathControlPointMoveCommand(const KPathPointData &pointData, const QPointF &offset,
            KPathPoint::PointType pointType, QUndoCommand *parent = 0);
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KPathPointData m_pointData;
    // the offset in shape coordinates
    QPointF m_offset;
    KPathPoint::PointType m_pointType;
};

#endif // KPATHCONTROLPOINTMOVECOMMAND_H
