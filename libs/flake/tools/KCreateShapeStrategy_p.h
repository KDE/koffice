/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOCREATESHAPESTRATEGY_H
#define KOCREATESHAPESTRATEGY_H

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


#include "KShapeRubberSelectStrategy.h"

#include <QPointF>
#include <QPainterPath>

class KCanvasBase;
class KCreateShapesTool;

/**
 * A strategy for the KCreateShapesTool.
 */
class KCreateShapeStrategy : public KShapeRubberSelectStrategy
{
public:
    /**
     * Constructor that starts to create a new shape.
     * @param tool the parent tool which controls this strategy
     * @param clicked the initial point that the user depressed (in pt).
     */
    KCreateShapeStrategy(KCreateShapesTool *tool, const QPointF &clicked);
    virtual ~KCreateShapeStrategy() {}

    void finishInteraction(Qt::KeyboardModifiers modifiers);
    QUndoCommand* createCommand(QUndoCommand *parent = 0);
    void paint(QPainter &painter, const KViewConverter &converter);
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);

private:
    QPainterPath m_outline;
    QRectF m_outlineBoundingRect;
    Q_DECLARE_PRIVATE(KShapeRubberSelectStrategy)
};

#endif

