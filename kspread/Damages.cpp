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

#include "KCCell.h"
#include "KCSheet.h"
#include "KCRegion.h"

class WorkbookDamage::Private
{
public:
    KCMap* map;
    Changes changes;
};

class KCSheetDamage::Private
{
public:
    KCSheet* sheet;
    Changes changes;
};

class KCCellDamage::Private
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

KCCellDamage::KCCellDamage(const KCCell& cell, Changes changes)
        : d(new Private)
{
    d->sheet = cell.sheet();
    if (KCRegion::isValid(QPoint(cell.column(), cell.row())))
        d->region = KCRegion(cell.column(), cell.row(), d->sheet);
    d->changes = changes;
}

KCCellDamage::KCCellDamage(KCSheet* sheet, const KCRegion& region, Changes changes)
        : d(new Private)
{
    d->sheet = sheet;
    d->region = region;
    d->changes = changes;
}

KCCellDamage::~KCCellDamage()
{
    delete d;
}

KCSheet* KCCellDamage::sheet() const
{
    return d->sheet;
}

const KCRegion& KCCellDamage::region() const
{
    return d->region;
}

KCCellDamage::Changes KCCellDamage::changes() const
{
    return d->changes;
}


KCSheetDamage::KCSheetDamage(KCSheet* sheet, Changes changes)
        : d(new Private)
{
    d->sheet = sheet;
    d->changes = changes;
}

KCSheetDamage::~KCSheetDamage()
{
    delete d;
}

KCSheet* KCSheetDamage::sheet() const
{
    return d->sheet;
}

KCSheetDamage::Changes KCSheetDamage::changes() const
{
    return d->changes;
}


WorkbookDamage::WorkbookDamage(KCMap* map, Changes changes)
        : d(new Private)
{
    d->map = map;
    d->changes = changes;
}

WorkbookDamage::~WorkbookDamage()
{
    delete d;
}

KCMap* WorkbookDamage::map() const
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

QDebug operator<<(QDebug str, const KCDamage& d)
{
    switch (d.type()) {
    case KCDamage::NoDamage: return str << "NoDamage";
    case KCDamage::DamagedDocument:  return str << "Document";
    case KCDamage::DamagedWorkbook: return str << "Workbook";
    case KCDamage::DamagedSheet: return str << "KCSheet";
    case KCDamage::DamagedRange: return str << "Range";
    case KCDamage::DamagedCell: return str << "KCCell";
    case KCDamage::DamagedSelection: return str << "Selection";
    }
    return str;
}

QDebug operator<<(QDebug str, const KCCellDamage& d)
{
    str << "KCCellDamage: " << d.region().name(d.sheet());
    if (d.changes() & KCCellDamage::Appearance) str << " Appearance";
    if (d.changes() & KCCellDamage::KCBinding)    str << " KCBinding";
    if (d.changes() & KCCellDamage::KCFormula)    str << " KCFormula";
    if (d.changes() & KCCellDamage::KCValue)      str << " KCValue";
    return str;
}

QDebug operator<<(QDebug str, const KCSheetDamage& d)
{
    str << "KCSheetDamage: " << (d.sheet() ? d.sheet()->sheetName() : "NULL POINTER!");
    switch (d.changes()) {
    case KCSheetDamage::None:               return str << " None";
    case KCSheetDamage::ContentChanged:     return str << " Content";
    case KCSheetDamage::PropertiesChanged:  return str << " Properties";
    case KCSheetDamage::Hidden:             return str << " Hidden";
    case KCSheetDamage::Shown:              return str << " Shown";
    case KCSheetDamage::Name:               return str << "Name";
    case KCSheetDamage::ColumnsChanged:     return str << "Columns";
    case KCSheetDamage::RowsChanged:        return str << "Rows";
    }
    return str;
}

QDebug operator<<(QDebug str, const SelectionDamage& d)
{
    str << "SelectionDamage: " << d.region().name();
    return str;
}
