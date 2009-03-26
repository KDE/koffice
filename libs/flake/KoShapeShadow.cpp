/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeShadow.h"
#include "KoShapeSavingContext.h"
#include "KoShapeBorderModel.h"
#include "KoShape.h"
#include "KoInsets.h"
#include "KoPathShape.h"
#include <KoGenStyle.h>
#include <KoViewConverter.h>
#include <QtGui/QPainter>
#include <QtCore/QAtomicInt>

class KoShapeShadow::Private
{
public:
    Private()
            : offset(10, 10), color(Qt::black), visible(true), refCount(0) {
    }
    QPointF offset;
    QColor color;
    bool visible;
    QAtomicInt refCount;
};

KoShapeShadow::KoShapeShadow()
        : d(new Private())
{
}

KoShapeShadow::~KoShapeShadow()
{
    delete d;
}

void KoShapeShadow::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    Q_UNUSED(context);

    style.addProperty("draw:shadow", d->visible ? "visible" : "hidden");
    style.addProperty("draw:shadow-color", d->color.name());
    if (d->color.alphaF() != 1.0)
        style.addProperty("draw:shadow-opacity", QString("%1%").arg(d->color.alphaF() * 100.0));
    style.addProperty("draw:shadow-offset-x", QString("%1pt").arg(d->offset.x()));
    style.addProperty("draw:shadow-offset-y", QString("%1pt").arg(d->offset.y()));
}

void KoShapeShadow::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    if (! d->visible)
        return;

    // calculate the shadow offset independent of shape transformation
    QMatrix tm;
    tm.translate(d->offset.x(), d->offset.y());   
    QMatrix tr = shape->absoluteTransformation(&converter);   
    QMatrix offsetMatrix = tr * tm * tr.inverted();
    
    if (shape->background()) {
        painter.save();
        
        KoShape::applyConversion(painter, converter);
        
        // the shadow direction is independent of the shapes transformation
        // please only change if you know what you are doing
        painter.setMatrix(offsetMatrix * painter.matrix());
        painter.setBrush(QBrush(d->color));

        QPainterPath path(shape->outline());
        KoPathShape * pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape)
            path.setFillRule(pathShape->fillRule());
        painter.drawPath(path);
        painter.restore();
    }

    if (shape->border()) {
        QMatrix oldPainterMatrix = painter.matrix();
        KoShape::applyConversion(painter, converter);
        QMatrix newPainterMatrix = painter.matrix();
        
        // the shadow direction is independent of the shapes transformation
        // please only change if you know what you are doing
        painter.setMatrix(offsetMatrix * painter.matrix());
        
        // compensate applyConversion call in paintBorder
        QMatrix scaleMatrix = newPainterMatrix * oldPainterMatrix.inverted();
        painter.setMatrix(scaleMatrix.inverted() * painter.matrix());
        
        shape->border()->paintBorder(shape, painter, converter, d->color);
    }
}

void KoShapeShadow::setOffset(const QPointF & offset)
{
    d->offset = offset;
}

QPointF KoShapeShadow::offset() const
{
    return d->offset;
}

void KoShapeShadow::setColor(const QColor &color)
{
    d->color = color;
}

QColor KoShapeShadow::color() const
{
    return d->color;
}

void KoShapeShadow::setVisibility(bool visible)
{
    d->visible = visible;
}

bool KoShapeShadow::isVisible() const
{
    return d->visible;
}

void KoShapeShadow::insets(const KoShape *shape, KoInsets &insets)
{
    if (! d->visible)
        return;

    Q_UNUSED(shape);

    insets.left = (d->offset.x() < 0.0) ? qAbs(d->offset.x()) : 0.0;
    insets.top = (d->offset.y() < 0.0) ? qAbs(d->offset.y()) : 0.0;
    insets.right = (d->offset.x() > 0.0) ? d->offset.x() : 0.0;
    insets.bottom = (d->offset.y() > 0.0) ? d->offset.y() : 0.0;
}

void KoShapeShadow::addUser()
{
    d->refCount.ref();
}

bool KoShapeShadow::removeUser()
{
    return d->refCount.deref();
}

int KoShapeShadow::useCount() const
{
    return d->refCount;
}
