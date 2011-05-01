/* This file is part of the KDE project
   Copyright 2007 Sascha Pfau <MrPeacock@gmail.com>
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2006 Ariya Hidayat <ariya@kde.org>

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

#include "TestStatisticalFunctions.h"

#include <math.h>

#include "qtest_kde.h"

#include <KCCellStorage.h>
#include <KCFormula.h>
#include <KCMap.h>
#include <KCSheet.h>

#include "TestKspreadCommon.h"

// NOTE: we do not compare the numbers _exactly_ because it is difficult
// to get one "true correct" expected values for the functions due to:
//  - different algorithms among spreadsheet programs
//  - precision limitation of floating-point number representation
//  - accuracy problem due to propagated error in the implementation
#define CHECK_EVAL(x,y) QCOMPARE(TestDouble(x,y,6),y)
#define CHECK_EVAL_SHORT(x,y) QCOMPARE(TestDouble(x,y,10),y)
#define CHECK_ARRAY(x,y) QCOMPARE(TestArray(x,y,10),true)
#define CHECK_ARRAY_NOSIZE(x,y) QCOMPARE(TestArray(x,y,10,false),true)
#define ROUND(x) (roundf(1e15 * x) / 1e15)

bool TestStatisticalFunctions::TestArray(const QString& formula, const QString& _Array, int accuracy, bool checkSize = true)
{
    // define epsilon
    double epsilon = DBL_EPSILON * pow(10.0, (double)(accuracy));

    KCValue Array = evaluate(_Array);
//   kDebug()<<"Array = "<<Array;

    KCValue result = evaluate(formula);

    // test match size
    if (checkSize)
        if (Array.rows() != result.rows() || Array.columns() != result.columns()) {
            kDebug() << "Array size do not match";
            return false;
        }

    // if checkSize is disabled the count of Array array could be lower than result array
    for (int e = 0; e < (int)Array.count(); e++) {
        kDebug() << "check element (" << e << ") " << (double)Array.element(e).asFloat() << " " << (double)result.element(e).asFloat();
        bool res = (long double) fabsl(Array.element(e).asFloat() - result.element(e).asFloat()) < epsilon;
        if (!res) {
            kDebug() << "check failed -->" << "Element(" << e << ") " << (double)Array.element(e).asFloat() << " to" << (double) result.element(e).asFloat() << "  diff =" << (double)(Array.element(e).asFloat() - result.element(e).asFloat());
            return false;
        }
    }
    // test passed
    return true;
}

KCValue TestStatisticalFunctions::TestDouble(const QString& formula, const KCValue& v2, int accuracy)
{
    double epsilon = DBL_EPSILON * pow(10.0, (double)(accuracy));

    KCFormula f(m_map->sheet(0)); // bind to test case data set
    QString expr = formula;
    if (expr[0] != '=')
        expr.prepend('=');
    f.setExpression(expr);
    KCValue result = f.eval();

    bool res = fabs(v2.asFloat() - result.asFloat()) < epsilon;

    if (!res)
        kDebug(36002) << "check failed -->" << "Epsilon =" << epsilon << "" << (double)v2.asFloat() << " to" << (double)result.asFloat() << "  diff =" << (double)(v2.asFloat() - result.asFloat());
//   else
//     kDebug(36002)<<"check -->" <<"  diff =" << v2.asFloat()-result.asFloat();
    if (res)
        return v2;
    else
        return result;
}

// round to get at most 15-digits number
static KCValue RoundNumber(const KCValue& v)
{
    if (v.isNumber()) {
        double d = numToDouble(v.asFloat());
        if (fabs(d) < DBL_EPSILON)
            d = 0.0;
        return KCValue(ROUND(d));
    } else
        return v;
}

KCValue TestStatisticalFunctions::evaluate(const QString& formula)
{
    KCFormula f(m_map->sheet(0));
    QString expr = formula;
    if (expr[0] != '=')
        expr.prepend('=');
    f.setExpression(expr);
    KCValue result = f.eval();

#if 0
    // this magically generates the CHECKs
    printf("  CHECK_EVAL( \"%s\",  %15g) );\n", qPrintable(formula), result.asFloat());
#endif

    return RoundNumber(result);
}

void TestStatisticalFunctions::initTestCase()
{
    FunctionModuleRegistry::instance()->loadFunctionModules();
    m_map = new KCMap(0 /*no Doc*/);
    m_map->addNewSheet();
    KCSheet* sheet = m_map->sheet(0);
    KCCellStorage* storage = sheet->cellStorage();

    //
    // Test case data set
    //


    // A19:A29
    storage->setValue(1, 19, KCValue(1));
    storage->setValue(1, 20, KCValue(2));
    storage->setValue(1, 21, KCValue(4));
    storage->setValue(1, 22, KCValue(8));
    storage->setValue(1, 23, KCValue(16));
    storage->setValue(1, 24, KCValue(32));
    storage->setValue(1, 25, KCValue(64));
    storage->setValue(1, 26, KCValue(128));
    storage->setValue(1, 27, KCValue(256));
    storage->setValue(1, 28, KCValue(512));
    storage->setValue(1, 29, KCValue(1024));
    storage->setValue(1, 30, KCValue(2048));
    storage->setValue(1, 31, KCValue(4096));


    // B3:B17
    storage->setValue(2, 3, KCValue("7"));
    storage->setValue(2, 4, KCValue(2));
    storage->setValue(2, 5, KCValue(3));
    storage->setValue(2, 6, KCValue(true));
    storage->setValue(2, 7, KCValue("Hello"));
    // B8 leave empty
    storage->setValue(2, 9, KCValue::errorDIV0());
    storage->setValue(2, 10, KCValue(0));
    storage->setValue(2, 11, KCValue(3));
    storage->setValue(2, 12, KCValue(4));
    storage->setValue(2, 13, KCValue("2005-0131T01:00:00"));
    storage->setValue(2, 14, KCValue(1));
    storage->setValue(2, 15, KCValue(2));
    storage->setValue(2, 16, KCValue(3));
    storage->setValue(2, 17, KCValue(4));
    CHECK_EVAL("AVEDEV(1;2;3;4)",              KCValue(1));

    // C4:C6
    storage->setValue(3, 4, KCValue(4));
    storage->setValue(3, 5, KCValue(5));
    storage->setValue(3, 6, KCValue(7));
    // C11:C17
    storage->setValue(3, 11, KCValue(5));
    storage->setValue(3, 12, KCValue(6));
    storage->setValue(3, 13, KCValue(8));
    storage->setValue(3, 14, KCValue(4));
    storage->setValue(3, 15, KCValue(3));
    storage->setValue(3, 16, KCValue(2));
    storage->setValue(3, 17, KCValue(1));
    // C19:C31
    storage->setValue(3, 19, KCValue(0));
    storage->setValue(3, 20, KCValue(5));
    storage->setValue(3, 21, KCValue(2));
    storage->setValue(3, 22, KCValue(5));
    storage->setValue(3, 23, KCValue(3));
    storage->setValue(3, 24, KCValue(4));
    storage->setValue(3, 25, KCValue(4));
    storage->setValue(3, 26, KCValue(0));
    storage->setValue(3, 27, KCValue(8));
    storage->setValue(3, 28, KCValue(1));
    storage->setValue(3, 29, KCValue(9));
    storage->setValue(3, 30, KCValue(6));
    storage->setValue(3, 31, KCValue(2));
    // C51:C57
    storage->setValue(3, 51, KCValue(7));
    storage->setValue(3, 52, KCValue(9));
    storage->setValue(3, 53, KCValue(11));
    storage->setValue(3, 54, KCValue(12));
    storage->setValue(3, 55, KCValue(15));
    storage->setValue(3, 56, KCValue(17));
    storage->setValue(3, 57, KCValue(19));


    // D51:D57
    storage->setValue(4, 51, KCValue(100));
    storage->setValue(4, 52, KCValue(105));
    storage->setValue(4, 53, KCValue(104));
    storage->setValue(4, 54, KCValue(108));
    storage->setValue(4, 55, KCValue(111));
    storage->setValue(4, 56, KCValue(120));
    storage->setValue(4, 57, KCValue(133));

    // F19:F26
    storage->setValue(6, 19, KCValue(20));
    storage->setValue(6, 20, KCValue(5));
    storage->setValue(6, 21, KCValue(-20));
    storage->setValue(6, 22, KCValue(-60));
    storage->setValue(6, 23, KCValue(75));
    storage->setValue(6, 24, KCValue(-29));
    storage->setValue(6, 25, KCValue(20));
    storage->setValue(6, 26, KCValue(30));
    // F51:F60
    storage->setValue(6, 51, KCValue(3));
    storage->setValue(6, 52, KCValue(4));
    storage->setValue(6, 53, KCValue(5));
    storage->setValue(6, 54, KCValue(2));
    storage->setValue(6, 55, KCValue(3));
    storage->setValue(6, 56, KCValue(4));
    storage->setValue(6, 57, KCValue(5));
    storage->setValue(6, 58, KCValue(6));
    storage->setValue(6, 59, KCValue(4));
    storage->setValue(6, 60, KCValue(7));


    // G51:G60
    storage->setValue(7, 51, KCValue(23));
    storage->setValue(7, 52, KCValue(24));
    storage->setValue(7, 53, KCValue(25));
    storage->setValue(7, 54, KCValue(22));
    storage->setValue(7, 55, KCValue(23));
    storage->setValue(7, 56, KCValue(24));
    storage->setValue(7, 57, KCValue(25));
    storage->setValue(7, 58, KCValue(26));
    storage->setValue(7, 59, KCValue(24));
    storage->setValue(7, 60, KCValue(27));

    // H19:H31
    storage->setValue(8, 19, KCValue("2005-03-12"));
    storage->setValue(8, 20, KCValue("2002-02-03"));
    storage->setValue(8, 21, KCValue("2005-03-08"));
    storage->setValue(8, 22, KCValue("1991-03-27"));
    storage->setValue(8, 23, KCValue("1967-07-05"));
    storage->setValue(8, 24, KCValue("1912-12-23"));
    storage->setValue(8, 25, KCValue("1992-02-06"));
    storage->setValue(8, 26, KCValue("1934-07-04"));
    storage->setValue(8, 27, KCValue("1909-01-08"));
    storage->setValue(8, 28, KCValue("1989-11-28"));
    storage->setValue(8, 29, KCValue("2000-02-22"));
    storage->setValue(8, 30, KCValue("2004-03-29"));
    storage->setValue(8, 31, KCValue("1946-07-13"));

    // I19:I31
    storage->setValue(9, 19, KCValue(13));
    storage->setValue(9, 20, KCValue(12));
    storage->setValue(9, 21, KCValue(11));
    storage->setValue(9, 22, KCValue(10));
    storage->setValue(9, 23, KCValue(9));
    storage->setValue(9, 24, KCValue(8));
    storage->setValue(9, 25, KCValue(7));
    storage->setValue(9, 26, KCValue(6));
    storage->setValue(9, 27, KCValue(5));
    storage->setValue(9, 28, KCValue(4));
    storage->setValue(9, 29, KCValue(3));
    storage->setValue(9, 30, KCValue(2));
    storage->setValue(9, 31, KCValue(1));
}

void TestStatisticalFunctions::testAVEDEV()
{
    // ODF-tests
    CHECK_EVAL("AVEDEV(1;2;3;4)",              KCValue(1));    //
}

void TestStatisticalFunctions::testAVERAGE()
{
    // ODF-tests
    CHECK_EVAL("AVERAGE(2; 4)",                KCValue(3));    //
}

void TestStatisticalFunctions::testAVERAGEA()
{
    // ODF-tests
    CHECK_EVAL("AVERAGEA(2; 4)",               KCValue(3));    //
    CHECK_EVAL("AVERAGEA(TRUE(); FALSE(); 5)", KCValue(2));    //
}

void TestStatisticalFunctions::testBETADIST()
{
    // ODF-tests

    // Cumulative tests
    CHECK_EVAL("BETADIST( 0  ; 3; 4)",           KCValue(0));           //
    CHECK_EVAL("BETADIST( 0.5; 3; 4)",           KCValue(0.656250));    //
    CHECK_EVAL("BETADIST( 0.9; 4; 3)",           KCValue(0.984150));    //
    CHECK_EVAL("BETADIST( 2  ; 3; 4)",           KCValue(1));           // constraints x > b should be 1 if cumulative
    CHECK_EVAL("BETADIST(-1  ; 3; 4)",           KCValue(0));           // constraints x < a

    CHECK_EVAL_SHORT("BETADIST(1.5;3;4;1;2)",    evaluate("BETADIST(0.5;3;4)"));    // diff = -2.27021e-09
    CHECK_EVAL_SHORT("BETADIST(2;3;4;1;3)",      evaluate("BETADIST(0.5;3;4)"));    // diff = -2.27021e-09

    // last parameter FALSE (non - Cumulative)
    CHECK_EVAL("BETADIST( 0  ;3;4;0;1;FALSE())", KCValue(0));               //
    CHECK_EVAL("BETADIST( 0.5;3;4;0;1;FALSE())", KCValue(0.0005208333));    // 0.000521
    CHECK_EVAL("BETADIST( 0.9;4;3;0;1;FALSE())", KCValue(0.0001215000));    // 0.000122
    CHECK_EVAL("BETADIST( 2  ;3;4;0;1;FALSE())", KCValue(0));               // constraints x > b should be 0 if non-cumulative
    CHECK_EVAL("BETADIST(-1  ;3;4;0;1;FALSE())", KCValue(0));               // constraints x < a

    CHECK_EVAL("BETADIST(1.5;3;4;1;2;FALSE())",  evaluate("BETADIST(0.5;3;4;0;1;FALSE())"));    //
    CHECK_EVAL("BETADIST(2  ;3;4;1;3;FALSE())",  evaluate("BETADIST(0.5;3;4;0;1;FALSE())"));    //
}

void TestStatisticalFunctions::testBETAINV()
{
    // ODF-tests
    CHECK_EVAL("BETADIST(BETAINV(0;3;4);3;4)",           KCValue(0));      //
    CHECK_EVAL("BETADIST(BETAINV(0.1;3;4);3;4)",         KCValue(0.1));    //
    CHECK_EVAL("BETADIST(BETAINV(0.3;3;4);3;4)",         KCValue(0.3));    //
    CHECK_EVAL("BETADIST(BETAINV(0.5;4;3);4;3)",         KCValue(0.5));    //
    CHECK_EVAL("BETADIST(BETAINV(0.7;4;3);4;3)",         KCValue(0.7));    //
    CHECK_EVAL("BETADIST(BETAINV(1;3;4);3;4)",           KCValue(1));      //
    CHECK_EVAL("BETADIST(BETAINV(0;3;4;1;3);3;4;1;3)",   KCValue(0));      //
    CHECK_EVAL("BETADIST(BETAINV(0.1;3;4;1;3);3;4;1;3)", KCValue(0.1));    //
    CHECK_EVAL("BETADIST(BETAINV(0.3;3;4;1;3);3;4;1;3)", KCValue(0.3));    //
    CHECK_EVAL("BETADIST(BETAINV(0.5;4;3;1;3);4;3;1;3)", KCValue(0.5));    //
    CHECK_EVAL("BETADIST(BETAINV(0.7;4;3;1;3);4;3;1;3)", KCValue(0.7));    //
    CHECK_EVAL("BETADIST(BETAINV(1;3;4;1;3);3;4;1;3)",   KCValue(1));      //
}

void TestStatisticalFunctions::testBINOMDIST()
{
    // bettersolution.com
    CHECK_EVAL("BINOMDIST(10;10;  1  ;0)", KCValue(1));               // Prob.=100% - all trials successful
    CHECK_EVAL("BINOMDIST(9 ; 1; 10  ;0)", KCValue(0));               // Prob. of -exactly- 9 trials successful is 0 then
    CHECK_EVAL("BINOMDIST(10;10;  0.1;1)", KCValue(1));               // Sum of probabilities of 0..10 hits is 1.
//     CHECK_EVAL("BINOMDIST(4 ;10;  0.4;1)", KCValue( 0.6331032576 ) ); // Some random values.
    // my tests
    CHECK_EVAL_SHORT("BINOMDIST(4 ;10;  0.4;1)", KCValue(0.6331032576));    // Some random values.
    CHECK_EVAL_SHORT("BINOMDIST(5 ;10;  0.4;1)", KCValue(0.8337613824));    // Some random values.
    CHECK_EVAL_SHORT("BINOMDIST(6 ;10;  0.4;1)", KCValue(0.9452381184));    // Some random values.
    CHECK_EVAL_SHORT("BINOMDIST(4 ;10;  0.2;1)", KCValue(0.9672065024));    // Some random values.
    CHECK_EVAL_SHORT("BINOMDIST(5 ;10;  0.2;1)", KCValue(0.9936306176));    // Some random values.
    CHECK_EVAL_SHORT("BINOMDIST(6 ;10;  0.2;1)", KCValue(0.9991356416));    // Some random values.
}

void TestStatisticalFunctions::testCHIDIST()
{
    // bettersolution.com
    CHECK_EVAL("CHIDIST( 18.307;10)",      KCValue(0.0500005892));    //
    CHECK_EVAL("CHIDIST(      2;2)",       KCValue(0.3678794412));    //
    CHECK_EVAL("CHIDIST(     -1;2)",       KCValue(1));               // constraint x<0 TODO EXCEL return #NUM!
//     CHECK_EVAL("CHIDIST(     4;\"texr\")", KCValue::VALUE()    ); // TODO
}

void TestStatisticalFunctions::testCONFIDENCE()
{
    // ODF-tests
    CHECK_EVAL("CONFIDENCE(0.5 ; 1;1)", KCValue(0.67448975));      //
    CHECK_EVAL("CONFIDENCE(0.25; 1;1)", KCValue(1.1503493804));    //
    CHECK_EVAL("CONFIDENCE(0.5 ; 4;1)", KCValue(2.6979590008));    // Multiplying stddev by X multiplies result by X.
    CHECK_EVAL("CONFIDENCE(0.5 ; 1;4)", KCValue(0.3372448751));    // Multiplying count by X*X divides result by X.

    // check constraints
    CHECK_EVAL("CONFIDENCE(-0.5; 1;4)", KCValue::errorNUM());    // 0 < alpha < 1
    CHECK_EVAL("CONFIDENCE( 1.5; 1;4)", KCValue::errorNUM());    // 0 < alpha < 1
    CHECK_EVAL("CONFIDENCE( 0.5;-1;4)", KCValue::errorNUM());    // stddev > 0
    CHECK_EVAL("CONFIDENCE( 0.5; 1;0)", KCValue::errorNUM());    // size >= 1
}

void TestStatisticalFunctions::testCORREL()
{
    //  KCCell | KCValue      KCCell | KCValue
    // ------+------     ------+------
    //   B14 |  1          C14 |  4
    //   B15 |  2          C15 |  3
    //   B16 |  3          C16 |  2
    //   B17 |  4          C17 |  1

    // ODF-tests
    CHECK_EVAL("CORREL(B14:B17;B14:B17)", KCValue(1));               // Perfect positive correlation given identical sequences
    CHECK_EVAL("CORREL(B14:B17;C14:C17)", KCValue(-1));              // Perfect negative correlation given reversed sequences
    CHECK_EVAL("CORREL(1;2)",             KCValue::errorNUM());      // Each list must contain at least 2 values
    CHECK_EVAL("CORREL(B14:B16;B15:B16)", KCValue::errorNUM());      // The length of each list must be equal
}

void TestStatisticalFunctions::testCOVAR()
{
    //  KCCell | KCValue      KCCell | KCValue
    // ------+------     ------+------
    //   B14 |  1          C14 |  4
    //   B15 |  2          C15 |  3
    //   B16 |  3          C16 |  2
    //   B17 |  4          C17 |  1

    // ODF-tests
    CHECK_EVAL("COVAR(C11:C17;C11:C17)", KCValue(4.9795918367));     //
    CHECK_EVAL("COVAR(B14:B17;C14:C17)", KCValue(-1.25));            //
    CHECK_EVAL("COVAR(B14:B17;C13:C17)", KCValue::errorNUM());       // TODO should we check for "array sizes don't match" or "value counts" in array?.
}

void TestStatisticalFunctions::testDEVSQ()
{
    // ODF-tests
    CHECK_EVAL("DEVSQ(4)",         KCValue(0));                // One value - no deviation.
    CHECK_EVAL("DEVSQ(5;5;5;5)",   KCValue(0));                // Identical values - no deviation.
    CHECK_EVAL("DEVSQ(2;4)",       KCValue(2));                // Each value deviates by 1.
    CHECK_EVAL("DEVSQ(-5;5;-1;1)", KCValue(52));               // Average=0 must work properly.
    CHECK_EVAL("DEVSQ(C11:C17)",   KCValue(34.8571428571));    // Test values.
    CHECK_EVAL("DEVSQ(B14:B17)",   KCValue(5.00));             // Test values.
    CHECK_EVAL("DEVSQ(B14)",       KCValue(0));                // One value - no deviation.
}

// void TestStatisticalFunctions::testDEVSQA()
// {
//     // no test available
// }

void TestStatisticalFunctions::testEXPONDIST()
{
    // ODF-tests
    CHECK_EVAL("EXPONDIST( 1;1;TRUE())",   KCValue(0.6321205588));    //
    CHECK_EVAL("EXPONDIST( 2;2;TRUE())",   KCValue(0.9816843611));    //
    CHECK_EVAL("EXPONDIST( 0;1;TRUE())",   KCValue(0));               //
    CHECK_EVAL("EXPONDIST(-1;1;TRUE())",   KCValue(0));               // constraint x<0

    CHECK_EVAL("EXPONDIST( 1;1;FALSE())",  KCValue(0.3678794412));    //
    CHECK_EVAL("EXPONDIST( 2;2;FALSE())",  KCValue(0.0366312778));    //
    CHECK_EVAL("EXPONDIST( 0;1;FALSE())",  KCValue(1));               //
    CHECK_EVAL("EXPONDIST(-1;1;FALSE())",  KCValue(0));               // constraint x<0

    // test disabled, because 3rd param. is not opt.!
    //CHECK_EVAL("EXPONDIST(1;1)", evaluate("EXPONDIST(1;1;TRUE())") );
}

void TestStatisticalFunctions::testFDIST()
{
    // ODF-tests

    // cumulative
    CHECK_EVAL("FDIST( 1;4;5)", KCValue(0.5143428033));       //
    CHECK_EVAL("FDIST( 2;5;4)", KCValue(0.7392019723));       //
    CHECK_EVAL("FDIST( 0;4;5)", KCValue(0));                  //
    CHECK_EVAL("FDIST(-1;4;5)", KCValue(0));                  //

    CHECK_EVAL_SHORT("FDIST( 1;4;5;TRUE())", evaluate("FDIST(1;4;5)"));  // diff = -1.39644e-09

    // non-cumulative
    CHECK_EVAL("FDIST( 1;4;5;FALSE())", KCValue(0.3976140792));    //
    CHECK_EVAL("FDIST( 2;5;4;FALSE())", KCValue(0.1540004108));    //
    CHECK_EVAL("FDIST( 0;4;5;FALSE())", KCValue(0));               //
    CHECK_EVAL("FDIST(-1;4;5;FALSE())", KCValue(0));               //
}

void TestStatisticalFunctions::testFINV()
{
    // ODF-tests
    CHECK_EVAL("FDIST(FINV(0.1;3;4);3;4)", KCValue(0.1));    //
    CHECK_EVAL("FDIST(FINV(0.3;3;4);3;4)", KCValue(0.3));    //
    CHECK_EVAL("FDIST(FINV(0.5;3;4);3;4)", KCValue(0.5));    //
    CHECK_EVAL("FDIST(FINV(0.7;3;4);3;4)", KCValue(0.7));    //
    CHECK_EVAL("FDIST(FINV(0.0;3;4);3;4)", KCValue(0.0));    //
}

void TestStatisticalFunctions::testFISHER()
{
    // ODF-tests
    CHECK_EVAL("FISHER(0)",                        KCValue(0));             // Fisher of 0.
    CHECK_EVAL("FISHER((EXP(1)-1)/(EXP(1)+1))",    KCValue(0.5));           // Argument chosen so that ln=1
    CHECK_EVAL_SHORT("FISHER(0.5)",                KCValue(0.54930614));    // TODO - be more precise - Some random value.
    CHECK_EVAL("FISHER(0.47)+FISHER(-0.47)",       KCValue(0));             // Function is symetrical.
}

void TestStatisticalFunctions::testFISHERINV()
{
    // ODF-tests
    CHECK_EVAL("FISHERINV(0)",                     KCValue(0));             // Fisherinv of 0.
    CHECK_EVAL("FISHERINV(LN(2))",                 KCValue(0.6));           // e^(2*ln(2))=4
    CHECK_EVAL("FISHERINV(FISHER(0.5))",           KCValue(0.5));           // Some random value.
    CHECK_EVAL("FISHERINV(0.47)+FISHERINV(-0.47)", KCValue(0));             // Function is symetrical.
}

void TestStatisticalFunctions::testFREQUENCY()
{
    KCValue result(KCValue::Array);
    result.setElement(0, 0, KCValue(3));
    result.setElement(0, 1, KCValue(2));
    result.setElement(0, 2, KCValue(4));
    result.setElement(0, 3, KCValue(1));
    CHECK_EVAL("FREQUENCY({1;2;3;4;5;6;7;8;9;10};{3|5|9})", result);
    // the second arg has to be a column vector
    CHECK_EVAL("ISERROR(FREQUENCY({1;2;3;4;5;6;7;8;9;10};{3;5;9}))", KCValue(true));
    // an empty second arg returns the overall number count
    CHECK_EVAL("FREQUENCY({1;2;3;4;5;6;7;8;9;10};)", KCValue(10));
}

void TestStatisticalFunctions::testFTEST()
{
    // TODO - be more precise
    // ODF-tests
    CHECK_EVAL_SHORT("FTEST(B14:B17; C14:C17)", KCValue(1.0));            // Same data (second reversed),
    CHECK_EVAL_SHORT("FTEST(B14:B15; C13:C14)", KCValue(0.311916521));    // Significantly different variances,
    // so less likely to come from same data set.
}

void TestStatisticalFunctions::testGAMMADIST()
{
    // bettersolution.com non-cumulative
    CHECK_EVAL("GAMMADIST(10 ;9;2;FALSE())",      KCValue(0.0326390197));    //

    // bettersolution.com cumulative
    CHECK_EVAL("GAMMADIST(10 ;9;2;TRUE())",       KCValue(0.0680936347));    //
    CHECK_EVAL("GAMMADIST(10 ;10;5;TRUE())",      KCValue(0.0000464981));    // Bettersolution = 0 .rounded?
    CHECK_EVAL("GAMMADIST(7 ;5;1;TRUE())",        KCValue(0.8270083921));    // TODO NOK / Bettersolution = 1

    // bettersolution.com constraints
    CHECK_EVAL("GAMMADIST(10 ;9;0;TRUE())",       KCValue::errorNUM());      // beta = 0 not allowed
    CHECK_EVAL("GAMMADIST(10 ;-2;2;TRUE())",      KCValue::errorNUM());      // was wird getestet? alpha
    CHECK_EVAL("GAMMADIST(-1 ;9;2;TRUE())",       KCValue::errorNUM());      // NOK
    CHECK_EVAL("GAMMADIST(7 ;\"text\";1;TRUE())", KCValue::errorVALUE());    // text not allowed
    CHECK_EVAL("GAMMADIST(7 ;5;\"text\";TRUE())", KCValue::errorVALUE());    // text not allowed

    // ODF-tests non-cumulative
    CHECK_EVAL("GAMMADIST(0  ;3;4;FALSE())",      KCValue(0));
    CHECK_EVAL("GAMMADIST(0.5;3;4;FALSE())",      KCValue(0.0017236268));    //
    CHECK_EVAL("GAMMADIST(9  ;4;3;FALSE())",      KCValue(0.0746806026));    // ODF-Specs -> 0.0666979468 should be 0,0746
    CHECK_EVAL("GAMMADIST(0  ;3;4;FALSE())",      KCValue(0));
    CHECK_EVAL("GAMMADIST(9  ;4;3;FALSE())",      KCValue(0.0746806026));    // TODO check ODF-Specs -> 0.390661

    // ODF-tests cumulative
    CHECK_EVAL("GAMMADIST(0.5;3;4;TRUE())",       KCValue(0.0002964775));    //
    CHECK_EVAL("GAMMADIST(9  ;4;3;TRUE())",       KCValue(0.3527681112));
    CHECK_EVAL("GAMMADIST(-1 ;4;3;TRUE())",       KCValue(0));               // neg. x return always 0
    CHECK_EVAL("GAMMADIST(-1 ;3;4;FALSE())",      KCValue(0));               // neg. x return always 0

    // various tests cumulative
    CHECK_EVAL("GAMMADIST(9 ;9;2;TRUE())",        KCValue(0.0402573125));    //
    CHECK_EVAL("GAMMADIST(9 ;8;2;TRUE())",        KCValue(0.0865864716));    //
}

void TestStatisticalFunctions::testGAMMAINV()
{
    // ODF-tests
    CHECK_EVAL("GAMMADIST(GAMMAINV(0.1;3;4);3;4;1)",     KCValue(0.1));    //
    CHECK_EVAL("GAMMADIST(GAMMAINV(0.3;3;4);3;4;1)",     KCValue(0.3));    //
    CHECK_EVAL("GAMMADIST(GAMMAINV(0.5;3;4);3;4;1)",     KCValue(0.5));    //
    CHECK_EVAL("GAMMADIST(GAMMAINV(0.7;3;4);3;4;1)",     KCValue(0.7));    //
    CHECK_EVAL("GAMMADIST(GAMMAINV(0  ;3;4);3;4;1)",     KCValue(0));      //
}

void TestStatisticalFunctions::testGAUSS()
{
    // ODF-tests
    CHECK_EVAL("GAUSS(0)",     KCValue(0));                // Mean of one value.
    CHECK_EVAL("GAUSS(1)",     KCValue(0.341344746));      // Multiple equivalent values.
    // my test
    CHECK_EVAL("GAUSS(-0.25)", KCValue(-0.0987063257));    // check neg. values. test for fixes gauss_func
}

void TestStatisticalFunctions::testGROWTH()
{
    // constraints
    CHECK_EVAL("GROWTH({}; C19:C23; 1)",          KCValue::errorNA());  // empty knownY matrix
    CHECK_EVAL("GROWTH({5.0;\"a\"}; C19:C23; 1)", KCValue::errorNA());  // knownY matrix constains chars

    // ODF-tests
    CHECK_ARRAY("GROWTH( A19:A23; C19:C23; 1 )",          "{2.5198420998}");  // with offset
    CHECK_ARRAY("GROWTH( A19:A23; C19:C23; 1; FALSE() )", "{1.4859942891}");  // without offset

    // http://www.techonthenet.com/excel/formulas/growth.php
    CHECK_ARRAY("GROWTH({4;5;6};{10;20;30};{15;30;45})", "{4.4569483434;6.0409611796;8.1879369384}");  //
    CHECK_ARRAY("GROWTH({4;5;6};{10;20;30})",            "{4.0273074534;4.9324241487;6.0409611796}");  //
    CHECK_ARRAY_NOSIZE("GROWTH({4;5;6})",                "{4.0273074534}");                            //
}

void TestStatisticalFunctions::testGEOMEAN()
{
    // ODF-tests
    CHECK_EVAL("GEOMEAN(7)",           KCValue(7));               // Mean of one value.
    CHECK_EVAL("GEOMEAN(5;5;5;5)",     KCValue(5));               // Multiple equivalent values.
    CHECK_EVAL("GEOMEAN(2;8;2;8)",     KCValue(4));               // Some values.
    CHECK_EVAL("GEOMEAN(8;0;8;8;8;8)", KCValue::errorNUM());      // Error if there is a 0 in the range.
    CHECK_EVAL("GEOMEAN(C11)",         KCValue(5));               // One value, range.
    CHECK_EVAL("GEOMEAN(C11:C17)",     KCValue(3.4451109418));    // Some values, range.
    CHECK_EVAL("GEOMEAN(B14:B17)",     KCValue(2.2133638394));    // Some values, range.
}

void TestStatisticalFunctions::testHARMEAN()
{
    // ODF-tests
    CHECK_EVAL("HARMEAN(7)",           KCValue(7));               // Mean of one value.
    CHECK_EVAL("HARMEAN(4;4;4;4)",     KCValue(4));               // Multiple equivalent values.
    CHECK_EVAL("HARMEAN(2;4;4)",       KCValue(3));               // Some values.
    CHECK_EVAL("HARMEAN(8;0;8;8;8;8)", KCValue::errorNUM());      // Error if there is a 0 in the range.
    CHECK_EVAL("HARMEAN(C11)",         KCValue(5));               // One value, range.
    CHECK_EVAL("HARMEAN(C11:C17)",     KCValue(2.7184466019));    // Some values, range.
    CHECK_EVAL("HARMEAN(B14:B17)",     KCValue(1.92));            // Some values, range.
}

void TestStatisticalFunctions::testHYPGEOMDIST()
{
    // ODF-tests
    CHECK_EVAL("HYPGEOMDIST( 2  ;3;3;6;FALSE())", KCValue(0.45));        // If an urn contains 3 red balls and 3 green balls, the probability
    // that 2 red balls will be selected after 3 selections without replacement.
    // (0.45=27/60).
    CHECK_EVAL("HYPGEOMDIST( 2  ;3;3;6)",         KCValue(0.45));        // The default for cumulative is FALSE().
    CHECK_EVAL("HYPGEOMDIST( 0  ;3;3;6)",         KCValue(0.05));        // There is a small (5%) chance of selecting only green balls.
    CHECK_EVAL("HYPGEOMDIST( 2  ;3;3;6;TRUE())",  KCValue(0.95));        // The probability of selecting at most two red balls (i.e 0, 1 or 2).
    CHECK_EVAL("HYPGEOMDIST( 4  ;3;3;6)",         KCValue::errorNUM());  // X must be <= M
    CHECK_EVAL("HYPGEOMDIST( 2.8;3;3;6)",         KCValue(0.45));        // Non-integers are truncated.
    CHECK_EVAL("HYPGEOMDIST(-2  ;3;3;6)",         KCValue::errorNUM());  // Values must be >= 0.
    CHECK_EVAL("HYPGEOMDIST( 0  ;0;0;0)",         KCValue(1));           //
}

void TestStatisticalFunctions::testINTERCEPT()
{
    // bettersolution.com
    CHECK_EVAL_SHORT("INTERCEPT({2;3;9;1;8};{6;5;11;7;5})", KCValue(0.048387097));    // TODO - be more precise
//     CHECK_EVAL_SHORT("INTERCEPT({2;4;6};{6;3;8})",          KCValue( 2.21053     ) ); // TODO - be more precise
    CHECK_EVAL("INTERCEPT({2;3;9};{6;5;11;7;5})",     KCValue::errorNUM());     //
    CHECK_EVAL("INTERCEPT(\"text\";{6;5;11;7;5})",    KCValue::errorNUM());     // text is not allowed
}

void TestStatisticalFunctions::testKURT()
{
    // TODO check function

    // ODF-tests
    CHECK_EVAL("KURT(C20:C25)",     KCValue(-0.446162998));    //
    CHECK_EVAL("KURT(C20:C23;4;4)", KCValue(-0.446162998));    //
}

void TestStatisticalFunctions::testLARGE()
{
    //  KCCell | KCValue | N'th
    // ------+-------+------
    //   B14 |   1   |   3
    //   B15 |   2   |   2
    //   B16 |   3   |   1

    // ODF-tests
    CHECK_EVAL("LARGE(B14:B16;1)", KCValue(3));           //
    CHECK_EVAL("LARGE(B14:B16;3)", KCValue(1));           //
    CHECK_EVAL("LARGE(B14:B16;4)", KCValue::errorNUM());  // N is greater than the length of the list
}

void TestStatisticalFunctions::testLEGACYCHIDIST()
{
    // ODF-tests LEGACY.CHIDIST
    CHECK_EVAL("LEGACYCHIDIST(-1;2)", KCValue(1));    // constraint x<0
    CHECK_EVAL("LEGACYCHIDIST( 0;2)", KCValue(1));    // constraint x=0
    CHECK_EVAL("LEGACYCHIDIST( 2;2)", KCValue(0.3678794412));    //
    CHECK_EVAL("LEGACYCHIDIST( 4;4)", KCValue(0.4060058497));    //
}

void TestStatisticalFunctions::testLEGACYCHIINV()
{
    // ODF-tests LEGACY.CHIINV
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.1;3);3)",   KCValue(0.1));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.3;3);3)",   KCValue(0.3));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.5;3);3)",   KCValue(0.5));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.7;3);3)",   KCValue(0.7));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.9;3);3)",   KCValue(0.9));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.1;20);20)", KCValue(0.1));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.3;20);20)", KCValue(0.3));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.5;20);20)", KCValue(0.5));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.7;20);20)", KCValue(0.7));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(0.9;20);20)", KCValue(0.9));    //
    CHECK_EVAL("LEGACYCHIDIST(LEGACYCHIINV(1.0;20);20)", KCValue(1.0));    //
}

void TestStatisticalFunctions::testLEGACYFDIST()
{
    // ODF-tests
    CHECK_EVAL("LEGACYFDIST( 1;4;5)", KCValue(0.4856571967));    //
    CHECK_EVAL("LEGACYFDIST( 2;5;4)", KCValue(0.2607980277));    //
    CHECK_EVAL("LEGACYFDIST( 0;4;5)", KCValue(1));               //
    CHECK_EVAL("LEGACYFDIST(-1;4;5)", KCValue::errorNUM());      //
}

void TestStatisticalFunctions::testLEGACYFINV()
{
    // ODF-tests
    CHECK_EVAL("LEGACYFDIST(LEGACYFINV(0.1;3;4);3;4)", KCValue(0.1));    //
    CHECK_EVAL("LEGACYFDIST(LEGACYFINV(0.3;3;4);3;4)", KCValue(0.3));    //
    CHECK_EVAL("LEGACYFDIST(LEGACYFINV(0.5;3;4);3;4)", KCValue(0.5));    //
    CHECK_EVAL("LEGACYFDIST(LEGACYFINV(0.7;3;4);3;4)", KCValue(0.7));    //
    CHECK_EVAL("LEGACYFDIST(LEGACYFINV(1.0;3;4);3;4)", KCValue(1.0));    //
}

void TestStatisticalFunctions::testLOGINV()
{
    // TODO check function

    // ODF-tests
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.1;0;1);0;1;TRUE())", KCValue(0.1));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.3;0;1);0;1;TRUE())", KCValue(0.3));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.5;0;1);0;1;TRUE())", KCValue(0.5));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.7;0;1);0;1;TRUE())", KCValue(0.7));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.9;0;1);0;1;TRUE())", KCValue(0.9));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.1;1;4);1;4;TRUE())", KCValue(0.1));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.3;1;4);1;4;TRUE())", KCValue(0.3));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.5;1;4);1;4;TRUE())", KCValue(0.5));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.7;1;4);1;4;TRUE())", KCValue(0.7));    //
    CHECK_EVAL("LOGNORMDIST(LOGINV(0.9;1;4);1;4;TRUE())", KCValue(0.9));    //
    CHECK_EVAL("LOGINV(0.5)",                             KCValue(1));      //
}

void TestStatisticalFunctions::testLOGNORMDIST()
{
    // TODO - implement cumulative calculation
    //      - check definition cumulative/non-cumulative and constraints

    // ODF-tests

    // cumulative
    CHECK_EVAL("LOGNORMDIST(1)",              KCValue(0.5));             //
    CHECK_EVAL("LOGNORMDIST(1;1;4)",          KCValue(0.4012936743));    //
    CHECK_EVAL("LOGNORMDIST(1;0;1;TRUE())",   KCValue(0.5));             //
    CHECK_EVAL("LOGNORMDIST(1;1;4;TRUE())",   KCValue(0.4012936743));    //
    CHECK_EVAL("LOGNORMDIST(1;-1;4;TRUE())",  KCValue(0.5987063257));    //
//     CHECK_EVAL("LOGNORMDIST(2;-1;4;TRUE())",  KCValue( 0.663957 ) ); // ??????
    CHECK_EVAL("LOGNORMDIST(3;0;1;TRUE())",   KCValue(0.8640313924));    //
    CHECK_EVAL("LOGNORMDIST(100;0;1;TRUE())", KCValue(0.9999979394));    //
    CHECK_EVAL("LOGNORMDIST(-1;0;1;TRUE())",  KCValue(0));               // constraint x<0 returns 0

    // non-cumulative
//     CHECK_EVAL("LOGNORMDIST( 1; 0; 1;FALSE())", KCValue( 0.398942 ) ); //
//     CHECK_EVAL("LOGNORMDIST( 1; 1; 4;FALSE())", KCValue( 0.096667 ) ); //
//     CHECK_EVAL("LOGNORMDIST( 1;-1; 4;FALSE())", KCValue( 0.096667 ) ); //
//     CHECK_EVAL("LOGNORMDIST( 2;-1; 4;FALSE())", KCValue( 0.045595 ) ); //
//     CHECK_EVAL("LOGNORMDIST(-1; 0; 1;FALSE())", KCValue::errorNUM() ); // constraint failure
//     CHECK_EVAL("LOGNORMDIST( 1; 0;-1;FALSE())", KCValue::errorNUM() ); // constraint failure
}

void TestStatisticalFunctions::testMAX()
{
    //  KCCell | KCValue      KCCell | KCValue
    // ------+------     ------+------
    //   B3  |  "7"        C14 |  4
    //   B4  |  2          C15 |  3
    //   B5  |  3          C16 |  2
    //   B6  |  true       C17 |  1
    //   B7  |  "Hello"
    //   B8  |
    //   B9  | DIV/0


    // ODF-tests
    CHECK_EVAL("MAX(2;4;1;-8)", KCValue(4));             // Negative numbers are smaller than positive numbers.
    CHECK_EVAL("MAX(B4:B5)",    KCValue(3));             // The maximum of (2,3) is 3.
//     CHECK_EVAL("ISNA(MAXA(NA())", KCValue(true)); // nline errors are propagated.
    CHECK_EVAL("MAX(B3:B5)",    KCValue(3));             // Strings are not converted to numbers and are ignored.
    CHECK_EVAL("MAX(-1;B7)",    KCValue(-1));            // Strings are not converted to numbers and are ignored.
    CHECK_EVAL("MAX(B3:B9)",    KCValue::errorVALUE());  // TODO check function - Errors inside ranges are NOT ignored.
}

void TestStatisticalFunctions::testMAXA()
{
    // ODF-tests
    CHECK_EVAL("MAXA(2;4;1;-8)", KCValue(4));             // Negative numbers are smaller than positive numbers.
    CHECK_EVAL("MAXA(B4:B5)",    KCValue(3));             // The maximum of (2,3) is 3.
//     CHECK_EVAL("ISNA(MAXA(NA())", KCValue(true)); // Inline errors are propagated.

// TODO check function - inline Text must be converted, but not Text in Cells
//     CHECK_EVAL("MAXA(B3:B5)",    KCValue(          3 ) ); // KCCell text is converted to 0.

    CHECK_EVAL("MAXA(-1;B7)",    KCValue(0));             // KCCell text is converted to 0.
    CHECK_EVAL("MAXA(\"a\")",    KCValue::errorVALUE());  // Text inline is NOT ignored.
    CHECK_EVAL("MAXA(B3:B9)",    KCValue::errorVALUE());  // TODO check function - Errors inside ranges are NOT ignored.
    CHECK_EVAL("MAXA(B6:B7)",    KCValue(1));             // Logicals are considered numbers.
}

void TestStatisticalFunctions::testMEDIAN()
{
    // ODF-tests
    CHECK_EVAL("=MEDIAN(10.5;7.2)",        KCValue(8.85));
    CHECK_EVAL("=MEDIAN(7.2;200;5.4;45)",  KCValue(26.1));
    CHECK_EVAL("=MEDIAN(7.2;200;5.4;8.1)", KCValue(7.65));
    CHECK_EVAL("=MEDIAN(1;3;13;14;15)",    KCValue(13.0));
    CHECK_EVAL("=MEDIAN(1;3;13;14;15;35)", KCValue(13.5));
    // Bug 148574: MEDIAN function gives incorrect results
    CHECK_EVAL("=MEDIAN(1;2;3)",     KCValue(2));
    CHECK_EVAL("=MEDIAN(1;2;3;4;5)", KCValue(3));
}

void TestStatisticalFunctions::testMIN()
{
    // ODF-tests
    CHECK_EVAL("MIN(2;4;1;-8)", KCValue(-8));          // Negative numbers are smaller than positive numbers.
    CHECK_EVAL("MIN(B4:B5)",    KCValue(2));           // The minimum of (2,3) is 2.
    CHECK_EVAL("MIN(B3)",       KCValue(0));           // If no numbers are provided in all ranges, MIN returns 0
    CHECK_EVAL("MIN(\"a\")",    KCValue::errorNUM());  // Non-numbers inline are NOT ignored.
    CHECK_EVAL("MIN(B3:B5)",    KCValue(2));           // KCCell text is not converted to numbers and is ignored.
}

void TestStatisticalFunctions::testMINA()
{
    // ODF-tests
    CHECK_EVAL("MINA(2;4;1;-8)", KCValue(-8));          // Negative numbers are smaller than positive numbers.
    CHECK_EVAL("MINA(B4:B5)",    KCValue(2));           // The minimum of (2,3) is 2.
    CHECK_EVAL("MINA(1;B7)",     KCValue(0));           // KCCell text is converted to 0.
    CHECK_EVAL("MINA(\"a\")",    KCValue::errorNUM());  // KCCell text inline is NOT ignored.

// TODO check function - inline Text must be converted, but not Text in Cells
//     CHECK_EVAL("MINA(B3:B5)",    KCValue(        0 ) ); // KCCell text is converted to 0.

    CHECK_EVAL("MINA(B6:C6)",    KCValue(1));           // The value "True" is considered equivalent to 1.
}

void TestStatisticalFunctions::testMODE()
{
    // ODF-tests
    CHECK_EVAL("MODE(F51:F60)",                                 KCValue(4));           //
    CHECK_EVAL("MODE(G51;G52;G53;G54;G55;G56;G57;G58;G59;G60)", KCValue(24));          //
    CHECK_EVAL("MODE(1;2;3;4;5;6;7;8;9;10)",                    KCValue::errorNUM());  //
}

void TestStatisticalFunctions::testNEGBINOMDIST()
{
    // ODF-test
//     CHECK_EVAL("NEGBINOMDIST(F20;I29;H6)", KCValue( 0.000130947 ) ); //

    // bettersolutions.com
    CHECK_EVAL("NEGBINOMDIST( 0;1; 0.25)", KCValue(0.25));            //
    CHECK_EVAL("NEGBINOMDIST( 0;1; 0.5)",  KCValue(0.5));             //
    CHECK_EVAL("NEGBINOMDIST( 1;6; 0.5)",  KCValue(0.046875));        //
    CHECK_EVAL("NEGBINOMDIST(10;5; 0.25)", KCValue(0.0550486604));    //
    CHECK_EVAL("NEGBINOMDIST(10;5;-4)",    KCValue::errorNUM());      //
//     CHECK_EVAL("NEGBINOMDIST(10;"text";0.25)", KCValue::NUM() ); //
}

void TestStatisticalFunctions::testNORMDIST()
{
    // ODF-tests
    CHECK_EVAL("NORMDIST(0;1;4;TRUE())",         KCValue(0.4012936743));    //
    CHECK_EVAL("NORMDIST(0;0;1;FALSE())",        KCValue(0.3989422804));    //
    CHECK_EVAL("NORMDIST(0;0;1;TRUE())",         KCValue(0.5));             //
    CHECK_EVAL("NORMDIST(0;1;4;FALSE())",        KCValue(0.0966670292));    //
    CHECK_EVAL("NORMDIST(0;-1;4;FALSE())",       KCValue(0.0966670292));    //
    CHECK_EVAL("NORMDIST(0;-1;4;TRUE())",        KCValue(0.5987063257));    //
    CHECK_EVAL("NORMDIST(1;-1;4;FALSE())",       KCValue(0.0880163317));    //
    CHECK_EVAL("NORMDIST(1;-1;4;TRUE())",        KCValue(0.6914624613));    //
    CHECK_EVAL("NORMDIST(1.281552;0;1;TRUE())",  KCValue(0.9000000762));    //
    CHECK_EVAL("NORMDIST(0;-1.281552;1;TRUE())", KCValue(0.9000000762));    //
    CHECK_EVAL("NORMDIST(0;0;-1;FALSE())",       KCValue::errorNUM());      //
}

void TestStatisticalFunctions::testNORMINV()
{
    // ODF-tests
    CHECK_EVAL("NORMDIST(NORMINV(0.1;0;1);0;1;TRUE())", KCValue(0.1));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.3;0;1);0;1;TRUE())", KCValue(0.3));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.5;0;1);0;1;TRUE())", KCValue(0.5));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.7;0;1);0;1;TRUE())", KCValue(0.7));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.9;0;1);0;1;TRUE())", KCValue(0.9));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.1;1;4);1;4;TRUE())", KCValue(0.1));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.3;1;4);1;4;TRUE())", KCValue(0.3));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.5;1;4);1;4;TRUE())", KCValue(0.5));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.7;1;4);1;4;TRUE())", KCValue(0.7));    //
    CHECK_EVAL("NORMDIST(NORMINV(0.9;1;4);1;4;TRUE())", KCValue(0.9));    //
}

void TestStatisticalFunctions::testPEARSON()
{
    //  KCCell | KCValue       KCCell | KCValue       KCCell | KCValue       KCCell | KCValue
    // ------+-------     ------+-------     ------+-------     ------+-------
    //  A19  |    1        C19  |  0           C51 |   7          D51 |  100
    //  A20  |    2        C20  |  5           C51 |   9          D52 |  105
    //  A21  |    4        C21  |  2           C53 |  11          D53 |  104
    //  A22  |    8        C22  |  5           C54 |  12          D54 |  108
    //  A23  |   16        C23  |  3           C55 |  15          D55 |  111
    //  A24  |   32        C24  |  4           C56 |  17          D56 |  120
    //  A25  |   64        C25  |  4           C57 |  19          D57 |  133
    //  A26  |  128        C26  |  0
    //  A27  |  256        C27  |  8
    //  A28  |  512        C28  |  1
    //  A29  | 1024        C29  |  9
    //  A30  | 2048        C30  |  6
    //  A31  | 4096        C31  |  2

    // ODF-tests
    CHECK_EVAL_SHORT("PEARSON(A19:A31;C19:C31)", KCValue(0.045989147));    //
    CHECK_EVAL_SHORT("PEARSON(C51:C57;D51:D57)", KCValue(0.930164207));    //
    CHECK_EVAL("PEARSON(C51:C57;D51:D56)", KCValue::errorNUM());     //
}

void TestStatisticalFunctions::testPERCENTILE()
{
    // ODF-tests
    CHECK_EVAL("PERCENTILE(A19:A31;0.38)",          KCValue(24.96));        //
    CHECK_EVAL("PERCENTILE(A19:A31;0.95)",          KCValue(2867.2));       //
    CHECK_EVAL("PERCENTILE(A19:A31;0.05)",          KCValue(1.6));          //

    // my tests
    CHECK_EVAL("PERCENTILE(A10:A15;-0.1)",          KCValue::errorVALUE()); //
    CHECK_EVAL("PERCENTILE(A19:A25;1.1)",           KCValue::errorVALUE()); //

}

void TestStatisticalFunctions::testPERMUT()
{
    // ODF-tests
    CHECK_EVAL("PERMUT(2;2)",     KCValue(2));           // =2!/(2-2)!
    CHECK_EVAL("PERMUT(4;2)",     KCValue(12));          // =4!/(4-2)!
    CHECK_EVAL("PERMUT(4.3;2.1)", KCValue(12));          // =PERMUT(4;2)
    CHECK_EVAL("PERMUT(-4;2)",    KCValue::errorNUM());  //
    CHECK_EVAL("PERMUT(4;-2)",    KCValue::errorNUM());  //
}

void TestStatisticalFunctions::testPHI()
{
    //  KCCell | KCValue
    // ------+-------
    //   C23 |   3
    //       |

    // ODF-tests
    CHECK_EVAL_SHORT("PHI(C23/10)",  KCValue(0.381387815));    // TODO - be more precise /
    CHECK_EVAL_SHORT("PHI(-C23/10)", KCValue(0.381387815));    // TODO - be more precise /
    CHECK_EVAL_SHORT("PHI(0)",       KCValue(0.398942280));    // TODO - be more precise /
}

void TestStatisticalFunctions::testPOISSON()
{
    // ODF-tests
    CHECK_EVAL_SHORT("POISSON(0;1;FALSE())", KCValue(0.367880));    // TODO - be more precise /
    CHECK_EVAL_SHORT("POISSON(0;2;FALSE())", KCValue(0.135335));    // TODO - be more precise /
}

void TestStatisticalFunctions::testRANK()
{
    //  KCCell | KCValue
    // ------+------
    //  A19  |   1
    //  A20  |   2
    //  A21  |   4
    //  A22  |   8
    //  A23  |  16
    //  A24  |  32
    //  A25  |  64

    // ODF-tests
    CHECK_EVAL("RANK(A20;A19:A25;1)", KCValue(2));    // ascending
    CHECK_EVAL("RANK(A25;A19:A25;0)", KCValue(1));    // descending
    CHECK_EVAL("RANK(A21;A19:A25  )", KCValue(5));    // ommitted equals descending order
}

void TestStatisticalFunctions::testRSQ()
{
    // ODF-tests
    CHECK_EVAL("RSQ(H19:H31;I19:I31)", KCValue(0.075215010));    //
    CHECK_EVAL("RSQ(H19:H31;I19:I30)", KCValue::errorNA());      // array does not have the same size
}

void TestStatisticalFunctions::testQUARTILE()
{
    // flag:
    //  0 equals MIN()
    //  1 25th percentile
    //  2 50th percentile equals MEDIAN()
    //  3 75th percentile
    //  4 equals MAX()

    // ODF-tests
    CHECK_EVAL("QUARTILE(A19:A25;3)",            KCValue(24));            //
    CHECK_EVAL("QUARTILE(F19:F26;1)",            KCValue(-22.25));        //
    CHECK_EVAL("QUARTILE(A10:A15;2)",            KCValue::errorVALUE());  //
    CHECK_EVAL("QUARTILE(A19:A25;5)",            KCValue::errorVALUE());  // flag > 4
    CHECK_EVAL("QUARTILE(F19:F26;1.5)",          KCValue(-22.25));        // 1.5 rounded down to 1
    CHECK_EVAL("QUARTILE({1;2;4;8;16;32;64};3)", KCValue(24));            //

    // my tests
    CHECK_EVAL("QUARTILE(A19:A25;0)",            KCValue(1));             // MIN()
    CHECK_EVAL("QUARTILE(A19:A25;4)",            KCValue(64));            // MAX()

}

void TestStatisticalFunctions::testSKEW()
{
    // ODF-tests
    CHECK_EVAL_SHORT("SKEW( 1; 2; 4 )", KCValue(0.935219));    // TODO - be more precise / Expectation value: 2.333333
    // Standard deviation: 1.257525
    // Third central moment: 0.740741
    CHECK_EVAL_SHORT("SKEW(A19:A23)",   KCValue(1.325315));    // TODO - be more precise /
    CHECK_EVAL("SKEW( 1; 2 )",    KCValue::errorNUM());  // At least three numbers.
}

void TestStatisticalFunctions::testSKEWP()
{
    // ODF-tests
    CHECK_EVAL_SHORT("SKEWP( 1; 2; 4 )", KCValue(0.381802));    // TODO - be more precise / Expectation value: 2.333333
    // Standard deviation: 1.247219
    // Third central moment: 0.740741
    CHECK_EVAL_SHORT("SKEWP(A19:A23)",   KCValue(0.889048));    // TODO - be more precise /
    CHECK_EVAL("SKEW( 1; 2 )",    KCValue::errorNUM());  // At least three numbers.
}

void TestStatisticalFunctions::testSLOPE()
{
    // ODF-tests
    CHECK_EVAL("SLOPE(B4:B5;C4:C5)",     KCValue(1));           //
    CHECK_EVAL_SHORT("SLOPE(A19:A24;A26:A31)", KCValue(0.007813));    // TODO - be more precise /
}

void TestStatisticalFunctions::testSMALL()
{
    // ODF-tests
    CHECK_EVAL("SMALL(B14:B16;1)", KCValue(1));           //
    CHECK_EVAL("SMALL(B14:B16;3)", KCValue(3));           //
    CHECK_EVAL("SMALL(B14:B16;4)", KCValue::errorNUM());  // N is greater than the length of the list
}

void TestStatisticalFunctions::testSTANDARDIZE()
{
    // ODF-tests
    CHECK_EVAL("STANDARDIZE( 1; 2.5; 0.1 )", KCValue(-15));         //
    CHECK_EVAL("STANDARDIZE( -1; -2; 2 )",   KCValue(0.5));         //
    CHECK_EVAL("STANDARDIZE( 1; 1; 0 )",     KCValue::errorNUM());  // N is greater than the length of the list
}

void TestStatisticalFunctions::testSTDEV()
{
    // ODF-tests
    CHECK_EVAL("STDEV(2;4)/SQRT(2)",        KCValue(1));               // The sample standard deviation of (2;4) is SQRT(2).
    CHECK_EVAL("STDEV(B4:B5)*SQRT(2)",      KCValue(1));               // The sample standard deviation of (2;3) is 1/SQRT(2).
    CHECK_EVAL("STDEV(B3:B5)*SQRT(2)",      KCValue(1));               // Strings are not converted to numbers and are ignored.
    CHECK_EVAL("STDEV({10000000001;10000000002;"
               "10000000003;10000000004;10000000005;"
               "10000000006;10000000007;10000000008;"
               "10000000009;10000000010})", KCValue(3.0276503541));    // Ensure that implementations use a reasonably stable way of calculating STDEV.
    CHECK_EVAL("STDEV(1)",                  KCValue::errorNUM());      // At least two numbers must be included
}

void TestStatisticalFunctions::testSTDEVA()
{
    // ODF-tests
    CHECK_EVAL("STDEVA(2;4)/SQRT(2)",      KCValue(1));           // The sample standard deviation of (2;4) is SQRT(2).
    CHECK_EVAL_SHORT("STDEVA(B5:C6)",            KCValue(2.581989));    // TODO - be more precise / Logicals (referenced) are converted to numbers.
    CHECK_EVAL_SHORT("STDEVA( TRUE();FALSE() )", KCValue(0.707107));    // TODO - be more precise / Logicals (inlined) are converted to numbers.
    CHECK_EVAL("STDEVA(1)",                KCValue::errorNUM());  // Logicals (inlined) are converted to numbers.
}

void TestStatisticalFunctions::testSTDEVP()
{
    // ODF-tests
    CHECK_EVAL("STDEVP(2;4)",     KCValue(1));    // The standard deviation of the set for (2;4) is 1.
    CHECK_EVAL("STDEVP(B4:B5)*2", KCValue(1));    // The standard deviation of the set for (2;3) is 0.5.
    CHECK_EVAL("STDEVP(B3:B5)*2", KCValue(1));    // Strings are not converted to numbers and are ignored.
    CHECK_EVAL("STDEVP(1)",       KCValue(0));    // STDEVP(1) is 0.
}

void TestStatisticalFunctions::testSTDEVPA()
{
    // ODF-tests
    CHECK_EVAL("STDEVPA(2;4)",            KCValue(1));           // The sample standard deviation of (2;4) is 1.
    CHECK_EVAL_SHORT("STDEVPA(B5:C6)",          KCValue(2.236068));    // TODO - be more precise / Logicals (referenced) are converted to numbers.
    CHECK_EVAL("STDEVPA(TRUE();FALSE())", KCValue(0.5));         // Logicals (inlined) are converted to numbers.
}

void TestStatisticalFunctions::testSTEYX()
{
    // ODF-tests
    CHECK_EVAL_SHORT("STEYX(C19:C23;A19:A23)", KCValue(2.370953));    // TODO - be more precise
    CHECK_EVAL("STEYX(A19:A23;A25:A29)", KCValue(0));           //
    CHECK_EVAL("STEYX(B4:B5;C4:C5)",     KCValue::errorNUM());  // at least three number per sequence
}

void TestStatisticalFunctions::testSUMPRODUCT()
{
    CHECK_EVAL("SUMPRODUCT(C19:C23;A19:A23)", KCValue(106));
    CHECK_EVAL("SUMPRODUCT(C19:C23^2;2*A19:A23)", KCValue(820));
}

void TestStatisticalFunctions::testTDIST()
{
    // mode
    // 1 = one tailed distribution
    // 2 = two tailed distribution

    // ODF-tests
    CHECK_EVAL("TDIST( 0.5; 1; 1 )",  KCValue(0.3524163823));    // ODF-specs -> 0.352416
    CHECK_EVAL_SHORT("TDIST( -1.5; 2; 2 )", KCValue(0.272393));    //  TODO - be more precise / OOo-2.3.0 returns error!!!
    CHECK_EVAL("TDIST( 0.5; 5; 1 )",  KCValue(0.3191494358));    // ODF-specs -> 0.319149
    CHECK_EVAL("TDIST( 1; 1; 3 )",    KCValue::errorNUM());      // mode = { 1; 2 }
    CHECK_EVAL("TDIST( 1; 0; 1 )",    KCValue::errorNUM());      // degreeOfFreedom >= 1
}

void TestStatisticalFunctions::testTINV()
{
    // TODO - be more precise

    // ODF-tests
    CHECK_EVAL("TINV( 1; 2 )",               KCValue(0));           // p=1 -> t=0
    CHECK_EVAL_SHORT("TINV( 0.5; 2 )",       KCValue(0.816497));    //
    CHECK_EVAL("TDIST( TINV(0.25;3); 3;2 )", KCValue(0.25));        //
    CHECK_EVAL("TDIST( TINV(0.5 ;3); 3;2 )", KCValue(0.5));         //
    CHECK_EVAL("TDIST( TINV(0.75;3); 3;2 )", KCValue(0.75));        //
    CHECK_EVAL("TDIST( 2; 3 )",              KCValue::errorNUM());  // 0 <= probability <= 1
    CHECK_EVAL("TDIST( 1; 0 )",              KCValue::errorNUM());  // degreeOfFreedom >= 1
}

void TestStatisticalFunctions::testTREND()
{
    //  KCCell | KCValue      KCCell | KCValue
    // ------+------     ------+------
    //  A19  |   1        C19  |  0
    //  A20  |   2        C20  |  5
    //  A21  |   4        C21  |  2
    //  A22  |   8        C22  |  5
    //  A23  |  16        C23  |  3

    CHECK_ARRAY("TREND(A19:A23; C19:C23; 1)     ", "{4.7555555555}");  // with    offset
    CHECK_ARRAY("TREND(A19:A23; C19:C23; 1; 0 ) ", "{1.6825396825}");  // without offset
}

void TestStatisticalFunctions::testTRIMMEAN()
{
    // ODF-tests
    CHECK_EVAL("TRIMMEAN(A19:A23; 0.8 )",      KCValue(4));               // cutOff = 2
    CHECK_EVAL("TRIMMEAN(A19:A23; 0.6 )",      KCValue(4.6666666666));    // cutOff = FLOOR(5 * 0.6/ 2) = FLOOR(1.5) = 1;
    // result = 14 / 3
    CHECK_EVAL("TRIMMEAN(A19:A23; 0.19 )",     KCValue(6.2));             // cutOff = 0
    CHECK_EVAL("TRIMMEAN(A19:A23; 0.999999 )", KCValue(4));               // cutOff = 2
    CHECK_EVAL("TRIMMEAN(A19:A23; 1)",         KCValue::errorNUM());      // 0 <= cutOffFraction < 1
}

void TestStatisticalFunctions::testTTEST()
{
    // ODF-tests
    CHECK_EVAL("TTEST(A19:A23;A24:A28; 1; 1 )", KCValue(0.0427206184));    //
    CHECK_EVAL("TTEST(A19:A23;A24:A28; 2; 1 )", KCValue(0.0854412368));    //
    CHECK_EVAL("TTEST(A19:A23;A24:A28; 1; 2 )", KCValue(0.0294544970));    //
    CHECK_EVAL("TTEST(A19:A23;A24:A28; 1; 3 )", KCValue(0.0462125526));    //
    CHECK_EVAL("TTEST(A19:A23;A24:A29; 1; 1 )", KCValue::errorNUM());      // same amount of numbers for paired samples
    CHECK_EVAL("TTEST(A19:A19;A24:A24; 1; 3 )", KCValue::errorNUM());      // two numbers at least for each sequence
}

void TestStatisticalFunctions::testVAR()
{
    // ODF-tests
    CHECK_EVAL("VAR(2;4)",     KCValue(2));           // The sample variance of (2;4) is 2.
    CHECK_EVAL("VAR(B4:B5)*2", KCValue(1));           // The sample variance of (2;3) is 0.5.
    CHECK_EVAL("VAR(B3:B5)*2", KCValue(1));           // Strings are not converted to numbers and are ignored.
    CHECK_EVAL("VAR(1)",       KCValue::errorNUM());  // At least two numbers must be included
}

void TestStatisticalFunctions::testVARA()
{
    // ODF-tests
    CHECK_EVAL("VARA(2;4)",            KCValue(2));               // The sample variance of (2;4) is 2.
    CHECK_EVAL("VARA(B5:C6)",          KCValue(6.6666666667));    // Logicals (referenced) are converted to numbers.
    CHECK_EVAL("VARA(TRUE();FALSE())", KCValue(0.5));             // Logicals (inlined) are converted to numbers.
    CHECK_EVAL("VARA(1)",              KCValue::errorNUM());      // Two numbers at least.
}

void TestStatisticalFunctions::testVARIANCE()
{
    // same as VAR

    // ODF-tests
    CHECK_EVAL("VARIANCE(2;4)",     KCValue(2));           // The sample variance of (2;4) is 2.
    CHECK_EVAL("VARIANCE(B4:B5)*2", KCValue(1));           // The sample variance of (2;3) is 0.5.
    CHECK_EVAL("VARIANCE(B3:B5)*2", KCValue(1));           // Strings are not converted to numbers and are ignored.
    CHECK_EVAL("VARIANCE(1)",       KCValue::errorNUM());  // At least two numbers must be included
}

void TestStatisticalFunctions::testVARP()
{
    //  KCCell | KCValue
    // ------+-------
    //   B3  |  "7"
    //   B4  |   2
    //   B5  |   3

    // ODF-tests
    CHECK_EVAL("VARP(2;4)",     KCValue(1));    // The variance of the set for (2;4) is 1.
    CHECK_EVAL("VARP(B4:B5)*4", KCValue(1));    // The variance of the set for (2;3) is 0.25.
    CHECK_EVAL("VARP(B3:B5)*4", KCValue(1));    // Strings are not converted to numbers and are ignored.
}

void TestStatisticalFunctions::testVARPA()
{
    //  KCCell | KCValue      KCCell | KCValue
    // ------+------     ------+------
    //   B5  |   3         C5  |  5
    //   B6  | true        C6  |  7

    // ODF-tests
    CHECK_EVAL("VARPA(2;4)",            KCValue(1));       // The sample variance of (2;4) is 1.
    CHECK_EVAL("VARPA(B5:C6)",          KCValue(5));       // Logicals (referenced) are converted to numbers.
    CHECK_EVAL("VARPA(TRUE();FALSE())", KCValue(0.25));    // Logicals (inlined) are converted to numbers.
}

void TestStatisticalFunctions::testWEIBULL()
{
    // TODO - be more precise

    // ODF-tests
    CHECK_EVAL_SHORT("WEIBULL(  2; 3; 4; 0 )", KCValue(0.165468));    // pdf
    CHECK_EVAL_SHORT("WEIBULL(  2; 3; 4; 1 )", KCValue(0.117503));    // cdf
    CHECK_EVAL_SHORT("WEIBULL( -1; 3; 4; 0 )", KCValue::errorNUM());  // value >= 0
    CHECK_EVAL_SHORT("WEIBULL(  2; 0; 4; 0 )", KCValue::errorNUM());  // alpha > 0
    CHECK_EVAL_SHORT("WEIBULL(  2; 3; 0; 0 )", KCValue::errorNUM());  // beta > 0
}

void TestStatisticalFunctions::testZTEST()
{
    // ODF-tests
    CHECK_EVAL("ZTEST(B4:C5; 3.5      )", KCValue(0));               // mean = average, estimated standard deviation: fits well
    CHECK_EVAL("ZTEST(B4:C5; 3  ; 2   )", KCValue(0.3829249225));    // mean near average, standard deviation greater than estimate: probable
    CHECK_EVAL("ZTEST(B4:C5; 4  ; 0.5 )", KCValue(0.9544997361));    // mean near the average, but small deviation: not probable
    CHECK_EVAL("ZTEST(B4:C5; 5        )", KCValue(0.9798632484));    // mean at a border value, standard deviation ~ 1,3: nearly improbable
    CHECK_EVAL("ZTEST(B4:C5; 5  ; 0.1 )", KCValue(1));               // mean at a border value, small standard deviation: improbable
}

void TestStatisticalFunctions::cleanupTestCase()
{
    delete m_map;
}

QTEST_KDEMAIN(TestStatisticalFunctions, GUI)

#include "TestStatisticalFunctions.moc"
