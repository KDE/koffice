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

#ifndef SHAPERESIZESTRATEGY_H
#define SHAPERESIZESTRATEGY_H

#include <KoInteractionStrategy.h>

#include <QPointF>

class KoCanvasBase;
class KoTool;

/**
 * A strategy for the KoInteractionTool.
 * This strategy is invoked when the user starts a resize of a selection of objects,
 * the stategy will then resize the objects interactively and provide a command afterwards.
 */
class ShapeResizeStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor
     */
    ShapeResizeStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked, KoFlake::SelectionHandle direction );
    virtual ~ShapeResizeStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    QUndoCommand* createCommand();
    void finishInteraction( Qt::KeyboardModifiers modifiers ) { Q_UNUSED(modifiers); }
    virtual void paint( QPainter &painter, const KoViewConverter &converter);
    virtual void handleCustomEvent( KoPointerEvent * event );
private:
    void resizeBy( const QPointF &center, qreal zoomX, qreal zoomY );

    QPointF m_start;
    QList<QPointF> m_startPositions;
    QList<QSizeF> m_startSizes;
    bool m_top, m_left, m_bottom, m_right;
    QMatrix m_unwindMatrix, m_windMatrix;
    QSizeF m_initialSize;
    QPointF m_initialPosition;
    QMatrix m_scaleMatrix;
    QList<QMatrix> m_oldTransforms;
    QList<QMatrix> m_transformations;
    QPointF m_lastScale;
};

#endif

