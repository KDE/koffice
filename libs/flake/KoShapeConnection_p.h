/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#include "KoShapeConnection.h"

class ConnectStrategy;

class KoShapeConnectionPrivate
{
public:
    KoShapeConnectionPrivate(KoShape *from, int gp1, KoShape *to, int gp2);
    KoShapeConnectionPrivate(KoShape *from, int gp1, const QPointF& ep);

    KoShape *shape1;
    KoShape *shape2;
    int gluePointIndex1;
    int gluePointIndex2;
    QPointF startPoint; // used if there is no shape1
    QPointF endPoint; // used if there is no shape2
    int zIndex;

    ConnectStrategy *connectionStrategy;
};
