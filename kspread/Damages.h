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

#ifndef KSPREAD_DAMAGES
#define KSPREAD_DAMAGES


#include "kdebug.h"

#include "kspread_export.h"

class KCCell;
class KCMap;
class KCSheet;
class KCRegion;

/**
 * \ingroup Damages
 * An abstract damage.
 */
class KSPREAD_EXPORT KCDamage
{
public:
    virtual ~KCDamage() {}

    typedef enum {
        NoDamage = 0,
        DamagedDocument,
        DamagedWorkbook,
        DamagedSheet,
        DamagedRange,
        DamagedCell,
        DamagedSelection
    } Type;

    virtual Type type() const {
        return NoDamage;
    }
};

/**
 * \ingroup Damages
 * A cell range damage.
 */
class KSPREAD_EXPORT KCCellDamage : public KCDamage
{
public:
    enum Change {
        KCBinding    = 0x02, ///< on value changes; always triggered; for binding updates
        KCFormula    = 0x04, ///< triggers a dependency update
        NamedArea  = 0x10, ///< triggers a named area update
        /// This indicates a value change. It is not triggered while a recalculation is in progress.
        /// KCRecalcManager takes over in this case. Otherwise, circular dependencies would cause
        /// infinite loops and the cells would be recalculated in arbitrary order.
        KCValue      = 0x20,
        /// On style changes; invalidates the style storage cache.
        StyleCache  = 0x40,
        /// The visual cache gets damaged, if any of CellView's data members is
        /// affected. E.g. the displayed text, the cell dimension or the merging.
        VisualCache = 0x80,
        // TODO Stefan: Detach the style cache from the CellView cache.
        /// Updates the caches and triggers a repaint of the cell region.
        Appearance = StyleCache | VisualCache
    };
    Q_DECLARE_FLAGS(Changes, Change)

    KCCellDamage(const KCCell& cell, Changes changes);
    KCCellDamage(KCSheet* sheet, const KCRegion& region, Changes changes);

    virtual ~KCCellDamage();

    virtual Type type() const {
        return KCDamage::DamagedCell;
    }

    KCSheet* sheet() const;
    const KCRegion& region() const;

    Changes changes() const;

private:
    Q_DISABLE_COPY(KCCellDamage)

    class Private;
    Private * const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KCCellDamage::Changes)


/**
 * \ingroup Damages
 * A sheet damage.
 */
class KSPREAD_EXPORT KCSheetDamage : public KCDamage
{
public:

    enum Change {
        None              = 0x00,
        ContentChanged    = 0x01,
        PropertiesChanged = 0x02,
        Hidden            = 0x04,
        Shown             = 0x08,
        Name              = 0x10,
        ColumnsChanged    = 0x20,
        RowsChanged       = 0x40
    };
    Q_DECLARE_FLAGS(Changes, Change)

    KCSheetDamage(KCSheet* sheet, Changes changes);

    virtual ~KCSheetDamage();

    virtual Type type() const {
        return KCDamage::DamagedSheet;
    }

    KCSheet* sheet() const;

    Changes changes() const;

private:
    Q_DISABLE_COPY(KCSheetDamage)

    class Private;
    Private * const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KCSheetDamage::Changes)


/**
 * \ingroup Damages
 * A workbook damage.
 */
class KCWorkbookDamage : public KCDamage
{
public:
    enum Change {
        None       = 0x00,
        KCFormula    = 0x01,
        KCValue      = 0x02
    };
    Q_DECLARE_FLAGS(Changes, Change)

    KCWorkbookDamage(KCMap* map, Changes changes);
    virtual ~KCWorkbookDamage();

    virtual Type type() const {
        return KCDamage::DamagedWorkbook;
    }
    KCMap* map() const;
    Changes changes() const;

private:
    Q_DISABLE_COPY(KCWorkbookDamage)

    class Private;
    Private * const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KCWorkbookDamage::Changes)


/**
 * \ingroup Damages
 * A selection damage.
 */
class KSPREAD_EXPORT SelectionDamage : public KCDamage
{
public:
    SelectionDamage(const KCRegion& region);
    virtual ~SelectionDamage();

    virtual Type type() const {
        return KCDamage::DamagedSelection;
    }

    const KCRegion& region() const;

private:
    Q_DISABLE_COPY(SelectionDamage)

    class Private;
    Private * const d;
};


/***************************************************************************
  kDebug support
****************************************************************************/

KSPREAD_EXPORT QDebug operator<<(QDebug str, const KCDamage& d);
KSPREAD_EXPORT QDebug operator<<(QDebug str, const KCCellDamage& d);
KSPREAD_EXPORT QDebug operator<<(QDebug str, const KCSheetDamage& d);
KSPREAD_EXPORT QDebug operator<<(QDebug str, const SelectionDamage& d);

#endif // KSPREAD_DAMAGES
