/* This file is part of the KDE project
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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
#ifndef KOSHAPECONNECTIONPOLICY_H
#define KOSHAPECONNECTIONPOLICY_H

#include "flake_export.h"

#include <Qt>

/**
 * Connection point policy used to alter the behavior of connection wiring and shape resizing.
 */
class FLAKE_EXPORT KoShapeConnectionPolicy
{
public:
    /// Escape directions to explain which direction the connection leaves the shape
    enum EscapeDirection {
        EscapeAny = 0, ///< Escape in all directions
        EscapeLeft = 1, ///< Escape left
        EscapeRight = 2, ///< Escape right
        EscapeUp = 4, ///< Escape up
        EscapeDown = 8, ///< Escape down
        EscapeHorizontal = EscapeLeft | EscapeRight,  ///< Escape left or right
        EscapeVertical = EscapeUp | EscapeDown, ///< Escape top or down
    };

    KoShapeConnectionPolicy() : data(0) { }

    KoShapeConnectionPolicy(EscapeDirection escapeDir, Qt::Alignment alignmentHint = Qt::AlignCenter);

    EscapeDirection escapeDirection() const;
    void setEscapeDirection(EscapeDirection escapeDirection);
    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment align);

    bool operator==(const KoShapeConnectionPolicy &p) const { return data == p.data; }
    bool operator!=(const KoShapeConnectionPolicy &p) const { return data != p.data; }

private:
    uint data;
};

#endif
