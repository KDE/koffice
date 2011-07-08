/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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

#include "KShapeBorderBase.h"
#include "KShape.h"
#include "KViewConverter.h"

#include <QAtomicInt>
#include <QRectF>
#include <QPainter>
#include <QImage>

class KShapeBorderBase::Private
{
public:
    Private() : refCount(0) { }
    QAtomicInt refCount;
};

KShapeBorderBase::KShapeBorderBase()
        : d(new Private())
{
}

KShapeBorderBase::~KShapeBorderBase()
{
    delete d;
}

bool KShapeBorderBase::ref()
{
    return d->refCount.ref();
}

bool KShapeBorderBase::deref()
{
    return d->refCount.deref();
}

int KShapeBorderBase::useCount() const
{
    return d->refCount;
}

void KShapeBorderBase::paint(KShape *shape, QPainter &painter, const KViewConverter &converter, const QColor &color)
{
    QSizeF size(converter.documentToView(shape->boundingRect()).size());
    // using a QImage
    QImage buffer = QImage(size.toSize(), QImage::Format_ARGB32_Premultiplied);
    buffer.fill(0);
    QPainter p(&buffer);
    paint(shape, p, converter);

    if (color.isValid()) {
        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(0, 0, buffer.width(), buffer.height(), color);
    }
    painter.drawImage(0, 0, buffer);
}
