/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KCELLS_SHEET_VIEW
#define KCELLS_SHEET_VIEW

#include <QObject>
#include <QPoint>
#include <QPointF>

#include "kcells_export.h"

class QPainter;
class QRect;
class QRectF;
class QSize;
class QSizeF;

class KViewConverter;

class CellView;
class KCRegion;
class KCSheet;

/**
 * \ingroup Painting
 * The SheetView controls the painting of the sheets' cells.
 * It caches a set of CellViews.
 */
class KCELLS_EXPORT SheetView : public QObject
{
    Q_OBJECT

    friend class CellView;

public:
    /**
     * Constructor.
     */
    explicit SheetView(const KCSheet* sheet);

    /**
     * Destructor.
     */
    ~SheetView();

    /**
     * \return the KCSheet
     */
    const KCSheet* sheet() const;

    /**
     * Sets the KViewConverter used by this SheetView.
     */
    void setViewConverter(const KViewConverter* viewConverter);

    /**
     * \return the view in which the KCSheet is painted
     */
    const KViewConverter* viewConverter() const;

    /**
     * Looks up a CellView for the position \p col , \p row in the cache.
     * If no CellView exists yet, one is created and inserted into the cache.
     *
     * \return the CellView for the position
     */
    const CellView& cellView(int col, int row);

    /**
     * Set the cell range, that should be painted to \p rect .
     * It also adjusts the cache size linear to the size of \p rect .
     */
    void setPaintCellRange(const QRect& rect);

    /**
     * Invalidates all cached CellViews in \p region .
     */
    void invalidateRegion(const KCRegion& region);

    /**
     * Invalidates all CellViews, the cached and the default.
     */
    void invalidate();

    /**
     * Paints the cells.
     */
    void paintCells(QPainter& painter, const QRectF& paintRect, const QPointF& topLeft);

public Q_SLOTS:
    void updateAccessedCellRange(const QPoint& location = QPoint());

Q_SIGNALS:
    void visibleSizeChanged(const QSizeF&);

private:
    /**
     * Helper method for invalidateRegion().
     * Invalidates all cached CellViews in \p range .
     * \internal
     */
    void invalidateRange(const QRect& range);

    /**
     * Marks other CellViews in \p range as obscured by the CellView at \p position .
     * Used by CellView.
     */
    void obscureCells(const QRect& range, const QPoint& position);

    /**
     * Returns the default CellView.
     * Used by CellView.
     */
    const CellView& defaultCellView() const;

    Q_DISABLE_COPY(SheetView)

    class Private;
    Private * const d;
};

#endif // KCELLS_SHEET_VIEW
