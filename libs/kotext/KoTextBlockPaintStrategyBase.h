/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTBLOCKPAINTSTRATEGYBASE_H
#define KOTEXTBLOCKPAINTSTRATEGYBASE_H

#include "kotext_export.h"

#include <QBrush>

class QPainter;

/**
 * This class is used to control aspects of textblock painting
 * Which is used when KPresenter animates text.
 * TODO rename to end in Base
 */
class KOTEXT_EXPORT KoTextBlockPaintStrategyBase
{
public:
    KoTextBlockPaintStrategyBase();
    virtual ~KoTextBlockPaintStrategyBase();
    /// returns a background for the block, the default implemntation returns the defaultBackground
    virtual QBrush background(const QBrush &defaultBackground);
    /// A strategy implementing this class can apply its settings by modifying the \a painter
    virtual void applyStrategy(QPainter *painter);
    /// Returns true if the block should be painted at all or false when it should be skipped
    virtual bool isVisible();
};

#endif
