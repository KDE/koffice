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

#ifndef KOPATHCONTROLPOINTMOVECOMMAND_H
#define KOPATHCONTROLPOINTMOVECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include "KoPathShape.h"
#include "KoPathPointData.h"
#include "KoPathPoint.h"
#include "flake_export.h"


/// The undo / redo command for path point moving.
class FLAKE_TEST_EXPORT KoPathControlPointMoveCommand : public QUndoCommand
{
public:
    /**
     * Command to move one control path point.
     * @param offset the offset by which the point is moved in document coordinates
     * @param pointType the type of the point to move
     * @param parent the parent command used for macro commands
     */
    KoPathControlPointMoveCommand(const KoPathPointData &pointData, const QPointF &offset, KoPathPoint::KoPointType pointType,
                                  QUndoCommand *parent = 0);
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathPointData m_pointData;
    // the offset in shape coordinates
    QPointF m_offset;
    KoPathPoint::KoPointType m_pointType;
};

#endif // KOPATHCONTROLPOINTMOVECOMMAND_H
