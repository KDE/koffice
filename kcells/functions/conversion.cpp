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

// built-in conversion functions

#include "ConversionModule.h"

#include "KCFunction.h"
#include "KCFunctionModuleRegistry.h"
#include "KCValueCalc.h"
#include "KCValueConverter.h"

#include <KLocale>

#include <QByteArray>

using namespace KCells;

// prototypes
KCValue func_arabic(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_carx(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_cary(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_decsex(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_polr(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_pola(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_roman(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_sexdec(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_AsciiToChar(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_CharToAscii(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_inttobool(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_booltoint(valVector args, KCValueCalc *calc, FuncExtra *);
KCValue func_ToString(valVector args, KCValueCalc *calc, FuncExtra *);


KCELLS_EXPORT_FUNCTION_MODULE("conversion", ConversionModule)


ConversionModule::ConversionModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    f = new KCFunction("ARABIC", func_arabic);
    add(f);
    f = new KCFunction("CARX", func_carx);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("CARY", func_cary);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("DECSEX", func_decsex);
    add(f);
    f = new KCFunction("POLR", func_polr);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("POLA", func_pola);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("ROMAN", func_roman);
    f->setParamCount(1, 2);
    add(f);
    f = new KCFunction("SEXDEC", func_sexdec);
    f->setParamCount(1, 3);
    add(f);
    f = new KCFunction("ASCIITOCHAR", func_AsciiToChar);
    f->setParamCount(1, -1);
    f->setAcceptArray();
    add(f);
    f = new KCFunction("CHARTOASCII", func_CharToAscii);
    add(f);
    f = new KCFunction("BOOL2INT", func_booltoint);
    add(f);
    f = new KCFunction("INT2BOOL", func_inttobool);
    add(f);
    f = new KCFunction("BOOL2STRING", func_ToString);
    add(f);
    f = new KCFunction("NUM2STRING", func_ToString);
    add(f);
    f = new KCFunction("STRING", func_ToString);
    add(f);
}

QString ConversionModule::descriptionFileName() const
{
    return QString("conversion.xml");
}


// KCFunction: POLR
KCValue func_polr(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // sqrt (a^2 + b^2)
    KCValue a = args[0];
    KCValue b = args[1];
    KCValue res = calc->sqrt(calc->add(calc->sqr(a), calc->sqr(b)));
    return res;
}

// KCFunction: POLA
KCValue func_pola(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // acos (a / polr(a,b))
    KCValue polr = func_polr(args, calc, 0);
    if (calc->isZero(polr))
        return KCValue::errorDIV0();
    KCValue res = calc->acos(calc->div(args[0], polr));
    return res;
}

// KCFunction: CARX
KCValue func_carx(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // a * cos(b)
    KCValue res = calc->mul(args[0], calc->cos(args[1]));
    return res;
}

// KCFunction: CARY
KCValue func_cary(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // a * sin(b)
    KCValue res = calc->mul(args[0], calc->sin(args[1]));
    return res;
}

// KCFunction: DECSEX
KCValue func_decsex(valVector args, KCValueCalc *calc, FuncExtra *)
{
    // original function was very compicated, but I see no reason for that,
    // when it can be done as simply as this ...
    // maybe it was due to all the infrastructure not being ready back then
    return calc->conv()->asTime(calc->div(args[0], 24));
}

// KCFunction: SEXDEC
KCValue func_sexdec(valVector args, KCValueCalc *calc, FuncExtra *)
{
    if (args.count() == 1) {
        // convert given value to number
        KCValue time = calc->conv()->asTime(args[0]);
        return calc->mul(calc->conv()->asFloat(time), 24);
    }

    // convert h/m/s to number of hours
    KCValue h = args[0];
    KCValue m = args[1];

    KCValue res = calc->add(h, calc->div(m, 60));
    if (args.count() == 3) {
        KCValue s = args[2];
        res = calc->add(res, calc->div(s, 3600));
    }
    return res;
}

// KCFunction: ROMAN
KCValue func_roman(valVector args, KCValueCalc *calc, FuncExtra *)
{
    const QByteArray RNUnits[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
    const QByteArray RNTens[] = {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
    const QByteArray RNHundreds[] = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
    const QByteArray RNThousands[] = {"", "M", "MM", "MMM"};

    // precision loss is not a problem here, as we only use the 0-3999 range
    qint64 value = calc->conv()->asInteger(args[0]).asInteger();
    if ((value < 0) || (value > 3999))
        return KCValue::errorNA();
    QString result;
    // There is an optional argument, but the specification only covers the case
    // where it is zero for conciseness, and zero is the default. So we just
    // ignore it.
    result = QString::fromLatin1(RNThousands[(value / 1000)] +
                                 RNHundreds[(value / 100) % 10] +
                                 RNTens[(value / 10) % 10] +
                                 RNUnits[(value) % 10]);
    return KCValue(result);
}

// convert single roman character to decimal
// return < 0 if invalid
int func_arabic_helper(QChar c)
{
    switch (c.toUpper().unicode()) {
    case 'M': return 1000;
    case 'D': return 500;
    case 'C': return 100;
    case 'L': return 50;
    case 'X': return 10;
    case 'V': return 5;
    case 'I': return 1;
    }
    return -1;
}

// KCFunction: ARABIC
KCValue func_arabic(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QString roman = calc->conv()->asString(args[0]).asString();
    if (roman.isEmpty()) return KCValue::errorVALUE();

    int val = 0, lastd = 0, d = 0;

    for (int i = 0; i < roman.length(); i++) {
        d = func_arabic_helper(roman[i]);
        if (d < 0) return KCValue::errorVALUE();

        if (lastd < d) val -= lastd;
        else val += lastd;
        lastd = d;
    }
    if (lastd < d) val -= lastd;
    else val += lastd;

    return KCValue(val);
}

// helper for AsciiToChar
void func_a2c_helper(KCValueCalc *calc, QString &s, KCValue val)
{
    if (val.isArray()) {
        for (uint row = 0; row < val.rows(); ++row)
            for (uint col = 0; col < val.columns(); ++col)
                func_a2c_helper(calc, s, val.element(col, row));
    } else {
        int v = calc->conv()->asInteger(val).asInteger();
        if (v == 0) return;
        QChar c(v);
        s = s + c;
    }
}

// KCFunction: AsciiToChar
KCValue func_AsciiToChar(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QString str;
    for (int i = 0; i < args.count(); i++)
        func_a2c_helper(calc, str, args[i]);
    return KCValue(str);
}

// KCFunction: CharToAscii
KCValue func_CharToAscii(valVector args, KCValueCalc *calc, FuncExtra *)
{
    QString val = calc->conv()->asString(args[0]).asString();
    if (val.length() == 1)
        return KCValue(QString(val[0]));
    return KCValue::errorVALUE();
}

// KCFunction: inttobool
KCValue func_inttobool(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->conv()->asBoolean(args[0]);
}

// KCFunction: booltoint
KCValue func_booltoint(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->conv()->asInteger(args[0]);
}

// KCFunction: BoolToString, NumberToString, String
KCValue func_ToString(valVector args, KCValueCalc *calc, FuncExtra *)
{
    return calc->conv()->asString(args[0]);
}

#include "ConversionModule.moc"
