/* This file is part of the KDE project
 * Copyright (C) 2006,2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KPATHPOINTMOVECOMMAND_H
#define KPATHPOINTMOVECOMMAND_H

#include "flake_export.h"

#include <QUndoCommand>
#include <QPointF>

#include "KPathShape.h"
#include "KPathPointData.h"

class KPathPointMoveCommandPrivate;

/// The undo / redo command for path point moving.
class FLAKE_EXPORT KPathPointMoveCommand : public QUndoCommand
{
public:
    /**
     * Command to move path points.
     * @param pointData the path points to move
     * @param offset the offset by which the point is moved in document coordinates
     * @param parent the parent command used for macro commands
     */
    KPathPointMoveCommand(const QList<KPathPointData> &pointData, const QPointF &offset, QUndoCommand *parent = 0);

    /**
    * Command to move path points.
    * @param pointData the path points to move
    * @param offsets the offsets by which the points are moved in document coordinates
    * @param parent the parent command used for macro commands
    */
    KPathPointMoveCommand(const QList<KPathPointData> &pointData, const QList<QPointF> &offsets, QUndoCommand *parent = 0);

    ~KPathPointMoveCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KPathPointMoveCommandPrivate *d;
};

#endif // KPATHPOINTMOVECOMMAND_H
