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

class KShape;
class QPainter;
class KoViewConverter;
class KXmlElement;
class KShapeLoadingContext;
class KShapeSavingContext;
class KShapeConnectionPrivate;

/**
 * The shapeConnection class represents a connection between two shapes.
 * In order to create a visible connection between any two shapes of any kind you can
 * create a new KShapeConnection passing the two shapes that it connects.
 * Each KShape instance can have a number of connection points, also called glue points, each
 * of which can be used to start or end a connection.  Consider an shape in the form of a man.
 * You would call KShape::addConnectionPoint() with a point where his hand is.  If you have a
 * pet with a similarly added connection point adding a connection is a simple case of
 * @code
   new KShapeConnection(man, 0, dog, 0);
   @endcode
 */
class FLAKE_EXPORT KShapeConnection
{
public:
    /// the visual type of connection
    enum ConnectionType {
        /** a standard connector escapes two connecting objects with straight lines and
          * connects them with one or more straight perpendicular lines that do not intersect
          * the connected shapes' bounding boxes */
        EdgedLinesOutside,
        /** a lines connector leaves two connecting objects with straight lines and connects
          * them with a straight (not necessarily perpendicular) line */
        EdgedLines,
        /** a line connector draws one straight line between the two glue points of
          * connected objects */
        Straight,
        /** a curve connector draws a single curved line between the two glue points
          * of connected objects */
        Curve
    };

    /// default constructor for a connector between two points only
    KShapeConnection();

    /**
     * Constructor for the connection between two shapes.
     * The connection will be added to each of the shapes.  Note that we refer to the gluePoints by index
     * instead of directly accessing the point.  This is done because resizing the shape may alter the actual
     * point, but not the index.
     * @param from is the originating shape
     * @param gluePointIndex1 The point to connect to is found via the index in the list of connectors on the originating shape.
     * @param to is the shape for the endpoint.
     * @param gluePointIndex2 The point to connect to is found via the index in the list of connectors on the end shape.
     */
    explicit KShapeConnection(KShape *from, int gluePointIndex1, KShape *to, int gluePointIndex2);
    /**
     * Constructor for connection between a shape and a fixed point.
     * The connection will be added to the shape. Note that we refer to the gluePoints by index
     * instead of directly accessing the point. This is done because resizing the shape may alter
     * the actual point but not the index.
     *
     * @param from is the originating shape.
     * @param gluePointIndex The point to connect to is found via the index in the list of of connectors on the originating shape.
     * @param endPoint Fixed point on the canvas where the connection is anchored.
     */
    explicit KShapeConnection(KShape* from, int gluePointIndex, const QPointF &endPoint);

    explicit KShapeConnection(KShape *from, KShape *to, int gluePointIndex2 = 0);
    ~KShapeConnection();

    /**
     * @brief Paint the connection.
     * The connection is painted from start to finish in absolute coordinates. Meaning that the top left
     * of the document is coordinate 0,0.  This in contradiction to shapes.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     */
    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * Return the first shape.
     */
    KShape *shape1() const;
    /**
     * Return the second shape.
     * Note that this can be 0.
     */
    KShape *shape2() const;

    /**
     * The z index in which the connection will be drawn.  If the index is higher it will be drawn on top.
     */
    int zIndex() const;
    /**
     * Set the z index in which the connection will be drawn.  If the index is higher it will be drawn on top.
     * @param index the z-index.
     */
    void setZIndex(int index);

    /**
     * Return the gluePointIndex for the originating shape.
     * Note that we refer to the gluePoints by index instead of directly accessing the point.
     * This is done because resizing the shape may alter the actual point, but not the index.
     */
    int gluePointIndex1() const;

    /**
     * Return the gluePointIndex for the end shape.
     * Note that we refer to the gluePoints by index instead of directly accessing the point.
     * This is done because resizing the shape may alter the actual point, but not the index.
     */
    int gluePointIndex2() const;

    /**
     * Calculate and return the absolute point where this connection starts.
     */
    QPointF startPoint() const;

    /**
     * Calculate and return the absolute point where this connection ends.
     */
    QPointF endPoint() const;

    /**
     * This is a method used to sort a list using the STL sorting methods.
     * @param c1 the first connection
     * @param c2 the second connection
     */
    static bool compareConnectionZIndex(KShapeConnection*c1, KShapeConnection *c2);

    /**
     * Return a bounding rectangle in which this connection is completely present.
     */
    QRectF boundingRect() const;

    /// Set startPoint to @p point
    void setStartPoint(const QPointF &point);
    /// Set startPoint to a point
    void setStartPoint(qreal x, qreal y) {
        setStartPoint(QPointF(x, y));
    }
    /// Set endPoint to @p point
    void setEndPoint(const QPointF &point);
    /// Set endPoint to a point
    void setEndPoint(qreal x, qreal y) {
        setEndPoint(QPointF(x, y));
    }
    /// Sets shape1 to @p shape and gluePointIndex1 to @p gluePointIndex
    void setStartPoint(KShape *shape, int gluePointIndex);
    /// Sets shape2 to @p shape and gluePointIndex2 to @p gluePointIndex
    void setEndPoint(KShape *shape, int gluePointIndex);

    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

    /**
     * @brief store the shape data as ODF XML.
     * This is the method that will be called when saving a shape as a described in
     * OpenDocument 9.2 Drawing Shapes.
     * @see saveOdfAttributes
     */
    void saveOdf(KShapeSavingContext &context) const;

    ConnectionType type() const;
    void setType(ConnectionType type);

    /**
     * Request a repaint to be queued.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     */
    void update() const;

    /// \internal
    KShapeConnectionPrivate * priv();

private:
    KShapeConnectionPrivate * const d;
};

/*
     TODO
    Add a strategy for InteractionTool that when it selects a connection it can be used to change
        the properties or delete it.

    Should we have a way to segment the repaint-rects of the connection?  Now if we have a long connection
      the repaint rect will be huge due to us using the plain bounding rect.  It might be useful to return
      a list of QRectFs which each will be redrawn. Allowing for a substantially smaller repaint area.

    Should I remove a shapeConnection from the ShapeManager on destruction?
*/

#endif
