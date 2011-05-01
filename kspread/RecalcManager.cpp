/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Local
#include "RecalcManager.h"

#include <QHash>
#include <QMap>

#include "KCCell.h"
#include "KCCellStorage.h"
#include "KCDependencyManager.h"
#include "KCFormula.h"
#include "FormulaStorage.h"
#include "KCMap.h"
#include "KCSheet.h"
#include "KCRegion.h"
#include "KCValue.h"
#include "ValueFormatter.h"

class RecalcManager::Private
{
public:
    /**
     * Finds all cells in region and their dependents, that need recalculation.
     *
     * \see RecalcManager::regionChanged
     */
    void cellsToCalculate(const KCRegion& region);

    /**
     * Finds all cells in \p sheet , that have got a formula and hence need
     * recalculation.
     * If \p sheet is zero, all cells in the KCMap a returned.
     *
     * \see RecalcManager::recalcMap
     * \see RecalcManager::recalcSheet
     */
    void cellsToCalculate(KCSheet* sheet = 0);

    /**
     * Helper function for cellsToCalculate(const KCRegion&) and cellsToCalculate(KCSheet*).
     */
    void cellsToCalculate(const KCRegion& region, QSet<KCCell>& cells) const;

    /*
     * Stores cells ordered by its reference depth.
     * Depth means the maximum depth of all cells this cell depends on plus one,
     * while a cell which has a formula without cell references has a depth
     * of zero.
     *
     * Examples:
     * \li A1: '=1.0'
     * \li A2: '=A1+A1'
     * \li A3: '=A1+A1+A2'
     *
     * \li depth(A1) = 0
     * \li depth(A2) = 1
     * \li depth(A3) = 2
     */
    QMap<int, KCCell> cells;
    const KCMap* map;
    bool active;
};

void RecalcManager::Private::cellsToCalculate(const KCRegion& region)
{
    if (region.isEmpty())
        return;

    // retrieve the cell depths
    QHash<KCCell, int> depths = map->dependencyManager()->depths();

    // create the cell map ordered by depth
    QSet<KCCell> cells;
    cellsToCalculate(region, cells);
    const QSet<KCCell>::ConstIterator end(cells.end());
    for (QSet<KCCell>::ConstIterator it(cells.begin()); it != end; ++it) {
        if ((*it).sheet()->isAutoCalculationEnabled())
            this->cells.insertMulti(depths[*it], *it);
    }
}

void RecalcManager::Private::cellsToCalculate(KCSheet* sheet)
{
    // retrieve the cell depths
    QHash<KCCell, int> depths = map->dependencyManager()->depths();

    // NOTE Stefan: It's necessary, that the cells are filled in row-wise;
    //              beginning with the top left; ending with the bottom right.
    //              This ensures, that the value storage is processed the same
    //              way, which boosts performance (using PointStorage) for an
    //              empty storage (on loading). For an already filled value
    //              storage, the speed gain is not that sensible.
    KCCell cell;
    if (!sheet) { // map recalculation
        for (int s = 0; s < map->count(); ++s) {
            sheet = map->sheet(s);
            for (int c = 0; c < sheet->formulaStorage()->count(); ++c) {
                cell = KCCell(sheet, sheet->formulaStorage()->col(c), sheet->formulaStorage()->row(c));
                cells.insertMulti(depths[cell], cell);
            }
        }
    } else { // sheet recalculation
        for (int c = 0; c < sheet->formulaStorage()->count(); ++c) {
            cell = KCCell(sheet, sheet->formulaStorage()->col(c), sheet->formulaStorage()->row(c));
            cells.insertMulti(depths[cell], cell);
        }
    }
}

void RecalcManager::Private::cellsToCalculate(const KCRegion& region, QSet<KCCell>& cells) const
{
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        const KCSheet* sheet = (*it)->sheet();
        for (int col = range.left(); col <= range.right(); ++col) {
            for (int row = range.top(); row <= range.bottom(); ++row) {
                KCCell cell(sheet, col, row);
                // Even empty cells may act as value
                // providers and need to be processed.

                // check for already processed cells
                if (cells.contains(cell))
                    continue;

                // add it to the list
                if (cell.isFormula())
                    cells.insert(cell);

                // add its consumers to the list
                cellsToCalculate(map->dependencyManager()->consumingRegion(cell), cells);
            }
        }
    }
}

RecalcManager::RecalcManager(KCMap *const map)
        : QObject(map)
        , d(new Private)
{
    d->map  = map;
    d->active = false;
}

RecalcManager::~RecalcManager()
{
    delete d;
}

void RecalcManager::regionChanged(const KCRegion& region)
{
    if (d->active || region.isEmpty())
        return;
    d->active = true;
    kDebug(36002) << "RecalcManager::regionChanged" << region.name();
    ElapsedTime et("Overall region recalculation", ElapsedTime::PrintOnlyTime);
    d->cellsToCalculate(region);
    recalc();
    d->active = false;
}

void RecalcManager::recalcSheet(KCSheet* const sheet)
{
    if (d->active)
        return;
    d->active = true;
    ElapsedTime et("Overall sheet recalculation", ElapsedTime::PrintOnlyTime);
    d->cellsToCalculate(sheet);
    recalc();
    d->active = false;
}

void RecalcManager::recalcMap()
{
    if (d->active)
        return;
    d->active = true;
    ElapsedTime et("Overall map recalculation", ElapsedTime::PrintOnlyTime);
    d->cellsToCalculate();
    recalc();
    d->active = false;
}

bool RecalcManager::isActive() const
{
    return d->active;
}

void RecalcManager::addSheet(KCSheet *sheet)
{
    // Manages also the revival of a deleted sheet.
    Q_UNUSED(sheet);
    recalcMap(); // FIXME Stefan: Implement a more elegant solution.
}

void RecalcManager::removeSheet(KCSheet *sheet)
{
    Q_UNUSED(sheet);
    recalcMap(); // FIXME Stefan: Implement a more elegant solution.
}

void RecalcManager::recalc()
{
    kDebug(36002) << "Recalculating" << d->cells.count() << " cell(s)..";
    ElapsedTime et("Recalculating cells", ElapsedTime::PrintOnlyTime);
    const QList<KCCell> cells = d->cells.values();
    for (int c = 0; c < cells.count(); ++c) {
        // only recalculate, if no circular dependency occurred
        if (cells.value(c).value() == KCValue::errorCIRCLE())
            continue;
        // Check for valid formula; parses the expression, if not done already.
        if (!cells.value(c).formula().isValid())
            continue;

        const KCSheet* sheet = cells.value(c).sheet();

        // evaluate the formula and set the result
        KCValue result = cells.value(c).formula().eval();
        if (result.isArray() && (result.columns() > 1 || result.rows() > 1)) {
            const QRect rect = cells.value(c).lockedCells();
            // unlock
            sheet->cellStorage()->unlockCells(rect.left(), rect.top());
            for (int row = rect.top(); row <= rect.bottom(); ++row) {
                for (int col = rect.left(); col <= rect.right(); ++col) {
                    KCCell(sheet, col, row).setValue(result.element(col - rect.left(), row - rect.top()));
                }
            }
            // relock
            sheet->cellStorage()->lockCells(rect);
        } else
            KCCell(cells.value(c)).setValue(result);
    }
//     dump();
    d->cells.clear();
}

void RecalcManager::dump() const
{
    QMap<int, KCCell>::ConstIterator end(d->cells.constEnd());
    for (QMap<int, KCCell>::ConstIterator it(d->cells.constBegin()); it != end; ++it) {
        KCCell cell = it.value();
        QString cellName = cell.name();
        while (cellName.count() < 4) cellName.prepend(' ');
        kDebug(36002) << "depth(" << cellName << " ) =" << it.key();
    }
}
