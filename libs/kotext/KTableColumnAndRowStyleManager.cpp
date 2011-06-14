/* This file is part of the KDE project
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "KTableColumnAndRowStyleManager.h"

#include "styles/KoTableColumnStyle.h"
#include "styles/KoTableRowStyle.h"
#include "styles/KTableCellStyle.h"
#include "styles/KoTableStyle.h"

#include <QVector>
#include <QSet>
#include <QVariant>
#include <QTextTable>

#include <QDebug>

class KTableColumnAndRowStyleManager::Private : public QSharedData
{
public:
    Private()  { }
    ~Private() {
    }
    QVector<KoTableColumnStyle> tableColumnStyles;
    QVector<KoTableRowStyle> tableRowStyles;

    QVector<KTableCellStyle*> defaultRowCellStyles;
    QVector<KTableCellStyle*> defaultColumnCellStyles;
};

KTableColumnAndRowStyleManager::KTableColumnAndRowStyleManager()
    : d(new Private())
{
}

KTableColumnAndRowStyleManager::KTableColumnAndRowStyleManager(const KTableColumnAndRowStyleManager &rhs)
    : d(rhs.d)
{
}

KTableColumnAndRowStyleManager &KTableColumnAndRowStyleManager::operator=(const KTableColumnAndRowStyleManager &rhs)
{
    d = rhs.d;

    return *this;
}

KTableColumnAndRowStyleManager::~KTableColumnAndRowStyleManager()
{
}

KTableColumnAndRowStyleManager KTableColumnAndRowStyleManager::manager(QTextTable *table)
{
    QTextTableFormat tableFormat = table->format();

    if (tableFormat.hasProperty(KoTableStyle::ColumnAndRowStyleManager)) {
        return  tableFormat.property(KoTableStyle::ColumnAndRowStyleManager).value<KTableColumnAndRowStyleManager>();
    } else {
        KTableColumnAndRowStyleManager carsManager;

        QVariant var;
        var.setValue(carsManager);
        tableFormat.setProperty(KoTableStyle::ColumnAndRowStyleManager, var);
        table->setFormat(tableFormat);
        return carsManager;
    }
}

void KTableColumnAndRowStyleManager::setColumnStyle(int column, const KoTableColumnStyle &columnStyle)
{
    Q_ASSERT(column >= 0);

    if (column < 0) {
        return;
    }

    if (column < d->tableColumnStyles.size() && d->tableColumnStyles.value(column) == columnStyle) {
        return;
    }

    while (column > d->tableColumnStyles.size())
        d->tableColumnStyles.append(KoTableColumnStyle());

    d->tableColumnStyles.insert(column, columnStyle);
}

void KTableColumnAndRowStyleManager::insertColumns(int column, int numberColumns, const KoTableColumnStyle &columnStyle)
{
    Q_ASSERT(column >= 0);
    Q_ASSERT(numberColumns >= 0);

    if (column < 0 || numberColumns < 0) {
        return;
    }

    while (column > d->tableColumnStyles.size())
        d->tableColumnStyles.append(KoTableColumnStyle());

    d->tableColumnStyles.insert(column, numberColumns, columnStyle);
}

void KTableColumnAndRowStyleManager::removeColumns(int column, int numberColumns)
{
    Q_ASSERT(column >= 0);
    Q_ASSERT(numberColumns >= 0);

    if (column >= d->tableColumnStyles.size() || column < 0 || numberColumns < 0) {
        return;
    }

    while (column > d->tableColumnStyles.size())
        d->tableColumnStyles.append(KoTableColumnStyle());

    d->tableColumnStyles.remove(column, numberColumns);
}

KoTableColumnStyle KTableColumnAndRowStyleManager::columnStyle(int column) const
{
    Q_ASSERT(column >= 0);

    if (column < 0) {
        return KoTableColumnStyle();
    }

    return d->tableColumnStyles.value(column);
}

void KTableColumnAndRowStyleManager::setRowStyle(int row, const KoTableRowStyle &rowStyle)
{
    Q_ASSERT(row >= 0);

    if (row < 0) {
        return;
    }

    if (row < d->tableRowStyles.size() && d->tableRowStyles.value(row) == rowStyle) {
        return;
    }

    while (row > d->tableRowStyles.size())
        d->tableRowStyles.append(KoTableRowStyle());

    d->tableRowStyles.insert(row, rowStyle);
}

void KTableColumnAndRowStyleManager::insertRows(int row, int numberRows, const KoTableRowStyle &rowStyle)
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(numberRows >= 0);

    if (row < 0 || numberRows < 0) {
        return;
    }

    while (row > d->tableRowStyles.size())
        d->tableRowStyles.append(KoTableRowStyle());

    d->tableRowStyles.insert(row, numberRows, rowStyle);
}

void KTableColumnAndRowStyleManager::removeRows(int row, int numberRows)
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(numberRows >= 0);

    if (row >= d->tableRowStyles.size() || row < 0 || numberRows < 0) {
        return;
    }

    while (row > d->tableRowStyles.size())
        d->tableRowStyles.append(KoTableRowStyle());

    d->tableRowStyles.remove(row, numberRows);
}

KoTableRowStyle KTableColumnAndRowStyleManager::rowStyle(int row) const
{
    Q_ASSERT(row >= 0);

    if (row < 0) {
        return KoTableRowStyle();
    }

    return d->tableRowStyles.value(row);
}

KTableCellStyle* KTableColumnAndRowStyleManager::defaultColumnCellStyle(int column) const
{
    Q_ASSERT(column >= 0);

    return d->defaultColumnCellStyles.value(column);
}

void KTableColumnAndRowStyleManager::setDefaultColumnCellStyle(int column, KTableCellStyle* cellStyle)
{
    Q_ASSERT(column >= 0);

    if (column < d->defaultColumnCellStyles.size() && d->defaultColumnCellStyles.value(column) == cellStyle) {
        return;
    }

    while (column > d->defaultColumnCellStyles.size())
        d->defaultColumnCellStyles.append(0);

    d->defaultColumnCellStyles.append(cellStyle);
}

KTableCellStyle* KTableColumnAndRowStyleManager::defaultRowCellStyle(int row) const
{
    Q_ASSERT(row >= 0);

    return d->defaultRowCellStyles.value(row);
}

void KTableColumnAndRowStyleManager::setDefaultRowCellStyle(int row, KTableCellStyle* cellStyle)
{
    Q_ASSERT(row >= 0);

    if (row < d->defaultRowCellStyles.size() && d->defaultRowCellStyles.value(row) == cellStyle) {
        return;
    }

    while (row > d->defaultRowCellStyles.size())
        d->defaultRowCellStyles.append(0);

    d->defaultRowCellStyles.append(cellStyle);
}
