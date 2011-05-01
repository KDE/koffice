/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>

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

// Local
#include "DependencyManager.h"
#include "DependencyManager_p.h"

#include <QHash>
#include <QList>

#include "KCCell.h"
#include "CellStorage.h"
#include "Formula.h"
#include "FormulaStorage.h"
#include "KCMap.h"
#include "NamedAreaManager.h"
#include "KCRegion.h"
#include "RTree.h"
#include "KCSheet.h"
#include "KCValue.h"

// This is currently not called - but it's really convenient to call it from
// gdb or from debug output to check that everything is set up ok.
void DependencyManager::Private::dump() const
{
    QHash<KCCell, KCRegion>::ConstIterator mend(providers.end());
    for (QHash<KCCell, KCRegion>::ConstIterator mit(providers.begin()); mit != mend; ++mit) {
        KCCell cell = mit.key();

        QStringList debugStr;
        KCRegion::ConstIterator rend((*mit).constEnd());
        for (KCRegion::ConstIterator rit((*mit).constBegin()); rit != rend; ++rit) {
            debugStr << (*rit)->name();
        }

        kDebug(36002) << cell.name() << " consumes values of:" << debugStr.join(",");
    }

    foreach(KCSheet* sheet, consumers.keys()) {
        QList<QRectF> keys = consumers[sheet]->keys();
        QList<KCCell> values = consumers[sheet]->values();
        QHash<QString, QString> table;
        for (int i = 0; i < keys.count(); ++i) {
            KCRegion tmpRange(keys[i].toRect(), sheet);
            table.insertMulti(tmpRange.name(), values[i].name());
        }
        foreach(QString uniqueKey, table.uniqueKeys()) {
            QStringList debugStr(table.values(uniqueKey));
            kDebug(36002) << uniqueKey << " provides values for:" << debugStr.join(",");
        }
    }

    foreach(KCCell cell, depths.keys()) {
        QString cellName = cell.name();
        while (cellName.count() < 4) cellName.prepend(' ');
        kDebug(36002) << "depth(" << cellName << " ) =" << depths[cell];
    }
}

DependencyManager::DependencyManager(const KCMap* map)
        : d(new Private)
{
    d->map = map;
}

DependencyManager::~DependencyManager()
{
    qDeleteAll(d->consumers);
    delete d;
}

void DependencyManager::reset()
{
    d->reset();
}

void DependencyManager::regionChanged(const KCRegion& region)
{
    if (region.isEmpty())
        return;
    kDebug(36002) << "DependencyManager::regionChanged" << region.name();
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        const KCSheet* sheet = (*it)->sheet();

        for (int col = range.left(); col <= range.right(); ++col) {
            for (int row = range.top(); row <= range.bottom(); ++row) {
                KCCell cell(sheet, col, row);
            bool first = true;
                if (first) {
                    cell = KCCell(sheet, col, row);
                    first = false;
                } else {
                    cell = sheet->cellStorage()->nextInRow(col, row);
                }
                const Formula formula = cell.formula();

                // remove it and all its consumers from the reference depth list
                d->removeDepths(cell);

                // cell without a formula? remove it
                if (formula.expression().isEmpty()) {
                    d->removeDependencies(cell);
                    continue;
                }

                d->generateDependencies(cell, formula);
            }
        }
    }
    {
        ElapsedTime et("Computing reference depths", ElapsedTime::PrintOnlyTime);
        d->generateDepths(region);
    }
//     d->dump();
}

void DependencyManager::namedAreaModified(const QString &name)
{
    d->namedAreaModified(name);
}

void DependencyManager::addSheet(KCSheet *sheet)
{
    // Manages also the revival of a deleted sheet.
    Q_UNUSED(sheet);
#if 0 // TODO Stefan: Enable, if dependencies should not be tracked all the time.
    ElapsedTime et("Generating dependencies", ElapsedTime::PrintOnlyTime);

    // Clear orphaned dependencies (i.e. cells formerly containing formulas)
    // FIXME Stefan: Iterate only over the consumers in sheet. Needs adjustment
    //               of the way the providers are stored. Now: only by cell.
    //               Future: QHash<KCSheet*, QHash<KCCell, KCRegion> >
    const QList<KCCell> consumers = d->providers.keys();
    foreach(const KCCell& cell, consumers) {
        if (cell.sheet() == sheet) {
            // Those cells may had got providing regions. Clear them first.
            // TODO

            // Clear this cell as provider.
            d->providers.remove(cell);
        }
    }

    KCCell cell;
    for (int c = 0; c < sheet->formulaStorage()->count(); ++c) {
        cell = KCCell(sheet, sheet->formulaStorage()->col(c), sheet->formulaStorage()->row(c));

        d->generateDependencies(cell, sheet->formulaStorage()->data(c));
        if (!d->depths.contains(cell)) {
            int depth = d->computeDepth(cell);
            d->depths.insert(cell , depth);
        }
    }
#endif
}

void DependencyManager::removeSheet(KCSheet *sheet)
{
    Q_UNUSED(sheet);
    // TODO Stefan: Implement, if dependencies should not be tracked all the time.
}

void DependencyManager::updateAllDependencies(const KCMap* map)
{
    ElapsedTime et("Generating dependencies", ElapsedTime::PrintOnlyTime);

    // clear everything
    d->providers.clear();
    qDeleteAll(d->consumers);
    d->consumers.clear();
    d->namedAreaConsumers.clear();
    d->depths.clear();

    KCCell cell;
    foreach(const KCSheet* sheet, map->sheetList()) {
        for (int c = 0; c < sheet->formulaStorage()->count(); ++c) {
            cell = KCCell(sheet, sheet->formulaStorage()->col(c), sheet->formulaStorage()->row(c));

            d->generateDependencies(cell, sheet->formulaStorage()->data(c));
            if (!d->depths.contains(cell)) {
                int depth = d->computeDepth(cell);
                d->depths.insert(cell , depth);
            }
            if (!sheet->formulaStorage()->data(c).isValid())
                cell.setValue(KCValue::errorPARSE());
        }
    }
}

QHash<KCCell, int> DependencyManager::depths() const
{
    return d->depths;
}

KCRegion DependencyManager::consumingRegion(const KCCell& cell) const
{
    return d->consumingRegion(cell);
}

KCRegion DependencyManager::reduceToProvidingRegion(const KCRegion& region) const
{
    KCRegion providingRegion;
    QList< QPair<QRectF, KCCell> > pairs;
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        if (!d->consumers.contains((*it)->sheet()))
            continue;
        pairs = d->consumers.value((*it)->sheet())->intersectingPairs((*it)->rect()).values();
        for (int i = 0; i < pairs.count(); ++i)
            providingRegion.add(pairs[i].first.toRect() & (*it)->rect(), (*it)->sheet());
    }
    return providingRegion;
}

void DependencyManager::regionMoved(const KCRegion& movedRegion, const KCCell& destination)
{
    KCRegion::Point locationOffset(destination.cellPosition() - movedRegion.boundingRect().topLeft());

    KCRegion::ConstIterator end(movedRegion.constEnd());
    for (KCRegion::ConstIterator it(movedRegion.constBegin()); it != end; ++it) {
        KCSheet* const sheet = (*it)->sheet();
        locationOffset.setSheet((sheet == destination.sheet()) ? 0 : destination.sheet());

        if (d->consumers.contains(sheet)) {
            QList<KCCell> dependentLocations = d->consumers[sheet]->intersects((*it)->rect());

            for (int i = 0; i < dependentLocations.count(); ++i) {
                const KCCell cell = dependentLocations[i];
                updateFormula(cell, (*it), locationOffset);
            }
        }
    }
}

void DependencyManager::updateFormula(const KCCell& cell, const KCRegion::Element* oldLocation, const KCRegion::Point& offset)
{
    // Not a formula -> no dependencies
    if (!cell.isFormula())
        return;

    const Formula formula = cell.formula();

    // Broken formula -> meaningless dependencies
    if (!formula.isValid())
        return;

    Tokens tokens = formula.tokens();

    //return empty list if the tokens aren't valid
    if (!tokens.valid())
        return;

    QString expression('=');
    KCSheet* sheet = cell.sheet();
    for (int i = 0; i < tokens.count(); ++i) {
        Token token = tokens[i];
        Token::Type tokenType = token.type();

        //parse each cell/range and put it to our expression
        if (tokenType == Token::KCCell || tokenType == Token::Range) {
            // FIXME Stefan: Special handling for named areas
            const KCRegion region(token.text(), sheet->map(), sheet);

//             kDebug(36002) << region.name();
            // the offset contains a sheet, only if it was an intersheet move.
            if ((oldLocation->sheet() == region.firstSheet()) &&
                    (oldLocation->rect().contains(region.firstRange()))) {
                const KCRegion yetAnotherRegion(region.firstRange().translated(offset.pos()),
                                              offset.sheet() ? offset.sheet() : sheet);
                expression.append(yetAnotherRegion.name(sheet));
            } else {
                expression.append(token.text());
            }
        } else {
            expression.append(token.text());
        }
    }
    KCCell(cell).parseUserInput(expression);
}

void DependencyManager::Private::reset()
{
    providers.clear();
    consumers.clear();
}

KCRegion DependencyManager::Private::consumingRegion(const KCCell& cell) const
{
    if (!consumers.contains(cell.sheet())) {
//         kDebug(36002) << "No consumer tree found for the cell's sheet.";
        return KCRegion();
    }

    const QList<KCCell> consumers = this->consumers.value(cell.sheet())->contains(cell.cellPosition());

    KCRegion region;
    for (int i = 0; i < consumers.count(); ++i)
        region.add(consumers[i].cellPosition(), consumers[i].sheet());
    return region;
}

void DependencyManager::Private::namedAreaModified(const QString& name)
{
    // since area names are something like aliases, modifying an area name
    // basically means that all cells referencing this area should be treated
    // as modified - that will retrieve updated area ranges and also update
    // everything as necessary ...
    if (!namedAreaConsumers.contains(name))
        return;

    KCRegion region;
    const QList<KCCell> namedAreaConsumers = this->namedAreaConsumers[name];
    for (int i = 0; i < namedAreaConsumers.count(); ++i) {
        generateDependencies(namedAreaConsumers[i], namedAreaConsumers[i].formula());
        region.add(namedAreaConsumers[i].cellPosition(), namedAreaConsumers[i].sheet());
    }
    generateDepths(region);
}

void DependencyManager::Private::removeDependencies(const KCCell& cell)
{
    // look if the cell has any providers
    if (!providers.contains(cell))
        return;  //it doesn't - nothing more to do

    // first this cell is no longer a provider for all consumers
    KCRegion region = providers[cell];
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        KCSheet* const sheet = (*it)->sheet();
        const QRect range = (*it)->rect();

        if (consumers.contains(sheet)) {
            consumers[sheet]->remove(range, cell);
        }
    }

    // remove information about named area dependencies
    const QList<QString> namedAreas = namedAreaConsumers.keys();
    for (int i = 0; i < namedAreas.count(); ++i) {
        namedAreaConsumers[namedAreas[i]].removeAll(cell);
        if (namedAreaConsumers[namedAreas[i]].isEmpty())
            namedAreaConsumers.remove(namedAreas[i]);
    }

    // clear the circular dependency flags
    removeCircularDependencyFlags(providers.value(cell), Backward);
    removeCircularDependencyFlags(consumingRegion(cell), Forward);

    // finally, remove the providers for this cell
    providers.remove(cell);
}

void DependencyManager::Private::removeDepths(const KCCell& cell)
{
    if (!depths.contains(cell) || !consumers.contains(cell.sheet()))
        return;
    depths.remove(cell);
    const QList<KCCell> consumers = this->consumers.value(cell.sheet())->contains(cell.cellPosition());
    for (int i = 0; i < consumers.count(); ++i)
        removeDepths(consumers[i]);
}

void DependencyManager::Private::generateDependencies(const KCCell& cell, const Formula& formula)
{
    //new dependencies only need to be generated if the cell contains a formula
//     if (cell.isNull())
//         return;
//     if (!cell.isFormula())
//         return;

    //get rid of old dependencies first
    removeDependencies(cell);

    //now we need to generate the providing region
    computeDependencies(cell, formula);
}

void DependencyManager::Private::generateDepths(const KCRegion& region)
{
    QSet<KCCell> computedDepths;

    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        const KCSheet* sheet = (*it)->sheet();
        const CellStorage *cells = sheet->cellStorage();

        int bottom = range.bottom();
        if (bottom > cells->rows()) bottom = cells->rows();

        for (int row = range.top(); row <= bottom; ++row) {
            int col = 0;
            Formula formula = sheet->formulaStorage()->firstInRow(row, &col);
            if (col > 0 && col < range.left())
                formula = sheet->formulaStorage()->nextInRow(col, row, &col);
            while (col != 0 && col <= range.right()) {
                KCCell cell(sheet, col, row);

                // compute the cell depth and automatically the depths of its providers
                int depth = computeDepth(cell);
                depths.insert(cell, depth);

                // compute the consumers' depths
                if (!consumers.contains(cell.sheet())) {
                    formula = sheet->formulaStorage()->nextInRow(col, row, &col);
                    continue;
                }
                const QList<KCCell> consumers = this->consumers.value(cell.sheet())->contains(cell.cellPosition());
                for (int i = 0; i < consumers.count(); ++i) {
                    if (!region.contains(consumers[i].cellPosition(), consumers[i].sheet()))
                        generateDepths(consumers[i], computedDepths);
                }

                formula = sheet->formulaStorage()->nextInRow(col, row, &col);
            }
        }
    }
}

void DependencyManager::Private::generateDepths(KCCell cell, QSet<KCCell>& computedDepths)
{
    static QSet<KCCell> processedCells;

    //prevent infinite recursion (circular dependencies)
    if (processedCells.contains(cell) || cell.value() == KCValue::errorCIRCLE()) {
        kDebug(36002) << "Circular dependency at" << cell.fullName();
        cell.setValue(KCValue::errorCIRCLE());
        depths.insert(cell, 0);
        return;
    }
    if (computedDepths.contains(cell)) {
        return;
    }

    // set the compute reference depth flag
    processedCells.insert(cell);

    int depth = computeDepth(cell);
    depths.insert(cell, depth);

    computedDepths.insert(cell);

    // Recursion. We need the whole dependency tree of the changed region.
    // An infinite loop is prevented by the check above.
    if (!consumers.contains(cell.sheet()))
        return;
    const QList<KCCell> consumers = this->consumers.value(cell.sheet())->contains(cell.cellPosition());
    for (int i = 0; i < consumers.count(); ++i)
        generateDepths(consumers[i], computedDepths);

    // clear the compute reference depth flag
    processedCells.remove(cell);
}

int DependencyManager::Private::computeDepth(KCCell cell) const
{
    // a set of cell, which depth is currently calculated
    static QSet<KCCell> processedCells;

    //prevent infinite recursion (circular dependencies)
    if (processedCells.contains(cell) || cell.value() == KCValue::errorCIRCLE()) {
        kDebug(36002) << "Circular dependency at" << cell.fullName();
        cell.setValue(KCValue::errorCIRCLE());
        return 0;
    }

    // set the compute reference depth flag
    processedCells.insert(cell);

    int depth = 0;

    const KCRegion region = providers.value(cell);

    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        KCSheet* sheet = (*it)->sheet();
        const int right = range.right();
        const int bottom = range.bottom();
        for (int col = range.left(); col <= right; ++col) {
            for (int row = range.top(); row <= bottom; ++row) {
                KCCell referencedCell = KCCell(sheet, col, row);
                if (!providers.contains(referencedCell)) {
                    // no further references
                    // depth is one at least
                    depth = qMax(depth, 1);
                    continue;
                }

                if (depths.contains(referencedCell)) {
                    // the referenced cell depth was already computed
                    depth = qMax(depths[referencedCell] + 1, depth);
                    continue;
                }

                // compute the depth of the referenced cell, add one and
                // take it as new depth, if it's greater than the current one
                depth = qMax(computeDepth(referencedCell) + 1, depth);
            }
        }
    }

    //clear the computing reference depth flag
    processedCells.remove(cell);

    return depth;
}

void DependencyManager::Private::computeDependencies(const KCCell& cell, const Formula& formula)
{
    // Broken formula -> meaningless dependencies
    if (!formula.isValid())
        return;

    const Tokens tokens = formula.tokens();

    //return empty list if the tokens aren't valid
    if (!tokens.valid())
        return;

    KCSheet* sheet = cell.sheet();
    int inAreasCall = 0;
    KCRegion providingRegion;
    for (int i = 0; i < tokens.count(); i++) {
        const Token token = tokens[i];

        if (inAreasCall) {
            if (token.isOperator() && token.asOperator() == Token::LeftPar)
                inAreasCall++;
            else if (token.isOperator() && token.asOperator() == Token::RightPar)
                inAreasCall--;
        } else {
            if (i > 0 && token.isOperator() && token.asOperator() == Token::LeftPar && tokens[i-1].isIdentifier() && QString::compare(tokens[i-1].text(), "AREAS", Qt::CaseInsensitive) == 0)
                inAreasCall = 1;

            //parse each cell/range and put it to our KCRegion
            if (token.type() == Token::KCCell || token.type() == Token::Range) {
                // check for named area
                const bool isNamedArea = sheet->map()->namedAreaManager()->contains(token.text());
                if (isNamedArea) {
                    // add cell as consumer of the named area
                    namedAreaConsumers[token.text()].append(cell);
                }

                // check if valid cell/range
                KCRegion region(token.text(), sheet->map(), sheet);
                if (region.isValid()) {
                    if (isNamedArea) {
                        if ((i > 0 && tokens[i-1].isOperator()) || (i < tokens.count()-1 && tokens[i+1].isOperator())) {
                            // TODO: this check is not quite correct, to really properly determine if the entire range is referenced
                            // or just a single cell we would need to actually have the compile formula, not just the tokenized one
                            // basically this is the same logic as Formula::Private::valueOrElement

                            KCRegion realRegion = region.intersectedWithRow(cell.row());
                            if (!realRegion.isEmpty()) {
                                region = realRegion;
                            }
                        }
                    }
                    // add it to the providers
                    providingRegion.add(region);

                    KCSheet* sheet = region.firstSheet();

                    // create consumer tree, if not existing yet
                    if (!consumers.contains(sheet)) consumers.insert(sheet, new RTree<KCCell>());
                    // add cell as consumer of the range
                    consumers[sheet]->insert(region.firstRange(), cell);
                }
            }
        }
    }

    // store the providing region
    // NOTE Stefan: Also store cells without dependencies to avoid an
    //              iteration over all cells in a map/sheet on recalculation.
    // empty region will be created automatically, if necessary
    providers[cell].add(providingRegion);
}

void DependencyManager::Private::removeCircularDependencyFlags(const KCRegion& region, Direction direction)
{
    // a set of cells, which circular dependency flag is currently removed
    static QSet<KCCell> processedCells;

    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it(region.constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        const KCSheet* sheet = (*it)->sheet();
        for (int col = range.left(); col <= range.right(); ++col) {
            for (int row = range.top(); row <= range.bottom(); ++row) {
                KCCell cell(sheet, col, row);

                if (processedCells.contains(cell))
                    continue;
                processedCells.insert(cell);

                if (cell.value() == KCValue::errorCIRCLE())
                    cell.setValue(KCValue::empty());

                if (direction == Backward)
                    removeCircularDependencyFlags(providers.value(cell), Backward);
                else // Forward
                    removeCircularDependencyFlags(consumingRegion(cell), Forward);

                processedCells.remove(cell);
            }
        }
    }
}

#include "DependencyManager.moc"
