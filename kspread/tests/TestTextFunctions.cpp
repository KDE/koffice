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
#include "TestTextFunctions.h"

#include "TestKspreadCommon.h"

void TestTextFunctions::initTestCase()
{
    KCFunctionModuleRegistry::instance()->loadFunctionModules();
}

#define CHECK_EVAL(x,y) { KCValue z(y); QCOMPARE(evaluate(x,z),(z)); }

KCValue TestTextFunctions::evaluate(const QString& formula, KCValue& ex)
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

void TestTextFunctions::testASC()
{
    // TODO reactivate after function is implemented
//     CHECK_EVAL( "ASC(\"ＡＢＣ\")", KCValue( "ABC" ) );
//     CHECK_EVAL( "ASC(\"アイウ\")", KCValue( "ｧｨｩ" ) );
}

void TestTextFunctions::testCHAR()
{
    CHECK_EVAL("CHAR(65)", KCValue("A"));
    CHECK_EVAL("CHAR(60)", KCValue("<"));
    CHECK_EVAL("CHAR(97)", KCValue("a"));
    CHECK_EVAL("CHAR(126)", KCValue("~"));
    CHECK_EVAL("CHAR(32)", KCValue(" "));

    // newline
    CHECK_EVAL("LEN(CHAR(10))", KCValue(1));
    // number has to be >=0
    CHECK_EVAL("CHAR(-1)", KCValue::errorNUM());
}

void TestTextFunctions::testCLEAN()
{
    CHECK_EVAL("CLEAN(\"Text\")", KCValue("Text"));
    CHECK_EVAL("CLEAN(CHAR(7)&\"Tex\"&CHAR(8)&\"t\"&CHAR(9))", KCValue("Text"));
    CHECK_EVAL("CLEAN(\"Hi there\")", KCValue("Hi there"));
}

void TestTextFunctions::testCODE()
{
    CHECK_EVAL("CODE(\"A\")", KCValue(65));
    CHECK_EVAL("CODE(\"0\")>0", KCValue(true));
    CHECK_EVAL("CODE(\"Text\")=CODE(\"T\")", KCValue(true));
}

void TestTextFunctions::testCONCATENATE()
{
    CHECK_EVAL("CONCATENATE(\"Hi \"; \"there\")", KCValue("Hi there"));
    CHECK_EVAL("CONCATENATE(\"A\"; \"B\"; \"C\")", KCValue("ABC"));
    CHECK_EVAL("CONCATENATE(2;3)", KCValue("23"));
    CHECK_EVAL("CONCATENATE(23)", KCValue("23"));
}

void TestTextFunctions::testEXACT()
{
    CHECK_EVAL("EXACT(\"A\";\"A\")",  KCValue(true));
    CHECK_EVAL("EXACT(\"A\";\"a\")",  KCValue(false));
    CHECK_EVAL("EXACT(1;1)",  KCValue(true));
    CHECK_EVAL("EXACT((1/3)*3;1)",  KCValue(true));
    CHECK_EVAL("EXACT(TRUE();TRUE())",  KCValue(true));
    CHECK_EVAL("EXACT(\"1\";2)",  KCValue(false));
    CHECK_EVAL("EXACT(\"h\";1)",  KCValue(false));
    CHECK_EVAL("EXACT(\"1\";1)",  KCValue(true));
    CHECK_EVAL("EXACT(\" 1\";1)",  KCValue(false));
}

void TestTextFunctions::testFIND()
{
    CHECK_EVAL("FIND(\"b\";\"abcabc\")", KCValue(2));
    CHECK_EVAL("FIND(\"b\";\"abcabcabc\"; 3)", KCValue(5));
    CHECK_EVAL("FIND(\"b\";\"ABC\";1)", KCValue::errorVALUE());
    CHECK_EVAL("FIND(\"b\";\"bbbb\")", KCValue(1));
    CHECK_EVAL("FIND(\"b\";\"bbbb\";2)", KCValue(2));
    CHECK_EVAL("FIND(\"b\";\"bbbb\";2.9)", KCValue(2));
    CHECK_EVAL("FIND(\"b\";\"bbbb\";0)", KCValue::errorVALUE());
    CHECK_EVAL("FIND(\"b\";\"bbbb\";0.9)", KCValue::errorVALUE());
}

void TestTextFunctions::testFIXED()
{
    CHECK_EVAL("FIXED(12345;3)", KCValue("12,345.000"));
    CHECK_EVAL("ISTEXT(FIXED(12345;3))", KCValue(true));
    CHECK_EVAL("FIXED(12345;3;FALSE())", KCValue("12,345.000"));
    CHECK_EVAL("FIXED(12345;3.95;FALSE())", KCValue("12,345.000"));
    CHECK_EVAL("FIXED(12345;4;TRUE())", KCValue("12345.0000"));
    CHECK_EVAL("FIXED(123.45;1)", KCValue("123.5"));
    CHECK_EVAL("FIXED(125.45; -1)", KCValue("130"));
    CHECK_EVAL("FIXED(125.45; -1.1)", KCValue("130"));
    CHECK_EVAL("FIXED(125.45; -1.9)", KCValue("130"));
    CHECK_EVAL("FIXED(125.45; -2)", KCValue("100"));
    CHECK_EVAL("FIXED(125.45; -2.87)", KCValue("100"));
    CHECK_EVAL("FIXED(125.45; -3)", KCValue("0"));
    CHECK_EVAL("FIXED(125.45; -4)", KCValue("0"));
    CHECK_EVAL("FIXED(125.45; -5)", KCValue("0"));
}

void TestTextFunctions::testJIS()
{
    // TODO reactivate after function is implemented
//     CHECK_EVAL( "JIS(\"ABC\")", KCValue( "ＡＢＣ") );
//     CHECK_EVAL( "JIS(\"ｧｨｩ\")", KCValue( "アイウ" ) );
}

void TestTextFunctions::testLEFT()
{
    CHECK_EVAL("LEFT(\"Hello\";2)", KCValue("He"));
    CHECK_EVAL("LEFT(\"Hello\")", KCValue("H"));
    CHECK_EVAL("LEFT(\"Hello\";20)", KCValue("Hello"));
    CHECK_EVAL("LEFT(\"Hello\";0)", KCValue(""));
    CHECK_EVAL("LEFT(\"\";4)", KCValue(""));
    CHECK_EVAL("LEFT(\"xxx\";-0.1)", KCValue::errorVALUE());
    CHECK_EVAL("LEFT(\"Hello\";2^15-1)", KCValue("Hello"));
    CHECK_EVAL("LEFT(\"Hello\";2.9)", KCValue("He"));
}

void TestTextFunctions::testLEN()
{
    CHECK_EVAL("LEN(\"Hi there\")", KCValue(8));
    CHECK_EVAL("LEN(\"\")", KCValue(0));
    CHECK_EVAL("LEN(55)", KCValue(2));
}

void TestTextFunctions::testLOWER()
{
    CHECK_EVAL("LOWER(\"HELLObc7\")", KCValue("hellobc7"));
}

void TestTextFunctions::testMID()
{
    CHECK_EVAL("MID(\"123456789\";5;3)", KCValue("567"));
    CHECK_EVAL("MID(\"123456789\";20;3)", KCValue(""));
    CHECK_EVAL("MID(\"123456789\";-1;0)", KCValue::errorVALUE());
    CHECK_EVAL("MID(\"123456789\";1;0)", KCValue(""));
    CHECK_EVAL("MID(\"123456789\";2.9;1)", KCValue("2"));
    CHECK_EVAL("MID(\"123456789\";2;2.9)", KCValue("23"));
    CHECK_EVAL("MID(\"123456789\";5)", KCValue("56789"));
}

void TestTextFunctions::testPROPER()
{
    CHECK_EVAL("PROPER(\"hello there\")", KCValue("Hello There"));
    CHECK_EVAL("PROPER(\"HELLO THERE\")", KCValue("Hello There"));
    CHECK_EVAL("PROPER(\"HELLO.THERE\")", KCValue("Hello.There"));
}

void TestTextFunctions::testREPLACE()
{
    CHECK_EVAL("REPLACE(\"123456789\";5;3;\"Q\")", KCValue("1234Q89"));
    CHECK_EVAL("REPLACE(\"123456789\";5;0;\"Q\")", KCValue("1234Q56789"));
}

void TestTextFunctions::testREPT()
{
    CHECK_EVAL("REPT(\"X\";3)",  KCValue("XXX"));
    CHECK_EVAL("REPT(\"XY\";2)",  KCValue("XYXY"));
    CHECK_EVAL("REPT(\"X\";2.9)",  KCValue("XX"));
    CHECK_EVAL("REPT(\"XY\";2.9)",  KCValue("XYXY"));
    CHECK_EVAL("REPT(\"X\";0)",  KCValue(""));
    CHECK_EVAL("REPT(\"XYZ\";0)",  KCValue(""));
    CHECK_EVAL("REPT(\"X\";-1)",  KCValue::errorVALUE());
    CHECK_EVAL("REPT(\"XYZ\";-0.1)",  KCValue::errorVALUE());
}

void TestTextFunctions::testRIGHT()
{
    CHECK_EVAL("RIGHT(\"Hello\";2)", KCValue("lo"));
    CHECK_EVAL("RIGHT(\"Hello\")", KCValue("o"));
    CHECK_EVAL("RIGHT(\"Hello\";20)", KCValue("Hello"));
    CHECK_EVAL("RIGHT(\"Hello\";0)", KCValue(""));
    CHECK_EVAL("RIGHT(\"\";4)", KCValue(""));
    CHECK_EVAL("RIGHT(\"xxx\";-1)", KCValue::errorVALUE());
    CHECK_EVAL("RIGHT(\"xxx\";-0.1)", KCValue::errorVALUE());
    CHECK_EVAL("RIGHT(\"Hello\";2^15-1)", KCValue("Hello"));
    CHECK_EVAL("RIGHT(\"Hello\";2.9)", KCValue("lo"));
}

void TestTextFunctions::testSEARCH()
{
    CHECK_EVAL("=SEARCH(\"b\";\"abcabc\")", KCValue(2));
    CHECK_EVAL("=SEARCH(\"b\";\"abcabcabc\"; 3)", KCValue(5));
    CHECK_EVAL("=SEARCH(\"b\";\"ABC\";1)", KCValue(2));
    CHECK_EVAL("=SEARCH(\"c?a\";\"abcabcda\")", KCValue(6));
    CHECK_EVAL("=SEARCH(\"e*o\";\"yes and no\")", KCValue(2));
    CHECK_EVAL("=SEARCH(\"b*c\";\"abcabcabc\")", KCValue(2));
}

void TestTextFunctions::testSUBSTITUTE()
{
    CHECK_EVAL("SUBSTITUTE(\"121212\";\"2\";\"ab\")", KCValue("1ab1ab1ab"));
    CHECK_EVAL("SUBSTITUTE(\"121212\";\"2\";\"ab\";2)", KCValue("121ab12"));
    CHECK_EVAL("SUBSTITUTE(\"Hello\";\"x\";\"ab\")", KCValue("Hello"));
    CHECK_EVAL("SUBSTITUTE(\"xyz\";\"\";\"ab\")", KCValue("xyz"));
    CHECK_EVAL("SUBSTITUTE(\"\";\"\";\"ab\")", KCValue(""));
    CHECK_EVAL("SUBSTITUTE(\"Hello\"; \"H\"; \"J\"; 0)", KCValue::errorVALUE());
    CHECK_EVAL("SUBSTITUTE(\"Hello\"; \"H\"; \"J\"; 1)", KCValue("Jello"));
}

void TestTextFunctions::testT()
{
    CHECK_EVAL("T(\"Hi\")", KCValue("Hi"));
    CHECK_EVAL("T(5)",      KCValue(""));
}

void TestTextFunctions::testTRIM()
{
    CHECK_EVAL("TRIM(\" Hi \")", KCValue("Hi"));
    CHECK_EVAL("LEN(TRIM(\"H\" & \" \" & \" \" & \"I\"))", KCValue(3));
}

void TestTextFunctions::testUNICHAR()
{
    CHECK_EVAL("UNICHAR(65)", KCValue("A"));
    CHECK_EVAL("UNICHAR(8364)", KCValue(QChar(8364)));
}

void TestTextFunctions::testUNICODE()
{
    QChar euro(8364);

    CHECK_EVAL("UNICODE(\"A\")", KCValue(65));
    CHECK_EVAL("UNICODE(\"AB€C\")", KCValue(65));
    CHECK_EVAL(QString("UNICODE(\"%1\")").arg(euro), KCValue(8364));
    CHECK_EVAL(QString("UNICODE(\"%1F\")").arg(euro), KCValue(8364));
}

void TestTextFunctions::testUPPER()
{
    CHECK_EVAL("UPPER(\"Habc7\")", KCValue("HABC7"));
}

void TestTextFunctions::testROT13()
{
    CHECK_EVAL("ROT13(\"KCells\")", KCValue("XFcernq"));
    CHECK_EVAL("ROT13(\"XFcernq\")", KCValue("KCells"));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETROT13(\"KCells\")", KCValue("XFcernq"));
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GETROT13(\"XFcernq\")", KCValue("KCells"));
}

void TestTextFunctions::testBAHTTEXT()
{
    KCValue r;
    r = evaluate("BAHTTEXT(23)", r);
    CHECK_EVAL("BAHTTEXT(23)", r);
    CHECK_EVAL("COM.MICROSOFT.BAHTTEXT(23)", r);
}

QTEST_KDEMAIN(TestTextFunctions, GUI)

#include "TestTextFunctions.moc"
