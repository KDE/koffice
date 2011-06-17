/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KTextBlockData.h"
#include "KTextBlockBorderData.h"
#include "KTextBlockPaintStrategyBase.h"

class KTextBlockData::Private
{
public:
    Private()
        : counterWidth(0),
        counterSpacing(0),
        counterIsImage(false),
        counterIndex(1),
        border(0),
        paintStrategy(0)
    {
    }

    ~Private() {
        if (border && !border->deref())
            delete border;
        delete paintStrategy;
    }
    qreal counterWidth;
    qreal counterSpacing;
    QString counterText;
    QString partialCounterText;
    bool counterIsImage;
    int counterIndex;
    QPointF counterPos;
    KTextBlockBorderData *border;
    KTextBlockPaintStrategyBase *paintStrategy;
};

KTextBlockData::KTextBlockData()
        : d(new Private())
{
    d->counterWidth = -1.0;
}

KTextBlockData::~KTextBlockData()
{
    delete d;
}

bool KTextBlockData::hasCounterData() const
{
    return d->counterWidth >= 0 && (!d->counterText.isNull() || d->counterIsImage);
}

qreal KTextBlockData::counterWidth() const
{
    return qMax(qreal(0), d->counterWidth);
}

void KTextBlockData::setBorder(KTextBlockBorderData *border)
{
    if (d->border && !d->border->deref())
        delete d->border;
    d->border = border;
    if (d->border)
        d->border->ref();
}

void KTextBlockData::setCounterWidth(qreal width)
{
    d->counterWidth = width;
}

qreal KTextBlockData::counterSpacing() const
{
    return d->counterSpacing;
}

void KTextBlockData::setCounterSpacing(qreal spacing)
{
    d->counterSpacing = spacing;
}

void KTextBlockData::setCounterText(const QString &text)
{
    d->counterText = text;
}

QString KTextBlockData::counterText() const
{
    return d->counterText;
}

void KTextBlockData::setPartialCounterText(const QString &text)
{
    d->partialCounterText = text;
}

QString KTextBlockData::partialCounterText() const
{
    return d->partialCounterText;
}

void KTextBlockData::setCounterIsImage(bool isImage)
{
    d->counterIsImage = isImage;
}

bool KTextBlockData::counterIsImage() const
{
    return d->counterIsImage;
}

void KTextBlockData::setCounterIndex(int index)
{
    d->counterIndex = index;
}

int KTextBlockData::counterIndex() const
{
    return d->counterIndex;
}

void KTextBlockData::setCounterPosition(const QPointF &position)
{
    d->counterPos = position;
}

QPointF KTextBlockData::counterPosition() const
{
    return d->counterPos;
}

KTextBlockBorderData *KTextBlockData::border() const
{
    return d->border;
}

void KTextBlockData::setPaintStrategy(KTextBlockPaintStrategyBase *paintStrategy)
{
    delete d->paintStrategy;
    d->paintStrategy = paintStrategy;
}

KTextBlockPaintStrategyBase *KTextBlockData::paintStrategy() const
{
    return d->paintStrategy;
}

