/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "KFilterEffect.h"
#include "KXmlWriter.h"

#include <QString>
#include <QtCore/QRectF>

class KFilterEffect::Private
{
public:
    Private()
        : filterRect(0, 0, 1, 1)
        , requiredInputCount(1), maximalInputCount(1)
    {
        // add the default input
        inputs.append(QString());
    }

    QString id;
    QString name;
    QRectF filterRect;
    QList<QString> inputs;
    QString output;
    int requiredInputCount;
    int maximalInputCount;
};

KFilterEffect::KFilterEffect(const QString &id, const QString &name)
    : d(new Private)
{
    d->id = id;
    d->name = name;
}

KFilterEffect::~KFilterEffect()
{
    delete d;
}

QString KFilterEffect::name() const
{
    return d->name;
}

QString KFilterEffect::id() const
{
    return d->id;
}

void KFilterEffect::setFilterRect(const QRectF &filterRect)
{
    d->filterRect = filterRect;
}

QRectF KFilterEffect::filterRect() const
{
    return d->filterRect;
}

QRectF KFilterEffect::filterRectForBoundingRect(const QRectF &boundingRect) const
{
    qreal x = boundingRect.x() + d->filterRect.x() * boundingRect.width();
    qreal y = boundingRect.y() + d->filterRect.y() * boundingRect.height();
    qreal w = d->filterRect.width() * boundingRect.width();
    qreal h = d->filterRect.height() * boundingRect.height();
    return QRectF(x, y, w, h);
}

QList<QString> KFilterEffect::inputs() const
{
    return d->inputs;
}

void KFilterEffect::addInput(const QString &input)
{
    if (d->inputs.count() < d->maximalInputCount)
        d->inputs.append(input);
}

void KFilterEffect::insertInput(int index, const QString &input)
{
    if (d->inputs.count() < d->maximalInputCount)
        d->inputs.insert(index, input);
}

void KFilterEffect::setInput(int index, const QString &input)
{
    if (index < d->inputs.count())
        d->inputs[index] = input;
}

void KFilterEffect::removeInput(int index)
{
    if (d->inputs.count() > d->requiredInputCount)
        d->inputs.removeAt(index);
}

void KFilterEffect::setOutput(const QString &output)
{
    d->output = output;
}

QString KFilterEffect::output() const
{
    return d->output;
}

int KFilterEffect::requiredInputCount() const
{
    return d->requiredInputCount;
}

int KFilterEffect::maximalInputCount() const
{
    return qMax(d->maximalInputCount, d->requiredInputCount);
}

QImage KFilterEffect::processImages(const QList<QImage> &images, const KoFilterEffectRenderContext &/*context*/) const
{
    Q_ASSERT(images.count());
    return images.first();
}

void KFilterEffect::setRequiredInputCount(int count)
{
    d->requiredInputCount = qMax(0, count);
    for (int i = d->inputs.count(); i < d->requiredInputCount; ++i)
        d->inputs.append(QString());
}

void KFilterEffect::setMaximalInputCount(int count)
{
    d->maximalInputCount = qMax(0,count);
    if (d->inputs.count() > maximalInputCount()) {
        int removeCount = d->inputs.count()-maximalInputCount();
        for (int i = 0; i < removeCount; ++i)
            d->inputs.pop_back();
    }
}

void KFilterEffect::saveCommonAttributes(KXmlWriter &writer)
{
    writer.addAttribute("result", output());
    if (requiredInputCount() == 1 && maximalInputCount() == 1 && d->inputs.count() == 1) {
        writer.addAttribute("in", d->inputs[0]);
    }
    writer.addAttribute("x", d->filterRect.x());
    writer.addAttribute("y", d->filterRect.y());
    writer.addAttribute("width", d->filterRect.width());
    writer.addAttribute("height", d->filterRect.height());
}
