/* This file is part of the KDE project
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

#ifndef SCANIMATIONVALUE_H
#define SCANIMATIONVALUE_H

#include <QtGlobal>

class KShape;
class KTextBlockData;
class SCAnimationCache;
class SCShapeAnimation;
class KoPASavingContext;

class SCAnimationValue
{
public:
    SCAnimationValue(SCShapeAnimation * shapeAnimation);
    virtual ~SCAnimationValue();
    virtual qreal value(qreal time) const = 0;
    virtual qreal startValue() const = 0;
    virtual qreal endValue() const = 0;
    virtual void setCache(SCAnimationCache * cache);
    virtual bool saveOdf(KoPASavingContext &paContext) const = 0;
    enum SmilCalcMode{
        discrete,
        linear,
        paced,
        spline
    };
protected:
    SmilCalcMode m_calcMode;
    SCAnimationCache * m_cache;
    KShape * m_shape;
    KTextBlockData * m_textBlockData;
};

#endif // SCANIMATIONVALUE_H
