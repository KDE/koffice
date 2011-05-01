/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus stefan.nikolaus@kdemail.net

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

#include "KCRegionModel.h"

// KSpread
#include "KCRegion.h"
#include "KCSheet.h"

class KCRegionModel::Private
{
public:
    KCSheet* sheet;
    QRect range;
    bool overwriteMode;
};


KCRegionModel::KCRegionModel(const KCRegion& region)
        : KCSheetModel(region.lastSheet())
        , d(new Private)
{
    Q_ASSERT(region.isContiguous());
    Q_ASSERT(!region.isEmpty());
    Q_ASSERT(region.lastSheet());
    d->sheet = region.lastSheet();
    d->range = region.lastRange();
    d->overwriteMode = true;
}

KCRegionModel::~KCRegionModel()
{
    delete d;
}

int KCRegionModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalPointer() != d->sheet->map()) {
        return false;
    }
    if (d->overwriteMode) {
        return KCSheetModel::columnCount(parent) - d->range.left() + 1;
    }
    return d->range.width();
}

QModelIndex KCRegionModel::index(int row, int column, const QModelIndex &parent) const
{
    return KCSheetModel::index(row + d->range.top() - 1, column + d->range.left() - 1, parent);
}

int KCRegionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalPointer() != d->sheet->map()) {
        return false;
    }
    if (d->overwriteMode) {
        return KCSheetModel::rowCount(parent) - d->range.top() + 1;
    }
    return d->range.height();
}
