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

#include <QtGui/QApplication>
#include <QtXml/QDomDocument>
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <KDebug>
#include <QtScript>
#include <QtTest/QtTest>

#include <KStyleManager.h>
#include <KOdfStylesReader.h>
#include <KOdfStore.h>
#include <KTextLoader.h>
#include <KXmlReader.h>
#include <KOdfStoreReader.h>
#include <KOdfWriteStore.h>
#include <KTemporaryFile>
#include <KOdfStorageDevice.h>
#include <KXmlWriter.h>
#include <KTextShapeData.h>
#include <KShapeLoadingContext.h>
#include <KOdfLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KOdfGenericStyles.h>
#include <KOdfXmlNS.h>
#include <kcomponentdata.h>
#include <KTextDebug_p.h>
#include <KListStyle.h>
#include <KTableStyle.h>
#include <KTableCellStyle.h>
#include <KTextDocumentLayout.h>
#include <KCharacterStyle.h>
#include <KParagraphStyle.h>
#include <KOdfText.h>
#include <KOdfEmbeddedDocumentSaver.h>
#include <KInlineTextObjectManager.h>
#include <KTextSharedLoadingData.h>
#include <KTextSharedSavingData.h>
#include <KTextDocument.h>
#include <kstandarddirs.h>

#include <KOdfGenericChanges.h>
#include <KChangeTracker.h>
#include <KDeleteChangeMarker.h>
#include <KChangeTrackerElement.h>

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

QTextDocument *TestChangeTracking::documentFromOdt(const QString &odt, const QString &changeFormat)
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
    if (changeFormat == "DeltaXML")
        changeTracker->setSaveFormat(KChangeTracker::DELTAXML);
    else
        changeTracker->setSaveFormat(KChangeTracker::ODF_1_2);


    KOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store(), *componentData);
    KShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0);
    KTextSharedLoadingData *textSharedLoadingData = new KTextSharedLoadingData;
    textSharedLoadingData->loadOdfStyles(shapeLoadingContext, styleManager);
    shapeLoadingContext.addSharedData(KODFTEXT_SHARED_LOADING_ID, textSharedLoadingData);

    KTextShapeData *textShapeData = new KTextShapeData;
    QTextDocument *document = new QTextDocument;
    textShapeData->setDocument(document, false /* ownership */);
    KTextDocumentLayout *layout = new KTextDocumentLayout(textShapeData->document());
    layout->setInlineTextObjectManager(new KInlineTextObjectManager(layout)); // required while saving
    KTextDocument(document).setStyleManager(styleManager);
    textShapeData->document()->setDocumentLayout(layout);
    KTextDocument(document).setChangeTracker(changeTracker);

    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KTextShapeData failed to load ODT";
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

    KOdfStore *store = KOdfStore::createStore(odt, KOdfStore::Write, "application/vnd.oasis.opendocument.text", KOdfStore::Zip);
    KOdfWriteStore odfWriteStore(store);
    KXmlWriter *manifestWriter = odfWriteStore.manifestWriter("application/vnd.oasis.opendocument.text");
    manifestWriter->addManifestEntry("content.xml", "text/xml");
    if (!store->open("content.xml"))
        return QString();

    KOdfStorageDevice contentDev(store);
    KXmlWriter* contentWriter = KOdfWriteStore::createOasisXmlWriter(&contentDev, "office:document-content");

    // for office:body
    KTemporaryFile contentTmpFile;
    if (!contentTmpFile.open())
        qFatal("Error opening temporary file!");
    KXmlWriter xmlWriter(&contentTmpFile, 1);

    KOdfGenericStyles mainStyles;
    KStyleManager *styleMan = KTextDocument(document).styleManager();
    Q_UNUSED(styleMan);
    KOdfEmbeddedDocumentSaver embeddedSaver;

    KOdfGenericChanges changes;
    KShapeSavingContext context(xmlWriter, mainStyles, embeddedSaver);

    KSharedSavingData *sharedData = context.sharedData(KODFTEXT_SHARED_SAVING_ID);
    KTextSharedSavingData *textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KTextSharedSavingData *>(sharedData);
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << textSharedData;

    if (!textSharedData) {
        textSharedData = new KTextSharedSavingData();
        textSharedData->setGenChanges(changes);
        if (!sharedData) {
            context.addSharedData(KODFTEXT_SHARED_SAVING_ID, textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KODFTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }

    KTextShapeData *textShapeData = new KTextShapeData;
    textShapeData->setDocument(document, false /* ownership */);
    if (qobject_cast<KTextDocumentLayout *>(document->documentLayout()) == 0) {
        // Setup layout and managers just like kotext
        KTextDocumentLayout *layout = new KTextDocumentLayout(textShapeData->document());
        textShapeData->document()->setDocumentLayout(layout);
        layout->setInlineTextObjectManager(new KInlineTextObjectManager(layout)); // required while saving
        KStyleManager *styleManager = new KStyleManager;
        KTextDocument(textShapeData->document()).setStyleManager(styleManager);
    }

    textShapeData->saveOdf(context);

    contentTmpFile.close();

    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);

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
    QFETCH(QString, changeFormat);
    QString testFileName = testcase;
    testFileName.prepend(QString(FILES_DATA_DIR));

    QTextDocument *originalDocument = documentFromOdt(testFileName, changeFormat);
    QString roundTripFileName = documentToOdt(originalDocument);

    QVERIFY(verifyContentXml(testFileName, roundTripFileName));
}

void TestChangeTracking::testChangeTracking_data()
{
    QTest::addColumn<QString>("testcase");
    QTest::addColumn<QString>("changeFormat");
    
    // Text unit-test-cases
    QTest::newRow("Simple Text Insertion") << "ChangeTracking/text/simple-addition/simple-addition-tracked.odt" << "DeltaXML";
    QTest::newRow("Simple Text Deletion")  << "ChangeTracking/text/simple-deletion/simple-deletion-tracked.odt" << "DeltaXML";
    QTest::newRow("Numbered Paragraph Addition")  << "ChangeTracking/text/simple-numbered-paragraph-addition/simple-numbered-paragraph-addition-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Addition")  << "ChangeTracking/text/simple-paragraph-addition/simple-paragraph-addition-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Deletion")  << "ChangeTracking/text/simple-paragraph-deletion/simple-paragraph-deletion-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-1")  << "ChangeTracking/text/simple-paragraph-split/simple-paragraph-split-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-2")  << "ChangeTracking/text/split-paragraph-with-text-addition/split-para-with-added-text-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-3")  << "ChangeTracking/text/split-paragraph-by-table-addition/para-split-by-table-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-4")  << "ChangeTracking/text/split-paragraph-by-table-and-text/split-para-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Merge")  << "ChangeTracking/text/simple-paragraph-merge/simple-paragraph-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("Different Element Merge")  << "ChangeTracking/text/different-element-merge/different-element-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("Multiple Paragraph Merge")  << "ChangeTracking/text/multiple-paragraph-merge/multiple-paragraph-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("Merge Across Table")  << "ChangeTracking/text/paragraph-merge-across-table/merge-across-table-tracked.odt" << "DeltaXML";
    
    // List unit-test-cases
    QTest::newRow("List Item Insertion")  << "ChangeTracking/lists/added-list-item/added-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("List Insertion")  << "ChangeTracking/lists/added-list/added-list-tracked.odt" << "DeltaXML";
    QTest::newRow("Multi Para List Item Insertion")  << "ChangeTracking/lists/added-multi-para-list-item/added-multi-para-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("Multi Para List Item Insertion Full")  << "ChangeTracking/lists/added-multi-para-list-item-full/added-multi-para-list-item-full-tracked.odt" << "DeltaXML";
    QTest::newRow("Multi Para List Item Insertion Partial")  << "ChangeTracking/lists/added-multi-para-list-item-partial/added-multi-para-list-item-partial-tracked.odt" << "DeltaXML";
    QTest::newRow("Sub-List Insertion Full")  << "ChangeTracking/lists/added-sublist-full/added-sublist-full-tracked.odt" << "DeltaXML";
    QTest::newRow("Sub-List Insertion Partial")  << "ChangeTracking/lists/added-sublist-partial/added-sublist-partial-tracked.odt" << "DeltaXML";
    QTest::newRow("List Deletion")  << "ChangeTracking/lists/deleted-list/deleted-list-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Deletion")  << "ChangeTracking/lists/deleted-list-item/deleted-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Merge")  << "ChangeTracking/lists/list-item-merge/list-item-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Split")  << "ChangeTracking/lists/list-item-split/list-item-split-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Numbering Restarted")  << "ChangeTracking/attributes/attribute-addition/restarted-numbering-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Numbering Restart Removed")  << "ChangeTracking/attributes/attribute-deletion/restarted-list-numbering-removed-tracked.odt" << "DeltaXML";

    //Table unit-test-cases
    QTest::newRow("Added Table")  << "ChangeTracking/tables/added-table/added-table-tracked.odt" << "DeltaXML";
    QTest::newRow("Deleted Table")  << "ChangeTracking/tables/deleted-table/deleted-table-tracked.odt" << "DeltaXML";
    QTest::newRow("Added Row")  << "ChangeTracking/tables/added-row/added-row-tracked.odt" << "DeltaXML";
    QTest::newRow("Deleted Row")  << "ChangeTracking/tables/deleted-row/deleted-row-tracked.odt" << "DeltaXML";
    QTest::newRow("Added Column")  << "ChangeTracking/tables/added-column/added-column-tracked.odt" << "DeltaXML";
    QTest::newRow("Deleted Column")  << "ChangeTracking/tables/deleted-column/deleted-column-tracked.odt" << "DeltaXML";

    //Text Style Changes
    QTest::newRow("Text Made Bold")  << "ChangeTracking/styling/text-made-bold/text-made-bold-tracked.odt" << "DeltaXML";
    QTest::newRow("Bold Made Normal")  << "ChangeTracking/styling/bold-text-unstyled/bold-text-unstyled-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Style Changes")  << "ChangeTracking/styling/create-new-style-for-para/new-style-for-para-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Style Changes - 2")  << "ChangeTracking/attributes/attribute-modification/paragraph-style-change-tracked.odt" << "DeltaXML";

    //Complex Delete Merges
    QTest::newRow("Paragraph with list-item - 1")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-list-item/paragraph-merge-with-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph with list-item - 2")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-list-item-from-a-list-at-a-higher-level/paragraph-merge-with-list-item-from-a-list-at-a-higher-level-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph with list-item - 3")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-list-item-simple/paragraph-merge-with-list-item-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item with a Paragraph - 1")  << "ChangeTracking/complex-delete-merges/list-item-merge-with-a-succeeding-paragraph/list-item-merge-with-a-succeeding-paragraph-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item with a Paragraph - 2")  << "ChangeTracking/complex-delete-merges/list-item-from-a-higher-level-list-merge-with-a-succeeding-paragraph/list-item-from-a-higher-level-list-merge-with-a-succeeding-paragraph-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item with a Paragraph - 3")  << "ChangeTracking/complex-delete-merges/list-item-merge-with-a-succeeding-paragraph-simple/list-item-merge-with-a-succeeding-paragraph-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Merges")  << "ChangeTracking/complex-delete-merges/list-item-merge-simple/list-item-merge-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("List Merges")  << "ChangeTracking/complex-delete-merges/list-merges/list-merges-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph with header")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-header-simple/paragraph-merge-with-header-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("Header with Paragraph")  << "ChangeTracking/complex-delete-merges/header-merge-with-paragrah-simple/header-merge-with-paragrah-simple-tracked.odt" << "DeltaXML";
    
    //Other tests
    //QTest::newRow("Others-1")  << "ChangeTracking/other/michiels-deletion-sample/delete-text-across-siblings-tracked.odt";
    QTest::newRow("Others-2")  << "ChangeTracking/other/list-id-sample/list-sample-tracked.odt" << "DeltaXML";
    //QTest::newRow("Others-3")  << "ChangeTracking/other/list-table-list-1/list-table-list-tracked.odt" << "DeltaXML";

    //Multiple and Overlapping changes
    QTest::newRow("Multiple Paragraph Changes")  << "ChangeTracking/multiple-changes/para-add-then-delete/para-add-delete-tracked.odt" << "DeltaXML";
    QTest::newRow("Multiple Span Changes")  << "ChangeTracking/multiple-changes/insert-delete-span/insert-delete-span-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Overlapping")  << "ChangeTracking/overlapping/text-delete-within-added-p/text-delete-within-added-p-tracked.odt" << "DeltaXML";
    QTest::newRow("List Overlapping")  << "ChangeTracking/overlapping/insert-list-item-delete-list/insert-list-item-delete-list-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Add Then Merge")  << "ChangeTracking/multiple-changes/para-add-then-merge/para-add-merge-tracked.odt" << "DeltaXML";
    
    //ODF 1.2 Change Tracking tests     
    QTest::newRow("Simple Text Insertion")  << "ChangeTracking/odf12/simple-text-addition/simple-text-addition-tracked.odt" << "ODF12";
    QTest::newRow("Simple Text Deletion")  << "ChangeTracking/odf12/simple-text-deletion/simple-text-deletion-tracked.odt" << "ODF12";
    QTest::newRow("Text Format Changes")  << "ChangeTracking/odf12/text-format-changes/text-format-changes-tracked.odt" << "ODF12";
    QTest::newRow("Paragraph Merge - ODF12")  << "ChangeTracking/odf12/paragraph-merge/paragraph-merge-tracked.odt" << "ODF12";
}

bool TestChangeTracking::verifyContentXml(QString &originalFileName, QString &roundTripFileName)
{
    KOdfStore *originalReadStore = KOdfStore::createStore(originalFileName, KOdfStore::Read, "", KOdfStore::Zip);
    QString originalDocumentString;

    QDomDocument originalDocument("originalDocument");
    originalReadStore->open("content.xml");
    originalDocument.setContent(originalReadStore->device());
    QTextStream originalDocumentStream(&originalDocumentString);
    originalDocumentStream << originalDocument.documentElement().namedItem("office:body");
    originalReadStore->close();

    KOdfStore *roundTripReadStore = KOdfStore::createStore(roundTripFileName, KOdfStore::Read, "", KOdfStore::Zip);
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
