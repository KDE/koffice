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

#ifndef KSPREAD_VALUE_CONVERTER
#define KSPREAD_VALUE_CONVERTER

#include "KCValue.h"

class CalculationSettings;
class ValueParser;

/**
 * \ingroup KCValue
 * Converts between the different KCValue types.
 */
class KSPREAD_EXPORT ValueConverter
{
public:
    /**
     * Constructor.
     */
    explicit ValueConverter(const ValueParser* parser);

    /**
     * Returns the calculation settings this ValueFormatter uses.
     */
    const CalculationSettings* settings() const;

    /**
     * Converts \p value to a KCValue of boolean type.
     */
    KCValue asBoolean(const KCValue& value, bool *ok = 0) const;

    /**
     * Converts \p value to a KCValue of integer type.
     */
    KCValue asInteger(const KCValue& value, bool *ok = 0) const;

    /**
     * Converts \p value to a KCValue of floating point type.
     */
    KCValue asFloat(const KCValue& value, bool* ok = 0) const;

    /**
     * Converts \p value to a KCValue of complex number type.
     */
    KCValue asComplex(const KCValue& value, bool *ok = 0) const;

    /**
     * Converts \p value to a KCValue of number type, i.e. Values of integer and
     * complex number type stay as they are; all others are converted to the
     * floating point type.
     */
    KCValue asNumeric(const KCValue& value, bool *ok = 0) const;

    /**
     * Converts \p value to a KCValue of string type.
     */
    KCValue asString(const KCValue& value) const;

    /**
     * Converts \p value to a KCValue of date/time type.
     */
    KCValue asDateTime(const KCValue& value, bool *ok = 0) const;

    /**
     * Converts \p value to a KCValue of date type.
     */
    KCValue asDate(const KCValue& value, bool *ok = 0) const;

    /**
     * Converts \p value to a KCValue of time type.
     */
    KCValue asTime(const KCValue& value, bool *ok = 0) const;


    bool toBoolean(const KCValue& value) const;
    int toInteger(const KCValue& value) const;
    Number toFloat(const KCValue& value) const;
    complex<Number> toComplex(const KCValue& value) const;
    QString toString(const KCValue& value) const;
    QDateTime toDateTime(const KCValue& value) const;
    QDate toDate(const KCValue& value) const;
    QTime toTime(const KCValue& value) const;

private:
    const ValueParser* m_parser;
};

#endif  //KSPREAD_VALUE_CONVERTER
