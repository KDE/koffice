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
#include <QPainter>
#include <QImage>
#include <math.h>

class KShapeBorderBase::Private
{
public:
    Private() : refCount(0), pen(QPen(QColor(Qt::black), 1)) { }

    QAtomicInt refCount;
    QPen pen;
    QString borderId;
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

void KShapeBorderBase::setPen(const QPen &pen)
{
    d->pen = pen;
}

QPen KShapeBorderBase::pen() const
{
    return d->pen;
}

KInsets KShapeBorderBase::borderInsets() const
{
    KInsets insets;
    qreal lineWidth = pen().widthF();
    if (lineWidth < 0)
        lineWidth = 1;
    lineWidth *= 0.5; // since we draw a line half inside, and half outside the object.

    // if we have square cap, we need a little more space
    // -> sqrt((0.5*penWidth)^2 + (0.5*penWidth)^2)
    if (pen().capStyle() == Qt::SquareCap)
        lineWidth *= M_SQRT2;

    insets.top = lineWidth;
    insets.bottom = lineWidth;
    insets.left = lineWidth;
    insets.right = lineWidth;
    return insets;
}

bool KShapeBorderBase::hasTransparency() const
{
    return pen().color().alpha() > 0;
}

void KShapeBorderBase::setBorderId(const QString &id)
{
    d->borderId = id;
}

QString KShapeBorderBase::borderId() const
{
    return d->borderId;
}
