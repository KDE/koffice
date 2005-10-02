/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KSpread Team
                           www.koffice.org/kspread
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

// built-in financial functions

#include <math.h>

#include "functions.h"
#include "kspread_functions_helper.h"
#include "valuecalc.h"
#include "valueconverter.h"

#include <klocale.h>
#include <kcalendarsystem.h>

using namespace KSpread;

// prototypes (sorted)
KSpreadValue func_accrint (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_accrintm (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_compound (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_continuous (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_coupnum (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_db (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_ddb (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_disc (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_dollarde (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_dollarfr (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_duration (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_effective (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_euro (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_fv (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_fv_annuity (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_intrate (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_ipmt (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_ispmt (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_level_coupon (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_nominal (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_nper (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_pmt (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_ppmt (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_pv (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_pv_annuity (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_received (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_sln (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_syd (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_tbilleq (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_tbillprice (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_tbillyield (valVector args, ValueCalc *calc, FuncExtra *);
KSpreadValue func_zero_coupon (valVector args, ValueCalc *calc, FuncExtra *);

// registers all financial functions
void KSpreadRegisterFinancialFunctions()
{
  FunctionRepository* repo = FunctionRepository::self();
  Function *f;

  f = new Function ("ACCRINT", func_accrint);
  f->setParamCount (6, 7);
  repo->add (f);
  f = new Function ("ACCRINTM", func_accrintm);
  f->setParamCount (3, 5);
  repo->add (f);
  f = new Function ("COMPOUND", func_compound);
  f->setParamCount (4);
  repo->add (f);
  f = new Function ("CONTINUOUS", func_continuous);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("COUPNUM", func_coupnum);
  f->setParamCount (3, 5);
  repo->add (f);
  f = new Function ("DB", func_db);
  f->setParamCount (4, 5);
  repo->add (f);
  f = new Function ("DDB", func_ddb);
  f->setParamCount (4, 5);
  repo->add (f);
  f = new Function ("DISC", func_disc);
  f->setParamCount (4, 5);
  repo->add (f);
  f = new Function ("DOLLARDE", func_dollarde);
  f->setParamCount (2);
  repo->add (f);
  f = new Function ("DOLLARFR", func_dollarfr);
  f->setParamCount (2);
  repo->add (f);
  f = new Function ("DURATION", func_duration);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("EFFECT", func_effective);
  f->setParamCount (2);
  repo->add (f);
  f = new Function ("EFFECTIVE", func_effective);
  f->setParamCount (2);
  repo->add (f);
  f = new Function ("EURO", func_euro);  // KSpread-specific, Gnumeric-compatible
  f->setParamCount (1);
  repo->add (f);
  f = new Function ("FV", func_fv);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("FV_ANNUITY", func_fv_annuity);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("INTRATE", func_intrate);
  f->setParamCount (4, 5);
  repo->add (f);
  f = new Function ("IPMT", func_ipmt);
  f->setParamCount (4, 6);
  repo->add (f);
  f = new Function ("ISPMT", func_ispmt);
  f->setParamCount (4);
  repo->add (f);
  f = new Function ("LEVEL_COUPON", func_level_coupon);
  f->setParamCount (5);
  repo->add (f);
  f = new Function ("NOMINAL", func_nominal);
  f->setParamCount (2);
  repo->add (f);
  f = new Function ("NPER", func_nper);
  f->setParamCount (3, 5);
  repo->add (f);
  f = new Function ("PMT", func_pmt);
  f->setParamCount (3, 5);
  repo->add (f);
  f = new Function ("PPMT", func_ppmt);
  f->setParamCount (4, 6);
  repo->add (f);
  f = new Function ("PV", func_pv);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("PV_ANNUITY", func_pv_annuity);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("RECEIVED", func_received);
  f->setParamCount (4, 5);
  repo->add (f);
  f = new Function ("SLN", func_sln);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("SYD", func_syd);
  f->setParamCount (4);
  repo->add (f);
  f = new Function ("TBILLEQ", func_tbilleq);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("TBILLPRICE", func_tbillprice);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("TBILLYIELD", func_tbillyield);
  f->setParamCount (3);
  repo->add (f);
  f = new Function ("ZERO_COUPON", func_zero_coupon);
  f->setParamCount (3);
  repo->add (f);
}

static KSpreadValue getPay (ValueCalc *calc, KSpreadValue rate,
    KSpreadValue nper, KSpreadValue pv, KSpreadValue fv, KSpreadValue type)
{
  KSpreadValue pvif, fvifa;

  if (calc->isZero (rate)) return KSpreadValue::errorVALUE();

  //pvif  = pow( 1 + rate, nper );
  //fvifa = ( pvif - 1 ) / rate;
  pvif = calc->pow (calc->add (rate, 1), nper);
  fvifa = calc->div (calc->sub (pvif, 1), rate);

  // ( -pv * pvif - fv ) / ( ( 1.0 + rate * type ) * fvifa );
  KSpreadValue val1 = calc->sub (calc->mul (calc->mul (-1, pv), pvif), fv);
  KSpreadValue val2 = calc->mul (calc->add (1.0, calc->mul (rate, type)),
      fvifa);
  return calc->div (val1, val2);
}

static KSpreadValue getPrinc (ValueCalc *calc, KSpreadValue start,
    KSpreadValue pay, KSpreadValue rate, KSpreadValue period)
{
  // val1 = pow( 1 + rate, period )
  KSpreadValue val1 = calc->pow (calc->add (rate, 1), period);
  // val2 = start * val1
  KSpreadValue val2 = calc->mul (start, val1);
  // val3 = pay * ( ( val1 - 1 ) / rate )
  KSpreadValue val3 = calc->mul (pay, calc->div (calc->sub (val1, 1), rate));
  // result = val2 + val3
  return calc->add (val2, val3);
}

// Function: COUPNUM - taken from GNUMERIC
KSpreadValue func_coupnum (valVector args, ValueCalc *calc, FuncExtra *)
{
  // dates and integers only - don't need high-precision for this
  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();
  int   frequency = calc->conv()->asInteger (args[2]).asInteger();
  int   basis = 0;
  bool  eom   = true;
  if (args.count() > 3)
    basis = calc->conv()->asInteger (args[3]).asInteger();
  if (args.count() == 5)
    eom = calc->conv()->asBoolean (args[4]).asBoolean();

  if (basis < 0 || basis > 5 || ( frequency == 0 ) || ( 12 % frequency != 0 )
      || settlement.daysTo( maturity ) <= 0)
    return KSpreadValue::errorVALUE();

  double result;
  QDate cDate( maturity );

  int months = maturity.month() - settlement.month()
    + 12 * ( maturity.year() - settlement.year() );

  cDate = calc->conv()->locale()->calendar()->addMonths (cDate, -months);

  if ( eom && maturity.daysInMonth() == maturity.day() )
  {
    while( cDate.daysInMonth() != cDate.day() )
      cDate.addDays( 1 );
  }

  if ( settlement.day() >= cDate.day() )
    --months;

  result = ( 1 + months / ( 12 / frequency ) );

  return KSpreadValue (result);
}

// Function: ACCRINT
KSpreadValue func_accrint (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate maturity = calc->conv()->asDate (args[0]).asDate();
  QDate firstInterest = calc->conv()->asDate (args[1]).asDate();
  QDate settlement = calc->conv()->asDate (args[2]).asDate();
  
  KSpreadValue rate = args[3];
  KSpreadValue par = args[4];
  int frequency = calc->conv()->asInteger (args[5]).asInteger();

  int basis = 0;
  if (args.count() == 7)
    basis = calc->conv()->asInteger (args[6]).asInteger();
  
  if ( basis < 0 || basis > 4 || (calc->isZero (frequency)) ||
      (12 % frequency != 0))
    return KSpreadValue::errorVALUE();

  if ( ( settlement.daysTo( firstInterest ) < 0 )
       || ( firstInterest.daysTo( maturity ) > 0 ) )
    return KSpreadValue::errorVALUE();

  double d = daysBetweenDates (maturity, settlement, basis);
  double y = daysPerYear (maturity, basis);

  if ( d < 0 || y <= 0 || calc->lower (par, 0) || calc->lower (rate, 0) ||
      calc->isZero (rate))
    return KSpreadValue::errorVALUE();

  KSpreadValue coeff = calc->div (calc->mul (par, rate), frequency);
  double n = d / y;

  return calc->mul (coeff, n * frequency);
}

// Function: ACCRINTM
KSpreadValue func_accrintm (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate issue = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();
  KSpreadValue rate = args[2];
  
  KSpreadValue par = 1000;
  int basis = 0;
  if (args.count() > 3)
    par = args[3];
  if (args.count() == 5)
    basis = calc->conv()->asInteger (args[4]).asInteger ();

  double d = daysBetweenDates (issue, maturity, basis);
  double y = daysPerYear (issue, basis);

  if (d < 0 || y <= 0 || calc->isZero (par) || calc->isZero (rate) ||
      calc->lower (par, 0) || calc->lower (rate, 0) || basis < 0 || basis > 4)
    return KSpreadValue::errorVALUE();

  // par*date * d/y
  return calc->mul (calc->mul (par, rate), d / y);
}

// Function: DISC
KSpreadValue func_disc (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();

  KSpreadValue par = args[2];
  KSpreadValue redemp = args[3];
  
  int basis = 0;
  if (args.count() == 5)
    basis = calc->conv()->asInteger (args[4]).asInteger();

  double y = daysPerYear (settlement, basis);
  double d = daysBetweenDates (settlement, maturity, basis);

  if ( y <= 0 || d <= 0 || basis < 0 || basis > 4 || calc->isZero (redemp) )
    return false;

  // (redemp - par) / redemp * (y / d)
  return calc->mul (calc->div (calc->sub (redemp, par), redemp), y / d);
}


// Function: TBILLPRICE
KSpreadValue func_tbillprice (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();

  KSpreadValue discount = args[2];

  double days = settlement.daysTo( maturity );

  if (settlement > maturity || calc->lower (discount, 0) || days > 265)
    return KSpreadValue::errorVALUE();

  // (discount * days) / 360.0
  KSpreadValue val = calc->div (calc->mul (discount, days), 360.0);
  // 100 * (1.0 - val);
  return calc->mul (calc->sub (1.0, val), 100);
}

// Function: TBILLYIELD
KSpreadValue func_tbillyield (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();

  KSpreadValue rate = args[2];

  double days = settlement.daysTo( maturity );

  if (settlement > maturity || calc->isZero (rate) || calc->lower (rate, 0)
      || days > 265)
    return KSpreadValue::errorVALUE();

  // (100.0 - rate) / rate * (360.0 / days);
  return calc->mul (calc->div (calc->sub (100.0, rate), rate), 360.0 / days);
}

// Function: TBILLEQ
KSpreadValue func_tbilleq (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();

  KSpreadValue discount = args[2];

  double days = settlement.daysTo( maturity );

  if (settlement > maturity || calc->lower (discount, 0) || days > 265)
    return KSpreadValue::errorVALUE();

  // 360 - discount*days
  KSpreadValue divisor = calc->sub (360.0, calc->mul (discount, days));
  if (calc->isZero (divisor))
    return KSpreadValue::errorVALUE();

  // 365.0 * discount / divisor
  return calc->mul (calc->div (discount, divisor), 356.0);
}

// Function: RECEIVED
KSpreadValue func_received (valVector args, ValueCalc *calc, FuncExtra *)
{

  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();

  KSpreadValue investment = args[2];
  KSpreadValue discount = args[3];

  int basis = 0;
  if (args.count() == 5)
    basis = calc->conv()->asInteger (args[4]).asInteger();

  double d = daysBetweenDates( settlement, maturity, basis );
  double y = daysPerYear( settlement, basis );

  if ( d <= 0 || y <= 0 || basis < 0 || basis > 4 )
    return false;

  // 1.0 - ( discount * d / y )
  KSpreadValue x = calc->sub (1.0, (calc->mul (discount, d / y)));

  if (calc->isZero (x))
    return KSpreadValue::errorVALUE();
  return calc->div (investment, x);
}

// Function: DOLLARDE
KSpreadValue func_dollarde (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue d = args[0];
  KSpreadValue f = args[1];

  if (!calc->greater (f, 0))
    return KSpreadValue::errorVALUE();

  KSpreadValue tmp = f;
  int n = 0;
  while (calc->greater (tmp, 0))
  {
    tmp = calc->div (tmp, 10);
    ++n;
  }

  KSpreadValue fl = calc->roundDown (d);
  KSpreadValue r = calc->sub (d, fl);

  // fl + (r * pow(10.0, n) / f)
  return calc->add (fl, calc->div (calc->mul (r, pow (10.0, n)), f));
}

// Function: DOLLARFR
KSpreadValue func_dollarfr (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue d = args[0];
  KSpreadValue f = args[1];

  if (!calc->greater (f, 0))
    return KSpreadValue::errorVALUE();

  KSpreadValue tmp = f;
  int n = 0;
  while (calc->greater (tmp, 0))
  {
    tmp = calc->div (tmp, 10);
    ++n;
  }

  KSpreadValue fl = calc->roundDown (d);
  KSpreadValue r = calc->sub (d, fl);

  // fl + ((r * f) / pow (10.0, n));
  return calc->add (fl, calc->div (calc->mul (r, f), pow (10.0, n)));
}

/// *** TODO continue here ***

// Function: INTRATE
KSpreadValue func_intrate (valVector args, ValueCalc *calc, FuncExtra *)
{
  QDate settlement = calc->conv()->asDate (args[0]).asDate();
  QDate maturity = calc->conv()->asDate (args[1]).asDate();

  KSpreadValue invest = args[2];
  KSpreadValue redemption = args[3];

  int basis = 0;
  if (args.count() == 5)
    basis = calc->conv()->asInteger (args[4]).asInteger();

  double d = daysBetweenDates (settlement, maturity, basis);
  double y = daysPerYear (settlement, basis);

  if ( d <= 0 || y <= 0 || calc->isZero (invest) || basis < 0 || basis > 4 )
    return KSpreadValue::errorVALUE();

  // (redemption - invest) / invest * (y / d)
  return calc->mul (calc->div (calc->sub (redemption, invest), invest), y/d);
}


// Function: DURATION
KSpreadValue func_duration (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue rate = args[0];
  KSpreadValue pv   = args[1];
  KSpreadValue fv   = args[2];

  if (!calc->greater (rate, 0.0))
    return KSpreadValue::errorVALUE();
  if (calc->isZero (fv) || calc->isZero (pv))
    return KSpreadValue::errorDIV0();
  
  if (calc->lower (calc->div (fv, pv), 0))
    return KSpreadValue::errorVALUE();

  // log(fv / pv) / log(1.0 + rate)
  return calc->div (calc->ln (calc->div (fv, pv)),
      calc->ln (calc->add (rate, 1.0)));
}

// Function: PMT
KSpreadValue func_pmt (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue rate = args[0];
  KSpreadValue nper = args[1];
  KSpreadValue pv   = args[2];
  KSpreadValue fv = 0.0;
  KSpreadValue type = 0;
  if (args.count() > 3) fv = args[3];
  if (args.count() == 5) type = args[4];

  return getPay (calc, rate, nper, pv, fv, type);
}

// Function: NPER
KSpreadValue func_nper (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue rate = args[0];
  KSpreadValue pmt  = args[1];
  KSpreadValue pv   = args[2];
  KSpreadValue fv = 0.0;
  KSpreadValue type = 0;
  if (args.count() > 3) fv = args[3];
  if (args.count() == 5) type = args[4];

  if (!calc->greater (rate, 0.0))
    return KSpreadValue::errorVALUE();

  // taken from Gnumeric
  // v = 1.0 + rate * type
  // d1 = pmt * v - fv * rate
  // d2 = pmt * v - pv * rate
  // res = d1 / d2;
  KSpreadValue v = calc->add (calc->mul (rate, type), 1.0);
  KSpreadValue d1 = calc->sub (calc->mul (pmt, v), calc->mul (fv, rate));
  KSpreadValue d2 = calc->add (calc->mul (pmt, v), calc->mul (pv, rate));
  KSpreadValue res = calc->div (d1, d2);

  if (!calc->greater (res, 0.0))  // res must be >0
    return KSpreadValue::errorVALUE();

  // ln (res) / ln (rate + 1.0)
  return calc->div (calc->ln (res), calc->ln (calc->add (rate, 1.0)));
}

// Function: ISPMT
KSpreadValue func_ispmt (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue rate = args[0];
  KSpreadValue per  = args[1];
  KSpreadValue nper = args[2];
  KSpreadValue pv   = args[3];

  if (calc->lower (per, 1) || calc->greater (per, nper))
    return KSpreadValue::errorVALUE();

  // d = -pv * rate
  KSpreadValue d = calc->mul (calc->mul (pv, -1), rate);

  // d - (d / nper * per)
  return calc->sub (d, calc->mul (calc->div (d, nper), per));
}

// Function: IPMT
KSpreadValue func_ipmt (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue rate = args[0];
  KSpreadValue per  = args[1];
  KSpreadValue nper = args[2];
  KSpreadValue pv   = args[3];
  
  KSpreadValue fv = 0.0;
  KSpreadValue type = 0;
  if (args.count() > 4) fv = args[4];
  if (args.count() == 6) type = args[5];

  KSpreadValue payment = getPay (calc, rate, nper, pv, fv, type);
  KSpreadValue ineg = getPrinc (calc, pv, payment, rate, calc->sub (per, 1));

  // -ineg * rate
  return calc->mul (calc->mul (ineg, -1), rate);
}

// Function: PPMT
// Uses IPMT.
KSpreadValue func_ppmt (valVector args, ValueCalc *calc, FuncExtra *)
{
  /*
Docs partly copied from OO.
Syntax
PPMT(Rate;Period;NPER;PV;FV;Type)

Rate is the periodic interest rate.
Period is the amortizement period. P=1 for the first and P=NPER for the last period.
NPER is the total number of periods during which annuity is paid.
PV is the present value in the sequence of payments.
FV (optional) is the desired (future) value.
Type (optional) defines the due date. F=1 for payment at the beginning of a period and F=0 for payment at the end of a period.
  */

  KSpreadValue rate = args[0];
  KSpreadValue per  = args[1];
  KSpreadValue nper = args[2];
  KSpreadValue pv   = args[3];
  KSpreadValue fv = 0.0;
  KSpreadValue type = 0;
  if (args.count() > 4) fv = args[4];
  if (args.count() == 6) type = args[5];
  
  KSpreadValue pay  = getPay (calc, rate, nper, pv, fv, type);
  KSpreadValue ipmt = func_ipmt (args, calc, 0);
  return calc->sub (pay, ipmt);
}

// Function: FV
/* Returns future value, given current value, interest rate and time */
KSpreadValue func_fv (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue present = args[0];
  KSpreadValue interest = args[1];
  KSpreadValue periods = args[2];

  // present * pow (1 + interest, periods)
  return calc->mul (present, calc->pow (calc->add (interest, 1), periods));
}

// Function: compound
/* Returns value after compounded interest, given principal, rate, periods
per year and year */
 KSpreadValue func_compound (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue principal = args[0];
  KSpreadValue interest = args[1];
  KSpreadValue periods = args[2];
  KSpreadValue years = args[3];

  // principal * pow(1+ (interest / periods), periods*years);
  KSpreadValue base = calc->add (calc->div (interest, periods), 1);
  return calc->mul (principal, calc->pow (base, calc->mul (periods, years)));
}

// Function: continuous
/* Returns value after continuous compounding of interest, given principal,
rate and years */
KSpreadValue func_continuous (valVector args, ValueCalc *calc, FuncExtra *)
{
  // If you still don't understand this, let me know!  ;-)  jsinger@leeta.net
  KSpreadValue principal = args[0];
  KSpreadValue interest = args[1];
  KSpreadValue years = args[2];

  // principal * exp(interest * years)
  return calc->mul (principal, calc->exp (calc->mul (interest, years)));
}

// Function: PV
KSpreadValue func_pv (valVector args, ValueCalc *calc, FuncExtra *)
{
/* Returns presnt value, given future value, interest rate and years */
  KSpreadValue future = args[0];
  KSpreadValue interest = args[1];
  KSpreadValue periods = args[2];

  // future / pow(1+interest, periods)
  return calc->div (future, calc->pow (calc->add (interest, 1), periods));
}

// Function: PV_annuity
KSpreadValue func_pv_annuity (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue amount = args[0];
  KSpreadValue interest = args[1];
  KSpreadValue periods = args[2];
  
  // recpow = 1 / pow (1 + interest, periods)
  // result = amount * (1 - recpow) / interest;
  KSpreadValue recpow;
  recpow = calc->div (1, calc->pow (calc->add (interest, 1), periods));
  return calc->mul (amount, calc->div (calc->sub (1, recpow), interest));
}

// Function: FV_annnuity
KSpreadValue func_fv_annuity (valVector args, ValueCalc *calc, FuncExtra *)
{
  /* Returns future value of an annuity or cash flow, given payment, interest
     rate and periods */

  KSpreadValue amount = args[0];
  KSpreadValue interest = args[1];
  KSpreadValue periods = args[2];

  // pw = pow (1 + interest, periods)
  // result = amount * ((pw - 1) / interest)
  KSpreadValue pw = calc->pow (calc->add (interest, 1), periods);
  return calc->mul (amount, calc->div (calc->sub (pw, 1), interest));
}

// Function: effective
KSpreadValue func_effective (valVector args, ValueCalc *calc, FuncExtra *)
{
  // Returns effective interest rate given nominal rate and periods per year

  KSpreadValue nominal = args[0];
  KSpreadValue periods = args[1];

  // base = 1 + (nominal / periods)
  // result = pow (base, periods) - 1
  KSpreadValue base = calc->add (calc->div (nominal, periods), 1);
  return calc->sub (calc->pow (base, periods), 1);
}

// Function: zero_coupon
KSpreadValue func_zero_coupon (valVector args, ValueCalc *calc, FuncExtra *)
{
  // Returns effective interest rate given nominal rate and periods per year

  KSpreadValue face = args[0];
  KSpreadValue rate = args[1];
  KSpreadValue years = args[2];

  // face / pow(1 + rate, years)
  return calc->div (face, calc->pow (calc->add (rate, 1), years));
}

// Function: level_coupon
KSpreadValue func_level_coupon (valVector args, ValueCalc *calc, FuncExtra *)
{
  // Returns effective interest rate given nominal rate and periods per year
  KSpreadValue face = args[0];
  KSpreadValue coupon_rate = args[1];
  KSpreadValue coupon_year = args[2];
  KSpreadValue years = args[3];
  KSpreadValue market_rate = args[4];

  KSpreadValue coupon, interest, pw, pv_annuity;
  // coupon = coupon_rate * face / coupon_year
  // interest = market_rate / coupon_year
  // pw = pow(1 + interest, years * coupon_year)
  // pv_annuity = (1 - 1 / pw) / interest
  // result = coupon * pv_annuity + face / pw
  coupon = calc->mul (coupon_rate, calc->div (face, coupon_year));
  interest = calc->div (market_rate, coupon_year);
  pw = calc->pow (calc->add (interest, 1), calc->mul (years, coupon_year));
  pv_annuity = calc->div (calc->sub (1, calc->div (1, pw)), interest);
  return calc->add (calc->mul (coupon, pv_annuity), calc->div (face, pw));
}

// Function: nominal
KSpreadValue func_nominal (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue effective = args[0];
  KSpreadValue periods = args[1];

  if (calc->isZero (periods)) // Check null
    return KSpreadValue::errorDIV0();
  
  // pw = pow (effective + 1, 1 / periods)
  // result = periods * (pw - 1);
  KSpreadValue pw;
  pw = calc->pow (calc->add (effective, 1), calc->div (1, periods));
  return calc->mul (periods, calc->sub (pw, 1));
}

// Function: SLN
/* straight-line depreciation for a single period */
KSpreadValue func_sln (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue cost = args[0];
  KSpreadValue salvage_value = args[1];
  KSpreadValue life = args[2];

  // sentinel check
  if (!calc->greater (life, 0.0))
    return KSpreadValue::errorVALUE();

  // (cost - salvage_value) / life
  return calc->div (calc->sub (cost, salvage_value), life);
}

// Function: SYD
/* sum-of-years digits depreciation */
KSpreadValue func_syd (valVector args, ValueCalc *calc, FuncExtra *)
{
  KSpreadValue cost = args[0];
  KSpreadValue salvage_value = args[1];
  KSpreadValue life = args[2];
  KSpreadValue period = args[3];

  // sentinel check
  if (!calc->greater (life, 0.0))
    return KSpreadValue::errorVALUE();

  // v1 = cost - salvage_value
  // v2 = life - period + 1
  // v3 = life * (life + 1.0)
  // result = (v1 * v2 * 2) / v3
  KSpreadValue v1, v2, v3;
  v1 = calc->sub (cost, salvage_value);
  v2 = calc->add (calc->sub (life, period), 1);
  v3 = calc->mul (life, calc->add (life, 1.0));
  return calc->div (calc->mul (calc->mul (v1, v2), 2), v3);
}

// Function: DB
/* fixed-declining depreciation */
KSpreadValue func_db (valVector args, ValueCalc *calc, FuncExtra *)
{
  // This function doesn't support extended datatypes, it simply
  // converts everything to double - because it does quite a bit
  // of computing, and, well, I'm lazy to convert it all (Tomas)
  double cost = calc->conv()->asFloat (args[0]).asFloat();
  double salvage = calc->conv()->asFloat (args[1]).asFloat();
  double life = calc->conv()->asFloat (args[2]).asFloat();
  double period = calc->conv()->asFloat (args[3]).asFloat();
  double month = 12;
  if (args.count() == 5)
    month = calc->conv()->asFloat (args[4]).asFloat();

  // sentinel check
  if (cost == 0 || life <= 0.0)
    return KSpreadValue::errorVALUE ();

  if (calc->lower (calc->div (salvage, cost), 0))
    return KSpreadValue::errorVALUE ();

  double rate = 1000 * (1 - pow( (salvage/cost), (1/life) ));
  rate = floor( rate + 0.5 )  / 1000;

  double total = cost * rate * month / 12;

  if( period == 1 )
    return KSpreadValue (total);

  for (int i = 1; i < life; ++i)
    if (i == period - 1)
      return KSpreadValue (rate * (cost-total));
    else total += rate * (cost-total);

  return KSpreadValue ((cost-total) * rate * (12-month)/12);
}

// Function: DDB
/* depreciation per period */
KSpreadValue func_ddb (valVector args, ValueCalc *calc, FuncExtra *)
{
  double cost = calc->conv()->asFloat (args[0]).asFloat();
  double salvage = calc->conv()->asFloat (args[1]).asFloat();
  double life = calc->conv()->asFloat (args[2]).asFloat();
  double period = calc->conv()->asFloat (args[3]).asFloat();
  double factor = 12;
  if (args.count() == 5)
    factor = calc->conv()->asFloat (args[4]).asFloat();
  
  double total   = 0.0;

  if ( cost < 0.0 || salvage < 0.0 || life <= 0.0 || period < 0.0 || factor < 0.0 )
    return KSpreadValue::errorVALUE();

  for( int i = 0; i < life; ++i )
  {
    double periodDep = ( cost - total ) * ( factor / life );
    if ( i == period - 1 )
      return KSpreadValue (periodDep);
    else
      total += periodDep;
  }

  return KSpreadValue (cost - total - salvage);
}

// Function: EURO
KSpreadValue func_euro (valVector args, ValueCalc *calc, FuncExtra *)
{
  QString currency = calc->conv()->asString (args[0]).asString().upper();
  double result = -1;

  if( currency == "ATS" ) result = 13.7603;  // Austria
  else if( currency == "BEF" ) result = 40.3399;  // Belgium
  else if( currency == "DEM" ) result = 1.95583;  // Germany
  else if( currency == "ESP" ) result = 166.386;  // Spain
  else if( currency == "FIM" ) result = 5.94573;  // Finland
  else if( currency == "FRF" ) result = 6.55957;  // France
  else if( currency == "GRD" ) result = 340.75;   // Greece
  else if( currency == "IEP" ) result = 0.787564; // Ireland
  else if( currency == "ITL" ) result = 1936.27;  // Italy
  else if( currency == "LUX" ) result = 40.3399;  // Luxemburg
  else if( currency == "NLG" ) result = 2.20371;  // Nederland
  else if( currency == "PTE" ) result = 200.482;  // Portugal
  else
    return KSpreadValue::errorVALUE();

  return KSpreadValue (result);
}
