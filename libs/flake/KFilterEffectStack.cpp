/* This file is part of the KDE project
 * Copyright (c) 2009-2010 Jan Hambrecht <jaham@gmx.net>
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

#include "KFilterEffectStack.h"
#include "KFilterEffect.h"
#include "KXmlWriter.h"

#include <QtCore/QAtomicInt>
#include <QtCore/QSet>
#include <QDebug>

class KFilterEffectStack::Private
{
public:
    Private()
    : clipRect(-0.1, -0.1, 1.2, 1.2) // initialize as per svg spec
    {
    }

    ~Private()
    {
        qDeleteAll(filterEffects);
    }

    QList<KFilterEffect*> filterEffects;
    QRectF clipRect;
    QAtomicInt refCount;
};

KFilterEffectStack::KFilterEffectStack()
: d(new Private())
{
}

KFilterEffectStack::~KFilterEffectStack()
{
    delete d;
}

QList<KFilterEffect*> KFilterEffectStack::filterEffects() const
{
    return d->filterEffects;
}

bool KFilterEffectStack::isEmpty() const
{
    return d->filterEffects.isEmpty();
}

void KFilterEffectStack::insertFilterEffect(int index, KFilterEffect * filter)
{
    if (filter)
        d->filterEffects.insert(index, filter);
}

void KFilterEffectStack::appendFilterEffect(KFilterEffect *filter)
{
    if (filter)
        d->filterEffects.append(filter);
}

void KFilterEffectStack::removeFilterEffect(int index)
{
    KFilterEffect * filter = takeFilterEffect(index);
    delete filter;
}

KFilterEffect* KFilterEffectStack::takeFilterEffect(int index)
{
    if (index >= d->filterEffects.size())
        return 0;
    return d->filterEffects.takeAt(index);
}

void KFilterEffectStack::setClipRect(const QRectF &clipRect)
{
    d->clipRect = clipRect;
}

QRectF KFilterEffectStack::clipRect() const
{
    return d->clipRect;
}

QRectF KFilterEffectStack::clipRectForBoundingRect(const QRectF &boundingRect) const
{
    qreal x = boundingRect.x() + d->clipRect.x() * boundingRect.width();
    qreal y = boundingRect.y() + d->clipRect.y() * boundingRect.height();
    qreal w = d->clipRect.width() * boundingRect.width();
    qreal h = d->clipRect.height() * boundingRect.height();
    return QRectF(x, y, w, h);
}

bool KFilterEffectStack::ref()
{
    return d->refCount.ref();
}

bool KFilterEffectStack::deref()
{
    return d->refCount.deref();
}

int KFilterEffectStack::useCount() const
{
    return d->refCount;
}

void KFilterEffectStack::save(KXmlWriter &writer, const QString &filterId)
{
    writer.startElement("filter");
    writer.addAttribute("id", filterId);
    writer.addAttribute("filterUnits", "objectBoundingBox");
    writer.addAttribute("primitiveUnits", "objectBoundingBox");
    writer.addAttribute("x", d->clipRect.x());
    writer.addAttribute("y", d->clipRect.y());
    writer.addAttribute("width", d->clipRect.width());
    writer.addAttribute("height", d->clipRect.height());

    foreach(KFilterEffect *effect, d->filterEffects) {
        effect->save(writer);
    }

    writer.endElement();
}

QSet<QString> KFilterEffectStack::requiredStandarsInputs() const
{
    static QSet<QString> stdInputs = QSet<QString>()
        << "SourceGraphic"
        << "SourceAlpha"
        << "BackgroundImage"
        << "BackgroundAlpha"
        << "FillPaint"
        << "StrokePaint";

    QSet<QString> requiredInputs;
    if (isEmpty())
        return requiredInputs;

    if (d->filterEffects.first()->inputs().contains(""))
        requiredInputs.insert("SourceGraphic");

    foreach(KFilterEffect *effect, d->filterEffects) {
        foreach(const QString &input, effect->inputs()) {
            if (stdInputs.contains(input))
                requiredInputs.insert(input);
        }
    }

    return requiredInputs;
}
