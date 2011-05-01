/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_RECALC_MANAGER
#define KSPREAD_RECALC_MANAGER

#include <KCRegion.h>

#include <QObject>

class KCCell;
class KCMap;
class KCSheet;

/**
 * \class KCRecalcManager
 * \brief Manages the recalculations of cells containing a formula.
 * \ingroup KCValue
 *
 * The recalculations of a cell region, a sheet or the map are based
 * on the following principle:
 *
 * A cell could refer to other cells, which need to be recalculated
 * before. The order of recalculation is determined by the depth of
 * references, i.e. first the cells, which do not refer to other cells,
 * are recalculated. Cells referring to those are next. Then cells, which
 * refer to the ones in the last step follow and so on until all cells
 * have been updated.
 *
 * KCCell value changes are blocked while doing this, i.e. they do not
 * trigger a new recalculation event.
 */
class KCELLS_EXPORT KCRecalcManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a KCRecalcManager. It is used for a whole map.
     *
     * \param map The KCMap which this KCRecalcManager belongs to.
     */
    explicit KCRecalcManager(KCMap *const map);

    /**
     * Destructor.
     */
    ~KCRecalcManager();

    /**
     * Recalculates the cells referring to cells in \p region .
     * The cells are recalculated sorted by the reference depth in ascending order.
     *
     * \see recalc()
     */
    void regionChanged(const KCRegion& region);

    /**
     * Recalculates the sheet \p sheet .
     * The cells are recalculated sorted by the reference depth in ascending order.
     *
     * \see recalc()
     */
    void recalcSheet(KCSheet* const sheet);

    /**
     * Recalculates the whole map.
     * The cells are recalculated sorted by the reference depth in ascending order.
     *
     * \see recalc()
     */
    void recalcMap();

    /**
     * Returns the recalculation state.
     * \return \c true, if recalculations are in progress
     */
    bool isActive() const;

    /**
     * Prints out the cell depths in the current recalculation event.
     */
    void dump() const;

public Q_SLOTS:
    /**
     * Called after a sheet was added.
     */
    void addSheet(KCSheet *sheet);

    /**
     * Called after a sheet was removed.
     */
    void removeSheet(KCSheet *sheet);

protected:
    /**
     * Iterates over the map of cell with their reference depths
     * and calls recalcCell().
     *
     * \see recalcCell()
     */
    void recalc();

private:
    Q_DISABLE_COPY(KCRecalcManager)

    class Private;
    Private * const d;
};

#endif // KSPREAD_RECALC_MANAGER
