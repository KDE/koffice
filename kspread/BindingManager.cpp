/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "BindingManager.h"

#include "BindingStorage.h"
#include "CellStorage.h"
#include "Map.h"
#include "KCRegion.h"
#include "KCSheet.h"

#include <QAbstractItemModel>

class BindingManager::Private
{
public:
    const Map* map;
};

BindingManager::BindingManager(const Map* map)
        : d(new Private)
{
    d->map = map;
}

BindingManager::~BindingManager()
{
    delete d;
}

const QAbstractItemModel* BindingManager::createModel(const QString& regionName)
{
    const KCRegion region(regionName, d->map);
    if (!region.isValid() || !region.isContiguous() || !region.firstSheet()) {
        return 0;
    }
    Binding binding(region);
    region.firstSheet()->cellStorage()->setBinding(region, binding);
    return binding.model();
}

bool BindingManager::removeModel(const QAbstractItemModel* model)
{
    QList< QPair<QRectF, Binding> > bindings;
    const QRect rect(QPoint(1, 1), QPoint(KS_colMax, KS_rowMax));
    const QList<KCSheet*> sheets = d->map->sheetList();
    for (int i = 0; i < sheets.count(); ++i) {
        KCSheet* const sheet = sheets[i];
        bindings = sheet->cellStorage()->bindingStorage()->intersectingPairs(KCRegion(rect, sheet));
        for (int j = 0; j < bindings.count(); ++j) {
            if (bindings[j].second.model() == model) {
                const KCRegion region(bindings[j].first.toRect(), sheet);
                sheet->cellStorage()->removeBinding(region, bindings[j].second);
                return true;
            }
        }
    }
    return false;
}

bool BindingManager::isCellRegionValid(const QString& regionName) const
{
    const KCRegion region(regionName, d->map);
    return (region.isValid() && region.isContiguous() && region.firstSheet());
}

void BindingManager::regionChanged(const KCRegion& region)
{
    KCSheet* sheet;
    QList< QPair<QRectF, Binding> > bindings;
    KCRegion::ConstIterator end(region.constEnd());
    for (KCRegion::ConstIterator it = region.constBegin(); it != end; ++it) {
        sheet = (*it)->sheet();
        const KCRegion changedRegion((*it)->rect(), sheet);
        bindings = sheet->cellStorage()->bindingStorage()->intersectingPairs(changedRegion);
        for (int j = 0; j < bindings.count(); ++j)
            bindings[j].second.update(changedRegion);
    }
}

void BindingManager::updateAllBindings()
{
    QList< QPair<QRectF, Binding> > bindings;
    const QRect rect(QPoint(1, 1), QPoint(KS_colMax, KS_rowMax));
    const QList<KCSheet*> sheets = d->map->sheetList();
    for (int i = 0; i < sheets.count(); ++i) {
        bindings = sheets[i]->cellStorage()->bindingStorage()->intersectingPairs(KCRegion(rect, sheets[i]));
        for (int j = 0; j < bindings.count(); ++j)
            bindings[j].second.update(KCRegion(bindings[j].first.toRect(), sheets[i]));
    }
}

#include "BindingManager.moc"
