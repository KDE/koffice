/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOPATHPARAMETERCHANGESTRATEGY_H
#define KOPATHPARAMETERCHANGESTRATEGY_H

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


#include <QPointF>
#include "KInteractionStrategy.h"

class KParameterShape;

/// Strategy for changing control points of parametric shapes
class KParameterChangeStrategy : public KInteractionStrategy
{
public:
    /**
     * Constructs a strategy for changing control points of parametric shapes.
     * @param tool the tool the strategy belongs to
     * @param parameterShape the parametric shapes the strategy is working on
     * @param handleId the id of the handle to modify
     */
    KParameterChangeStrategy(KToolBase *tool, KParameterShape *parameterShape, int handleId);
    virtual ~KParameterChangeStrategy();

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual QUndoCommand* createCommand(QUndoCommand *parent = 0);

private:
    KParameterShape * const m_parameterShape; ///< the parametric shape we are working on
    const int m_handleId;                      ///< the id of the control point
    const QPointF m_startPoint;                ///< the starting position of the control point
    Qt::KeyboardModifiers m_lastModifierUsed;
    QPointF m_releasePoint;
};

#endif /* KOPATHPARAMETERCHANGESTRATEGY_H */
