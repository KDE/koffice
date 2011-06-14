/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MORPHOLOGYEFFECT_H
#define MORPHOLOGYEFFECT_H

#include "KFilterEffect.h"
#include <QtCore/QPointF>

#define MorphologyEffectId "feMorphology"

class KFilterEffectLoadingContext;

/// A morphology filter effect
class MorphologyEffect : public KFilterEffect
{
public:
    /// Morphology operator type
    enum Operator {
        Erode,
        Dilate
    };

    MorphologyEffect();

    /// Returns the morphology radius
    QPointF morphologyRadius() const;

    /// Sets the morphology radius
    void setMorphologyRadius(const QPointF &radius);

    /// Returns the morphology operator
    Operator morphologyOperator() const;

    /// Sets the morphology operator
    void setMorphologyOperator(Operator op);

    /// reimplemented from KFilterEffect
    virtual QImage processImage(const QImage &image, const KoFilterEffectRenderContext &context) const;
    /// reimplemented from KFilterEffect
    virtual bool load(const KXmlElement &element, const KFilterEffectLoadingContext &context);
    /// reimplemented from KFilterEffect
    virtual void save(KXmlWriter &writer);

private:
    QPointF m_radius;
    Operator m_operator;
};

#endif // MORPHOLOGYEFFECT_H
