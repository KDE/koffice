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

#ifndef KOSHAPEBACKGROUND_H
#define KOSHAPEBACKGROUND_H

#include "flake_export.h"

class QSizeF;
class QPainter;
class QPainterPath;
class KOdfGenericStyle;
class KShapeSavingContext;
class KOdfLoadingContext;
class KShapeBackgroundPrivate;

/**
 * This is the base class for shape backgrounds.
 * Derived classes are used to paint the background of
 * a shape within a given painter path.
 */
class FLAKE_EXPORT KShapeBackground
{
public:
    KShapeBackground();
    virtual ~KShapeBackground();

    /// Paints the background using the given fill path
    virtual void paint(QPainter &painter, const QPainterPath &fillPath) const = 0;

    /// Returns if the background has some transparency.
    virtual bool hasTransparency() const;

    /**
     * Fills the style object
     * @param style object
     * @param context used for saving
     */
    virtual void fillStyle(KOdfGenericStyle &style, KShapeSavingContext &context) = 0;

    /// load background from odf styles
    virtual bool loadStyle(KOdfLoadingContext &context, const QSizeF &shapeSize) = 0;

    /**
     * Increments the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool ref();
    /**
     * Decrements the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool deref();
    /// Return the usage count
    int useCount() const;

protected:
    KShapeBackground(KShapeBackgroundPrivate &);
    KShapeBackgroundPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KShapeBackground)
};

#endif // KOSHAPEBACKGROUND_H
