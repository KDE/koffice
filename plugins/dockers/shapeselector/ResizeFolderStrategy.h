/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#ifndef RESIZEFOLDER_H
#define RESIZEFOLDER_H

#include "InteractionStrategy.h"

#include <QSizeF>

class Canvas;
class FolderShape;

class ResizeFolderStrategy : public InteractionStrategy {
public:
    ResizeFolderStrategy(Canvas *canvas, FolderShape *clickedFolder, KoPointerEvent &event);

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);

private:
    enum ScaleRole {
        MoveLeftBorder,
        MoveRightBorder,
        MoveBottomBorder,
        ScaleShape
    };

    Canvas *m_canvas;
    FolderShape *m_folder;
    QPointF m_startPosition;
    QPointF m_folderStartPosition;
    QSizeF m_startSize;
    ScaleRole m_scaleRole;
};

#endif
