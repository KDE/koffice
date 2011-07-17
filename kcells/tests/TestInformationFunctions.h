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

#ifndef KCELLS_TEST_INFORMATION_FUNCTIONS
#define KCELLS_TEST_INFORMATION_FUNCTIONS

#include <QtGui>
#include <QtTest/QtTest>

#include <KCValue.h>

class KCMap;

class TestInformationFunctions: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void testAREAS();
    // void testCELL();        // to be implemented
    void testCOLUMN();
    void testCOLUMNS();
    void testCOUNT();
    void testCOUNTA();
    void testCOUNTBLANK();
    void testCOUNTIF();
    void testERRORTYPE();
    // void testFORMULA();  // to be implemented
    void testINFO();
    void testISBLANK();
    void testISERR();
    void testISERROR();
    void testISEVEN();
    // void testISFORMULA();   // to be implemented
    void testISLOGICAL();
    void testISNONTEXT();
    void testISNA();
    void testISNUMBER();
    void testISODD();
    void testISTEXT();
    void testISREF();
    void testMATCH();
    void testN();
    void testNA();
    // void testNUMBERVALUE(); // to be implemented
    void testROW();
    void testROWS();
    // void testSHEET();       // to be implemented
    // void testSHEETS();      // to be implemented
    void testTYPE();
    void testVALUE();

    void cleanupTestCase();

private:
    KCValue evaluate(const QString&, KCValue& ex);

    KCMap* m_map;
};

#endif
