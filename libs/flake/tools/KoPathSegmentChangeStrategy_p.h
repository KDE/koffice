/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHSEGMENTCHANGESTRATEGY_H
#define KOPATHSEGMENTCHANGESTRATEGY_H

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


#include "KInteractionStrategy.h"
#include "KoPathSegment.h"
#include "KPathPointData.h"
#include <QtCore/QPointF>

class KoPathTool;
class KCanvasBase;
class KoPathShape;

/**
* @brief Strategy for deforming a segment of a path shape.
*/
class KoPathSegmentChangeStrategy : public KInteractionStrategy
{
public:
    KoPathSegmentChangeStrategy(KoPathTool *tool, const QPointF &pos, const KPathPointData &segment, qreal segmentParam);
    virtual ~KoPathSegmentChangeStrategy();
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual QUndoCommand *createCommand(QUndoCommand *parent = 0);

private:
    QPointF m_originalPosition;
    QPointF m_lastPosition;
    /// the accumulated point move amount
    QPointF m_move;
    /// pointer to the path tool
    KoPathTool *m_tool;
    KoPathShape *m_path;
    KoPathSegment m_segment;
    qreal m_segmentParam;
    QPointF m_ctrlPoint1Move;
    QPointF m_ctrlPoint2Move;
    KPathPointData m_pointData1;
    KPathPointData m_pointData2;
    int m_originalSegmentDegree;
};

#endif // KOPATHSEGMENTCHANGESTRATEGY_H
