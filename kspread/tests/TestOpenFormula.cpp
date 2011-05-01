/* This file is part of the KDE project
   Copyright 2007 Ariya Hidayat <ariya@kde.org>
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2005 Tomas Mecir <mecirt@gmail.com>

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

#include "TestOpenFormula.h"

#include <klocale.h>

#include "qtest_kde.h"

#include <KCFormula.h>
#include <KCFunctionModuleRegistry.h>
#include <KCRegion.h>
#include <Util.h>
#include <KCValue.h>

// because we may need to promote expected value from integer to float
#define CHECK_EVAL(x,y) { KCValue z(y); QCOMPARE(evaluate(x,z),(z)); }

KCValue TestOpenFormula::evaluate(const QString& formula, KCValue& ex)
{
    KCFormula f;
    QString expr = formula;
    if (expr[0] != '=')
        expr.prepend('=');
    f.setExpression(expr);
    KCValue result = f.eval();

    if (result.isFloat() && ex.isInteger())
        ex = KCValue(ex.asFloat());
    if (result.isInteger() && ex.isFloat())
        result = KCValue(result.asFloat());

    return result;
}

namespace QTest
{
template<>
char *toString(const KCValue& value)
{
    QString message;
    QTextStream ts(&message, QIODevice::WriteOnly);
    ts << value;
    return qstrdup(message.toLatin1());
}
}

#define CHECK_CONVERT(x,y) \
    { QCOMPARE(convertToOpenFormula(x),QString(y)); \
        QCOMPARE(convertFromOpenFormula(y),QString(x)); }

QString TestOpenFormula::convertToOpenFormula(const QString& expr)
{
    KLocale locale("en_US");
    locale.setDecimalSymbol(",");
    locale.setThousandsSeparator(" ");

    QString formula = KCells::Odf::encodeFormula(expr, &locale);
    return formula;
}

QString TestOpenFormula::convertFromOpenFormula(const QString& expr)
{
    KLocale locale("en_US");
    locale.setDecimalSymbol(",");
    locale.setThousandsSeparator(" ");

    QString formula = KCells::Odf::decodeFormula(expr, &locale);
    return formula;
}

void TestOpenFormula::initTestCase()
{
    KCFunctionModuleRegistry::instance()->loadFunctionModules();
}

void TestOpenFormula::testEvaluation()
{
    // tests from the OpenFormula testing suite:
    // note that these get auto-generated using generate-openformula-tests
    CHECK_EVAL("=(1/3)*3=1", KCValue(true));  // row 51
    CHECK_EVAL("=(\"4\" & \"5\")+2", KCValue(47));  // row 57
    CHECK_EVAL("=2+(\"4\" & \"5\")", KCValue(47));  // row 58
    CHECK_EVAL("=1+2", KCValue(3));  // row 63
    CHECK_EVAL("=3-1", KCValue(2));  // row 65
    CHECK_EVAL("=5--2", KCValue(7));  // row 67
    CHECK_EVAL("=3*4", KCValue(12));  // row 68
    CHECK_EVAL("=2+3*4", KCValue(14));  // row 70
    CHECK_EVAL("=6/3", KCValue(2));  // row 71
    CHECK_EVAL("=5/2", KCValue(2.5));  // row 72
    CHECK_EVAL("=ISERROR(1/0)", KCValue(true));  // row 73
    CHECK_EVAL("=2^3", KCValue(8));  // row 74
    CHECK_EVAL("=9^0.5", KCValue(3));  // row 75
    CHECK_EVAL("=(-5)^3", KCValue(-125));  // row 76
    CHECK_EVAL("=4^-1", KCValue(0.25));  // row 77
    CHECK_EVAL("=5^0", KCValue(1));  // row 78
    CHECK_EVAL("=0^5", KCValue(0));  // row 79
    CHECK_EVAL("=2+3*4^2", KCValue(50));  // row 80
    CHECK_EVAL("=-2^2", KCValue(4));  // row 81
    CHECK_EVAL("=1=1", KCValue(true));  // row 82
    CHECK_EVAL("=1=0", KCValue(false));  // row 84
    CHECK_EVAL("=3=3.0001", KCValue(false));  // row 85
// Not passed for line 86.
    CHECK_EVAL("=\"Hi\"=\"Bye\"", KCValue(false));  // row 87
    CHECK_EVAL("=FALSE()=FALSE()", KCValue(true));  // row 88
    CHECK_EVAL("=TRUE()=FALSE()", KCValue(false));  // row 89
    CHECK_EVAL("=\"5\"=5", KCValue(false));  // row 90
    CHECK_EVAL("=TRUE()=1", KCValue(false));  // row 91
// Not passed for line 92.
// Not passed for line 93.
    CHECK_EVAL("=1<>1", KCValue(false));  // row 94
    CHECK_EVAL("=1<>2", KCValue(true));  // row 95
    CHECK_EVAL("=1<>\"1\"", KCValue(true));  // row 96
// Not passed for line 97.
    CHECK_EVAL("=5<6", KCValue(true));  // row 98
    CHECK_EVAL("=5<=6", KCValue(true));  // row 99
    CHECK_EVAL("=5>6", KCValue(false));  // row 100
    CHECK_EVAL("=5>=6", KCValue(false));  // row 101
    CHECK_EVAL("=\"A\"<\"B\"", KCValue(true));  // row 102
// Not passed for line 103.
    CHECK_EVAL("=\"AA\">\"A\"", KCValue(true));  // row 104
    CHECK_EVAL("=\"Hi \" & \"there\"", KCValue("Hi there"));  // row 107
    CHECK_EVAL("=\"H\" & \"\"", KCValue("H"));  // row 108
// Not passed for line 109.
    CHECK_EVAL("=50%", KCValue(0.5));  // row 111
    CHECK_EVAL("=20+50%", KCValue(20.5));  // row 112
    CHECK_EVAL("=+5", KCValue(5));  // row 113
    CHECK_EVAL("=+\"Hello\"", KCValue("Hello"));  // row 114
    CHECK_EVAL("=-\"7\"", KCValue(-7));  // row 116
    /*
     These are currently disabled, due to being locale specific.
     CHECK_EVAL("=DATE(2005;1;3)=DATEVALUE(\"2005-01-03\")", KCValue(true));  // row 118
      CHECK_EVAL("=DATE(2017.5; 1; 2)=DATEVALUE(\"2017-01-02\")", KCValue(true));  // row 119
      CHECK_EVAL("=DATE(2006; 2.5; 3)=DATEVALUE(\"2006-02-03\")", KCValue(true));  // row 120
      CHECK_EVAL("=DATE(2006; 1; 3.5)=DATEVALUE(\"2006-01-03\")", KCValue(true));  // row 121
      CHECK_EVAL("=DATE(2006; 13; 3)=DATEVALUE(\"2007-01-03\")", KCValue(true));  // row 122
      CHECK_EVAL("=DATE(2006; 1; 32)=DATEVALUE(\"2006-02-01\")", KCValue(true));  // row 123
      CHECK_EVAL("=DATE(2006; 25; 34)=DATEVALUE(\"2008-02-03\")", KCValue(true));  // row 124
      CHECK_EVAL("=DATE(2006;-1; 1)=DATEVALUE(\"2005-11-01\")", KCValue(true));  // row 125
    // Not passed for line 126.
    // Not passed for line 127.
      CHECK_EVAL("=DATE(2004;2;29)=DATEVALUE(\"2004-02-29\")", KCValue(true));  // row 128
      CHECK_EVAL("=DATE(2003;2;29)=DATEVALUE(\"2003-03-01\")", KCValue(true));  // row 129
      CHECK_EVAL("=DATE(1904; 1; 1)=DATEVALUE(\"1904-01-01\")", KCValue(true));  // row 130
      CHECK_EVAL("=DATEVALUE(\"2004-12-25\")=DATE(2004;12;25)", KCValue(true));  // row 131
      CHECK_EVAL("=DAY(\"2006-05-21\")", KCValue(21));  // row 132
      CHECK_EVAL("=DAY(\"5/21/2006\")", KCValue(21));  // row 133
      CHECK_EVAL("=DAY(\"05-21-2006\")", KCValue(21));  // row 134
      CHECK_EVAL("=DAY(\"5/21/06\")", KCValue(21));  // row 135
      CHECK_EVAL("=DAY(\"5-21-06\")", KCValue(21));  // row 136
    */

}

void TestOpenFormula::testFormulaConversion()
{
    // cell references
    CHECK_CONVERT("=A1", "=[.A1]");
    CHECK_CONVERT("=A1:A4", "=[.A1:.A4]");
    CHECK_CONVERT("=A$1:$A4", "=[.A$1:.$A4]");
    CHECK_CONVERT("=Sheet2!A1", "=[Sheet2.A1]");
    CHECK_CONVERT("='KCSheet 2'!A1", "=['KCSheet 2'.A1]");
    CHECK_CONVERT("=Sheet2!A1:B4", "=[Sheet2.A1:Sheet2.B4]");
    CHECK_CONVERT("='KCSheet 2'!A1:B4", "=['KCSheet 2'.A1:'KCSheet 2'.B4]");

    // equality
    CHECK_CONVERT("=A1==A2", "=[.A1]=[.A2]");

    // strings
    CHECK_CONVERT("=\"2,2\"+2,1+\"2,0\"", "=\"2,2\"+2.1+\"2,0\"");

    // decimal separator ','
    CHECK_CONVERT("=,12", "=.12");
    CHECK_CONVERT("=12,12", "=12.12");
    CHECK_CONVERT("=368*7*(0,1738+0,1784)*(0,1738+0,1784)", "=368*7*(0.1738+0.1784)*(0.1738+0.1784)");

    // function names
    CHECK_CONVERT("=sum(A1;A2;A3;A4;A5)", "=sum([.A1];[.A2];[.A3];[.A4];[.A5])");
}

void TestOpenFormula::testReferenceLoading()
{
    QCOMPARE(KCRegion::loadOdf(".A1"),                         QString("A1"));
    QCOMPARE(KCRegion::loadOdf(".A1:.A4"),                     QString("A1:A4"));
    QCOMPARE(KCRegion::loadOdf(".A$1:.$A4"),                   QString("A$1:$A4"));
    QCOMPARE(KCRegion::loadOdf("Sheet2.A1"),                   QString("Sheet2!A1"));
    QCOMPARE(KCRegion::loadOdf("'KCSheet 2'.A1"),                QString("'KCSheet 2'!A1"));
    QCOMPARE(KCRegion::loadOdf("Sheet2.A1:Sheet2.B4"),         QString("Sheet2!A1:B4"));
    QCOMPARE(KCRegion::loadOdf("'KCSheet 2'.A1:'KCSheet 2'.B4"),   QString("'KCSheet 2'!A1:B4"));
    QCOMPARE(KCRegion::loadOdf("$Sheet2.A1:$Sheet2.B4"),       QString("Sheet2!A1:B4"));
    QCOMPARE(KCRegion::loadOdf("$'KCSheet 2'.A1:$'KCSheet 2'.B4"), QString("'KCSheet 2'!A1:B4"));
}

void TestOpenFormula::testReferenceSaving()
{
    QCOMPARE(KCRegion::saveOdf("A1"),              QString(".A1"));
    QCOMPARE(KCRegion::saveOdf("A1:A4"),           QString(".A1:.A4"));
    QCOMPARE(KCRegion::saveOdf("A$1:$A4"),         QString(".A$1:.$A4"));
    QCOMPARE(KCRegion::saveOdf("Sheet2!A1"),       QString("Sheet2.A1"));
    QCOMPARE(KCRegion::saveOdf("'KCSheet 2'!A1"),    QString("'KCSheet 2'.A1"));
    QCOMPARE(KCRegion::saveOdf("Sheet2!A1:B4"),    QString("Sheet2.A1:Sheet2.B4"));
    QCOMPARE(KCRegion::saveOdf("'KCSheet 2'!A1:B4"), QString("'KCSheet 2'.A1:'KCSheet 2'.B4"));
}

QTEST_KDEMAIN(TestOpenFormula, GUI)

#include "TestOpenFormula.moc"
