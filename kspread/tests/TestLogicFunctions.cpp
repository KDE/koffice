/* This file is part of the KDE project
   Copyright 2007 Brad Hards <bradh@frogmouth.net>

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
#include "TestLogicFunctions.h"

#include "TestKspreadCommon.h"

#include <part/Doc.h>
#include <KCMap.h>
#include <KCSheet.h>

void TestLogicFunctions::init()
{
    m_doc = new Doc();
    m_doc->map()->addNewSheet();
    m_sheet = m_doc->map()->sheet(0);
}

void TestLogicFunctions::cleanup()
{
    delete m_doc;
}

void TestLogicFunctions::initTestCase()
{
    FunctionModuleRegistry::instance()->loadFunctionModules();
}

// because we may need to promote expected value from integer to float
#define CHECK_EVAL(x,y) { KCValue z(y); QCOMPARE(evaluate(x,z),(z)); }

KCValue TestLogicFunctions::evaluate(const QString& formula, KCValue& ex)
{
    KCFormula f(m_sheet);
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

void TestLogicFunctions::testAND()
{
    CHECK_EVAL("AND(FALSE();FALSE())", KCValue(false));
    CHECK_EVAL("AND(FALSE();TRUE())", KCValue(false));
    CHECK_EVAL("AND(TRUE();FALSE())", KCValue(false));
    CHECK_EVAL("AND(TRUE();TRUE())", KCValue(true));
    // errors propogate
    CHECK_EVAL("AND(TRUE();NA())", KCValue::errorNA());
    CHECK_EVAL("AND(NA();TRUE())", KCValue::errorNA());
    // Nonzero considered TRUE
    CHECK_EVAL("AND(1;TRUE())", KCValue(true));
    CHECK_EVAL("AND(2;TRUE())", KCValue(true));
    // zero considered false
    CHECK_EVAL("AND(0;TRUE())", KCValue(false));
    // multiple parameters...
    CHECK_EVAL("AND(TRUE();TRUE();TRUE())", KCValue(true));
    CHECK_EVAL("AND(TRUE();TRUE();FALSE())", KCValue(false));
    CHECK_EVAL("AND(FALSE();TRUE();TRUE())", KCValue(false));
    CHECK_EVAL("AND(TRUE();FALSE();TRUE())", KCValue(false));
    CHECK_EVAL("AND(TRUE();FALSE();FALSE())", KCValue(false));
    // single parameter
    CHECK_EVAL("AND(TRUE())", KCValue(true));
    CHECK_EVAL("AND(FALSE())", KCValue(false));

    // literal non-convertable text should give an error
    //CHECK_EVAL("AND(FALSE();\"a\")", KCValue::errorVALUE());
}

void TestLogicFunctions::testFALSE()
{
    CHECK_EVAL("FALSE()", KCValue(false));
    // Applications that implement logical values as 0/1 must map FALSE() to 0
    CHECK_EVAL("IF(ISNUMBER(FALSE());FALSE()=0;FALSE())", KCValue(false));
    // note that kspread distinguishes between boolean and math
    CHECK_EVAL("FALSE()=0", KCValue(false));
    CHECK_EVAL("FALSE()=1", KCValue(false));
    // False converts to 0 in KCNumber context
    CHECK_EVAL("2+FALSE()", KCValue(2));
}

void TestLogicFunctions::testIF()
{
    CHECK_EVAL("IF(FALSE();7;8)", KCValue(8));
    CHECK_EVAL("IF(TRUE();7;8)", KCValue(7));
    CHECK_EVAL("IF(FALSE();7.1;8.2)", KCValue(8.2));
    CHECK_EVAL("IF(TRUE();7.1;8.2)", KCValue(7.1));
    CHECK_EVAL("IF(TRUE();\"HI\";8)", KCValue("HI"));
    CHECK_EVAL("IF(1;7;8)", KCValue(7));
    CHECK_EVAL("IF(5;7;8)", KCValue(7));
    CHECK_EVAL("IF(0;7;8)", KCValue(8));
    // there are a couple of indirect references in the spec test
    // vectors here. Sorry
    CHECK_EVAL("IF(\"x\";7;8)", KCValue::errorVALUE());
    CHECK_EVAL("IF(\"1\";7;8)", KCValue::errorVALUE());
    CHECK_EVAL("IF(\"\";7;8)", KCValue::errorVALUE());
    CHECK_EVAL("IF(FALSE();7)", KCValue(false));
    CHECK_EVAL("IF(FALSE();7;)", KCValue(0));
    // Assuming A1 is an empty cell, using it in the following
    // context should be different from passing no argument at all
    CHECK_EVAL("IF(FALSE();7;A1)", KCValue(KCValue::Empty));
    CHECK_EVAL("IF(TRUE();4;1/0)", KCValue(4));
    CHECK_EVAL("IF(FALSE();1/0;5)", KCValue(5));
}

void TestLogicFunctions::testNOT()
{
    CHECK_EVAL("NOT(FALSE())", KCValue(true));
    CHECK_EVAL("NOT(TRUE())", KCValue(false));
    CHECK_EVAL("NOT(1/0)", KCValue::errorDIV0());
    CHECK_EVAL("NOT(\"a\")", KCValue::errorVALUE());
}

void TestLogicFunctions::testOR()
{
    CHECK_EVAL("OR(FALSE();FALSE())", KCValue(false));
    CHECK_EVAL("OR(FALSE();TRUE())", KCValue(true));
    CHECK_EVAL("OR(TRUE();FALSE())", KCValue(true));
    CHECK_EVAL("OR(TRUE();TRUE())", KCValue(true));
    // errors propogate
    CHECK_EVAL("OR(TRUE();NA())", KCValue::errorNA());
    CHECK_EVAL("OR(NA();TRUE())", KCValue::errorNA());
    // Nonzero considered TRUE
    CHECK_EVAL("OR(1;TRUE())", KCValue(true));
    CHECK_EVAL("OR(2;TRUE())", KCValue(true));
    // zero considered false
    CHECK_EVAL("OR(0;TRUE())", KCValue(true));
    CHECK_EVAL("OR(0;1)", KCValue(true));
    CHECK_EVAL("OR(0;0)", KCValue(false));
    // multiple parameters...
    CHECK_EVAL("OR(TRUE();TRUE();TRUE())", KCValue(true));
    CHECK_EVAL("OR(FALSE();FALSE();FALSE())", KCValue(false));
    CHECK_EVAL("OR(TRUE();TRUE();FALSE())", KCValue(true));
    CHECK_EVAL("OR(FALSE();TRUE();TRUE())", KCValue(true));
    CHECK_EVAL("OR(TRUE();FALSE();TRUE())", KCValue(true));
    CHECK_EVAL("OR(TRUE();FALSE();FALSE())", KCValue(true));
    // single parameter
    CHECK_EVAL("OR(TRUE())", KCValue(true));
    CHECK_EVAL("OR(FALSE())", KCValue(false));

    // literal non-convertable text should give an error
    //CHECK_EVAL("OR(TRUE();\"a\")", KCValue::errorVALUE());
}

void TestLogicFunctions::testTRUE()
{
    CHECK_EVAL("TRUE()", KCValue(true));
    // Applications that implement logical values as 0/1 must map TRUE() to 1
    CHECK_EVAL("IF(ISNUMBER(TRUE());TRUE()=0;TRUE())", KCValue(true));
    // note that kspread distinguishes between boolean and math
    CHECK_EVAL("TRUE()=1", KCValue(false));
    CHECK_EVAL("TRUE()=0", KCValue(false));
    // False converts to 0 in KCNumber context
    CHECK_EVAL("2+TRUE()", KCValue(3));
}

void TestLogicFunctions::testXOR()
{
    CHECK_EVAL("XOR(FALSE();FALSE())", KCValue(false));
    CHECK_EVAL("XOR(FALSE();TRUE())", KCValue(true));
    CHECK_EVAL("XOR(TRUE();FALSE())", KCValue(true));
    CHECK_EVAL("XOR(TRUE();TRUE())", KCValue(false));
    // errors propogate
    CHECK_EVAL("XOR(TRUE();NA())", KCValue::errorNA());
    CHECK_EVAL("XOR(NA();TRUE())", KCValue::errorNA());
    CHECK_EVAL("XOR(FALSE();NA())", KCValue::errorNA());
    CHECK_EVAL("XOR(NA();FALSE())", KCValue::errorNA());
    // Nonzero considered TRUE
    CHECK_EVAL("XOR(1;TRUE())", KCValue(false));
    CHECK_EVAL("XOR(3;4)", KCValue(false));
    CHECK_EVAL("XOR(2;TRUE())", KCValue(false));
    CHECK_EVAL("XOR(FALSE();1)", KCValue(true));
    CHECK_EVAL("XOR(2;FALSE())", KCValue(true));
    // zero considered false
    CHECK_EVAL("XOR(0;TRUE())", KCValue(true));
    CHECK_EVAL("XOR(0;1)", KCValue(true));
    CHECK_EVAL("XOR(0;0)", KCValue(false));
    // multiple parameters...
    CHECK_EVAL("XOR(TRUE();TRUE();TRUE())", KCValue(false));
    CHECK_EVAL("XOR(FALSE();FALSE();FALSE())", KCValue(false));
    CHECK_EVAL("XOR(TRUE();TRUE();FALSE())", KCValue(false));
    CHECK_EVAL("XOR(FALSE();TRUE();TRUE())", KCValue(false));
    CHECK_EVAL("XOR(TRUE();FALSE();TRUE())", KCValue(false));
    CHECK_EVAL("XOR(TRUE();FALSE();FALSE())", KCValue(true));
    CHECK_EVAL("XOR(FALSE();FALSE();TRUE())", KCValue(true));
    CHECK_EVAL("XOR(FALSE();FALSE();TRUE();FALSE())", KCValue(true));
    // single parameter
    CHECK_EVAL("XOR(TRUE())", KCValue(true));
    CHECK_EVAL("XOR(FALSE())", KCValue(false));

    // literal non-convertable text should give an error
    //CHECK_EVAL("XOR(TRUE();\"a\")", KCValue::errorVALUE());
}

QTEST_KDEMAIN(TestLogicFunctions, GUI)

#include "TestLogicFunctions.moc"
