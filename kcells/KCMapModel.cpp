/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "KCMapModel.h"

#include "KCMap.h"
#include "ModelSupport.h"
#include "KCSheet.h"
#include "KCSheetModel.h"

#include "commands/SheetCommands.h"

#include <KIcon>

class KCMapModel::Private
{
public:
    KCMap* map;

public:
    bool isSheetIndex(const QModelIndex& index, const KCMapModel* mapModel) const;
};

bool KCMapModel::Private::isSheetIndex(const QModelIndex& index, const KCMapModel* mapModel) const
{
    if (!index.parent().isValid()) {
        return false;
    }
    // If it is a cell, the parent's (the sheet's) model has to be this model.
    if (index.parent().model() != mapModel || index.parent().internalPointer() != map) {
        return false;
    }
    // If it is a cell, the parent (the sheet) has no parent.
    if (index.parent().parent().isValid()) {
        return false;
    }
    // Do not exceed the sheet list.
    if (index.parent().row() >= map->count()) {
        return false;
    }
    // The index' (the cell's) model has to match the sheet model.
    if (index.model() != map->sheet(index.parent().row())->model()) {
        return false;
    }
    return true;
}


KCMapModel::KCMapModel(KCMap* map)
        : QAbstractListModel(map)
        , d(new Private)
{
    d->map = map;
    connect(d->map, SIGNAL(sheetAdded(KCSheet *)),
            this, SLOT(addSheet(KCSheet *)));
    connect(d->map, SIGNAL(sheetRemoved(KCSheet *)),
            this, SLOT(removeSheet(KCSheet *)));
}

KCMapModel::~KCMapModel()
{
    delete d;
}

QVariant KCMapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    // Propagation to sheet model
    if (d->isSheetIndex(index, this)) {
        return d->map->sheet(index.parent().row())->model()->data(index, role);
    }
    if (index.row() >= d->map->count()) {
        return QVariant();
    }
    //
    const KCSheet* const sheet = d->map->sheet(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return QVariant(sheet->sheetName());
    case Qt::DecorationRole:
        return QVariant(KIcon("x-office-spreadsheet"));
    case VisibilityRole:
        return QVariant(!sheet->isHidden());
    case ProtectionRole:
        return QVariant(sheet->isProtected());
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags KCMapModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    // Propagation to sheet model
    if (d->isSheetIndex(index, this)) {
        return d->map->sheet(index.parent().row())->model()->flags(index);
    }
    if (index.row() >= d->map->count()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (!d->map->isProtected()) {
        flags |= Qt::ItemIsSelectable;
        const KCSheet* const sheet = d->map->sheet(index.row());
        if (!sheet->isProtected()) {
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}

QVariant KCMapModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    if (section == 0 && role == Qt::DisplayRole) {
        return QVariant(i18n("Sheet name"));
    }
    return QVariant();
}

QModelIndex KCMapModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (parent.isValid()) {
        // If it is a cell, the parent's (the sheet's) model has to be this model.
        if (parent.model() != this || parent.internalPointer() != d->map) {
            return QModelIndex();
        }
        // If it is a cell, the parent (the sheet) has no parent.
        if (parent.parent().isValid()) {
            return QModelIndex();
        }
        // Do not exceed the sheet list.
        if (parent.row() >= d->map->count()) {
            return QModelIndex();
        }
        KCSheet* const sheet = d->map->sheet(index.parent().row());
        index = sheet->model()->index(row, column, parent);
    } else {
        index = createIndex(row, column, d->map);
    }
    return index;
}

int KCMapModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->map->count();
}

bool KCMapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // Propagation to sheet model
    if (d->isSheetIndex(index, this)) {
        return d->map->sheet(index.parent().row())->model()->setData(index, value, role);
    }

    if (index.isValid() && index.row() < d->map->count()) {
        KCSheet* const sheet(d->map->sheet(index.row()));
        switch (role) {
        case Qt::EditRole: {
            const QString name(value.toString());
            if (!name.isEmpty()) {
                QUndoCommand* const command = new RenameSheetCommand(sheet, name);
                emit addCommandRequested(command);
                emit dataChanged(index, index);
                return true;
            }
            break;
        }
        case VisibilityRole:
            setHidden(sheet, value.toBool());
            break;
        case ProtectionRole:
            break;
        default:
            break;
        }
    }
    return false;
}

bool KCMapModel::setHidden(KCSheet* sheet, bool hidden)
{
    QUndoCommand* command;
    if (hidden && !sheet->isHidden()) {
        command = new HideSheetCommand(sheet);
    } else if (!hidden && sheet->isHidden()) {
        command = new ShowSheetCommand(sheet);
    } else {
        return false; // nothing to do
    }
    emit addCommandRequested(command);
    return true;
}

KCMap* KCMapModel::map() const
{
    return d->map;
}

void KCMapModel::addSheet(KCSheet* sheet)
{
    kDebug() << "Added sheet:" << sheet->sheetName();
    emit layoutChanged();
}

void KCMapModel::removeSheet(KCSheet *sheet)
{
    kDebug() << "Removed sheet:" << sheet->sheetName();
    emit layoutChanged();
}

#include "KCMapModel.moc"
