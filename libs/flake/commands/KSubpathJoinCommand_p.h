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

#ifndef KOSUBPATHJOINCOMMAND_H
#define KOSUBPATHJOINCOMMAND_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QUndoCommand>
#include <QPointF>
#include "KPathPoint.h"
#include "KPathPointData.h"

/// The undo / redo command for joining two subpath end points
class KSubpathJoinCommand : public QUndoCommand
{
public:
    /**
     * Command to join two subpath end points.
     *
     * The points have to be from the same path shape.
     *
     * @param pointData1 the data of the first point to join
     * @param pointData2 the data of the second point to join
     * @param parent the parent command used for macro commands
     */
    KSubpathJoinCommand(const KPathPointData &pointData1, const KPathPointData &pointData2, QUndoCommand *parent = 0);
    ~KSubpathJoinCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KPathPointData m_pointData1;
    KPathPointData m_pointData2;
    KoPathPointIndex m_splitIndex;
    // the control points have to be stored in document positions
    QPointF m_oldControlPoint1;
    QPointF m_oldControlPoint2;
    KPathPoint::PointProperties m_oldProperties1;
    KPathPoint::PointProperties m_oldProperties2;
    enum Reverse {
        ReverseFirst = 1,
        ReverseSecond = 2
    };
    int m_reverse;
};

#endif // KOSUBPATHJOINCOMMAND_H
