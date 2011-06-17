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

#include "SCAnimSet.h"

#include "SCDurationParser.h"
#include "SCAnimationCache.h"
#include "SCTextBlockPaintStrategy.h"
#include "SCShapeAnimation.h"

#include <KoPASavingContext.h>
#include <QString>
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KTextBlockData.h>
#include "KXmlWriter.h"
#include <kdebug.h>

SCAnimSet::SCAnimSet(SCShapeAnimation *shapeAnimation)
: SCAnimationBase(shapeAnimation)
{
}

SCAnimSet::~SCAnimSet()
{
}

bool SCAnimSet::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    bool retval = false;

    QString attributeName(element.attributeNS(KOdfXmlNS::smil, "attributeName", QString()));
    if (attributeName == "visibility") {
        m_visible = element.attributeNS(KOdfXmlNS::smil, "to", "hidden") == "visible";
        retval = true;
        kDebug(33003) << "animate visibility for shape with id" << m_visible;
    }
    else {
        kWarning(33003) << "attributeName" << attributeName << "not yet supported";
    }
    SCAnimationBase::loadOdf(element, context);

    return retval;
}

bool SCAnimSet::saveOdf(KoPASavingContext &paContext) const
{
    KXmlWriter &writer = paContext.xmlWriter();
    writer.startElement("anim:set");
    saveAttribute(paContext);
    writer.endElement();
    return true;
}

bool SCAnimSet::saveAttribute(KoPASavingContext &paContext) const
{
    SCAnimationBase::saveAttribute(paContext);
    KXmlWriter &writer = paContext.xmlWriter();
    // Anim set allow only visibility change currently
    writer.addAttribute("smil:attributeName","visibility");
    writer.addAttribute("smil:to", m_visible ? "visible" : "hidden");
    return true;
}


void SCAnimSet::init(SCAnimationCache *animationCache, int step)
{
    m_animationCache = animationCache;
    animationCache->init(step, m_shapeAnimation->shape(), m_shapeAnimation->textBlockData(), "visibility", !m_visible);
    animationCache->init(step + 1, m_shapeAnimation->shape(), m_shapeAnimation->textBlockData(), "visibility", m_visible);
}

void SCAnimSet::next(int currentTime)
{
    Q_UNUSED(currentTime);
    updateCache("visibility", m_visible);
}
