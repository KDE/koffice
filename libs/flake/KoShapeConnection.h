/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOSHAPECONNECTION_H
#define KOSHAPECONNECTION_H

#include "flake_export.h"

#include <QPointF>
#include <QRectF>

class KoShape;
class QPainter;
class KoViewConverter;

class FLAKE_EXPORT KoShapeConnection {
public:

    enum ConnectionType {
        TautConnection,
        CurvedConnection
        // etc?
    };

    explicit KoShapeConnection(KoShape *from, int gluePointIndex1, KoShape *to = 0,  int gluePointIndex2 = 0);
    ~KoShapeConnection();

    void paint(QPainter &painter, const KoViewConverter &converter);

    KoShape *shape1() const;
    KoShape *shape2() const;

    int zIndex() const;
    void setZIndex(int index);

    int gluePointIndex1() const;
    int gluePointIndex2() const;

    QPointF startPoint() const;
    QPointF endPoint() const;

    /**
     * This is a method used to sort a list using the STL sorting methods.
     * @param c1 the first connection
     * @param c2 the second connection
     */
    static bool compareConnectionZIndex(KoShapeConnection*c1, KoShapeConnection *c2);

    QRectF boundingRect() const;

private:
    class Private;
    Private * const d;
};

/*
     TODO
    Add a strategy for InteractionTool that when it selects a connection it can be used to change
        the properties or delete it.
*/

#endif
