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
#include "KShape.h"
#include "KShapeSavingContext.h"

#include <KOdf.h>
#include <QPainter>

KLineBorder::KLineBorder()
{
    setBorderId("LineBorder");
}

KLineBorder::KLineBorder(qreal lineWidth, const QColor &color)
{
    QPen pen(color, qMax(qreal(0.0), lineWidth));
    pen.setJoinStyle(Qt::MiterJoin);
    setPen(pen);
}

KLineBorder::~KLineBorder()
{
}

KLineBorder &KLineBorder::operator=(const KLineBorder &other)
{
    if (this == &other)
        return *this;

    setPen(other.pen());

    return *this;
}

void KLineBorder::saveOdf(KOdfGenericStyle &style, KShapeSavingContext &context) const
{
    KOdf::saveOdfStrokeStyle(style, context.mainStyles(), pen());
}

void KLineBorder::paint(KShape *shape, QPainter &painter, const KViewConverter &converter)
{
    KShape::applyConversion(painter, converter);

    if (!pen().isCosmetic())
        painter.strokePath(shape->outline(), pen());
}
