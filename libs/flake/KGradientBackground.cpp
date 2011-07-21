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

#include "KGradientBackground.h"
#include "KShapeBackgroundBase_p.h"
#include "KoFlake.h"
#include <KOdfStyleStack.h>
#include <KOdfXmlNS.h>
#include <KOdfLoadingContext.h>
#include <KOdf.h>
#include <KShapeSavingContext.h>

#include <KDebug>

#include <QtGui/QBrush>
#include <QtGui/QPainter>

class KGradientBackgroundPrivate : public KShapeBackgroundBasePrivate
{
public:
    KGradientBackgroundPrivate() : gradient(0) {};
    ~KGradientBackgroundPrivate() {
        delete gradient;
    }

    QGradient * gradient;
    QTransform matrix;
};

KGradientBackground::KGradientBackground(QGradient *gradient, const QTransform &matrix)
    : KShapeBackgroundBase(*(new KGradientBackgroundPrivate()))
{
    Q_D(KGradientBackground);
    d->gradient = gradient;
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
    Q_ASSERT(d->gradient->coordinateMode() == QGradient::ObjectBoundingMode);
}

KGradientBackground::KGradientBackground(const QGradient &gradient, const QTransform &matrix)
    : KShapeBackgroundBase(*(new KGradientBackgroundPrivate()))
{
    Q_D(KGradientBackground);
    d->gradient = KoFlake::cloneGradient(&gradient);
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
    Q_ASSERT(d->gradient->coordinateMode() == QGradient::ObjectBoundingMode);
}

KGradientBackground::~KGradientBackground()
{
}

void KGradientBackground::setTransform(const QTransform &matrix)
{
    Q_D(KGradientBackground);
    d->matrix = matrix;
}

QTransform KGradientBackground::transform() const
{
    Q_D(const KGradientBackground);
    return d->matrix;
}

void KGradientBackground::setGradient(QGradient *gradient)
{
    Q_D(KGradientBackground);
    Q_ASSERT(gradient);
    if (gradient == d->gradient)
        return;
    delete d->gradient;

    d->gradient = gradient;
    Q_ASSERT(d->gradient->coordinateMode() == QGradient::ObjectBoundingMode);
}

void KGradientBackground::setGradient(const QGradient &gradient)
{
    Q_D(KGradientBackground);
    delete d->gradient;

    d->gradient = KoFlake::cloneGradient(&gradient);
    Q_ASSERT(d->gradient);
    Q_ASSERT(d->gradient->coordinateMode() == QGradient::ObjectBoundingMode);
}

const QGradient *KGradientBackground::gradient() const
{
    Q_D(const KGradientBackground);
    return d->gradient;
}

KGradientBackground &KGradientBackground::operator = (const KGradientBackground &rhs)
{
    Q_D(KGradientBackground);
    if (this == &rhs)
        return *this;

    KGradientBackgroundPrivate *other = static_cast<KGradientBackgroundPrivate*>(rhs.d_ptr);

    d->matrix = other->matrix;
    delete d->gradient;
    d->gradient = KoFlake::cloneGradient(other->gradient);
    Q_ASSERT(d->gradient);
    Q_ASSERT(d->gradient->coordinateMode() == QGradient::ObjectBoundingMode);

    return *this;
}

void KGradientBackground::paint(QPainter &painter, const QPainterPath &fillPath) const
{
    Q_D(const KGradientBackground);
    QBrush brush(*d->gradient);
    brush.setTransform(d->matrix);

    painter.setBrush(brush);
    painter.drawPath(fillPath);
}

void KGradientBackground::fillStyle(KOdfGenericStyle &style, KShapeSavingContext &context)
{
    Q_D(KGradientBackground);
    QBrush brush(*d->gradient);
    brush.setTransform(d->matrix);
    KOdf::saveOdfFillStyle(style, context.mainStyles(), brush);
}

bool KGradientBackground::loadStyle(KOdfLoadingContext &context, const QSizeF &shapeSize)
{
    Q_D(KGradientBackground);
    KOdfStyleStack &styleStack = context.styleStack();
    if (! styleStack.hasProperty(KOdfXmlNS::draw, "fill"))
        return false;

    QString fillStyle = styleStack.property(KOdfXmlNS::draw, "fill");
    if (fillStyle == "gradient") {
        QBrush brush = KOdf::loadOdfGradientStyle(styleStack, context.stylesReader(), shapeSize);
        const QGradient * gradient = brush.gradient();
        if (gradient) {
            d->gradient = KoFlake::cloneGradient(gradient);
            d->matrix = brush.transform();
            return true;
        }
    }
    return false;
}
