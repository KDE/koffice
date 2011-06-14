/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef KPRSHAPEANIMATION_H
#define KPRSHAPEANIMATION_H

#include <QParallelAnimationGroup>
#include "SCAnimationData.h"

class KShape;
class KoTextBlockData;
class KXmlElement;
class KShapeLoadingContext;
class KoShapeSavingContext;
class SCAnimationCache;
class KoPASavingContext;

class SCShapeAnimation : public QParallelAnimationGroup, SCAnimationData
{
public:
    SCShapeAnimation(KShape *shape, KoTextBlockData *textBlockData);
    virtual ~SCShapeAnimation();

    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    virtual bool saveOdf(KoPASavingContext &paContext, bool startStep, bool startSubStep) const;

    KShape * shape() const;
    KoTextBlockData * textBlockData() const;

    virtual void init(SCAnimationCache *animationCache, int step);

    virtual void deactivate();
    /**
     * @return true if this shape animation change the visibility
     */
    bool visibilityChange();

    /**
     * @return true if this shape animation enable visibility of the shape
     */
    bool visible();

    /**
     * Read the value from the first SCAnimationBase object
     */
    //QPair<KShape *, KoTextBlockData *> animationShape() const;

private:
    KShape *m_shape;
    KoTextBlockData *m_textBlockData;
};

#endif /* KPRSHAPEANIMATION_H */
