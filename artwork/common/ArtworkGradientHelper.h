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

#ifndef ARTWORKGRADIENTHELPER_H
#define ARTWORKGRADIENTHELPER_H

#include <artworkcommon_export.h>
#include <QtGui/QBrush>

class KShape;
class QGradient;

namespace ArtworkGradientHelper
{
/// applies given gradient stops to given shape returning the new gradient wrapped in a brush
ARTWORKCOMMON_EXPORT QBrush applyGradientStops(KShape *shape, const QGradientStops &stops, bool fillGradient);

/// creates default gradient
ARTWORKCOMMON_EXPORT QGradient *defaultGradient(QGradient::Type type, QGradient::Spread spread, const QGradientStops &stops);

/// Converts gradient type, preserving as much data as possible
ARTWORKCOMMON_EXPORT QGradient *convertGradient(const QGradient *gradient, QGradient::Type newType);

/// Calculates color at given position from given gradient stops
ARTWORKCOMMON_EXPORT QColor colorAt(qreal position, const QGradientStops &stops);
};

#endif // ARTWORKGRADIENTHELPER_H
