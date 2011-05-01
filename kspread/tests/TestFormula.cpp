/* This file is part of the KDE project
   Copyright 2004,2007 Ariya Hidayat <ariya@kde.org>

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
#include "TestFormula.h"

#include <klocale.h>

#include "TestKspreadCommon.h"

static char encodeTokenType(const KCToken& token)
{
    char result = '?';
    switch (token.type()) {
    case KCToken::Boolean:    result = 'b'; break;
    case KCToken::Integer:    result = 'i'; break;
    case KCToken::Float:      result = 'f'; break;
    case KCToken::Operator:   result = 'o'; break;
    case KCToken::KCCell:       result = 'c'; break;
    case KCToken::Range:      result = 'r'; break;
    case KCToken::Identifier: result = 'x'; break;
    default: break;
    }
    return result;
}

#if 0 // not used?
static QString describeTokenCodes(const QString& tokenCodes)
{
    QString result;

    if (tokenCodes.isEmpty())
        result = "(invalid)";
    else
        for (int i = 0; i < tokenCodes.length(); i++) {
            switch (tokenCodes[i].unicode()) {
            case 'b': result.append("Boolean"); break;
            case 'i': result.append("integer"); break;
            case 'f': result.append("float"); break;
            case 'o': result.append("operator"); break;
            case 'c': result.append("cell"); break;
            case 'r': result.append("range"); break;
            case 'x': result.append("identifier"); break;
            default:  result.append("unknown"); break;
            }
            if (i < tokenCodes.length() - 1) result.append(", ");
        }

    return result.prepend("{").append("}");
}
#endif

#define CHECK_TOKENIZE(x,y) QCOMPARE(tokenizeFormula(x), QString(y))

static QString tokenizeFormula(const QString& formula)
{
    KCFormula f;
    QString expr = formula;
    expr.prepend('=');
    f.setExpression(expr);
    Tokens tokens = f.tokens();

    QString resultCodes;
    if (tokens.valid())
        for (int i = 0; i < tokens.count(); i++)
            resultCodes.append(encodeTokenType(tokens[i]));

    return resultCodes;
}


// because we may need to promote expected value from integer to float
#define CHECK_EVAL(x,y) { KCValue z(y); QCOMPARE(evaluate(x,z),(z)); }

KCValue TestFormula::evaluate(const QString& formula, KCValue& ex)
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

void TestFormula::initTestCase()
{
    KCFunctionModuleRegistry::instance()->loadFunctionModules();
}

void TestFormula::testTokenizer()
{
    // simple, single-token formulas
    CHECK_TOKENIZE("True", "x");
    CHECK_TOKENIZE("False", "x");
    CHECK_TOKENIZE("36", "i");
    CHECK_TOKENIZE("0", "i");
    CHECK_TOKENIZE("3.14159", "f");
    CHECK_TOKENIZE(".25", "f");
    CHECK_TOKENIZE("1e-9", "f");
    CHECK_TOKENIZE("2e3", "f");
    CHECK_TOKENIZE(".3333e0", "f");

    // cell/range/identifier
    CHECK_TOKENIZE("A1", "c");
    CHECK_TOKENIZE("Sheet1!A1", "c");
    CHECK_TOKENIZE("'Sheet1'!A1", "c");
    CHECK_TOKENIZE("'KCSheet One'!A1", "c");
    CHECK_TOKENIZE("2006!A1", "c");
    CHECK_TOKENIZE("2006bak!A1", "c");
    CHECK_TOKENIZE("2006bak2!A1", "c");
    CHECK_TOKENIZE("'2006bak2'!A1", "c");
    CHECK_TOKENIZE("A1:B100", "r");
    CHECK_TOKENIZE("Sheet1!A1:B100", "r");
    CHECK_TOKENIZE("'KCSheet One'!A1:B100", "r");
    CHECK_TOKENIZE("SIN", "x");

    // log2 and log10 are cell references and function identifiers
    CHECK_TOKENIZE("LOG2", "c");
    CHECK_TOKENIZE("LOG10:11", "r");
    CHECK_TOKENIZE("LOG2(2)", "xoio");
    CHECK_TOKENIZE("LOG10(10)", "xoio");

    // operators
    CHECK_TOKENIZE("+", "o");
    CHECK_TOKENIZE("-", "o");
    CHECK_TOKENIZE("*", "o");
    CHECK_TOKENIZE("/", "o");
    CHECK_TOKENIZE("+", "o");
    CHECK_TOKENIZE("^", "o");
    CHECK_TOKENIZE("(", "o");
    CHECK_TOKENIZE(")", "o");
    CHECK_TOKENIZE(",", "o");
    CHECK_TOKENIZE(";", "o");
    CHECK_TOKENIZE("=", "o");
    CHECK_TOKENIZE("<", "o");
    CHECK_TOKENIZE(">", "o");
    CHECK_TOKENIZE("<=", "o");
    CHECK_TOKENIZE(">=", "o");
    CHECK_TOKENIZE("%", "o");

    // commonly used formulas
    CHECK_TOKENIZE("A1+A2", "coc");
    CHECK_TOKENIZE("2.5*B1", "foc");
    CHECK_TOKENIZE("SUM(A1:Z10)", "xoro");
    CHECK_TOKENIZE("MAX(Sheet1!Sales)", "xoro");
    CHECK_TOKENIZE("-ABS(A1)", "oxoco");

    // should be correctly parsed though they are nonsense (can't be evaluated)
    CHECK_TOKENIZE("0E0.5", "ff");
    CHECK_TOKENIZE("B3 D4:D5 Sheet1!K1", "crc");
    CHECK_TOKENIZE("SIN A1", "xc");
    CHECK_TOKENIZE("SIN A1:A20", "xr");

    // invalid formulas, can't be parsed correctly
    CHECK_TOKENIZE("+1.23E", QString());

    // empty parameter
    CHECK_TOKENIZE("IF(A1;A2;)", "xococoo");

    // function cascade
    CHECK_TOKENIZE("SUM(ABS(-1);ABS(-1))", "xoxooiooxooioo");
}

void TestFormula::testConstant()
{
    // simple constants
    CHECK_EVAL("0", KCValue(0));
    CHECK_EVAL("1", KCValue(1));
    CHECK_EVAL("-1", KCValue(-1));
    CHECK_EVAL("3.14e7", KCValue(3.14e7));
    CHECK_EVAL("3.14e-7", KCValue(3.14e-7));

    // String constants (from Odf 1.2 spec)
    CHECK_EVAL("\"Hi\"", KCValue("Hi"));
    CHECK_EVAL("\"Hi\"\"t\"", KCValue("Hi\"t"));
    CHECK_EVAL("\"\\n\"", KCValue("\\n"));

    // Constant errors
    CHECK_EVAL("#N/A", KCValue::errorNA());
    CHECK_EVAL("#DIV/0!", KCValue::errorDIV0());
    CHECK_EVAL("#NAME?", KCValue::errorNAME());
    CHECK_EVAL("#NULL!", KCValue::errorNULL());
    CHECK_EVAL("#NUM!", KCValue::errorNUM());
    CHECK_EVAL("#REF!", KCValue::errorREF());
    CHECK_EVAL("#VALUE!", KCValue::errorVALUE());

}

void TestFormula::testWhitespace()
{
    CHECK_EVAL("=ROUND(  10.1  ;  0  )", KCValue(10));
    CHECK_EVAL("= ROUND(10.1;0)", KCValue(10));
    CHECK_EVAL(" =ROUND(10.1;0)", KCValue::errorPARSE());
    CHECK_EVAL("= ( ROUND( 9.8 ; 0 )  +  ROUND( 9.8 ; 0 ) ) ", KCValue(20));
    CHECK_EVAL("=(ROUND(9.8;0) ROUND(9.8;0))", KCValue::errorPARSE());
}

void TestFormula::testInvalid()
{
    // Basic operations always throw errors if one of the values
    // is invalid. This is the difference to SUM and co.
    CHECK_EVAL("a+0", KCValue::errorVALUE());
    CHECK_EVAL("0-z", KCValue::errorVALUE());
    CHECK_EVAL("a*b", KCValue::errorVALUE());
    CHECK_EVAL("u/2", KCValue::errorVALUE());
}

void TestFormula::testUnary()
{
    // unary minus
    CHECK_EVAL("-1", KCValue(-1));
    CHECK_EVAL("--1", KCValue(1));
    CHECK_EVAL("---1", KCValue(-1));
    CHECK_EVAL("----1", KCValue(1));
    CHECK_EVAL("-----1", KCValue(-1));
    CHECK_EVAL("5-1", KCValue(4));
    CHECK_EVAL("5--1", KCValue(6));
    CHECK_EVAL("5---1", KCValue(4));
    CHECK_EVAL("5----1", KCValue(6));
    CHECK_EVAL("5-----1", KCValue(4));
    CHECK_EVAL("5-----1*2.5", KCValue(2.5));
    CHECK_EVAL("5------1*2.5", KCValue(7.5));
    CHECK_EVAL("-SIN(0)", KCValue(0));
    CHECK_EVAL("1.1-SIN(0)", KCValue(1.1));
    CHECK_EVAL("1.2--SIN(0)", KCValue(1.2));
    CHECK_EVAL("1.3---SIN(0)", KCValue(1.3));
    CHECK_EVAL("-COS(0)", KCValue(-1));
    CHECK_EVAL("1.1-COS(0)", KCValue(0.1));
    CHECK_EVAL("1.2--COS(0)", KCValue(2.2));
    CHECK_EVAL("1.3---COS(0)", KCValue(0.3));
}

void TestFormula::testBinary()
{
    // simple binary operation
    CHECK_EVAL("0+0", KCValue(0));
    CHECK_EVAL("1+1", KCValue(2));

    // power operator is left associative
    CHECK_EVAL("2^3", KCValue(8));
    CHECK_EVAL("2^3^2", KCValue(64));

    // lead to division by zero
    CHECK_EVAL("0/0", KCValue::errorDIV0());
    CHECK_EVAL("1/0", KCValue::errorDIV0());
    CHECK_EVAL("-4/0", KCValue::errorDIV0());
    CHECK_EVAL("(2*3)/(6-2*3)", KCValue::errorDIV0());
    CHECK_EVAL("1e3+7/0", KCValue::errorDIV0());
    CHECK_EVAL("2^(99/0)", KCValue::errorDIV0());

}

void TestFormula::testOperators()
{
    // no parentheses, checking operator precendences
    CHECK_EVAL("14+3*77", KCValue(245));
    CHECK_EVAL("14-3*77", KCValue(-217));
    CHECK_EVAL("26*4+81", KCValue(185));
    CHECK_EVAL("26*4-81", KCValue(23));
    CHECK_EVAL("30-45/3", KCValue(15));
    CHECK_EVAL("45+45/3", KCValue(60));
    CHECK_EVAL("4+3*2-1", KCValue(9));
}

void TestFormula::testComparison()
{
    // compare numbers
    CHECK_EVAL("6>5", KCValue(true));
    CHECK_EVAL("6<5", KCValue(false));
    CHECK_EVAL("2=2", KCValue(true));
    CHECK_EVAL("2=22", KCValue(false));
    CHECK_EVAL("=3=3.0001", KCValue(false));
    // compare booleans
    CHECK_EVAL("=TRUE()=FALSE()", KCValue(false));
    CHECK_EVAL("=TRUE()=TRUE()", KCValue(true));
    CHECK_EVAL("=FALSE()=FALSE()", KCValue(true));
    // compare strings
    CHECK_EVAL("=\"Hi\"=\"Bye\"", KCValue(false));
    CHECK_EVAL("=\"5\"=5", KCValue(false));
    CHECK_EVAL("=\"Hi\"=\"HI\"", KCValue(false));
    CHECK_EVAL("b>a", KCValue(true));
    CHECK_EVAL("b<aa", KCValue(false));
    CHECK_EVAL("c<d", KCValue(true));
    CHECK_EVAL("cc>d", KCValue(false));
    // compare dates
    CHECK_EVAL("=DATE(2001;12;12)>DATE(2001;12;11)", KCValue(true));
    CHECK_EVAL("=DATE(2001;12;12)<DATE(2001;12;11)", KCValue(false));
    CHECK_EVAL("=DATE(1999;01;01)=DATE(1999;01;01)", KCValue(true));
    CHECK_EVAL("=DATE(1998;01;01)=DATE(1999;01;01)", KCValue(false));
    // errors cannot be compared
    CHECK_EVAL("=NA()=NA()", KCValue::errorNA());
    CHECK_EVAL("=NA()>NA()", KCValue::errorNA());
    CHECK_EVAL("#DIV/0!>0", KCValue::errorDIV0());
    CHECK_EVAL("5<#VALUE!", KCValue::errorVALUE());
    CHECK_EVAL("#DIV/0!=#DIV/0!", KCValue::errorDIV0());
}

void TestFormula::testString()
{
    // string expansion ...
    CHECK_EVAL("\"2\"+5", KCValue(7));
    CHECK_EVAL("2+\"5\"", KCValue(7));
    CHECK_EVAL("\"2\"+\"5\"", KCValue(7));
}

void TestFormula::testFunction()
{
    // function with no arguments
    CHECK_EVAL("TRUE()", KCValue(true));

    //the built-in sine function
    CHECK_EVAL("SIN(0)", KCValue(0));
    CHECK_EVAL("2+sin(\"2\"-\"2\")", KCValue(2));
    CHECK_EVAL("\"1\"+sin(\"0\")", KCValue(1));

    // function cascades
    CHECK_EVAL("SUM(ABS( 1);ABS( 1))", KCValue(2));
    CHECK_EVAL("SUM(ABS( 1);ABS(-1))", KCValue(2));
    CHECK_EVAL("SUM(ABS(-1);ABS( 1))", KCValue(2));
    CHECK_EVAL("SUM(ABS(-1);ABS(-1))", KCValue(2));
    CHECK_EVAL("SUM(SUM(-2;-2;-2);SUM(-2;-2;-2;-2);SUM(-2;-2;-2;-2;-2))", KCValue(-24));
}

void TestFormula::testInlineArrays()
{
#ifdef KSPREAD_INLINE_ARRAYS
    // inline arrays
    CHECK_TOKENIZE("{1;2|3;4}", "oioioioio");

    KCValue array(KCValue::Array);
    array.setElement(0, 0, KCValue((int)1));
    array.setElement(1, 0, KCValue((int)2));
    array.setElement(0, 1, KCValue((int)3));
    array.setElement(1, 1, KCValue((int)4));
    CHECK_EVAL("={1;2|3;4}", array);

    array.setElement(1, 0, KCValue(0.0));
    CHECK_EVAL("={1;SIN(0)|3;4}", array);   // "dynamic"
    CHECK_EVAL("=SUM({1;2|3;4})", KCValue(10));
#endif
}

QTEST_KDEMAIN(TestFormula, GUI)

#include "TestFormula.moc"
