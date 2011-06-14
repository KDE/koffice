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

#ifndef KOPATHTOOLSELECTION_H
#define KOPATHTOOLSELECTION_H

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


#include <KoToolSelection.h>
#include <KPathShape.h>

class KPathTool;
class KPathPoint;
class KPathPointData;
class KoViewConverter;
class QPainter;

/**
* @brief Handle the selection of points
*
* This class handles the selection of points. It makes sure
* the canvas is repainted when the selection changes.
*/
class FLAKE_TEST_EXPORT KoPathToolSelection : public KoToolSelection
{
    Q_OBJECT

public:
    explicit KoPathToolSelection(KPathTool *tool);

    ~KoPathToolSelection();

    /// @brief Draw the selected points
    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
    * @brief Add a point to the selection
    *
    * @param point to add to the selection
    * @param clear if true the selection will be cleared before adding the point
    */
    void add(KPathPoint *point, bool clear);

    /**
    * @brief Remove a point form the selection
    *
    * @param point to remove from the selection
    */
    void remove(KPathPoint *point);

    /**
    * @brief Clear the selection
    */
    void clear();

    /**
     * @brief Select points in rect
     *
     * @param rect the selection rectangle in document coordinates
     * @param clearSelection if set clear the current selection before the selection
     */
    void selectPoints(const QRectF &rect, bool clearSelection);

    /**
    * @brief Get the number of path objects in the selection
    *
    * @return number of path object in the point selection
    */
    int objectCount() const;

    /**
    * @brief Get the number of path points in the selection
    *
    * @return number of points in the selection
    */
    int size() const;

    /**
    * @brief Check if a point is in the selection
    *
    * @return true when the point is in the selection, false otherwise
    */
    bool contains(KPathPoint *point);

    /**
    * @brief Get all selected points
    *
    * @return set of selected points
    */
    const QSet<KPathPoint *> &selectedPoints() const;

    /**
    * @brief Get the point data of all selected points
    *
    * This is subject to change
    */
    QList<KPathPointData> selectedPointsData() const;

    /**
    * @brief Get the point data of all selected segments
    *
    * This is subject to change
    */
    QList<KPathPointData> selectedSegmentsData() const;

    /// Returns list of selected shapes
    QList<KPathShape*> selectedShapes() const;

    /// Sets list of selected shapes
    void setSelectedShapes(const QList<KPathShape*> shapes);

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

    /// reimplemented from KoToolSelection
    virtual bool hasSelection();

signals:
    void selectionChanged();

private:
    typedef QMap<KPathShape *, QSet<KPathPoint *> > PathShapePointMap;

    QSet<KPathPoint *> m_selectedPoints;
    PathShapePointMap m_shapePointMap;
    KPathTool *m_tool;
    QList<KPathShape*> m_selectedShapes;
};

#endif // PATHTOOLSELECTION_H
