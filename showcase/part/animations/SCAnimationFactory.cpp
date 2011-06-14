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

#include "SCAnimationFactory.h"

#include "SCAnimSet.h"
#include "SCAnimate.h"
#include "SCAnimateColor.h"
#include "SCAnimateMotion.h"
#include "SCAnimateTransform.h"
#include "SCAnimTransitionFilter.h"

#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KShapeLoadingContext.h>

SCAnimationBase * SCAnimationFactory::createAnimationFromOdf(const KXmlElement &element, KShapeLoadingContext &context,
                                                               SCShapeAnimation *shapeAnimation)
{
    SCAnimationBase * animation = 0;
    if (element.namespaceURI() == KOdfXmlNS::anim) {
        if (element.tagName() == "set") {
            animation = new SCAnimSet(shapeAnimation);
        }
        else if (element.tagName() == "animate") {
            animation = new SCAnimate(shapeAnimation);
        }
        else if (element.tagName() == "animateMotion") {
            animation = new SCAnimateMotion(shapeAnimation);
        }
        else if (element.tagName() == "animateColor") {
            animation = new SCAnimateColor(shapeAnimation);
        }
        else if (element.tagName() == "animationTransform") {
            animation = new SCAnimateTransform(shapeAnimation);
        }
        else if (element.tagName() == "transitionFilter") {
            animation = new SCAnimTransitionFilter(shapeAnimation);
        }

        if (animation) {
            if (!animation->loadOdf(element, context)) {
                delete animation;
                animation = 0;
            }
            else {
            }
        }
        else {
        }
    }
    return animation;
}
