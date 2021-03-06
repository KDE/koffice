/* This file is part of the KDE project
   Copyright (C) 2007 Sascha Pfau <MrPeacock@gmail.com>
   Copyright (C) 2002,2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>
   Copyright (C) 2002 Norbert Andres <nandres@web.de>
   Copyright (C) 1999-2000 Laurent Montel <montel@kde.org>
   Copyright (C) 1999-2000 Laurent Montel <montel@kde.org>
   Copyright (C) 1999-2000 Jonathan Singer <jsinger@genome.wi.mit.edu>
   Copyright (C) 1998-1999 Torben Weis <weis@kde.org>

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

// built-in financial functions

#include "FinancialModule.h"

#include "KCCalculationSettings.h"
#include "KCFunction.h"
#include "KCFunctionModuleRegistry.h"
#include "functions/helper.h"
#include "KCValueCalc.h"
#include "KCValueConverter.h"

#include <kcalendarsystem.h>
#include <KLocale>

#include <math.h>

using namespace KCells;

// prototypes (sorted)
KCValue func_accrint(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_accrintm(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_amordegrc(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_amorlinc(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_compound(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_continuous(valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_coupdaybs (valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_coupdays (valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_coupdaysnc (valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_coupncd (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_coupnum(valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_couppcd (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_cumipmt(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_cumprinc(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_db(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ddb(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_disc(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dollarde(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dollarfr(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_duration(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_duration_add(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_effective(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_euro(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_euroconvert(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_fv(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_fvschedule(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_fv_annuity(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_intrate(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ipmt(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_irr (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ispmt(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_level_coupon(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mduration(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_mirr(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_nominal(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_nper(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_npv(valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_oddfprice (valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_oddfyield (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_oddlprice(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_oddlyield(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_pmt(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ppmt(valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_price (valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_pricedisc (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_pricemat(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_pv(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_pv_annuity(valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_rate (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_received(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_rri(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sln(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_syd(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_tbilleq(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_tbillprice(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_tbillyield(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_vdb(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_xirr(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_xnpv(valVector args, KCValueCalc *calc, FuncExtra *);
// KCValue func_yield (valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_yielddisc(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_yieldmat(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_zero_coupon(valVector args, KCValueCalc *calc, FuncExtra *);


KCELLS_EXPORT_FUNCTION_MODULE("financial", FinancialModule)


FinancialModule::FinancialModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    f = new KCFunction("ACCRINT", func_accrint);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETACCRINT");
    f->setParamCount(6, 7);
    add(f);
    f = new KCFunction("ACCRINTM", func_accrintm);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETACCRINTM");
    f->setParamCount(3, 5);
    add(f);
    f = new KCFunction("AMORDEGRC", func_amordegrc);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETAMORDEGRC");
    f->setParamCount(6, 7);
    add(f);
    f = new KCFunction("AMORLINC", func_amorlinc);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETAMORLINC");
    f->setParamCount(6, 7);
    add(f);
    f = new KCFunction("COMPOUND", func_compound);
    f->setParamCount(4);
    add(f);
    f = new KCFunction("CONTINUOUS", func_continuous);
    f->setParamCount(3);
    add(f);
//   f = new KCFunction ("COUPDAYBS", func_coupdaybs);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYBS");
//   f->setParamCount (3, 4);
//   add(f);
//   f = new KCFunction ("COUPDAYS", func_coupdays);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYS");
//   f->setParamCount (3, 4);
//   add(f);
//   f = new KCFunction ("COUPDAYSNC", func_coupdaysnc);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPDAYSNC");
//   f->setParamCount (3, 4);
//   add(f);
//   f = new KCFunction ("COUPNCD", func_coupncd);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPNCD");
//   f->setParamCount (3, 4);
//   add(f);
    f = new KCFunction("COUPNUM", func_coupnum);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPNUM");
    f->setParamCount(3, 5);
    add(f);
//   f = new KCFunction ("COUPPCD", func_couppcd);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCOUPPCD");
//   f->setParamCount (3, 4);
//   add(f);
    f = new KCFunction("CUMIPMT", func_cumipmt);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCUMIPMT");
    f->setParamCount(6);
    add(f);
    f = new KCFunction("CUMPRINC", func_cumprinc);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETCUMPRINC");
    f->setParamCount(6);
    add(f);
    f = new KCFunction("DB", func_db);
    f->setParamCount(4, 5);
    add(f);
    f = new KCFunction("DDB", func_ddb);
    f->setParamCount(4, 5);
    add(f);
    f = new KCFunction("DISC", func_disc);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDISC");
    f->setParamCount(4, 5);
    add(f);
    f = new KCFunction("DOLLARDE", func_dollarde);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDOLLARDE");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("DOLLARFR", func_dollarfr);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDOLLARFR");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("DURATION", func_duration);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETDURATION");
    f->setParamCount(3);
    add(f);
    f = new KCFunction("DURATION_ADD", func_duration_add);
    f->setParamCount(5, 6);
    add(f);
    f = new KCFunction("EFFECT", func_effective);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEFFECT");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("EFFECTIVE", func_effective);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("EURO", func_euro);   // KCells-specific, Gnumeric-compatible
    f->setParamCount(1);
    add(f);
    f = new KCFunction("EUROCONVERT", func_euroconvert);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("FV", func_fv);
    f->setParamCount(3, 5);
    add(f);
    f = new KCFunction("FVSCHEDULE", func_fvschedule);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETFVSCHEDULE");
    f->setParamCount(2);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("FV_ANNUITY", func_fv_annuity);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("INTRATE", func_intrate);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETINTRATE");
    f->setParamCount(4, 5);
    add(f);
    f = new KCFunction("IPMT", func_ipmt);
    f->setParamCount(4, 6);
    add(f);
    f = new KCFunction ("IRR", func_irr);
    f->setParamCount (1, 2);
    f->setAcceptArray();
  add(f);
    f = new KCFunction("ISPMT", func_ispmt);
    f->setParamCount(4);
    add(f);
    f = new KCFunction("LEVEL_COUPON", func_level_coupon);
    f->setParamCount(5);
    add(f);
    f = new KCFunction("MDURATION", func_mduration);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETMDURATION");
    f->setParamCount(5, 6);
    add(f);
    f = new KCFunction("MIRR", func_mirr);
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("NOMINAL", func_nominal);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETNOMINAL");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("NPER", func_nper);
    f->setParamCount(3, 5);
    add(f);
    f = new KCFunction("NPV", func_npv);
    f->setParamCount(2, -1);
    f->setAcceptArray();
    add(f);
//   f = new KCFunction ("ODDFPRICE", func_oddfprice);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDFPRICE");
//   f->setParamCount (8, 9);
//   add(f);
//   f = new KCFunction ("ODDFYIELD", func_oddfyield);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDFYIELD");
//   f->setParamCount (8, 9);
//   add(f);
    f = new KCFunction("ODDLPRICE", func_oddlprice);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDLPRICE");
    f->setParamCount(7, 8);
    add(f);
    f = new KCFunction("ODDLYIELD", func_oddlyield);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETODDLYIELD");
    f->setParamCount(7, 8);
    add(f);
    f = new KCFunction("PDURATION", func_duration);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("PMT", func_pmt);
    f->setParamCount(3, 5);
    add(f);
    f = new KCFunction("PPMT", func_ppmt);
    f->setParamCount(4, 6);
    add(f);
//   f = new KCFunction ("PRICE", func_price);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICE");
//   f->setParamCount (6, 7);
//   add(f);
//   f = new KCFunction ("PRICEDISC", func_pricedisc);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICEDISC");
//   f->setParamCount (4, 5);
//   add(f);
    f = new KCFunction("PRICEMAT", func_pricemat);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETPRICEMAT");
    f->setParamCount(5, 6);
    add(f);
    f = new KCFunction("PV", func_pv);
    f->setParamCount(3, 5);
    add(f);
    f = new KCFunction("PV_ANNUITY", func_pv_annuity);
    f->setParamCount(3);
    add(f);
//   f = new KCFunction ("RATE", func_rate);
//   f->setParamCount (3, 6);
//   add(f);
    f = new KCFunction("RECEIVED", func_received);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETRECEIVED");
    f->setParamCount(4, 5);
    add(f);
    f = new KCFunction("RRI", func_rri);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("SLN", func_sln);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("SYD", func_syd);
    f->setParamCount(4);
    add(f);
    f = new KCFunction("TBILLEQ", func_tbilleq);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLEQ");
    f->setParamCount(3);
    add(f);
    f = new KCFunction("TBILLPRICE", func_tbillprice);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLPRICE");
    f->setParamCount(3);
    add(f);
    f = new KCFunction("TBILLYIELD", func_tbillyield);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETTBILLYIELD");
    f->setParamCount(3);
    add(f);
    f = new KCFunction("VDB", func_vdb);
    f->setParamCount(5, 7);
    add(f);
    f = new KCFunction("XIRR", func_xirr);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETXIRR");
    f->setParamCount(2, 3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("XNPV", func_xnpv);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETXNPV");
    f->setParamCount(3);
    f->setAcceptArray();
    add(f);
//   f = new KCFunction ("YIELD", func_yield);
//   f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELD");
//   f->setParamCount (6, 7);
//   add(f);
    f = new KCFunction("YIELDDISC", func_yielddisc);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELDDISC");
    f->setParamCount(4, 5);
    add(f);
    f = new KCFunction("YIELDMAT", func_yieldmat);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYIELDMAT");
    f->setParamCount(5, 6);
    add(f);
    f = new KCFunction("ZERO_COUPON", func_zero_coupon);
    f->setParamCount(3);
    add(f);
}

QString FinancialModule::descriptionFileName() const
{
    return QString("financial.xml");
}


///////////////////////////////////////////////////////////////////////////

//
// helper: awNpv
//
void awNpv(KCValueCalc *c, KCValue &res, KCValue val, KCValue rate)
{
    KCValue result = c->conv()->asFloat(res.element(0, 0));
    KCValue value = c->conv()->asFloat(val);
    KCValue i = c->conv()->asFloat(res.element(1, 0));
    res.setElement(0, 0, c->add(result, c->div(value, c->pow(c->add(KCValue(1.0), rate), i))));
    res.setElement(1, 0, c->add(i, KCValue(1.0)));     // increment counter
}

//
// helper: calc_fvifa
//
static KCValue calc_fvifa(KCValueCalc * /*calc*/, KCValue rate,
                        KCValue nper)
{
    KCValue res;

    if (rate.isZero())
        return (nper);
    else
        return KCValue(pow1pm1(rate.asFloat(), nper.asFloat()) / rate.asFloat());
}


//
// helper: getPay
//
static KCValue getPay(KCValueCalc *calc, KCValue rate,
                    KCValue nper, KCValue pv, KCValue fv, KCValue type)
{
    KCValue pvif, fvifa;

    // TODO This is for PMT. Check constraints of the other function using this
    //if (calc->isZero (rate)) return KCValue::errorVALUE();
    if (calc->isZero(rate))
        return calc->mul(calc->div(calc->add(pv, fv), nper), KCValue(-1.0));

    //pvif  = pow( 1 + rate, nper );
    //fvifa = ( pvif - 1 ) / rate;
    pvif = KCValue(pow1p(rate.asFloat(), nper.asFloat()));
//   fvifa = calc->div (calc->sub (pvif, 1), rate);
    fvifa = calc_fvifa(calc, rate, nper);

    // ( -pv * pvif - fv ) / ( ( 1.0 + rate * type ) * fvifa );
    KCValue val1 = calc->sub(calc->mul(calc->mul(KCValue(-1), pv), pvif), fv);
    KCValue val2 = calc->mul(calc->add(KCValue(1.0), calc->mul(rate, type)),
                           fvifa);


    kDebug() << "(1.0 + " << rate << "*" << type << ")*" << fvifa << ") =" << val2;
    return calc->div(val1, val2);
}

#if 0 // Not used?
//
// helper: getPrinc
//
static KCValue getPrinc(KCValueCalc *calc, KCValue start,
                      KCValue pay, KCValue rate, KCValue period)
{
    // val1 = pow( 1 + rate, period )
    KCValue val1 = calc->pow(calc->add(rate, 1), period);
    // val2 = start * val1
    KCValue val2 = calc->mul(start, val1);
    // val3 = pay * ( ( val1 - 1 ) / rate )
    KCValue val3 = calc->mul(pay, calc->div(calc->sub(val1, KCValue(1)), rate));
    // result = val2 + val3
    return calc->add(val2, val3);
}
#endif
//
// helper: eurofactor
//
static double helper_eurofactor(const QString& currency)
{
    QString cur = currency.toUpper();
    double result = -1;

    if (cur == "ATS") result = 13.7603;        // Austria
    else if (cur == "BEF") result = 40.3399;   // Belgium
    else if (cur == "DEM") result = 1.95583;   // Germany
    else if (cur == "ESP") result = 166.386;   // Spain
    else if (cur == "EUR") result = 1.0;       // Euro
    else if (cur == "FIM") result = 5.94573;   // Finland
    else if (cur == "FRF") result = 6.55957;   // France
    else if (cur == "GRD") result = 340.75;    // Greece
    else if (cur == "IEP") result = 0.787564;  // Ireland
    else if (cur == "ITL") result = 1936.27;   // Italy
    else if (cur == "LUX") result = 40.3399;   // Luxembourg
    else if (cur == "NLG") result = 2.20371;   // Netherlands
    else if (cur == "PTE") result = 200.482;   // Portugal

    return result;
}

//
// helper for IPMT and CUMIPMT is calculation of IPMT
//
static KCValue helper_ipmt(KCValueCalc* calc, KCValue rate, KCValue per, KCValue nper, KCValue pv, KCValue fv, KCValue type)
{
//     const KCValue payment = getPay (calc, rate, nper, pv, fv, type);
//     const KCValue ineg = getPrinc (calc, pv, payment, rate, calc->sub (per, KCValue(1)));
//     // -ineg * rate
//     return calc->mul (calc->mul (ineg, KCValue(-1)), rate);

    const KCValue pmt = getPay(calc, rate, nper, pv, fv, KCValue(0));  // Type 0

    // pow1p (rate, per-1)
    const KCValue val1(pow1p(rate.asFloat(), calc->sub(per, KCValue(1)).asFloat()));
    // pow1pm1 (rate, per-1)
    const KCValue val2(pow1pm1(rate.asFloat(), calc->sub(per, KCValue(1)).asFloat()));

    KCValue ipmt;
    // -1*(pv * pow1p(rate, per-1)*rate + pmt* pow1pm1(rate, per-1))
    ipmt = calc->mul(KCValue(-1), calc->add(calc->mul(calc->mul(pv, val1), rate), calc->mul(pmt, val2)));

    return (type.isZero()) ? ipmt : calc->div(ipmt, calc->add(KCValue(1), rate));
}


//
// helper vdbGetGDA
//
static double vdbGetGDA(const double cost, const double salvage, const double life,
                        const double period, const double depreciationFactor)
{
    double res, rate, oldCost, newCost;

    rate = depreciationFactor / life;
    if (rate >= 1.0) {
        rate = 1.0;
        if (period == 1.0)
            oldCost = cost;
        else
            oldCost = 0.0;
    } else
        oldCost = cost * pow(1.0 - rate, period - 1.0);

    newCost = cost * pow(1.0 - rate, period);

    if (newCost < salvage)
        res = oldCost - salvage;
    else
        res = oldCost - newCost;

    if (res < 0.0)
        res = 0.0;

    return (res);
}


//
// helper vdbInterVDB
//
static double vdbInterVDB(const double cost, const double salvage,
                          const double life, const double life1,
                          const double period, const double depreciationFactor)
{
    double res = 0.0;

    double intEnd         = ceil(period);
    unsigned long loopEnd = (unsigned long) intEnd;

    double term, lia = 0;
    double balance = cost - salvage;
    bool nowLia = false;
    double gda;
    unsigned long i;

    for (i = 1; i <= loopEnd; i++) {
        if (!nowLia) {
            gda = vdbGetGDA(cost, salvage, life, (double) i, depreciationFactor);
            lia = balance / (life1 - (double)(i - 1));

            if (lia > gda) {
                term = lia;
                nowLia = true;
            } else {
                term = gda;
                balance -= gda;
            }
        } else {
            term = lia;
        }

        if (i == loopEnd)
            term *= (period + 1.0 - intEnd);

        res += term;
    }

    return (res);
}


//
// helper: xirrResult
//
// args[0] = values
// args[1] = dates
//
static double xirrResult(valVector& args, KCValueCalc *calc, double& rate)
{
    QDate date;

    QDate date0 = calc->conv()->asDate(args[1].element(0)).asDate(calc->settings());

    double r = rate + 1.0;
    double res = calc->conv()->asFloat(args[0].element(0)).asFloat();

    for (int i = 1, count = args[0].count(); i < count; ++i) {
        date = calc->conv()->asDate(args[1].element(i)).asDate(calc->settings());
        double e_i = (date0.daysTo(date)) / 365.0;
        double val = calc->conv()->asFloat(args[0].element(i)).asFloat();

        res += val / pow(r, e_i);
    }

    return res;
}


//
// helper: xirrResultDerive
//
// args[0] = values
// args[1] = dates
//
static double xirrResultDerive(valVector& args, KCValueCalc *calc, double& rate)
{
    QDate date;

    QDate date0 = calc->conv()->asDate(args[1].element(0)).asDate(calc->settings());

    double r = rate + 1.0;
    double res = 0.0;

    for (int i = 1, count = args[0].count(); i < count; ++i) {
        date = calc->conv()->asDate(args[1].element(i)).asDate(calc->settings());
        double e_i = (date0.daysTo(date)) / 365.0;
        double val = calc->conv()->asFloat(args[0].element(i)).asFloat();

        res -= e_i * val / pow(r, e_i + 1.0);
    }

    return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////


//
// KCFunction: ACCRINT
//
KCValue func_accrint(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate maturity = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate firstInterest = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate settlement = calc->conv()->asDate(args[2]).asDate(calc->settings());

    KCValue rate = args[3];
    KCValue par = args[4];
    int frequency = calc->conv()->asInteger(args[5]).asInteger();

    int basis = 0;
    if (args.count() == 7)
        basis = calc->conv()->asInteger(args[6]).asInteger();

    if (basis < 0 || basis > 4 || (calc->isZero(KCValue(frequency))) ||
            (12 % frequency != 0))
        return KCValue::errorVALUE();

    if (maturity >= settlement) {
        kDebug() << "maturity >= settlement";
        return KCValue::errorVALUE();
    }

    double d = daysBetweenDates(maturity, settlement, basis);
    double y = daysPerYear(maturity, basis);

    if (d < 0 || y <= 0 || calc->lower(par, KCValue(0)) || calc->lower(rate, KCValue(0)) ||
            calc->isZero(rate))
        return KCValue::errorVALUE();

    KCValue coeff = calc->div(calc->mul(par, rate), frequency);
    double n = d / y;

    return calc->mul(coeff, n * frequency);
}


//
// KCFunction: ACCRINTM
//
KCValue func_accrintm(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate issue = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    KCValue rate = args[2];

    KCValue par = KCValue(1000);
    int basis = 0;
    if (args.count() > 3)
        par = args[3];
    if (args.count() == 5)
        basis = calc->conv()->asInteger(args[4]).asInteger();

    double d = daysBetweenDates(issue, maturity, basis);
    double y = daysPerYear(issue, basis);

    if (d < 0 || y <= 0 || calc->isZero(par) || calc->isZero(rate) ||
            calc->lower(par, KCValue(0)) || calc->lower(rate, KCValue(0)) || basis < 0 || basis > 4)
        return KCValue::errorVALUE();

    // par*date * d/y
    return calc->mul(calc->mul(par, rate), d / y);
}


//
// KCFunction: AMORDEGRC
//
// AMORDEGRC( Cost; purchaseDate; firstPeriodEndDate; salvage; period; basis)
//
KCValue func_amordegrc(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double cost = calc->conv()->asFloat(args[0]).asFloat();
    QDate purchaseDate = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate firstPeriodEndDate = calc->conv()->asDate(args[2]).asDate(calc->settings());
    double salvage = calc->conv()->asFloat(args[3]).asFloat();
    int period = calc->conv()->asInteger(args[4]).asInteger();
    double rate = calc->conv()->asFloat(args[5]).asFloat();

    int basis = 0;
    if (args.count() > 6)
        basis = calc->conv()->asInteger(args[6]).asInteger();

    int n;
    double amorCoeff, nRate, rest, usePer;

#define ROUND(x,y) (floor ((x) + 0.5))

    usePer = 1.0 / rate;

    if (usePer < 3.0)
        amorCoeff = 1.0;
    else if (usePer < 5.0)
        amorCoeff = 1.5;
    else if (usePer <= 6.0)
        amorCoeff = 2.0;
    else
        amorCoeff = 2.5;

    QDate date0 = calc->settings()->referenceDate(); // referenceDat

    rate *= amorCoeff;
    nRate = ROUND(yearFrac(date0, purchaseDate, firstPeriodEndDate, basis) * rate * cost, 0);
    cost -= nRate;
    rest = cost - salvage;

    for (n = 0 ; n < period ; n++) {
        nRate = ROUND(rate * cost, 0);
        rest -= nRate;

        if (rest < 0.0) {
            switch (period - n) {
            case 0:
            case 1:
                return KCValue(ROUND(cost * 0.5, 0));
            default:
                return KCValue(0.0);
            }
        }

        cost -= nRate;
    }

    return KCValue(nRate);
#undef ROUND
}

//
// KCFunction: AMORLINC
//
// AMORLINC( Cost; purchaseDate; firstPeriodEndDate; salvage; period; basis)
//
KCValue func_amorlinc(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double cost = calc->conv()->asFloat(args[0]).asFloat();
    QDate purchaseDate = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate firstPeriodEndDate = calc->conv()->asDate(args[2]).asDate(calc->settings());
    double salvage = calc->conv()->asFloat(args[3]).asFloat();
    int period = calc->conv()->asInteger(args[4]).asInteger();
    double rate = calc->conv()->asFloat(args[5]).asFloat();

    int basis = 0;
    if (args.count() > 6)
        basis = calc->conv()->asInteger(args[6]).asInteger();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    double oneRate = cost * rate;
    double costDelta = cost - salvage;
    double nullRate = yearFrac(date0, purchaseDate, firstPeriodEndDate, basis) * rate * cost;
    int numOfFullPeriods = (cost - salvage - nullRate) / oneRate;

    double res ;

    if (period == 0)
        res = nullRate;
    else if (period <= numOfFullPeriods)
        res = oneRate;
    else if (period == numOfFullPeriods + 1)
        res = costDelta - oneRate * numOfFullPeriods - nullRate;
    else
        res = 0.0;

    return KCValue(res);
#undef ROUND
}

//
// KCFunction: compound
//
// Returns value after compounded interest, given principal, rate, periods
// per year and year
//
KCValue func_compound(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue principal = args[0];
    KCValue interest = args[1];
    KCValue periods = args[2];
    KCValue years = args[3];

    // principal * pow(1+ (interest / periods), periods*years);
    KCValue base = calc->add(calc->div(interest, periods), 1);
    return calc->mul(principal, calc->pow(base, calc->mul(periods, years)));
}


//
// KCFunction: continuous
//
// Returns value after continuous compounding of interest, given principal,
// rate and years
//
KCValue func_continuous(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // If you still don't understand this, let me know!  ;-)  jsinger@leeta.net
    KCValue principal = args[0];
    KCValue interest = args[1];
    KCValue years = args[2];

    // principal * exp(interest * years)
    return calc->mul(principal, calc->exp(calc->mul(interest, years)));
}


//
// KCFunction: COUPNUM - taken from GNUMERIC
//
//
// COUPNUM ( settlement, maturity, freq, [ basis = 0 ], [ eom ] )
//
KCValue func_coupnum(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // dates and integers only - don't need high-precision for this
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    int   frequency = calc->conv()->asInteger(args[2]).asInteger();

    // defaults
    int   basis = 0;
    bool  eom   = true;

    if (args.count() > 3)
        basis = calc->conv()->asInteger(args[3]).asInteger();
    if (args.count() == 5)
        eom = calc->conv()->asBoolean(args[4]).asBoolean();

    if (basis < 0 || basis > 5 || (frequency == 0) || (12 % frequency != 0)
            || settlement.daysTo(maturity) <= 0)
        return KCValue::errorVALUE();

    double result;
    QDate cDate(maturity);

    int months = maturity.month() - settlement.month()
                 + 12 * (maturity.year() - settlement.year());

    cDate = calc->settings()->locale()->calendar()->addMonths(cDate, -months);

    if (eom && maturity.daysInMonth() == maturity.day()) {
        while (cDate.daysInMonth() != cDate.day())
            cDate.addDays(1);
    }

    if (settlement.day() >= cDate.day())
        --months;

    result = (1 + months / (12 / frequency));

    return KCValue(result);
}


//
// KCFunction: CUMIPMT
//
KCValue func_cumipmt(valVector args, KCValueCalc *calc, FuncExtra *)
{
    const KCValue rate = args[0];
    if (rate.asFloat() <= 0.0)
        return KCValue::errorVALUE();
    const KCValue nper = args[1];
    const int periods = nper.asInteger();
    if (periods <= 0)
        return KCValue::errorVALUE();
    const KCValue pv = args[2];
    if (pv.asFloat() <= 0.0)
        return KCValue::errorVALUE();
    const KCValue v1(calc->conv()->asInteger(args[3]));
    if (v1.isError())
        return KCValue::errorVALUE();
    const int start = v1.asInteger();
    if (start <= 0 || start > periods)
        return KCValue::errorVALUE();
    const KCValue v2(calc->conv()->asInteger(args[4]));
    if (v2.isError())
        return KCValue::errorVALUE();
    const int end = v2.asInteger();
    if (end < start || end > periods)
        return KCValue::errorVALUE();
    const KCValue type(calc->conv()->asInteger(args[5]));
    if (type.isError())
        return KCValue::errorVALUE();

    KCValue result(0.0);
    for (int per = start; per <= end; ++per)
        result = calc->add(result, helper_ipmt(calc, rate, KCValue(per), nper, pv, KCValue(0.0), type));

    return result;
}


//
// KCFunction: CUMPRINC
//
KCValue func_cumprinc(valVector args, KCValueCalc *calc, FuncExtra *)
{
    const KCValue rate = args[0];
    if (rate.asFloat() <= 0.0)
        return KCValue::errorVALUE();
    const KCValue nper = args[1];
    const int periods = nper.asInteger();
    if (periods <= 0)
        return KCValue::errorVALUE();
    const KCValue pv = args[2];
    if (pv.asFloat() <= 0.0)
        return KCValue::errorVALUE();
    const KCValue v1(calc->conv()->asInteger(args[3]));
    if (v1.isError())
        return KCValue::errorVALUE();
    const int start = v1.asInteger();
    if (start <= 0 || start > periods)
        return KCValue::errorVALUE();
    const KCValue v2(calc->conv()->asInteger(args[4]));
    if (v2.isError())
        return KCValue::errorVALUE();
    const int end = v2.asInteger();
    if (end <= 0 || end < start || end > periods)
        return KCValue::errorVALUE();
    const KCValue type(calc->conv()->asInteger(args[5]));
    if (type.isError())
        return KCValue::errorVALUE();

    const KCValue pay = getPay(calc, rate, nper, pv, KCValue(0.0), type);
    const KCValue cumipmt = func_cumipmt(args, calc, 0);

    return calc->sub(calc->mul(pay, KCValue(end - start + 1)), cumipmt);
}


//
// KCFunction: DB
//
// fixed-declining depreciation
//
KCValue func_db(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // This function doesn't support extended datatypes, it simply
    // converts everything to double - because it does quite a bit
    // of computing, and, well, I'm lazy to convert it all (Tomas)
    double cost = numToDouble(calc->conv()->toFloat(args[0]));
    double salvage = numToDouble(calc->conv()->toFloat(args[1]));
    double life = numToDouble(calc->conv()->toFloat(args[2]));
    double period = numToDouble(calc->conv()->toFloat(args[3]));
    double month = 12;
    if (args.count() == 5)
        month = numToDouble(calc->conv()->toFloat(args[4]));

    // sentinel check
    if (cost == 0 || life <= 0.0 || period == 0)
        return KCValue::errorNUM();

    if (calc->lower(calc->div(KCValue(salvage), KCValue(cost)), KCValue(0)))
        return KCValue::errorNUM();

    double rate = 1000 * (1 - pow((salvage / cost), (1 / life)));
    rate = floor(rate + 0.5)  / 1000;

    double total = cost * rate * month / 12;

    if (period == 1)
        return KCValue(total);

    for (int i = 1; i < life; ++i)
        if (i == period - 1)
            return KCValue(rate *(cost - total));
        else total += rate * (cost - total);

    return KCValue((cost - total) * rate *(12 - month) / 12);
}


//
// KCFunction: DDB
//
// depreciation per period
//
KCValue func_ddb(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double cost = numToDouble(calc->conv()->toFloat(args[0]));
    double salvage = numToDouble(calc->conv()->toFloat(args[1]));
    double life = numToDouble(calc->conv()->toFloat(args[2]));
    double period = numToDouble(calc->conv()->toFloat(args[3]));
    double factor = 2;
    if (args.count() == 5)
        factor = numToDouble(calc->conv()->toFloat(args[4]));

    if (cost < 0.0 || salvage < 0.0 || life <= 0.0 || period < 0.0 || factor < 0.0)
        return KCValue::errorVALUE();

    double result = 0.0;

    if (factor >= life)
        // special case: amazingly gigantic depreciating rate
        result = (period > 1) ? 0 : (cost < salvage) ? 0 : cost - salvage;
    else {
        // depreciation is the value between two periods
        double invrate = (life - factor) / life;
        double current = (period == 1) ? invrate : pow(invrate, period);
        double previous = current / invrate;
        current *= cost;
        previous *= cost;
        result = previous - current;

        // should not be more than the salvage
        if (current < salvage)
            result = previous - salvage;
    }

    // can't be negative
    if (result < 0.0)
        result = 0.0;

    return KCValue(result);
}


//
// KCFunction: DISC
//
KCValue func_disc(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());

// TODO sascha fix error value checking
//   // check dates
//   if ( settlement > maturity || )
//     return KCValue(false);

    KCValue par = args[2];
    KCValue redemp = args[3];

    // check parameters
    if (settlement > maturity || redemp.asFloat() <= 0.0 || par.asFloat() <= 0.0)
        return KCValue(false);

    int basis = 0;
    if (args.count() == 5)
        basis = calc->conv()->asInteger(args[4]).asInteger();

    /*  double y = daysPerYear (settlement, basis);
      double d = daysBetweenDates (settlement, maturity, basis);

      if ( y <= 0 || d <= 0 || basis < 0 || basis > 4 || calc->isZero (redemp) )
        return KCValue(false);*/

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    // res=(1-(price/redemption)/yearfrac)
    return KCValue((1.0 - par.asFloat() / redemp.asFloat()) / yearFrac(date0, settlement, maturity, basis));
}


//
// KCFunction: DOLLARDE
//
KCValue func_dollarde(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double dollarFrac = args[0].asFloat();
    double frac = calc->conv()->asInteger(args[1]).asInteger();

    if (frac <= 0)
        return KCValue::errorVALUE();

    double fl;
    double res = modf(dollarFrac, &fl);

    res /= frac;
    res *= pow(10.0, ceil(log10(frac)));
    res += fl;

    return KCValue(res);
}


//
// KCFunction: DOLLARFR
//
KCValue func_dollarfr(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double dollarFrac = args[0].asFloat();
    double frac = calc->conv()->asInteger(args[1]).asInteger();

    if (frac <= 0)
        return KCValue::errorVALUE();

    double fl;
    double res = modf(dollarFrac, &fl);

    res *= frac;
    res *= pow(10.0, -ceil(log10(frac)));
    res += fl;

    return KCValue(res);
}


//
// KCFunction: DURATION
//
//
// duration( rate, pv, fv )
//
KCValue func_duration(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue rate = args[0];
    KCValue pv   = args[1];
    KCValue fv   = args[2];

    if (!calc->greater(rate, KCValue(0.0)))
        return KCValue::errorVALUE();
    if (calc->isZero(fv) || calc->isZero(pv))
        return KCValue::errorDIV0();

    if (calc->lower(calc->div(fv, pv), KCValue(0)))
        return KCValue::errorVALUE();

    // log(fv / pv) / log(1.0 + rate)
    return calc->div(calc->ln(calc->div(fv, pv)),
                     calc->ln(calc->add(rate, KCValue(1.0))));
}


//
// KCFunction: DURATION
//
//
// duration( settlement, maturity, coup, yield, freq, [basis = 0] )
//
KCValue func_duration_add(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());

    double coup = numToDouble(calc->conv()->toFloat(args[2]));
    double yield = numToDouble(calc->conv()->toFloat(args[3]));
    int freq = calc->conv()->asInteger(args[4]).asInteger();

    int basis = 0;
    if (args.count() > 5)
        basis = calc->conv()->asInteger(args[3]).asInteger();

    // TODO add chk_freq
    if (coup < 0.0 || yield < 0.0)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    valVector param;
    param.append(args[0]);
    param.append(args[1]);
    param.append(args[4]);
    param.append(KCValue(basis));

    int numOfCoups = KCValue(func_coupnum(param, calc, 0)).asInteger();

    kDebug(36002) << "DURATION";
    kDebug(36002) << "numOfCoup =" << numOfCoups;


    return KCValue(duration(date0, settlement, maturity, coup, yield, freq, basis, numOfCoups));
}


//
// KCFunction: effective
//
KCValue func_effective(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // Returns effective interest rate given nominal rate and periods per year

    KCValue nominal = args[0];
    KCValue periods = args[1];

    // base = 1 + (nominal / periods)
    // result = pow (base, periods) - 1
    KCValue base = calc->add(calc->div(nominal, periods), 1);
    return calc->sub(calc->pow(base, periods), 1);
}


//
// KCFunction: EURO
//
KCValue func_euro(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QString currency = calc->conv()->asString(args[0]).asString();
    double result = helper_eurofactor(currency);
    if (result < 0)
        return KCValue::errorNUM();

    return KCValue(result);
}


//
// EUROCONVERT(number,source,target)
//
KCValue func_euroconvert(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double number = numToDouble(calc->conv()->toFloat(args[0]));
    QString source = calc->conv()->asString(args[1]).asString();
    QString target = calc->conv()->asString(args[2]).asString();

    double factor1 = helper_eurofactor(source);
    double factor2 = helper_eurofactor(target);

    if (factor1 < 0)
        return KCValue::errorNUM();
    if (factor2 < 0)
        return KCValue::errorNUM();

    double result = number * factor2 / factor1;

    return KCValue(result);
}


//
// KCFunction: FV
//
// Returns future value, given current value, interest rate and time
//
KCValue func_fv(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue rate = args[0];
    KCValue nper = args[1];
    KCValue pmt  = args[2];

    // defaults
    KCValue pv(0.0);
    int type = 0;

    if (args.count() > 3)
        pv = KCValue(calc->conv()->asFloat(args[3]).asFloat());

    if (args.count() == 5)
        type = calc->conv()->asInteger(args[4]).asInteger();

    //TODO check payType

    KCValue pvif = KCValue(pow1p(rate.asFloat(), nper.asFloat()));
    KCValue fvifa = calc_fvifa(calc, rate, nper);

    KCValue res(calc->mul(KCValue(-1), calc->add(calc->mul(pv, pvif), calc->mul(pmt, calc->mul(calc->add(KCValue(1), calc->mul(rate, type)), fvifa)))));

    return (res);
    // present * pow (1 + interest, periods)
//   return calc->mul (present, calc->pow (calc->add (interest, 1), periods));
}

//
// KCFunction: FVSCHEDULE
//
// Returns future value
//
KCValue func_fvschedule(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue pv       = args[0];
    KCValue schedule = args[1];

    int n = schedule.count();
    int i;

    KCValue v;
    KCValue res(pv);

    for (i = 0; i < n; ++i) {
        v = args[1].element(i);
        res = KCValue(calc->mul(res, calc->add(KCValue(1), v)));
    }

    return (res);
}


//
// KCFunction: FV_annnuity
//
KCValue func_fv_annuity(valVector args, KCValueCalc *calc, FuncExtra *)
{
    /* Returns future value of an annuity or cash flow, given payment, interest
       rate and periods */

    KCValue amount = args[0];
    KCValue interest = args[1];
    KCValue periods = args[2];

    // pw = pow (1 + interest, periods)
    // result = amount * ((pw - 1) / interest)
    KCValue pw = calc->pow(calc->add(interest, 1), periods);
    return calc->mul(amount, calc->div(calc->sub(pw, 1), interest));
}


//
// KCFunction: INTRATE
//
KCValue func_intrate(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());

    KCValue invest = args[2];
    KCValue redemption = args[3];

    int basis = 0;
    if (args.count() == 5)
        basis = calc->conv()->asInteger(args[4]).asInteger();

    double d = daysBetweenDates(settlement, maturity, basis);
    double y = daysPerYear(settlement, basis);

    if (d <= 0 || y <= 0 || calc->isZero(invest) || basis < 0 || basis > 4)
        return KCValue::errorVALUE();

    // (redemption - invest) / invest * (y / d)
    return calc->mul(calc->div(calc->sub(redemption, invest), invest), y / d);
}


//
// KCFunction: IPMT
//
KCValue func_ipmt(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue rate = args[0];
    KCValue per  = args[1];
    KCValue nper = args[2];
    KCValue pv   = args[3];

    KCValue fv = KCValue(0.0);
    KCValue type = KCValue(0);
    if (args.count() > 4) fv = args[4];
    if (args.count() == 6) type = args[5];

    return helper_ipmt(calc, rate, per, nper, pv, fv, type);
}

static double irrResult(KCValue sec, KCValueCalc *calc, double rate)
{
    double res = 0;
    for (unsigned i = 0; i < sec.count(); i++) {
        double val = calc->conv()->asFloat(sec.element(i)).asFloat();
        res += val / pow(1.0 + rate, double(i));
    }
    return res;
}

static double irrResultDerive(KCValue sec, KCValueCalc *calc, double rate)
{
    double res = 0;
    for (unsigned i = 0; i < sec.count(); i++) {
        double val = calc->conv()->asFloat(sec.element(i)).asFloat();
        res += -double(i) * val / pow(1.0 + rate, double(i + 1));
    }
    return res;
}

//
// KCFunction: IRR
//
KCValue func_irr(valVector args, KCValueCalc *calc, FuncExtra *)
{
    static const double maxEpsilon = 1e-10;
    static const int maxIter = 50;

    KCValue seq = args[0];

    double rate = 0.1;
    if (args.count() > 1) rate = calc->conv()->asFloat(args[1]).asFloat();

    bool contLoop;
    int i = 0;
    do {
        double newRate = rate - irrResult(seq, calc, rate) / irrResultDerive(seq, calc, rate);
        double rateEpsilon = fabs(newRate - rate);
        rate = newRate;
        contLoop = (rateEpsilon > maxEpsilon) && (fabs(rate) > maxEpsilon);
    } while (contLoop && (++i < maxIter));

    return KCValue(rate);
}

//
// KCFunction: ISPMT
//
KCValue func_ispmt(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue rate = args[0];
    KCValue per  = args[1];
    KCValue nper = args[2];
    KCValue pv   = args[3];

    if (calc->lower(per, KCValue(1)) || calc->greater(per, nper))
        return KCValue::errorVALUE();

    // d = -pv * rate
    KCValue d = calc->mul(calc->mul(pv, KCValue(-1)), rate);

    // d - (d / nper * per)
    return calc->sub(d, calc->mul(calc->div(d, nper), per));
}


//
// KCFunction: MDURATION
//
//
// duration( settlement, maturity, coup, yield, freq, [basis = 0] )
//
KCValue func_mduration(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());

    double coup = numToDouble(calc->conv()->toFloat(args[2]));
    double yield = numToDouble(calc->conv()->toFloat(args[3]));
    int freq = calc->conv()->asInteger(args[4]).asInteger();

    int basis = 0;
    if (args.count() > 5)
        basis = calc->conv()->asInteger(args[3]).asInteger();

    // TODO add chk_freq
    if (coup < 0.0 || yield < 0.0)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    valVector param;
    param.append(args[0]);
    param.append(args[1]);
    param.append(args[4]);
    param.append(KCValue(basis));

    int numOfCoups = KCValue(func_coupnum(param, calc, 0)).asInteger();

    double res = duration(date0, settlement, maturity, coup, yield, freq, basis, numOfCoups);
    res /= 1.0 + (yield / double(freq));

    return KCValue(res);
}


//
// KCFunction: MIRR
//
KCValue func_mirr(valVector args, KCValueCalc *calc, FuncExtra *)
{
    long double inv = calc->conv()->asFloat(args[1]).asFloat();
    long double reInvRate = calc->conv()->asFloat(args[2]).asFloat();

    long double npv_pos, npv_neg;
    KCValue v;
    int n = args[0].count();
    int i;

    for (i = 0, npv_pos = npv_neg = 0; i < n; ++i) {
        v = args[0].element(i);
        if (v.asFloat() >= 0)
            npv_pos += v.asFloat() / pow1p(reInvRate, i);
        else
            npv_neg += v.asFloat() / pow1p(inv, i);
    }

    if (npv_neg == 0 || npv_pos == 0 || reInvRate <= -1.0l)
        return KCValue::errorVALUE();

    long double res = std::pow((-npv_pos * pow1p(reInvRate, n)) / (npv_neg * (1 + reInvRate)), (1.0l / (n - 1))) - 1.0l;

    return KCValue(res);
}


//
// KCFunction: level_coupon
//
KCValue func_level_coupon(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // Returns effective interest rate given nominal rate and periods per year
    KCValue face = args[0];
    KCValue coupon_rate = args[1];
    KCValue coupon_year = args[2];
    KCValue years = args[3];
    KCValue market_rate = args[4];

    KCValue coupon, interest, pw, pv_annuity;
    // coupon = coupon_rate * face / coupon_year
    // interest = market_rate / coupon_year
    // pw = pow(1 + interest, years * coupon_year)
    // pv_annuity = (1 - 1 / pw) / interest
    // result = coupon * pv_annuity + face / pw
    coupon = calc->mul(coupon_rate, calc->div(face, coupon_year));
    interest = calc->div(market_rate, coupon_year);
    pw = calc->pow(calc->add(interest, KCValue(1)), calc->mul(years, coupon_year));
    pv_annuity = calc->div(calc->sub(KCValue(1), calc->div(KCValue(1), pw)), interest);
    return calc->add(calc->mul(coupon, pv_annuity), calc->div(face, pw));
}


//
// KCFunction: nominal
//
KCValue func_nominal(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue effective = args[0];
    KCValue periods = args[1];

    // sentinel checks
    if (calc->isZero(periods))
        return KCValue::errorDIV0();
    if (!calc->greater(periods, KCValue(0.0)))
        return KCValue::errorVALUE();
    if (calc->isZero(effective))
        return KCValue::errorVALUE();
    if (!calc->greater(effective, KCValue(0.0)))
        return KCValue::errorVALUE();

    // pw = pow (effective + 1, 1 / periods)
    // result = periods * (pw - 1);
    KCValue pw;
    pw = calc->pow(calc->add(effective, KCValue(1)), calc->div(KCValue(1), periods));
    return calc->mul(periods, calc->sub(pw, KCValue(1)));
}


//
// KCFunction: NPER
//
KCValue func_nper(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double rate = calc->conv()->asFloat(args[0]).asFloat();
    double pmt = calc->conv()->asFloat(args[1]).asFloat();
    double pv = calc->conv()->asFloat(args[2]).asFloat();

    // defaults
    double fv = 0.0;
    double type = 0;

    // opt. params
    if (args.count() > 3) fv = calc->conv()->asFloat(args[3]).asFloat();
    if (args.count() == 5) type = calc->conv()->asFloat(args[4]).asFloat();

    // if rate is 0, ther NPER solves this
    //   PV = -FV -( Payment*NPER )
    if (rate == 0.0)
        return KCValue(-(pv + fv) / pmt);

    if (type > 0)
        return KCValue(log(-(rate*fv - pmt*(1.0 + rate)) / (rate*pv + pmt*(1.0 + rate))) / log(1.0 + rate));
    else
        return KCValue(log(-(rate*fv - pmt) / (rate*pv + pmt)) / log(1.0 + rate));
}


//
// NPV
//
KCValue func_npv(valVector args, KCValueCalc* calc, FuncExtra*)
{
    KCValue result(KCValue::Array);
    result.setElement(0, 0, KCValue(0.0));     // actual result
    result.setElement(1, 0, KCValue(1.0));     // counter
    if (args.count() == 2) {
        KCValue vector = args[1]; // may be an array
        calc->arrayWalk(vector, result, awNpv, calc->conv()->asFloat(args[0]));
    } else {
        valVector vector = args.mid(1);
        calc->arrayWalk(vector, result, awNpv, calc->conv()->asFloat(args[0]));
    }
    return result.element(0, 0);
}

//
// ODDLPRICE
//
KCValue func_oddlprice(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // this implementation is not correct. disabling the function.
    // specifically, we cannot use yearfrac here
    // refer to gnumeric sources or something like
    // http://www.advsystems.com/products/TOOLKIT/finlibdocs/FinLib-05.htm
    // for correct formulas
    return KCValue::errorVALUE();

    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate last = calc->conv()->asDate(args[2]).asDate(calc->settings());
    double rate = calc->conv()->asFloat(args[3]).asFloat();
    double yield = calc->conv()->asFloat(args[4]).asFloat();
    double redemp = calc->conv()->asFloat(args[5]).asFloat();
    double freq = calc->conv()->asFloat(args[6]).asFloat();

    // opt. basis
    int basis = 0;
    if (args.count() > 7)
        basis = calc->conv()->asInteger(args[7]).asInteger();

//   kDebug(36002)<<"ODDLPRICE";
//   kDebug(36002)<<"settlement ="<<settlement<<" maturity="<<maturity<<" last="<<last<<" rate="<<rate<<" yield="<<yield<<" redemp="<<redemp<<" freq="<<freq<<" basis="<<basis;

    // TODO check frequency
    if (yield <= 0.0 || rate <= 0.0 || maturity <= settlement || settlement <= last)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    double dci  = yearFrac(date0, last, maturity, basis) * freq;
    double dsci = yearFrac(date0, settlement, maturity, basis) * freq;
    double ai = yearFrac(date0, last, settlement, basis) * freq;

    double res = redemp + dci * 100.0 * rate / freq;
    res /= dsci * yield / freq + 1.0;
    res -= ai * 100.0 * rate / freq;

    return KCValue(res);
}


//
// ODDLYIELD
//
KCValue func_oddlyield(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // this implementation is not correct. disabling the function.
    // specifically, we cannot use yearfrac here
    // refer to gnumeric sources or something like
    // http://www.advsystems.com/products/TOOLKIT/finlibdocs/FinLib-05.htm
    // for correct formulas
    return KCValue::errorVALUE();

    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate last = calc->conv()->asDate(args[2]).asDate(calc->settings());
    double rate = calc->conv()->asFloat(args[3]).asFloat();
    double price = calc->conv()->asFloat(args[4]).asFloat();
    double redemp = calc->conv()->asFloat(args[5]).asFloat();
    double freq = calc->conv()->asFloat(args[6]).asFloat();

    // opt. basis
    int basis = 0;
    if (args.count() > 7)
        basis = calc->conv()->asInteger(args[7]).asInteger();

//   kDebug(36002)<<"ODDLYIELD";
//   kDebug(36002)<<"settlement ="<<settlement<<" maturity="<<maturity<<" last="<<last<<" rate="<<rate<<" price="<<price<<" redemp="<<redemp<<" freq="<<freq<<" basis="<<basis;

    // TODO check frequency
    if (rate < 0.0 || price <= 0.0 || maturity <= settlement || settlement <= last)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    double dci  = yearFrac(date0, last, maturity, basis) * freq;
    double dsci = yearFrac(date0, settlement, maturity, basis) * freq;
    double ai = yearFrac(date0, last, settlement, basis) * freq;

    double res = redemp + dci * 100.0 * rate / freq;
    res /= price + ai * 100.0 * rate / freq;
    res --;
    res *= freq / dsci;

    return KCValue(res);
}


//
// KCFunction: PMT
//
KCValue func_pmt(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue rate = args[0];
    KCValue nper = args[1];
    KCValue pv   = args[2];
    KCValue fv = KCValue(0.0);
    KCValue type = KCValue(0);
    if (args.count() > 3) fv = args[3];
    if (args.count() == 5) type = args[4];

    return getPay(calc, rate, nper, pv, fv, type);
}


//
// KCFunction: PPMT
//
// Uses IPMT.
//
KCValue func_ppmt(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // Docs partly copied from OO.
    //
    // PPMT(Rate;Period;NPER;PV;FV;Type)
    //
    // Rate is the periodic interest rate.
    // Period is the amortizement period. P=1 for the first and P=NPER for the last period.
    // NPER is the total number of periods during which annuity is paid.
    // PV is the present value in the sequence of payments.
    // FV (optional) is the desired (future) value.
    // Type (optional) defines the due date. F=1 for payment at the beginning of a period and F=0 for payment at the end of a period.
    //

    KCValue rate = args[0];
    KCValue per  = args[1];
    KCValue nper = args[2];
    KCValue pv   = args[3];

    // defaults
    KCValue fv = KCValue(0.0);
    KCValue type = KCValue(0);

    if (args.count() > 4) fv = args[4];
    if (args.count() == 6) type = args[5];

    kDebug() << "Type=" << type;

    KCValue pay  = getPay(calc, rate, nper, pv, fv, type);
    KCValue ipmt = func_ipmt(args, calc, 0);
    return calc->sub(pay, ipmt);
}


//
// PRICEMAT
//
KCValue func_pricemat(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate issue = calc->conv()->asDate(args[2]).asDate(calc->settings());
    double rate = calc->conv()->asFloat(args[3]).asFloat();
    double yield = calc->conv()->asFloat(args[4]).asFloat();

    // opt. basis
    int basis = 0;
    if (args.count() > 5)
        basis = calc->conv()->asInteger(args[5]).asInteger();

    //kDebug(36002)<<"PRICEMAT";
    //kDebug(36002)<<"settlement ="<<settlement<<" maturity="<<maturity<<" issue="<<issue<<" rate="<<rate<<" yield="<<yield<<" basis="<<basis;

    if (rate < 0.0 || yield < 0.0 || settlement >= maturity)
        return KCValue::errorVALUE();

    long double y = daysPerYear(settlement, basis);
    if (!y) return KCValue::errorVALUE();
    long double issMat = daysBetweenDates(issue, maturity, basis) / y;
    long double issSet = daysBetweenDates(issue, settlement, basis) / y;
    long double setMat = daysBetweenDates(settlement, maturity, basis) / y;

    long double res = 1.0l + issMat * rate;
    res /= 1.0l + setMat * yield;
    res -= issSet * rate;
    res *= 100.0l;

    return KCValue(res);
}


//
// KCFunction: PV
//
// Returns present value, given future value, interest rate and years
//
KCValue func_pv(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double rate = calc->conv()->asFloat(args[0]).asFloat();
    double nper = calc->conv()->asFloat(args[1]).asFloat();
    double pmt = calc->conv()->asFloat(args[2]).asFloat();

    double fv = 0;
    int type = 0;

    if (args.count() > 3)
        fv = calc->conv()->asFloat(args[3]).asFloat();
    if (args.count() > 4)
        type = calc->conv()->asInteger(args[4]).asInteger();
    // TODO error KCValue checking for type


    double pvif = pow(1 + rate, nper);
    double fvifa = (pvif - 1) / rate;

    if (pvif == 0)
        return KCValue::errorDIV0();

    double res = (-fv - pmt * (1.0 + rate * type) * fvifa) / pvif;

    return KCValue(res);
}


//
// KCFunction: PV_annuity
//
KCValue func_pv_annuity(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue amount = args[0];
    KCValue interest = args[1];
    KCValue periods = args[2];

    // recpow = 1 / pow (1 + interest, periods)
    // result = amount * (1 - recpow) / interest;
    KCValue recpow;
    recpow = calc->div(KCValue(1), calc->pow(calc->add(interest, KCValue(1)), periods));
    return calc->mul(amount, calc->div(calc->sub(KCValue(1), recpow), interest));
}


//
// KCFunction: RECEIVED
//
KCValue func_received(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());

    KCValue investment = args[2];
    KCValue discount = args[3];

    int basis = 0;
    if (args.count() == 5)
        basis = calc->conv()->asInteger(args[4]).asInteger();

    double d = daysBetweenDates(settlement, maturity, basis);
    double y = daysPerYear(settlement, basis);

    if (d <= 0 || y <= 0 || basis < 0 || basis > 4)
        return KCValue(false);

    // 1.0 - ( discount * d / y )
    KCValue x = calc->sub(KCValue(1.0), (calc->mul(discount, d / y)));

    if (calc->isZero(x))
        return KCValue::errorVALUE();
    return calc->div(investment, x);
}


//
// KCFunction: RRI
//
KCValue func_rri(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double p = calc->conv()->asFloat(args[0]).asFloat();
    double pv = calc->conv()->asFloat(args[1]).asFloat();
    double fv = calc->conv()->asFloat(args[2]).asFloat();

    // constraints N>0
    if (p < 1)
        return KCValue::errorVALUE();

    double res = pow((fv / pv), 1 / p) - 1;
    return KCValue(res);
}


//
// KCFunction: SLN
//
// straight-line depreciation for a single period
//
KCValue func_sln(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue cost = args[0];
    KCValue salvage_value = args[1];
    KCValue life = args[2];

    // sentinel check
    if (!calc->greater(life, KCValue(0.0)))
        return KCValue::errorVALUE();

    // (cost - salvage_value) / life
    return calc->div(calc->sub(cost, salvage_value), life);
}


//
// KCFunction: SYD
//
// sum-of-years digits depreciation
//
KCValue func_syd(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue cost = args[0];
    KCValue salvage_value = args[1];
    KCValue life = args[2];
    KCValue period = args[3];

    // sentinel check
    if (!calc->greater(life, KCValue(0.0)))
        return KCValue::errorVALUE();

    // v1 = cost - salvage_value
    // v2 = life - period + 1
    // v3 = life * (life + 1.0)
    // result = (v1 * v2 * 2) / v3
    KCValue v1, v2, v3;
    v1 = calc->sub(cost, salvage_value);
    v2 = calc->add(calc->sub(life, period), 1);
    v3 = calc->mul(life, calc->add(life, 1.0));
    return calc->div(calc->mul(calc->mul(v1, v2), 2), v3);
}


//
// KCFunction: TBILLEQ
//
KCValue func_tbilleq(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    double discount = calc->conv()->asFloat(args[2]).asFloat();

    maturity = maturity.addDays(1);
    int days = days360(settlement, maturity, false);  // false -> US

    if (settlement >= maturity || discount <= 0.0 || days > 360)
        return KCValue::errorVALUE();

    double res = (365 * discount) / (360 - (discount * double(days)));

    return KCValue(res);
}


//
// KCFunction: TBILLPRICE
//
KCValue func_tbillprice(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    KCValue discount = args[2];

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    double fraction = yearFrac(date0, settlement, maturity.addDays(1), 0); // basis: USA 30/360
    double dummy;

    if (modf(fraction, &dummy) == 0.0)
        return KCValue::errorVALUE();

//   double days = settlement.daysTo( maturity );
// TODO error value checking
//   if (settlement > maturity || calc->lower (discount, KCValue(0)) || days > 265)
//     return KCValue::errorVALUE();

    return KCValue(100.0*(1.0 - discount.asFloat()*fraction));
}


//
// KCFunction: TBILLYIELD
//
KCValue func_tbillyield(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    double price = calc->conv()->asFloat(args[2]).asFloat();

    double days = days360(settlement, maturity, false);  // false -> US
    days++;

    if (settlement >= maturity || days > 360 || price <= 0.0)
        return KCValue::errorVALUE();

    double res = 100.0;
    res /= price;
    res--;
    res /= days;
    res *= 360.0;

    return KCValue(res);
}


//
// KCFunction: VDB
//
// VDB( cost; salvage; life; startPeriod; endPeriod [; depreciation-factor = 2 [; straight-line depreciation = TRUE ] )
//
KCValue func_vdb(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double cost = calc->conv()->asFloat(args[0]).asFloat();
    double salvage = calc->conv()->asFloat(args[1]).asFloat();
    double life = calc->conv()->asFloat(args[2]).asFloat();
    double startPeriod = calc->conv()->asFloat(args[3]).asFloat();
    double endPeriod = calc->conv()->asFloat(args[4]).asFloat();

    // defaults
    double depreciationFactor = 2;
    bool flag = false;

    // opt. parameter
    if (args.count() > 6)
        flag = calc->conv()->asInteger(args[6]).asInteger();
    if (args.count() >= 5)
        depreciationFactor = calc->conv()->asFloat(args[5]).asFloat();

    // check if parameters are valid
    if (cost < 0.0 || endPeriod < startPeriod || endPeriod > life || cost < 0.0 || salvage > cost || depreciationFactor <= 0.0)
        return KCValue::errorVALUE();

    // calc loop start and end
    double         intStart = floor(startPeriod);
    double         intEnd   = ceil(endPeriod);
    unsigned long loopStart = (unsigned long) intStart;
    unsigned long loopEnd   = (unsigned long) intEnd;

    double res = 0.0;

    if (flag) {
        // no straight-line depreciation
        for (unsigned long i = loopStart + 1; i <= loopEnd; i++) {
            double term = vdbGetGDA(cost, salvage, life, (double) i, depreciationFactor);

            //
            if (i == loopStart + 1)
                term *= (fmin(endPeriod, intStart + 1.0) - startPeriod);
            else if (i == loopEnd)
                term *= (endPeriod + 1.0 - intEnd);

            res += term;
        }
    } else {
        double life1 = life;
        double part;

        if (startPeriod != floor(startPeriod)) {
            if (depreciationFactor > 1) {
                if (startPeriod >= life / 2 || startPeriod == life / 2) {
                    part        = startPeriod - life / 2;
                    startPeriod = life / 2;
                    endPeriod  -= part;
                    life1      += 1;
                }
            }
        }

        cost -= vdbInterVDB(cost, salvage, life, life1, startPeriod, depreciationFactor);
        res   = vdbInterVDB(cost, salvage, life, life - startPeriod, endPeriod - startPeriod, depreciationFactor);
    } // end not flag

    return KCValue(res);
}


//
// KCFunction: XIRR
//
// Compute the internal rate of return for a non-periodic series of cash flows.
//
// XIRR ( Values; Dates; [ Guess = 0.1 ] )
//

// TODO sascha check dates
KCValue func_xirr(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double resultRate = 0.1;
    if (args.count() > 2)
        resultRate = calc->conv()->asFloat(args[2]).asFloat();

    // check pairs and count >= 2 and guess > -1.0
    if (args[0].count() != args[1].count() || args[1].count() < 2 || resultRate <= -1.0)
        return KCValue::errorVALUE();

    // define max epsilon
    static const double maxEpsilon = 1e-10;

    // max number of iterations
    static const int maxIter = 50;

    // Newton's method - try to find a res, with a accuracy of maxEpsilon
    double newRate, rateEpsilon, resultValue;
    int i = 0;
    bool contLoop;

    do {
        resultValue = xirrResult(args, calc, resultRate);
        newRate =  resultRate - resultValue / xirrResultDerive(args, calc, resultRate);
        rateEpsilon = fabs(newRate - resultRate);
        resultRate = newRate;
        contLoop = (rateEpsilon > maxEpsilon) && (fabs(resultValue) > maxEpsilon);
    } while (contLoop && (++i < maxIter));

    if (contLoop)
        return KCValue::errorVALUE();

    return KCValue(resultRate);
}

//
// KCFunction: xnpv
//
// Compute the net present value of a series of cash flows.
//
// XNPV ( Rate; Values; Dates )
//
KCValue func_xnpv(valVector args, KCValueCalc *calc, FuncExtra *)
{
    double rate = calc->conv()->asFloat(args[0]).asFloat();
    rate++;

    int numValues = args[1].count();
    int numDates  = args[2].count();

    double res = 0;

    // check pairs
    if (numValues != numDates || numValues < 2)
        return KCValue::errorVALUE();
    // check rate
    if (rate < -1.0)
        return KCValue::errorNUM();

    QDate date0 = calc->conv()->asDate(args[2].element(0)).asDate(calc->settings());
    double val;
    QDate date;

    for (int i = 0; i < numValues; ++i) {
        val  = calc->conv()->asFloat(args[1].element(i)).asFloat();
        KCValue tmpDate(calc->conv()->asDate(args[2].element(i)));

        if (tmpDate.isError()) return tmpDate;
        date = tmpDate.asDate(calc->settings());

        // check if date is valid
        if (!date.isValid())
            return KCValue::errorNUM();

        int days = date0.daysTo(date);

        res += val / pow(rate, days / 365.0);
    }

    return KCValue(res);
}


//
// YIELDDISC
//
KCValue func_yielddisc(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    double price = calc->conv()->asFloat(args[2]).asFloat();
    double redemp = calc->conv()->asFloat(args[3]).asFloat();

    // opt. basis
    int basis = 0;
    if (args.count() > 4)
        basis = calc->conv()->asInteger(args[4]).asInteger();

    if (price <= 0.0 || redemp <= 0.0 || settlement >= maturity)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    double res = (redemp / price) - 1.0;
    res /= yearFrac(date0, settlement, maturity, basis);

    return KCValue(res);
}


//
// YIELDMAT
//
KCValue func_yieldmat(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate settlement = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate maturity = calc->conv()->asDate(args[1]).asDate(calc->settings());
    QDate issue = calc->conv()->asDate(args[2]).asDate(calc->settings());
    long double rate = calc->conv()->asFloat(args[3]).asFloat();
    long double price = calc->conv()->asFloat(args[4]).asFloat();

    // opt. basis
    int basis = 0;
    if (args.count() > 5)
        basis = calc->conv()->asInteger(args[5]).asInteger();

    if (price <= 0.0 || rate <= 0.0 || settlement >= maturity)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    long double issMat = yearFrac(date0, issue, maturity, basis);
    long double issSet = yearFrac(date0, issue, settlement, basis);
    long double setMat = yearFrac(date0, settlement, maturity, basis);

    long double res = 1.0l + issMat * rate;
    res /= price / 100.0l + issSet * rate;
    res--;
    res /= setMat;

    return KCValue(res);
}

//
// KCFunction: zero_coupon
//
KCValue func_zero_coupon(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // Returns effective interest rate given nominal rate and periods per year

    KCValue face = args[0];
    KCValue rate = args[1];
    KCValue years = args[2];

    // face / pow(1 + rate, years)
    return calc->div(face, calc->pow(calc->add(rate, 1), years));
}

#include "FinancialModule.moc"
