/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
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

#include "KPrAnimSet.h"

#include <QString>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoTextBlockData.h>
#include "KPrAnimationCache.h"
#include "KPrTextBlockPaintStrategy.h"
#include "KPrShapeAnimation.h"
#include "KoXmlWriter.h"
#include "KPrDurationParser.h"
#include <kdebug.h>

KPrAnimSet::KPrAnimSet(KPrShapeAnimation *shapeAnimation)
: KPrAnimationBase(shapeAnimation)
{
}

KPrAnimSet::~KPrAnimSet()
{
}

bool KPrAnimSet::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    bool retval = false;

    QString attributeName(element.attributeNS(KoXmlNS::smil, "attributeName", QString()));
    if (attributeName == "visibility") {
        m_visible = element.attributeNS(KoXmlNS::smil, "to", "hidden") == "visible";
        retval = true;
        kDebug(33003) << "animate visibility for shape with id" << m_visible;
    }
    else {
        kWarning(33003) << "attributeName" << attributeName << "not yet supported";
    }
    KPrAnimationBase::loadOdf(element, context);

    return retval;
}

bool KPrAnimSet::saveOdf(KoPASavingContext &paContext) const
{
    KoXmlWriter &writer = paContext.xmlWriter();
    writer.startElement("anim:set");
    saveAttribute(paContext);
    writer.endElement();
    return true;
}

bool KPrAnimSet::saveAttribute(KoPASavingContext &paContext) const
{
    KPrAnimationBase::saveAttribute(paContext);
    KoXmlWriter &writer = paContext.xmlWriter();
    // Anim set allow only visibility change currently
    writer.addAttribute("smil:attributeName","visibility");
    writer.addAttribute("smil:to", m_visible ? "visible" : "hidden");
    return true;
}


void KPrAnimSet::init(KPrAnimationCache *animationCache, int step)
{
    m_animationCache = animationCache;
    animationCache->init(step, m_shapeAnimation->shape(), m_shapeAnimation->textBlockData(), "visibility", !m_visible);
    animationCache->init(step + 1, m_shapeAnimation->shape(), m_shapeAnimation->textBlockData(), "visibility", m_visible);
}

void KPrAnimSet::next(int currentTime)
{
    Q_UNUSED(currentTime);
    updateCache("visibility", m_visible);
}
