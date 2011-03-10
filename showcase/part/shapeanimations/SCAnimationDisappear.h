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

#ifndef KPRANIMATIONDISAPPEAR_H
#define KPRANIMATIONDISAPPEAR_H

#include "SCShapeAnimationOld.h"

#include <QPointF>
#include <QRectF>

class SCAnimationDisappear : public SCShapeAnimationOld
{
public:
    SCAnimationDisappear(KoShape * shape, int step);
    virtual ~SCAnimationDisappear();

    virtual SCAnimationData * animationData(KoCanvasBase * canvas, KoShapeManager * shapeManager, const QRectF & pageRect);
    virtual bool animate(QPainter &painter, const KoViewConverter &converter, SCAnimationData * animationData);
    virtual void animateRect(QRectF & rect, SCAnimationData * animationData);
    virtual void next(int currentTime, SCAnimationData * animationData);
    virtual void finish(SCAnimationData * animationData);
};

#endif /* KPRANIMATIONDISAPPEAR_H */
