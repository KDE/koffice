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
#include "TestBitopsFunctions.h"

#include "TestKspreadCommon.h"

#include <KStandardDirs>

void TestBitopsFunctions::initTestCase()
{
    FunctionModuleRegistry::instance()->loadFunctionModules();
}

// because we may need to promote expected value from integer to float
#define CHECK_EVAL(x,y) { KCValue z(y); QCOMPARE(evaluate(x,z),(z)); }

KCValue TestBitopsFunctions::evaluate(const QString& formula, KCValue& ex)
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

void TestBitopsFunctions::testBITAND()
{
    // basic check of all four bit combinations
    CHECK_EVAL("BITAND(12;10)", KCValue(8));
    // test using an all-zero combo
    CHECK_EVAL("BITAND(7;0)", KCValue(0));
    // test of 31-bit value
    CHECK_EVAL("BITAND(2147483641; 2147483637)", KCValue(2147483633));
    // test of 32-bit value
    CHECK_EVAL("BITAND(4294967289.0; 4294967285.0)", KCValue(4294967281LL));
    // test of 32-bit value
    CHECK_EVAL("BITAND(4294967289; 4294967285)", KCValue(4294967281LL));
    // test of 48 bit value
    CHECK_EVAL("BITAND(281474976710649 ; 281474976710645)",  KCValue(281474976710641LL));
    // test of 48 bit value
    CHECK_EVAL("BITAND(281474976710655; 281474976710655)",  KCValue(281474976710655LL));
    // test of 48 bit value
    CHECK_EVAL("BITAND(281474976710655; 281474976710655)<>281474976710656", KCValue(true));
}


void TestBitopsFunctions::testBITOR()
{
    // basic check of all four bit combinations
    CHECK_EVAL("BITOR(12;10)", KCValue(14));
    // test using an all-zero combo
    CHECK_EVAL("BITOR(7;0)", KCValue(7));
    // test of 31-bit value
    CHECK_EVAL("BITOR(2147483641; 2147483637)", KCValue(2147483645));
    // test of 32-bit value
    CHECK_EVAL("BITOR(4294967289.0; 4294967285.0)", KCValue(4294967293LL));
    // test of 32-bit value
    CHECK_EVAL("BITOR(4294967289; 4294967285)", KCValue(4294967293LL));
    // test of 48 bit value
    CHECK_EVAL("BITOR(281474976710649; 281474976710645)",  KCValue(281474976710653LL));
    // test of 48 bit value
    CHECK_EVAL("BITOR(281474976710655; 281474976710655)",  KCValue(281474976710655LL));
    // test of 48 bit value
    CHECK_EVAL("BITOR(281474976710655; 281474976710655)<>281474976710656", KCValue(true));
}

void TestBitopsFunctions::testBITXOR()
{
    // basic check of all four bit combinations
    CHECK_EVAL("BITXOR(12;10)", KCValue(6));
    // test using an all-zero combo
    CHECK_EVAL("BITXOR(7;0)", KCValue(7));
    // test of 31-bit value
    CHECK_EVAL("BITXOR(2147483641; 2)", KCValue(2147483643));
    // test of 32-bit value
    CHECK_EVAL("BITXOR(4294967289.0; 2.0)", KCValue(4294967291LL));
    // test of 32-bit value
    CHECK_EVAL("BITXOR(4294967289; 2)", KCValue(4294967291LL));
    // test of 48 bit value
    CHECK_EVAL("BITXOR(281474976710649 ; 2)",  KCValue(281474976710651LL));
    // test of 48 bit value
    CHECK_EVAL("BITXOR(281474976710655; 0)",  KCValue(281474976710655LL));
    // test of 48 bit value
    CHECK_EVAL("BITXOR(281474976710655; 0)<>281474976710656", KCValue(true));
}

void TestBitopsFunctions::testBITLSHIFT()
{
    CHECK_EVAL("BITLSHIFT(63;2)", KCValue(252));
    CHECK_EVAL("BITLSHIFT(63;0)", KCValue(63));
    CHECK_EVAL("BITLSHIFT(63;-2)", KCValue(15));
    CHECK_EVAL("BITLSHIFT(1;47)", KCValue(140737488355328LL));
    // test for 31 bits
    CHECK_EVAL("BITLSHIFT(2147483641; 0)", KCValue(2147483641LL));
    // test for 32 bits
    CHECK_EVAL("BITLSHIFT(4294967289; 0)", KCValue(4294967289LL));
    // test for 48 bits
    CHECK_EVAL("BITLSHIFT(281474976710649; 0)", KCValue(281474976710649LL));
}

void TestBitopsFunctions::testBITRSHIFT()
{
    CHECK_EVAL("BITRSHIFT(63;2)", KCValue(15));
    CHECK_EVAL("BITRSHIFT(63;0)", KCValue(63));
    CHECK_EVAL("BITRSHIFT(63;-2)", KCValue(252));
    CHECK_EVAL("BITRSHIFT(63;48)", KCValue(0));
    // test for 31 bits
    CHECK_EVAL("BITRSHIFT(2147483641; 0)", KCValue(2147483641LL));
    // test for 32 bits
    CHECK_EVAL("BITRSHIFT(4294967289; 0)", KCValue(4294967289LL));
    // test for 48 bits
    CHECK_EVAL("BITRSHIFT(281474976710649 ; 0)", KCValue(281474976710649LL));
}

QTEST_KDEMAIN(TestBitopsFunctions, GUI)

#include "TestBitopsFunctions.moc"
