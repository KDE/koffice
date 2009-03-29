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

#ifndef KOODFWORKAROUND_H
#define KOODFWORKAROUND_H

#include "flake_export.h"

class KoXmlElement;
class KoShapeLoadingContext;
class QPen;
class QString;

/**
 * This class should contain all workarounds to correct problems with differenet ODF 
 * implementations. If you need to access application specifc things please create a
 * new namespace in the app you need it
 * All calls to methods of this class should be wrapped into ifndefs like e.g.
 *
 * #ifndef NWORKAROUND_ODF_BUGS
 *     KoOdfWorkaround::fixPenWidth(pen, context);
 * #endif
 */
namespace KoOdfWorkaround
{
    /**
     * OpenOffice handels a line with the width of 0 as a cosmetic line but in svg it makes the line invisible. 
     * To show it in koffice use a very small line width. However this is not a cosmetic line.
     */
    FLAKE_EXPORT void fixPenWidth(QPen & pen, KoShapeLoadingContext &context);

    /**
     * OpenOffice < 3.0 does not store the draw:enhanced-path for draw:type="ellipse"
     * Add the path needed for the ellipse
     */
    FLAKE_EXPORT void fixEnhancedPath(QString & path, const KoXmlElement &element, KoShapeLoadingContext &context);
}

#endif /* KOODFWORKAROUND_H */
