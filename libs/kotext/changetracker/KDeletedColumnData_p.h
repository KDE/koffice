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
#ifndef KDELETEDCOLUMNDATA_H
#define KDELETEDCOLUMNDATA_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KoText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QVector>
#include <styles/KTableColumnStyle.h>

class KDeletedCellData;
class QTextTable;

class KDeletedColumnData
{
public:
    KDeletedColumnData(QTextTable *table, int columnNumber);

    ~KDeletedColumnData();

    int columnNumber();

    KTableColumnStyle columnStyle();

    const QVector<KDeletedCellData *>& deletedCells();

private:
    int column_number;

    KTableColumnStyle column_style;

    QVector<KDeletedCellData *> deleted_cells;

    void storeDeletedCells(QTextTable *table);

    void setColumnStyle(KTableColumnStyle columnStyle);
};

#endif
