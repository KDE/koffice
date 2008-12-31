/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOPATHSHAPE_H
#define KOPATHSHAPE_H

#include "flake_export.h"

#include <QMap>

#include "KoShape.h"
#include "KoPathSegment.h"

#define KoPathShapeId "KoPathShape"

class KoPathShape;
class KoPointGroup;
class KoPathPoint;

typedef QPair<int, int> KoPathPointIndex;
typedef QMap<KoPathShape *, QSet<KoPathPointIndex> > KoPathShapePointIndexMap;

/// a KoSubpath contains a path from a moveTo until a close or a new moveTo
typedef QList<KoPathPoint *> KoSubpath;
typedef QList<KoSubpath *> KoSubpathList;
/// The position of a path point within a path shape
typedef QPair<KoSubpath*, int> KoPointPosition;
/**
 * @brief This is the base for all graphical objects.
 *
 * All graphical objects are based on this object e.g. lines, rectangulars, pies
 * and so on.
 *
 * The KoPathShape uses KoPathPoint's to describe the path of the shape.
 *
 * Here a short example:
 * 3 points connected by a curveTo's described by the following svg:
 * M 100,200 C 100,100 250,100 250,200 C 250,200 400,300 400,200.
 *
 * This will be stored in 3 KoPathPoint's as
 * The first point contains in
 *       point 100,200
 *       controlPoint2 100,100
 * The second point contains in
 *       point 250,200
 *       controlPoint1 250,100
 *       controlPoint2 250,300
 * The third point contains in
 *       point 400,300
 *       controlPoint1 400,200
 *
 * Not the segments are stored but the points. Out of the points the segments are
 * generated. See the outline method. The reason for storing it like that is that
 * it is the points that are modified by the user and not the segments.
 */
class FLAKE_EXPORT KoPathShape : public KoShape
{
public:
    /**
     * @brief constructor
     */
    KoPathShape();

    /**
     * @brief
     */
    virtual ~KoPathShape();

    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void paintPoints(QPainter &painter, const KoViewConverter &converter, int handleRadius);
    /// reimplemented
    virtual const QPainterPath outline() const;
    /// reimplemented
    virtual QRectF boundingRect() const;
    /// reimplemented
    virtual QSizeF size() const;
    /**
     * Resize the shape
     *
     * This makes sure that the pathshape will not be resized to 0 if the new size
     * is null as that makes it impossible to undo the change.
     *
     * All functions that overwrite this function should also use the resizeMatrix
     * function to get and use the same data in resizing.
     *
     * @see resizeMatrix()
     */
    virtual void setSize(const QSizeF &size);
    /// reimplemented
    virtual bool hitTest(const QPointF &position) const;
    
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext & context) const;
    // reimplemented
    virtual bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context);

    /// Removes all subpaths and their points from the path
    void clear();
    /**
     * @brief Start a new Subpath
     *
     * Moves the pen to p and starts a new subpath.
     *
     * @return The newly created point
     */
    KoPathPoint * moveTo(const QPointF &p);

    /**
     * @brief add a line
     *
     * Adds a straight line between the last point and the given p.
     *
     * @return The newly created point
     */
    KoPathPoint * lineTo(const QPointF &p);

    /**
     * @brief add a cubic Bezier curve.
     *
     * Adds a cubic Bezier curve between the last point and the given p,
     * using the control points specified by c1, and c2.
     * @param c1 control point1
     * @param c2 control point2
     * @param p The endpoint of this curve-part
     *
     * @return The newly created point
     */
    KoPathPoint * curveTo(const QPointF &c1, const QPointF &c2, const QPointF &p);

    /**
     * @brief add a quadratic Bezier curve.
     *
     * Adds a quadratic Bezier curve between the last point and the given p,
     * using the control point specified by c.
     * @param c control point
     * @param p The endpoint of this curve-part
     *
     * @return The newly created point
     */
    KoPathPoint * curveTo(const QPointF &c, const QPointF &p);

    /**
     * @brief add a arc.
     *
     * Adds an arc starting at the current point. The arc will be converted to bezier curves.
     * @param rx x radius of the ellipse
     * @param ry y radius of the ellipse
     * @param startAngle the angle where the arc will be started
     * @param sweepAngle the length of the angle
     * TODO add param to have angle of the ellipse
     *
     * @return The newly created point
     */
    KoPathPoint * arcTo(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle);


    /**
     * @brief close the current subpath
     */
    void close();

    /**
     * @brief close the current subpath
     *
     * It tries to merge the last and first point of the subpath
     * to one point and then closes the subpath. If merging is not
     * possible as the two point are to far from each other a close
     * will be done.
     * TODO define a maximum distance between  the two points until this is working
     */
    void closeMerge();

    /**
     * @brief Normalizes the path data.
     *
     * The path points are transformed so that the top-left corner
     * of the bounding rect is (0,0).
     * This should be called after adding points to the path.
     * @return the offset by which the points are moved in shape coordinates.
     */
    virtual QPointF normalize();

    /**
     * @brief Returns the path points within the given rectangle.
     * @param r the rectangle the requested points are in
     * @return list of points within the rectangle
     */
    QList<KoPathPoint*> pointsAt(const QRectF &r);

    /**
     * @brief Returns the list of path segments within the given rectangle.
     * @param r the rectangle the requested segments are in
     * @return list of segments within the rectangle
     */
    QList<KoPathSegment> segmentsAt(const QRectF &r);

    /**
     * @brief Get the path point index of a point
     *
     * @param point for which you want to get the info
     * @return path point index of the point if it exists
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex pathPointIndex(const KoPathPoint *point) const;

    /**
     * @brief Get the point defined by a path point index
     *
     * @param pointIndex of the point to get
     *
     * @return KoPathPoint on success, 0 otherwise e.g. out of bounds
     */
    KoPathPoint * pointByIndex(const KoPathPointIndex &pointIndex) const;

    /**
     * @brief Get the segment defined by a path point index
     *
     * A semgent is defined by the point index of the first point in the segment.
     * A segment contains the defined point and its following point. If the subpath is
     * closed and the and the pointIndex point to the last point in the subpath, the
     * following point is the first point in the subpath.
     *
     * @param pointIndex of the first point of the segment
     *
     * @return Segment containing both points of the segment or KoPathSegment( 0, 0 ) on error e.g. out of bounds
     */
    KoPathSegment segmentByIndex(const KoPathPointIndex &pointIndex) const;

    /**
     * @brief Get the number of points in the path
     *
     * @return The number of points in the path
     */
    int pointCount() const;

    /**
     * @brief Get the number of subpaths in the path
     *
     * @return The number of subpaths in the path
     */
    int subpathCount() const;

    /**
     * @brief Get the number of points in a subpath
     *
     * @return The number of points in the subpath or -1 if subpath out of bounds
     */
    int pointCountSubpath(int subpathIndex) const;

    /**
     * @brief Check if subpath is closed
     *
     * @param subpathIndex of the subpath to check
     *
     * @return true when the subpath is closed, false otherwise
     */
    bool isClosedSubpath(int subpathIndex);

    /**
     * @brief Inserts a new point into the given subpath at the specified position
     *
     * This method keeps the subpath closed if it is closed, and open when it was
     * open. So it can change the properties of the point inserted.
     * You might need to update the point before/after to get the wanted result
     * e.g. when you insert the point into a curve.
     *
     * @param point to insert
     * @param pointIndex where the point should be inserted
     *
     * @return true on success,
     *         false when pointIndex is out of bounds
     */
    bool insertPoint(KoPathPoint* point, const KoPathPointIndex &pointIndex);

    /**
     * @brief Removes point from the path.
     *
     * Note that the ownership of the point will pass to the caller.
     *
     * @param pointIndex of the point which should be removed
     *
     * @return The removed point on success,
     *         otherwise 0
     */
    KoPathPoint * removePoint(const KoPathPointIndex &pointIndex);

    /**
     * @brief Breaks the path after the point index
     *
     * The new subpath will be behind the one that was broken. The segment between
     * the given point and the one behind will be removed. If you want to split at
     * one point insert first a copy of the point behind it.
     * This does not work when the subpath is closed. Use openSubpath for this.
     * It does not break at the last position of a subpath or if there is only one
     * point in the subpath.
     *
     * @param pointIndex after which the path should be broken
     *
     * @return true if the subpath was broken, otherwise false
     */
    bool breakAfter(const KoPathPointIndex &pointIndex);

    /**
     * @brief Joins the given subpath with the following one
     *
     * Joins the given subpath with the following one by inserting a segment between
     * the two subpathes.
     * This does nothing if the given ot the following subpath is closed or on the
     * last subpath of the path.
     *
     * @param subpathIndex of the subpath which will be joined with its following one
     *
     * @return true if the subpath was joined, otherwise false
     */
    bool join(int subpathIndex);

    /**
     * @brief Move the position of a subpath within a path
     *
     * @param oldSubpathIndex old index of the subpath
     * @param newSubpathIndex new index of the subpath
     *
     * @return true if the subpath was moved, otherwise false e.g. if an index is out of bounds
     */
    bool moveSubpath(int oldSubpathIndex, int newSubpathIndex);

    /**
     * @brief Open a closed subpath
     *
     * The subpath is opened by removing the segment before the given point, makeing
     * the given point the new start point of the subpath.
     *
     * @param pointIndex to where to open the closed subpath
     * @return the new position of the old first point in the subpath
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex openSubpath(const KoPathPointIndex &pointIndex);

    /**
     * @brief Close a open subpath
     *
     * The subpath is closed be inserting a segment between the start and end point, making
     * the given point the new start point of the subpath.
     *
     * @return the new position of the old first point in the subpath
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex closeSubpath(const KoPathPointIndex &pointIndex);

    /**
     * @brief Reverse subpath
     *
     * The last point becomes the first point and the first one becomes the last one.
     *
     * @param subpathIndex the pointIndex of the subpath to reverse
     */
    bool reverseSubpath(int subpathIndex);

    /**
     * @brief Remove subpath from path
     *
     * @return The removed subpath on succes, 0 otherwise.
     */
    KoSubpath * removeSubpath(int subpathIndex);

    /**
     * @brief Add a Subpath at the given index to the path
     *
     * @return true on success, false otherwise e.g. subpathIndex out of bounds
     */
    bool addSubpath(KoSubpath * subpath, int subpathIndex);

    /**
     * @brief Combines two path by appending the data of the specified path.
     * @param path the path to combine with
     * @return true if combining was successful, else false
     */
    bool combine(KoPathShape *path);

    /**
     * @brief Creates separate path shapes, one for each existing subpath.
     * @param separatedPaths the list which contains the separated path shapes
     * @return true if separating the path was successful, false otherwise
     */
    bool separate(QList<KoPathShape*> & separatedPaths);

#ifndef NDEBUG
    /**
     * @brief print debug information about a the points of the path
     */
    void debugPath();
#endif

    /**
     * Returns the specific path shape id.
     *
     * Path shape derived shapes have a different shape id which link them
     * to their respective shape factories. In most cases they do not have
     * a special tool for editing them.
     * This function returns the specific shape id for finding the shape
     * factory from KoShapeRegistry. The default KoPathShapeId is returned
     * from KoShape::shapeId() so that the generic path editing tool gets
     * activated when the shape is selected.
     *
     * @return the specific shape id
     */
    virtual QString pathShapeId() const;

    /// Returns a odf/svg string represenatation of the path data with the given matrix applied.
    QString toString(const QMatrix &matrix) const;

    /// Returns the fill rule for the path object
    Qt::FillRule fillRule() const;

    /// Sets the fill rule to be used for painting the background
    void setFillRule(Qt::FillRule fillRule);

    /// Creates path shape from given QPainterPath
    static KoPathShape * fromQPainterPath(const QPainterPath &path);

private:
    // TODO move all the private methods to live on the d pointer object
    void map(const QMatrix &matrix);

    void updateLast(KoPathPoint ** lastPoint);

    /// closes specified subpath
    void closeSubpath(KoSubpath *subpath);
    /// close-merges specified subpath
    void closeMergeSubpath(KoSubpath *subpath);

    /**
     * @brief Get subpath at subpathIndex
     *
     * @return subPath on success, or 0 when subpathIndex is out of bounds
     */
    KoSubpath * subPath(int subpathIndex) const;
#ifndef NDEBUG
    void paintDebug(QPainter &painter);
#endif
    /// reimplemented
    virtual QString saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const;
    /// reimplemented
    virtual void loadStyle(const KoXmlElement & element, KoShapeLoadingContext &context);

protected:
    QRectF handleRect(const QPointF &p, qreal radius) const;
    /**
     * @brief add a arc.
     *
     * Adds an arc starting at the current point. The arc will be converted to bezier curves.
     * @param rx x radius of the ellipse
     * @param ry y radius of the ellipse
     * @param startAngle the angle where the arc will be started
     * @param sweepAngle the length of the angle
     * TODO add param to have angle of the ellipse
     * @param offset to the first point in the arc
     * @param curvePoints a array which take the cuve points, pass a 'QPointF curvePoins[12]';
     *
     * @return number of points created by the curve
     */
    int arcToCurve(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle, const QPointF & offset, QPointF * curvePoints) const;

    /// Returns the viewbox from the given xml element.
    QRectF loadOdfViewbox(const KoXmlElement & element) const;
    /// Applies the viewbox transformation defined in the given element
    void applyViewboxTransformation(const KoXmlElement & element);

    /**
     * @brief Save the node types
     *
     * This is inspired by inkscape and uses the same mechanism as they do.
     * The only difference is that they use sodipodi:nodeTypes as element and 
     * we use koffice:nodeTyes as attribute.
     * This attribute contains of a string which has the node type of each point
     * in it. The following node types exist:
     *
     * c corner
     * s smooth
     * z symetric
     *
     * The first point of a path is always of the type c.
     * If the path is closed the type of the first point is saved in the last elemeent
     * E.g. you have a closed path with 2 points in it. The first one (start/end of path) 
     * is symetric and the second one is smooth that will result in the nodeType="czs"
     * So if there is a closed sub path the nodeTypes contain one more entry then there 
     * are points. That is due to the first and the last pojnt of a closed sub path get 
     * merged into one when they are on the same position.
     *
     * @return The node types as string
     */
    QString nodeTypes() const;

    /**
     * @brief Load node types
     */
    void loadNodeTypes(const KoXmlElement & element);

    /**
     * Get the resize matrix
     *
     * This makes sure that also if the newSize isNull that there will be a
     * very small size of 0.000001 pixels
     */
    QMatrix resizeMatrix( const QSizeF & newSize ) const;

    KoSubpathList m_subpaths;

private:
    class Private;
    Private * const d;
};

#endif /* KOPATHSHAPE_H */
