/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KCells Team <koffice-devel@kde.org>
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

// built-in database functions

#include "DatabaseModule.h"

#include "KCFunction.h"
#include "KCFunctionModuleRegistry.h"
#include "KCValueCalc.h"
#include "KCValueConverter.h"

#include <KLocale>

using namespace KCells;

// prototypes
KCValue func_daverage(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dcount(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dcounta(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dget(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dmax(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dmin(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dproduct(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dstdev(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dstdevp(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dsum(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dvar(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dvarp(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_getpivotdata(valVector args, KCValueCalc *calc, FuncExtra *);


KCELLS_EXPORT_FUNCTION_MODULE("database", DatabaseModule)


DatabaseModule::DatabaseModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    f = new KCFunction("DAVERAGE",     func_daverage);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DCOUNT",       func_dcount);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DCOUNTA",      func_dcounta);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DGET",         func_dget);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DMAX",         func_dmax);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DMIN",         func_dmin);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DPRODUCT",     func_dproduct);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DSTDEV",       func_dstdev);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DSTDEVP",      func_dstdevp);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DSUM",         func_dsum);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DVAR",         func_dvar);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("DVARP",        func_dvarp);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("GETPIVOTDATA", func_getpivotdata);  // partially Excel-compatible
    f->setParamCount(2);
    f->setAcceptArray();
    add(f);
}

QString DatabaseModule::descriptionFileName() const
{
    return QString("database.xml");
}


int getFieldIndex(KCValueCalc *calc, KCValue fieldName,
                  KCValue database)
{
    if (fieldName.isNumber())
        return fieldName.asInteger() - 1;
    if (!fieldName.isString())
        return -1;

    QString fn = fieldName.asString();
    int cols = database.columns();
    for (int i = 0; i < cols; ++i)
        if (fn.toLower() ==
                calc->conv()->asString(database.element(i, 0)).asString().toLower())
            return i;
    return -1;
}

// ***********************************************************
// *** DBConditions class - maintains an array of conditions ***
// ***********************************************************

class DBConditions
{
public:
    DBConditions(KCValueCalc *vc, KCValue database, KCValue conds);
    ~DBConditions();
    /** Does a specified row of the database match the given criteria?
    The row with column names is ignored - hence 0 specifies first data row. */
    bool matches(unsigned row);
private:
    void parse(KCValue conds);
    KCValueCalc *calc;
    QList<QList<Condition*> > cond;
    int rows, cols;
    KCValue db;
};

DBConditions::DBConditions(KCValueCalc *vc, KCValue database,
                           KCValue conds) : calc(vc), rows(0), cols(0), db(database)
{
    parse(conds);
}

DBConditions::~DBConditions()
{
    int count = rows * cols;
    for (int r = 0; r < count; ++r)
        qDeleteAll(cond[r]);
}

void DBConditions::parse(KCValue conds)
{
    // initialize the array
    rows = conds.rows() - 1;
    cols = db.columns();
    int count = rows * cols;

    // if rows or cols is zero or negative, then we don't need to parse anything
    if(count <= 0)
        return;

    for (int r = 0; r < count; ++r)
        cond.append(QList<Condition*>());

    // perform the parsing itself
    int cc = conds.columns();
    for (int c = 0; c < cc; ++c) {
        // first row contains column names
        int col = getFieldIndex(calc, conds.element(c, 0), db);
        if (col < 0) continue;  // failed - ignore the column

        // fill in the conditions for a given column name
        for (int r = 0; r < rows; ++r) {
            KCValue cnd = conds.element(c, r + 1);
            if (cnd.isEmpty()) continue;
            int idx = r * cols + col;
            //if (cond[idx]) delete cond[idx];
            Condition* theCond = new Condition;
            calc->getCond(*theCond, cnd);
            cond[idx].append(theCond);
        }
    }
}

bool DBConditions::matches(unsigned row)
{
    if (row >= db.rows() - 1)
        return false;    // out of range

    // we have a match, if at least one row of criteria matches
    for (int r = 0; r < rows; ++r) {
        // within a row, all criteria must match
        bool match = true;
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            if (cond[idx].isEmpty()) continue;
            for (int i = 0; i < cond[idx].size(); i++) {
                if (!calc->matches(*cond[idx][i], db.element(c, row + 1))) {
                    match = false;  // didn't match
                    break;
                }
            }
        }
        if (match)  // all conditions in this row matched
            return true;
    }

    // no row matched
    return false;
}


// *******************************************
// *** KCFunction implementations start here ***
// *******************************************

// KCFunction: DSUM
KCValue func_dsum(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue res(0.0);
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty())
                res = calc->add(res, val);
        }

    return res;
}

// KCFunction: DAVERAGE
KCValue func_daverage(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue res;
    int count = 0;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty()) {
                res = calc->add(res, val);
                count++;
            }
        }
    if (count) res = calc->div(res, count);
    return res;
}

// KCFunction: DCOUNT
KCValue func_dcount(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    int count = 0;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            // fieldIndex is optional, if no field is specified count all rows matching the conditions
            if (fieldIndex < 0)
                count++;
            else {
                KCValue val = database.element(fieldIndex, r + 1);
                // include this value in the result
                if ((!val.isEmpty()) && (!val.isBoolean()) && (!val.isString()))
                    count++;
            }
        }

    return KCValue(count);
}

// KCFunction: DCOUNTA
KCValue func_dcounta(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    int count = 0;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            // fieldIndex is optional, if no field is specified count all rows matching the conditions
            if (fieldIndex < 0)
                count++;
            else {
                KCValue val = database.element(fieldIndex, r + 1);
                // include this value in the result
                if (!val.isEmpty())
                    count++;
            }
        }

    return KCValue(count);
}

// KCFunction: DGET
KCValue func_dget(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    bool match = false;
    KCValue result = KCValue::errorVALUE();
    int rows = database.rows() - 1;  // first row contains column names
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            if (match) {
                // error on multiple matches
                result = KCValue::errorVALUE();
                break;
            }
            result = database.element(fieldIndex, r + 1);
            match = true;
        }

    return result;
}

// KCFunction: DMAX
KCValue func_dmax(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue res;
    bool got = false;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty()) {
                if (!got) {
                    res = val;
                    got = true;
                } else if (calc->greater(val, res))
                    res = val;
            }
        }

    return res;
}

// KCFunction: DMIN
KCValue func_dmin(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue res;
    bool got = false;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty()) {
                if (!got) {
                    res = val;
                    got = true;
                } else if (calc->lower(val, res))
                    res = val;
            }
        }

    return res;
}

// KCFunction: DPRODUCT
KCValue func_dproduct(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue res = KCValue((double)1.0);
    bool got = false;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty()) {
                got = true;
                res = calc->mul(res, val);
            }
        }
    if (got)
        return res;
    return KCValue::errorVALUE();
}

// KCFunction: DSTDEV
KCValue func_dstdev(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // sqrt (dvar)
    return calc->sqrt(func_dvar(args, calc, 0));
}

// KCFunction: DSTDEVP
KCValue func_dstdevp(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // sqrt (dvarp)
    return calc->sqrt(func_dvarp(args, calc, 0));
}

// KCFunction: DVAR
KCValue func_dvar(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue avg;
    int count = 0;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty()) {
                avg = calc->add(avg, val);
                count++;
            }
        }
    if (count < 2) return KCValue::errorDIV0();
    avg = calc->div(avg, count);

    KCValue res;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty())
                res = calc->add(res, calc->sqr(calc->sub(val, avg)));
        }

    // res / (count-1)
    return calc->div(res, count - 1);
}

// KCFunction: DVARP
KCValue func_dvarp(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    KCValue conditions = args[2];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();

    DBConditions conds(calc, database, conditions);

    int rows = database.rows() - 1;  // first row contains column names
    KCValue avg;
    int count = 0;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty()) {
                avg = calc->add(avg, val);
                count++;
            }
        }
    if (count == 0) return KCValue::errorDIV0();
    avg = calc->div(avg, count);

    KCValue res;
    for (int r = 0; r < rows; ++r)
        if (conds.matches(r)) {
            KCValue val = database.element(fieldIndex, r + 1);
            // include this value in the result
            if (!val.isEmpty())
                res = calc->add(res, calc->sqr(calc->sub(val, avg)));
        }

    // res / count
    return calc->div(res, count);
}

// KCFunction: GETPIVOTDATA
// FIXME implement more things with this, see Excel !
KCValue func_getpivotdata(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue database = args[0];
    int fieldIndex = getFieldIndex(calc, args[1], database);
    if (fieldIndex < 0)
        return KCValue::errorVALUE();
    // the row at the bottom
    int row = database.rows() - 1;

    return database.element(fieldIndex, row);
}

#include "DatabaseModule.moc"
