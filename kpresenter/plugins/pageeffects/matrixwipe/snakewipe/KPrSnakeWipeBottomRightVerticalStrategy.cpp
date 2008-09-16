/* This file is part of the KDE project
   Copyright (C) 2008 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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

#include "KPrSnakeWipeBottomRightVerticalStrategy.h"
#include "KPrSnakeWipeEffectFactory.h"

KPrSnakeWipeBottomRightVerticalStrategy::KPrSnakeWipeBottomRightVerticalStrategy()
    : KPrMatrixWipeStrategy( KPrSnakeWipeEffectFactory::FromRight, "snakeWipe", "topLeftVertical", true, true )
{
}

KPrSnakeWipeBottomRightVerticalStrategy::~KPrSnakeWipeBottomRightVerticalStrategy()
{
}

int KPrSnakeWipeBottomRightVerticalStrategy::squareIndex(int x, int y, int columns, int rows)
{
    int Y = y;
    if (x & 1) Y = rows - y - 1;
    return Y + (columns - x - 1) * rows;
}

KPrMatrixWipeStrategy::Direction KPrSnakeWipeBottomRightVerticalStrategy::squareDirection(int x, int y, int columns, int rows)
{
    if (x & 1) return BottomToTop;
    else return TopToBottom;
}

int KPrSnakeWipeBottomRightVerticalStrategy::maxIndex(int columns, int rows)
{
    return columns * rows;
}

