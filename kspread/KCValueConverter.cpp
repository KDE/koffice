/* This file is part of the KDE project
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>

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

// Local
#include "KCValueConverter.h"

#include "KCCalculationSettings.h"
#include "KCLocalization.h"
#include "KCValueParser.h"

KCValueConverter::KCValueConverter(const KCValueParser* parser)
        : m_parser(parser)
{
}

const KCCalculationSettings* KCValueConverter::settings() const
{
    return m_parser->settings();
}

KCValue KCValueConverter::asBoolean(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;
    bool okay = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(false);
        break;
    case KCValue::Boolean:
        val = value;
        break;
    case KCValue::Integer:
        val = KCValue(value.asInteger() ? true : false);
        break;
    case KCValue::Float:
        val = KCValue((value.asFloat() == 0.0) ? false : true);
        break;
    case KCValue::Complex:
        val = KCValue((value.asComplex().real() == complex<KCNumber>(0.0, 0.0)) ? false : true);
        break;
    case KCValue::String:
        val = m_parser->tryParseBool(value.asString(), &okay);
        if (!okay)
            val = KCValue(false);
        if (ok)
            *ok = okay;
        break;
    case KCValue::Array:
        val = asBoolean(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        val = KCValue(false);
        break;
    };

    return val;
}

KCValue KCValueConverter::asInteger(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(0);
        break;
    case KCValue::Boolean:
        val = KCValue(value.asBoolean() ? 1 : 0);
        break;
    case KCValue::Integer:
        val = value;
        break;
    case KCValue::Float:
        val = KCValue(value.asInteger());
        break;
    case KCValue::Complex:
        val = KCValue(value.asInteger());
        break;
    case KCValue::String:
        val = m_parser->parse(value.asString());
        if (!val.isNumber()) {
            val = KCValue(0);
            if (ok)
                *ok = false;
        }
        val = KCValue(val.asInteger());
        break;
    case KCValue::Array:
        val = asInteger(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        val = KCValue(0);
        break;
    };

    return val;
}

KCValue KCValueConverter::asFloat(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(0.0);
        break;
    case KCValue::Boolean:
        val = KCValue(value.asBoolean() ? 1.0 : 0.0);
        break;
    case KCValue::Integer:
        val = KCValue(value.asFloat());
        break;
    case KCValue::Float:
        val = value;
        break;
    case KCValue::Complex:
        val = KCValue(value.asFloat());
        break;
    case KCValue::String:
        val = m_parser->parse(value.asString());
        if (!val.isNumber()) {
            val = KCValue(0.0);
            if (ok)
                *ok = false;
        }
        val = KCValue(val.asFloat());
        break;
    case KCValue::Array:
        val = asFloat(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        val = KCValue(0.0);
        break;
    };

    return val;
}

KCValue KCValueConverter::asComplex(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(complex<KCNumber>(0.0, 0.0));
        break;
    case KCValue::Boolean:
        val = KCValue(complex<KCNumber>(value.asBoolean() ? 1.0 : 0.0, 0.0));
        break;
    case KCValue::Integer:
    case KCValue::Float:
        val = KCValue(complex<KCNumber>(value.asFloat(), 0.0));
        break;
    case KCValue::Complex:
        val = value;
        break;
    case KCValue::String:
        val = m_parser->parse(value.asString());
        if (!val.isNumber()) {
            val = KCValue(complex<KCNumber>(0.0, 0.0));
            if (ok)
                *ok = false;
        }
        val = KCValue(val.asComplex());
        break;
    case KCValue::Array:
        val = asComplex(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        val = KCValue(complex<KCNumber>(0.0, 0.0));
        break;
    };

    return val;
}

KCValue KCValueConverter::asNumeric(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(0.0);
        break;
    case KCValue::Boolean:
        val = KCValue(value.asBoolean() ? 1.0 : 0.0);
        break;
    case KCValue::Integer:
    case KCValue::Float:
    case KCValue::Complex:
        val = value;
        break;
    case KCValue::String:
        val = m_parser->parse(value.asString());
        if (!val.isNumber()) {
            val = KCValue(0.0);
            if (ok)
                *ok = false;
        }
        break;
    case KCValue::Array:
        val = asNumeric(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        val = KCValue(0.0);
        break;
    };

    return val;
}

KCValue KCValueConverter::asString(const KCValue &value) const
{
    // This is a simpler version of KCValueFormatter... We cannot use that one,
    // as we sometimes want to generate the string differently ...

    KCValue val;
    QString s;
    KCValue::KCFormat fmt;
    int pos;
    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(QString());
        break;
    case KCValue::Boolean:
        val = KCValue(value.asBoolean() ? ki18n("True").toString(m_parser->settings()->locale()) :
                    ki18n("False").toString(m_parser->settings()->locale()));
        break;
    case KCValue::Integer: {
        fmt = value.format();
        if (fmt == KCValue::fmt_Percent)
            val = KCValue(QString::number(value.asInteger() * 100) + " %");
        else if (fmt == KCValue::fmt_DateTime)
            val = KCValue(m_parser->settings()->locale()->formatDateTime(value.asDateTime(settings())));
        else if (fmt == KCValue::fmt_Date)
            val = KCValue(m_parser->settings()->locale()->formatDate(value.asDate(settings())));
        else if (fmt == KCValue::fmt_Time)
            val = KCValue(m_parser->settings()->locale()->formatTime(value.asTime(settings())));
        else
            val = KCValue(QString::number(value.asInteger()));
    }
    break;
    case KCValue::Float:
        fmt = value.format();
        if (fmt == KCValue::fmt_DateTime)
            val = KCValue(m_parser->settings()->locale()->formatDateTime(value.asDateTime(settings())));
        else if (fmt == KCValue::fmt_Date)
            val = KCValue(m_parser->settings()->locale()->formatDate(value.asDate(settings()), KLocale::ShortDate));
        else if (fmt == KCValue::fmt_Time)
            val = KCValue(m_parser->settings()->locale()->formatTime(value.asTime(settings())));
        else {
            //convert the number, change decimal point from English to local
            s = QString::number(numToDouble(value.asFloat()), 'g', 10);
            const QString decimalSymbol = m_parser->settings()->locale()->decimalSymbol();
            if (!decimalSymbol.isNull() && ((pos = s.indexOf('.')) != -1))
                s = s.replace(pos, 1, decimalSymbol);
            if (fmt == KCValue::fmt_Percent)
                s += " %";
            val = KCValue(s);
        }
        break;
    case KCValue::Complex:
        fmt = value.format();
        if (fmt == KCValue::fmt_DateTime)
            val = KCValue(m_parser->settings()->locale()->formatDateTime(value.asDateTime(settings())));
        else if (fmt == KCValue::fmt_Date)
            val = KCValue(m_parser->settings()->locale()->formatDate(value.asDate(settings()), KLocale::ShortDate));
        else if (fmt == KCValue::fmt_Time)
            val = KCValue(m_parser->settings()->locale()->formatTime(value.asTime(settings())));
        else {
            //convert the number, change decimal point from English to local
            const QString decimalSymbol = m_parser->settings()->locale()->decimalSymbol();
            QString real = QString::number(numToDouble(value.asComplex().real()), 'g', 10);
            if (!decimalSymbol.isNull() && ((pos = real.indexOf('.')) != -1))
                real = real.replace(pos, 1, decimalSymbol);
            QString imag = QString::number(numToDouble(value.asComplex().imag()), 'g', 10);
            if (!decimalSymbol.isNull() && ((pos = imag.indexOf('.')) != -1))
                imag = imag.replace(pos, 1, decimalSymbol);
            s = real;
            if (value.asComplex().imag() >= 0.0)
                s += '+';
            // TODO Stefan: Some prefer 'j'. Configure option? Spec?
            s += imag + 'i';
            // NOTE Stefan: Never recognized a complex percentage anywhere. ;-)
//         if (fmt == KCValue::fmt_Percent)
//           s += " %";
            val = KCValue(s);
        }
        break;
    case KCValue::String:
        val = value;
        break;
    case KCValue::Array:
        val = KCValue(asString(value.element(0, 0)));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        val = KCValue(value.errorMessage());
        break;
    };

    return val;
}

KCValue KCValueConverter::asDateTime(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;
    bool okay = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(QDateTime::currentDateTime(), settings());
        break;
    case KCValue::Boolean:
        //ignore the bool value... any better idea? ;)
        val = KCValue(QDateTime::currentDateTime(), settings());
        break;
    case KCValue::Integer:
    case KCValue::Float:
    case KCValue::Complex:
        val = KCValue(value.asFloat());
        val.setFormat(KCValue::fmt_DateTime);
        break;
    case KCValue::String:
        //no DateTime m_parser, so we parse as Date, hoping for the best ...
        val = m_parser->tryParseDate(value.asString(), &okay);
        if (!okay)
            val = KCValue::errorVALUE();
        if (ok)
            *ok = okay;
        val.setFormat(KCValue::fmt_DateTime);
        break;
    case KCValue::Array:
        val = asDateTime(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        break;
    };

    return val;
}

KCValue KCValueConverter::asDate(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;
    bool okay = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(QDate::currentDate(), settings());
        break;
    case KCValue::Boolean:
        //ignore the bool value... any better idea? ;)
        val = KCValue(QDate::currentDate(), settings());
        break;
    case KCValue::Integer:
    case KCValue::Float:
    case KCValue::Complex:
        val = KCValue(value.asFloat());
        val.setFormat(KCValue::fmt_Date);
        break;
    case KCValue::String:
        val = m_parser->tryParseDate(value.asString(), &okay);
        if (!okay)
            val = KCValue::errorVALUE();
        if (ok)
            *ok = okay;
        break;
    case KCValue::Array:
        val = asDate(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        break;
    };

    return val;
}

KCValue KCValueConverter::asTime(const KCValue &value, bool* ok) const
{
    KCValue val;

    if (ok)
        *ok = true;
    bool okay = true;

    switch (value.type()) {
    case KCValue::Empty:
        val = KCValue(QTime::currentTime(), settings());
        break;
    case KCValue::Boolean:
        //ignore the bool value... any better idea? ;)
        val = KCValue(QTime::currentTime(), settings());
        break;
    case KCValue::Integer:
    case KCValue::Float:
    case KCValue::Complex:
        val = KCValue(value.asFloat());
        val.setFormat(KCValue::fmt_Time);
        break;
    case KCValue::String:
        val = m_parser->tryParseTime(value.asString(), &okay);
        if (!okay)
            val = KCValue::errorVALUE();
        if (ok)
            *ok = okay;
        break;
    case KCValue::Array:
        val = asTime(value.element(0, 0));
        break;
    case KCValue::CellRange:
        /* NOTHING */
        break;
    case KCValue::Error:
        break;
    };

    return val;
}

bool KCValueConverter::toBoolean(const KCValue& value) const
{
    return asBoolean(value).asBoolean();
}

int KCValueConverter::toInteger(const KCValue& value) const
{
    return asInteger(value).asInteger();
}

KCNumber KCValueConverter::toFloat(const KCValue& value) const
{
    return asFloat(value).asFloat();
}

complex<KCNumber> KCValueConverter::toComplex(const KCValue& value) const
{
    return asComplex(value).asComplex();
}

QString KCValueConverter::toString(const KCValue& value) const
{
    return asString(value).asString();
}

QDateTime KCValueConverter::toDateTime(const KCValue& value) const
{
    return asDateTime(value).asDateTime(settings());
}

QDate KCValueConverter::toDate(const KCValue& value) const
{
    return asDate(value).asDate(settings());
}

QTime KCValueConverter::toTime(const KCValue& value) const
{
    return asTime(value).asTime(settings());
}
