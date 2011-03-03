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

#include "SCWindShieldWipeStrategy.h"
#include "SCWindShieldWipeEffectFactory.h"

#include <math.h>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include "SCClockWipeSubpathHelper.h"

#include <kdebug.h>

SCWindShieldWipeStrategy::SCWindShieldWipeStrategy(int subType, const char * smilType, const char *smilSubType, bool reverse)
    : SCSweepWipeStrategy(subType, smilType, smilSubType, reverse)
{
}

SCWindShieldWipeStrategy::~SCWindShieldWipeStrategy()
{
}

void SCWindShieldWipeStrategy::setup(const SCPageEffect::Data &data, QTimeLine &timeLine)
{
    Q_UNUSED(data);
    timeLine.setFrameRange(0, 360);
}

void SCWindShieldWipeStrategy::paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data)
{
    int width = data.m_widget->width();
    int height = data.m_widget->height();
    QRect rect(0, 0, width, height);
    p.drawPixmap(QPoint(0, 0), data.m_oldPage, rect);

    double startAngle1;
    QRect boundingRect1;

    double startAngle2;
    QRect boundingRect2;

    double rotationRange1 = -0.5*M_PI;
    double rotationRange2 = -0.5*M_PI;

    if (subType() == SCWindShieldWipeEffectFactory::Right || subType() == SCWindShieldWipeEffectFactory::Up ||
       subType() == SCWindShieldWipeEffectFactory::RightReverse || subType() == SCWindShieldWipeEffectFactory::UpReverse) {

        switch(subType())
        {
            case SCWindShieldWipeEffectFactory::Right:
            case SCWindShieldWipeEffectFactory::RightReverse:
                startAngle1 = 1.5*M_PI;
                boundingRect1 = QRect(0, 0, width, height/2);

                startAngle2 = 0.5*M_PI;
                boundingRect2 = QRect(0, height/2, width, height/2);

                rotationRange1 = 2*M_PI;
                rotationRange2 = -rotationRange1;
                break;
            case SCWindShieldWipeEffectFactory::Up:
            case SCWindShieldWipeEffectFactory::UpReverse:
                startAngle1 = 0;
                boundingRect1 = QRect(0, 0, width/2, height);

                startAngle2 = M_PI;
                boundingRect2 = QRect(width/2, 0, width/2, height);

                rotationRange1 = 2*M_PI;
                rotationRange2 = -rotationRange1;
                break;
            default:
                return;
        }

        if (reverse()) {
            startAngle1 = startAngle1 + rotationRange1;
            rotationRange1 *= -1;
            startAngle2 = startAngle2 + rotationRange2;
            rotationRange2 *= -1;
        }

        drawSweep(p, startAngle1, rotationRange1*currPos/360, boundingRect1, data);
        drawSweep(p, startAngle2, rotationRange2*currPos/360, boundingRect2, data);
    }
    else {
        switch(subType())
        {
            case SCWindShieldWipeEffectFactory::Vertical:
            case SCWindShieldWipeEffectFactory::VerticalReverse:

                startAngle1 = 0.5*M_PI;
                boundingRect1 = QRect(0, 0, width, height/2);

                startAngle2 = 1.5*M_PI;
                boundingRect2 = QRect(0, height/2, width, height/2);

                rotationRange1 = 2*M_PI;
                rotationRange2 = rotationRange1;
                break;
            case SCWindShieldWipeEffectFactory::Horizontal:
            case SCWindShieldWipeEffectFactory::HorizontalReverse:

                startAngle1 = M_PI;
                boundingRect1 = QRect(0, 0, width/2, height);

                startAngle2 = 0;
                boundingRect2 = QRect(width/2, 0, width/2, height);

                rotationRange1 = 2*M_PI;
                rotationRange2 = rotationRange1;
                break;
            default:
                return;
        }

        if (reverse()) {
            startAngle1 += M_PI;
            startAngle2 += M_PI;
        }

        double angle = static_cast<double>(currPos)/360 * M_PI;
        drawSweep(p, startAngle1 - angle, 2*angle, boundingRect1, data);
        drawSweep(p, startAngle2 - angle, 2*angle, boundingRect2, data);
    }
}
