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

#include "SCAnimate.h"
#include "SCAnimationCache.h"
#include "SCShapeAnimation.h"

#include "strategy/SCSmilValues.h"
#include "strategy/SCAnimationValue.h"
#include "strategy/SCAnimationAttribute.h"
#include "strategy/SCAttributeX.h"
#include "strategy/SCAttributeY.h"
#include "strategy/SCAttributeWidth.h"
#include "strategy/SCAttributeHeight.h"
#include "strategy/SCAttributeRotate.h"

#include <KOdfXmlNS.h>
#include <KoShape.h>
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoTextBlockData.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoPASavingContext.h>

#include <kdebug.h>

SCAnimate::SCAnimate(SCShapeAnimation *shapeAnimation)
: SCAnimationBase(shapeAnimation)
,m_attribute(0)
,m_values(0)
{
}

SCAnimate::~SCAnimate()
{
    if(m_attribute)
        delete m_attribute;
    if(m_values)
        delete m_values;
}

bool SCAnimate::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    SCAnimationBase::loadOdf(element, context);
    bool retval = true;
    // attributeName
    QString attributeName(element.attributeNS(KOdfXmlNS::smil, "attributeName", QString()));
    if (attributeName == "x") {
        m_attribute = new SCAttributeX();
    }
    else if (attributeName == "y") {
        m_attribute = new SCAttributeY();
    }
    else if (attributeName == "width") {
        m_attribute = new SCAttributeWidth();
    }
    else if (attributeName == "height") {
        m_attribute = new SCAttributeHeight();
    }
    else if (attributeName == "rotate") {
        m_attribute = new SCAttributeRotate();
    }
    else {
        kWarning(33003) << "attributeName" << attributeName << "not yet supported";
        retval = false;
    }

    if (!retval){
        return false;
    }

    // calcMode
    SCAnimationValue::SmilCalcMode smilCalcMode = SCAnimationValue::linear;
    QString calcMode = element.attributeNS(KOdfXmlNS::smil, "calcMode", "linear");
    if(calcMode == "linear"){
        smilCalcMode = SCAnimationValue::linear;
    } else if (calcMode == "discrete") {
        smilCalcMode = SCAnimationValue::discrete;
        kWarning(33003) << "calcMode discrete not yes supported";
        retval = false;
    } else if (calcMode == "paced") {
        smilCalcMode = SCAnimationValue::paced;
        kWarning(33003) << "calcMode paced not yes supported";
        retval = false;
    } else if (calcMode == "spline") {
        smilCalcMode = SCAnimationValue::spline;
        kWarning(33003) << "calcMode spline not yes supported";
        retval = false;
    }


    // value
    QString formula = element.attributeNS(KOdfXmlNS::anim, "formula", QString());
    if (!formula.isEmpty()) {
        kWarning(33003) << "formula not yes supported";
        retval = false;
    }
    else {
        QString values = element.attributeNS(KOdfXmlNS::smil, "values", QString());
        if (!values.isEmpty()) {
            QString keyTimes = element.attributeNS(KOdfXmlNS::smil, "keyTimes", QString());
            QString keySplines = element.attributeNS(KOdfXmlNS::smil, "keySplines", QString());
            SCSmilValues * smilValue = new SCSmilValues(m_shapeAnimation);
            retval = retval && smilValue->loadValues(values, keyTimes, keySplines, smilCalcMode);
            m_values = smilValue;
        }
        else {
            QString from = element.attributeNS(KOdfXmlNS::smil, "from", "0");
            QString to = element.attributeNS(KOdfXmlNS::smil, "to", "0");
            QString by = element.attributeNS(KOdfXmlNS::smil, "by", "0");
            kWarning(33003) << "from to by not yes supported";
            retval = false;
        }
    }
    return retval;
}

bool SCAnimate::saveOdf(KoPASavingContext &paContext) const
{
    KoXmlWriter &writer = paContext.xmlWriter();
    writer.startElement("anim:animate");
    saveAttribute(paContext);
    writer.endElement();
    return true;
}

void SCAnimate::init(SCAnimationCache *animationCache, int step)
{
    m_animationCache = animationCache;
    m_values->setCache(m_animationCache);
    m_attribute->initCache(animationCache, step, m_shapeAnimation, m_values->startValue(), m_values->endValue());
}

void SCAnimate::next(int currentTime)
{
    qreal value = m_values->value(qreal(currentTime)/qreal(animationDuration()));
    m_attribute->updateCache(m_animationCache, m_shapeAnimation, value);
}

bool SCAnimate::saveAttribute(KoPASavingContext &paContext) const
{
    SCAnimationBase::saveAttribute(paContext);
    KoXmlWriter &writer = paContext.xmlWriter();
    writer.addAttribute("smil:attributeName", m_attribute->attributeName());
    m_values->saveOdf(paContext);
    return true;
}
