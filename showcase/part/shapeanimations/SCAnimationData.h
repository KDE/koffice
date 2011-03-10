/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPRANIMATIONDATA_H
#define KPRANIMATIONDATA_H

#include <QTimeLine>
#include <QRectF>
#include <QPointF>

class KoCanvasBase;
class KoShapeManager;

class SCAnimationData
{
public:
    SCAnimationData(KoCanvasBase * canvas, KoShapeManager * shapeManager, QRectF boundingRect)
    : m_canvas(canvas)
    , m_shapeManager(shapeManager)
    , m_boundingRect(boundingRect)
    , m_finished(false)
    {}

    virtual ~SCAnimationData() {}

    KoCanvasBase * m_canvas;
    KoShapeManager * m_shapeManager;
    QTimeLine m_timeLine;
    QRectF m_boundingRect;
    bool m_finished;
};

class SCAnimationDataTranslate : public SCAnimationData
{
public:
    SCAnimationDataTranslate(KoCanvasBase * canvas, KoShapeManager * shapeManager, QRectF boundingRect)
    : SCAnimationData(canvas, shapeManager, boundingRect)
    {}

    virtual ~SCAnimationDataTranslate() {}

    QPointF m_translate;
};

#endif /* KPRANIMATIONDATA_H */
