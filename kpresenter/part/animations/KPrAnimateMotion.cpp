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

#include "KPrAnimateMotion.h"

#include "KPrAnimationCache.h"

#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>

KPrAnimateMotion::KPrAnimateMotion(KPrShapeAnimation *shapeAnimation)
: KPrAnimationBase(shapeAnimation)
{
}

KPrAnimateMotion::~KPrAnimateMotion()
{
}

bool KPrAnimateMotion::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KPrAnimationBase::loadOdf(element, context);
    return false;
}

bool KPrAnimateMotion::saveOdf(KoPASavingContext & paContext) const
{
    Q_UNUSED(paContext);
    return true;
}


void KPrAnimateMotion::init(KPrAnimationCache *animationCache, int step)
{
    Q_UNUSED(animationCache);
    Q_UNUSED(step);
}

void KPrAnimateMotion::next(int currentTime)
{
    Q_UNUSED(currentTime);
}
