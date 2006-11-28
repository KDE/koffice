/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPATHCOMMAND_H
#define KOPATHCOMMAND_H

#include <kcommand.h>
#include <QList>
#include <QMap>
#include <QPointF>
#include "KoPathShape.h"
#include <koffice_export.h>

class KoParameterShape;

/// the base command for commands altering a path shape
class KoPathBaseCommand : public KCommand {
public:
    /// initialize the base command with the shape
    KoPathBaseCommand( KoPathShape *shape );
protected:
    /**
     * Call this to repaint the shape after altering.
     * @param oldControlPointRect the control point rect of the shape before altering
     */
    void repaint( const QRectF &oldControlPointRect );
    KoPathShape *m_shape; ///< the shape the command operates on
};

/// The undo / redo command for path point moving.
class KoPointMoveCommand : public KCommand 
{
public:
    /**
     * Command to move path point.
     * @param pointMap map of the path point to move
     * @param offset the offset by which the point is moved in document coordinates
     */
    KoPointMoveCommand( const KoPathShapePointMap &pointMap, const QPointF &offset );

    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathShapePointMap m_pointMap;
    QPointF m_offset;
};

/// The undo / redo command for path point moving.
class KoControlPointMoveCommand : public KCommand
{
public:
    /**
     * Command to move one control path point.
     * @param point the path point to control point belongs to
     * @param offset the offset by which the point is moved in document coordinates
     * @param pointType the type of the point to move
     */
    KoControlPointMoveCommand( KoPathPoint *point, const QPointF &offset, KoPathPoint::KoPointType pointType );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathPoint * m_point;
    // the offset in shape coordinates
    QPointF m_offset;
    KoPathPoint::KoPointType m_pointType;
};

/// The undo / redo command for changing the path point type.
class KoPointPropertyCommand : public KoPathBaseCommand {
public:
    /**
     * Command to change the type of the point
     * @param shape the path shape containing the point
     * @param point the path point to move
     * @param property ??
     */
    KoPointPropertyCommand( KoPathShape *shape, KoPathPoint *point, KoPathPoint::KoPointProperties property );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathPoint* m_point;
    KoPathPoint::KoPointProperties m_newProperties;
    KoPathPoint::KoPointProperties m_oldProperties;
    QPointF m_controlPoint1;
    QPointF m_controlPoint2;
};

/// The undo / redo command for removing path points.
class KoPointRemoveCommand : public KCommand {
public:
    /**
     * @brief Command to remove a points from path shapes
     * @param pointMap map of the path points to remove
     */
    KoPointRemoveCommand( const KoPathShapePointMap &pointMap );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    struct KoPointRemoveData
    {
        KoPointRemoveData( KoPathPoint * point, KoSubpath * subpath, int position )
        : m_point( point )
        , m_subpath( subpath )
        , m_position( position )
        {}
        KoPathPoint * m_point;
        KoSubpath * m_subpath;///< the position in the path 
        int m_position;
    };
    KoPathShapePointMap m_pointMap;
    QList<KoPointRemoveData> m_data;
};

/// The undo / redo command for splitting a path segment
class KoSegmentSplitCommand : public KoPathBaseCommand
{
public:
    /**
     * Command to split a single path segment at the given position
     * @param shape the path shape containing the points
     * @param segment the segment to split
     * @param splitPosition the position to split at [0..1]
     */
    KoSegmentSplitCommand( KoPathShape *shape, const KoPathSegment &segment, double splitPosition );
    /**
     * Command to split multiple path segments at different positions
     * @param shape the path shape containing the points
     * @param segments the segments to split
     * @param splitPositions the positions to split at [0..1]
     */
    KoSegmentSplitCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, const QList<double> &splitPositions );
    /**
     * Command to split multiple path segments at the same position
     * @param shape the path shape containing the points
     * @param segments the segments to split
     * @param splitPosition the position to split at [0..1]
     */
    KoSegmentSplitCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, double splitPosition );
    virtual ~KoSegmentSplitCommand();
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    QList<KoPathSegment> m_segments;
    typedef QPair<KoPathPoint,KoPathPoint> KoSegmentData;
    QList<KoSegmentData> m_oldNeighbors;
    QList<KoSegmentData> m_newNeighbors;
    QList<double> m_splitPos;
    QList<KoPathPoint*> m_splitPoints;
    bool m_deletePoint;
    QList< QPair<KoSubpath*,int> > m_splitPointPos;
};

/// The undo / redo command for joining two start/end path points
class KoPointJoinCommand : public KoPathBaseCommand
{
public:
    /**
     * Command to join two start/end path points.
     * @param shape the path shape whose points to join
     * @param point1 the first point of the subpath to join
     * @param point2 the second point of the subpath to join
     */
    KoPointJoinCommand( KoPathShape *shape, KoPathPoint *point1, KoPathPoint *point2 );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathPoint* m_point1;
    KoPathPoint* m_point2;
    bool m_joined;
};

/// The undo / redo command for breaking a subpath
class KoSubpathBreakCommand : public KoPathBaseCommand
{
public:
    /**
     * Command to break a subpath at a single point.
     * @param shape the path shape whose subpath to close
     * @param breakPoint the point to break at
     */
    KoSubpathBreakCommand( KoPathShape *shape, KoPathPoint *breakPoint );
    /**
     * Command to break a subpath at a path segment
     * @param shape the path shape whose subpath to close
     * @param segment the segment
     */
    KoSubpathBreakCommand( KoPathShape *shape, const KoPathSegment &segment );
    virtual ~KoSubpathBreakCommand();
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathPoint* m_breakPoint;
    KoPathSegment m_segment;
    bool m_breakSegment;
    bool m_broken;
    KoPathPoint* m_newPoint;
    KoPathPoint m_pointData1; ///< data of the first point to restore
    KoPathPoint m_pointData2; ///< data of the second point to restore 
};

/// The undo / redo command for changing a segments type (curve/line)
class KoSegmentTypeCommand : public KoPathBaseCommand
{
public:
    /**
     * Command for changing a segments type (curve/line)
     * @param shape the path shape whose subpath to close
     * @param segment the segment to change the type of
     * @param changeToLine if true, changes segment to line, else changes segment to curve
     */
    KoSegmentTypeCommand( KoPathShape *shape, const KoPathSegment &segment, bool changeToLine );
    /**
     * Command for changing a segments type (curve/line)
     * @param shape the path shape whose subpath to close
     * @param segments the segments to change the type of
     * @param changeToLine if true, changes segments to lines, else changes segments to curves
     */
    KoSegmentTypeCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, bool changeToLine );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    QList<KoPathSegment> m_segments;
    QMap<KoPathPoint*,KoPathPoint> m_oldPointData;
    bool m_changeToLine;
};

class KoShapeControllerBase;

/// The undo / redo command for combining two or more paths into one
class FLAKE_EXPORT KoPathCombineCommand : public KCommand
{
public:
    /**
     * Command for combining a list of paths into one single path.
     * @param controller the controller to used for removing/inserting.
     * @param paths the list of paths to combine
     */
    KoPathCombineCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths );
    virtual ~KoPathCombineCommand();
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoShapeControllerBase *m_controller;
    QList<KoPathShape*> m_paths;
    KoPathShape *m_combinedPath;
    bool m_isCombined;
};

/// The undo / redo command for changing a parameter
class KoParameterChangeCommand : public KCommand
{
public:
    KoParameterChangeCommand( KoParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint );
    virtual ~KoParameterChangeCommand();

    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoParameterShape *m_shape;
    int m_handleId;
    QPointF m_startPoint;
    QPointF m_endPoint;
};

/// The undo / redo command for changing a KoParameterShape into a KoPathShape
class KoParameterToPathCommand : public KCommand
{
public:
    KoParameterToPathCommand( KoParameterShape *shape );
    virtual ~KoParameterToPathCommand();

    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoParameterShape *m_shape;
};

/// The undo / redo command for separating subpaths into different paths
class FLAKE_EXPORT KoPathSeparateCommand : public KCommand
{
public:
    /**
     * Command for separating subpaths of a list of paths into different paths.
     * @param controller the controller to used for removing/inserting.
     * @param paths the list of paths to separate
     */
    KoPathSeparateCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths );
    virtual ~KoPathSeparateCommand();
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoShapeControllerBase *m_controller;
    QList<KoPathShape*> m_paths;
    QList<KoPathShape*> m_separatedPaths;
    bool m_isSeparated;
};

#endif
