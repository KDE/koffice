/* This file is part of the KDE project
   Copyright (C) 2008 Sven Langkamp <sven.langkamp@gmail.com>

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

#include "SCClockWipeStrategy.h"

#include <math.h>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include <kdebug.h>

#include "SCClockWipeSubpathHelper.h"

SCClockWipeStrategy::SCClockWipeStrategy(int startAngle, int bladeCount, int subType, const char * smilType, const char *smilSubType, bool reverse)
    : SCPageEffectStrategy(subType, smilType, smilSubType, reverse), m_bladeCount(bladeCount)
{
    m_startAngle = static_cast<double>(startAngle)/180 * M_PI;
}

SCClockWipeStrategy::~SCClockWipeStrategy()
{
}

void SCClockWipeStrategy::setup(const SCPageEffect::Data &data, QTimeLine &timeLine)
{
    Q_UNUSED(data);
    timeLine.setFrameRange(0, 360);
}

void SCClockWipeStrategy::paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data)
{
    QRect rect(0, 0, data.m_widget->width(), data.m_widget->height());
    p.drawPixmap(QPoint(0, 0), data.m_oldPage, rect);

    QPainterPath clipPath;
    for(int i = 0; i < m_bladeCount; i++) {

        double bladeStartAngle;
        double bladeEndAngle;

        double angle = static_cast<double>(currPos)/m_bladeCount/180 * M_PI;

        if(!reverse()) {
            bladeEndAngle = 2*M_PI/m_bladeCount*i + m_startAngle;
            bladeStartAngle = bladeEndAngle - angle;
        }
        else {
            bladeStartAngle = 2*M_PI/m_bladeCount*(i+1) + m_startAngle;
            bladeEndAngle = bladeStartAngle + angle;
        }
        SCClockWipeSubpathHelper::addSubpathForCircularArc(&clipPath, rect, bladeStartAngle, bladeEndAngle);
    }
    p.setClipPath(clipPath);

    p.drawPixmap(QPoint(0, 0), data.m_newPage, rect);
}

void SCClockWipeStrategy::next(const SCPageEffect::Data &data)
{
    data.m_widget->update();
}
