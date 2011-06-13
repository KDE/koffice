/*
 * This file is part of KOffice tests
 *
 * Copyright (C) 2005-2010 Thomas Zander <zander@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <qtest_kde.h>

#include <QObject>

#include <KXmlReader.h>
#include <KOdfSettings.h>

class TestOdfSettings : public QObject
{
    Q_OBJECT
public:
    TestOdfSettings() { }

private slots:
    void initTestCase();
    void testParseConfigItemString();
    void testSelectItemSet();
    void testIndexedMap();
    void testNamedMap();

private:
    KXmlDocument doc;
    KOdfSettings *settings;
};

void TestOdfSettings::initTestCase()
{
    const QString xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<office:document-settings xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
                " xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\">"
        "<office:settings>"
        "<config:config-item-set config:name=\"view-settings\">"
        "<config:config-item config:name=\"unit\" config:type=\"string\">mm</config:config-item>"
        "<config:config-item-map-indexed config:name=\"Views\">"
        "<config:config-item-map-entry>"
        "<config:config-item config:name=\"ZoomFactor\" config:type=\"short\">100</config:config-item>"
        "</config:config-item-map-entry>"
        "</config:config-item-map-indexed>"
        "<config:config-item-map-named config:name=\"NamedMap\">"
        "<config:config-item-map-entry config:name=\"foo\">"
        "<config:config-item config:name=\"ZoomFactor\" config:type=\"int\">100</config:config-item>"
        "</config:config-item-map-entry>"
        "</config:config-item-map-named>"
        "</config:config-item-set>"
        "</office:settings>"
        "</office:document-settings>";

    bool ok = doc.setContent( xml, true /* namespace processing */ );
    QVERIFY(ok);
    settings = new KOdfSettings(doc);
}

void TestOdfSettings::testSelectItemSet()
{
    KOdfSettings::Items items = settings->itemSet("notexist");
    QVERIFY(items.isNull());
    items = settings->itemSet("view-settings");
    QVERIFY(!items.isNull());
}

void TestOdfSettings::testParseConfigItemString()
{
    KOdfSettings::Items viewSettings = settings->itemSet("view-settings");
    const QString unit = viewSettings.parseConfigItemString("unit");
    QCOMPARE(unit, QString("mm"));
}

void TestOdfSettings::testIndexedMap()
{
    KOdfSettings::Items viewSettings = settings->itemSet("view-settings");
    QVERIFY(!viewSettings.isNull());
    KOdfSettings::IndexedMap viewMap = viewSettings.indexedMap("Views");
    QVERIFY(!viewMap.isNull());
    KOdfSettings::Items firstView = viewMap.entry(0);
    QVERIFY(!firstView.isNull());
    const short zoomFactor = firstView.parseConfigItemShort("ZoomFactor");
    QCOMPARE(zoomFactor, (short) 100);
    KOdfSettings::Items secondView = viewMap.entry(1);
    QVERIFY(secondView.isNull());
}

void TestOdfSettings::testNamedMap()
{
    KOdfSettings::Items viewSettings = settings->itemSet("view-settings");
    QVERIFY(!viewSettings.isNull());
    KOdfSettings::NamedMap viewMap = viewSettings.namedMap("NamedMap");
    QVERIFY(!viewMap.isNull());
    KOdfSettings::Items foo = viewMap.entry("foo");
    QVERIFY(!foo.isNull());
    const int zoomFactor = foo.parseConfigItemShort("ZoomFactor");
    QCOMPARE(zoomFactor, 100);
    KOdfSettings::Items secondView = viewMap.entry("foobar");
    QVERIFY(secondView.isNull());
}

QTEST_KDEMAIN(TestOdfSettings, GUI)

#include <TestOdfSettings.moc>
