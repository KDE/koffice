/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOODFGRAPHICSTYLES_H
#define KOODFGRAPHICSTYLES_H

#include "kodf_export.h"

#include <QtGui/QTransform>

class QBrush;
class QPen;
class QString;
class QSizeF;

class KOdfGenericStyle;
class KOdfGenericStyles;
class KOdfStyleStack;

class KOdfStylesReader;
class KOdfLoadingContext;

namespace KoOdfGraphicStyles
{
    KODF_EXPORT void saveOdfFillStyle(KOdfGenericStyle &styleFill, KOdfGenericStyles& mainStyles, const QBrush &brush);

    KODF_EXPORT void saveOdfStrokeStyle(KOdfGenericStyle &styleStroke, KOdfGenericStyles &mainStyles, const QPen &pen);

    KODF_EXPORT QString saveOdfHatchStyle(KOdfGenericStyles &mainStyles, const QBrush &brush);

    /// Saves gradient style of brush into mainStyles and returns the styles name
    KODF_EXPORT QString saveOdfGradientStyle(KOdfGenericStyles &mainStyles, const QBrush &brush);

    /// Loads gradient style from style stack and stylesReader adapted to the given size and returns a brush
    KODF_EXPORT QBrush loadOdfGradientStyle(const KOdfStyleStack &styleStack, const KOdfStylesReader &stylesReader, const QSizeF &size);

    /// Loads gradient style with the given name from style stack and stylesReader adapted to the given size and returns a brush
    KODF_EXPORT QBrush loadOdfGradientStyleByName(const KOdfStylesReader &stylesReader, const QString &styleName, const QSizeF &size);

    KODF_EXPORT QBrush loadOdfFillStyle(const KOdfStyleStack &styleStack, const QString &fill,  const KOdfStylesReader &stylesReader);

    KODF_EXPORT QPen loadOdfStrokeStyle(const KOdfStyleStack &styleStack, const QString &stroke, const KOdfStylesReader &stylesReader);

    /// Helper function to parse a transformation attribute
    KODF_EXPORT QTransform loadTransformation(const QString &transformation);

    /// Helper function to create a transformation attribute
    KODF_EXPORT QString saveTransformation(const QTransform &transformation, bool appendTranslateUnit = true);
};

#endif /* KOODFGRAPHICSTYLES_H */
