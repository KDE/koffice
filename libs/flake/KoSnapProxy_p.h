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
#ifndef KOSNAPPROXY_H
#define KOSNAPPROXY_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


class KSnapGuide;
class KShape;
class KPathSegment;
class KCanvasBase;

#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtCore/QRectF>

/**
 * This class provides access to different shape related snap targets to snap strategies.
 */
class KoSnapProxy
{
public:
    KoSnapProxy(KSnapGuide *snapGuide);

    /// returns list of points in given rectangle in document coordinates
    QList<QPointF> pointsInRect(const QRectF &rect);

    /// returns list of shape in given rectangle in document coordinates
    QList<KShape*> shapesInRect(const QRectF &rect, bool omitEditedShape = false);

    /// returns list of points from given shape
    QList<QPointF> pointsFromShape(KShape *shape);

    /// returns list of points in given rectangle in document coordinates
    QList<KPathSegment> segmentsInRect(const QRectF &rect);

    /// returns list of all shapes
    QList<KShape*> shapes(bool omitEditedShape = false);

    /// returns canvas we are working on
    KCanvasBase *canvas();

private:
    KSnapGuide *m_snapGuide;
};

#endif
