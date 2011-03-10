/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCFadeCrossStrategy.h"
#include "SCFadeEffectFactory.h"

#include <QWidget>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>

#include <kdebug.h>

SCFadeCrossStrategy::SCFadeCrossStrategy()
: SCPageEffectStrategy(SCFadeEffectFactory::CrossFade, "fade", "crossfade", false, true)
{
}

SCFadeCrossStrategy::~SCFadeCrossStrategy()
{
}

void SCFadeCrossStrategy::setup(const SCPageEffect::Data &data, QTimeLine &timeLine)
{
    timeLine.setFrameRange(0, 1000); // TODO might not be needed
    data.m_oldPageItem->setZValue(1);
    data.m_newPageItem->setZValue(2);
    data.m_newPageItem->setOpacity(0);
    data.m_oldPageItem->show();
    data.m_newPageItem->show();
}

void SCFadeCrossStrategy::paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data)
{
    Q_UNUSED(p);
    Q_UNUSED(currPos);
    Q_UNUSED(data);
}

void SCFadeCrossStrategy::next(const SCPageEffect::Data &data)
{
    int frame = data.m_timeLine.frameForTime(data.m_currentTime);
    if (frame >= data.m_timeLine.endFrame()) {
        finish(data);
    }
    else {
        qreal value = data.m_timeLine.valueForTime(data.m_currentTime);
        data.m_newPageItem->setOpacity(value);
    }
}

void SCFadeCrossStrategy::finish(const SCPageEffect::Data &data)
{
    data.m_graphicsView->hide();
}
