/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KSpread Team <koffice-devel@kde.org>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>
   Copyright 2007 Sascha Pfau <MrPeacock@gmail.com>
   Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
     Contact: Suresh Chande suresh.chande@nokia.com

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

// built-in math functions
#include "MathModule.h"

// needed for RANDBINOM and so
#include <math.h>

#include <kdebug.h>
#include <KLocale>

#include "KCFunctionModuleRegistry.h"
#include "KCFunction.h"
#include "KCFunctionRepository.h"
#include "KCValueCalc.h"
#include "ValueConverter.h"

// needed for SUBTOTAL:
#include "KCCell.h"
#include "KCSheet.h"
#include "RowColumnFormat.h"

// needed by MDETERM and MINVERSE
#include <Eigen/LU>

using namespace KSpread;

// RANDBINOM and RANDNEGBINOM won't support arbitrary precision

// prototypes
KCValue func_abs(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ceil(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ceiling(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_count(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_counta(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_countblank(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_countif(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_cur(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_div(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_eps(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_even(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_exp(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_fact(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_factdouble(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_fib(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_floor(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_gamma(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_gcd(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_int(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_inv(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_kproduct(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_lcm(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ln(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_log2(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_log10(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_logn(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_max(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_maxa(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mdeterm(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_min(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mina(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_minverse(valVector args, KCValueCalc* calc, FuncExtra*);
KCValue func_mmult(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mod(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mround(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mult(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_multinomial(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_munit(valVector args, KCValueCalc* calc, FuncExtra*);
KCValue func_odd(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_pow(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_quotient(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_product(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_rand(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randbetween(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randbernoulli(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randbinom(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randexp(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randnegbinom(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randnorm(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_randpoisson(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_rootn(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_round(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_rounddown(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_roundup(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_seriessum(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sign(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sqrt(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sqrtpi(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_subtotal(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sum(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_suma(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sumif(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sumsq(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_transpose(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_trunc(valVector args, KCValueCalc *calc, FuncExtra *);


// KCValue func_multipleOP (valVector args, KCValueCalc *calc, FuncExtra *);


KSPREAD_EXPORT_FUNCTION_MODULE("math", MathModule)


MathModule::MathModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    /*
      f = new KCFunction ("MULTIPLEOPERATIONS", func_multipleOP);
    add(f);
    */

    // functions that don't take array parameters
    f = new KCFunction("ABS",           func_abs);
    add(f);
    f = new KCFunction("CEIL",          func_ceil);
    add(f);
    f = new KCFunction("CEILING",       func_ceiling);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("CUR",           func_cur);
    add(f);
    f = new KCFunction("EPS",           func_eps);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("EVEN",          func_even);
    add(f);
    f = new KCFunction("EXP",           func_exp);
    add(f);
    f = new KCFunction("FACT",          func_fact);
    add(f);
    f = new KCFunction("FACTDOUBLE",    func_factdouble);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFACTDOUBLE");
    add(f);
    f = new KCFunction("FIB",           func_fib);  // KSpread-specific, like Quattro-Pro's FIB
    add(f);
    f = new KCFunction("FLOOR",         func_floor);
    f->setParamCount(1, 3);
    add(f);
    f = new KCFunction("GAMMA",         func_gamma);
    add(f);
    f = new KCFunction("INT",           func_int);
    add(f);
    f = new KCFunction("INV",           func_inv);
    add(f);
    f = new KCFunction("LN",            func_ln);
    add(f);
    f = new KCFunction("LOG",           func_logn);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("LOG2",          func_log2);
    add(f);
    f = new KCFunction("LOG10",         func_log10);
    add(f);
    f = new KCFunction("LOGN",          func_logn);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("MOD",           func_mod);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("MROUND",        func_mround);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMROUND");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("MULTINOMIAL",   func_multinomial);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMULTINOMIAL");
    f->setParamCount(1, -1);
    add(f);
    f = new KCFunction("ODD",           func_odd);
    add(f);
    f = new KCFunction("POW",         func_pow);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("POWER",         func_pow);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("QUOTIENT",      func_quotient);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETQUOTIENT");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("RAND",          func_rand);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("RANDBERNOULLI", func_randbernoulli);
    add(f);
    f = new KCFunction("RANDBETWEEN",   func_randbetween);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRANDBETWEEN");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("RANDBINOM",     func_randbinom);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("RANDEXP",       func_randexp);
    add(f);
    f = new KCFunction("RANDNEGBINOM",  func_randnegbinom);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("RANDNORM",      func_randnorm);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("RANDPOISSON",   func_randpoisson);
    add(f);
    f = new KCFunction("ROOTN",         func_rootn);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("ROUND",         func_round);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("ROUNDDOWN",     func_rounddown);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("ROUNDUP",       func_roundup);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("SIGN",          func_sign);
    add(f);
    f = new KCFunction("SQRT",          func_sqrt);
    add(f);
    f = new KCFunction("SQRTPI",        func_sqrtpi);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETSQRTPI");
    add(f);
    f = new KCFunction("TRUNC",         func_trunc);
    f->setParamCount(1, 2);
    add(f);

    // functions that operate over arrays
    f = new KCFunction("COUNT",         func_count);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("COUNTA",        func_counta);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("COUNTBLANK",    func_countblank);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("COUNTIF",       func_countif);
    f->setParamCount(2);
    f->setAcceptArray();
    f->setNeedsExtra(true);
    add(f);
    f = new KCFunction("DIV",           func_div);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("G_PRODUCT",     func_kproduct);  // Gnumeric compatibility
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("GCD",           func_gcd);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETGCD");
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("KPRODUCT",      func_kproduct);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("LCM",           func_lcm);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETLCM");
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MAX",           func_max);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MAXA",          func_maxa);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MDETERM",          func_mdeterm);
    f->setParamCount(1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MIN",           func_min);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MINA",          func_mina);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MINVERSE",         func_minverse);
    f->setParamCount(1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MMULT",          func_mmult);
    f->setParamCount(2);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MULTIPLY",      func_product);   // same as PRODUCT
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("MUNIT",         func_munit);
    f->setParamCount(1);
    add(f);
    f = new KCFunction("PRODUCT",       func_product);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("SERIESSUM",     func_seriessum);
    f->setParamCount(3, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("SUM",           func_sum);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("SUMA",          func_suma);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("SUBTOTAL",      func_subtotal);
    f->setParamCount(2);
    f->setAcceptArray();
    f->setNeedsExtra(true);
    add(f);
    f = new KCFunction("SUMIF",         func_sumif);
    f->setParamCount(2, 3);
    f->setAcceptArray();
    f->setNeedsExtra(true);
    add(f);
    f = new KCFunction("SUMSQ",         func_sumsq);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("TRANSPOSE",     func_transpose);
    f->setParamCount(1);
    f->setAcceptArray();
    add(f);
}

QString MathModule::descriptionFileName() const
{
    return QString("math.xml");
}


// KCFunction: SQRT
KCValue func_sqrt(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue arg = args[0];
    if (calc->gequal(arg, KCValue(0.0)))
        return calc->sqrt(arg);
    else
        return KCValue::errorVALUE();
}

// KCFunction: SQRTPI
KCValue func_sqrtpi(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // sqrt (val * PI)
    KCValue arg = args[0];
    if (calc->gequal(arg, KCValue(0.0)))
        return calc->sqrt(calc->mul(args[0], calc->pi()));
    else
        return KCValue::errorVALUE();
}

// KCFunction: ROOTN
KCValue func_rootn(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->pow(args[0], calc->div(KCValue(1), args[1]));
}

// KCFunction: CUR
KCValue func_cur(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->pow(args[0], KCValue(1.0 / 3.0));
}

// KCFunction: ABS
KCValue func_abs(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->abs(args[0]);
}

// KCFunction: exp
KCValue func_exp(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->exp(args[0]);
}

// KCFunction: ceil
KCValue func_ceil(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->roundUp(args[0], KCValue(0));
}

// KCFunction: ceiling
KCValue func_ceiling(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue number = args[0];
    KCValue res;
    if (args.count() == 2)
        res = args[1];
    else
        res = calc->gequal(number, KCValue(0.0)) ? KCValue(1.0) : KCValue(-1.0);

    // short-circuit, and allow CEILING(0;0) to give 0 (which is correct)
    // instead of DIV0 error
    if (calc->isZero(number))
        return KCValue(0.0);

    if (calc->isZero(res))
        return KCValue::errorDIV0();

    KCValue d = calc->div(number, res);
    if (calc->greater(KCValue(0), d))
        return KCValue::errorNUM();

    KCValue rud = calc->roundDown(d);
    if (calc->approxEqual(rud, d))
        d = calc->mul(rud, res);
    else
        d = calc->mul(calc->roundUp(d), res);

    return d;
}

// KCFunction: FLOOR
KCValue func_floor(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (calc->approxEqual(args[0], KCValue(0.0)))
        return KCValue(0);
    KCNumber number = args[0].asFloat();

    KCNumber significance;
    if (args.count() >= 2) { // we have the optional "significance" argument
        significance = args[1].asFloat();
        // Sign of number and significance must match.
        if (calc->gequal(args[0], KCValue(0.0)) != calc->gequal(args[1], KCValue(0.0)))
            return KCValue::errorVALUE();
    } else // use 1 or -1, depending on the sign of the first argument
        significance = calc->gequal(args[0], KCValue(0.0)) ? 1.0 : -1.0;
    if (calc->approxEqual(KCValue(significance), KCValue(0.0)))
        return KCValue(0);

    const bool mode = (args.count() == 3) ? (args[2].asFloat() != 0.0) : false;

    KCNumber result;
    if (mode) // round towards zero
        result = ((int)(number / significance)) * significance;
    else { // round towards negative infinity
        result = number / significance; // always positive, because signs match
        if (calc->gequal(args[0], KCValue(0.0))) // positive values
            result = floor(result) * significance;
        else // negative values
            result = ceil(result) * significance;
    }
    return KCValue(result);
}

// KCFunction: GAMMA
KCValue func_gamma(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->GetGamma(args[0]);
}

// KCFunction: ln
KCValue func_ln(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if ((args [0].isNumber() == false) || args[0].asFloat() <= 0)
        return KCValue::errorNUM();
    return calc->ln(args[0]);
}

// KCFunction: LOGn
KCValue func_logn(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args [0].isError())
        return args [0];
    if (args [0].isEmpty())
        return KCValue::errorNUM();
    if (args [0].isNumber() == false)
        return KCValue::errorVALUE();
    if (args[0].asFloat() <= 0)
        return KCValue::errorNUM();
    if (args.count() == 2) {
        if (args [1].isError())
            return args [1];
        if (args [1].isEmpty())
            return KCValue::errorNUM();
        if (args [1].isNumber() == false)
            return KCValue::errorVALUE();
        if (args [1].asFloat() <= 0)
            return KCValue::errorNUM();
        return calc->log(args[0], args[1]);
    } else
        return calc->log(args[0]);
}

// KCFunction: LOG2
KCValue func_log2(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->log(args[0], KCValue(2.0));
}

// KCFunction: LOG10
KCValue func_log10(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args [0].isError())
        return args [0];
    if ((args [0].isNumber() == false) || (args[0].asFloat() <= 0))
        return KCValue::errorNUM();
    return calc->log(args[0]);
}

// KCFunction: sum
KCValue func_sum(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->sum(args, false);
}

// KCFunction: suma
KCValue func_suma(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->sum(args, true);
}

// KCFunction: SUMIF
KCValue func_sumif(valVector args, KCValueCalc *calc, FuncExtra *e)
{
    KCValue checkRange = args[0];
    QString condition = calc->conv()->asString(args[1]).asString();
    Condition cond;
    calc->getCond(cond, KCValue(condition));

    if (args.count() == 3) {
        KCCell sumRangeStart(e->sheet, e->ranges[2].col1, e->ranges[2].row1);
        return calc->sumIf(sumRangeStart, checkRange, cond);
    } else {
        return calc->sumIf(checkRange, cond);
    }
}

// KCFunction: product
KCValue func_product(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->product(args, KCValue(0.0));
}

// KCFunction: seriessum
KCValue func_seriessum(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double fX = calc->conv()->asFloat(args[0]).asFloat();
    double fN = calc->conv()->asFloat(args[1]).asFloat();
    double fM = calc->conv()->asFloat(args[2]).asFloat();

    if (fX == 0.0 && fN == 0.0)
        return KCValue::errorNUM();

    double res = 0.0;

    if (fX != 0.0) {

        for (unsigned int i = 0 ; i < args[3].count(); i++) {
            res += args[3].element(i).asFloat() * pow(fX, fN);
            fN += fM;
        }
    }

    return KCValue(res);
}

// KCFunction: kproduct
KCValue func_kproduct(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->product(args, KCValue(1.0));
}

// KCFunction: DIV
KCValue func_div(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue val = args[0];
    for (int i = 1; i < args.count(); ++i) {
        val = calc->div(val, args[i]);
        if (val.isError())
            return val;
    }
    return val;
}

// KCFunction: SUMSQ
KCValue func_sumsq(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue res;
    calc->arrayWalk(args, res, calc->awFunc("sumsq"), KCValue(0));
    return res;
}

// KCFunction: MAX
KCValue func_max(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue m = calc->max(args, false);
    return m.isEmpty() ? KCValue(0.0) : m;
}

// KCFunction: MAXA
KCValue func_maxa(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue m = calc->max(args);
    return m.isEmpty() ? KCValue(0.0) : m;
}

// KCFunction: MIN
KCValue func_min(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue m = calc->min(args, false);
    return m.isEmpty() ? KCValue(0.0) : m;
}

// KCFunction: MINA
KCValue func_mina(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue m = calc->min(args, true);
    return m.isEmpty() ? KCValue(0.0) : m;
}

// KCFunction: INT
KCValue func_int(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->conv()->asInteger(args[0]);
}

// KCFunction: QUOTIENT
KCValue func_quotient(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (calc->isZero(args[1]))
        return KCValue::errorDIV0();

    double res = calc->conv()->toFloat(calc->div(args[0], args[1]));
    if (res < 0)
        res = ceil(res);
    else
        res = floor(res);

    return KCValue(res);
}


// KCFunction: eps
KCValue func_eps(valVector, KCValueCalc *calc, FuncExtra *)
{
    return calc->eps();
}

KCValue func_randexp(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // -1 * d * log (random)
    return calc->mul(calc->mul(args[0], KCValue(-1)), calc->random());
}

KCValue func_randbinom(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // this function will not support arbitrary precision

    double d  = numToDouble(calc->conv()->toFloat(args[0]));
    int    tr = calc->conv()->toInteger(args[1]);

    if (d < 0 || d > 1)
        return KCValue::errorVALUE();

    if (tr < 0)
        return KCValue::errorVALUE();

    // taken from gnumeric
    double x = pow(1 - d, tr);
    double r = (double) rand() / (RAND_MAX + 1.0);
    double t = x;
    int i = 0;

    while (r > t) {
        x *= (((tr - i) * d) / ((1 + i) * (1 - d)));
        i++;
        t += x;
    }

    return KCValue(i);
}

KCValue func_randnegbinom(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // this function will not support arbitrary precision

    double d  = numToDouble(calc->conv()->toFloat(args[0]));
    int    f = calc->conv()->toInteger(args[1]);

    if (d < 0 || d > 1)
        return KCValue::errorVALUE();

    if (f < 0)
        return KCValue::errorVALUE();


    // taken from Gnumeric
    double x = pow(d, f);
    double r = (double) rand() / (RAND_MAX + 1.0);
    double t = x;
    int i = 0;

    while (r > t) {
        x *= (((f + i) * (1 - d)) / (1 + i)) ;
        i++;
        t += x;
    }

    return KCValue(i);
}

KCValue func_randbernoulli(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue rnd = calc->random();
    return KCValue(calc->greater(rnd, args[0]) ? 1.0 : 0.0);
}

KCValue func_randnorm(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue mu = args[0];
    KCValue sigma = args[1];

    //using polar form of the Box-Muller transformation
    //refer to http://www.taygeta.com/random/gaussian.html for more info

    KCValue x1, x2, w;
    do {
        // x1,x2 = 2 * random() - 1
        x1 = calc->random(2.0);
        x2 = calc->random(2.0);
        x1 = calc->sub(x1, 1);
        x1 = calc->sub(x2, 1);
        w = calc->add(calc->sqr(x1), calc->sqr(x2));
    } while (calc->gequal(w, KCValue(1.0)));    // w >= 1.0

    //sqrt ((-2.0 * log (w)) / w) :
    w = calc->sqrt(calc->div(calc->mul(KCValue(-2.0), calc->ln(w)), w));
    KCValue res = calc->mul(x1, w);

    res = calc->add(calc->mul(res, sigma), mu);    // res*sigma + mu
    return res;
}

KCValue func_randpoisson(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (calc->lower(args[0], KCValue(0)))
        return KCValue::errorVALUE();

    // taken from Gnumeric...
    KCValue x = calc->exp(calc->mul(KCValue(-1), args[0]));     // e^(-A)
    KCValue r = calc->random();
    KCValue t = x;
    int i = 0;

    while (calc->greater(r, t)) {    // r > t
        x = calc->mul(x, calc->div(args[0], i + 1));    // x *= (A/(i+1))
        t = calc->add(t, x);     //t += x
        i++;
    }

    return KCValue(i);
}

// KCFunction: rand
KCValue func_rand(valVector, KCValueCalc *calc, FuncExtra *)
{
    return calc->random();
}

// KCFunction: RANDBETWEEN
KCValue func_randbetween(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v1 = args[0];
    KCValue v2 = args[1];
    if (calc->greater(v2, v1)) {
        v1 = args[1];
        v2 = args[0];
    }
    return calc->add(v1, calc->random(calc->sub(v2, v1)));
}

// KCFunction: POW
KCValue func_pow(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->pow(args[0], args[1]);
}

// KCFunction: MOD
KCValue func_mod(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->mod(args[0], args[1]);
}

// KCFunction: fact
KCValue func_fact(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args[0].isInteger() || args[0].asInteger() > 0)
        return calc->fact(args[0]);
    else
        return KCValue::errorNUM();
}

// KCFunction: FACTDOUBLE
KCValue func_factdouble(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args[0].isInteger() || args[0].asInteger() > 0)
        return calc->factDouble(args[0]);
    else
        return KCValue::errorNUM();
}

// KCFunction: MULTINOMIAL
KCValue func_multinomial(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // (a+b+c)! / a!b!c!  (any number of params possible)
    KCValue num = KCValue(0), den = KCValue(1);
    for (int i = 0; i < args.count(); ++i) {
        num = calc->add(num, args[i]);
        den = calc->mul(den, calc->fact(args[i]));
    }
    num = calc->fact(num);
    return calc->div(num, den);
}

// KCFunction: sign
KCValue func_sign(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return KCValue(calc->sign(args[0]));
}

// KCFunction: INV
KCValue func_inv(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->mul(args[0], -1);
}

// KCFunction: MROUND
KCValue func_mround(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue d = args[0];
    KCValue m = args[1];

    // signs must be the same
    if ((calc->greater(d, KCValue(0)) && calc->lower(m, KCValue(0)))
            || (calc->lower(d, KCValue(0)) && calc->greater(m, KCValue(0))))
        return KCValue::errorVALUE();

    int sign = 1;

    if (calc->lower(d, KCValue(0))) {
        sign = -1;
        d = calc->mul(d, KCValue(-1));
        m = calc->mul(m, KCValue(-1));
    }

    // from gnumeric:
    KCValue mod = calc->mod(d, m);
    KCValue div = calc->sub(d, mod);

    KCValue result = div;
    if (calc->gequal(mod, calc->div(m, KCValue(2))))  // mod >= m/2
        result = calc->add(result, m);      // result += m
    result = calc->mul(result, sign);     // add the sign

    return result;
}

// KCFunction: ROUNDDOWN
KCValue func_rounddown(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args.count() == 2) {
        if (calc->greater(args[0], 0.0))
            return calc->roundDown(args[0], args[1]);
        else
            return calc->roundUp(args[0], args[1]);
    }

    if (calc->greater(args[0], 0.0))
        return calc->roundDown(args[0], 0);
    else
        return calc->roundUp(args[0], 0);
}

// KCFunction: ROUNDUP
KCValue func_roundup(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args.count() == 2) {
        if (calc->greater(args[0], 0.0))
            return calc->roundUp(args[0], args[1]);
        else
            return calc->roundDown(args[0], args[1]);
    }

    if (calc->greater(args[0], 0.0))
        return calc->roundUp(args[0], 0);
    else
        return calc->roundDown(args[0], 0);
}

// KCFunction: ROUND
KCValue func_round(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args.count() == 2)
        return calc->round(args[0], args[1]);
    return calc->round(args[0], 0);
}

// KCFunction: EVEN
KCValue func_even(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (calc->greater(args[0], 0.0)) {
        const KCValue value = calc->roundUp(args[0], 0);
        return calc->isZero(calc->mod(value, KCValue(2))) ? value : calc->add(value, KCValue(1));
    } else {
        const KCValue value = calc->roundDown(args[0], 0);
        return calc->isZero(calc->mod(value, KCValue(2))) ? value : calc->sub(value, KCValue(1));
    }
}

// KCFunction: ODD
KCValue func_odd(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (calc->gequal(args[0], KCValue(0))) {
        const KCValue value = calc->roundUp(args[0], 0);
        return calc->isZero(calc->mod(value, KCValue(2))) ? calc->add(value, KCValue(1)) : value;
    } else {
        const KCValue value = calc->roundDown(args[0], 0);
        return calc->isZero(calc->mod(value, KCValue(2))) ? calc->add(value, KCValue(-1)) : value;
    }
}

KCValue func_trunc(valVector args, KCValueCalc *calc, FuncExtra *)
{
    Q_UNUSED(calc)
    KCNumber result = args[0].asFloat();
    if (args.count() == 2)
        result = result * ::pow(10, (int)args[1].asInteger());
    result = (args[0].asFloat() < 0) ? -(qint64)(-result) : (qint64)result;
    if (args.count() == 2)
        result = result * ::pow(10, -(int)args[1].asInteger());
    return KCValue(result);
}

// KCFunction: COUNT
KCValue func_count(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return KCValue(calc->count(args, false));
}

// KCFunction: COUNTA
KCValue func_counta(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return KCValue(calc->count(args));
}

// KCFunction: COUNTBLANK
KCValue func_countblank(valVector args, KCValueCalc *, FuncExtra *)
{
    int cnt = 0;
    for (int i = 0; i < args.count(); ++i)
        if (args[i].isArray()) {
            int rows = args[i].rows();
            int cols = args[i].columns();
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols; ++c)
                    if (args[i].element(c, r).isEmpty())
                        cnt++;
        } else if (args[i].isEmpty())
            cnt++;
    return KCValue(cnt);
}

// KCFunction: COUNTIF
KCValue func_countif(valVector args, KCValueCalc *calc, FuncExtra *e)
{
    // the first parameter must be a reference
    if ((e->ranges[0].col1 == -1) || (e->ranges[0].row1 == -1))
        return KCValue::errorNA();

    KCValue range = args[0];
    QString condition = calc->conv()->asString(args[1]).asString();

    Condition cond;
    calc->getCond(cond, KCValue(condition));

    return KCValue(calc->countIf(range, cond));
}

// KCFunction: FIB
KCValue func_fib(valVector args, KCValueCalc *calc, FuncExtra *)
{
    /*
    Lucas' formula for the nth Fibonacci number F(n) is given by

             ((1+sqrt(5))/2)^n - ((1-sqrt(5))/2)^n
      F(n) = ------------------------------------- .
                             sqrt(5)

    */
    KCValue n = args[0];
    if (!n.isNumber())
        return KCValue::errorVALUE();

    if (!calc->greater(n, KCValue(0.0)))
        return KCValue::errorNUM();

    KCValue s = calc->sqrt(KCValue(5.0));
    // u1 = ((1+sqrt(5))/2)^n
    KCValue u1 = calc->pow(calc->div(calc->add(KCValue(1), s), KCValue(2)), n);
    // u2 = ((1-sqrt(5))/2)^n
    KCValue u2 = calc->pow(calc->div(calc->sub(KCValue(1), s), KCValue(2)), n);

    KCValue result = calc->div(calc->sub(u1, u2), s);
    return result;
}

static KCValue func_gcd_helper(const KCValue &val, KCValueCalc *calc)
{
    KCValue res(0);
    if (!val.isArray())
        return val;
    for (uint row = 0; row < val.rows(); ++row)
        for (uint col = 0; col < val.columns(); ++col) {
            KCValue v = val.element(col, row);
            if (v.isArray())
                v = func_gcd_helper(v, calc);
            res = calc->gcd(res, calc->roundDown(v));
        }
    return res;
}

// KCFunction: GCD
KCValue func_gcd(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue result = KCValue(0);
    for (int i = 0; i < args.count(); ++i) {
        if (args[i].isArray()) {
            result = calc->gcd(result, func_gcd_helper(args[i], calc));
        } else {
            if (args[i].isNumber() && args[i].asInteger() >= 0) {
                result = calc->gcd(result, calc->roundDown(args[i]));
            } else {
                return KCValue::errorNUM();
            }
        }
    }
    return result;
}

static KCValue func_lcm_helper(const KCValue &val, KCValueCalc *calc)
{
    KCValue res = KCValue(0);
    if (!val.isArray())
        return val;
    for (unsigned int row = 0; row < val.rows(); ++row)
        for (unsigned int col = 0; col < val.columns(); ++col) {
            KCValue v = val.element(col, row);
            if (v.isArray())
                v = func_lcm_helper(v, calc);
            res = calc->lcm(res, calc->roundDown(v));
        }
    return res;
}

// KCFunction: lcm
KCValue func_lcm(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue result = KCValue(0);
    for (int i = 0; i < args.count(); ++i) {
        if (args[i].isArray()) {
            result = calc->lcm(result, func_lcm_helper(args[i], calc));
        } else {
            if (args[i].isNumber() == false) {
                return KCValue::errorNUM();
            } else {
                // its a number
                if (args[i].asInteger() < 0) {
                    return KCValue::errorNUM();
                } else if (args[i].asInteger() == 0) {
                    return KCValue(0);
                } else { // number > 0
                    result = calc->lcm(result, calc->roundDown(args[i]));
                }
            }
        }
    }
    return result;
}

static Eigen::MatrixXd convert(const KCValue& matrix, KCValueCalc *calc)
{
    const int rows = matrix.rows(), cols = matrix.columns();
    Eigen::MatrixXd eMatrix(rows, cols);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            eMatrix(row, col) = numToDouble(calc->conv()->toFloat(matrix.element(col, row)));
        }
    }
    return eMatrix;
}

static KCValue convert(const Eigen::MatrixXd& eMatrix)
{
    const int rows = eMatrix.rows(), cols = eMatrix.cols();
    KCValue matrix(KCValue::Array);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            matrix.setElement(col, row, KCValue(eMatrix(row, col)));
        }
    }
    return matrix;
}

// KCFunction: MDETERM
KCValue func_mdeterm(valVector args, KCValueCalc* calc, FuncExtra*)
{
    KCValue matrix = args[0];
    if (matrix.columns() != matrix.rows() || matrix.rows() < 1)
        return KCValue::errorVALUE();

    const Eigen::MatrixXd eMatrix = convert(matrix, calc);

    return KCValue(eMatrix.determinant());
}

// KCFunction: MINVERSE
KCValue func_minverse(valVector args, KCValueCalc* calc, FuncExtra*)
{
    KCValue matrix = args[0];
    if (matrix.columns() != matrix.rows() || matrix.rows() < 1)
        return KCValue::errorVALUE();

    Eigen::MatrixXd eMatrix = convert(matrix, calc),
                              eMatrixInverse(eMatrix.rows(), eMatrix.cols());
    Eigen::LU<Eigen::MatrixXd> lu(eMatrix);
    if (lu.isInvertible()) {
        lu.computeInverse(&eMatrixInverse);
        return convert(eMatrixInverse);
    } else
        return KCValue::errorDIV0();
}

// KCFunction: mmult
KCValue func_mmult(valVector args, KCValueCalc *calc, FuncExtra *)
{
    const Eigen::MatrixXd eMatrix1 = convert(args[0], calc);
    const Eigen::MatrixXd eMatrix2 = convert(args[1], calc);

    if (eMatrix1.cols() != eMatrix2.rows())    // row/column counts must match
        return KCValue::errorVALUE();

    return convert(eMatrix1 * eMatrix2);
}

// KCFunction: MUNIT
KCValue func_munit(valVector args, KCValueCalc* calc, FuncExtra*)
{
    const int dim = calc->conv()->asInteger(args[0]).asInteger();
    if (dim < 1)
        return KCValue::errorVALUE();
    KCValue result(KCValue::Array);
    for (int row = 0; row < dim; ++row)
        for (int col = 0; col < dim; ++col)
            result.setElement(col, row, KCValue(col == row ? 1 : 0));
    return result;
}

// KCFunction: SUBTOTAL
// This function requires access to the KCSheet and so on, because
// it needs to check whether cells contain the SUBTOTAL formula or not ...
// Cells containing a SUBTOTAL formula must be ignored.
KCValue func_subtotal(valVector args, KCValueCalc *calc, FuncExtra *e)
{
    int function = calc->conv()->asInteger(args[0]).asInteger();
    KCValue range = args[1];
    int r1 = -1, c1 = -1, r2 = -1, c2 = -1;
    if (e) {
        r1 = e->ranges[1].row1;
        c1 = e->ranges[1].col1;
        r2 = e->ranges[1].row2;
        c2 = e->ranges[1].col2;
    }

    // exclude manually hidden rows. http://tools.oasis-open.org/issues/browse/OFFICE-2030
    bool excludeHiddenRows = false;
    if(function > 100) {
        excludeHiddenRows = true;
        function = function % 100; // translate e.g. 106 to 6.
    }

    // run through the cells in the selected range
    KCValue empty;
    if ((r1 > 0) && (c1 > 0) && (r2 > 0) && (c2 > 0)) {
        for (int r = r1; r <= r2; ++r) {
            const bool setAllEmpty = excludeHiddenRows && e->sheet->rowFormat(r)->isHidden();
            for (int c = c1; c <= c2; ++c) {
                // put an empty value to all cells in a hidden row
                if(setAllEmpty) {
                    range.setElement(c - c1, r - r1, empty);
                    continue;
                }
                KCCell cell(e->sheet, c, r);
                // put an empty value to the place of all occurrences of the SUBTOTAL function
                if (!cell.isDefault() && cell.isFormula() && cell.userInput().indexOf("SUBTOTAL", 0, Qt::CaseInsensitive) != -1)
                    range.setElement(c - c1, r - r1, empty);
            }
        }
    }

    // Good. Now we can execute the necessary function on the range.
    KCValue res;
    QSharedPointer<KCFunction> f;
    valVector a;
    switch (function) {
    case 1: // Average
        res = calc->avg(range, false);
        break;
    case 2: // Count
        res = KCValue(calc->count(range, false));
        break;
    case 3: // CountA
        res = KCValue(calc->count(range));
        break;
    case 4: // MAX
        res = calc->max(range, false);
        break;
    case 5: // Min
        res = calc->min(range, false);
        break;
    case 6: // Product
        res = calc->product(range, KCValue(0.0), false);
        break;
    case 7: // StDev
        res = calc->stddev(range, false);
        break;
    case 8: // StDevP
        res = calc->stddevP(range, false);
        break;
    case 9: // Sum
        res = calc->sum(range, false);
        break;
    case 10: // Var
        f = KCFunctionRepository::self()->function("VAR");
        if (!f) return KCValue::errorVALUE();
        a.resize(1);
        a[0] = range;
        res = f->exec(a, calc, 0);
        break;
    case 11: // VarP
        f = KCFunctionRepository::self()->function("VARP");
        if (!f) return KCValue::errorVALUE();
        a.resize(1);
        a[0] = range;
        res = f->exec(a, calc, 0);
        break;
    default:
        return KCValue::errorVALUE();
    }
    return res;
}

// KCFunction: TRANSPOSE
KCValue func_transpose(valVector args, KCValueCalc *calc, FuncExtra *)
{
    Q_UNUSED(calc);
    KCValue matrix = args[0];
    const int cols = matrix.columns();
    const int rows = matrix.rows();

    KCValue transpose(KCValue::Array);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (!matrix.element(col, row).isEmpty())
                transpose.setElement(row, col, matrix.element(col, row));
        }
    }
    return transpose;
}

/*
Commented out.
Absolutely no idea what this thing is supposed to do.
To anyone who would enable this code: it still uses koscript calls - you need
to convert it to the new style prior to uncommenting.

// KCFunction: MULTIPLEOPERATIONS
KCValue func_multipleOP (valVector args, KCValueCalc *calc, FuncExtra *)
{
  if (gCell)
  {
    context.setValue( new KSValue( ((Interpreter *) context.interpreter() )->cell()->value().asFloat() ) );
    return true;
  }

  gCell = ((Interpreter *) context.interpreter() )->cell();

  QValueList<KSValue::Ptr>& args = context.value()->listValue();
  QValueList<KSValue::Ptr>& extra = context.extraData()->listValue();

  if ( !KSUtil::checkArgumentsCount( context, 5, "MULTIPLEOPERATIONS", true ) )
  {
    gCell = 0;
    return false;
  }

  // 0: cell must contain formula with double/int result
  // 0, 1, 2, 3, 4: must contain integer/double
  for (int i = 0; i < 5; ++i)
  {
    if ( !KSUtil::checkType( context, args[i], KSValue::DoubleType, true ) )
    {
      gCell = 0;
      return false;
    }
  }

  //  ((Interpreter *) context.interpreter() )->document()->emitBeginOperation();

  double oldCol = args[1]->doubleValue();
  double oldRow = args[3]->doubleValue();
  kDebug() <<"Old values: Col:" << oldCol <<", Row:" << oldRow;

  KCCell * cell;
  KCSheet * sheet = ((Interpreter *) context.interpreter() )->sheet();

  Point point( extra[1]->stringValue() );
  Point point2( extra[3]->stringValue() );
  Point point3( extra[0]->stringValue() );

  if ( ( args[1]->doubleValue() != args[2]->doubleValue() )
       || ( args[3]->doubleValue() != args[4]->doubleValue() ) )
  {
    cell = KCCell( sheet, point.pos.x(), point.pos.y() );
    cell->setValue( args[2]->doubleValue() );
    kDebug() <<"Setting value" << args[2]->doubleValue() <<" on cell" << point.pos.x()
              << ", " << point.pos.y() << endl;

    cell = KCCell( sheet, point2.pos.x(), point.pos.y() );
    cell->setValue( args[4]->doubleValue() );
    kDebug() <<"Setting value" << args[4]->doubleValue() <<" on cell" << point2.pos.x()
              << ", " << point2.pos.y() << endl;
  }

  KCCell * cell1 = KCCell( sheet, point3.pos.x(), point3.pos.y() );
  cell1->calc( false );

  double d = cell1->value().asFloat();
  kDebug() <<"KCCell:" << point3.pos.x() <<";" << point3.pos.y() <<" with value"
            << d << endl;

  kDebug() <<"Resetting old values";

  cell = KCCell( sheet, point.pos.x(), point.pos.y() );
  cell->setValue( oldCol );

  cell = KCCell( sheet, point2.pos.x(), point2.pos.y() );
  cell->setValue( oldRow );

  cell1->calc( false );

  // ((Interpreter *) context.interpreter() )->document()->emitEndOperation();

  context.setValue( new KSValue( (double) d ) );

  gCell = 0;
  return true;
}

*/

#include "MathModule.moc"
