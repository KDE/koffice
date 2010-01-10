/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoGradientBackground.h"
#include "KoShapeBackground_p.h"
#include "KoFlake.h"
#include <KoStyleStack.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfGraphicStyles.h>
#include <KoShapeSavingContext.h>

#include <KDebug>

#include <QtGui/QBrush>
#include <QtGui/QPainter>

class KoGradientBackgroundPrivate : public KoShapeBackgroundPrivate
{
public:
    KoGradientBackgroundPrivate() : gradient(0) {};
    ~KoGradientBackgroundPrivate() {
        delete gradient;
    }

    QGradient * gradient;
    QMatrix matrix;
};

KoGradientBackground::KoGradientBackground(QGradient * gradient, const QMatrix &matrix)
    : KoShapeBackground(*(new KoGradientBackgroundPrivate()))
{
    Q_D(KoGradientBackground);
    d->gradient = gradient;
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoGradientBackground::KoGradientBackground(const QGradient & gradient, const QMatrix &matrix)
    : KoShapeBackground(*(new KoGradientBackgroundPrivate()))
{
    Q_D(KoGradientBackground);
    d->gradient = KoFlake::cloneGradient(&gradient);
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoGradientBackground::~KoGradientBackground()
{
}

void KoGradientBackground::setMatrix(const QMatrix &matrix)
{
    Q_D(KoGradientBackground);
    d->matrix = matrix;
}

QMatrix KoGradientBackground::matrix() const
{
    Q_D(const KoGradientBackground);
    return d->matrix;
}

void KoGradientBackground::setGradient(QGradient * gradient)
{
    Q_D(KoGradientBackground);
    if (d->gradient)
        delete d->gradient;

    d->gradient = gradient;
    Q_ASSERT(d->gradient);
}

void KoGradientBackground::setGradient(const QGradient &gradient)
{
    Q_D(KoGradientBackground);
    if (d->gradient)
        delete d->gradient;

    d->gradient = KoFlake::cloneGradient(&gradient);
    Q_ASSERT(d->gradient);
}

const QGradient * KoGradientBackground::gradient() const
{
    Q_D(const KoGradientBackground);
    return d->gradient;
}

KoGradientBackground &KoGradientBackground::operator = (const KoGradientBackground &rhs)
{
    Q_D(KoGradientBackground);
    if (this == &rhs)
        return *this;

    KoGradientBackgroundPrivate *other = static_cast<KoGradientBackgroundPrivate*>(rhs.d_ptr);

    d->matrix = other->matrix;
    delete d->gradient;
    d->gradient = KoFlake::cloneGradient(other->gradient);
    Q_ASSERT(d->gradient);

    return *this;
}

void KoGradientBackground::paint(QPainter &painter, const QPainterPath &fillPath) const
{
    Q_D(const KoGradientBackground);
    QBrush brush(*d->gradient);
    brush.setMatrix(d->matrix);

    painter.setBrush(brush);
    painter.drawPath(fillPath);
}

void KoGradientBackground::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    Q_D(KoGradientBackground);
    QBrush brush(*d->gradient);
    brush.setMatrix(d->matrix);
    KoOdfGraphicStyles::saveOdfFillStyle(style, context.mainStyles(), brush);
}

bool KoGradientBackground::loadStyle(KoOdfLoadingContext & context, const QSizeF &shapeSize)
{
    Q_D(KoGradientBackground);
    KoStyleStack &styleStack = context.styleStack();
    if (! styleStack.hasProperty(KoXmlNS::draw, "fill"))
        return false;

    QString fillStyle = styleStack.property(KoXmlNS::draw, "fill");
    if (fillStyle == "gradient") {
        QBrush brush = KoOdfGraphicStyles::loadOdfGradientStyle(styleStack, context.stylesReader(), shapeSize);
        const QGradient * gradient = brush.gradient();
        if (gradient) {
            d->gradient = KoFlake::cloneGradient(gradient);
            d->matrix = brush.matrix();
            return true;
        }
    }
    return false;
}
