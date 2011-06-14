/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KDeletedRowColumnDataStore_p.h"
#include "KDeletedRowData_p.h"
#include "KDeletedColumnData_p.h"

KDeletedRowColumnDataStore::KDeletedRowColumnDataStore()
{
}

KDeletedRowColumnDataStore::~KDeletedRowColumnDataStore()
{
}

KDeletedRowData *KDeletedRowColumnDataStore::addDeletedRow(QTextTable *table, int rowNumber, int changeId)
{
    KDeletedRowData *deletedRowData = new KDeletedRowData(table, rowNumber);
    deletedRowDataMap.insert(changeId, deletedRowData);
    QVector<int> *tableChangeIds = tableChangeIdsMap.value(table, NULL);
    if (!tableChangeIds) {
        tableChangeIds = new QVector<int>();
        tableChangeIdsMap.insert(table, tableChangeIds);
    }
    tableChangeIds->push_back(changeId);
    return deletedRowData;
}

KDeletedColumnData *KDeletedRowColumnDataStore::addDeletedColumn(QTextTable *table, int columnNumber, int changeId)
{
    KDeletedColumnData *deletedColumnData = new KDeletedColumnData(table, columnNumber);
    deletedColumnDataMap.insert(changeId, deletedColumnData);
    QVector<int> *tableChangeIds = tableChangeIdsMap.value(table, NULL);
    if (!tableChangeIds) {
        tableChangeIds = new QVector<int>();
        tableChangeIdsMap.insert(table, tableChangeIds);
    }
    tableChangeIds->push_back(changeId);
    return deletedColumnData;
}

const QVector<int> *KDeletedRowColumnDataStore::deletedRowColumnChangeIds(QTextTable *table)
{
    return tableChangeIdsMap.value(table, NULL);
}

KDeletedRowColumnDataStore::DeleteType KDeletedRowColumnDataStore::deleteType(int changeId)
{
    KDeletedRowColumnDataStore::DeleteType retValue;
    if (deletedRowDataMap.value(changeId, NULL)) {
        retValue = KDeletedRowColumnDataStore::eDeletedRow;
    } else if(deletedColumnDataMap.value(changeId, NULL)) {
        retValue = KDeletedRowColumnDataStore::eDeletedColumn;
    } else {
        retValue = KDeletedRowColumnDataStore::eUnknownDeleteType;
    }

    return retValue;
}

KDeletedRowData *KDeletedRowColumnDataStore::deletedRowData(int changeId)
{
    return deletedRowDataMap.value(changeId, NULL);
}

KDeletedColumnData *KDeletedRowColumnDataStore::deletedColumnData(int changeId)
{
    return deletedColumnDataMap.value(changeId, NULL);
}

