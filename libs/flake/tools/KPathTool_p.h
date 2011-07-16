/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KPATHTOOL_H
#define KPATHTOOL_H

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


#include "../KPathShape.h"
#include "../KToolBase.h"
#include "KPathToolSelection_p.h"

class QButtonGroup;
class KCanvasBase;
class KInteractionStrategy;
class KPathToolHandle;
class PathToolOptionWidget;

class KAction;

/// The tool for editing a KPathShape or a KParameterShape
class FLAKE_TEST_EXPORT KPathTool : public KToolBase
{
    Q_OBJECT
public:
    explicit KPathTool(KCanvasBase *canvas);
    ~KPathTool();

    /// reimplemented
    virtual void paint(QPainter &painter, const KViewConverter &converter);

    /// reimplemented
    virtual void repaintDecorations();

    /// reimplemented
    virtual void mousePressEvent(KPointerEvent *event);
    /// reimplemented
    virtual void mouseMoveEvent(KPointerEvent *event);
    /// reimplemented
    virtual void mouseReleaseEvent(KPointerEvent *event);
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented
    virtual void keyReleaseEvent(QKeyEvent *event);
    /// reimplemented
    virtual void mouseDoubleClickEvent(KPointerEvent *event);
    /// reimplemented
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    /// reimplemented
    virtual void deactivate();

    /// reimplemented
    virtual void deleteSelection();

    /// reimplemented
    virtual KToolSelection* selection();

    /// repaints the specified rect
    void repaint(const QRectF &repaintRect);

protected:
    /// reimplemented
    virtual QMap<QString, QWidget *>  createOptionWidgets();

private:
    void updateOptionsWidget();
    bool segmentAtPoint(const QPointF &point, KPathShape* &shape, KPathPoint* &segmentStart, qreal &pointParam);

private slots:
    void pointTypeChanged(QAction *type);
    void insertPoints();
    void removePoints();
    void segmentToLine();
    void segmentToCurve();
    void convertToPath();
    void joinPoints();
    void mergePoints();
    void breakAtPoint();
    void breakAtSegment();
    void resourceChanged(int key, const QVariant &res);
    void pointSelectionChanged();
    void updateActions();
    void pointToLine();
    void pointToCurve();
    void activate();

private:
    KPathToolHandle *m_activeHandle;       ///< the currently active handle
    int m_handleRadius;    ///< the radius of the control point handles
    uint m_grabSensitivity; ///< the grab sensitivity
    /// the point selection
    KPathToolSelection m_pointSelection;
    // needed for interaction strategy
    QPointF m_lastPoint;

    // make a frind so that it can test private member/methods
    friend class TestPathTool;

    KInteractionStrategy *m_currentStrategy; ///< the rubber selection strategy

    QButtonGroup *m_pointTypeGroup;

    KAction *m_actionPathPointCorner;
    KAction *m_actionPathPointSmooth;
    KAction *m_actionPathPointSymmetric;
    KAction *m_actionCurvePoint;
    KAction *m_actionLinePoint;
    KAction *m_actionLineSegment;
    KAction *m_actionCurveSegment;
    KAction *m_actionAddPoint;
    KAction *m_actionRemovePoint;
    KAction *m_actionBreakPoint;
    KAction *m_actionBreakSegment;
    KAction *m_actionJoinSegment;
    KAction *m_actionMergePoints;
    KAction *m_actionConvertToPath;
    QCursor m_selectCursor;
    QCursor m_moveCursor;

    PathToolOptionWidget *m_toolOptionWidget;

    Q_DECLARE_PRIVATE(KToolBase)
};

#endif
