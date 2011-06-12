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

#include "SCAnimateMotion.h"

#include "SCAnimationCache.h"

#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KXmlReader.h>

SCAnimateMotion::SCAnimateMotion(SCShapeAnimation *shapeAnimation)
: SCAnimationBase(shapeAnimation)
{
}

SCAnimateMotion::~SCAnimateMotion()
{
}

bool SCAnimateMotion::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    SCAnimationBase::loadOdf(element, context);
    return false;
}

bool SCAnimateMotion::saveOdf(KoPASavingContext &paContext) const
{
    Q_UNUSED(paContext);
    return true;
}


void SCAnimateMotion::init(SCAnimationCache *animationCache, int step)
{
    Q_UNUSED(animationCache);
    Q_UNUSED(step);
}

void SCAnimateMotion::next(int currentTime)
{
    Q_UNUSED(currentTime);
}
