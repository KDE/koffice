/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2010 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestLoadStyle.h"

#include <KOdfStore.h>
#include <KoTextLoader.h>
#include <KXmlReader.h>
#include <KOdfStoreReader.h>
#include <KTemporaryFile>
#include <KoTextShapeData.h>
#include <KShapeLoadingContext.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>
#include <KTextDocumentLayout.h>
#include <KStyleManager.h>
#include <KCharacterStyle.h>
#include <KParagraphStyle.h>
#include <KoText.h>
#include <KInlineTextObjectManager.h>
#include <KoTextSharedLoadingData.h>
#include <KTextDocument.h>
#include <KChangeTracker.h>

#include <kstandarddirs.h>
#include <KDebug>
#include <kcomponentdata.h>

TestLoadStyle::TestLoadStyle()
{
    componentData = new KComponentData("TestLoadStyle");
    componentData->dirs()->addResourceType("styles", "data", "kword/styles/");
}

TestLoadStyle::~TestLoadStyle()
{
    delete componentData;
}

// initTestCase/cleanupTestCase are called beginning and end of test suite
void TestLoadStyle::initTestCase()
{
}

void TestLoadStyle::cleanupTestCase()
{
}

// init/cleanup are called beginning and end of every test case
void TestLoadStyle::init()
{
}

void TestLoadStyle::cleanup()
{
}

QTextDocument *TestLoadStyle::documentFromOdt(const QString &odt)
{
    if (!QFile(odt).exists()) {
        qFatal("%s does not exist", qPrintable(odt));
        return 0;
    }

    KOdfStore *readStore = KOdfStore::createStore(odt, KOdfStore::Read, "", KOdfStore::Zip);
    KOdfStoreReader odfReadStore(readStore);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        qDebug() << "Parsing error : " << error;
    }

    KXmlElement content = odfReadStore.contentDoc().documentElement();
    KXmlElement realBody(KoXml::namedItemNS(content, KOdfXmlNS::office, "body"));
    KXmlElement body = KoXml::namedItemNS(realBody, KOdfXmlNS::office, "text");

    KStyleManager *styleManager = new KStyleManager;
    KChangeTracker *changeTracker = new KChangeTracker;

    KOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store(), *componentData);
    KShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0);
    KoTextSharedLoadingData *textSharedLoadingData = new KoTextSharedLoadingData;
    textSharedLoadingData->loadOdfStyles(shapeLoadingContext, styleManager);
    shapeLoadingContext.addSharedData(KOTEXT_SHARED_LOADING_ID, textSharedLoadingData);

    KoTextShapeData *textShapeData = new KoTextShapeData;
    QTextDocument *document = new QTextDocument;
    textShapeData->setDocument(document, false /* ownership */);
    KTextDocumentLayout *layout = new KTextDocumentLayout(textShapeData->document());
    layout->setInlineTextObjectManager(new KInlineTextObjectManager(layout)); // required while saving
    KTextDocument(document).setStyleManager(styleManager);
    textShapeData->document()->setDocumentLayout(layout);
    KTextDocument(document).setChangeTracker(changeTracker);

    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KoTextShapeData failed to load ODT";
    }

    delete readStore;
    delete textShapeData;
    return document;
}

void TestLoadStyle::testLoadStyle()
{
    QTextDocument *document = documentFromOdt(QString(FILES_DATA_DIR)
            + "/TextContents/TextFormatting/charStyle.odt");
    QVERIFY(document != 0);

    QTextBlock block = document->begin();
    QVERIFY(block.isValid());
    QCOMPARE(block.text(), QString("The following is a word which uses the named character style MyStyle."));

    QTextCursor cursor(block);
    QCOMPARE(cursor.blockFormat().property(KParagraphStyle::StyleId).toInt(), 100);
    QCOMPARE(cursor.blockCharFormat().property(KCharacterStyle::StyleId).toInt(), 101);
    QCOMPARE(cursor.charFormat().property(KCharacterStyle::StyleId).toInt(), 101);

    cursor.setPosition(62);
    //qDebug() << cursor.charFormat().property(KCharacterStyle::StyleId).toInt();

    KTextDocument textDoc(document);
    KStyleManager *sm = textDoc.styleManager();
    KCharacterStyle *myStyle = sm->characterStyle("MyStyle");
    QVERIFY(myStyle);
    //qDebug() << myStyle->styleId();
    QCOMPARE(cursor.charFormat().property(KCharacterStyle::StyleId).toInt(), myStyle->styleId());
}

QTEST_KDEMAIN(TestLoadStyle, GUI)
#include <TestLoadStyle.moc>
