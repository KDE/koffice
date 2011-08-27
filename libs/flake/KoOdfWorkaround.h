/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KODFWORKAROUND_H
#define KODFWORKAROUND_H

#include "flake_export.h"

class KXmlElement;
class KShape;
class KShapeLoadingContext;
class QPen;
class QColor;
class QString;
class KColorBackground;

/**
 * This class should contain all workarounds to correct problems with different ODF
 * implementations. If you need to access application specific things please create a
 * new namespace in the application you need it in
 * All calls to methods of this class should be wrapped into ifndefs like e.g.
 *
 * @code
 * #ifndef NWORKAROUND_ODF_BUGS
 *     KoOdfWorkaround::fixPenWidth(pen, context);
 * #endif
 * @endcode
 */
namespace KoOdfWorkaround
{
    /**
     * OpenOffice handles a line with the width of 0 as a cosmetic line but in svg it makes the line invisible.
     * To show it in koffice use a very small line width. However this is not a cosmetic line.
     */
    FLAKE_EXPORT void fixPenWidth(QPen &pen, KShapeLoadingContext &context);

    /**
     * OpenOffice < 3.0 does not store the draw:enhanced-path for draw:type="ellipse"
     * Add the path needed for the ellipse
     */
    FLAKE_EXPORT void fixEnhancedPath(QString &path, const KXmlElement &element, KShapeLoadingContext &context);

    /**
     * OpenOffice interchanges the position coordinates for polar handles.
     * According to the specification the first coordinate is the angle, the
     * second coordinates is the radius. OpenOffice does it the other way around.
     */
    FLAKE_EXPORT void fixEnhancedPathPolarHandlePosition(QString &position, const KXmlElement &element, KShapeLoadingContext &context);

    FLAKE_EXPORT bool   fixMissingStroke(QPen &pen, const KXmlElement &element, KShapeLoadingContext &context, const KShape *shape = 0);
    FLAKE_EXPORT QColor fixMissingFillColor(const KXmlElement &element, KShapeLoadingContext &context);
    FLAKE_EXPORT bool   fixMissingStyle_DisplayLabel(const KXmlElement &element, KShapeLoadingContext &context);

    FLAKE_EXPORT KColorBackground *fixBackgroundColor(const KShape *shape, KShapeLoadingContext &context);

    /**
     * Old versions of ooimpress does not set the placeholder for shapes that should have it set
     * See open office issue http://www.openoffice.org/issues/show_bug.cgi?id=96406
     * And kde bug https://bugs.kde.org/show_bug.cgi?id=185354
     */
    FLAKE_EXPORT void setFixPresentationPlaceholder(bool fix, KShapeLoadingContext &context);
    FLAKE_EXPORT bool fixPresentationPlaceholder();
    FLAKE_EXPORT void fixPresentationPlaceholder(KShape *shape);
}

#endif /* KODFWORKAROUND_H */
