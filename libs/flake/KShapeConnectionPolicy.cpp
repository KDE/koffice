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

#include "KShapeConnectionPolicy.h"

/*
     data is 32 bits, we reserve 8 for the EscapeDirection, the rest goes to align.
*/

KShapeConnectionPolicy::KShapeConnectionPolicy(KFlake::EscapeDirection dir, Qt::Alignment align)
    : data (dir + (align << 8))
{
    Q_ASSERT(dir == escapeDirection());
    Q_ASSERT(align == alignment());
}

KFlake::EscapeDirection KShapeConnectionPolicy::escapeDirection() const
{
    return static_cast<KFlake::EscapeDirection>(data & 0xff);
}

void KShapeConnectionPolicy::setEscapeDirection(KFlake::EscapeDirection escapeDirection)
{
    data = (data & 0xffffff00) | escapeDirection;
}

Qt::Alignment KShapeConnectionPolicy::alignment() const
{
    uint answer = data >> 8;
    if (answer == 0)
        return Qt::AlignCenter;
    return static_cast<Qt::Alignment>(answer);
}

void KShapeConnectionPolicy::setAlignment(Qt::Alignment align)
{
    data = (data & 0xff) | (align << 8);
}

