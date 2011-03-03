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

#include "SCAnimationBase.h"

#include <KoXmlNS.h>
#include "SCDurationParser.h"
#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include "SCAnimationCache.h"
#include "SCShapeAnimation.h"
#include "KoShapeLoadingContext.h"
#include "KoTextBlockData.h"

SCAnimationBase::SCAnimationBase(SCShapeAnimation *shapeAnimation)
: m_shapeAnimation(shapeAnimation)
, m_begin(0)
,m_duration(1)
{
}

SCAnimationBase::~SCAnimationBase()
{
}

int SCAnimationBase::duration() const
{
    return m_duration;
}

bool SCAnimationBase::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context)
    m_begin = SCDurationParser::durationMs(element.attributeNS(KoXmlNS::smil, "begin"));
    if (m_begin == -1) {
        m_begin = 0;
    }
    m_duration = SCDurationParser::durationMs(element.attributeNS(KoXmlNS::smil, "dur"));
    if (m_duration == -1) {
        m_duration = 1;
    }
    m_duration += m_begin;
    return true;
}

void SCAnimationBase::updateCache(const QString &id, const QVariant &value)
{
    m_animationCache->update(m_shapeAnimation->shape(), m_shapeAnimation->textBlockData(), id, value);
}

void SCAnimationBase::updateCurrentTime(int currentTime)
{
    if (currentTime >= m_begin) {
        next(currentTime - m_begin);
    }
}

int SCAnimationBase::animationDuration() const
{
    return totalDuration() - m_begin;
}

bool SCAnimationBase::saveAttribute(KoPASavingContext &paContext) const
{
    KoXmlWriter &writer = paContext.xmlWriter();
    writer.addAttribute("smil:begin", SCDurationParser::msToString(m_begin));
    writer.addAttribute("smil:dur", SCDurationParser::msToString(m_duration));
    if (m_shapeAnimation->textBlockData()) {
        writer.addAttribute("smil:targetElement", paContext.subId(m_shapeAnimation->textBlockData(), false));
        writer.addAttribute("anim:sub-item", "text");
    }
    else {
        writer.addAttribute("smil:targetElement", paContext.drawId(m_shapeAnimation->shape(), false));
    }
    return true;
}
