/* This file is part of the KDE project
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

#ifndef KSPREAD_TEST_STATISTICAL_FUNCTIONS
#define KSPREAD_TEST_STATISTICAL_FUNCTIONS

#include <QtGui>
#include <QtTest/QtTest>

#include <Value.h>

namespace KSpread
{
class Doc;

class TestStatisticalFunctions : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
//     void testARRANG();
//     void testAVEDEV();
//     void testAVERAGE();
    void testAVERAGEA();
    void testBETADIST();
//     void testBINO();
//     void testCHIDIST();
//     void testCOMBIN();  // in -> TestMathFunctions
//     void testCOMBINA(); // in -> TestMathFunctions
    void testCONFIDENCE();
    void testCORREL();
    void testCOVAR();
//     void testDEVSQ();
//     void testDEVSQA();
//     void testEXPONDIST();
//     void testFDIST();
//     void testFISHER();
//     void testFISHERINV();
    void testFREQUENCY();
    void testGAMMADIST();
//     void testGAMMALN(); in -> TestMathFunctions
    void testGAUSS();
//     void testGROWTH(); // TODO check implemented?
    void testGEOMEAN();
    void testHARMEAN();
//     void testHYPGEOMDIST();
//     void testINTERCEPT();
//     void testINVBINO();
//     void testKURT();
//     void testLARGE();
//     void testLOGINV();
//     void testLOGNORMDIST();
//     void testMEDIAN();
//     void testMODE();
//     void testNEGBINOMDIST();
//     void testNORMDIST();
//     void testNORMINV();
//     void testNORMSDIST();
//     void testLEGACYNORMSDIST(); // same as NORMSDIST required for OpenFormula compliance
//     void testPEARSON();
//     void testPERMUT();
//     void testPHI();
//     void testPOISSON();
//     void testSKEW();
//     void testSLOPE();
//     void testSMALL();
//     void testSTANDARDIZE();
//     void testSTDEV();
//     void testSTDEVA();
//     void testSTDEVP();
//     void testSTDEVPA();
//     void testSTEYX();
//     void testSUMXMY2();
//     void testSUMPRODUCT();
//     void testSUMX2PY2();
//     void testSUMX2MY2();
//     void testTDIST();
//     void testTTEST();
//     void testVARIANCE();
//     void testVAR();
//     void testVARP();
//     void testVARA();
//     void testVARPA();
//     void testWEIBULL();
//     void testZTEST();

    
    void testMAXA();
    void testMINA();

    void cleanupTestCase();

private:
    Value evaluate(const QString&);
    Value TestDouble(const QString& formula, const Value& v2, int accuracy);

    Doc* m_doc;
};

} // namespace KSpread

#endif // KSPREAD_TEST_STATISTICAL_FUNCTIONS
