/* This file is part of the KDE project
   Copyright (C) 1998-2003 The KCells Team <koffice-devel@kde.org>
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

// built-in date/time functions

#include "DateTimeModule.h"

#include "KCCalculationSettings.h"
#include "KCFunction.h"
#include "KCFunctionModuleRegistry.h"
#include "functions/helper.h"
#include "KCValueCalc.h"
#include "KCValueConverter.h"

#include <kcalendarsystem.h>
#include <KLocale>

using namespace KCells;

// prototypes, sorted
KCValue func_currentDate(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_currentDateTime(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_currentTime(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_date(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_date2unix(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dateDif(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_datevalue(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_day(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dayname(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_dayOfYear(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_days(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_days360(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_daysInMonth(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_daysInYear(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_easterSunday(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_edate(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_eomonth(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_hour(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_hours(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_isLeapYear(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_isoWeekNum(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_minute(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_minutes(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_month(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_monthname(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_months(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_networkday(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_second(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_seconds(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_time(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_timevalue(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_today(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_unix2date(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_weekday(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_weekNum(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_weeks(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_weeksInYear(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_workday(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_year(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_yearFrac(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_years(valVector args, KCValueCalc *calc, FuncExtra *);


KCELLS_EXPORT_FUNCTION_MODULE("datetime", DateTimeModule)


DateTimeModule::DateTimeModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    f = new KCFunction("CURRENTDATE",  func_currentDate);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("CURRENTDATETIME",  func_currentDateTime);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("CURRENTTIME",  func_currentTime);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("DATE",  func_date);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("DATE2UNIX",  func_date2unix);
    f->setParamCount(1);
    add(f);
    f = new KCFunction("DATEDIF",  func_dateDif);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("DATEVALUE",  func_datevalue);
    add(f);
    f = new KCFunction("DAY",  func_day);
    add(f);
    f = new KCFunction("DAYNAME",  func_dayname);
    add(f);
    f = new KCFunction("DAYOFYEAR",  func_dayOfYear);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("DAYS",  func_days);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("DAYS360",  func_days360);
    f->setParamCount(2, 3);
    add(f);
    f = new KCFunction("DAYSINMONTH",  func_daysInMonth);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINMONTH");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("DAYSINYEAR",  func_daysInYear);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDAYSINYEAR");
    add(f);
    f = new KCFunction("EASTERSUNDAY",  func_easterSunday);
    add(f);
    f = new KCFunction("EDATE",  func_edate);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEDATE");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("EOMONTH",  func_eomonth);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEOMONTH");
    f->setParamCount(2);
    add(f);
    f = new KCFunction("HOUR",  func_hour);
    f->setParamCount(0, 1);
    add(f);
    f = new KCFunction("HOURS",  func_hour);   // same as HOUR
    f->setParamCount(0, 1);
    add(f);
    f = new KCFunction("ISLEAPYEAR",  func_isLeapYear);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETISLEAPYEAR");
    add(f);
    f = new KCFunction("ISOWEEKNUM",  func_isoWeekNum);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("MINUTE",  func_minute);
    f->setParamCount(0, 1);
    add(f);
    f = new KCFunction("MINUTES",  func_minute);   // same as MINUTE
    f->setParamCount(0, 1);
    add(f);
    f = new KCFunction("MONTH",  func_month);
    add(f);
    f = new KCFunction("MONTHNAME",  func_monthname);
    add(f);
    f = new KCFunction("MONTHS",  func_months);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFMONTHS");
    f->setParamCount(3);
    add(f);
    f = new KCFunction("NETWORKDAY",  func_networkday);
    //f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETNETWORKDAYS");
    f->setParamCount(2, 3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("NOW",  func_currentDateTime);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("SECOND",  func_second);
    f->setParamCount(0, 1);
    add(f);
    f = new KCFunction("SECONDS",  func_second);   // same as SECOND
    f->setParamCount(0, 1);
    add(f);
    f = new KCFunction("TIME",  func_time);
    f->setParamCount(3);
    add(f);
    f = new KCFunction("TIMEVALUE",  func_timevalue);
    add(f);
    f = new KCFunction("TODAY",  func_currentDate);
    f->setParamCount(0);
    add(f);
    f = new KCFunction("UNIX2DATE",  func_unix2date);
    f->setParamCount(1);
    add(f);
    f = new KCFunction("WEEKDAY",  func_weekday);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("WEEKNUM",  func_weekNum);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWEEKNUM");
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("WEEKS",  func_weeks);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFWEEKS");
    f->setParamCount(3);
    add(f);
    f = new KCFunction("WEEKSINYEAR",  func_weeksInYear);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETWEEKSINYEAR");
    add(f);
    f = new KCFunction("WORKDAY",  func_workday);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETWORKDAY");
    f->setParamCount(2, 3);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("YEAR",   func_year);
    add(f);
    f = new KCFunction("YEARFRAC",  func_yearFrac);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETYEARFRAC");
    f->setParamCount(2, 3);
    add(f);
    f = new KCFunction("YEARS",  func_years);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETDIFFYEARS");
    f->setParamCount(3);
    add(f);
}

QString DateTimeModule::descriptionFileName() const
{
    return QString("datetime.xml");
}


// KCFunction: EDATE
KCValue func_edate(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date = calc->conv()->asDate(args[0]).asDate(calc->settings());
    int months = calc->conv()->asInteger(args[1]).asInteger();

    date = calc->settings()->locale()->calendar()->addMonths(date, months);

    if (!date.isValid())
        return KCValue::errorVALUE();

    return KCValue(date, calc->settings());
}

// KCFunction: EOMONTH
KCValue func_eomonth(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // add months to date using EDATE
    KCValue modDate = func_edate(args, calc, 0);
    if (modDate.isError()) return modDate;

    // modDate is currently in Date format
    QDate date = modDate.asDate(calc->settings());
    date.setYMD(date.year(), date.month(), date.daysInMonth());

    return KCValue(date, calc->settings());
}

// internal helper function
static int func_days360_helper(const QDate& _date1, const QDate& _date2, bool european)
{
    int day1, day2;
    int month1, month2;
    int year1, year2;
    bool negative = false;
    QDate date1(_date1);
    QDate date2(_date2);

    if (date1.daysTo(date2) < 0) {
        QDate tmp(date1);
        date1 = date2;
        date2 = tmp;
        negative = true;
    }

    day1   = date1.day();
    day2   = date2.day();
    month1 = date1.month();
    month2 = date2.month();
    year1  = date1.year();
    year2  = date2.year();

    if (european) {
        if (day1 == 31)
            day1 = 30;
        if (day2 == 31)
            day2 = 30;
    } else {
        // thanks to the Gnumeric developers for this...
        if (month1 == 2 && month2 == 2
                && date1.daysInMonth() == day1
                && date2.daysInMonth() == day2)
            day2 = 30;

        if (month1 == 2 && date1.daysInMonth() == day1)
            day1 = 30;

        if (day2 == 31 && day1 >= 30)
            day2 = 30;

        if (day1 == 31)
            day1 = 30;
    }

    return ((year2 - year1) * 12 + (month2 - month1)) * 30
           + (day2 - day1);
}

// KCFunction: DAYS360
// algorithm adapted from gnumeric
KCValue func_days360(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date1 = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate date2 = calc->conv()->asDate(args[1]).asDate(calc->settings());
    bool european = false;
    if (args.count() == 3)
        european = calc->conv()->asBoolean(args[2]).asBoolean();

    return KCValue(func_days360_helper(date1, date2, european));
}

// KCFunction: YEAR
KCValue func_year(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v = calc->conv()->asDate(args[0]);
    if (v.isError()) return v;
    QDate date = v.asDate(calc->settings());
    return KCValue(date.year());
}

// KCFunction: MONTH
KCValue func_month(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v = calc->conv()->asDate(args[0]);
    if (v.isError()) return v;
    QDate date = v.asDate(calc->settings());
    return KCValue(date.month());
}

// KCFunction: DAY
KCValue func_day(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v = calc->conv()->asDate(args[0]);
    if (v.isError()) return v;
    QDate date = v.asDate(calc->settings());
    return KCValue(date.day());
}

// KCFunction: HOUR
KCValue func_hour(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QTime time;
    if (args.count() == 1) {
        KCValue v = calc->conv()->asTime(args[0]);
        if (v.isError()) return v;
        time = v.asTime(calc->settings());
    } else
        time = QTime::currentTime();
    return KCValue(time.hour());
}

// KCFunction: MINUTE
KCValue func_minute(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QTime time;
    if (args.count() == 1) {
        KCValue v = calc->conv()->asTime(args[0]);
        if (v.isError()) return v;
        time = v.asTime(calc->settings());
    } else
        time = QTime::currentTime();
    return KCValue(time.minute());
}

// KCFunction: SECOND
KCValue func_second(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QTime time;
    if (args.count() == 1) {
        KCValue v = calc->conv()->asTime(args[0]);
        if (v.isError()) return v;
        time = v.asTime(calc->settings());
    } else
        time = QTime::currentTime();
    return KCValue(time.second() + qRound(time.msec() * 0.001));
}

// KCFunction: WEEKDAY
KCValue func_weekday(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v(calc->conv()->asDate(args[0]));
    if (v.isError()) return v;
    QDate date = v.asDate(calc->settings());
    int method = 1;
    if (args.count() == 2)
        method = calc->conv()->asInteger(args[1]).asInteger();

    if (method < 1 || method > 3)
        return KCValue::errorVALUE();

    int result = date.dayOfWeek();

    if (method == 3)
        --result;
    else if (method == 1) {
        ++result;
        if (result > 7) result = result % 7;
    }

    return KCValue(result);
}

// KCFunction: DATEVALUE
// same result would be obtained by applying number format on a date value
KCValue func_datevalue(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args[0].isString()) {
        KCValue v = calc->conv()->asDate(args[0]);
        if (! v.isError())
            return calc->conv()->asFloat(v);
    }
    return KCValue::errorVALUE();
}

// KCFunction: timevalue
// same result would be obtained by applying number format on a time value
KCValue func_timevalue(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args[0].isString()) {
        KCValue v = calc->conv()->asTime(args[0]);
        if (! v.isError())
            return calc->conv()->asFloat(v);
    }
    return KCValue::errorVALUE();
}

// KCFunction: YEARS
KCValue func_years(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date1 = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate date2 = calc->conv()->asDate(args[1]).asDate(calc->settings());
    if (!date1.isValid() || !date2.isValid())
        return KCValue::errorVALUE();

    int type = calc->conv()->asInteger(args[2]).asInteger();
    if (type == 0) {
        // max. possible years between both dates
        int years = date2.year() - date1.year();

        if (date2.month() < date1.month())
            --years;
        else if ((date2.month() == date1.month()) && (date2.day() < date1.day()))
            --years;

        return KCValue(years);
    }

    // type is non-zero now
    // the number of full years in between, starting on 1/1/XXXX
    if (date1.year() == date2.year())
        return KCValue(0);

    if ((date1.month() != 1) || (date1.day() != 1))
        date1.setYMD(date1.year() + 1, 1, 1);
    date2.setYMD(date2.year(), 1, 1);

    return KCValue(date2.year() - date1.year());
}

// KCFunction: MONTHS
KCValue func_months(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date1 = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate date2 = calc->conv()->asDate(args[1]).asDate(calc->settings());
    if (!date1.isValid() || !date2.isValid())
        return KCValue::errorVALUE();

    int type = calc->conv()->asInteger(args[2]).asInteger();
    if (type == 0) {
        int months  = (date2.year() - date1.year()) * 12;
        months += date2.month() - date1.month();

        if (date2.day() < date1.day())
            if (date2.day() != date2.daysInMonth())
                --months;

        return KCValue(months);
    }

    // type is now non-zero
    // the number of full months in between, starting on 1/XX/XXXX
    if (date1.month() == 12)
        date1.setYMD(date1.year() + 1, 1, 1);
    else
        date1.setYMD(date1.year(), date1.month() + 1, 1);
    date2.setYMD(date2.year(), date2.month(), 1);

    int months = (date2.year() - date1.year()) * 12;
    months += date2.month() - date1.month();

    return KCValue(months);
}

// KCFunction: WEEKS
KCValue func_weeks(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date1 = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate date2 = calc->conv()->asDate(args[1]).asDate(calc->settings());
    if (!date1.isValid() || !date2.isValid())
        return KCValue::errorVALUE();

    int type = calc->conv()->asInteger(args[2]).asInteger();
    int days = date1.daysTo(date2);
    if (type == 0)
        // just the number of full weeks between
        return KCValue((int)(days / 7));

    // the number of full weeks between starting on mondays
    int weekStartDay = calc->settings()->locale()->weekStartDay();

    int dow1 = date1.dayOfWeek();
    int dow2 = date2.dayOfWeek();

    days -= (7 + (weekStartDay % 7) - dow1);
    days -= ((dow2 - weekStartDay) % 7);

    return KCValue((int)(days / 7));
}

// KCFunction: DAYS
KCValue func_days(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date1 = calc->conv()->asDate(args[0]).asDate(calc->settings());
    QDate date2 = calc->conv()->asDate(args[1]).asDate(calc->settings());
    if (!date1.isValid() || !date2.isValid())
        return KCValue::errorVALUE();

    return KCValue(date2.daysTo(date1));
}

// KCFunction: DATE
KCValue func_date(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int y = calc->conv()->asInteger(args[0]).asInteger();
    int m = calc->conv()->asInteger(args[1]).asInteger();
    int d = calc->conv()->asInteger(args[2]).asInteger();

    if (m == 0 || d == 0)
        return KCValue::errorVALUE(); // month or day zero is not allowed
    else {
        QDate tmpDate(y, 1, 1);
        tmpDate = tmpDate.addMonths(m - 1);
        tmpDate = tmpDate.addDays(d - 1);

        //kDebug(36002) <<"func_date:: date =" << tmpDate;
        return KCValue(tmpDate, calc->settings());
    }
}

// KCFunction: DAY
KCValue func_dayname(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int number = calc->conv()->asInteger(args[0]).asInteger();

    QString weekName = calc->settings()->locale()->calendar()->weekDayName(number);
    if (weekName.isNull())
        return KCValue::errorVALUE();
    return KCValue(weekName);
}

// KCFunction: MONTHNAME
KCValue func_monthname(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int number = calc->conv()->asInteger(args[0]).asInteger();

    QString monthName = calc->settings()->locale()->calendar()->monthName(number,
                        QDate::currentDate().year());
    if (monthName.isNull())
        return KCValue::errorVALUE();
    return KCValue(monthName);
}

// KCFunction: TIME
KCValue func_time(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int h = calc->conv()->asInteger(args[0]).asInteger();
    int m = calc->conv()->asInteger(args[1]).asInteger();
    int s = calc->conv()->asInteger(args[2]).asInteger();

    QTime res;
    res = res.addSecs(60 * 60 * h);
    res = res.addSecs(60 * m);
    res = res.addSecs(s);

    return KCValue(res, calc->settings());
}

// KCFunction: CURRENTDATE
KCValue func_currentDate(valVector, KCValueCalc * calc, FuncExtra *)
{
    return KCValue(QDate::currentDate(), calc->settings());
}

// KCFunction: CURRENTTIME
KCValue func_currentTime(valVector, KCValueCalc * calc, FuncExtra *)
{
    return KCValue(QTime::currentTime(), calc->settings());
}

// KCFunction: CURRENTDATETIME
KCValue func_currentDateTime(valVector, KCValueCalc * calc, FuncExtra *)
{
    return KCValue(QDateTime::currentDateTime(), calc->settings());
}

// KCFunction: DAYOFYEAR
KCValue func_dayOfYear(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue date = func_date(args, calc, 0);
    if (date.isError()) return date;
    return KCValue(date.asDate(calc->settings()).dayOfYear());
}

// KCFunction: DAYSINMONTH
KCValue func_daysInMonth(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int y = calc->conv()->asInteger(args[0]).asInteger();
    int m = calc->conv()->asInteger(args[1]).asInteger();
    QDate date(y, m, 1);
    return KCValue(date.daysInMonth());
}

// KCFunction: ISLEAPYEAR
KCValue func_isLeapYear(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int y = calc->conv()->asInteger(args[0]).asInteger();
    return KCValue(QDate::isLeapYear(y));
}

// KCFunction: DAYSINYEAR
KCValue func_daysInYear(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int y = calc->conv()->asInteger(args[0]).asInteger();
    return KCValue(QDate::isLeapYear(y) ? 366 : 365);
}

// KCFunction: WEEKSINYEAR
KCValue func_weeksInYear(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int y = calc->conv()->asInteger(args[0]).asInteger();
    QDate date(y, 12, 31);   // last day of the year
    return KCValue(date.weekNumber());
}

// KCFunction: EASTERSUNDAY
KCValue func_easterSunday(valVector args, KCValueCalc *calc, FuncExtra *)
{
    int nDay, nMonth;
    int nYear = calc->conv()->asInteger(args[0]).asInteger();

    // (Tomas) the person who wrote this should be hanged :>
    int B, C, D, E, F, G, H, I, K, L, M, N, O;
    N = nYear % 19;
    B = int(nYear / 100);
    C = nYear % 100;
    D = int(B / 4);
    E = B % 4;
    F = int((B + 8) / 25);
    G = int((B - F + 1) / 3);
    H = (19 * N + B - D - G + 15) % 30;
    I = int(C / 4);
    K = C % 4;
    L = (32 + 2 * E + 2 * I - H - K) % 7;
    M = int((N + 11 * H + 22 * L) / 451);
    O = H + L - 7 * M + 114;
    nDay = O % 31 + 1;
    nMonth = int(O / 31);

    return KCValue(QDate(nYear, nMonth, nDay), calc->settings());
}

// KCFunction: ISOWEEKNUM
//
//              method  startday name of day
// default:     1       1        sunday
//              2       0        monday
//
KCValue func_isoWeekNum(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QDate date = calc->conv()->asDate(args[0]).asDate(calc->settings());
    if (!date.isValid())
        return KCValue::errorVALUE();

    int method = 2; // default method = 2
    if (args.count() > 1)
        method = calc->conv()->asInteger(args[1]).asInteger();

    if (method < 1 || method > 2)
        return KCValue::errorVALUE();

    int startday = 1;
    if (method != 1)
        startday = 0;

    int weeknum;
    int day;                   // current date
    int day4;                  // 4th of jan.
    int day0;                  // offset to 4th of jan.

    // date to find
    day = date.toJulianDay();

    // 4th of jan. of current year
    day4 = QDate(date.year(), 1, 4).toJulianDay();

    // difference in days to the 4th of jan including correction of startday
    day0 = QDate::fromJulianDay(day4 - 1 + startday).dayOfWeek();

    // do we need to count from last year?
    if (day < day4 - day0) { // recalculate day4 and day0
        day4 = QDate(date.year() - 1, 1, 4).toJulianDay(); // 4th of jan. last year
        day0 = QDate::fromJulianDay(day4 - 1 + startday).dayOfWeek();
    }

    // calc weeeknum
    weeknum = (day - (day4 - day0)) / 7 + 1;

    // if weeknum is greater 51, we have to do some extra checks
    if (weeknum >= 52) {
        day4 = QDate(date.year() + 1, 1, 4).toJulianDay(); // 4th of jan. next year
        day0 = QDate::fromJulianDay(day4 - 1 + startday).dayOfWeek();

        if (day >= day4 - day0) { // recalculate weeknum
            weeknum = (day - (day4 - day0)) / 7 + 1;
        }
    }

    return KCValue(weeknum);
}

// KCFunction: WEEKNUM
//
//   method  startday name of day
// default:  1  0  sunday
//  2 -1  monday
//
// weeknum = (startday + 7 + dayOfWeek of New Year + difference in days) / 7
//
KCValue func_weekNum(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v(calc->conv()->asDate(args[0]));
    if (v.isError()) return v;
    QDate date = v.asDate(calc->settings());

    if (!date.isValid())
        return KCValue::errorVALUE();

    int method = 1;
    if (args.count() > 1)
        method = calc->conv()->asInteger(args[1]).asInteger();

    if (method < 1 || method > 2)
        return KCValue::errorVALUE();

    QDate date1(date.year(), 1, 1);
    int days = date1.daysTo(date);

    int startday = 0;
    if (method == 2)
        startday = -1;

    int res = (int)((startday + 7 + date1.dayOfWeek() + days) / 7);

    if (date1.dayOfWeek() == 7 && method == 1)
        res--;

    //kDebug(36002) <<"weeknum = [startday(" << startday <<") + base(7) + New Year(" << date1.dayOfWeek() <<") + days(" << days <<")] / 7 =" << res;

    return KCValue(res);
}

// KCFunction: DATEDIF
//
// interval difference |  type descrition
// --------------------|----------------------------------
// default: m          |  months
//              d      |  days
//              y      |  complete years
//              ym     |  months excluding years
//              yd     |  days excluding years
//              md     |  days excluding months and years
//
KCValue func_dateDif(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v1(calc->conv()->asDate(args[0]));
    if (v1.isError()) return v1;
    QDate date1 = v1.asDate(calc->settings());

    if (!date1.isValid())
        return KCValue::errorVALUE();

    KCValue v2(calc->conv()->asDate(args[1]));
    if (v2.isError()) return v2;
    QDate date2 = v2.asDate(calc->settings());

    if (!date2.isValid())
        return KCValue::errorVALUE();

    // check if interval is valid
    QString interval = calc->conv()->asString(args[2]).asString();
    if (!(interval == "m" || interval == "d" || interval == "y" || interval == "ym" || interval == "yd" || interval == "md"))
        return KCValue::errorVALUE();

    // local vars
    int y, m, d;
    int sign = 1; // default
    int res = 0;

    QDate Temp1, Temp2;

    //QDate date0(1899,12,30); // referenceDate
    QDate date0 = calc->settings()->referenceDate();

    if (date2 < date1) {
        // exchange values and set sign
        Temp1 = date1;
        date1 = date2;
        date2 = Temp1;
        sign = -1;
    }

    //
    // calculate
    //

    // Temp1 = DateSerial(Year(Date2), Month(Date1), Day(Date1))
    Temp1.setDate(date2.year(), date1.month(), date1.day());

    // Y = Year(Date2) - Year(Date1) + (Temp1 > Date2)
    y = date2.year() - date1.year() + (date0.daysTo(Temp1) > date0.daysTo(date2) ? -1 : 0);

    // M = Month(Date2) - Month(Date1) - (12 * (Temp1 > Date2))
    m = date2.month() - date1.month() - (12 * (Temp1 > date2 ? -1 : 0));

    // D = Day(Date2) - Day(Date1)
    d = date2.day() - date1.day();

    if (d < 0) {
        // M = M - 1
        m--;
        // D = Day(DateSerial(Year(date2), Month(date2), 0)) + D
        Temp2.setDate(date2.year(), date2.month() - 1, 1);
        d = Temp2.daysInMonth() + d;
    }

    //
    // output
    //

    if (interval == "y") {
        // year
        res = y * sign;
    } else if (interval == "m") {
        // month
        res = (12 * y + m) * sign;
    } else if (interval == "d") {
        // days
        int days = date0.daysTo(date2) - date0.daysTo(date1);
        res = days * sign;
    } else if (interval == "ym") {
        // month excl. years
        res = m * sign;
    } else if (interval == "yd") {
        // days excl. years
        QDate Temp3(date2.year(), date1.month(), date1.day());
        int days = date0.daysTo(date2) - date0.daysTo(Temp3);

        res = days * sign;
    } else if (interval == "md") {
        // days excl. month and years
        res = d * sign;
    }
    return KCValue(res);
}

// KCFunction: YEARFRAC
//
//            | basis  |  descritption day-count
// -----------|--------|--------------------------------------------------------
// default:   |   0    |  US (NASD) system. 30 days/month, 360 days/year (30/360)
//            |   1    |  Actual/actual (Euro), also known as AFB
//            |   2    |  Actual/360
//            |   3    |  Actual/365
//            |   4    |  European 30/360
//
KCValue func_yearFrac(valVector args, KCValueCalc *calc, FuncExtra *)
{
    KCValue v1(calc->conv()->asDate(args[0]));
    if (v1.isError()) return v1;
    QDate date1 = v1.asDate(calc->settings());

    if (!date1.isValid())
        return KCValue::errorVALUE();

    KCValue v2(calc->conv()->asDate(args[1]));
    if (v2.isError()) return v2;
    QDate date2 = v2.asDate(calc->settings());

    if (!date2.isValid())
        return KCValue::errorVALUE();

    // check if basis is valid
    int basis = calc->conv()->asInteger(args[2]).asInteger();
    if (basis < 0 || basis > 4)
        return KCValue::errorVALUE();

    QDate date0 = calc->settings()->referenceDate(); // referenceDate

    return KCValue(yearFrac(date0, date1, date2, basis));
}

// KCFunction: WORKDAY
//
// - negative days count backwards
// - if holidays is not an array it is only added to days (neg. are not allowed)
//
KCValue func_workday(valVector args, KCValueCalc *calc, FuncExtra *e)
{
    KCValue v(calc->conv()->asDate(args[0]));

    if (v.isError()) return v;
    QDate startdate = v.asDate(calc->settings());

    if (!startdate.isValid())
        return KCValue::errorVALUE();

    //
    // vars
    //
    int days = calc->conv()->asInteger(args[1]).asInteger();

    QDate date0 = calc->settings()->referenceDate();   // referenceDate
    QDate enddate = startdate;                    // enddate
    valVector holidays;                           // stores holidays
    int sign = 1;                                 // sign 1 = forward, -1 = backward

    if (days < 0) {
        // change sign and set count to ccw
        days = days * -1;
        sign = -1;
    }

    //
    // check for holidays
    //
    if (args.count() > 2) {
        if (args[2].type() == KCValue::Array) { // parameter is array
            unsigned int row1, col1, rows, cols;

            row1 = e->ranges[2].row1;
            col1 = e->ranges[2].col1;
            rows = e->ranges[2].row2 - row1 + 1;
            cols = e->ranges[2].col2 - col1 + 1;

            KCValue holiargs = args[2];

            for (unsigned r = 0; r < rows; ++r) {
                for (unsigned c = 0; c < cols; ++c) {
                    // only append if element is a valid date
                    if (!holiargs.element(c + col1, r + row1).isEmpty()) {
                        KCValue v(calc->conv()->asDate(holiargs.element(c + col1, r + row1)));
                        if (v.isError())
                            return KCValue::errorVALUE();

                        if (v.asDate(calc->settings()).isValid())
                            holidays.append(v);
                    }
                } // cols
            }   // rows
        } else {
            // no array parameter
            if (args[2].isString()) {
                // isString
                KCValue v(calc->conv()->asDate(args[2]));
                if (v.isError())
                    return KCValue::errorVALUE();

                if (v.asDate(calc->settings()).isValid())
                    holidays.append(v);
            } else {
                // isNumber
                int hdays = calc->conv()->asInteger(args[2]).asInteger();

                if (hdays < 0)
                    return KCValue::errorVALUE();
                days = days + hdays;
            }
        }
    }

    //
    // count days
    //
    while (days) {
        // exclude weekends and holidays
        do {
            enddate = enddate.addDays(1 * sign);
        } while (enddate.dayOfWeek() > 5 || holidays.contains(KCValue(date0.daysTo(enddate))));

        days--;
    }

    return KCValue(enddate, calc->settings());
}

// KCFunction: NETWORKDAY
//
// - if holidays is not an array it is only added to days (neg. are not allowed)
//
KCValue func_networkday(valVector args, KCValueCalc *calc, FuncExtra *e)
{
    KCValue v1(calc->conv()->asDate(args[0]));

    if (v1.isError()) return v1;
    QDate startdate = v1.asDate(calc->settings());

    KCValue v2(calc->conv()->asDate(args[1]));

    if (v2.isError()) return v2;
    QDate enddate = v2.asDate(calc->settings());

    if (!startdate.isValid() || !enddate.isValid())
        return KCValue::errorVALUE();

    int days = 0;                                 // workdays
    QDate date0 = calc->settings()->referenceDate();   // referenceDate
    valVector holidays;                           // stores holidays
    int sign = 1;                                 // sign 1 = forward, -1 = backward

    if (enddate < startdate) {
        // change sign and set count to ccw
        sign = -1;
    }

    //
    // check for holidays
    //
    if (args.count() > 2) {
        if (args[2].type() == KCValue::Array) {
            // parameter is array
            unsigned int row1, col1, rows, cols;

            row1 = e->ranges[2].row1;
            col1 = e->ranges[2].col1;
            rows = e->ranges[2].row2 - row1 + 1;
            cols = e->ranges[2].col2 - col1 + 1;

            KCValue holiargs = args[2];

            for (unsigned r = 0; r < rows; ++r) {
                for (unsigned c = 0; c < cols; ++c) {
                    // only append if element is a valid date
                    if (!holiargs.element(c + col1, r + row1).isEmpty()) {
                        KCValue v(calc->conv()->asDate(holiargs.element(c + col1, r + row1)));
                        if (v.isError())
                            return KCValue::errorVALUE();

                        if (v.asDate(calc->settings()).isValid())
                            holidays.append(v);
                    }
                } // cols
            }   // rows
        } else {
            // no array parameter
            if (args[2].isString()) {
                KCValue v(calc->conv()->asDate(args[2]));
                if (v.isError())
                    return KCValue::errorVALUE();

                if (v.asDate(calc->settings()).isValid())
                    holidays.append(v);

            } else {
                // isNumber
                int hdays = calc->conv()->asInteger(args[2]).asInteger();

                if (hdays < 0)
                    return KCValue::errorVALUE();
                days = days - hdays;
            }
        }
    }

    //
    // count days
    //
    while (startdate != enddate) {
        if (startdate.dayOfWeek() > 5 || holidays.contains(KCValue(date0.daysTo(startdate)))) {
            startdate = startdate.addDays(1 * sign);
            continue;
        }

        startdate = startdate.addDays(1 * sign);
        days++;
    }
    return KCValue(days);
}

// KCFunction: DATE2UNIX
//
// Gnumeric docs says 01/01/2000 = 946656000
// TODO:
// - create FormatType mm/dd/yyyy hh:mm:ss
// - add method tryParseDateTime
KCValue func_unix2date(valVector args, KCValueCalc *calc, FuncExtra *)
{
    const KCValue v(calc->conv()->asInteger(args[0]));
    if (v.isError())
        return v;

    QDateTime datetime;
    datetime.setTimeSpec(Qt::UTC);
    datetime.setTime_t(v.asInteger());

    return KCValue(datetime, calc->settings());
}

// KCFunction: UNIX2DATE
KCValue func_date2unix(valVector args, KCValueCalc *calc, FuncExtra *)
{
    const KCValue v(calc->conv()->asDateTime(args[0]));
    if (v.isError())
        return v;

    const QDateTime datetime(v.asDateTime(calc->settings()));

    return KCValue(static_cast<int>(datetime.toTime_t()));
}

#include "DateTimeModule.moc"
