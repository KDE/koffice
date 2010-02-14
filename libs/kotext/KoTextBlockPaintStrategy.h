/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
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

#ifndef KOTEXTBLOCKPAINTSTRATEGY_H
#define KOTEXTBLOCKPAINTSTRATEGY_H

#include "kotext_export.h"

#include <QBrush>

/**
 * This class is used to control aspects of textblock painting
 * Which is used when KPresenter animate text. Though KPresenter uses a subclass.
 */
class KOTEXT_EXPORT KoTextBlockPaintStrategy
{
public:
    KoTextBlockPaintStrategy();
    virtual ~KoTextBlockPaintStrategy() {}
    virtual QBrush background(const QBrush &defaultBackground) {return defaultBackground;}
    virtual void modifyPainter(QPainter *painter) {}
    virtual bool visible() {return true;}
};

#endif
