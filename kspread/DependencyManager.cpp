/* This file is part of the KDE project
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

#include <QHash>
#include <QLinkedList>

#include "Cell.h"
#include "CellStorage.h"
#include "Formula.h"
#include "Map.h"
#include "Region.h"
#include "RTree.h"
#include "Sheet.h"
#include "Value.h"

#include "DependencyManager.h"

using namespace KSpread;

class DependencyManager::Private
{
public:
    /**
     * Clears internal structures.
     */
    void reset();

    /**
     * Generates the dependencies of \p cell .
     * First, it removes the old providing region. Then, the new providing
     * region is computed. Finally, adds \p cell as consumer and the new
     * providing region to the data structures.
     * \see removeDependencies
     * \see computeDependencies
     * \see addDependencies
     */
    void generateDependencies(const Cell& cell, const Formula& formula);

    /**
     * Computes the reference depth.
     * Depth means the maximum depth of all cells this cell depends on plus one,
     * while a cell, which do not refer to other cells, has a depth
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
    int computeDepth(Cell cell) const;

    /**
     * Used in the recalculation events for changed regions.
     * Determines the reference depth for each position in \p region .
     * Calls itself recursively for the regions, that depend on cells
     * in \p region .
     *
     * \see computeDepth
     */
    void generateDepths(const Region& region);

    /**
     * Returns the region, that consumes the value of \p cell.
     * \return region consuming \p cell 's value
     */
    Region consumingRegion(const Cell& cell) const;

    void areaModified (const QString &name);

    /**
     * Updates structures: \p cell depends on cells in \p region and vice versa.
     */
    void addDependencies(const Cell& cell, const Region& region);

    /**
     * Removes all dependencies of \p cell .
     */
    void removeDependencies(const Cell& cell);

    /**
     * \return a list of cells that \p cell depends on
     */
    Region computeDependencies(const Cell& cell, const Formula& formula) const;

    enum Direction { Forward, Backward };
    /**
     * Removes the circular dependency flag from \p region and all their dependencies.
     */
    void removeCircularDependencyFlags( const Region& region, Direction direction );

    /**
     * For debugging/testing purposes.
     */
    void dump() const;

    const Map* map;
    // stores providing regions ordered by their consuming cell locations
    QHash<Cell, Region> providers;
    // stores consuming cell locations ordered by their providing regions
    QHash<Sheet*, RTree<Cell>*> consumers;
    // list of cells referencing a given named area
    QHash<QString, QHash<Cell, bool> > areaDeps;
    /*
     * Stores cells with its reference depth.
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
    QHash<Cell, int> depths;
};

// This is currently not called - but it's really convenient to call it from
// gdb or from debug output to check that everything is set up ok.
void DependencyManager::Private::dump() const
{
    QHash<Cell, Region>::ConstIterator mend(providers.end());
    for ( QHash<Cell, Region>::ConstIterator mit(providers.begin()); mit != mend; ++mit )
    {
        Cell cell = mit.key();

        QStringList debugStr;
        Region::ConstIterator rend((*mit).constEnd());
        for ( Region::ConstIterator rit((*mit).constBegin()); rit != rend; ++rit )
        {
            debugStr << (*rit)->name();
        }

        kDebug(36002) << cell.name() << " consumes values of: " << debugStr.join(", ") << endl;
    }

    foreach (Sheet* sheet, consumers.keys())
    {
        QList<QRectF> keys = consumers[sheet]->keys();
        QList<Cell> values = consumers[sheet]->values();
        QHash<QString, QString> table;
        for (int i = 0; i < keys.count(); ++i)
        {
            Region tmpRange(keys[i].toRect(), sheet);
            table.insertMulti(tmpRange.name(), values[i].name());
        }
        foreach (QString uniqueKey, table.uniqueKeys())
        {
            QStringList debugStr(table.values(uniqueKey));
            kDebug(36002) << uniqueKey << " provides values for: " << debugStr.join(", ") << endl;
        }
    }

    foreach ( Cell cell, depths.keys() )
    {
        QString cellName = cell.name();
        while ( cellName.count() < 4 ) cellName.prepend( ' ' );
        kDebug(36002) << "depth( " << cellName << " ) = " << depths[cell] << endl;
    }
}

DependencyManager::DependencyManager( const Map* map )
    : d(new Private)
{
    d->map = map;
}

DependencyManager::~DependencyManager ()
{
    qDeleteAll(d->consumers.values());
    delete d;
}

void DependencyManager::reset ()
{
    d->reset();
}

void DependencyManager::regionChanged(const Region& region)
{
    if (region.isEmpty())
        return;
    kDebug(36002) << "DependencyManager::regionChanged " << region.name() << endl;
    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        const QRect range = (*it)->rect();
        const Sheet* sheet = (*it)->sheet();
        for (int col = range.left(); col <= range.right(); ++col)
        {
            for (int row = range.top(); row <= range.bottom(); ++row)
            {
                const Cell cell( sheet, col, row );
                const Formula formula = cell.formula();

                // remove it from the reference depth list
                d->depths.remove( cell );

                // cell without a formula? remove it
                if ( formula.expression().isEmpty() )
                {
                    d->removeDependencies(cell);
                    continue;
                }

                d->generateDependencies(cell, formula);
            }
        }
    }
    {
        ElapsedTime et( "Computing reference depths", ElapsedTime::PrintOnlyTime );
        d->generateDepths(region);
    }
//     d->dump();
}

void DependencyManager::areaModified (const QString &name)
{
    d->areaModified (name);
}

void DependencyManager::updateAllDependencies(const Map* map)
{
    ElapsedTime et( "Generating dependencies", ElapsedTime::PrintOnlyTime );

    // clear the reference depth list
    d->depths.clear();

    Cell cell;
    foreach (const Sheet* sheet, map->sheetList())
    {
        for ( int c = 0; c < sheet->formulaStorage()->count(); ++c )
        {
            cell = Cell( sheet, sheet->formulaStorage()->col( c ), sheet->formulaStorage()->row( c ) );

            // empty or default cell or cell without a formula? remove it
            if ( sheet->formulaStorage()->data( c ).expression().isEmpty() )
            {
                d->removeDependencies( cell );
                continue;
            }

            d->generateDependencies( cell, sheet->formulaStorage()->data( c ) );
            if (!d->depths.contains( cell ))
            {
                int depth = d->computeDepth( cell );
                d->depths.insert( cell , depth);
            }
        }
    }
}

QHash<Cell, int> DependencyManager::depths() const
{
    return d->depths;
}

KSpread::Region DependencyManager::consumingRegion(const Cell& cell) const
{
    return d->consumingRegion(cell);
}

KSpread::Region DependencyManager::reduceToProvidingRegion(const Region& region) const
{
    Region providingRegion;
    QList< QPair<QRectF, Cell> > pairs;
    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        if (!d->consumers.contains((*it)->sheet()))
            continue;
        pairs = d->consumers.value((*it)->sheet())->intersectingPairs((*it)->rect());
        for (int i = 0; i < pairs.count(); ++i)
            providingRegion.add(pairs[i].first.toRect() & (*it)->rect(), (*it)->sheet());
    }
    return providingRegion;
}

void DependencyManager::regionMoved( const Region& movedRegion, const Cell& destination )
{
    Region::Point locationOffset( destination.cellPosition() - movedRegion.boundingRect().topLeft() );

    Region::ConstIterator end( movedRegion.constEnd() );
    for ( Region::ConstIterator it( movedRegion.constBegin() ); it != end; ++it )
    {
        Sheet* const sheet = (*it)->sheet();
        locationOffset.setSheet( ( sheet == destination.sheet() ) ? 0 : destination.sheet() );
        const QRect range = (*it)->rect();

        if ( d->consumers.contains( sheet ) )
        {
            const QRectF rangeF = QRectF( range ).adjusted( 0, 0, -0.1, -0.1 );
            QList<Cell> dependentLocations = d->consumers[sheet]->intersects( rangeF );

            for ( int i = 0; i < dependentLocations.count(); ++i )
            {
                const Cell cell = dependentLocations[i];
                updateFormula( cell, (*it), locationOffset );
            }
        }
    }
}

void DependencyManager::updateFormula( const Cell& cell, const Region::Element* oldLocation, const Region::Point& offset )
{
    // Not a formula -> no dependencies
    if (!cell.isFormula())
        return;

    const Formula formula = cell.formula();

    // Broken formula -> meaningless dependencies
    if ( !formula.isValid() )
        return;

    Tokens tokens = formula.tokens();

    //return empty list if the tokens aren't valid
    if (!tokens.valid())
        return;

    QString expression = "=";
    Sheet* sheet = cell.sheet();
    Region region;
    for( int i = 0; i < tokens.count(); i++ )
    {
        Token token = tokens[i];
        Token::Type tokenType = token.type();

        //parse each cell/range and put it to our expression
        if (tokenType == Token::Cell || tokenType == Token::Range)
        {
            const Region region( sheet->map(), token.text(), sheet );
            const Region::Element* element = *region.constBegin();

            kDebug(36002) << region.name() << endl;
            // the offset contains a sheet, only if it was an intersheet move.
            if ( ( oldLocation->sheet() == element->sheet() ) &&
                   ( oldLocation->rect().contains( element->rect() ) ) )
            {
                const Region yetAnotherRegion( element->rect().translated( offset.pos() ), offset.sheet() ? offset.sheet() : sheet );
                expression.append( yetAnotherRegion.name( sheet ) );
            }
            else
            {
                expression.append( token.text() );
            }
        }
        else
        {
            expression.append( token.text() );
        }
    }
    Cell( cell ).setCellText( expression );
}

void DependencyManager::Private::reset()
{
    providers.clear();
    consumers.clear();
}

KSpread::Region DependencyManager::Private::consumingRegion(const Cell& cell) const
{
    if (!consumers.contains(cell.sheet()))
    {
        kDebug(36002) << "No consumer tree found for the cell's sheet." << endl;
        return Region();
    }

    const QList<Cell> providers = consumers.value(cell.sheet())->contains(cell.cellPosition());

    Region region;
    foreach ( const Cell& cell, providers )
        region.add( cell.cellPosition(), cell.sheet() );
    return region;
}

void DependencyManager::Private::areaModified (const QString &name)
{
    // since area names are something like aliases, modifying an area name
    // basically means that all cells referencing this area should be treated
    // as modified - that will retrieve updated area ranges and also update
    // everything as necessary ...
    if (!areaDeps.contains (name))
        return;

    QHash<Cell, bool>::iterator it;
    for (it = areaDeps[name].begin(); it != areaDeps[name].end(); ++it)
    {
        Cell cell = it.key();
        // this forces the cell to regenerate everything - new range dependencies
        // and so on
        cell.setValue (cell.value ());
    }
}

void DependencyManager::Private::addDependencies(const Cell& cell, const Region& region)
{
    // NOTE Stefan: Also store cells without dependencies to avoid an
    //              iteration over all cells in a map/sheet on recalculation.

    // empty region will be created automatically, if necessary
    providers[cell].add(region);

    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        Sheet* sheet = (*it)->sheet();
        QRectF range = QRectF((*it)->rect()).adjusted(0, 0, -0.1, -0.1);

        if ( !consumers.contains( sheet ) ) consumers.insert( sheet, new RTree<Cell>() );
        consumers[sheet]->insert( range, cell );
    }
}

void DependencyManager::Private::removeDependencies(const Cell& cell)
{
    // look if the cell has any providers
    if ( !providers.contains( cell ) )
        return;  //it doesn't - nothing more to do

    // first this cell is no longer a provider for all providers
    Region region = providers[cell];
    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        Sheet* const sheet = (*it)->sheet();
        const QRect range = (*it)->rect();

        if (consumers.contains(sheet))
        {
            consumers[sheet]->remove( range, cell );
        }
    }

    // remove information about named area dependencies
    QHash<QString, QHash<Cell, bool> >::iterator itr;
    for (itr = areaDeps.begin(); itr != areaDeps.end(); ++itr) {
        if ( itr.value().contains( cell ) )
            itr.value().remove( cell );
    }

    // clear the circular dependency flags
    removeCircularDependencyFlags( providers.value( cell ), Backward );
    removeCircularDependencyFlags( consumingRegion( cell ), Forward );

    // finally, remove the entry about this cell
    providers.remove( cell );
}

void DependencyManager::Private::generateDependencies(const Cell& cell, const Formula& formula)
{
    //new dependencies only need to be generated if the cell contains a formula
//     if (cell.isNull())
//         return;
//     if (!cell.isFormula())
//         return;

    //get rid of old dependencies first
    removeDependencies(cell);

    //now we need to generate dependencies
    Region region = computeDependencies(cell, formula);

    //now that we have the new dependencies, we put them into our data structures
    //and we're done
    addDependencies(cell, region);
}

void DependencyManager::Private::generateDepths(const Region& region)
{
    static QSet<Cell> processedCells;

    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        const QRect range = (*it)->rect();
        const Sheet* sheet = (*it)->sheet();
        const int bottom = range.bottom();
        for (int row = range.top(); row <= bottom; ++row)
        {
            int col = 0;
            Formula formula = sheet->formulaStorage()->firstInRow( row, &col );
            if ( col > 0 && col < range.left() )
                formula = sheet->formulaStorage()->nextInRow( col, row, &col );
            else if ( col > range.right() )
                col = 0;
            while ( col != 0 )
            {
                Cell cell( sheet,col, row);

                //prevent infinite recursion (circular dependencies)
                if ( processedCells.contains( cell ) || cell.value() == Value::errorCIRCLE() )
                {
                    kDebug(36002) << "Circular dependency at " << cell.fullName() << endl;
                    cell.setValue( Value::errorCIRCLE() );
                    depths.insert(cell, 0);
                    // clear the compute reference depth flag
                    processedCells.remove( cell );
                    continue;
                }

                // set the compute reference depth flag
                processedCells.insert( cell );

                int depth = computeDepth(cell);
                depths.insert(cell, depth);

                // Recursion. We need the whole dependency tree of the changed region.
                // An infinite loop is prevented by the check above.
                Region dependentRegion = consumingRegion(cell);
                if (!dependentRegion.contains(QPoint(col, row), cell.sheet()))
                    generateDepths(dependentRegion);

                // clear the compute reference depth flag
                processedCells.remove( cell );

                formula = sheet->formulaStorage()->nextInRow( col, row, &col );
            }
        }
    }
}

int DependencyManager::Private::computeDepth(Cell cell) const
{
    // a set of cell, which depth is currently calculated
    static QSet<Cell> processedCells;

    //prevent infinite recursion (circular dependencies)
    if ( processedCells.contains( cell ) || cell.value() == Value::errorCIRCLE() )
    {
        kDebug(36002) << "Circular dependency at " << cell.fullName() << endl;
        cell.setValue( Value::errorCIRCLE() );
        //clear the compute reference depth flag
        processedCells.remove( cell );
        return 0;
    }

    // set the compute reference depth flag
    processedCells.insert( cell );

    int depth = 0;

    const Region region = providers.value( cell );

    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        const QRect range = (*it)->rect();
        Sheet* sheet = (*it)->sheet();
        const int right = range.right();
        const int bottom = range.bottom();
        for (int col = range.left(); col <= right; ++col)
        {
            for (int row = range.top(); row <= bottom; ++row)
            {
                Cell referencedCell = Cell( sheet, col, row );
                if ( !providers.contains( referencedCell ) )
                {
                    // no further references
                    // depth is one at least
                    depth = qMax(depth, 1);
                    continue;
                }

                if (depths.contains(referencedCell))
                {
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
    processedCells.remove( cell );

    return depth;
}

KSpread::Region DependencyManager::Private::computeDependencies( const Cell& cell, const Formula& formula ) const
{
    // Broken formula -> meaningless dependencies
    if ( !formula.isValid() )
        return Region();

    Tokens tokens = formula.tokens();

    //return empty list if the tokens aren't valid
    if (!tokens.valid())
        return Region();

    Sheet* sheet = cell.sheet();
    Region region;
    for( int i = 0; i < tokens.count(); i++ )
    {
        Token token = tokens[i];
        Token::Type tokenType = token.type();

        //parse each cell/range and put it to our Region
        if (tokenType == Token::Cell || tokenType == Token::Range)
        {
            Region subRegion(sheet->map(), token.text(), sheet);
            if (subRegion.isValid())
                region.add(subRegion);
        }
    }
    return region;
}

void DependencyManager::Private::removeCircularDependencyFlags( const Region& region, Direction direction )
{
    // a set of cells, which circular dependency flag is currently removed
    static QSet<Cell> processedCells;

    Region::ConstIterator end(region.constEnd());
    for (Region::ConstIterator it(region.constBegin()); it != end; ++it)
    {
        const QRect range = (*it)->rect();
        const Sheet* sheet = (*it)->sheet();
        for (int col = range.left(); col <= range.right(); ++col)
        {
            for (int row = range.top(); row <= range.bottom(); ++row)
            {
                Cell cell( sheet,col, row);

                if ( processedCells.contains( cell ) )
                    continue;
                processedCells.insert( cell );

                if ( cell.value() == Value::errorCIRCLE() )
                    cell.setValue( Value::empty() );

                if ( direction == Backward )
                    removeCircularDependencyFlags( providers.value( cell ), Backward );
                else // Forward
                    removeCircularDependencyFlags( consumingRegion( cell ), Forward );

                processedCells.remove( cell );
            }
        }
    }
}
