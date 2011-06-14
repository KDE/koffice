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
#ifndef KOSHAPECONNECTION_P_H
#define KOSHAPECONNECTION_P_H
#include "KShapeConnection.h"

#include <QStringList>
#include <QPainterPath>

class ConnectStrategy
{
public:
    ConnectStrategy(KShapeConnectionPrivate *qq, KShapeConnection::ConnectionType type)
        : q(qq),
        m_dirty(true),
        m_type(type)
    {
    }
    virtual ~ConnectStrategy() { }
    KShapeConnection::ConnectionType type() const { return m_type; }

    virtual void paint(QPainter &painter, const KoViewConverter &converter) = 0;
    virtual void setSkew(const QStringList &values) {
        Q_UNUSED(values);
    }
    virtual void saveOdf(KShapeSavingContext &context) const = 0;

    virtual QRectF boundingRect() const = 0;

    void foul() { m_dirty = true; }
    void wipe() { m_dirty = false; }

protected:
    KShapeConnectionPrivate *q;
    mutable bool m_dirty;

private:
    const KShapeConnection::ConnectionType m_type;
};

class KShapeConnectionPrivate
{
public:
    KShapeConnectionPrivate(KShapeConnection *qq, KShape *from, int gp1, KShape *to, int gp2);
    KShapeConnectionPrivate(KShapeConnection *qq, KShape *from, int gp1, const QPointF &ep);

    /// return the start point or the point from the shape connector if that exists
    QPointF resolveStartPoint() const;
    /// return the end point or the point from the shape connector if that exists
    QPointF resolveEndPoint() const;

    inline void foul() {
        if (connectionStrategy)
            connectionStrategy->foul();
    }

    QPainterPath createConnectionPath(const QPointF &from, const QPointF &to) const;

    QLineF calculateShapeFalloutPrivate(const QPointF &begin, bool start) const;

    KShapeConnection *q;
    KShape *shape1;
    KShape *shape2;
    int gluePointIndex1;
    int gluePointIndex2;
    QPointF startPoint; // used if there is no shape1
    QPointF endPoint; // used if there is no shape2
    int zIndex;
    /**
     * if true then the user has not set any shapes to connect to.
     * In flake a connection will only show up when its a shape and as such the
     * shapeRegistry will add a dummyShape for 'shape1' if this is the case.
     */
    bool hasDummyShape;

    ConnectStrategy *connectionStrategy;
};

#endif
