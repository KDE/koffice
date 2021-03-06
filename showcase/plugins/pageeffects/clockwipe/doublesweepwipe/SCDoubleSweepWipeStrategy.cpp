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

#include "SCDoubleSweepWipeStrategy.h"
#include "SCDoubleSweepWipeEffectFactory.h"

#include <math.h>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include "SCClockWipeSubpathHelper.h"

#include <kdebug.h>

SCDoubleSweepWipeStrategy::SCDoubleSweepWipeStrategy(int subType, const char * smilType, const char *smilSubType, bool reverse)
    : SCSweepWipeStrategy(subType, smilType, smilSubType, reverse)
{
}

SCDoubleSweepWipeStrategy::~SCDoubleSweepWipeStrategy()
{
}

void SCDoubleSweepWipeStrategy::setup(const SCPageEffect::Data &data, QTimeLine &timeLine)
{
    Q_UNUSED(data);
    timeLine.setFrameRange(0, 360);
}

void SCDoubleSweepWipeStrategy::paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data)
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

    switch(subType())
    {
        case SCDoubleSweepWipeEffectFactory::ParallelVertical:
        case SCDoubleSweepWipeEffectFactory::ParallelVerticalReverse:
            startAngle1 = M_PI;
            boundingRect1 = QRect(0, 0, width, 2*height);

            startAngle2 = 0;
            boundingRect2 = QRect(0, - height, width, 2*height);

            rotationRange1 = -0.5*M_PI;
            rotationRange2 = rotationRange1;
            break;
        case SCDoubleSweepWipeEffectFactory::ParallelDiagonal:
        case SCDoubleSweepWipeEffectFactory::ParallelDiagonalReverse:
            startAngle1 = 0.5*M_PI;
            boundingRect1 = QRect(-width, 0, 2*width, height);

            startAngle2 = -0.5*M_PI;
            boundingRect2 = QRect(0, 0, 2*width, height);

            rotationRange1 = -0.5*M_PI;
            rotationRange2 = rotationRange1;
            break;
        case SCDoubleSweepWipeEffectFactory::OppositeVertical:
        case SCDoubleSweepWipeEffectFactory::OppositeVerticalReverse:
            startAngle1 = 0;
            boundingRect1 = QRect(0, -height/2, width, height);

            startAngle2 = 0;
            boundingRect2 = QRect(0, height/2, width, height);

            rotationRange1 = -M_PI;
            rotationRange2 = M_PI;
            break;
        case SCDoubleSweepWipeEffectFactory::OppositeHorizontal:
        case SCDoubleSweepWipeEffectFactory::OppositeHorizontalReverse:
            startAngle1 = 0.5*M_PI;
            boundingRect1 = QRect(-width/2, 0, width, height);

            startAngle2 = 0.5*M_PI;
            boundingRect2 = QRect(width/2, 0, width, height);

            rotationRange1 = -M_PI;
            rotationRange2 = M_PI;
            break;
        case SCDoubleSweepWipeEffectFactory::ParallelDiagonalTopLeft:
        case SCDoubleSweepWipeEffectFactory::ParallelDiagonalTopLeftReverse:
            startAngle1 = 0;
            boundingRect1 = QRect(-width, -height, 2*width, 2*height);

            startAngle2 = M_PI;
            boundingRect2 = QRect(0, 0, 2*width, 2*height);

            rotationRange1 = -atan(static_cast<double>(height)/width);
            rotationRange2 = rotationRange1;
            break;
        case SCDoubleSweepWipeEffectFactory::ParallelDiagonalBottomLeft:
        case SCDoubleSweepWipeEffectFactory::ParallelDiagonalBottomLeftReverse:
            startAngle1 = 0.5*M_PI;
            boundingRect1 = QRect(-width, 0, 2*width, 2*height);

            startAngle2 = 1.5*M_PI;
            boundingRect2 = QRect(0, -height, 2*width, 2*height);

            rotationRange1 = -atan(static_cast<double>(width)/height);
            rotationRange2 = rotationRange1;
            break;
        default:
            return;
    }

    if(reverse()) {
        startAngle1 = startAngle1 + rotationRange1;
        rotationRange1 *= -1;
        startAngle2 = startAngle2 + rotationRange2;
        rotationRange2 *= -1;
    }

    drawSweep(p, startAngle1, rotationRange1*currPos/360, boundingRect1, data);
    drawSweep(p, startAngle2, rotationRange2*currPos/360, boundingRect2, data);

}
