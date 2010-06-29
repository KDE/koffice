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


#include "KPrAnimateColor.h"

#include "KPrAnimationCache.h"
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>

KPrAnimateColor::KPrAnimateColor(KPrShapeAnimation *shapeAnimation)
: KPrAnimationBase(shapeAnimation)
{
}

KPrAnimateColor::~KPrAnimateColor()
{
}

bool KPrAnimateColor::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KPrAnimationBase::loadOdf(element, context);
    return false;
}

bool KPrAnimateColor::saveOdf(KoPASavingContext & paContext) const
{
    Q_UNUSED(paContext);
    return true;
}

void KPrAnimateColor::init(KPrAnimationCache *animationCache, int step)
{
    Q_UNUSED(animationCache);
    Q_UNUSED(step);
}

void KPrAnimateColor::next(int currentTime)
{
    Q_UNUSED(currentTime);
}
