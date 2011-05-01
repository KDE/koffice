/* This file is part of the KDE project
   Copyright 2007 Brad Hards <bradh@frogmouth.net>
   Copyright 2007 Sascha Pfau <MrPeacock@gmail.com>

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

#include <KCCellStorage.h>
#include <KCFormula.h>
#include <KCMap.h>
#include <KCSheet.h>

#include "TestKspreadCommon.h"

#include "TestInformationFunctions.h"

// because we may need to promote expected value from integer to float
#define CHECK_EVAL(x,y) { KCValue z(y); QCOMPARE(evaluate(x,z),(z)); }

KCValue TestInformationFunctions::evaluate(const QString& formula, KCValue& ex)
{
    KCSheet* sheet = m_map->sheet(0);
    KCFormula f(sheet);
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

void TestInformationFunctions::initTestCase()
{
    KCFunctionModuleRegistry::instance()->loadFunctionModules();
    m_map = new KCMap(0 /* no Doc */);
    m_map->addNewSheet();
    KCSheet* sheet = m_map->sheet(0);
    sheet->setSheetName("Sheet1");
    KCCellStorage* storage = sheet->cellStorage();

    //
    // Test case data set
    //


     // A19:A31
     storage->setValue(1,19, KCValue(    1 ) );
     storage->setValue(1,20, KCValue(    2 ) );
     storage->setValue(1,21, KCValue(    4 ) );
     storage->setValue(1,22, KCValue(    8 ) );
     storage->setValue(1,23, KCValue(   16 ) );
     storage->setValue(1,24, KCValue(   32 ) );
     storage->setValue(1,25, KCValue(   64 ) );
     storage->setValue(1,26, KCValue(  128 ) );
     storage->setValue(1,27, KCValue(  256 ) );
     storage->setValue(1,28, KCValue(  512 ) );
     storage->setValue(1,29, KCValue( 1024 ) );
     storage->setValue(1,30, KCValue( 2048 ) );
     storage->setValue(1,31, KCValue( 4096 ) );


     // B3:B17
    storage->setValue(2, 3, KCValue("7"));
    storage->setValue(2, 4, KCValue(2));
    storage->setValue(2, 5, KCValue(3));
    storage->setValue(2, 6, KCValue(true));
    storage->setValue(2, 7, KCValue("Hello"));
     // B8 leave empty
    storage->setValue(2, 9, KCValue::errorDIV0());
    storage->setValue(2, 10, KCValue(0));
//     storage->setValue(2,11, KCValue(      3    ) );
//     storage->setValue(2,12, KCValue(      4    ) );
//     storage->setValue(2,13, KCValue( "2005-0131T01:00:00" ));
//     storage->setValue(2,14, KCValue(      1    ) );
//     storage->setValue(2,15, KCValue(      2    ) );
//     storage->setValue(2,16, KCValue(      3    ) );
//     storage->setValue(2,17, KCValue(      4    ) );
//
//
//     // C4:C7
    storage->setValue(3, 4, KCValue(4));
    storage->setValue(3, 5, KCValue(5));
//     storage->setValue(3, 6, KCValue( 7 ) );
    storage->setValue(3, 7, KCValue("2005-01-31"));

     // C11:C17
     storage->setValue(3,11, KCValue( 5 ) );
     storage->setValue(3,12, KCValue( 6 ) );
     storage->setValue(3,13, KCValue( 8 ) );
     storage->setValue(3,14, KCValue( 4 ) );
     storage->setValue(3,15, KCValue( 3 ) );
     storage->setValue(3,16, KCValue( 2 ) );
     storage->setValue(3,17, KCValue( 1 ) );

//     // C19:C31
//     storage->setValue(3,19, KCValue( 0 ) );
//     storage->setValue(3,20, KCValue( 5 ) );
//     storage->setValue(3,21, KCValue( 2 ) );
//     storage->setValue(3,22, KCValue( 5 ) );
//     storage->setValue(3,23, KCValue( 3 ) );
//     storage->setValue(3,24, KCValue( 4 ) );
//     storage->setValue(3,25, KCValue( 4 ) );
//     storage->setValue(3,26, KCValue( 0 ) );
//     storage->setValue(3,27, KCValue( 8 ) );
//     storage->setValue(3,28, KCValue( 1 ) );
//     storage->setValue(3,29, KCValue( 9 ) );
//     storage->setValue(3,30, KCValue( 6 ) );
//     storage->setValue(3,31, KCValue( 2 ) );
//     // C51:C57
//     storage->setValue(3,51, KCValue(  7 ) );
//     storage->setValue(3,52, KCValue(  9 ) );
//     storage->setValue(3,53, KCValue( 11 ) );
//     storage->setValue(3,54, KCValue( 12 ) );
//     storage->setValue(3,55, KCValue( 15 ) );
//     storage->setValue(3,56, KCValue( 17 ) );
//     storage->setValue(3,57, KCValue( 19 ) );
//
//     // D51:D57
//     storage->setValue(4,51, KCValue( 100 ) );
//     storage->setValue(4,52, KCValue( 105 ) );
//     storage->setValue(4,53, KCValue( 104 ) );
//     storage->setValue(4,54, KCValue( 108 ) );
//     storage->setValue(4,55, KCValue( 111 ) );
//     storage->setValue(4,56, KCValue( 120 ) );
//     storage->setValue(4,57, KCValue( 133 ) );
//
//
//     // F51:F60
//     storage->setValue(6,51, KCValue( 3 ) );
//     storage->setValue(6,52, KCValue( 4 ) );
//     storage->setValue(6,53, KCValue( 5 ) );
//     storage->setValue(6,54, KCValue( 2 ) );
//     storage->setValue(6,55, KCValue( 3 ) );
//     storage->setValue(6,56, KCValue( 4 ) );
//     storage->setValue(6,57, KCValue( 5 ) );
//     storage->setValue(6,58, KCValue( 6 ) );
//     storage->setValue(6,59, KCValue( 4 ) );
//     storage->setValue(6,60, KCValue( 7 ) );
//
//
//     // G51:G60
//     storage->setValue(7,51, KCValue( 23 ) );
//     storage->setValue(7,52, KCValue( 24 ) );
//     storage->setValue(7,53, KCValue( 25 ) );
//     storage->setValue(7,54, KCValue( 22 ) );
//     storage->setValue(7,55, KCValue( 23 ) );
//     storage->setValue(7,56, KCValue( 24 ) );
//     storage->setValue(7,57, KCValue( 25 ) );
//     storage->setValue(7,58, KCValue( 26 ) );
//     storage->setValue(7,59, KCValue( 24 ) );
//     storage->setValue(7,60, KCValue( 27 ) );

    // A1000:G1000
    storage->setValue(1, 1000, KCValue("abc"));
    storage->setValue(2, 1000, KCValue("def"));
    storage->setValue(3, 1000, KCValue("efoob"));
    storage->setValue(4, 1000, KCValue("flka"));
    storage->setValue(5, 1000, KCValue("kde"));
    storage->setValue(6, 1000, KCValue("kde"));
    storage->setValue(7, 1000, KCValue("xxx"));

     // Z19:Z23
     storage->setValue(26,19, KCValue(   16 ) );
     storage->setValue(26,20, KCValue(    8 ) );
     storage->setValue(26,21, KCValue(    4 ) );
     storage->setValue(26,22, KCValue(    2 ) );
     storage->setValue(26,23, KCValue(    1 ) );

}

//
// unittests
//

void TestInformationFunctions::testAREAS()
{
    CHECK_EVAL("AREAS(B3)",          KCValue(1));     // A reference to a single cell has one area
    CHECK_EVAL("AREAS(B3:C4)",       KCValue(1));     // A reference to a single range has one area
// concatenation is not supported yet
//    CHECK_EVAL( "AREAS(B3:C4~D5:D6)", KCValue( 2 ) ); // KCCell concatenation creates multiple areas
//    CHECK_EVAL( "AREAS(B3:C4~B3)",    KCValue( 2 ) ); // KCCell concatenation counts, even if the cells are duplicated
}

/*
void TestInformationFunctions::testCELL()
{
    CHECK_EVAL( "CELL(\"COL\";B7)",            KCValue( 2              ) ); // Column B is column number 2.
    CHECK_EVAL( "CELL(\"ADDRESS\";B7)",        KCValue( "$B$7"         ) ); // Absolute address
    CHECK_EVAL( "CELL(\"ADDRESS\";Sheet2!B7)", KCValue( "$Sheet2.$B$7" ) ); // Absolute address including sheet name

    // Absolute address including sheet name and IRI of location of documentare duplicated
    CHECK_EVAL( "CELL(\"ADDRESS\";'x:\\sample.ods'#Sheet3!B7)", KCValue( "'file:///x:/sample.ods'#$Sheet3.$B$7" ) );

    // The current cell is saved in a file named ��sample.ods�� which is located at ��file:///x:/��
    CHECK_EVAL( "CELL(\"FILENAME\")",          KCValue( "file:///x:/sample.ods" ) );

    CHECK_EVAL( "CELL(\"FORMAT\";C7)",         KCValue( "D4" ) ); // C7's number format is like ��DD-MM-YYYY HH:MM:SS��
}
*/

void TestInformationFunctions::testCOLUMN()
{
    CHECK_EVAL("COLUMN(B7)",       KCValue(2));     // Column "B" is column number 2.
//    CHECK_EVAL( "COLUMN()",         KCValue( 5 ) ); // Column of current cell is default, here formula in column E.

    KCValue res(KCValue::Array);
    res.setElement(0, 0, KCValue(2));
    res.setElement(1, 0, KCValue(3));
    res.setElement(2, 0, KCValue(4));
    CHECK_EVAL("COLUMN(B2:D2)", res);   // Array with column numbers.
}

void TestInformationFunctions::testCOLUMNS()
{
    CHECK_EVAL("COLUMNS(C1)",      KCValue(1));     // Single cell range contains one column.
    CHECK_EVAL("COLUMNS(C1:C4)",   KCValue(1));     // Range with only one column.
    CHECK_EVAL("COLUMNS(A4:D100)", KCValue(4));     // KCNumber of columns in range.
}

void TestInformationFunctions::testCOUNT()
{
    CHECK_EVAL("COUNT(1;2;3)",       KCValue(3));     // Simple count.
    CHECK_EVAL("COUNT(B4:B5)",       KCValue(2));     // Two numbers in the range.
    CHECK_EVAL("COUNT(B4:B5;B4:B5)", KCValue(4));     // Duplicates are not removed.
    CHECK_EVAL("COUNT(B4:B9)",       KCValue(2));     // Errors in referenced cells or ranges are ignored.
    CHECK_EVAL("COUNT(B4:B8;1/0)",   KCValue(2));     // Errors in direct parameters are still ignored..
    CHECK_EVAL("COUNT(B3:B5)",       KCValue(2));     // Conversion to NumberSequence ignores strings (in B3).
}

void TestInformationFunctions::testCOUNTA()
{
    CHECK_EVAL("COUNTA(\"1\";2;TRUE())",     KCValue(3));     // Simple count of 3 constant values.
    CHECK_EVAL("COUNTA(B3:B5)",              KCValue(3));     // Three non-empty cells in the range.
    CHECK_EVAL("COUNTA(B3:B5;B3:B5)",        KCValue(6));     // Duplicates are not removed.
    CHECK_EVAL("COUNTA(B3:B9)",              KCValue(6));     // Where B9 is "=1/0", i.e. an error,
    // counts the error as non-empty; errors contained
    // in a reference do not propogate the error into the result.
    CHECK_EVAL("COUNTA(\"1\";2;SUM(B3:B9))", KCValue(3));     // Errors in an evaluated formula do not propagate; they are just counted.
    CHECK_EVAL("COUNTA(\"1\";2;B3:B9)",      KCValue(8));     // Errors in the range do not propagate either
}

void TestInformationFunctions::testCOUNTBLANK()
{
    CHECK_EVAL("COUNTBLANK(B3:B10)",    KCValue(1));     // Only B8 is blank. Zero ('0') in B10 is not considered blank.
}

void TestInformationFunctions::testCOUNTIF()
{
    CHECK_EVAL("COUNTIF(B4:B5;\">2.5\")", KCValue(1));           // B4 is 2 and B5 is 3, so there is one cell in the range with a value greater than 2.5.
    CHECK_EVAL("COUNTIF(B3:B5;B4)",       KCValue(1));           // Test if a cell equals the value in [.B4].
    CHECK_EVAL("COUNTIF(\"\";B4)",        KCValue::errorNA());   // Constant values are not allowed for the range.
    CHECK_EVAL("COUNTIF(B3:B10;\"7\")",   KCValue(1));           // [.B3] is the string "7".
    CHECK_EVAL("COUNTIF(B3:B10;1+1)",     KCValue(1));           // The criteria can be an expression.
}

void TestInformationFunctions::testERRORTYPE()
{
    CHECK_EVAL("ERRORTYPE(0)",    KCValue::errorVALUE());   // Non-errors produce an error.
    CHECK_EVAL("ERRORTYPE(NA())", KCValue(7));              // By convention, the ERROR.TYPE of NA() is 7.
    CHECK_EVAL("ERRORTYPE(1/0)",  KCValue(2));
}

/*
void TestInformationFunctions::testFORMULA()
{
    CHECK_EVAL( "LENGTH(FORMULA(B7))>0", KCValue( true ) ); // B7 is a formula, so this is fine and will produce a text value
    CHECK_EVAL( "FORMULA(B3)",           KCValue( true ) ); // Simple formulas that produce Text are still formulas
}
*/

void TestInformationFunctions::testINFO()
{
    CHECK_EVAL("INFO(\"recalc\")",             KCValue("Automatic"));     //
    CHECK_EVAL("ISTEXT(INFO(\"system\"))",     KCValue(true));            // The details of "system" vary by system, but it is always a text value
    CHECK_EVAL("ISTEXT(INFO(\"directory\"))",  KCValue(true));            // Test to see that every required category is supported
//     CHECK_EVAL( "ISNUMBER(INFO(\"memavail\"))", KCValue( true        ) ); // not implemented
//     CHECK_EVAL( "ISNUMBER(INFO(\"memused\"))",  KCValue( true        ) ); // not implemented
    CHECK_EVAL("ISNUMBER(INFO(\"numfile\"))",  KCValue(true));            //
    CHECK_EVAL("ISTEXT(INFO(\"osversion\"))",  KCValue(true));            //
//     CHECK_EVAL( "ISTEXT(INFO(\"origin\"))",     KCValue( true        ) ); // not implemented
    CHECK_EVAL("ISTEXT(INFO(\"recalc\"))",     KCValue(true));            //
    CHECK_EVAL("ISTEXT(INFO(\"release\"))",    KCValue(true));            //
//     CHECK_EVAL( "ISNUMBER(INFO(\"totmem\"))",   KCValue( true        ) ); // not implemented

    // TODO should ISTEXT return errorVALUE() if args is errorVALUE? false seems to be more logical
//     CHECK_EVAL( "ISTEXT(INFO(\"completely-unknown-category\"))", KCValue::errorVALUE()  ); // Error if the category is unknown
}

void TestInformationFunctions::testISBLANK()
{
    CHECK_EVAL("ISBLANK(1)",    KCValue(false));     // Numbers return false.
    CHECK_EVAL("ISBLANK(\"\")", KCValue(false));     // Text, even empty string, returns false.
    CHECK_EVAL("ISBLANK(B8)",   KCValue(true));      // Blank cell is true.
    CHECK_EVAL("ISBLANK(B7)",   KCValue(false));     // Non-blank cell is false.
}

void TestInformationFunctions::testISERR()
{
    CHECK_EVAL("ISERR(1/0)",      KCValue(true));      // Error values other than NA() return true.
    CHECK_EVAL("ISERR(NA())",     KCValue(false));     // NA() does NOT return True.
    CHECK_EVAL("ISERR(\"#N/A\")", KCValue(false));     // Text is not an error.
    CHECK_EVAL("ISERR(1)",        KCValue(false));     // Numbers are not an error.
}

void TestInformationFunctions::testISERROR()
{
    CHECK_EVAL("ISERROR(1/0)",      KCValue(true));      // Error values return true.
    CHECK_EVAL("ISERROR(NA())",     KCValue(true));      // Even NA().
    CHECK_EVAL("ISERROR(\"#N/A\")", KCValue(false));     // Text is not an error.
    CHECK_EVAL("ISERROR(1)",        KCValue(false));     // Numbers are not an error.
    CHECK_EVAL("ISERROR(CHOOSE(0; \"Apple\"; \"Orange\";"
               " \"Grape\"; \"Perry\"))", KCValue(true));    // If CHOOSE given
    // out-of-range value, ISERROR needs to capture it.
}

void TestInformationFunctions::testISEVEN()
{
    CHECK_EVAL("ISEVEN( 2)",   KCValue(true));        // 2 is even, because (2 modulo 2) = 0
    CHECK_EVAL("ISEVEN( 6)",   KCValue(true));        // 6 is even, because (6 modulo 2) = 0
    CHECK_EVAL("ISEVEN( 2.1)", KCValue(true));        //
    CHECK_EVAL("ISEVEN( 2.5)", KCValue(true));        //
    CHECK_EVAL("ISEVEN( 2.9)", KCValue(true));        // TRUNC(2.9)=2, and 2 is even.
    CHECK_EVAL("ISEVEN( 3)",   KCValue(false));       // 3 is not even.
    CHECK_EVAL("ISEVEN( 3.9)", KCValue(false));       // TRUNC(3.9)=3, and 3 is not even.
    CHECK_EVAL("ISEVEN(-2)",   KCValue(true));        //
    CHECK_EVAL("ISEVEN(-2.1)", KCValue(true));        //
    CHECK_EVAL("ISEVEN(-2.5)", KCValue(true));        //
    CHECK_EVAL("ISEVEN(-2.9)", KCValue(true));        // TRUNC(-2.9)=-2, and -2 is even.
    CHECK_EVAL("ISEVEN(-3)",   KCValue(false));       //
    CHECK_EVAL("ISEVEN(NA())", KCValue::errorNA());   //
    CHECK_EVAL("ISEVEN( 0)",   KCValue(true));        //
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISEVEN(2.5)", KCValue(true)); // alternate function name
}

/*
void TestInformationFunctions::testISFORMULA()
{
    CHECK_EVAL( "ISFORMULA(B5)", KCValue( true  ) ); // Simple formulas that produce KCNumber are still formulas
    CHECK_EVAL( "ISFORMULA(B3)", KCValue( true  ) ); // Simple formulas that produce Text are still formulas
    CHECK_EVAL( "ISFORMULA(C5)", KCValue( false ) ); // KCCell constants are not formulas
    CHECK_EVAL( "ISFORMULA(C7)", KCValue( false ) ); // KCCell constants are not formulas, even if they are dates
    CHECK_EVAL( "ISFORMULA(B9)", KCValue( true  ) ); // Formulas that return an error are still formulas
}
*/

void TestInformationFunctions::testISLOGICAL()
{
    CHECK_EVAL("ISLOGICAL(TRUE())",   KCValue(true));      // Logical values return true.
    CHECK_EVAL("ISLOGICAL(FALSE())",  KCValue(true));      // Logical values return true.
    CHECK_EVAL("ISLOGICAL(\"TRUE\")", KCValue(false));     // Text values are not logicals,
    // even if they can be converted.
}

void TestInformationFunctions::testISNONTEXT()
{
    CHECK_EVAL("ISNONTEXT(1)",      KCValue(true));      // Numbers are not text
    CHECK_EVAL("ISNONTEXT(TRUE())", KCValue(true));      // Logical values are not text.
    CHECK_EVAL("ISNONTEXT(\"1\")",  KCValue(false));     // TexText values are text, even
    // if they can be converted into a number.
    CHECK_EVAL("ISNONTEXT(B7)",     KCValue(false));     // B7 is a cell with text
    CHECK_EVAL("ISNONTEXT(B9)",     KCValue(true));      // B9 is an error, thus not text
    CHECK_EVAL("ISNONTEXT(B8)",     KCValue(true));      // B8 is a blank cell, so this will return TRUE
}

void TestInformationFunctions::testISNA()
{
    CHECK_EVAL("ISNA(1/0)",      KCValue(false));     // Error values other than NA() return False - the error does not propagate.
    CHECK_EVAL("ISNA(NA())",     KCValue(true));      // By definition
    // CHECK_EVAL( "ISNA(#N/A)",     KCValue( true  ) ); // By definition
    CHECK_EVAL("ISNA(\"#N/A\")", KCValue(false));     // Text is not NA
    CHECK_EVAL("ISNA(1)",        KCValue(false));     // Numbers are not NA
}

void TestInformationFunctions::testISNUMBER()
{
    CHECK_EVAL("ISNUMBER(1)",     KCValue(true));      // Numbers are numbers
    CHECK_EVAL("ISNUMBER(\"1\")", KCValue(false));     // Text values are not numbers,
    // even if they can be converted into a number.
}

void TestInformationFunctions::testISODD()
{
    CHECK_EVAL("ISODD(3)",    KCValue(true));      // 3 is odd, because (3 modulo 2) = 0
    CHECK_EVAL("ISODD(5)",    KCValue(true));      // 5 is odd, because (5 modulo 2) = 0
    CHECK_EVAL("ISODD(3.1)",  KCValue(true));      // TRUNC(3.1)=3, and 3 is odd
    CHECK_EVAL("ISODD(3.5)",  KCValue(true));      //
    CHECK_EVAL("ISODD(3.9)",  KCValue(true));      // TRUNC(3.9)=3, and 3 is odd.
    CHECK_EVAL("ISODD(4)",    KCValue(false));     // 4 is not even.
    CHECK_EVAL("ISODD(4.9)",  KCValue(false));     // TRUNC(4.9)=4, and 3 is not even.
    CHECK_EVAL("ISODD(-3)",   KCValue(true));      //
    CHECK_EVAL("ISODD(-3.1)", KCValue(true));      //
    CHECK_EVAL("ISODD(-3.5)", KCValue(true));      //
    CHECK_EVAL("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISODD(3.1)", KCValue(true)); // alternate function name
}

void TestInformationFunctions::testISTEXT()
{
    CHECK_EVAL("ISTEXT(1)",     KCValue(false));     // Numbers are not text
    CHECK_EVAL("ISTEXT(\"1\")", KCValue(true));      // Text values are text,
    // even if they can be converted into a number.
}

void TestInformationFunctions::testISREF()
{
    CHECK_EVAL("ISREF(B3)",     KCValue(true));        //
    CHECK_EVAL("ISREF(B3:C4)",  KCValue(true));        // The range operator produces references
    CHECK_EVAL("ISREF(1)",      KCValue(false));       // Numbers are not references
    CHECK_EVAL("ISREF(\"A1\")", KCValue(false));       // Text is not a reference, even if it looks a little like one
    CHECK_EVAL("ISREF(NA())",   KCValue::errorNA());   // Errors propagate through this function
}

void TestInformationFunctions::testN()
{
    CHECK_EVAL("N(6)",       KCValue(6));     // N does not change numbers.
    CHECK_EVAL("N(TRUE())",  KCValue(1));     // Does convert logicals.
    CHECK_EVAL("N(FALSE())", KCValue(0));     // Does convert logicals.
}

void TestInformationFunctions::testNA()
{
    CHECK_EVAL("ISERROR(NA())", KCValue(true));     // NA is an error.
    CHECK_EVAL("ISNA(NA())",    KCValue(true));     // Obviously, if this doesn't work, NA() or ISNA() is broken.
    CHECK_EVAL("ISNA(5+NA())",  KCValue(true));     // NA propagates through various functions
    // and operators, just like any other error type.
}

/*
void TestInformationFunctions::testNUMBERVALUE()
{
    CHECK_EVAL( "NUMBERVALUE(\"6\"      ; \".\")", KCValue(    6   ) ); // VALUE converts text to numbers (unlike N).
    CHECK_EVAL( "NUMBERVALUE(\"6,000.5\"; \".\")", KCValue( 6000.5 ) ); // Period works.
    CHECK_EVAL( "NUMBERVALUE(\"6.000,5\"; \",\")", KCValue( 6000.5 ) ); // Comma works
}
*/

// TODO row not working here
void TestInformationFunctions::testROW()
{
    CHECK_EVAL("ROW(B7)", KCValue(7));     // The second value of a cell reference is the row number.
//     CHECK_EVAL( "ROW()",   KCValue( 5 ) ); // Row of current cell is default, here formula in row 5.

    KCValue res(KCValue::Array);
    res.setElement(0, 0, KCValue(2));
    res.setElement(0, 1, KCValue(3));
    res.setElement(0, 2, KCValue(4));

    CHECK_EVAL("ROW(B2:B4)", res);      // Array with row numbers.
}

void TestInformationFunctions::testROWS()
{
    CHECK_EVAL("ROWS(C1)",      KCValue(1));      // Single cell range contains one row.
    CHECK_EVAL("ROWS(C1:C4)",   KCValue(4));      // Range with four rows.
    CHECK_EVAL("ROWS(A4:D100)", KCValue(97));     // KCNumber of rows in range.
}

/*
void TestInformationFunctions::testSHEET()
{
    CHECK_EVAL( "SHEET(B7)>=1",         KCValue( true ) ); // If given, the sheet number of the reference is used.
    CHECK_EVAL( "SHEET(\"Sheet1\")>=1", KCValue( true ) ); // Given a sheet name, the sheet number is returned.
}

void TestInformationFunctions::testSHEETS()
{
    CHECK_EVAL( "SHEETS(B7)",  KCValue(    1 ) ); // If given, the sheet number of the reference is used.
    CHECK_EVAL( "SHEETS()>=1", KCValue( true ) ); // Range with four rows.
}
*/

void TestInformationFunctions::testTYPE()
{
    //  KCValue's Type | Type return
    // --------------+-------------
    //     KCNumber    |     1
    //     Text      |     2
    //     Logical   |     4
    //     Error     |    16
    //     Array     |    64

    CHECK_EVAL("TYPE(1+2)",              KCValue(1));      // KCNumber has TYPE code of 1
    CHECK_EVAL("TYPE(\"Hi\"&\"there\")", KCValue(2));      // Text has TYPE 2
    CHECK_EVAL("TYPE(NA())",             KCValue(16));     // Errors have TYPE 16.
}

void TestInformationFunctions::testVALUE()
{
    CHECK_EVAL("VALUE(\"6\")", KCValue(6));
    CHECK_EVAL("VALUE(\"1E5\")", KCValue(100000));
    CHECK_EVAL("VALUE(\"200%\")",  KCValue(2));
    CHECK_EVAL("VALUE(\"1.5\")", KCValue(1.5));
    // Check fractions
    CHECK_EVAL("VALUE(\"7 1/4\")", KCValue(7.25));
    CHECK_EVAL("VALUE(\"0 1/2\")", KCValue(0.5));
    CHECK_EVAL("VALUE(\"0 7/2\")", KCValue(3.5));
    CHECK_EVAL("VALUE(\"-7 1/5\")", KCValue(-7.2));
    CHECK_EVAL("VALUE(\"-7 10/50\")", KCValue(-7.2));
    CHECK_EVAL("VALUE(\"-7 10/500\")", KCValue(-7.02));
    CHECK_EVAL("VALUE(\"-7 4/2\")", KCValue(-9));
    CHECK_EVAL("VALUE(\"-7 40/20\")", KCValue(-9));
    // Check times
    CHECK_EVAL("VALUE(\"00:00\")", KCValue(0));
    CHECK_EVAL("VALUE(\"00:00:00\")", KCValue(0));
    CHECK_EVAL("VALUE(\"02:00\")-2/24", KCValue(0));
    CHECK_EVAL("VALUE(\"02:00:00\")-2/24", KCValue(0));
    CHECK_EVAL("VALUE(\"02:00:00.0\")-2/24", KCValue(0));
    CHECK_EVAL("VALUE(\"02:00:00.00\")-2/24", KCValue(0));
    CHECK_EVAL("VALUE(\"02:00:00.000\")-2/24", KCValue(0));
    CHECK_EVAL("VALUE(\"2:03:05\") -2/24-3/(24*60) -5/(24*60*60)", KCValue(0));
    CHECK_EVAL("VALUE(\"2:03\")-(2/24)-(3/(24*60))", KCValue(0));
    // check dates - local dependent
    // CHECK_EVAL( "VALUE(\"5/21/06\")=DATE(2006;5;21)", KCValue( true ) );
    // CHECK_EVAL( "VALUE(\"1/2/2005\")=DATE(2005;1;2)", KCValue( true ) );
}

void TestInformationFunctions::testMATCH()
{
    // invalid matchType
    CHECK_EVAL("MATCH(1;A19:A31;\"foo\")", KCValue::errorVALUE());

    // matchType == 0, exact match
    CHECK_EVAL("MATCH(5;C11:C17;0)", KCValue(1));
    CHECK_EVAL("MATCH(8;C11:C17;0)", KCValue(3));
    CHECK_EVAL("MATCH(1;C11:C17;0)", KCValue(7));
    CHECK_EVAL("MATCH(13;C11:C17;0)", KCValue::errorNA());
    CHECK_EVAL("MATCH(5;C11:C11;0)", KCValue(1));
    CHECK_EVAL("MATCH(5;C11;0)", KCValue(1));
    CHECK_EVAL("MATCH(5;C11:D13;0)", KCValue::errorNA()); // not sure if this is the best error
    CHECK_EVAL("MATCH(\"Hello\";B3:B10;0)", KCValue(5));
    CHECK_EVAL("MATCH(\"hello\";B3:B10;0)", KCValue(5)); // match is always case insensitive
    CHECK_EVAL("MATCH(\"kde\";A1000:G1000;0)", KCValue(5));

    // matchType == 1 or omitted, largest value less than or equal to search value in sorted range
    CHECK_EVAL("MATCH(0;A19:A31;1)", KCValue::errorNA());
    CHECK_EVAL("MATCH(1;A19:A31;1)", KCValue(1));
    CHECK_EVAL("MATCH(16;A19:A31;1)", KCValue(5));
    CHECK_EVAL("MATCH(40;A19:A31;1)", KCValue(6));
    CHECK_EVAL("MATCH(4096;A19:A31;1)", KCValue(13));
    CHECK_EVAL("MATCH(5000;A19:A31;1)", KCValue(13));
    CHECK_EVAL("MATCH(\"aaa\";A1000:G1000)", KCValue::errorNA());
    CHECK_EVAL("MATCH(\"abc\";A1000:G1000)", KCValue(1));
    CHECK_EVAL("MATCH(\"efoob\";A1000:G1000)", KCValue(3));
    CHECK_EVAL("MATCH(\"epub\";A1000:G1000)", KCValue(3));
    CHECK_EVAL("MATCH(\"kde\";A1000:G1000)", KCValue(6));
    CHECK_EVAL("MATCH(\"xxx\";A1000:G1000)", KCValue(7));
    CHECK_EVAL("MATCH(\"zzz\";A1000:G1000)", KCValue(7));
    CHECK_EVAL("MATCH(13;C11:D13;1)", KCValue::errorNA()); // not sure if this is the best error

    // matchType == -1, smallest value greater than or equal to search value, in descending range
    CHECK_EVAL("MATCH(0;Z19:Z23;-1)", KCValue(5));
    CHECK_EVAL("MATCH(1;Z19:Z23;-1)", KCValue(5));
    CHECK_EVAL("MATCH(4;Z19:Z23;-1)", KCValue(3));
    CHECK_EVAL("MATCH(5;Z19:Z23;-1)", KCValue(2));
    CHECK_EVAL("MATCH(16;Z19:Z23;-1)", KCValue(1));
    CHECK_EVAL("MATCH(33;Z19:Z23;-1)", KCValue::errorNA());
    CHECK_EVAL("MATCH(13;C11:D13;-1)", KCValue::errorNA()); // not sure if this is the best error
}

//
// cleanup test
//

void TestInformationFunctions::cleanupTestCase()
{
    delete m_map;
}

QTEST_KDEMAIN(TestInformationFunctions, GUI)

#include "TestInformationFunctions.moc"
