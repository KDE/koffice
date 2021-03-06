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

#ifndef KZOOMSTRATEGY_H
#define KZOOMSTRATEGY_H

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

class KCanvasController;
class KZoomTool;

/**
 * //internal
 * This is a strategy for the KZoomTool which will be used to do the actual zooming
 */
class KZoomStrategy : public KShapeRubberSelectStrategy
{
public:
    /**
     * constructor
     * @param tool the parent tool this strategy is for
     * @param controller the canvas controller that wraps the canvas the tool is acting on.
     * @param clicked the location (in documnet points) where the interaction starts.
     */
    KZoomStrategy(KZoomTool *tool, KCanvasController *controller, const QPointF &clicked);

    void forceZoomOut();
    void forceZoomIn();

    /// Execute the zoom
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);
    virtual void cancelInteraction();
private:
    KCanvasController *m_controller;

    bool m_forceZoomOut;
    Q_DECLARE_PRIVATE(KShapeRubberSelectStrategy)
};

#endif

