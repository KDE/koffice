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

#include "kspread_limits.h"
#include "KCMap.h"
#include "KCRegion.h"
#include "KCSheet.h"

void TestRegion::initTestCase()
{
    m_map = new KCMap(0 /* no KCDoc*/);
    KCSheet* sheet = m_map->addNewSheet();
    sheet->setSheetName("Sheet1");
    sheet = m_map->addNewSheet();
    sheet->setSheetName("Sheet2");
    sheet = m_map->addNewSheet();
    sheet->setSheetName("Sheet3");
    sheet = m_map->addNewSheet();
    sheet->setSheetName("KCSheet 4");
}

void TestRegion::testComparison()
{
    KCRegion region1;
    KCRegion region2;
    region1 = KCRegion("A1");
    region2 = KCRegion("A1");
    QVERIFY(region1 == region2);
    region1 = KCRegion("A1:A5");
    region2 = KCRegion("A1:A5");
    QVERIFY(region1 == region2);
    region1 = KCRegion("A1:A5;B4");
    region2 = KCRegion("A1:A5;B4");
    QVERIFY(region1 == region2);
    region2 = KCRegion("A1");
    QVERIFY(region1 != region2);
    region2 = KCRegion("A1:A5");
    QVERIFY(region1 != region2);
}

void TestRegion::testFixation()
{
    KCRegion region;
    region = KCRegion("$A1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A1"));
    region = KCRegion("A$1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A$1"));
    region = KCRegion("$A$1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A$1"));
    region = KCRegion("$A1:B4", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A1:B4"));
    region = KCRegion("A$1:B4", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A$1:B4"));
    region = KCRegion("$A$1:B4", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!$A$1:B4"));
    region = KCRegion("A1:$B4", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1:$B4"));
    region = KCRegion("A1:B$4", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1:B$4"));
    region = KCRegion("A1:$B$4", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1:$B$4"));
}

void TestRegion::testSheet()
{
    KCRegion region;
    region = KCRegion(QPoint(1, 1), m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(0));
    region = KCRegion("A1");
    QCOMPARE(region.name(), QString("A1"));
    QCOMPARE(region.firstSheet(), (KCSheet*)0);
    region = KCRegion("A1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet1!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(0));
    region = KCRegion("Sheet1!A1", m_map, m_map->sheet(1));
    QCOMPARE(region.name(), QString("Sheet1!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(0));
    region = KCRegion("Sheet2!A1", m_map);
    QCOMPARE(region.name(), QString("Sheet2!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(1));
    region = KCRegion("Sheet2!A1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("Sheet2!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(1));
    region = KCRegion("KCSheet 4!A1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("'KCSheet 4'!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(3));
    region = KCRegion("'KCSheet 4'!A1", m_map, m_map->sheet(0));
    QCOMPARE(region.name(), QString("'KCSheet 4'!A1"));
    QCOMPARE(region.firstSheet(), m_map->sheet(3));
    // invalid calls:
    region = KCRegion("!A1", m_map, m_map->sheet(0));
    QVERIFY(region.isEmpty());
    region = KCRegion("Sheet99!A1", m_map, m_map->sheet(0));
    QVERIFY(region.isEmpty());
}

void TestRegion::testExtrem()
{
    KCRegion region1 = KCRegion(QPoint(-1, -1), m_map->sheet(0));
    QVERIFY(region1.isEmpty());
    QVERIFY(!region1.isValid());

    KCRegion region2 = KCRegion("A1:A6553634523563453456356");
    QVERIFY(region2.isValid());

    KCRegion region3 = KCRegion(QRect(1,1,KS_colMax,KS_rowMax), m_map->sheet(0));
    QVERIFY(region3.isValid());

    KCRegion region4 = KCRegion(QRect(1,1,KS_colMax,KS_rowMax), m_map->sheet(0));
    QVERIFY(region4.isValid());
    KCRegion region5 = KCRegion(QRect(1,1,KS_colMax+12345,KS_rowMax+12345), m_map->sheet(0));
    QVERIFY(region5.isValid());
    QCOMPARE(region4.name(), region5.name());
    
    KCRegion region6 = KCRegion(QPoint(KS_colMax, KS_rowMax), m_map->sheet(0));
    QVERIFY(region6.isValid());
    KCRegion region7 = KCRegion(QPoint(KS_colMax+22, KS_rowMax+22), m_map->sheet(0));
    QVERIFY(region7.isValid());
    QCOMPARE(region6.name(), region7.name());
}

void TestRegion::cleanupTestCase()
{
    delete m_map;
}

QTEST_KDEMAIN(TestRegion, GUI)

#include "TestRegion.moc"
