/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KLineBorder.h"
#include "KoViewConverter.h"
#include "KShape.h"
#include "KShapeSavingContext.h"

#include <QPainter>
#include <QPainterPath>

#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KOdf.h>

#include <math.h>

class KLineBorder::Private
{
public:
    QColor color;
    QPen pen;
    QBrush brush;
};

KLineBorder::KLineBorder()
        : d(new Private())
{
    d->color = QColor(Qt::black);
    // we are not rendering stroke with zero width anymore
    // so lets use a default width of 1.0
    d->pen.setWidthF(1.0);
}

KLineBorder::KLineBorder(qreal lineWidth, const QColor &color)
        : d(new Private())
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
    d->pen.setJoinStyle(Qt::MiterJoin);
    d->color = color;
}

KLineBorder::~KLineBorder()
{
    delete d;
}

KLineBorder &KLineBorder::operator = (const KLineBorder &rhs)
{
    if (this == &rhs)
        return *this;

    d->pen = rhs.d->pen;
    d->color = rhs.d->color;
    d->brush = rhs.d->brush;

    return *this;
}

void KLineBorder::saveOdf(KOdfGenericStyle &style, KShapeSavingContext &context) const
{
    QPen pen = d->pen;
    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);
    KOdf::saveOdfStrokeStyle(style, context.mainStyles(), pen);
}

void KLineBorder::borderInsets(KoInsets &insets) const
{
    qreal lineWidth = d->pen.widthF();
    if (lineWidth < 0)
        lineWidth = 1;
    lineWidth *= 0.5; // since we draw a line half inside, and half outside the object.

    // if we have square cap, we need a little more space
    // -> sqrt((0.5*penWidth)^2 + (0.5*penWidth)^2)
    if (capStyle() == Qt::SquareCap)
        lineWidth *= M_SQRT2;

    insets.top = lineWidth;
    insets.bottom = lineWidth;
    insets.left = lineWidth;
    insets.right = lineWidth;
}

bool KLineBorder::hasTransparency() const
{
    return d->color.alpha() > 0;
}

void KLineBorder::paint(KShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    KShape::applyConversion(painter, converter);

    QPen pen = d->pen;

    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);

    if (!pen.isCosmetic())
        painter.strokePath(shape->outline(), pen);
}

void KLineBorder::paint(KShape *shape, QPainter &painter, const KoViewConverter &converter, const QColor &color)
{
    KShape::applyConversion(painter, converter);

    QPen pen = d->pen;
    pen.setColor(color);

    if (!pen.isCosmetic()) {
        painter.strokePath(shape->outline(), pen);
    }
}

void KLineBorder::setCapStyle(Qt::PenCapStyle style)
{
    d->pen.setCapStyle(style);
}

Qt::PenCapStyle KLineBorder::capStyle() const
{
    return d->pen.capStyle();
}

void KLineBorder::setJoinStyle(Qt::PenJoinStyle style)
{
    d->pen.setJoinStyle(style);
}

Qt::PenJoinStyle KLineBorder::joinStyle() const
{
    return d->pen.joinStyle();
}

void KLineBorder::setLineWidth(qreal lineWidth)
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
}

qreal KLineBorder::lineWidth() const
{
    return d->pen.widthF();
}

void KLineBorder::setMiterLimit(qreal miterLimit)
{
    d->pen.setMiterLimit(miterLimit);
}

qreal KLineBorder::miterLimit() const
{
    return d->pen.miterLimit();
}

QColor KLineBorder::color() const
{
    return d->color;
}

void KLineBorder::setColor(const QColor &color)
{
    d->color = color;
}

void KLineBorder::setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes)
{
    if (style < Qt::CustomDashLine)
        d->pen.setStyle(style);
    else
        d->pen.setDashPattern(dashes);
}

Qt::PenStyle KLineBorder::lineStyle() const
{
    return d->pen.style();
}

QVector<qreal> KLineBorder::lineDashes() const
{
    return d->pen.dashPattern();
}

void KLineBorder::setDashOffset(qreal dashOffset)
{
    d->pen.setDashOffset(dashOffset);
}

qreal KLineBorder::dashOffset() const
{
    return d->pen.dashOffset();
}

void KLineBorder::setLineBrush(const QBrush &brush)
{
    d->brush = brush;
}

QBrush KLineBorder::lineBrush() const
{
    return d->brush;
}
