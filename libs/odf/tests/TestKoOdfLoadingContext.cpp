/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestKoOdfLoadingContext.h"

#include <QByteArray>
#include <QBuffer>
#include <KOdfStore.h>
#include <KOdfStorageDevice.h>
#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KOdfLoadingContext.h>
#include <KOdfStoreReader.h>
#include <KOdfWriteStore.h>
#include <KXmlWriter.h>

void TestKoOdfLoadingContext::testFillStyleStack()
{
#if 0
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
#endif
    const char * mimeType = "application/vnd.oasis.opendocument.text";
    KOdfStore * store(KOdfStore::createStore("test.odt", KOdfStore::Write, mimeType));
    KOdfWriteStore odfStore(store);
    KXmlWriter* manifestWriter = odfStore.manifestWriter(mimeType);

    KXmlWriter* contentWriter = odfStore.contentWriter();
    QVERIFY(contentWriter != 0);

    KXmlWriter * bodyWriter = odfStore.bodyWriter();

    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:text");
    bodyWriter->startElement("draw:rect");
    bodyWriter->addAttribute("draw:style-name", "gr1");
    bodyWriter->endElement();
    bodyWriter->endElement();
    bodyWriter->endElement();

    contentWriter->startElement("office:automatic-styles");
    contentWriter->startElement("style:style");
    contentWriter->addAttribute("style:name", "gr1");
    contentWriter->addAttribute("style:family", "graphic");
    contentWriter->addAttribute("style:parent-style-name", "standard");
    contentWriter->startElement("style:graphic-properties");
    contentWriter->addAttribute("draw:fill", "solid");
    contentWriter->endElement();
    contentWriter->endElement();
    contentWriter->endElement(); // office:automatic-styles

    odfStore.closeContentWriter();

    //add manifest line for content.xml
    manifestWriter->addManifestEntry("content.xml", "text/xml");

    QVERIFY(store->open("styles.xml") == true);

    KOdfStorageDevice stylesDev(store);
    KXmlWriter* stylesWriter = KOdfWriteStore::createOasisXmlWriter(&stylesDev, "office:document-styles");

    stylesWriter->startElement("office:styles");
    stylesWriter->startElement("style:style");
    stylesWriter->addAttribute("style:name", "standard");
    stylesWriter->addAttribute("style:family", "graphic");
    stylesWriter->startElement("style:graphic-properties");
    stylesWriter->addAttribute("draw:fill", "hatch");
    stylesWriter->addAttribute("draw:stroke", "solid");
    stylesWriter->endElement();
    stylesWriter->endElement();
    stylesWriter->endElement();

    stylesWriter->startElement("office:automatic-styles");
    stylesWriter->startElement("style:style");
    stylesWriter->addAttribute("style:name", "gr1");
    stylesWriter->addAttribute("style:family", "graphic");
    stylesWriter->addAttribute("style:parent-style-name", "standard");
    stylesWriter->startElement("style:graphic-properties");
    stylesWriter->addAttribute("draw:fill", "none");
    stylesWriter->addAttribute("draw:stroke", "none");
    stylesWriter->endElement();
    stylesWriter->endElement();
    stylesWriter->endElement(); // office:automatic-styles

    stylesWriter->endElement(); // root element (office:document-styles)
    stylesWriter->endDocument();
    delete stylesWriter;

    manifestWriter->addManifestEntry("styles.xml", "text/xml");

    QVERIFY(store->close() == true);   // done with styles.xml

    QVERIFY(odfStore.closeManifestWriter() == true);

    delete store;

    store = KOdfStore::createStore("test.odt", KOdfStore::Read, mimeType);
    KOdfStoreReader readStore(store);
    QString errorMessage;
    QVERIFY(readStore.loadAndParse(errorMessage) == true);
    KOdfLoadingContext context(readStore.styles(), readStore.store());

    KoXmlElement content = readStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KOdfXmlNS::office, "body"));

    QVERIFY(realBody.isNull() == false);

    KoXmlElement body = KoXml::namedItemNS(realBody, KOdfXmlNS::office, "text");
    QVERIFY(body.isNull() == false);

    KoXmlElement tag;
    forEachElement(tag, body) {
        //tz: So now that I have a test the fails I can go on implementing the solution
        QCOMPARE(tag.localName(), QString("rect"));
        KOdfStyleStack & styleStack = context.styleStack();
        styleStack.save();
        context.fillStyleStack(tag, KOdfXmlNS::draw, "style-name", "graphic");
        styleStack.setTypeProperties("graphic");
        QVERIFY(styleStack.hasProperty(KOdfXmlNS::draw, "fill"));
        QCOMPARE(styleStack.property(KOdfXmlNS::draw, "fill"), QString("solid"));
        QVERIFY(styleStack.hasProperty(KOdfXmlNS::draw, "stroke"));
        QCOMPARE(styleStack.property(KOdfXmlNS::draw, "stroke"), QString("solid"));
        styleStack.restore();
    }
}

QTEST_KDEMAIN(TestKoOdfLoadingContext, NoGUI)
#include <TestKoOdfLoadingContext.moc>
