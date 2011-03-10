/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCAnimationDisappear.h"

#include <QPainter>
#include <KoCanvasBase.h>
#include <KoShape.h>
#include <KoViewConverter.h>

#include "SCAnimationData.h"

SCAnimationDisappear::SCAnimationDisappear(KoShape * shape, int step)
: SCShapeAnimationOld(shape, step, Disappear)
{
}

SCAnimationDisappear::~SCAnimationDisappear()
{
}

SCAnimationData * SCAnimationDisappear::animationData(KoCanvasBase * canvas, KoShapeManager * shapeManager, const QRectF & pageRect)
{
    Q_UNUSED(pageRect);
    SCAnimationDataTranslate * data = new SCAnimationDataTranslate(canvas, shapeManager, m_shape->boundingRect());
    double x = data->m_boundingRect.x() + data->m_boundingRect.width() + 2.0;
    data->m_translate = QPointF(-x, 0);
    data->m_timeLine.setDuration(1);
    return data;
}

bool SCAnimationDisappear::animate(QPainter &painter, const KoViewConverter &converter, SCAnimationData * animationData )
{
    SCAnimationDataTranslate * data = dynamic_cast<SCAnimationDataTranslate *>(animationData);
    Q_ASSERT(data);
    painter.translate(converter.documentToView(data->m_translate));
    return data->m_finished;
}

void SCAnimationDisappear::animateRect(QRectF & rect, SCAnimationData * animationData)
{
    SCAnimationDataTranslate * data = dynamic_cast<SCAnimationDataTranslate *>(animationData);
    Q_ASSERT(data);
    rect.translate(data->m_translate);
}

void SCAnimationDisappear::next(int currentTime, SCAnimationData * animationData)
{
    Q_UNUSED(currentTime);
    SCAnimationDataTranslate * data = dynamic_cast<SCAnimationDataTranslate *>(animationData);
    Q_ASSERT(data);
    data->m_canvas->updateCanvas(data->m_boundingRect.translated(data->m_translate));
    data->m_finished = true;
}

void SCAnimationDisappear::finish(SCAnimationData * animationData)
{
    SCAnimationDataTranslate * data = dynamic_cast<SCAnimationDataTranslate *>(animationData);
    Q_ASSERT(data);
    data->m_canvas->updateCanvas(data->m_boundingRect.translated(data->m_translate));
}
