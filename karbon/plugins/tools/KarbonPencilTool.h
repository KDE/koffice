/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KARBONPENCILTOOL_H
#define KARBONPENCILTOOL_H

#include <KToolBase.h>
#include <QtCore/QRectF>

class KPathShape;
class KLineBorder;
class KPathPoint;

class KarbonPencilTool : public KToolBase
{
    Q_OBJECT
public:
    explicit KarbonPencilTool(KCanvasBase *canvas);
    ~KarbonPencilTool();

    void paint(QPainter &painter, const KViewConverter &converter);
    void repaintDecorations();

    void mousePressEvent(KPointerEvent *event) ;
    void mouseMoveEvent(KPointerEvent *event);
    void mouseReleaseEvent(KPointerEvent *event);
    void keyPressEvent(QKeyEvent *event);

    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    void deactivate();

protected:
    virtual QWidget * createOptionWidget();

private slots:
    void selectMode(int mode);
    void setOptimize(int state);
    void setDelta(double delta);
private:

    qreal lineAngle(const QPointF &p1, const QPointF &p2);
    void addPoint(const QPointF & point);
    void finish(bool closePath);
    KLineBorder * currentBorder();

    /// returns the nearest existing path point
    KPathPoint* endPointAtPosition(const QPointF &position);

    /// Connects given path with the ones we hit when starting/finishing
    bool connectPaths(KPathShape *pathShape, KPathPoint *pointAtStart, KPathPoint *pointAtEnd);

    enum PencilMode { ModeRaw, ModeCurve, ModeStraight };

    PencilMode m_mode;
    bool m_optimizeRaw;
    bool m_optimizeCurve;
    qreal m_combineAngle;
    qreal m_fittingError;
    bool m_close;

    QList<QPointF> m_points; // the raw points

    KPathShape * m_shape;
    KPathPoint *m_existingStartPoint; ///< an existing path point we started a new path at
    KPathPoint *m_existingEndPoint;   ///< an existing path point we finished a new path at
    KPathPoint *m_hoveredPoint; ///< an existing path end point the mouse is hovering on
};

#endif // KARBONPENCILTOOL_H
