/* This file is part of the KDE project
   Copyright 2009 Johannes Simon <johannes.simon@gmail.com>

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

// Ours
#include "KCSheetAccessModel.h"
#include "kcells_limits.h"
#include "KCMap.h"
#include "KCBinding.h"
#include "KCBindingManager.h"
#include "Damages.h"
#include "KCRegion.h"

// Qt
#include <QList>
#include <QMap>
#include <QStandardItem>
#include <QAbstractItemModel>
#include <QVariant>

// KOffice
//#include <KOdfStore.h>
//#include <KXmlWriter.h>
//#include <KoShapeSavingContext.h>

Q_DECLARE_METATYPE(QPointer<QAbstractItemModel>)


class KCSheetAccessModel::Private
{
public:
    KCMap *map;
    /// Stores in what column each KCSheet is. We need this because
    /// a KCSheet is removed from its KCMap before the sheetRemoved() signal
    /// is emitted, thus we can't ask the KCMap what index it had.
    QMap<KCSheet*, int> cols;
};

KCSheetAccessModel::KCSheetAccessModel(KCMap *map)
        : d(new Private)
{
    d->map = map;

    connect(map, SIGNAL(sheetAdded(KCSheet*)),
            this, SLOT(slotSheetAdded(KCSheet*)));
    // FIXME: Check if we can simply connect sheetRevived() to slotSheetAdded()
    connect(map, SIGNAL(sheetRevived(KCSheet*)),
            this, SLOT(slotSheetAdded(KCSheet*)));
    connect(map, SIGNAL(sheetRemoved(KCSheet*)),
            this, SLOT(slotSheetRemoved(KCSheet*)));
    connect(map, SIGNAL(damagesFlushed(const QList<KCDamage*>&)),
            this, SLOT(handleDamages(const QList<KCDamage*>&)));

    setRowCount(1);
    setColumnCount(0);
}

KCSheetAccessModel::~KCSheetAccessModel()
{
    delete d;
}

void KCSheetAccessModel::slotSheetAdded(KCSheet *sheet)
{
    Q_ASSERT(!d->cols.contains(sheet));

    QStandardItem *item = new QStandardItem;
    QList<QStandardItem*> col;
    col.append(item);

    // This region contains the entire sheet
    const KCRegion region(1, 1, KS_colMax, KS_rowMax, sheet);
    const QPointer<QAbstractItemModel> model = const_cast<QAbstractItemModel*>( d->map->bindingManager()->createModel( region.name() ) );

    item->setData( QVariant::fromValue( model ), Qt::DisplayRole );

    const int sheetIndex = d->map->indexOf( sheet );
    d->cols.insert(sheet, sheetIndex);

    insertColumn( sheetIndex, col );
    setHeaderData( sheetIndex, Qt::Horizontal, sheet->sheetName() );
}

void KCSheetAccessModel::slotSheetRemoved(KCSheet *sheet)
{
    Q_ASSERT(d->cols.contains(sheet));
    removeColumn(d->cols[sheet]);
    d->cols.remove(sheet);
}

void KCSheetAccessModel::handleDamages(const QList<KCDamage*>& damages)
{
    QList<KCDamage*>::ConstIterator end(damages.end());
    for (QList<KCDamage*>::ConstIterator it = damages.begin(); it != end; ++it) {
        KCDamage* damage = *it;
        if (!damage) {
            continue;
        }

        if (damage->type() == KCDamage::DamagedSheet) {
            KCSheetDamage* sheetDamage = static_cast<KCSheetDamage*>(damage);
            kDebug(36007) << "Processing\t" << *sheetDamage;

            if (sheetDamage->changes() & KCSheetDamage::Name) {
                KCSheet *sheet = sheetDamage->sheet();
                // We should never receive signals from sheets that are not in our model
                Q_ASSERT(d->cols.contains(sheet));
                const int sheetIndex = d->cols[sheet];
                setHeaderData(sheetIndex, Qt::Horizontal, sheet->sheetName());
            }
            continue;
        }
    }
}
