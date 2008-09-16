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

#ifndef KPRWATERFALLWIPEBOTTOMLEFTSTRATEGY_H
#define KPRWATERFALLWIPEBOTTOMLEFTSTRATEGY_H

#include "../KPrMatrixWipeStrategy.h"

class KPrWaterfallWipeBottomLeftStrategy : public KPrMatrixWipeStrategy
{
public:
    KPrWaterfallWipeBottomLeftStrategy(Direction direction);
    virtual ~KPrWaterfallWipeBottomLeftStrategy();
protected:
    virtual int squareIndex(int x, int y, int columns, int rows);
    virtual Direction squareDirection(int x, int y, int columns, int rows);
    virtual int maxIndex(int columns, int rows);
private:
    Direction m_direction;
};

#endif // KPRWATERFALLWIPEBOTTOMLEFTSTRATEGY_H
