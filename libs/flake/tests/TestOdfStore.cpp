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

//#include <KDebug>

#include <KXmlReader.h>
#include <KOdfStoreReader.h>

class TestOdfStore : public QObject
{
    Q_OBJECT
public:
    TestOdfStore() {}

private slots:
    void testMimeForPath();

};

void TestOdfStore::testMimeForPath()
{
    const QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n"
        "<manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"/\"/>\n"
        "<manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n"
        "<manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"Object 1\"/>\n"
        "</manifest:manifest>";

    KXmlDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent( xml, true /* namespace processing */, &errorMsg, &errorLine, &errorColumn );
    QVERIFY(ok);

    QString mime = KOdfStoreReader::mimeForPath(doc, "Object 1");
    QCOMPARE(mime, QString::fromLatin1("application/vnd.oasis.opendocument.text"));
}

QTEST_KDEMAIN(TestOdfStore, GUI)

#include <TestOdfStore.moc>
