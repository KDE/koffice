/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KFilterEffectRenderContext.h"
#include "KViewConverter.h"

#include <QtCore/QRectF>
#include <QtGui/QTransform>

class KFilterEffectRenderContext::Private
{
public:
    Private(const KViewConverter &viewConverter)
        : converter(viewConverter)
    {}

    QRectF filterRegion;
    QRectF shapeBound;
    const KViewConverter & converter;
};

KFilterEffectRenderContext::KFilterEffectRenderContext(const KViewConverter &converter)
: d(new Private(converter))
{
}

KFilterEffectRenderContext::~KFilterEffectRenderContext()
{
    delete d;
}

QRectF KFilterEffectRenderContext::filterRegion() const
{
    return d->filterRegion;
}

void KFilterEffectRenderContext::setFilterRegion(const QRectF &filterRegion)
{
    d->filterRegion = filterRegion;
}

void KFilterEffectRenderContext::setShapeBoundingBox(const QRectF &bound)
{
    d->shapeBound = bound;
}

QPointF KFilterEffectRenderContext::toUserSpace(const QPointF &value) const
{
    return QPointF(value.x()*d->shapeBound.width(), value.y()*d->shapeBound.height());
}

qreal KFilterEffectRenderContext::toUserSpaceX(qreal value) const
{
    return value * d->shapeBound.width();
}

qreal KFilterEffectRenderContext::toUserSpaceY(qreal value) const
{
    return value * d->shapeBound.height();
}

const KViewConverter * KFilterEffectRenderContext::viewConverter() const
{
    return &d->converter;
}
