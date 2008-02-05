/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KOPATHTOOL_H
#define KOPATHTOOL_H

#include "KoPathPoint.h"
#include "KoPathShape.h"
#include "KoTool.h"

class QButtonGroup;
class KoCanvasBase;
class KoParameterShape;
class KoInteractionStrategy;
class KoPathPointMoveStrategy;
class KoPathPointRubberSelectStrategy;
class KoPathPointData;

class QAction;

/// The tool for editing a KoPathShape or a KoParameterShape
class FLAKE_TEST_EXPORT KoPathTool : public KoTool {
    Q_OBJECT
public:
    explicit KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool();

    /// reimplemented
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented
    virtual void repaintDecorations();

    /// reimplemented
    virtual void mousePressEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented
    virtual void keyReleaseEvent(QKeyEvent *event);

    /// reimplemented
    virtual void activate (bool temporary=false);
    /// reimplemented
    virtual void deactivate();

    /// reimplemented
    virtual void deleteSelection();

signals:
    void typeChanged(int types);
    void pathChanged(KoPathShape* path);
protected:
    /// reimplemented
    virtual QWidget * createOptionWidget();

private:
    /**
     * @brief Select points in rect
     *
     * @param rect in document coordinated
     * @param clearSelection if set clear the current selection before the selection
     */
    void selectPoints( const QRectF &rect, bool clearSelection );

    void updateOptionsWidget();
    void updateActions();

    /// repaints the specified rect
    void repaint( const QRectF &repaintRect );
    /// returns a handle rect at the given position
    QRectF handleRect( const QPointF &p );

    // needed for interaction strategy
    QPointF m_lastPoint;

private slots:
    void pointTypeChanged( QAction *type );
    void insertPoints();
    void removePoints();
    void segmentToLine();
    void segmentToCurve();
    void convertToPath();
    void joinPoints();
    void breakAtPoint();
    void breakAtSegment();
    void resourceChanged( int key, const QVariant & res );
private:

    class ActiveHandle
    {
    public:
        ActiveHandle( KoPathTool *tool )
        : m_tool( tool )
        {}
        virtual ~ActiveHandle() {}
        virtual void paint( QPainter &painter, const KoViewConverter &converter ) = 0;
        virtual void repaint() const = 0;
        virtual void mousePressEvent( KoPointerEvent *event ) = 0;
        // test if handle is still valid
        virtual bool check() = 0;

        KoPathTool *m_tool;
    };

    class ActivePointHandle : public ActiveHandle
    {
    public:
        ActivePointHandle( KoPathTool *tool, KoPathPoint *activePoint, KoPathPoint::KoPointType activePointType )
        : ActiveHandle( tool )
        , m_activePoint( activePoint )
        , m_activePointType( activePointType )
        {}
        void paint( QPainter &painter, const KoViewConverter &converter );
        void repaint() const;
        void mousePressEvent( KoPointerEvent *event );
        bool check();

        KoPathPoint *m_activePoint;
        KoPathPoint::KoPointType m_activePointType;
    };

    class ActiveParameterHandle : public ActiveHandle
    {
    public:
        ActiveParameterHandle( KoPathTool *tool, KoParameterShape *parameterShape, int handleId )
        : ActiveHandle( tool )
        , m_parameterShape( parameterShape )
        , m_handleId( handleId )
        {}
        void paint( QPainter &painter, const KoViewConverter &converter );
        void repaint() const;
        void mousePressEvent( KoPointerEvent *event );
        bool check();

        KoParameterShape *m_parameterShape;
        int m_handleId;
    };

    class ActiveConnectionHandle : public ActiveParameterHandle
    {
    public:
        ActiveConnectionHandle( KoPathTool *tool, KoParameterShape *parameterShape, int handleId )
        : ActiveParameterHandle( tool, parameterShape, handleId )
            {
            }

        // XXX: Later: create a paint even to distinguish a connection
        // handle  from another handle type
        void mousePressEvent( KoPointerEvent *event );
    };

    /**
     * @brief Handle the selection of points
     *
     * This class handles the selection of points. It makes sure
     * the canvas is repainted when the selection changes.
     */
    class FLAKE_TEST_EXPORT KoPathPointSelection
    {
    public:
        explicit KoPathPointSelection( KoPathTool * tool )
        : m_tool( tool )
        {}
        ~KoPathPointSelection() {}

        /// @brief Draw the selected points
        void paint( QPainter &painter, const KoViewConverter &converter );

        /**
         * @brief Add a point to the selection
         *
         * @param point to add to the selection
         * @param clear if true the selection will be cleared before adding the point
         */
        void add( KoPathPoint * point, bool clear );

        /**
         * @brief Remove a point form the selection
         *
         * @param point to remove from the selection
         */
        void remove( KoPathPoint * point );

        /**
         * @brief Clear the selection
         */
        void clear();

        /**
         * @brief Get the number of path objects in the selection
         *
         * @return number of path object in the point selection
         */
        int objectCount() const { return m_shapePointMap.size(); }

        /**
         * @brief Get the number of path objects in the selection
         *
         * @return number of points in the selection
         */
        int size() const { return m_selectedPoints.size(); }

        /**
         * @brief Check if a point is in the selection
         *
         * @return true when the point is in the selection, false otherwise
         */
        bool contains( KoPathPoint * point ) { return m_selectedPoints.contains( point ); }

        /**
         * @brief Get all selected points
         *
         * @return set of selected points
         */
        const QSet<KoPathPoint *> & selectedPoints() const { return m_selectedPoints; }

        /**
         * @brief Get the point data of all selected points
         *
         * This is subject to change
         */
        QList<KoPathPointData> selectedPointsData() const;

        /**
         * @brief Get the point data of all selected segments
         *
         * This is subject to change
         */
        QList<KoPathPointData> selectedSegmentsData() const;

        /**
         * @brief Get the selected point map
         *
         * @return KoSelectedPointMap containing all objects and selected points
         * typedef QMap<KoPathShape *, QSet<KoPathPoint *> > KoSelectedPointMap;
         */
        const KoPathShapePointMap & selectedPointMap() const { return m_shapePointMap; }

        /**
         * @brief trigger a repaint
         */
        void repaint();

        /**
         * @brief Update the selection to contain only valid points
         *
         * This function checks which points are no longer valid and removes them
         * from the selection.
         * If e.g. some points are selected and the shape which contains the points
         * is removed by undo, the points are no longer valid and have therefore to
         * be removed from the selection.
         */
        void update();

        /// returns if the selection has any points selected
        bool isEmpty() const { return m_selectedPoints.isEmpty(); }

    private:
        QSet<KoPathPoint *> m_selectedPoints;
        KoPathShapePointMap m_shapePointMap;
        KoPathTool * m_tool;
    };


    ActiveHandle * m_activeHandle;       ///< the currently active handle
    int m_handleRadius;                ///< the radius of the control point handles
    /// the point selection
    KoPathPointSelection m_pointSelection;

    friend class ActiveNoHandle;
    friend class ActivePointHandle;
    friend class KoPathPointSelection;
    friend class KoPathPointMoveStrategy;
    friend class KoPathControlPointMoveStrategy;
    friend class KoPathPointRubberSelectStrategy;

    // make a frind so that it can test private member/methods
    friend class TestPathTool;

    KoInteractionStrategy *m_currentStrategy; ///< the rubber selection strategy

    QButtonGroup *m_pointTypeGroup;
    QList<KoPathShape*> m_selectedShapes;

    QAction *m_actionPathPointCorner;
    QAction *m_actionPathPointSmooth;
    QAction *m_actionPathPointSymmetric;
    QAction *m_actionLineSegment;
    QAction *m_actionCurveSegment;
    QAction *m_actionAddPoint;
    QAction *m_actionRemovePoint;
    QAction *m_actionBreakPoint;
    QAction *m_actionBreakSegment;
    QAction *m_actionJoinSegment;
    QAction *m_actionMergePoints;
    QAction *m_actionConvertToPath;

    QCursor m_selectCursor;
    QCursor m_moveCursor;
};

#endif
