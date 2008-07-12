/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "TestRegion.h"

#include "qtest_kde.h"

#include "part/Doc.h" // FIXME detach from part
#include "Map.h"
#include "Region.h"
#include "Sheet.h"

using namespace KSpread;

void TestRegion::initTestCase()
{
    m_doc = new Doc();
    Sheet* sheet = m_doc->map()->addNewSheet();
    sheet->setSheetName("Sheet1");
    sheet = m_doc->map()->addNewSheet();
    sheet->setSheetName("Sheet2");
    sheet = m_doc->map()->addNewSheet();
    sheet->setSheetName("Sheet3");
    sheet = m_doc->map()->addNewSheet();
    sheet->setSheetName("Sheet 4");
}

void TestRegion::testComparison()
{
    Region region1;
    Region region2;
    region1 = Region("A1");
    region2 = Region("A1");
    QVERIFY(region1 == region2);
    region1 = Region("A1:A5");
    region2 = Region("A1:A5");
    QVERIFY(region1 == region2);
    region1 = Region("A1:A5;B4");
    region2 = Region("A1:A5;B4");
    QVERIFY(region1 == region2);
    region2 = Region("A1");
    QVERIFY(region1 != region2);
    region2 = Region("A1:A5");
    QVERIFY(region1 != region2);
}

void TestRegion::testFixation()
{
    Region region;
    region = Region("$A1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A1"));
    region = Region("A$1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A$1"));
    region = Region("$A$1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A$1"));
    region = Region("$A1:B4", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A1:B4"));
    region = Region("A$1:B4", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A$1:B4"));
    region = Region("$A$1:B4", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A$1:B4"));
    region = Region("A1:$B4", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1:$B4"));
    region = Region("A1:B$4", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1:B$4"));
    region = Region("A1:$B$4", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1:$B$4"));
}

void TestRegion::testSheet()
{
    Region region;
    region = Region(QPoint(1, 1), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(0));
    region = Region("A1");
    QCOMPARE(region.name(), QString("A1"));
    QCOMPARE(region.firstSheet(), (Sheet*)0);
    region = Region("A1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(0));
    region = Region("Sheet1!A1", m_doc->map(), m_doc->map()->sheet(1));
    QCOMPARE(region.name(), QString("Sheet1!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(0));
    region = Region("Sheet2!A1", m_doc->map());
    QCOMPARE(region.name(), QString("Sheet2!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(1));
    region = Region("Sheet2!A1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("Sheet2!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(1));
    region = Region("Sheet 4!A1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("'Sheet 4'!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(3));
    region = Region("'Sheet 4'!A1", m_doc->map(), m_doc->map()->sheet(0));
    QCOMPARE(region.name(), QString("'Sheet 4'!A1"));
    QCOMPARE(region.firstSheet(), m_doc->map()->sheet(3));
    // invalid calls:
    region = Region("!A1", m_doc->map(), m_doc->map()->sheet(0));
    QVERIFY(region.isEmpty());
    region = Region("Sheet99!A1", m_doc->map(), m_doc->map()->sheet(0));
    QVERIFY(region.isEmpty());
}

void TestRegion::cleanupTestCase()
{
    delete m_doc;
}

QTEST_KDEMAIN(TestRegion, GUI)

#include "TestRegion.moc"
