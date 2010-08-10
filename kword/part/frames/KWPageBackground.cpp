/* This file is part of the KDE project
   Copyright (C) 2010 by Nokia

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KWPageBackground.h"
#include <KoShapeBackground.h>

KWPageBackground::KWPageBackground()
{
    setSelectable(false);
}

KWPageBackground::~KWPageBackground()
{

}

void KWPageBackground::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (background()) {
        applyConversion(painter, converter);
        QPainterPath p;
        p.addRect(QRectF(QPointF(), size()));
        background()->paint(painter, p);
    }
}

bool KWPageBackground::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    return true;
}

void KWPageBackground::saveOdf(KoShapeSavingContext &) const
{

}
