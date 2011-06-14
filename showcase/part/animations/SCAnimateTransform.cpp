/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "SCAnimateTransform.h"

#include "SCAnimationCache.h"
#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KXmlReader.h>

SCAnimateTransform::SCAnimateTransform(SCShapeAnimation *shapeAnimation)
: SCAnimationBase(shapeAnimation)
{
    m_duration = 1;
}

SCAnimateTransform::~SCAnimateTransform()
{
}

bool SCAnimateTransform::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    SCAnimationBase::loadOdf(element, context);
    return true;
}

bool SCAnimateTransform::saveOdf(KoPASavingContext &paContext) const
{
    Q_UNUSED(paContext);
    return true;
}

void SCAnimateTransform::init(SCAnimationCache *animationCache, int step)
{
    Q_UNUSED(animationCache);
    Q_UNUSED(step);
}

void SCAnimateTransform::next(int currentTime)
{
    Q_UNUSED(currentTime);
}
