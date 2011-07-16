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

#ifndef KLINEBORDER_H
#define KLINEBORDER_H

#include "KShapeBorderBase.h"

#include "flake_export.h"

class KShape;
class QPainter;
class QColor;
class KViewConverter;

/**
 * A border for shapes that draws a single line around the object.
 */
class FLAKE_EXPORT KLineBorder : public KShapeBorderBase
{
public:
    /// Constructor for a thin line in black
    KLineBorder();

    /**
     * Constructor for a lineBorder
     * @param lineWidth the width, in pt
     * @param color the color we draw the outline in.
     */
    explicit KLineBorder(qreal lineWidth, const QColor &color = Qt::black);
    virtual ~KLineBorder();

    /// Assignment operator
    KLineBorder &operator=(const KLineBorder &other);

    virtual void saveOdf(KOdfGenericStyle &style, KShapeSavingContext &context) const;
    virtual void paint(KShape *shape, QPainter &painter, const KViewConverter &converter);
};

#endif
