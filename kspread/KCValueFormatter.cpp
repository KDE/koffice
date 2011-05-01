/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>
   Copyright 1998-2004 KSpread Team <koffice-devel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KCValueFormatter.h"

#include "KCCalculationSettings.h"
#include "KCCell.h"
#include "KCLocalization.h"
#include "KCValueConverter.h"

#include <kcalendarsystem.h>
#include <kdebug.h>
#include <klocale.h>

#include <float.h>
#include <math.h>

KCValueFormatter::KCValueFormatter(const KCValueConverter* converter)
        : m_converter(converter)
{
}

const KCCalculationSettings* KCValueFormatter::settings() const
{
    return m_converter->settings();
}

KCValue KCValueFormatter::formatText(const KCValue &value, KCFormat::Type fmtType, int precision,
                                 KCStyle::FloatFormat floatFormat, const QString &prefix,
                                 const QString &postfix, const QString &currencySymbol, const QString &formatString)
{
    if (value.isError())
        return KCValue(value.errorMessage());

    //if we have an array, use its first element
    if (value.isArray())
        return formatText(value.element(0, 0), fmtType, precision,
                          floatFormat, prefix, postfix, currencySymbol, formatString);

    KCValue result;

    //step 1: determine formatting that will be used
    fmtType = determineFormatting(value, fmtType);

    //step 2: format the value !
    bool ok = false;

    //text
    if (fmtType == KCFormat::Text) {
        QString str = m_converter->asString(value).asString();
        if (!str.isEmpty() && str[0] == '\'')
            str = str.mid(1);
        result = KCValue(str);
        ok = true;
    }

    //datetime
    else if (fmtType == KCFormat::DateTime || (KCFormat::isDate(fmtType) && !formatString.isEmpty()) ) {
        KCValue dateValue = m_converter->asDateTime(value, &ok);
        if (ok) {
            result = KCValue(dateTimeFormat(dateValue.asDateTime(settings()), fmtType, formatString));
            result.setFormat(KCValue::fmt_DateTime);
        }
    }

    //
    else if (KCFormat::isDate(fmtType)) {
        KCValue dateValue = m_converter->asDate(value, &ok);
        if (ok) {
            result = KCValue(dateFormat(dateValue.asDate(settings()), fmtType, formatString));
            result.setFormat(KCValue::fmt_Date);
        }
    }

    //time
    else if (KCFormat::isTime(fmtType)) {
        KCValue timeValue = m_converter->asDateTime(value, &ok);
        if (ok) {
            result = KCValue(timeFormat(timeValue.asDateTime(settings()), fmtType, formatString));
            result.setFormat(KCValue::fmt_Time);
        }
    }

    //fraction
    else if (KCFormat::isFraction(fmtType)) {
        KCValue fractionValue = m_converter->asFloat(value, &ok);
        if (ok) {
            result = KCValue(fractionFormat(fractionValue.asFloat(), fmtType));
            result.setFormat(KCValue::fmt_Number);
        }
    }

    //another
    else {
        // complex
        if (value.isComplex()) {
            KCValue complexValue = m_converter->asComplex(value, &ok);
            if (ok) {
                result = KCValue(complexFormat(complexValue, precision, fmtType, floatFormat, currencySymbol));
                result.setFormat(KCValue::fmt_Number);
            }
        }

        // real number
        else {
            KCNumber number = m_converter->asFloat(value, &ok).asFloat();
            if (ok) {
                result = KCValue(createNumberFormat(number, precision, fmtType, floatFormat, currencySymbol, formatString));
                result.setFormat(KCValue::fmt_Number);
            }
        }
    }

    // Only string values can fail. If so, keep the string.
    if (!ok) {
        QString str = m_converter->asString(value).asString();
        if (!str.isEmpty() && str[0] == '\'')
            str = str.mid(1);
        result = KCValue(str);
    }

    if (!prefix.isEmpty())
        result = KCValue(prefix + ' ' + result.asString());

    if (!postfix.isEmpty())
        result = KCValue(result.asString() + ' ' + postfix);

    //kDebug() <<"KCValueFormatter says:" << str;
    return result;
}

KCFormat::Type KCValueFormatter::determineFormatting(const KCValue &value,
        KCFormat::Type fmtType)
{
    //now, everything depends on whether the formatting is Generic or not
    if (fmtType == KCFormat::Generic) {
        //here we decide based on value's format...
        KCValue::KCFormat fmt = value.format();
        switch (fmt) {
        case KCValue::fmt_None:
            fmtType = KCFormat::Text;
            break;
        case KCValue::fmt_Boolean:
            fmtType = KCFormat::Text;
            break;
        case KCValue::fmt_Number: {
            KCNumber val = fabs(value.asFloat());
            if (((val > 10000e+10) || (val < 10000e-10)) && (val != 0.0))
                fmtType = KCFormat::Scientific;
            else
                fmtType = KCFormat::KCNumber;
            }
            break;
        case KCValue::fmt_Percent:
            fmtType = KCFormat::Percentage;
            break;
        case KCValue::fmt_Money:
            fmtType = KCFormat::Money;
            break;
        case KCValue::fmt_DateTime:
            fmtType = KCFormat::DateTime;
            break;
        case KCValue::fmt_Date:
            fmtType = KCFormat::ShortDate;
            break;
        case KCValue::fmt_Time:
            fmtType = KCFormat::Time8; // [h]:mm
            break;
        case KCValue::fmt_String:
            //this should never happen
            fmtType = KCFormat::Text;
            break;
        };
        return fmtType;
    } else {
        //we'll mostly want to use the given formatting, the only exception
        //being Boolean values

        //TODO: is this correct? We may also want to convert bools to 1s and 0s
        //if we want to display a number...

        //TODO: what to do about Custom formatting? We don't support it as of now,
        //  but we'll have it ... one day, that is ...
        if (value.isBoolean())
            return KCFormat::Text;
        else
            return fmtType;
    }
}


QString KCValueFormatter::removeTrailingZeros(const QString& str, const QString& decimalSymbol)
{
    if (!str.contains(decimalSymbol))
        //no decimal symbol -> nothing to do
        return str;

    int start = 0;
    int cslen = m_converter->settings()->locale()->currencySymbol().length();
    if (str.indexOf('%') != -1)
        start = 2;
    else if (str.indexOf(m_converter->settings()->locale()->currencySymbol()) ==
             ((int)(str.length() - cslen)))
        start = cslen + 1;
    else if ((start = str.indexOf('E')) != -1)
        start = str.length() - start;
    else
        start = 0;

    QString result = str;
    int i = str.length() - start;
    bool bFinished = false;
    while (!bFinished && i > 0) {
        QChar ch = result[i - 1];
        if (ch == '0')
            result.remove(--i, 1);
        else {
            bFinished = true;
            if (result.mid(i - decimalSymbol.length(), decimalSymbol.length()) == decimalSymbol)
                result.remove(i - decimalSymbol.length(), decimalSymbol.length());
        }
    }
    return result;
}

QString KCValueFormatter::createNumberFormat(KCNumber value, int precision,
        KCFormat::Type fmt, KCStyle::FloatFormat floatFormat, const QString& currencySymbol, const QString& _formatString)
{
    QString prefix, postfix;
    QString formatString(_formatString);

    // try to split formatstring into prefix, formatstring and postfix.
    if (!formatString.isEmpty() ) {
        QRegExp re( QLatin1String( "^([^0#.,E+]*)([0#.,E+]*)(.*)$" ) );
        if( re.exactMatch( formatString ) ) {
            prefix = re.cap( 1 );
            formatString = re.cap( 2 );
            postfix = re.cap( 3 );
        }
        if (formatString.isEmpty()) {
            return prefix + postfix;
        } else if (formatString.contains(QLatin1Char('.'))) {
            precision = formatString.length() - formatString.indexOf(QLatin1Char('.')) - 1;
        } else if (precision != -1){
            precision = 0;
        }
    }

    int p = precision;
    if (p == -1) {
        // If precision (obtained from the cell style) is -1 (arbitrary), use the document default decimal precision
        // and if that value is -1 too then use either automatic decimal place adjustment or a hardcoded default.
        p = settings()->defaultDecimalPrecision();
        if (p == -1) {
            if (fmt == KCFormat::KCNumber) {
                QString s = QString::number(double(numToDouble(value)));
                int _p = s.indexOf('.');
                p = _p >= 0 ? qMax(0, 10 - _p) : 0;
            } else {
                p = 2; // hardcoded default
            }
        }
    }

    QString localizedNumber;
    int pos = 0;

    // Always unsigned ?
    if ((floatFormat == KCStyle::AlwaysUnsigned) && (value < 0.0))
        value *= -1.0;

    //multiply value by 100 for percentage format
    if (fmt == KCFormat::Percentage)
        value *= 100;

    // round the number, based on desired precision if not scientific is chosen
    //(scientific has relative precision)
    if (fmt != KCFormat::Scientific) {
        // this will avoid displaying negative zero, i.e "-0.0000"
        // TODO: is this really a good solution?
        if (fabs(value) < DBL_EPSILON) value = 0.0;

        double m[] = { 1, 10, 100, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10 };
        double mm = (p > 10) ? ::pow(10.0, p) : m[p];
        bool neg = value < 0;
        value = floor(numToDouble(fabs(value)) * mm + 0.5) / mm;
        if (neg) value = -value;
    }

    double val = numToDouble(value);
    switch (fmt) {
    case KCFormat::KCNumber:
        localizedNumber = m_converter->settings()->locale()->formatNumber(val, p);
        break;
    case KCFormat::Percentage:
        localizedNumber = m_converter->settings()->locale()->formatNumber(val, p);
        if(!postfix.endsWith('%')) // percent formattings needs to end with a "%"-sign
            postfix += '%';
        break;
    case KCFormat::Money:
        localizedNumber = m_converter->settings()->locale()->formatMoney(val, currencySymbol.isEmpty() ? m_converter->settings()->locale()->currencySymbol() : currencySymbol, p);
        break;
    case KCFormat::Scientific: {
        const QString decimalSymbol = m_converter->settings()->locale()->decimalSymbol();
        localizedNumber = QString::number(val, 'E', p);
        if ((pos = localizedNumber.indexOf('.')) != -1)
            localizedNumber = localizedNumber.replace(pos, 1, decimalSymbol);
        break;
    }
    default :
        //other formatting?
        // This happens with KCFormat::Custom...
        kDebug(36001) << "Wrong usage of KCValueFormatter::createNumberFormat fmt=" << fmt << "";
        break;
    }

    //prepend positive sign if needed
    if ((floatFormat == KCStyle::AlwaysSigned) && value >= 0)
        if (m_converter->settings()->locale()->positiveSign().isEmpty())
            localizedNumber = '+' + localizedNumber;

    // Remove trailing zeros and the decimal point if necessary
    // unless the number has no decimal point
    if (precision == -1) {
        QString decimalSymbol = m_converter->settings()->locale()->decimalSymbol();
        if (decimalSymbol.isNull())
            decimalSymbol = '.';

        localizedNumber = removeTrailingZeros(localizedNumber, decimalSymbol);
    }

    // remove negative sign if prefix already ends with '-'
    if (!prefix.isEmpty() && prefix[prefix.length()-1] == '-' && !localizedNumber.isEmpty() && localizedNumber[0] == '-') {
        localizedNumber = localizedNumber.mid(1);
    }

    return prefix + localizedNumber + postfix;
}

QString KCValueFormatter::fractionFormat(KCNumber value, KCFormat::Type fmtType)
{
    bool isNegative = value < 0;
    QString prefix = isNegative ? "-" : "";
    value = abs(value);
    KCNumber result = value - floor(numToDouble(value));
    int index;
    int limit = 0;

    /* return w/o fraction part if not necessary */
    if (result == 0)
        return prefix + QString::number((double) numToDouble(value));

    switch (fmtType) {
    case KCFormat::fraction_half:
        index = 2;
        break;
    case KCFormat::fraction_quarter:
        index = 4;
        break;
    case KCFormat::fraction_eighth:
        index = 8;
        break;
    case KCFormat::fraction_sixteenth:
        index = 16;
        break;
    case KCFormat::fraction_tenth:
        index = 10;
        break;
    case KCFormat::fraction_hundredth:
        index = 100;
        break;
    case KCFormat::fraction_one_digit:
        index = 3;
        limit = 9;
        break;
    case KCFormat::fraction_two_digits:
        index = 4;
        limit = 99;
        break;
    case KCFormat::fraction_three_digits:
        index = 5;
        limit = 999;
        break;
    default:
        kDebug(36001) << "Error in Fraction format";
        return prefix + QString::number((double) numToDouble(value));
        break;
    } /* switch */


    /* handle halves, quarters, tenths, ... */
    if (fmtType != KCFormat::fraction_three_digits
            && fmtType != KCFormat::fraction_two_digits
            && fmtType != KCFormat::fraction_one_digit) {
        KCNumber calc = 0;
        int index1 = 0;
        KCNumber diff = result;
        for (int i = 1; i <= index; i++) {
            calc = i * 1.0 / index;
            if (fabs(result - calc) < diff) {
                index1 = i;
                diff = fabs(result - calc);
            }
        }
        if (index1 == 0) return prefix + QString("%1").arg((double) floor(numToDouble(value)));
        if (index1 == index) return prefix + QString("%1").arg((double) floor(numToDouble(value)) + 1);
        if (floor(numToDouble(value)) == 0)
            return prefix + QString("%1/%2").arg(index1).arg(index);

        return prefix + QString("%1 %2/%3")
               .arg((double) floor(numToDouble(value)))
               .arg(index1)
               .arg(index);
    }


    /* handle KCFormat::fraction_one_digit, KCFormat::fraction_two_digit and KCFormat::fraction_three_digit style */
    double target = numToDouble(result);
    double numerator = 1;
    double denominator = 1;
    double bestNumerator = 0;
    double bestDenominator = 1;
    double bestDist = target;

    // as soon as either numerator or denominator gets above the limit, we're done
    while (numerator <= limit && denominator <= limit) {
        double dist = abs((numerator / denominator) - target);
        if (dist < bestDist) {
            bestDist = dist;
            bestNumerator = numerator;
            bestDenominator = denominator;
        }
        if (numerator / denominator > target) {
            denominator++;
        } else {
            numerator++;
        }
    }

    if (bestNumerator == 0)
        return prefix + QString().setNum((double) floor(numToDouble(value)));
    else if (bestDenominator == bestNumerator)
        return prefix + QString().setNum((double) floor(numToDouble(value + 1)));
    else {
        if (floor(numToDouble(value)) == 0)
            return prefix + QString("%1/%2").arg(bestNumerator).arg(bestDenominator);
        else
            return prefix + QString("%1 %2/%3")
                   .arg((double)floor(numToDouble(value)))
                   .arg(bestNumerator)
                   .arg(bestDenominator);
    }
}

QString KCValueFormatter::timeFormat(const QDateTime &_dt, KCFormat::Type fmtType, const QString& formatString)
{
    if (!formatString.isEmpty()) {
        return _dt.toString( formatString );
    }

    const QDateTime dt(_dt.toUTC());
    QString result;
    if (fmtType == KCFormat::Time)
        result = m_converter->settings()->locale()->formatTime(dt.time(), false);
    else if (fmtType == KCFormat::SecondeTime)
        result = m_converter->settings()->locale()->formatTime(dt.time(), true);
    else {
        const int d = settings()->referenceDate().daysTo(dt.date());
        int h, m, s;
        if (fmtType != KCFormat::Time6 && fmtType != KCFormat::Time7 && fmtType != KCFormat::Time8) { // time
            h = dt.time().hour();
            m = dt.time().minute();
            s = dt.time().second();
        } else if (d >= 0) { // positive duration
            h = dt.time().hour() + 24 * d;
            m = dt.time().minute();
            s = dt.time().second();
        } else { // negative duration
            s = (60 - dt.time().second()) % 60;
            m = (60 - dt.time().minute() - ((s == 0) ? 0 : 1)) % 60;
            h = -(dt.time().hour() + 24 * d) - ((m == 0 && s == 0) ? 0 : 1);
        }
        const bool pm = (h > 12);
        const QString sign = d < 0 ? QString('-') : QString("");

        if (fmtType == KCFormat::Time1) {  // 9:01 AM
            result = QString("%1:%2 %3")
                     .arg(QString::number(pm ? h - 12 : h), 1)
                     .arg(QString::number(m), 2, '0')
                     .arg(pm ? i18n("PM") : i18n("AM"));
        } else if (fmtType == KCFormat::Time2) { // 9:01:05 AM
            result = QString("%1:%2:%3 %4")
                     .arg(QString::number(pm ? h - 12 : h), 1)
                     .arg(QString::number(m), 2, '0')
                     .arg(QString::number(s), 2, '0')
                     .arg(pm ? i18n("PM") : i18n("AM"));
        } else if (fmtType == KCFormat::Time3) { // 9 h 01 min 28 s
            result = QString("%1 %2 %3 %4 %5 %6")
                     .arg(QString::number(h), 2, '0')
                     .arg(i18n("h"))
                     .arg(QString::number(m), 2, '0')
                     .arg(i18n("min"))
                     .arg(QString::number(s), 2, '0')
                     .arg(i18n("s"));
        } else if (fmtType == KCFormat::Time4) { // 9:01
            result = QString("%1:%2")
                     .arg(QString::number(h), 1)
                     .arg(QString::number(m), 2, '0');
        } else if (fmtType == KCFormat::Time5) { // 9:01:12
            result = QString("%1:%2:%3")
                     .arg(QString::number(h), 1)
                     .arg(QString::number(m), 2, '0')
                     .arg(QString::number(s), 2, '0');
        } else if (fmtType == KCFormat::Time6) { // [mm]:ss
            result = sign + QString("%1:%2")
                     .arg(QString::number(m + h * 60), 2, '0')
                     .arg(QString::number(s), 2, '0');
        } else if (fmtType == KCFormat::Time7) { // [h]:mm:ss
            result = sign + QString("%1:%2:%3")
                     .arg(QString::number(h), 1)
                     .arg(QString::number(m), 2, '0')
                     .arg(QString::number(s), 2, '0');
        } else if (fmtType == KCFormat::Time8) { // [h]:mm
            result = sign + QString("%1:%2")
                     .arg(QString::number(h), 1)
                     .arg(QString::number(m), 2, '0');
        }
    }
    return result;
}

QString KCValueFormatter::dateTimeFormat(const QDateTime &_dt, KCFormat::Type fmtType, const QString& formatString )
{
    if( !formatString.isEmpty() ) {
        if (formatString.contains("X")) {               // if we have the special extra-short month in the format string
            int monthPos = formatString.indexOf("X");
            QString before = formatString.left(monthPos);                               // get string before and after the extra-short month sign
            QString after = formatString.right(formatString.size() - monthPos - 1);
            QString monthShort = _dt.toString("MMM").left(1);                           // format the month as extra-short (only 1st letter)
            return _dt.toString( before ) + monthShort + _dt.toString( after );         // and construct the final date
        }

        return _dt.toString( formatString );
    }

    Q_UNUSED(fmtType);
    QString result;
    // pretty lame, just asssuming something for the format
    // TODO: locale-aware formatting
    result += dateFormat(_dt.date(), KCFormat::ShortDate) + ' ' + timeFormat(_dt, KCFormat::Time1);
    return result;
}

QString KCValueFormatter::dateFormat(const QDate &date, KCFormat::Type fmtType, const QString& formatString )
{
    if( !formatString.isEmpty() ) {
        return date.toString( formatString );
    }

    QString tmp;
    if (fmtType == KCFormat::ShortDate) {
        tmp = m_converter->settings()->locale()->formatDate(date, KLocale::ShortDate);
    } else if (fmtType == KCFormat::TextDate) {
        tmp = m_converter->settings()->locale()->formatDate(date, KLocale::LongDate);
    } else if (fmtType == KCFormat::Date1) { /*18-Feb-99 */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '-' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '-';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date2) { /*18-Feb-1999 */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '-' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '-';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date3) { /*18-Feb */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '-' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat);
    } else if (fmtType == KCFormat::Date4) { /*18-05 */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '-' + QString().sprintf("%02d", date.month());
    } else if (fmtType == KCFormat::Date5) { /*18/05/00 */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '/' + QString().sprintf("%02d", date.month()) + '/';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date6) { /*18/05/1999 */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '/' + QString().sprintf("%02d", date.month()) + '/';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date7) { /*Feb-99 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '-';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date8) { /*February-99 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::LongFormat) + '-';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date9) { /*February-1999 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::LongFormat) + '-';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date10) { /*F-99 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::LongFormat).at(0) + '-';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date11) { /*18/Feb */
        tmp = QString().sprintf("%02d", date.day()) + '/';
        tmp += m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat);
    } else if (fmtType == KCFormat::Date12) { /*18/02 */
        tmp = QString().sprintf("%02d", date.day()) + '/';
        tmp += QString().sprintf("%02d", date.month());
    } else if (fmtType == KCFormat::Date13) { /*18/Feb/1999 */
        tmp = QString().sprintf("%02d", date.day());
        tmp += '/' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '/';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date14) { /*2000/Feb/18 */
        tmp = QString::number(date.year());
        tmp += '/' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '/';
        tmp += QString().sprintf("%02d", date.day());
    } else if (fmtType == KCFormat::Date15) { /*2000-Feb-18 */
        tmp = QString::number(date.year());
        tmp += '-' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '-';
        tmp += QString().sprintf("%02d", date.day());
    } else if (fmtType == KCFormat::Date16) { /*2000-02-18 */
        tmp = QString::number(date.year());
        tmp += '-' + QString().sprintf("%02d", date.month()) + '-';
        tmp += QString().sprintf("%02d", date.day());
    } else if (fmtType == KCFormat::Date17) { /*2 february 2000 */
        tmp = QString().sprintf("%d", date.day());
        tmp += ' ' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::LongFormat) + ' ';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date18) { /*02/18/1999 */
        tmp = QString().sprintf("%02d", date.month());
        tmp += '/' + QString().sprintf("%02d", date.day());
        tmp += '/' + QString::number(date.year());
    } else if (fmtType == KCFormat::Date19) { /*02/18/99 */
        tmp = QString().sprintf("%02d", date.month());
        tmp += '/' + QString().sprintf("%02d", date.day());
        tmp += '/' + QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date20) { /*Feb/18/99 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat);
        tmp += '/' + QString().sprintf("%02d", date.day());
        tmp += '/' + QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date21) { /*Feb/18/1999 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat);
        tmp += '/' + QString().sprintf("%02d", date.day());
        tmp += '/' + QString::number(date.year());
    } else if (fmtType == KCFormat::Date22) { /*Feb-1999 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '-';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date23) { /*1999 */
        tmp = QString::number(date.year());
    } else if (fmtType == KCFormat::Date24) { /*99 */
        tmp = QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date25) { /*2000/02/18 */
        tmp = QString::number(date.year());
        tmp += '/' + QString().sprintf("%02d", date.month());
        tmp += '/' + QString().sprintf("%02d", date.day());
    } else if (fmtType == KCFormat::Date26) { /*2000/Feb/18 */
        tmp = QString::number(date.year());
        tmp += '/' + m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat);
        tmp += '/' + QString().sprintf("%02d", date.day());
    } else if (fmtType == KCFormat::Date27) { /*Feb/99 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '/';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date28) { /*Feb/1999 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::ShortFormat) + '/';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date29) { /*February/99 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::LongFormat) + '/';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date30) { /*February/1999 */
        tmp = m_converter->settings()->locale()->calendar()->monthString(date, KCalendarSystem::LongFormat) + '/';
        tmp += QString::number(date.year());
    } else if (fmtType == KCFormat::Date31) { /*18-02 */
        tmp = QString().sprintf("%02d", date.day()) + '-';
        tmp += QString().sprintf("%02d", date.month());
    } else if (fmtType == KCFormat::Date32) { /*02/99 */
        tmp = QString().sprintf("%02d", date.month()) + '/';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date33) { /*02-99 */
        tmp = QString().sprintf("%02d", date.month()) + '-';
        tmp += QString::number(date.year()).right(2);
    } else if (fmtType == KCFormat::Date34 || fmtType == KCFormat::Date35) { /*Mon, 2 Feb 2000 and Mon, 2 February 2000 */
        QLocale l(QLocale::English);
        tmp = l.toString(date, fmtType == KCFormat::Date34 ? "ddd d MMM yy" : "dddd d MMM yyyy");
    } else { /*fallback... */
        tmp = m_converter->settings()->locale()->formatDate(date, KLocale::ShortDate);
    }

    // Missing compared with gnumeric:
    //  "m/d/yy h:mm",    /* 20 */
    //  "m/d/yyyy h:mm",  /* 21 */
    //  "mmm/ddd/yy",    /* 12 */
    //  "mmm/ddd/yyyy",    /* 13 */
    //  "mm/ddd/yy",    /* 14 */
    //  "mm/ddd/yyyy",    /* 15 */

    return tmp;
}

QString KCValueFormatter::complexFormat(const KCValue& value, int precision,
                                      KCFormat::Type formatType,
                                      KCStyle::FloatFormat floatFormat,
                                      const QString& currencySymbol)
{
    // FIXME Stefan: percentage, currency and scientific formats!
    QString str;
    const KCNumber real = value.asComplex().real();
    const KCNumber imag = value.asComplex().imag();
    str = createNumberFormat(real, precision, formatType, floatFormat, QString(), QString());
    str += createNumberFormat(imag, precision, formatType, KCStyle::AlwaysSigned, currencySymbol, QString());
    str += 'i';
    return str;
}
