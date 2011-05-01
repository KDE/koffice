/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KSpread Team <koffice-devel@kde.org>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>

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

// built-in reference functions
#include "ReferenceModule.h"

#include "KCCell.h"
#include "KCRegion.h"
#include "KCSheet.h"
#include "Util.h"
#include "KCValue.h"

#include "CellStorage.h"
#include "Formula.h"
#include "Function.h"
#include "FunctionModuleRegistry.h"
#include "ValueCalc.h"
#include "ValueConverter.h"

#include <KLocale>

using namespace KSpread;

// prototypes (sorted alphabetically)
KCValue func_address(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_areas(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_choose(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_column(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_columns(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_hlookup(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_index(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_indirect(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_lookup(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_match(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_multiple_operations(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_row(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_rows(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_vlookup(valVector args, ValueCalc *calc, FuncExtra *);


KSPREAD_EXPORT_FUNCTION_MODULE("reference", ReferenceModule)


ReferenceModule::ReferenceModule(QObject* parent, const QVariantList&)
        : FunctionModule(parent)
{
    Function *f;

    f = new Function("ADDRESS",  func_address);
    f->setParamCount(2, 5);
    add(f);
    f = new Function("AREAS",    func_areas);
    f->setParamCount(1);
    f->setNeedsExtra(true);
    f->setAcceptArray();
    add(f);
    f = new Function("CHOOSE",   func_choose);
    f->setParamCount(2, -1);
    f->setAcceptArray();
    add(f);
    f = new Function("COLUMN",   func_column);
    f->setParamCount(0, 1);
    add(f);
    f = new Function("COLUMNS",  func_columns);
    f->setParamCount(1);
    f->setAcceptArray();
    f->setNeedsExtra(true);
    add(f);
    f = new Function("HLOOKUP",  func_hlookup);
    f->setParamCount(3, 4);
    f->setAcceptArray();
    add(f);
    f = new Function("INDEX",   func_index);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new Function("INDIRECT", func_indirect);
    f->setParamCount(1, 2);
    f->setNeedsExtra(true);
    add(f);
    f = new Function("LOOKUP",   func_lookup);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new Function("MATCH", func_match);
    f->setParamCount(2, 3);
    f->setAcceptArray();
    f->setNeedsExtra(true);
  add(f);
    f = new Function("MULTIPLE.OPERATIONS", func_multiple_operations);
    f->setParamCount(3, 5);
    f->setNeedsExtra(true);
    add(f);
    f = new Function("ROW",      func_row);
    f->setParamCount(0, 1);
    add(f);
    f = new Function("ROWS",     func_rows);
    f->setParamCount(1);
    f->setAcceptArray();
    f->setNeedsExtra(true);
    add(f);
    f = new Function("VLOOKUP",  func_vlookup);
    f->setParamCount(3, 4);
    f->setAcceptArray();
    add(f);
}

QString ReferenceModule::descriptionFileName() const
{
    return QString("reference.xml");
}


//
// Function: ADDRESS
//
KCValue func_address(valVector args, ValueCalc *calc, FuncExtra *)
{
    bool r1c1 = false;
    QString sheetName;
    int absNum = 1;
    if (args.count() > 2)
        absNum = calc->conv()->asInteger(args[2]).asInteger();
    if (args.count() > 3)
        r1c1 = !(calc->conv()->asBoolean(args[3]).asBoolean());
    if (args.count() == 5)
        sheetName = calc->conv()->asString(args[4]).asString();

    QString result;
    int row = calc->conv()->asInteger(args[0]).asInteger();
    int col = calc->conv()->asInteger(args[1]).asInteger();

    if (!sheetName.isEmpty()) {
        result += sheetName;
        result += '!';
    }

    if (r1c1) {
        // row first
        bool abs = false;
        if (absNum == 1 || absNum == 2)
            abs = true;

        result += 'R';
        if (!abs)
            result += '[';
        result += QString::number(row);

        if (!abs)
            result += ']';

        // column
        abs = false;
        if (absNum == 1 || absNum == 3)
            abs = true;

        result += 'C';
        if (!abs)
            result += '[';
        result += QString::number(col);

        if (!abs)
            result += ']';
    } else {
        bool abs = false;
        if (absNum == 1 || absNum == 3)
            abs = true;

        if (abs)
            result += '$';

        result += KCCell::columnName(col);

        abs = false;
        if (absNum == 1 || absNum == 2)
            abs = true;

        if (abs)
            result += '$';

        result += QString::number(row);
    }

    return KCValue(result);
}


//
// Function: AREAS
//
KCValue func_areas(valVector args, ValueCalc *calc, FuncExtra *e)
{
    if (e) {
        if (e->regions[0].isValid())
            return KCValue(e->regions[0].rects().size());

        if ((e->ranges[0].col1 != -1) && (e->ranges[0].row1 != -1) &&
                (e->ranges[0].col2 != -1) && (e->ranges[0].row2 != -1))
            // we have a range reference - return 1
            return KCValue(1);
    }

    QString s = calc->conv()->asString(args[0]).asString();
    if (s[0] != '(' || s[s.length() - 1] != ')')
        return KCValue::errorVALUE();

    int l = s.length();

    int num = 0;
    QString ref;
    for (int i = 1; i < l; ++i) {
        if (s[i] == ',' || s[i] == ')') {
            if (!KCRegion(ref).isValid())
                return KCValue::errorVALUE();
            else {
                ++num;
                ref = "";
            }
        } else
            ref += s[i];
    }

    return KCValue(num);
}


//
// Function: CHOOSE
//
KCValue func_choose(valVector args, ValueCalc *calc, FuncExtra *)
{
    int cnt = args.count() - 1;
    int num = calc->conv()->asInteger(args[0]).asInteger();
    if ((num <= 0) || (num > cnt))
        return KCValue::errorVALUE();
    return args[num];
}


//
// Function: COLUMN
//
KCValue func_column(valVector args, ValueCalc *, FuncExtra *e)
{
    int col = e ? e->mycol : 0;
    if (e && args.count())
        col = e->ranges[0].col1;
    if (col > 0)
        return KCValue(col);
    return KCValue::errorVALUE();
}


//
// Function: COLUMNS
//
KCValue func_columns(valVector, ValueCalc *, FuncExtra *e)
{
    int col1 = e->ranges[0].col1;
    int col2 = e->ranges[0].col2;
    if ((col1 == -1) || (col2 == -1))
        return KCValue::errorVALUE();
    return KCValue(col2 - col1 + 1);
}


//
// Function: HLOOKUP
//
KCValue func_hlookup(valVector args, ValueCalc *calc, FuncExtra *)
{
    const KCValue key = args[0];
    const KCValue data = args[1];
    const int row = calc->conv()->asInteger(args[2]).asInteger();
    const int cols = data.columns();
    const int rows = data.rows();
    if (row < 1 || row > rows)
        return KCValue::errorVALUE();
    const bool rangeLookup = (args.count() > 3) ? calc->conv()->asBoolean(args[3]).asBoolean() : true;

    // now traverse the array and perform comparison
    KCValue r;
    KCValue v = KCValue::errorNA();
    for (int col = 0; col < cols; ++col) {
        // search in the first row
        const KCValue le = data.element(col, 0);
        if (calc->naturalEqual(key, le)) {
            return data.element(col, row - 1);
        }
        // optionally look for the next largest value that is less than key
        if (rangeLookup && calc->naturalLower(le, key) && calc->naturalLower(r, le)) {
            r = le;
            v = data.element(col, row - 1);
        }
    }
    return v;
}


//
// Function: INDEX
//
KCValue func_index(valVector args, ValueCalc *calc, FuncExtra *)
{
    // first argument can be either a range, then we return a given cell's
    // value, or a single cell containing an array - then we return the array
    // element. In any case, this function can assume that the given value
    // is the same. Because it is.

    KCValue val = args[0];
    unsigned row = calc->conv()->asInteger(args[1]).asInteger() - 1;
    unsigned col = calc->conv()->asInteger(args[2]).asInteger() - 1;
    if ((row >= val.rows()) || (col >= val.columns()))
        return KCValue::errorREF();
    return val.element(col, row);
}


//
// Function: INDIRECT
//
KCValue func_indirect(valVector args, ValueCalc *calc, FuncExtra *e)
{
    bool r1c1 = false;
    QString ref = calc->conv()->asString(args[0]).asString();
    if (args.count() == 2)
        r1c1 = !(calc->conv()->asBoolean(args[1]).asBoolean());

    if (ref.isEmpty())
        return KCValue::errorVALUE();

    if (r1c1) {
        // TODO: translate the r1c1 style to a1 style
        ref = ref;
    }

    const KCRegion region(ref, e->sheet->map(), e->sheet);
    if (!region.isValid() || !region.isSingular())
        return KCValue::errorVALUE();

    const KCCell cell(region.firstSheet(), region.firstRange().topLeft());
    if (!cell.isNull())
        return cell.value();
    return KCValue::errorVALUE();
}


//
// Function: LOOKUP
//
KCValue func_lookup(valVector args, ValueCalc *calc, FuncExtra *)
{
    KCValue num = calc->conv()->asNumeric(args[0]);
    if (num.isArray())
        return KCValue::errorVALUE();
    KCValue lookup = args[1];
    KCValue rr = args[2];
    unsigned cols = lookup.columns();
    unsigned rows = lookup.rows();
    if ((cols != rr.columns()) || (rows != rr.rows()))
        return KCValue::errorVALUE();
    KCValue res = KCValue::errorNA();

    // now traverse the array and perform comparison
    for (unsigned r = 0; r < rows; ++r)
        for (unsigned c = 0; c < cols; ++c) {
            // update the result, return if we cross the line
            KCValue le = lookup.element(c, r);
            if (calc->lower(le, num) || calc->equal(num, le))
                res = rr.element(c, r);
            else
                return res;
        }
    return res;
}

//
// Function: MATCH
//
KCValue func_match(valVector args, ValueCalc *calc, FuncExtra* e)
{
    int matchType = 1;
    if (args.count() == 3) {
        bool ok = true;
        matchType = calc->conv()->asInteger(args[2], &ok).asInteger();
        if (!ok)
            return KCValue::errorVALUE(); // invalid matchtype
    }

    const KCValue& searchValue = args[0];
    const KCValue& searchArray = args[1];

    if (e->ranges[1].rows() != 1 && e->ranges[1].columns() != 1)
        return KCValue::errorNA();
    int dr = 1, dc = 0;
    if (searchArray.columns() != 1) {
        dr = 0; dc = 1;
    }
    int n = qMax(searchArray.rows(), searchArray.columns());

    if (matchType == 0) {
        // linear search
        for (int r = 0, c = 0; r < n && c < n; r += dr, c += dc) {
            if (calc->naturalEqual(searchValue, searchArray.element(c, r), false)) {
                return KCValue(qMax(r, c) + 1);
            }
        }
        return KCValue::errorNA();
    } else if (matchType > 0) {
        // binary search
        int l = -1;
        int h = n;
        while (l+1 < h) {
            int m = (l+h)/2;
            if (calc->naturalLequal(searchArray.element(m*dc, m*dr), searchValue, false)) {
                l = m;
            } else {
                h = m;
            }
        }
        if (l == -1) return KCValue::errorNA();
        return KCValue(l+1);
    } else /* matchType < 0 */ {
        // binary search
        int l = -1;
        int h = n;
        while (l+1 < h) {
            int m = (l+h)/2;
            if (calc->naturalGequal(searchArray.element(m*dc, m*dr), searchValue, false)) {
                l = m;
            } else {
                h = m;
            }
        }
        if (l == -1) return KCValue::errorNA();
        return KCValue(l+1);
    }
}

//
// Function: MULTIPLE.OPERATIONS
//
KCValue func_multiple_operations(valVector args, ValueCalc *, FuncExtra *e)
{
    if (args.count() != 3 && args.count() != 5)
        return KCValue::errorVALUE(); // invalid number of parameters

    for (int i = 0; i < args.count(); i++) {
        if (e->ranges[i].col1 == -1 || e->ranges[i].row1 == -1)
            return KCValue::errorVALUE();
    }

    CellStorage *s = e->sheet->cellStorage();

    // get formula to evaluate
    int formulaCol = e->ranges[0].col1;
    int formulaRow = e->ranges[0].row1;
    Formula formula = s->formula(formulaCol, formulaRow);
    if (!formula.isValid())
        return KCValue::errorVALUE();

    CellIndirection cellIndirections;
    cellIndirections.insert(KCCell(e->sheet, e->ranges[1].col1, e->ranges[1].row1), KCCell(e->sheet, e->ranges[2].col1, e->ranges[2].row1));
    if (args.count() > 3) {
        cellIndirections.insert(KCCell(e->sheet, e->ranges[3].col1, e->ranges[3].row1), KCCell(e->sheet, e->ranges[4].col1, e->ranges[4].row1));
    }

    return formula.eval(cellIndirections);
}

//
// Function: ROW
//
KCValue func_row(valVector args, ValueCalc *, FuncExtra *e)
{
    int row = e ? e->myrow : 0;
    if (e && args.count())
        row = e->ranges[0].row1;
    if (row > 0)
        return KCValue(row);
    return KCValue::errorVALUE();
}


//
// Function: ROWS
//
KCValue func_rows(valVector, ValueCalc *, FuncExtra *e)
{
    int row1 = e->ranges[0].row1;
    int row2 = e->ranges[0].row2;
    if ((row1 == -1) || (row2 == -1))
        return KCValue::errorVALUE();
    return KCValue(row2 - row1 + 1);
}


//
// Function: VLOOKUP
//
KCValue func_vlookup(valVector args, ValueCalc *calc, FuncExtra *)
{
    const KCValue key = args[0];
    const KCValue data = args[1];
    const int col = calc->conv()->asInteger(args[2]).asInteger();
    const int cols = data.columns();
    const int rows = data.rows();
    if (col < 1 || col > cols)
        return KCValue::errorVALUE();
    const bool rangeLookup = (args.count() > 3) ? calc->conv()->asBoolean(args[3]).asBoolean() : true;

    // now traverse the array and perform comparison
    KCValue r;
    KCValue v = KCValue::errorNA();
    for (int row = 0; row < rows; ++row) {
        // search in the first column
        const KCValue le = data.element(0, row);
        if (calc->naturalEqual(key, le)) {
            return data.element(col - 1, row);
        }
        // optionally look for the next largest value that is less than key
        if (rangeLookup && calc->naturalLower(le, key) && calc->naturalLower(r, le)) {
            r = le;
            v = data.element(col - 1, row);
        }
    }
    return v;
}

#include "ReferenceModule.moc"
