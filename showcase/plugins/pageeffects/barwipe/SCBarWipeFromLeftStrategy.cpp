/* This file is part of the KDE project
   Copyright (C) 2008 Timothe Lacroix <dakeyras.khan@gmail.com>

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

#include "SCBarWipeFromLeftStrategy.h"
#include "SCBarWipeEffectFactory.h"

#include <QWidget>
#include <QPainter>

SCBarWipeFromLeftStrategy::SCBarWipeFromLeftStrategy()
: SCPageEffectStrategy(SCBarWipeEffectFactory::FromLeft, "barWipe", "leftToRight", false)
{
}

SCBarWipeFromLeftStrategy::~SCBarWipeFromLeftStrategy()
{
}

void SCBarWipeFromLeftStrategy::setup(const SCPageEffect::Data &data, QTimeLine &timeLine)
{
    timeLine.setFrameRange(0, data.m_widget->width());
}

void SCBarWipeFromLeftStrategy::paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data)
{
    int height = data.m_widget->height();
    int width = data.m_widget->width();
    QRect rect1(0, 0, currPos, height);
    QRect rect2(currPos, 0, width, height);
    p.drawPixmap(QPoint(0, 0), data.m_newPage, rect1);
    p.drawPixmap(QPoint(currPos, 0), data.m_oldPage, rect2);
}

void SCBarWipeFromLeftStrategy::next(const SCPageEffect::Data &data)
{
    int lastPos = data.m_timeLine.frameForTime(data.m_lastTime);
    int currPos = data.m_timeLine.frameForTime(data.m_currentTime);
    data.m_widget->update(lastPos, 0, currPos - lastPos,data.m_widget->height());
}
