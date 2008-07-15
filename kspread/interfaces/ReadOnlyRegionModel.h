/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus stefan.nikolaus@kdemail.net

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

#ifndef KSPREAD_READONLY_REGION_MODEL
#define KSPREAD_READONLY_REGION_MODEL

#include <QAbstractProxyModel>

namespace KSpread
{
class Region;

/**
 * A model for a contiguous cell region.
 */
class ReadOnlyRegionModel : QAbstractProxyModel
{
public:
    /**
     * Constructor.
     */
    ReadOnlyRegionModel(const Region& region);

    /**
     * Destructor.
     */
    ~ReadOnlyRegionModel();

    // QAbstractTableModel interface
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

    // QAbstractProxyModel interface
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

private:
    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREAD_READONLY_REGION_MODEL
