/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
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

#include "SCShapeManagerAnimationStrategy.h"

#include <QPainter>
#include <KShape.h>
#include <KShapeManager.h>
#include <KoPAPage.h>

#include "SCShapeAnimations.h"
#include "SCPlaceholderShape.h"
#include "animations/SCAnimationCache.h"
#include "SCPageSelectStrategyBase.h"
#include "kdebug.h"

SCShapeManagerAnimationStrategy::SCShapeManagerAnimationStrategy(KShapeManager *shapeManager, SCAnimationCache* animationCache,
                                                                    SCPageSelectStrategyBase * strategy)
: KShapeManagerPaintingStrategy(shapeManager)
, m_animationCache(animationCache)
, m_strategy(strategy)
{
}

SCShapeManagerAnimationStrategy::~SCShapeManagerAnimationStrategy()
{
    delete m_strategy;
}

void SCShapeManagerAnimationStrategy::paint(KShape * shape, QPainter &painter, const KViewConverter &converter, bool forPrint)
{
    if (! dynamic_cast<SCPlaceholderShape *>(shape) && m_strategy->page()->displayShape(shape)) {
        if (m_animationCache->value(shape, "visibility", true).toBool()) {
            painter.save();
            QTransform animationTransform = m_animationCache->value(shape, "transform", QTransform()).value<QTransform>();;
            QTransform transform(painter.transform() * shape->absoluteTransformation(&converter));
            if (animationTransform.isScaling()) {
                transform = animationTransform * transform;
            } else {
                transform = transform * animationTransform;
            }

            painter.setTransform(transform);
            // paint shape
            shapeManager()->paintShape(shape, painter, converter, forPrint);
            painter.restore();  // for the transform
        }
    }
}

void SCShapeManagerAnimationStrategy::adapt(KShape * shape, QRectF & rect)
{
    Q_UNUSED(shape)
    Q_UNUSED(rect)
}
