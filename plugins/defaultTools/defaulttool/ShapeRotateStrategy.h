/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef SHAPEROTATESTRATEGY_H
#define SHAPEROTATESTRATEGY_H

#include <KInteractionStrategy.h>

#include <QPointF>
#include <QRectF>
#include <QTransform>
#include <QList>

class KCanvasBase;
class KToolBase;
class KShape;

/**
 * A strategy for the KInteractionTool.
 * This strategy is invoked when the user starts a rotate of a selection of objects,
 * the stategy will then rotate the objects interactively and provide a command afterwards.
 */
class ShapeRotateStrategy : public KInteractionStrategy
{
public:
    /**
     * Constructor that starts to rotate the objects.
     * @param tool the parent tool which controls this strategy
     * @param clicked the initial point that the user depressed (in pt).
     */
    ShapeRotateStrategy(KToolBase *tool, const QPointF &clicked, Qt::MouseButtons buttons);
    virtual ~ShapeRotateStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    QUndoCommand* createCommand(QUndoCommand *parent = 0);
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

private:
    void rotateBy(qreal angle);
    QRectF m_initialBoundingRect;
    QPointF m_start;
    QTransform m_rotationMatrix;
    QTransform m_initialSelectionMatrix;
    QList<QTransform> m_oldTransforms;
    QPointF m_rotationCenter;
    QList<KShape*> m_selectedShapes;
};

#endif

