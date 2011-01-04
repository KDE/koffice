/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestChangeTracking.h"

#include <QtGui>
#include <KDebug>
#include <QtScript>
#include <QtTest>

#include <KoStyleManager.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoOdfStylesReader.h>
#include <KoTextLoader.h>
#include <KoXmlReader.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KTemporaryFile>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoTextShapeData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <kcomponentdata.h>
#include <KoTextDebug.h>
#include <KoListStyle.h>
#include <KoTableStyle.h>
#include <KoTableCellStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoText.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextSharedLoadingData.h>
#include <KoTextSharedSavingData.h>
#include <KoTextDocument.h>
#include <kstandarddirs.h>

#include <KoGenChanges.h>
#include <KoChangeTracker.h>
#include <KoDeleteChangeMarker.h>
#include <KoChangeTrackerElement.h>

TestChangeTracking::TestChangeTracking()
{
    componentData = new KComponentData("TestLoading");
    componentData->dirs()->addResourceType("styles", "data", "kword/styles/");
}

TestChangeTracking::~TestChangeTracking()
{
    delete componentData;
}

void TestChangeTracking::init()
{
}

void TestChangeTracking::cleanup()
{
}

QTextDocument *TestChangeTracking::documentFromOdt(const QString &odt)
{
    if (!QFile(odt).exists()) {
        qFatal("%s does not exist", qPrintable(odt));
        return 0;
    }

    KoStore *readStore = KoStore::createStore(odt, KoStore::Read, "", KoStore::Zip);
    KoOdfReadStore odfReadStore(readStore);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        qDebug() << "Parsing error : " << error;
    }

    KoXmlElement content = odfReadStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));
    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, "text");

    KoStyleManager *styleManager = new KoStyleManager;
    KoChangeTracker *changeTracker = new KoChangeTracker;

    KoOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store(), *componentData);
    KoShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0);
    KoTextSharedLoadingData *textSharedLoadingData = new KoTextSharedLoadingData;
    textSharedLoadingData->loadOdfStyles(shapeLoadingContext, styleManager);
    shapeLoadingContext.addSharedData(KOTEXT_SHARED_LOADING_ID, textSharedLoadingData);

    KoTextShapeData *textShapeData = new KoTextShapeData;
    QTextDocument *document = new QTextDocument;
    textShapeData->setDocument(document, false /* ownership */);
    KoTextDocumentLayout *layout = new KoTextDocumentLayout(textShapeData->document());
    layout->setInlineTextObjectManager(new KoInlineTextObjectManager(layout)); // required while saving
    KoTextDocument(document).setStyleManager(styleManager);
    textShapeData->document()->setDocumentLayout(layout);
    KoTextDocument(document).setChangeTracker(changeTracker);

    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KoTextShapeData failed to load ODT";
    }

    delete readStore;
    delete textShapeData;
    return document;
}

QString TestChangeTracking::documentToOdt(QTextDocument *document)
{
    QString odt("test.odt");
    if (QFile::exists(odt))
        QFile::remove(odt);
    QFile f(odt);
    f.open(QFile::WriteOnly);
    f.close();

    KoStore *store = KoStore::createStore(odt, KoStore::Write, "application/vnd.oasis.opendocument.text", KoStore::Zip);
    KoOdfWriteStore odfWriteStore(store);
    KoXmlWriter *manifestWriter = odfWriteStore.manifestWriter("application/vnd.oasis.opendocument.text");
    manifestWriter->addManifestEntry("content.xml", "text/xml");
    if (!store->open("content.xml"))
        return QString();

    KoStoreDevice contentDev(store);
    KoXmlWriter* contentWriter = KoOdfWriteStore::createOasisXmlWriter(&contentDev, "office:document-content");

    // for office:body
    KTemporaryFile contentTmpFile;
    if (!contentTmpFile.open())
        qFatal("Error opening temporary file!");
    KoXmlWriter xmlWriter(&contentTmpFile, 1);

    KoGenStyles mainStyles;
    KoStyleManager *styleMan = KoTextDocument(document).styleManager();
    Q_UNUSED(styleMan);
    KoEmbeddedDocumentSaver embeddedSaver;

    KoGenChanges changes;
    KoShapeSavingContext context(xmlWriter, mainStyles, embeddedSaver);

    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    KoTextSharedSavingData *textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << textSharedData;

    if (!textSharedData) {
        textSharedData = new KoTextSharedSavingData();
        textSharedData->setGenChanges(changes);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_SAVING_ID, textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }

    KoTextShapeData *textShapeData = new KoTextShapeData;
    textShapeData->setDocument(document, false /* ownership */);
    if (qobject_cast<KoTextDocumentLayout *>(document->documentLayout()) == 0) {
        // Setup layout and managers just like kotext
        KoTextDocumentLayout *layout = new KoTextDocumentLayout(textShapeData->document());
        textShapeData->document()->setDocumentLayout(layout);
        layout->setInlineTextObjectManager(new KoInlineTextObjectManager(layout)); // required while saving
        KoStyleManager *styleManager = new KoStyleManager;
        KoTextDocument(textShapeData->document()).setStyleManager(styleManager);
    }

    textShapeData->saveOdf(context);

    contentTmpFile.close();

    mainStyles.saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, contentWriter);

    contentWriter->startElement("office:body");
    contentWriter->startElement("office:text");

    changes.saveOdfChanges(contentWriter);

    contentWriter->addCompleteElement(&contentTmpFile);

    contentWriter->endElement(); //office text
    contentWriter->endElement(); //office body

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;


    if (!store->close())
        qWarning() << "Failed to close the store";

    mainStyles.saveOdfStylesDotXml(store, manifestWriter);

    odfWriteStore.closeManifestWriter();


    delete store;
    delete textShapeData;

    return odt;
}

void TestChangeTracking::testChangeTracking()
{
    
    QFETCH(QString, testcase);
    QString testFileName = testcase;
    testFileName.prepend(QString(FILES_DATA_DIR));

    QTextDocument *originalDocument = documentFromOdt(testFileName);
    QString roundTripFileName = documentToOdt(originalDocument);

    QVERIFY(verifyContentXml(testFileName, roundTripFileName));
}

void TestChangeTracking::testChangeTracking_data()
{
    QTest::addColumn<QString>("testcase");

    QTest::newRow("SimpleTextInsertion") << "ChangeTracking/text/simple-addition/simple-addition-tracked.odt";
    QTest::newRow("SimpleTextDeletion")  << "ChangeTracking/text/simple-deletion/simple-deletion-tracked.odt";
    QTest::newRow("NumberedParagraphAddition")  << "ChangeTracking/text/simple-numbered-paragraph-addition/simple-numbered-paragraph-addition-tracked.odt";
    QTest::newRow("ParagraphAddition")  << "ChangeTracking/text/simple-paragraph-addition/simple-paragraph-addition-tracked.odt";
    QTest::newRow("ParagraphDeletion")  << "ChangeTracking/text/simple-paragraph-deletion/simple-paragraph-deletion-tracked.odt";
    QTest::newRow("ParagraphSplit")  << "ChangeTracking/text/simple-paragraph-split/simple-paragraph-split-tracked.odt";
    QTest::newRow("ParagraphSplit-2")  << "ChangeTracking/text/split-paragraph-with-text-addition/split-para-with-added-text-tracked.odt";
}

bool TestChangeTracking::verifyContentXml(QString &originalFileName, QString &roundTripFileName)
{
    KoStore *originalReadStore = KoStore::createStore(originalFileName, KoStore::Read, "", KoStore::Zip);
    QString originalDocumentString;

    QDomDocument originalDocument("originalDocument");
    originalReadStore->open("content.xml");
    originalDocument.setContent(originalReadStore->device());
    QTextStream originalDocumentStream(&originalDocumentString);
    originalDocumentStream << originalDocument.documentElement().namedItem("office:body");
    originalReadStore->close();

    KoStore *roundTripReadStore = KoStore::createStore(roundTripFileName, KoStore::Read, "", KoStore::Zip);
    QString roundTripDocumentString;

    QDomDocument roundTripDocument("roundTripDocument");
    roundTripReadStore->open("content.xml");
    roundTripDocument.setContent(roundTripReadStore->device());
    QTextStream roundTripDocumentStream(&roundTripDocumentString);
    roundTripDocumentStream << roundTripDocument.documentElement().namedItem("office:body");
    roundTripReadStore->close();

    bool returnValue = (originalDocumentString == roundTripDocumentString);
    return returnValue;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    return QTest::qExec(new TestChangeTracking, argc, argv);
}

#include <TestChangeTracking.moc>
