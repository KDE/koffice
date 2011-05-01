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

// built-in logical functions
#include "LogicModule.h"

#include "KCFunction.h"
#include "KCFunctionModuleRegistry.h"
#include "KCValueCalc.h"
#include "ValueConverter.h"

#include <KLocale>

using namespace KSpread;

// prototypes (sorted alphabetically)
KCValue func_and(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_false(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_if(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_nand(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_nor(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_not(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_or(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_true(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_xor(valVector args, KCValueCalc *calc, FuncExtra *);


KSPREAD_EXPORT_FUNCTION_MODULE("logic", LogicModule)


LogicModule::LogicModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    f = new KCFunction("FALSE", func_false);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("TRUE", func_true);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("NOT", func_not);
    f->setParamCount(1);
    add(f);
    f = new KCFunction("AND", func_and);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("NAND", func_nand);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("NOR", func_nor);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("OR", func_or);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("XOR", func_xor);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("IF", func_if);
    f->setParamCount(2, 3);
    add(f);
}

QString LogicModule::descriptionFileName() const
{
    return QString("logic.xml");
}


// helper for most logical functions
static bool asBool(KCValue val, KCValueCalc *calc, bool* ok = 0)
{
    return calc->conv()->asBoolean(val, ok).asBoolean();
}

///////////////////////////////////////////////////////////////////////////////


//
// ArrayWalker: AND
//
void awAnd(KCValueCalc *calc, KCValue &res, KCValue value, KCValue)
{
    if (res.asBoolean())
        res = KCValue(asBool(value, calc));
}


//
// ArrayWalker: OR
//
void awOr(KCValueCalc *calc, KCValue &res, KCValue value, KCValue)
{
    if (!res.asBoolean())
        res = KCValue(asBool(value, calc));
}


//
// ArrayWalker: XOR
//
void awXor(KCValueCalc *calc, KCValue &count, KCValue value, KCValue)
{
    if (asBool(value, calc))
        count = KCValue(count.asInteger() + 1);
}

///////////////////////////////////////////////////////////////////////////////


//
// KCFunction: AND
//
KCValue func_and(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue result(true);
    int cnt = args.count();
    for (int i = 0; i < cnt; ++i) {
        if (args[i].isError())
            return args[i];
    }
    for (int i = 0; i < cnt; ++i) {
        calc->arrayWalk(args[i], result, awAnd, KCValue(0));
        if (! result.asBoolean())
            // if any value is false, return false
            return result;
    }
    // nothing is false -> return true
    return result;
}


//
// KCFunction: FALSE
//
KCValue func_false(valVector, KCValueCalc *, FuncExtra *)
{
    return KCValue(false);
}


//
// KCFunction: IF
//
KCValue func_if(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if ((args[0].isError()))
        return args[0];
    bool ok = true;
    bool guard = asBool(args[0], calc, &ok);
    if (!ok)
        return KCValue::errorVALUE();
    if (guard)
        return args[1];
    // evaluated to false
    if (args.count() == 3) {
        if (args[2].isNull()) {
            return KCValue(0);
        } else {
            return args[2];
        }
    } else {
        // only two arguments
        return KCValue(false);
    }
}


//
// KCFunction: NAND
//
KCValue func_nand(valVector args, KCValueCalc *calc, FuncExtra *extra)
{
    // AND in reverse
    return KCValue(! func_and(args, calc, extra).asBoolean());
}


//
// KCFunction: NOR
//
KCValue func_nor(valVector args, KCValueCalc *calc, FuncExtra *extra)
{
    // OR in reverse
    return KCValue(! func_or(args, calc, extra).asBoolean());
}


//
// KCFunction: NOT
//
KCValue func_not(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args[0].isError())
        return args[0];

    bool ok = true;
    bool val = !asBool(args[0], calc, &ok);
    if (!ok) return KCValue::errorVALUE();
    return KCValue(val);
}


//
// KCFunction: OR
//
KCValue func_or(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue result(false);
    int cnt = args.count();
    for (int i = 0; i < cnt; ++i) {
        if (args[i].isError())
            return args[i];
    }
    for (int i = 0; i < cnt; ++i) {
        calc->arrayWalk(args[i], result, awOr, KCValue(0));
        if (result.asBoolean())
            // if any value is true, return true
            return result;
    }
    // nothing is true -> return false
    return result;
}


//
// KCFunction: TRUE
//
KCValue func_true(valVector, KCValueCalc *, FuncExtra *)
{
    return KCValue(true);
}


//
// KCFunction: XOR
//
KCValue func_xor(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // exclusive OR - exactly one value must be true
    int cnt = args.count();
    KCValue count(0);
    for (int i = 0; i < cnt; ++i) {
        if (args[i].isError())
            return args[i];
    }
    for (int i = 0; i < cnt; ++i)
        calc->arrayWalk(args[i], count, awXor, KCValue(0));
    return KCValue(count.asInteger() == 1);
}

#include "LogicModule.moc"
