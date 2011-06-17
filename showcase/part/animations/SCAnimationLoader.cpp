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

#include "SCAnimationLoader.h"

#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KShapeLoadingContext.h>
#include <KTextBlockData.h>

#include <QVariant>
#include <QDomDocument>
#include <QDomNode>

#include <kdebug.h>

#include "SCShapeAnimation.h"
#include "SCAnimationBase.h"
#include "SCAnimationFactory.h"
#include "SCAnimationStep.h"
#include "SCAnimationSubStep.h"

SCAnimationLoader::SCAnimationLoader()
{
}

SCAnimationLoader::~SCAnimationLoader()
{
}

void debugXml(const QString &pos, const KXmlElement &element)
{
    QByteArray array;
    QDomDocument doc;
    QTextStream st(&array);
    st << element.asQDomNode(doc);
    kDebug() << pos << array;
}

bool SCAnimationLoader::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    // have an overall structure for animations each step needs to be in its own QSequentialAnimationGroup subclass
    // use SCAnimationStep for that
    KXmlElement stepElement;
    forEachElement(stepElement, element) {
        if (stepElement.tagName() == "par" && stepElement.namespaceURI() == KOdfXmlNS::anim) {
            // this creates a new step
            SCAnimationStep *animationStep = new SCAnimationStep();

            KXmlElement parElement;
            forEachElement(parElement, stepElement) {
                KXmlElement innerParElement;
                forEachElement(innerParElement, parElement) {
                    if (innerParElement.tagName() == "par" && innerParElement.namespaceURI() == KOdfXmlNS::anim) {
                        loadOdfAnimation(&animationStep, innerParElement, context);
                    }
                }
            }
            m_animations.append(animationStep);
        }
        else {
            Q_ASSERT( 0 );
            // accoring to spec there should be onla par elements
        }
    }

    debug();

    return true;
}

void SCAnimationLoader::debug()
{
    foreach (SCAnimationStep *step, m_animations) {
        kDebug() << "step";
        debug(step, 1);
    }
}

void SCAnimationLoader::debug(QAbstractAnimation *animation, int level)
{
    QString indent;
    for (int i = 0; i < level; ++i) {
        indent += ' ';
    }
    if (SCAnimationStep *a = dynamic_cast<SCAnimationStep*>(animation)) {
        Q_UNUSED(a);
        kDebug() << indent + "animation step";
    }
    else if (SCAnimationSubStep *a = dynamic_cast<SCAnimationSubStep*>(animation)) {
        Q_UNUSED(a);
        kDebug() << indent + "animation sub step";
    }
    else if (SCShapeAnimation *a = dynamic_cast<SCShapeAnimation*>(animation)) {
        Q_UNUSED(a);
        kDebug() << indent + "shape animation";
    }
    else if (SCAnimationBase *a = dynamic_cast<SCAnimationBase*>(animation)) {
        Q_UNUSED(a);
        kDebug() << indent + "animation base";
    }

    if (QAnimationGroup *group = dynamic_cast<QAnimationGroup*>(animation)) {
        for (int i = 0; i < group->animationCount(); ++i) {
            debug(group->animationAt(i), level + 1);
        }
    }
}

bool SCAnimationLoader::loadOdfAnimation(SCAnimationStep **animationStep, const KXmlElement &element, KShapeLoadingContext &context)
{
    QString nodeType = element.attributeNS(KOdfXmlNS::presentation, "node-type", "with-previous");

    kDebug() << "nodeType:" << nodeType;
    SCAnimationSubStep *subStep = 0;
    if (nodeType == "on-click") {
        // if there is allready an aniation create a new step
        if ((*animationStep)->animationCount() != 0 || m_animations.isEmpty()) {
            m_animations.append(*animationStep);
            *animationStep = new SCAnimationStep();
        }
        subStep = new SCAnimationSubStep();
        (*animationStep)->addAnimation(subStep);
        // add par animation
    }
    else if (nodeType == "after-previous") {
        // add to sequence
        // add par
        subStep = new SCAnimationSubStep();
        (*animationStep)->addAnimation(subStep);
        // add par animation
    }
    else {
        if (nodeType != "with-previous") {
            kWarning(33003) << "unsupported node-type" << nodeType << "found. Using with-previous";
        }
        // use the current substep
        if ((*animationStep)->animationCount()) {
            subStep = dynamic_cast<SCAnimationSubStep*>((*animationStep)->animationAt((*animationStep)->animationCount() - 1));
        }
        else {
            subStep = new SCAnimationSubStep();
            (*animationStep)->addAnimation(subStep);
        }
        // add par to current par
    }

    SCShapeAnimation *shapeAnimation = 0;
    // The shape info and create a SCShapeAnimation. If there is
    KXmlElement e;
    forEachElement(e, element) {
        // TODO add a check that the shape animation is still the correct one
        if (shapeAnimation == 0) {
            QString targetElement(e.attributeNS(KOdfXmlNS::smil, "targetElement", QString()));
            if (!targetElement.isEmpty()) {
                KShape *shape = 0;
                KTextBlockData *textBlockData = 0;

                if (e.attributeNS(KOdfXmlNS::anim, "sub-item", "whole") == "text") {
                    QPair<KShape *, QVariant> pair = context.shapeSubItemById(targetElement);
                    shape = pair.first;
                    textBlockData = pair.second.value<KTextBlockData *>();
                    kDebug() << "subitem text" << textBlockData;
                }
                else {
                    shape = context.shapeById(targetElement);
                }
                kDebug() << "shape:" << shape << "textBlockData" << textBlockData;

                if (shape) {
                    shapeAnimation = new SCShapeAnimation(shape, textBlockData);
                }
                else {
                    // shape animation not created
                    // TODO logging
                    continue;
                }
            }
        }

        SCAnimationBase *animation(SCAnimationFactory::createAnimationFromOdf(e, context, shapeAnimation));
        if (shapeAnimation && animation) {
            shapeAnimation->addAnimation(animation);
        }
    }

    if (shapeAnimation) {
        subStep->addAnimation(shapeAnimation);
    }
    return true;
}

QList<SCAnimationStep *> SCAnimationLoader::animations()
{
    return m_animations;
}
