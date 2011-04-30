/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

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
#include "Damages.h"

#include <QPoint>

#include "Cell.h"
#include "KCSheet.h"
#include "KCRegion.h"

class WorkbookDamage::Private
{
public:
    Map* map;
    Changes changes;
};

class SheetDamage::Private
{
public:
    KCSheet* sheet;
    Changes changes;
};

class CellDamage::Private
{
public:
    KCSheet* sheet;
    KCRegion region;
    Changes changes;
};

class SelectionDamage::Private
{
public:
    KCRegion region;
};

CellDamage::CellDamage(const Cell& cell, Changes changes)
        : d(new Private)
{
    d->sheet = cell.sheet();
    if (KCRegion::isValid(QPoint(cell.column(), cell.row())))
        d->region = KCRegion(cell.column(), cell.row(), d->sheet);
    d->changes = changes;
}

CellDamage::CellDamage(KCSheet* sheet, const KCRegion& region, Changes changes)
        : d(new Private)
{
    d->sheet = sheet;
    d->region = region;
    d->changes = changes;
}

CellDamage::~CellDamage()
{
    delete d;
}

KCSheet* CellDamage::sheet() const
{
    return d->sheet;
}

const KCRegion& CellDamage::region() const
{
    return d->region;
}

CellDamage::Changes CellDamage::changes() const
{
    return d->changes;
}


SheetDamage::SheetDamage(KCSheet* sheet, Changes changes)
        : d(new Private)
{
    d->sheet = sheet;
    d->changes = changes;
}

SheetDamage::~SheetDamage()
{
    delete d;
}

KCSheet* SheetDamage::sheet() const
{
    return d->sheet;
}

SheetDamage::Changes SheetDamage::changes() const
{
    return d->changes;
}


WorkbookDamage::WorkbookDamage(Map* map, Changes changes)
        : d(new Private)
{
    d->map = map;
    d->changes = changes;
}

WorkbookDamage::~WorkbookDamage()
{
    delete d;
}

Map* WorkbookDamage::map() const
{
    return d->map;
}

WorkbookDamage::Changes WorkbookDamage::changes() const
{
    return d->changes;
}


SelectionDamage::SelectionDamage(const KCRegion& region)
        : d(new Private)
{
    d->region = region;
}

SelectionDamage::~SelectionDamage()
{
    delete d;
}

const KCRegion& SelectionDamage::region() const
{
    return d->region;
}


/***************************************************************************
  kDebug support
****************************************************************************/

QDebug operator<<(QDebug str, const Damage& d)
{
    switch (d.type()) {
    case Damage::NoDamage: return str << "NoDamage";
    case Damage::DamagedDocument:  return str << "Document";
    case Damage::DamagedWorkbook: return str << "Workbook";
    case Damage::DamagedSheet: return str << "KCSheet";
    case Damage::DamagedRange: return str << "Range";
    case Damage::DamagedCell: return str << "Cell";
    case Damage::DamagedSelection: return str << "Selection";
    }
    return str;
}

QDebug operator<<(QDebug str, const CellDamage& d)
{
    str << "CellDamage: " << d.region().name(d.sheet());
    if (d.changes() & CellDamage::Appearance) str << " Appearance";
    if (d.changes() & CellDamage::Binding)    str << " Binding";
    if (d.changes() & CellDamage::Formula)    str << " Formula";
    if (d.changes() & CellDamage::Value)      str << " Value";
    return str;
}

QDebug operator<<(QDebug str, const SheetDamage& d)
{
    str << "SheetDamage: " << (d.sheet() ? d.sheet()->sheetName() : "NULL POINTER!");
    switch (d.changes()) {
    case SheetDamage::None:               return str << " None";
    case SheetDamage::ContentChanged:     return str << " Content";
    case SheetDamage::PropertiesChanged:  return str << " Properties";
    case SheetDamage::Hidden:             return str << " Hidden";
    case SheetDamage::Shown:              return str << " Shown";
    case SheetDamage::Name:               return str << "Name";
    case SheetDamage::ColumnsChanged:     return str << "Columns";
    case SheetDamage::RowsChanged:        return str << "Rows";
    }
    return str;
}

QDebug operator<<(QDebug str, const SelectionDamage& d)
{
    str << "SelectionDamage: " << d.region().name();
    return str;
}
