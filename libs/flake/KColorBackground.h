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

#ifndef KCOLORBACKGROUND_H
#define KCOLORBACKGROUND_H

#include "KShapeBackgroundBase.h"
#include "flake_export.h"
#include <Qt>

class QColor;

/// A simple solid color shape background
class FLAKE_EXPORT KColorBackground : public KShapeBackgroundBase
{
public:
    KColorBackground();

    /// Creates background from given color and style
    explicit KColorBackground(const QColor &color, Qt::BrushStyle style = Qt::SolidPattern);

    virtual ~KColorBackground();

    /// Returns the background color
    QColor color() const;

    /// Sets the background color
    void setColor(const QColor &color);

    /// Returns the background style
    Qt::BrushStyle style() const;

    // reimplemented from KShapeBackgroundBase
    virtual void paint(QPainter &painter, const QPainterPath &fillPath) const;
    // reimplemented from KShapeBackgroundBase
    virtual void fillStyle(KOdfGenericStyle &style, KShapeSavingContext &context);
    // reimplemented from KShapeBackgroundBase
    virtual bool loadStyle(KOdfLoadingContext & context, const QSizeF &shapeSize);

private:
    class Private;
    Private * const d;
};

#endif // KOCOLORBACKGROUND_H
