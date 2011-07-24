/* This file is part of the KDE project
 * Copyright (C) 2001-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007,2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
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

#include "KTextLoader.h"
#include "KTextLoader_p.h"

#include <KTextMeta.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>
#include <KInlineNote.h>
#include <KInlineTextObjectManager.h>
#include "KoList.h"
#include <KOdfLoadingContext.h>
#include <KOdfStylesReader.h>
#include <KProperties.h>
#include <KShapeContainer.h>
#include <KShapeFactoryBase.h>
#include <KShapeLoadingContext.h>
#include <KShapeRegistry.h>
#include <KTableColumnAndRowStyleManager.h>
#include <KTextAnchor.h>
#include <KTextBlockData.h>
#include "KTextDebug_p.h"
#include "KTextDocument.h"
#include <KTextDocumentLayout.h>
#include <KTextShapeData.h>
#include "KTextSharedLoadingData.h"
#include <KUnit.h>
#include <KVariable.h>
#include <KVariableManager.h>
#include <KInlineObjectRegistry.h>
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include "KTextInlineRdf.h"

#include "changetracker/KChangeTracker.h"
#include "changetracker/KChangeTrackerElement.h"
#include "changetracker/KDeleteChangeMarker.h"
#include <KFormatChangeInformation_p.h>
#include "styles/KStyleManager.h"
#include "styles/KParagraphStyle.h"
#include "styles/KCharacterStyle.h"
#include "styles/KListStyle.h"
#include "styles/KListLevelProperties.h"
#include "styles/KTableStyle.h"
#include "styles/KTableColumnStyle.h"
#include "styles/KTableCellStyle.h"
#include "styles/KSectionStyle.h"

#include <klocale.h>
#include <kdebug.h>

#include <QList>
#include <QMap>
#include <QRect>
#include <QStack>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>
#include <QTextTable>
#include <QTime>
#include <QString>
#include <QTextDocumentFragment>

#include <QTextStream>
#include <QXmlStreamReader>

// if defined then debugging is enabled
// #define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KTextLoader::Private
{
public:
    KShapeLoadingContext &context;
    KTextSharedLoadingData *textSharedData;
    // store it here so that you don't need to get it all the time from
    // the KOdfLoadingContext.
    bool stylesDotXml;

    QTextBlockFormat defaultBlockFormat;
    QTextCharFormat defaultCharFormat;
    int bodyProgressTotal;
    int bodyProgressValue;
    int nextProgressReportMs;
    QTime dt;

    KoList *currentList;
    KListStyle *currentListStyle;
    int currentListLevel;
    // Two lists that follow the same style are considered as one for numbering purposes
    // This hash keeps all the lists that have the same style in one KoList.
    QHash<KListStyle *, KoList *> lists;

    KStyleManager *styleManager;

    KChangeTracker *changeTracker;

    KShape *shape;

    int loadSpanLevel;
    int loadSpanInitialPos;
    QStack<int> changeStack;
    QMap<QString, int> changeTransTable;
    QMap<QString, KXmlElement> deleteChangeTable;
    QMap<QString, QString> endIdMap;
    QMap<QString, int> splitPositionMap;

    // For handling complex-deletes i.e delete changes that merges elements of different types
    int openedElements;
    QMap<QString, QString> removeLeavingContentMap;
    QMap<QString, QString> removeLeavingContentChangeIdMap;
    QVector<QString> nameSpacesList;
    bool deleteMergeStarted;
    void copyRemoveLeavingContentStart(const KXmlNode &node, QTextStream &xmlStream);
    void copyRemoveLeavingContentEnd(const KXmlNode &node, QTextStream &xmlStream);
    void copyInsertAroundContent(const KXmlNode &node, QTextStream &xmlStream);
    void copyNode(const KXmlNode &node, QTextStream &xmlStream, bool copyOnlyChildren = false);
    void copyTagStart(const KXmlElement &element, QTextStream &xmlStream, bool ignoreChangeAttributes = false);
    void copyTagEnd(const KXmlElement &element, QTextStream &xmlStream);

    //For handling delete changes
    KDeleteChangeMarker *insertDeleteChangeMarker(QTextCursor &cursor, const QString &id);
    void processDeleteChange(QTextCursor &cursor);

    // For Merging consecutive delete changes into a single change
    bool checkForDeleteMerge(QTextCursor &cursor, const QString &id, int startPosition);
    QMap<KDeleteChangeMarker *, QPair<int, int> > deleteChangeMarkerMap;

    // For Loading of list item splits
    bool checkForListItemSplit(const KXmlElement &element);
    KXmlNode loadListItemSplit(const KXmlElement &element, QString *generatedXmlString);

    explicit Private(KShapeLoadingContext &context, KShape *s)
            : context(context),
            textSharedData(0),
            // stylesDotXml says from where the office:automatic-styles are to be picked from:
            // the content.xml or the styles.xml (in a multidocument scenario). It does not
            // decide from where the office:styles are to be picked (always picked from styles.xml).
            // For our use here, stylesDotXml is always false (see ODF1.1 spec §2.1).
            stylesDotXml(context.odfLoadingContext().useStylesAutoStyles()),
            bodyProgressTotal(0),
            bodyProgressValue(0),
            nextProgressReportMs(0),
            currentList(0),
            currentListStyle(0),
            currentListLevel(1),
            styleManager(0),
            changeTracker(0),
            shape(s),
            loadSpanLevel(0),
            loadSpanInitialPos(0),
            openedElements(0),
            deleteMergeStarted(false)
    {
        dt.start();
    }

    ~Private() {
        kDebug(32500) << "Loading took" << (float)(dt.elapsed()) / 1000 << " seconds";
    }

    KoList *list(const QTextDocument *document, KListStyle *listStyle);

    void openChangeRegion(const KXmlElement &element);
    void closeChangeRegion(const KXmlElement &element);
    void splitStack(int id);
};

class AttributeChangeRecord {
    public:
        AttributeChangeRecord():isValid(false){};

        void setChangeRecord(const QString& changeRecord)
        {
            QStringList strList = changeRecord.split(',');
            this->changeId = strList.value(0);
            this->changeType = strList.value(1);
            this->attributeName = strList.value(2);
            this->attributeValue = strList.value(3);
            this->isValid = true;
        };

        bool isValid;
        QString changeId;
        QString changeType;
        QString attributeName;
        QString attributeValue;
};

bool KTextLoader::containsRichText(const KXmlElement &element)
{
    KXmlElement textParagraphElement;
    forEachElement(textParagraphElement, element) {

        if (textParagraphElement.localName() != "p" ||
            textParagraphElement.namespaceURI() != KOdfXmlNS::text)
            return true;

        // if any of this nodes children are elements, we're dealing with richtext (exceptions: text:s (space character) and text:tab (tab character)
        for (KXmlNode n = textParagraphElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
            const KXmlElement e = n.toElement();
            if (!e.isNull() && (e.namespaceURI() != KOdfXmlNS::text
                || (e.localName() != "s" // space
                && e.localName() != "annotation"
                && e.localName() != "bookmark"
                && e.localName() != "line-break"
                && e.localName() != "meta"
                && e.localName() != "tab" //\\t
                && e.localName() != "tag")))
                return true;
        }
    }
    return false;
}

void KTextLoader::Private::openChangeRegion(const KXmlElement& element)
{
    QString id;
    AttributeChangeRecord attributeChange;

    if (element.localName() == "change-start") {
        //This is a ODF 1.1 Change
        id = element.attributeNS(KOdfXmlNS::text, "change-id");
    } else if(element.localName() == "inserted-text-start") {
        //This is a ODF 1.2 Change
        id = element.attributeNS(KOdfXmlNS::delta, "insertion-change-idref");
        QString textEndId = element.attributeNS(KOdfXmlNS::delta, "inserted-text-end-idref");
        endIdMap.insert(textEndId, id);
    } else if((element.localName() == "removed-content") || (element.localName() == "merge")) {
        id = element.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
    } else if(element.localName() == "remove-leaving-content-start") {
        id = element.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
        QString endId = element.attributeNS(KOdfXmlNS::delta, "end-element-idref");
        endIdMap.insert(endId, id);
    } else if(!element.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty()) {
        QString insertionType = element.attributeNS(KOdfXmlNS::delta, "insertion-type");
        if ((insertionType == "insert-with-content") || (insertionType == "insert-around-content")) {
            id = element.attributeNS(KOdfXmlNS::delta, "insertion-change-idref");
        }
    } else if(!element.attributeNS(KOdfXmlNS::ac, "change001").isEmpty()) {
        attributeChange.setChangeRecord(element.attributeNS(KOdfXmlNS::ac, "change001"));
        id = attributeChange.changeId;
    } else {
    }

    int changeId = changeTracker->loadedChangeId(id);
    if (!changeId)
        return;

    if (!changeStack.empty() && (changeStack.top() != changeId)) {
        //Parent child relationship is defined by the order in which the change meta-data is seen
        //So check the changeId to set the parent-child relationship
        if (changeId > changeStack.top()) {
            changeTracker->setParent(changeId, changeStack.top());
            changeStack.push(changeId);
        } else {
            int duplicateId = changeTracker->createDuplicateChangeId(changeStack.top());
            changeTracker->setParent(duplicateId, changeId);
            changeStack.push(duplicateId);
        }
    } else {
        changeStack.push(changeId);
    }

    changeTransTable.insert(id, changeId);

    KChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
    changeElement->setEnabled(true);

    if ((element.localName() == "remove-leaving-content-start")) {
        changeElement->setChangeType(KOdfGenericChange::FormatChange);

        KXmlElement spanElement = element.firstChild().toElement();
        QString styleName = spanElement.attributeNS(KOdfXmlNS::text, "style-name", QString());

        QTextCharFormat cf;
        KCharacterStyle *characterStyle = textSharedData->characterStyle(styleName, stylesDotXml);
        if (characterStyle) {
             characterStyle->applyStyle(cf);
        }

        KTextStyleChangeInformation *formatChangeInformation = new KTextStyleChangeInformation();
        formatChangeInformation->setPreviousCharFormat(cf);
        changeTracker->setFormatChangeInformation(changeId, formatChangeInformation);
    } else if((element.localName() == "p") && attributeChange.isValid) {
        changeElement->setChangeType(KOdfGenericChange::FormatChange);
        QTextBlockFormat blockFormat;
        if (attributeChange.attributeName == "text:style-name") {
            QString styleName = attributeChange.attributeValue;
            KParagraphStyle *paragraphStyle = textSharedData->paragraphStyle(styleName, stylesDotXml);
            if (paragraphStyle) {
                paragraphStyle->applyStyle(blockFormat);
            }
        }

        KParagraphStyleChangeInformation *paragraphChangeInformation = new KParagraphStyleChangeInformation();
        paragraphChangeInformation->setPreviousBlockFormat(blockFormat);
        changeTracker->setFormatChangeInformation(changeId, paragraphChangeInformation);
    } else if((element.localName() == "list-item") && attributeChange.isValid) {
        changeElement->setChangeType(KOdfGenericChange::FormatChange);
        if (attributeChange.changeType == "insert") {
            KListItemNumChangeInformation *listItemChangeInformation = new KListItemNumChangeInformation(KListItemNumChangeInformation::eNumberingRestarted);
            changeTracker->setFormatChangeInformation(changeId, listItemChangeInformation);
        } else if (attributeChange.changeType == "remove") {
            KListItemNumChangeInformation *listItemChangeInformation = new KListItemNumChangeInformation(KListItemNumChangeInformation::eRestartRemoved);
            listItemChangeInformation->setPreviousStartNumber(attributeChange.attributeValue.toInt());
            changeTracker->setFormatChangeInformation(changeId, listItemChangeInformation);
        }
    } else if((element.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content")) {
        changeElement->setChangeType(KOdfGenericChange::FormatChange);
    } else if ((element.localName() == "removed-content") || (element.localName() == "merge")) {
        changeElement->setChangeType(KOdfGenericChange::DeleteChange);
    }
}

void KTextLoader::Private::closeChangeRegion(const KXmlElement& element)
{
    QString id;
    int changeId;
    if (element.localName() == "change-end") {
        //This is a ODF 1.1 Change
        id = element.attributeNS(KOdfXmlNS::text, "change-id");
    } else if(element.localName() == "inserted-text-end"){
        // This is a ODF 1.2 Change
        QString textEndId = element.attributeNS(KOdfXmlNS::delta, "inserted-text-end-id");
        id = endIdMap.value(textEndId);
        endIdMap.remove(textEndId);
    } else if((element.localName() == "removed-content") || (element.localName() == "merge")) {
        id = element.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
    } else if(element.localName() == "remove-leaving-content-end"){
        QString endId = element.attributeNS(KOdfXmlNS::delta, "end-element-id");
        id = endIdMap.value(endId);
        endIdMap.remove(endId);
    } else if(!element.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty()){
        QString insertionType = element.attributeNS(KOdfXmlNS::delta, "insertion-type");
        if ((insertionType == "insert-with-content") || (insertionType == "insert-around-content")) {
            id = element.attributeNS(KOdfXmlNS::delta, "insertion-change-idref");
        }
    } else if(!element.attributeNS(KOdfXmlNS::ac, "change001").isEmpty()) {
        AttributeChangeRecord attributeChange;
        attributeChange.setChangeRecord(element.attributeNS(KOdfXmlNS::ac, "change001"));
        id = attributeChange.changeId;
    } else {
    }

    changeId = changeTracker->loadedChangeId(id);
    splitStack(changeId);
}

void KTextLoader::Private::splitStack(int id)
{
    if (changeStack.isEmpty())
        return;
    int oldId = changeStack.top();
    changeStack.pop();
    if ((id == oldId) || changeTracker->isParent(id, oldId))
        return;
    int newId = changeTracker->split(oldId);
    splitStack(id);
    changeTracker->setParent(newId, changeStack.top());
    changeStack.push(newId);
}

KoList *KTextLoader::Private::list(const QTextDocument *document, KListStyle *listStyle)
{
    if (lists.contains(listStyle))
        return lists[listStyle];
    KoList *newList = new KoList(document, listStyle);
    lists[listStyle] = newList;
    return newList;
}

/////////////KTextLoader

KTextLoader::KTextLoader(KShapeLoadingContext &context, KShape *shape)
        : QObject()
        , d(new Private(context, shape))
{
    KSharedLoadingData *sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    if (sharedData) {
        d->textSharedData = dynamic_cast<KTextSharedLoadingData *>(sharedData);
    }

    //kDebug(32500) << "sharedData" << sharedData << "textSharedData" << d->textSharedData;

    if (!d->textSharedData) {
        d->textSharedData = new KTextSharedLoadingData();
        KResourceManager *rm = context.documentResourceManager();
        KStyleManager *styleManager = rm->resource(KoText::StyleManager).value<KStyleManager*>();
        d->textSharedData->loadOdfStyles(context, styleManager);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_LOADING_ID, d->textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_LOADING_ID;
            Q_ASSERT(false);
        }
    }
}

KTextLoader::~KTextLoader()
{
    delete d;
}

void KTextLoader::loadBody(const KXmlElement &bodyElem, QTextCursor &cursor)
{
    static int rootCallChecker = 0;
    if (rootCallChecker == 0) {
        //This is the first call of loadBody.
        //Store the default block and char formats
        //Will be used whenever a new block is inserted
        d->defaultBlockFormat = cursor.blockFormat();
        d->defaultCharFormat = cursor.charFormat();
    }
    rootCallChecker++;

    cursor.beginEditBlock();
    const QTextDocument *document = cursor.block().document();

    d->styleManager = KTextDocument(document).styleManager();
    d->changeTracker = KTextDocument(document).changeTracker();

    kDebug(32500) << "text-style:" << KTextDebug::textAttributes(cursor.blockCharFormat());
    bool usedParagraph = false; // set to true if we found a tag that used the paragraph, indicating that the next round needs to start a new one.
    if (bodyElem.namespaceURI() == KOdfXmlNS::table && bodyElem.localName() == "table") {
        if (!bodyElem.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
            d->openChangeRegion(bodyElem);
        loadTable(bodyElem, cursor);
        if (!bodyElem.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
            d->closeChangeRegion(bodyElem);
    }
    else {
        startBody(KoXml::childNodesCount(bodyElem));

        KXmlElement tag;
        for ( KXmlNode _node = bodyElem.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
        if ( ( tag = _node.toElement() ).isNull() ) {
            //Don't do anything
        } else {
            if (! tag.isNull()) {
                const QString localName = tag.localName();
                if (tag.namespaceURI() == KOdfXmlNS::delta) {
                    if (d->changeTracker && localName == "tracked-changes")
                        d->changeTracker->loadOdfChanges(tag);
                    else if (d->changeTracker && localName == "removed-content") {
                        QString changeId = tag.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
                        int deleteStartPosition = cursor.position();
                        if ((usedParagraph) && (tag.firstChild().toElement().localName() != "table"))
                            cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);

                        d->openChangeRegion(tag);
                        loadBody(tag, cursor);
                        d->closeChangeRegion(tag);

                        if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
                            QTextCursor tempCursor(cursor);
                            tempCursor.setPosition(deleteStartPosition);
                            KDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
                            d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
                        }

                        if (tag.lastChild().toElement().localName() == "table") {
                            usedParagraph = false;
                        }

                    } else if (d->changeTracker && localName == "remove-leaving-content-start"){
                        if (usedParagraph)
                            cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                        usedParagraph = true;
                        QString generatedXmlString;
                        _node = loadDeleteMerges(tag,&generatedXmlString);
                        //Parse and Load the generated xml
                        QString errorMsg;
                        int errorLine, errorColumn;
                        KXmlDocument doc;

                        QXmlStreamReader reader(generatedXmlString);
                        reader.setNamespaceProcessing(true);

                        bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                        if (ok) {
                            loadBody(doc.documentElement(), cursor);
                        }
                    } else {
                    }
                }

                if (tag.namespaceURI() == KOdfXmlNS::text) {
                    if ((usedParagraph) && (tag.localName() != "table"))
                        cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                    usedParagraph = true;
                    if (d->changeTracker && localName == "tracked-changes") {
                        d->changeTracker->loadOdfChanges(tag);
                        storeDeleteChanges(tag);
                        usedParagraph = false;
                    } else if (d->changeTracker && localName == "change-start") {
                        d->openChangeRegion(tag);
                        usedParagraph = false;
                    } else if (d->changeTracker && localName == "change-end") {
                        d->closeChangeRegion(tag);
                        usedParagraph = false;
                    } else if (d->changeTracker && localName == "change") {
                        QString id = tag.attributeNS(KOdfXmlNS::text, "change-id");
                        int changeId = d->changeTracker->loadedChangeId(id);
                        if (changeId) {
                            if (d->changeStack.count() && (d->changeStack.top() != changeId))
                                d->changeTracker->setParent(changeId, d->changeStack.top());
                            KDeleteChangeMarker *deleteChangemarker = new KDeleteChangeMarker(d->changeTracker);
                            deleteChangemarker->setChangeId(changeId);
                            KChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
                            changeElement->setDeleteChangeMarker(deleteChangemarker);
                            changeElement->setEnabled(true);
                            KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
                            if (layout) {
                                KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                                textObjectManager->insertInlineObject(cursor, deleteChangemarker);
                            }
                        }

                        loadDeleteChangeOutsidePorH(id, cursor);
                        usedParagraph = false;
                    } else if (localName == "p") {    // text paragraph
                        if (tag.attributeNS(KOdfXmlNS::delta, "insertion-type") != "insert-around-content") {
                            if (!tag.attributeNS(KOdfXmlNS::split, "split001-idref").isEmpty())
                                d->splitPositionMap.insert(tag.attributeNS(KOdfXmlNS::split, "split001-idref"),cursor.position());

                            if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty()) {
                                QString insertionType = tag.attributeNS(KOdfXmlNS::delta, "insertion-type");
                                if (insertionType == "insert-with-content") {
                                    d->openChangeRegion(tag);
                                }

                                if (insertionType == "split") {
                                    QString splitId = tag.attributeNS(KOdfXmlNS::delta, "split-id");
                                    QString changeId = tag.attributeNS(KOdfXmlNS::delta, "insertion-change-idref");
                                    markBlocksAsInserted(cursor, d->splitPositionMap.value(splitId), changeId);
                                    d->splitPositionMap.remove(splitId);
                                }
                            } else if (!tag.attributeNS(KOdfXmlNS::ac, "change001").isEmpty()) {
                                    d->openChangeRegion(tag);
                            }

                            loadParagraph(tag, cursor);

                            if ((!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty()) ||
                                 (!tag.attributeNS(KOdfXmlNS::ac, "change001").isEmpty())) {
                                d->closeChangeRegion(tag);
                            }

                        } else {
                            QString generatedXmlString;
                            _node = loadDeleteMerges(tag,&generatedXmlString);
                            //Parse and Load the generated xml
                            QString errorMsg;
                            int errorLine, errorColumn;
                            KXmlDocument doc;

                            QXmlStreamReader reader(generatedXmlString);
                            reader.setNamespaceProcessing(true);

                            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                            if (ok) {
                                loadBody(doc.documentElement(), cursor);
                            }
                        }
                    } else if (localName == "h") {  // heading
                        if (tag.attributeNS(KOdfXmlNS::delta, "insertion-type") != "insert-around-content") {
                            if (!tag.attributeNS(KOdfXmlNS::split, "split001-idref").isEmpty())
                                d->splitPositionMap.insert(tag.attributeNS(KOdfXmlNS::split, "split001-idref"),cursor.position());

                            if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty()) {
                                QString insertionType = tag.attributeNS(KOdfXmlNS::delta, "insertion-type");
                                if (insertionType == "insert-with-content")
                                    d->openChangeRegion(tag);
                                if (insertionType == "split") {
                                    QString splitId = tag.attributeNS(KOdfXmlNS::delta, "split-id");
                                    QString changeId = tag.attributeNS(KOdfXmlNS::delta, "insertion-change-idref");
                                    markBlocksAsInserted(cursor, d->splitPositionMap.value(splitId), changeId);
                                    d->splitPositionMap.remove(splitId);
                                }
                            }

                            loadHeading(tag, cursor);

                            if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                                d->closeChangeRegion(tag);
                        } else {
                            QString generatedXmlString;
                            _node = loadDeleteMerges(tag,&generatedXmlString);
                            //Parse and Load the generated xml
                            QString errorMsg;
                            int errorLine, errorColumn;
                            KXmlDocument doc;

                            QXmlStreamReader reader(generatedXmlString);
                            reader.setNamespaceProcessing(true);

                            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                            if (ok) {
                                loadBody(doc.documentElement(), cursor);
                            }
                        }
                    } else if (localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                            || localName == "list" || localName == "numbered-paragraph") {  // OASIS
                        if (tag.attributeNS(KOdfXmlNS::delta, "insertion-type") != "insert-around-content") {
                            if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                                d->openChangeRegion(tag);
                            loadList(tag, cursor);
                            if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                                d->closeChangeRegion(tag);
                        } else {
                            QString generatedXmlString;
                            _node = loadDeleteMerges(tag,&generatedXmlString);
                            //Parse and Load the generated xml
                            QString errorMsg;
                            int errorLine, errorColumn;
                            KXmlDocument doc;

                            QXmlStreamReader reader(generatedXmlString);
                            reader.setNamespaceProcessing(true);

                            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                            if (ok) {
                                loadBody(doc.documentElement(), cursor);
                            }
                        }
                    } else if (localName == "section") {  // Temporary support (TODO)
                        loadSection(tag, cursor);
                    } else if (localName == "table-of-content") {
                        loadTableOfContents(tag, cursor);
		    } else if (localName == "user-field-decls") {
			loadVariableDeclarations(tag, cursor);
                    } else {
                        KInlineObject *obj = KInlineObjectRegistry::instance()->createFromOdf(tag, d->context);
                        if (obj) {
                            KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
                            if (layout) {
                                KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                                if (textObjectManager) {
                                    KVariableManager *varManager = textObjectManager->variableManager();
                                    if (varManager) {
                                        textObjectManager->insertInlineObject(cursor, obj);
                                    }
                                }
                            }
                        } else {
                            usedParagraph = false;
                            kWarning(32500) << "unhandled text:" << localName;
                        }
                    }
                } else if (tag.namespaceURI() == KOdfXmlNS::draw) {
                    loadShape(tag, cursor);
                } else if (tag.namespaceURI() == KOdfXmlNS::table) {
                    if (localName == "table") {
                        if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                            d->openChangeRegion(tag);
                        loadTable(tag, cursor);
                        usedParagraph = false;
                        if (!tag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                            d->closeChangeRegion(tag);
                    } else {
                        kWarning(32500) << "KTextLoader::loadBody unhandled table::" << localName;
                    }
                }
            }
            processBody();
        }
        endBody();
    }

    if (document->isEmpty() && d->styleManager && !document->begin().blockFormat()
            .hasProperty(KParagraphStyle::StyleId)) {
        QTextBlock block = document->begin();
        d->styleManager->defaultParagraphStyle()->applyStyle(block);
    }

    rootCallChecker--;
    if (rootCallChecker == 0) {
        d->processDeleteChange(cursor);
    }
    cursor.endEditBlock();
}

KXmlNode KTextLoader::loadDeleteMerges(const KXmlElement& elem, QString *generatedXmlString)
{
    KXmlNode lastProcessedNode = elem;
    d->nameSpacesList.clear();

    QString generatedXml;
    QTextStream xmlStream(&generatedXml);
    do {
        KXmlElement element;
        bool isElementNode = lastProcessedNode.isElement();
        if (isElementNode)
            element = lastProcessedNode.toElement();

        if (isElementNode && (element.localName() == "remove-leaving-content-start")) {
            d->copyRemoveLeavingContentStart(element, xmlStream);
        } else if (isElementNode && (element.localName() == "remove-leaving-content-end")) {
            d->copyRemoveLeavingContentEnd(element, xmlStream);
        } else if (isElementNode && (element.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content")) {
            d->copyInsertAroundContent(element, xmlStream);
        } else {
            d->copyNode(element, xmlStream);
        }

        lastProcessedNode = lastProcessedNode.nextSibling();
    } while(d->openedElements && !lastProcessedNode.isNull());

    QTextStream docStream(generatedXmlString);

    docStream << "<generated-xml";
    for (int i=0; i<d->nameSpacesList.size();i++) {
        docStream << " xmlns:ns" << i << "=";
        docStream << "\"" << d->nameSpacesList.at(i) << "\"";
    }
    docStream << ">";
    docStream << generatedXml;
    docStream << "</generated-xml>";

    return lastProcessedNode.previousSibling();
}

void KTextLoader::Private::copyRemoveLeavingContentStart(const KXmlNode &node, QTextStream &xmlStream)
{
    KXmlElement element = node.firstChild().toElement();
    QString changeEndId = node.toElement().attributeNS(KOdfXmlNS::delta, "end-element-idref");
    int index = nameSpacesList.indexOf(element.namespaceURI());
    if (index == -1) {
        nameSpacesList.append(element.namespaceURI());
        index = nameSpacesList.size() - 1;
    }
    QString nodeName  = QString("ns%1") + ':' + element.localName();
    nodeName = nodeName.arg(index);

    removeLeavingContentMap.insert(changeEndId, nodeName);
    openedElements++;

    QString changeId = node.toElement().attributeNS(KOdfXmlNS::delta, "removal-change-idref");
    removeLeavingContentChangeIdMap.insert(changeEndId, changeId);

    xmlStream << "<" << nodeName;
    QList<KoText::StringPair> attributeNSNames = element.attributeNSNames();

    foreach(const KoText::StringPair &attributeName, attributeNSNames) {
        QString nameSpace = attributeName.first;
        if (nameSpace != "http://www.w3.org/XML/1998/namespace") {
            int index = nameSpacesList.indexOf(nameSpace);
            if (index == -1) {
                nameSpacesList.append(nameSpace);
                index = nameSpacesList.size() - 1;
            }
            xmlStream << " " << "ns" << index << ":" << attributeName.second << "=";
        } else {
            xmlStream << " " << "xml:" << attributeName.second << "=";
        }
        xmlStream << "\"" << element.attributeNS(nameSpace, attributeName.second) << "\"";
    }

    xmlStream << ">";

    if (deleteMergeStarted && (nodeName.endsWith(":p") || nodeName.endsWith(":h"))) {
        KXmlElement nextElement = node.nextSibling().toElement();
        if (nextElement.localName() != "removed-content") {
            int deltaIndex = nameSpacesList.indexOf(KOdfXmlNS::delta);
            xmlStream << "<" << "ns" << deltaIndex << ":removed-content ";
            QString changeId = removeLeavingContentChangeIdMap.value(changeEndId);
            xmlStream << "ns" << deltaIndex << ":removal-change-idref=" << "\"" << changeId << "\"" << ">";
            xmlStream << "</" << "ns" << deltaIndex << ":removed-content>";
        }
    }
}

void KTextLoader::Private::copyRemoveLeavingContentEnd(const KXmlNode &node, QTextStream &xmlStream)
{
    QString changeEndId = node.toElement().attributeNS(KOdfXmlNS::delta, "end-element-id");
    QString nodeName = removeLeavingContentMap.value(changeEndId);
    removeLeavingContentMap.remove(changeEndId);
    openedElements--;

    if (nodeName.endsWith(":p") || nodeName.endsWith(":h")) {
        if (!deleteMergeStarted) {
            //We are not already inside a simple delete merge
            //Check Whether the previous sibling is a removed-content.
            //If not, then this is the starting p or h of a simple merge.
            KXmlElement previousElement = node.previousSibling().toElement();
            if (previousElement.localName() != "removed-content") {
                int deltaIndex = nameSpacesList.indexOf(KOdfXmlNS::delta);
                if (deltaIndex == -1) {
                    nameSpacesList.append(KOdfXmlNS::delta);
                    deltaIndex = nameSpacesList.size() - 1;
                }
                xmlStream << "<" << "ns" << deltaIndex << ":removed-content ";
                QString changeId = removeLeavingContentChangeIdMap.value(changeEndId);
                xmlStream << "ns" << deltaIndex << ":removal-change-idref=" << "\"" << changeId << "\"" << ">";
                xmlStream << "</" << "ns" << deltaIndex << ":removed-content>";
            }
            deleteMergeStarted = true;
        } else {
            deleteMergeStarted = false;
        }
    }

    removeLeavingContentChangeIdMap.remove(changeEndId);
    xmlStream << "</" << nodeName << ">";
}

void KTextLoader::Private::copyInsertAroundContent(const KXmlNode &node, QTextStream &xmlStream)
{
    copyNode(node, xmlStream, true);
}

void KTextLoader::Private::copyNode(const KXmlNode &node, QTextStream &xmlStream, bool copyOnlyChildren)
{
    if (node.isText()) {
        xmlStream << node.toText().data();
    } else if (node.isElement()) {
        KXmlElement element = node.toElement();
        if (!copyOnlyChildren) {
            copyTagStart(element, xmlStream);
        }

        for ( KXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() ) {
            KXmlElement childElement;
            bool isElementNode = node.isElement();
            if (isElementNode)
                childElement = node.toElement();


            if (isElementNode && (childElement.localName() == "remove-leaving-content-start")) {
                copyRemoveLeavingContentStart(childElement, xmlStream);
            } else if (isElementNode && (childElement.localName() == "remove-leaving-content-end")) {
                copyRemoveLeavingContentEnd(childElement, xmlStream);
            } else if (isElementNode && (childElement.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content")) {
                copyInsertAroundContent(childElement, xmlStream);
            } else {
                copyNode(node, xmlStream);
            }
        }

        if (!copyOnlyChildren) {
            copyTagEnd(element, xmlStream);
        }
    } else {
    }
}

void KTextLoader::Private::copyTagStart(const KXmlElement &element, QTextStream &xmlStream, bool ignoreChangeAttributes)
{
    int index = nameSpacesList.indexOf(element.namespaceURI());
    if (index == -1) {
        nameSpacesList.append(element.namespaceURI());
        index = nameSpacesList.size() - 1;
    }
    QString nodeName  = QString("ns%1") + ':' + element.localName();
    nodeName = nodeName.arg(index);
    xmlStream << "<" << nodeName;
    QList<KoText::StringPair> attributeNSNames = element.attributeNSNames();

    foreach(const KoText::StringPair &attributeName, attributeNSNames) {
        QString nameSpace = attributeName.first;
        if (nameSpace == KOdfXmlNS::delta && ignoreChangeAttributes) {
            continue;
        }
        if (nameSpace != "http://www.w3.org/XML/1998/namespace") {
            int index = nameSpacesList.indexOf(nameSpace);
            if (index == -1) {
                nameSpacesList.append(nameSpace);
                index = nameSpacesList.size() - 1;
            }
            xmlStream << " " << "ns" << index << ":" << attributeName.second << "=";
        } else {
            xmlStream << " " << "xml:" << attributeName.second << "=";
        }
        xmlStream << "\"" << element.attributeNS(nameSpace, attributeName.second) << "\"";
    }
    xmlStream << ">";
}

void KTextLoader::Private::copyTagEnd(const KXmlElement &element, QTextStream &xmlStream)
{
    int index = nameSpacesList.indexOf(element.namespaceURI());
    QString nodeName  = QString("ns%1") + ':' + element.localName();
    nodeName = nodeName.arg(index);
    xmlStream << "</" << nodeName << ">";
}

void KTextLoader::loadDeleteChangeOutsidePorH(QString id, QTextCursor &cursor)
{
    int startPosition = cursor.position();
    int changeId = d->changeTracker->loadedChangeId(id);

    if (changeId) {
        KChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
        KXmlElement element = d->deleteChangeTable.value(id);

        //Call loadBody with this element
        loadBody(element, cursor);

        int endPosition = cursor.position();

        //Set the char format to the changeId
        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
        cursor.mergeCharFormat(format);

        //Get the QTextDocumentFragment from the selection and store it in the changeElement
        QTextDocumentFragment deletedFragment(cursor);
        changeElement->setDeleteData(deletedFragment);

        //Now Remove this from the document. Will be re-inserted whenever changes have to be seen
        cursor.removeSelectedText();
    }
}

void KTextLoader::loadParagraph(const KXmlElement &element, QTextCursor &cursor)
{
    const QString styleName = element.attributeNS(KOdfXmlNS::text, "style-name",
                                                  QString());

    KParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);

    Q_ASSERT(d->styleManager);
    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
    if (cursor.position() == cursor.block().position()) {
        QTextBlock block = cursor.block();
        if (!paragraphStyle) {
            // Either the paragraph has no style or the style-name could not be found.
            if (!styleName.isEmpty())
                kWarning(32500) << "paragraph style " << styleName << "not found - using default style";
            // all parags should always have the default applied.
            d->styleManager->defaultParagraphStyle()->applyStyle(block);
        } else {
            Q_ASSERT(paragraphStyle->parentStyle()); // should be the default parag style.
            // Apply list style when loading a list but we don't have a list style
            paragraphStyle->applyStyle(block, d->currentList && !d->currentListStyle);
        }
/*
        // Clear the outline level property. If a default-outline-level was set, it should not
        // be applied when loading a document, only on user action.
could be true; but the following code has no effect... (TZander)
        block.blockFormat().clearProperty(KParagraphStyle::OutlineLevel);
*/
    }

    // Some paragraph have id's defined which we need to store so that we can eg
    // attach text animations to this specific paragraph later on
    QString id(element.attributeNS(KOdfXmlNS::text, "id"));
    if (!id.isEmpty() && d->shape) {
        QTextBlock block = cursor.block();
        KTextBlockData *data = dynamic_cast<KTextBlockData*>(block.userData());
        if (!data) {
            data = new KTextBlockData();
            block.setUserData(data);
        }
        d->context.addShapeSubItemId(d->shape, QVariant::fromValue(data), id);
    }

    // attach Rdf to cursor.block()
    // remember inline Rdf metadata
    if (element.hasAttributeNS(KOdfXmlNS::xhtml, "property")
            || element.hasAttribute("id")) {
        QTextBlock block = cursor.block();
        KTextInlineRdf* inlineRdf =
            new KTextInlineRdf((QTextDocument*)block.document(), block);
        inlineRdf->loadOdf(element);
        KTextInlineRdf::attach(inlineRdf, cursor);
    }
    kDebug(32500) << "text-style:" << KTextDebug::textAttributes(cursor.blockCharFormat()) << d->currentList << d->currentListStyle;

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf);   // restore the cursor char format
}

void KTextLoader::loadHeading(const KXmlElement &element, QTextCursor &cursor)
{
    Q_ASSERT(d->styleManager);
    int level = qMax(-1, element.attributeNS(KOdfXmlNS::text, "outline-level", "-1").toInt());
    // This will fallback to the default-outline-level applied by KParagraphStyle

    QString styleName = element.attributeNS(KOdfXmlNS::text, "style-name", QString());

    QTextBlock block = cursor.block();

    // Set the paragraph-style on the block
    KParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);
    if (!paragraphStyle) {
        kWarning(32500) << "paragraph style " << styleName << " not found";
        paragraphStyle = d->styleManager->defaultParagraphStyle();
    }
    if (paragraphStyle) {
        // Apply list style when loading a list but we don't have a list style
        paragraphStyle->applyStyle(block, d->currentList && !d->currentListStyle);
    }

    if ((block.blockFormat().hasProperty(KParagraphStyle::OutlineLevel)) && (level == -1)) {
        level = block.blockFormat().property(KParagraphStyle::OutlineLevel).toInt();
    } else {
        if (level == -1)
            level = 1;
        QTextBlockFormat blockFormat;
        blockFormat.setProperty(KParagraphStyle::OutlineLevel, level);
        cursor.mergeBlockFormat(blockFormat);
    }

    if (!d->currentList) { // apply <text:outline-style> (if present) only if heading is not within a <text:list>
        KListStyle *outlineStyle = d->styleManager->outlineStyle();
        if (outlineStyle) {
            KoList *list = d->list(block.document(), outlineStyle);
            if (!KTextDocument(block.document()).headingList()) {
                KTextDocument(block.document()).setHeadingList(list);
            }
            list->applyStyle(block, outlineStyle, level);
        }
    }

    // attach Rdf to cursor.block()
    // remember inline Rdf metadata
    if (element.hasAttributeNS(KOdfXmlNS::xhtml, "property")
            || element.hasAttribute("id")) {
        QTextBlock block = cursor.block();
        KTextInlineRdf* inlineRdf =
            new KTextInlineRdf((QTextDocument*)block.document(), block);
        inlineRdf->loadOdf(element);
        KTextInlineRdf::attach(inlineRdf, cursor);
    }

    kDebug(32500) << "text-style:" << KTextDebug::textAttributes(cursor.blockCharFormat());

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf);   // restore the cursor char format
}

void KTextLoader::loadList(const KXmlElement &element, QTextCursor &cursor)
{
    const bool numberedParagraph = element.localName() == "numbered-paragraph";

    QString styleName = element.attributeNS(KOdfXmlNS::text, "style-name", QString());
    KListStyle *listStyle = d->textSharedData->listStyle(styleName, d->stylesDotXml);

    int level;

    // TODO: get level from the style, if it has a style:list-level attribute (new in ODF-1.2)
    if (numberedParagraph) {
        d->currentList = d->list(cursor.block().document(), listStyle);
        d->currentListStyle = listStyle;
        level = element.attributeNS(KOdfXmlNS::text, "level", "1").toInt();
    } else {
        if (!listStyle)
            listStyle = d->currentListStyle;
        d->currentList = d->list(cursor.block().document(), listStyle);
        d->currentListStyle = listStyle;
        level = d->currentListLevel++;
    }

    if (element.hasAttributeNS(KOdfXmlNS::text, "continue-numbering")) {
        const QString continueNumbering = element.attributeNS(KOdfXmlNS::text, "continue-numbering", QString());
        d->currentList->setContinueNumbering(level, continueNumbering == "true");
    }

#ifdef KOOPENDOCUMENTLOADER_DEBUG
    if (d->currentListStyle)
        kDebug(32500) << "styleName =" << styleName << "listStyle =" << d->currentListStyle->name()
        << "level =" << level << "hasLevelProperties =" << d->currentListStyle->hasLevelProperties(level)
        //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
        ;
    else
        kDebug(32500) << "styleName =" << styleName << " currentListStyle = 0";
#endif

    KXmlElement e;
    QList<KXmlElement> childElementsList;

    QString generatedXmlString;
    KXmlDocument doc;
    QXmlStreamReader reader;

    for ( KXmlNode _node = element.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
    if ( ( e = _node.toElement() ).isNull() ) {
        //Don't do anything
    } else {
        if ((e.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content") || (e.localName() == "remove-leaving-content-start")) {
            //Check whether this is a list-item split or a merge
            if ((e.localName() == "remove-leaving-content-start") && d->checkForListItemSplit(e)) {
                _node = d->loadListItemSplit(e, &generatedXmlString);
            } else {
                _node = loadDeleteMerges(e,&generatedXmlString);
            }

            //Parse and Load the generated xml
            QString errorMsg;
            int errorLine, errorColumn;

            reader.addData(generatedXmlString);
            reader.setNamespaceProcessing(true);

            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
            QDomDocument dom;
            if (ok) {
                KXmlElement childElement;
                forEachElement (childElement, doc.documentElement()) {
                    childElementsList.append(childElement);
                }
            }
        } else {
            childElementsList.append(e);
        }
    }

    // Iterate over list items and add them to the textlist
    bool firstTime = true;
    foreach (const KXmlElement &elem, childElementsList) {
        if (elem.localName() == "removed-content") {
            QString changeId = elem.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
            int deleteStartPosition = cursor.position();
            d->openChangeRegion(elem);
            KXmlElement deletedElement;
            forEachElement(deletedElement, elem) {
                if (!firstTime && !numberedParagraph)
                    cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                firstTime = false;
                loadListItem(deletedElement, cursor, level);
            }
            d->closeChangeRegion(elem);
            if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
                QTextCursor tempCursor(cursor);
                tempCursor.setPosition(deleteStartPosition);
                KDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
                d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
            }
        } else {
            if (!firstTime && !numberedParagraph)
                cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
            firstTime = false;
            loadListItem(elem, cursor, level);
        }
    }

    if (numberedParagraph || --d->currentListLevel == 1) {
        d->currentListStyle = 0;
        d->currentList = 0;
    }
}

void KTextLoader::loadListItem(const KXmlElement &e, QTextCursor &cursor, int level)
{
    bool numberedParagraph = e.parentNode().toElement().localName() == "numbered-paragraph";

    if (!numberedParagraph && e.parentNode().toElement().localName() == "removed-content") {
        numberedParagraph = e.parentNode().parentNode().toElement().localName() == "numbered-paragraph";
    }

    if (e.isNull() || e.namespaceURI() != KOdfXmlNS::text)
        return;

    const bool listHeader = e.tagName() == "list-header";

    if (!numberedParagraph && e.tagName() != "list-item" && !listHeader)
        return;

    if (!e.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty()) {
        d->openChangeRegion(e);
    } else if (!e.attributeNS(KOdfXmlNS::ac, "change001").isEmpty()){
        d->openChangeRegion(e);
    }

    QTextBlock current = cursor.block();

    QTextBlockFormat blockFormat;

    if (numberedParagraph) {
        if (e.localName() == "p") {
            loadParagraph(e, cursor);
        } else if (e.localName() == "h") {
            loadHeading(e, cursor);
        }
        blockFormat.setProperty(KParagraphStyle::ListLevel, level);
    } else {
        loadBody(e, cursor);
    }

    if (!current.textList()) {
        if (!d->currentList->style()->hasLevelProperties(level)) {
            KListLevelProperties llp;
            // Look if one of the lower levels are defined to we can copy over that level.
            for(int i = level - 1; i >= 0; --i) {
                if(d->currentList->style()->hasLevelProperties(i)) {
                    llp = d->currentList->style()->levelProperties(i);
                    break;
                }
            }
            llp.setLevel(level);
           // TODO make the 10 configurable
            llp.setIndent(level * 10.0);
            d->currentList->style()->setLevelProperties(llp);
        }

        d->currentList->add(current, level);
    }

    if (listHeader)
        blockFormat.setProperty(KParagraphStyle::IsListHeader, true);

    if (e.hasAttributeNS(KOdfXmlNS::text, "start-value")) {
        int startValue = e.attributeNS(KOdfXmlNS::text, "start-value", QString()).toInt();
        blockFormat.setProperty(KParagraphStyle::ListStartValue, startValue);
    }


    // mark intermediate paragraphs as unnumbered items
    QTextCursor c(current);
    c.mergeBlockFormat(blockFormat);
    while (c.block() != cursor.block()) {
        c.movePosition(QTextCursor::NextBlock);
        if (c.block().textList()) // a sublist
            break;
        blockFormat = c.blockFormat();
        blockFormat.setProperty(listHeader ? KParagraphStyle::IsListHeader : KParagraphStyle::UnnumberedListItem, true);
        c.setBlockFormat(blockFormat);
        d->currentList->add(c.block(), level);
    }

    if (!e.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
        d->closeChangeRegion(e);
    kDebug(32500) << "text-style:" << KTextDebug::textAttributes(cursor.blockCharFormat());
}

bool KTextLoader::Private::checkForListItemSplit(const KXmlElement &element)
{
    QString endId = element.attributeNS(KOdfXmlNS::delta, "end-element-idref");
    int insertedListItems = 0;
    KXmlElement currentElement = element;
    bool isSplitListItem = false;

    while(true) {
        currentElement = currentElement.nextSibling().toElement();

        if (currentElement.isNull()) {
            continue;
        }

        if ((currentElement.localName() == "list-item") &&
            (currentElement.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content")) {
            insertedListItems++;
        }

        if ((currentElement.localName() == "remove-leaving-content-end") &&
            (currentElement.attributeNS(KOdfXmlNS::delta, "end-element-id") == endId)) {
            break;
        }
    }

    isSplitListItem = (insertedListItems > 1)?true:false;
    return isSplitListItem;
}

KXmlNode KTextLoader::Private::loadListItemSplit(const KXmlElement &elem, QString *generatedXmlString)
{
    KXmlNode lastProcessedNode = elem;

    nameSpacesList.clear();
    nameSpacesList.append(KOdfXmlNS::split);
    nameSpacesList.append(KOdfXmlNS::delta);

    QString generatedXml;
    QTextStream xmlStream(&generatedXml);

    static int splitIdCounter = 0;
    bool splitStarted = false;

    QString endId = elem.attributeNS(KOdfXmlNS::delta, "end-element-idref");
    QString changeId = elem.attributeNS(KOdfXmlNS::delta, "removal-change-idref");

    while(true) {
        KXmlElement element;
        lastProcessedNode = lastProcessedNode.nextSibling();
        bool isElementNode = lastProcessedNode.isElement();

        if (isElementNode)
            element = lastProcessedNode.toElement();

        if (isElementNode && (element.localName() == "remove-leaving-content-start")) {
            //Ignore this...
        } else if (isElementNode && (element.localName() == "remove-leaving-content-end")) {
            if(element.attributeNS(KOdfXmlNS::delta, "end-element-id") == endId) {
                break;
            }
        } else if (isElementNode && (element.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content")) {
            copyTagStart(element, xmlStream, true);
            KXmlElement childElement;
            forEachElement(childElement, element) {
                if (childElement.attributeNS(KOdfXmlNS::delta, "insertion-type") == "insert-around-content") {
                    copyTagStart(childElement, xmlStream, true);

                    if (splitStarted) {
                        generatedXml.remove((generatedXml.length() - 1), 1);
                        xmlStream << " ns1:split-id=\"split" << splitIdCounter << "\"";
                        xmlStream << " ns1:insertion-change-idref=\"" << changeId << "\"";
                        xmlStream << " ns1:insertion-type=\"split\"";
                        xmlStream << ">";
                        splitStarted = false;
                        splitIdCounter++;
                    } else {
                        generatedXml.remove((generatedXml.length() - 1), 1);
                        xmlStream << " ns0:split001-idref=\"split" << splitIdCounter << "\"";
                        xmlStream << ">";
                        splitStarted = true;
                    }

                    copyNode(childElement, xmlStream, true);
                    copyTagEnd(childElement, xmlStream);
                } else {
                    copyNode(childElement, xmlStream);
                }
            }
            copyTagEnd(element, xmlStream);
        } else {
            copyNode(element, xmlStream);
        }
    }

    QTextStream docStream(generatedXmlString);
    docStream << "<generated-xml";
    for (int i=0; i<nameSpacesList.size();i++) {
        docStream << " xmlns:ns" << i << "=";
        docStream << "\"" << nameSpacesList.at(i) << "\"";
    }
    docStream << ">";
    docStream << generatedXml;
    docStream << "</generated-xml>";

    return lastProcessedNode;
}

void KTextLoader::loadSection(const KXmlElement &sectionElem, QTextCursor &cursor)
{
    // Add a frame to the current layout
    QTextFrameFormat sectionFormat;
    QString sectionStyleName = sectionElem.attributeNS(KOdfXmlNS::text, "style-name", "");
    if (!sectionStyleName.isEmpty()) {
        KSectionStyle *secStyle = d->textSharedData->sectionStyle(sectionStyleName, d->stylesDotXml);
        if (secStyle)
            secStyle->applyStyle(sectionFormat);
    }
        cursor.insertFrame(sectionFormat);
    // Get the cursor of the frame
    QTextCursor cursorFrame = cursor.currentFrame()->lastCursorPosition();

    loadBody(sectionElem, cursorFrame);

    // Get out of the frame
    cursor.movePosition(QTextCursor::End);
}

void KTextLoader::loadNote(const KXmlElement &noteElem, QTextCursor &cursor)
{
    kDebug(32500) << "Loading a text:note element.";
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
    if (layout) {
        KInlineNote *note = new KInlineNote(KInlineNote::Footnote);
        if (note->loadOdf(noteElem, d->context, d->styleManager, d->changeTracker)) {
            KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
            textObjectManager->insertInlineObject(cursor, note, cursor.charFormat());
        } else {
            kDebug(32500) << "Error while loading the note !";
            delete note;
        }
    }
}

QString KTextLoader::createUniqueBookmarkName(KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker)
{
    QString ret = bookmarkName;
    int uniqID = 0;

    while (true) {
        if (bmm->bookmark(ret)) {
            ret = QString("%1_%2").arg(bookmarkName).arg(++uniqID);
        } else {
            if (isEndMarker) {
                --uniqID;
                if (!uniqID)
                    ret = bookmarkName;
                else
                    ret = QString("%1_%2").arg(bookmarkName).arg(uniqID);
            }
            break;
        }
    }
    return ret;
}

void KTextLoader::loadText(const QString &fulltext, QTextCursor &cursor,
                            bool *stripLeadingSpace, bool isLastNode)
{
    QString text = KTextLoaderP::normalizeWhitespace(fulltext, *stripLeadingSpace);
#ifdef KOOPENDOCUMENTLOADER_DEBUG
    kDebug(32500) << "  <text> text=" << text << text.length();
#endif

    if (!text.isEmpty()) {
        // if present text ends with a space,
        // we can remove the leading space in the next text
        *stripLeadingSpace = text[text.length() - 1].isSpace();

        if (d->changeTracker && d->changeStack.count()) {
            QTextCharFormat format;
            format.setProperty(KCharacterStyle::ChangeTrackerId, d->changeStack.top());
            cursor.mergeCharFormat(format);
        } else {
            QTextCharFormat format = cursor.charFormat();
            if (format.hasProperty(KCharacterStyle::ChangeTrackerId)) {
                format.clearProperty(KCharacterStyle::ChangeTrackerId);
                cursor.setCharFormat(format);
            }
        }
        cursor.insertText(text);

        if (d->loadSpanLevel == 1 && isLastNode
                && cursor.position() > d->loadSpanInitialPos) {
            QTextCursor tempCursor(cursor);
            tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1); // select last char loaded
            if (tempCursor.selectedText() == " " && *stripLeadingSpace) {            // if it's a collapsed blankspace
                tempCursor.removeSelectedText();                                    // remove it
            }
        }
    }
}

void KTextLoader::loadSpan(const KXmlElement &element, QTextCursor &cursor, bool *stripLeadingSpace)
{
    kDebug(32500) << "text-style:" << KTextDebug::textAttributes(cursor.blockCharFormat());
    Q_ASSERT(stripLeadingSpace);
    if (d->loadSpanLevel++ == 0)
        d->loadSpanInitialPos = cursor.position();

    for (KXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KXmlElement ts = node.toElement();
        const QString localName(ts.localName());
        const bool isTextNS = ts.namespaceURI() == KOdfXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KOdfXmlNS::draw;
        const bool isDeltaNS = ts.namespaceURI() == KOdfXmlNS::delta;
//        const bool isOfficeNS = ts.namespaceURI() == KOdfXmlNS::office;
        if (node.isText()) {
            bool isLastNode = node.nextSibling().isNull();
            loadText(node.toText().data(), cursor, stripLeadingSpace,
                     isLastNode);
        } else if (isDeltaNS && localName == "inserted-text-start") {
            d->openChangeRegion(ts);
        } else if (isDeltaNS && localName == "inserted-text-end") {
            d->closeChangeRegion(ts);
        } else if (isDeltaNS && localName == "remove-leaving-content-start") {
            d->openChangeRegion(ts);
        } else if (isDeltaNS && localName == "remove-leaving-content-end") {
            d->closeChangeRegion(ts);
        } else if (isDeltaNS && localName == "removed-content") {
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            QString changeId = ts.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
            int deleteStartPosition = cursor.position();
            bool stripLeadingSpace = true;
            d->openChangeRegion(ts);
            loadSpan(ts,cursor,&stripLeadingSpace);
            d->closeChangeRegion(ts);
            if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
                QTextCursor tempCursor(cursor);
                tempCursor.setPosition(deleteStartPosition);
                KDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
                d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
            }
            cursor.setCharFormat(cf); // restore the cursor char format
        } else if (isDeltaNS && localName == "merge") {
            loadMerge(ts, cursor);
        } else if (isTextNS && localName == "change-start") { // text:change-start
            d->openChangeRegion(ts);
        } else if (isTextNS && localName == "change-end") {
            d->closeChangeRegion(ts);
        } else if (isTextNS && localName == "change") {
            QString id = ts.attributeNS(KOdfXmlNS::text, "change-id");
            int changeId = d->changeTracker->loadedChangeId(id);
            if (changeId) {
                if (d->changeStack.count() && (d->changeStack.top() != changeId))
                    d->changeTracker->setParent(changeId, d->changeStack.top());
                KDeleteChangeMarker *deleteChangemarker = new KDeleteChangeMarker(d->changeTracker);
                deleteChangemarker->setChangeId(changeId);
                KChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
                changeElement->setDeleteChangeMarker(deleteChangemarker);
                changeElement->setEnabled(true);
                KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());

                if (layout) {
                    KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                    textObjectManager->insertInlineObject(cursor, deleteChangemarker);
                }

                loadDeleteChangeWithinPorH(id, cursor);
            }
        } else if (isTextNS && localName == "span") { // text:span
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <span> localName=" << localName;
#endif
            if (!ts.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(ts);
            QString styleName = ts.attributeNS(KOdfXmlNS::text, "style-name", QString());

            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            KCharacterStyle *characterStyle = d->textSharedData->characterStyle(styleName, d->stylesDotXml);
            if (characterStyle) {
                characterStyle->applyStyle(&cursor);
            } else if (!styleName.isEmpty()) {
                kWarning(32500) << "character style " << styleName << " not found";
            }

            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
            if (!ts.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(ts);
        } else if (isTextNS && localName == "s") { // text:s
            int howmany = 1;
            if (!ts.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(ts);
            if (ts.hasAttributeNS(KOdfXmlNS::text, "c")) {
                howmany = ts.attributeNS(KOdfXmlNS::text, "c", QString()).toInt();
            }
            cursor.insertText(QString().fill(32, howmany));
            if (!ts.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(ts);
        } else if ( (isTextNS && localName == "note")) { // text:note
            loadNote(ts, cursor);
        } else if (isTextNS && localName == "tab") { // text:tab
            cursor.insertText("\t");
        } else if (isTextNS && localName == "a") { // text:a
            if (!ts.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(ts);
            QString target = ts.attributeNS(KOdfXmlNS::xlink, "href");
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            if (!target.isEmpty()) {
                QTextCharFormat linkCf(cf);   // and copy it to alter it
                linkCf.setAnchor(true);
                linkCf.setAnchorHref(target);

                // TODO make configurable ? Ho, and it will interfere with saving :/
                QBrush foreground = linkCf.foreground();
                foreground.setColor(Qt::blue);
//                 foreground.setStyle(Qt::Dense1Pattern);
                linkCf.setForeground(foreground);
                linkCf.setProperty(KCharacterStyle::UnderlineStyle, KCharacterStyle::SolidLine);
                linkCf.setProperty(KCharacterStyle::UnderlineType, KCharacterStyle::SingleLine);

                cursor.setCharFormat(linkCf);
            }
            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf);   // restore the cursor char format
            if (!ts.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(ts);
        } else if (isTextNS && localName == "line-break") { // text:line-break
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <line-break> Node localName=" << localName;
#endif
            cursor.insertText(QChar(0x2028));
        }
        else if (isTextNS && localName == "meta") {
            kDebug(30015) << "loading a text:meta";
            KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
            if (layout) {
                const QTextDocument *document = cursor.block().document();
                KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                KTextMeta* startmark = new KTextMeta();
                textObjectManager->insertInlineObject(cursor, startmark);

                // Add inline Rdf here.
                if (ts.hasAttributeNS(KOdfXmlNS::xhtml, "property")
                        || ts.hasAttribute("id")) {
                    KTextInlineRdf* inlineRdf =
                        new KTextInlineRdf((QTextDocument*)document, startmark);
                    inlineRdf->loadOdf(ts);
                    startmark->setInlineRdf(inlineRdf);
                }

                loadSpan(ts, cursor, stripLeadingSpace);   // recurse

                KTextMeta* endmark = new KTextMeta();
                textObjectManager->insertInlineObject(cursor, endmark);
                startmark->setEndBookmark(endmark);
            }
        }
        // text:bookmark, text:bookmark-start and text:bookmark-end
        else if (isTextNS && (localName == "bookmark" || localName == "bookmark-start" || localName == "bookmark-end")) {
            QString bookmarkName = ts.attribute("name");

            KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
            if (layout) {
                const QTextDocument *document = cursor.block().document();
                KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                // For cut and paste, make sure that the name is unique.
                QString uniqBookmarkName = createUniqueBookmarkName(textObjectManager->bookmarkManager(),
                                           bookmarkName,
                                           (localName == "bookmark-end"));
                KoBookmark *bookmark = new KoBookmark(uniqBookmarkName);

                if (localName == "bookmark")
                    bookmark->setType(KoBookmark::SinglePosition);
                else if (localName == "bookmark-start") {
                    bookmark->setType(KoBookmark::StartBookmark);

                    // Add inline Rdf to the bookmark.
                    if (ts.hasAttributeNS(KOdfXmlNS::xhtml, "property")
                            || ts.hasAttribute("id")) {
                        KTextInlineRdf* inlineRdf =
                            new KTextInlineRdf((QTextDocument*)document, bookmark);
                        inlineRdf->loadOdf(ts);
                        bookmark->setInlineRdf(inlineRdf);
                    }
                } else if (localName == "bookmark-end") {
                    bookmark->setType(KoBookmark::EndBookmark);
                    KoBookmark *startBookmark = textObjectManager->bookmarkManager()->bookmark(uniqBookmarkName);
                    if (startBookmark) {        // set end bookmark only if we got start bookmark (we might not have in case of broken document)
                        startBookmark->setEndBookmark(bookmark);
                    } else {
                        kWarning(32500) << "bookmark-end of non-existing bookmark - broken document?";
                    }
                }
                textObjectManager->insertInlineObject(cursor, bookmark, cursor.charFormat());
            }
        } else if (isTextNS && localName == "bookmark-ref") {
            QString bookmarkName = ts.attribute("ref-name");
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            if (!bookmarkName.isEmpty()) {
                QTextCharFormat linkCf(cf); // and copy it to alter it
                linkCf.setAnchor(true);
                QStringList anchorName;
                anchorName << bookmarkName;
                linkCf.setAnchorNames(anchorName);
                cursor.setCharFormat(linkCf);
            }
            bool stripLeadingSpace = true;
            loadSpan(ts, cursor, &stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf);   // restore the cursor char format
        } else if (isTextNS && localName == "number") { // text:number
            /*                ODF Spec, §4.1.1, Formatted Heading Numbering
            If a heading has a numbering applied, the text of the formatted number can be included in a
            <text:number> element. This text can be used by applications that do not support numbering of
            headings, but it will be ignored by applications that support numbering.                   */
        } else if (isDrawNS) {
            loadShape(ts, cursor);
        } else {
            KInlineObject *obj = KInlineObjectRegistry::instance()->createFromOdf(ts, d->context);

            if (obj) {
                KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
                if (layout) {
                    KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                    if (textObjectManager) {
                        KVariableManager *varManager = textObjectManager->variableManager();
                        if (varManager) {
                            textObjectManager->insertInlineObject(cursor, obj, cursor.charFormat());
                        }
                    }
                }
            } else {
#if 0 //1.6:
                bool handled = false;
                // Check if it's a variable
                KVariable *var = context.variableCollection().loadOasisField(textDocument(), ts, context);
                if (var) {
                    textData = "#";     // field placeholder
                    customItem = var;
                    handled = true;
                }
                if (!handled) {
                    handled = textDocument()->loadSpanTag(ts, context, this, pos, textData, customItem);
                    if (!handled) {
                        kWarning(32500) << "Ignoring tag " << ts.tagName();
                        context.styleStack().restore();
                        continue;
                    }
                }
#else
                kDebug(32500) << "Node '" << localName << "' unhandled";
            }
#endif
        }
    }
    --d->loadSpanLevel;
}

void KTextLoader::loadDeleteChangeWithinPorH(QString id, QTextCursor &cursor)
{
    int startPosition = cursor.position();
    int changeId = d->changeTracker->loadedChangeId(id);
    int loadedTags = 0;

    QTextCharFormat charFormat = cursor.block().charFormat();
    QTextBlockFormat blockFormat = cursor.block().blockFormat();

    if (changeId) {
        KChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
        KXmlElement element = d->deleteChangeTable.value(id);
        KXmlElement tag;
        forEachElement(tag, element) {
            QString localName = tag.localName();
            if (localName == "p") {
                if (loadedTags)
                    cursor.insertBlock(blockFormat, charFormat);
                bool stripLeadingSpace = true;
                loadSpan(tag, cursor, &stripLeadingSpace);
                loadedTags++;
            } else if (localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                       || localName == "list" || localName == "numbered-paragraph") {  // OASIS
                cursor.insertBlock(blockFormat, charFormat);
                loadList(tag, cursor);
            } else if (localName == "table") {
                loadTable(tag, cursor);
            }
        }

        int endPosition = cursor.position();

        //Set the char format to the changeId
        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
        cursor.mergeCharFormat(format);

        //Get the QTextDocumentFragment from the selection and store it in the changeElement
        QTextDocumentFragment deletedFragment = KChangeTracker::generateDeleteFragment(cursor, changeElement->deleteChangeMarker());
        changeElement->setDeleteData(deletedFragment);

        //Now Remove this from the document. Will be re-inserted whenever changes have to be seen
        cursor.removeSelectedText();
    }
}

void KTextLoader::loadMerge(const KXmlElement &element, QTextCursor &cursor)
{
    d->openChangeRegion(element);
    QString changeId = element.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
    int deleteStartPosition = cursor.position();

    for (KXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KXmlElement ts = node.toElement();
        const QString localName(ts.localName());
        const bool isDeltaNS = ts.namespaceURI() == KOdfXmlNS::delta;

        if (isDeltaNS && localName == "leading-partial-content") {
            bool stripLeadingSpaces = false;
            loadSpan(ts, cursor, &stripLeadingSpaces);
        } else if (isDeltaNS && localName == "intermediate-content") {
            if (ts.hasChildNodes()) {
                if (ts.firstChild().toElement().localName() != "table") {
                    cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                }
                loadBody(ts, cursor);
            }
        } else if (isDeltaNS && localName == "trailing-partial-content") {
            if (ts.previousSibling().lastChild().toElement().localName() != "table") {
                cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
            }
            loadBody(ts, cursor);
        }
    }

    if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
        QTextCursor tempCursor(cursor);
        tempCursor.setPosition(deleteStartPosition);
        KDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
        d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
    }
    d->closeChangeRegion(element);
}

KDeleteChangeMarker * KTextLoader::Private::insertDeleteChangeMarker(QTextCursor &cursor, const QString &id)
{
    KDeleteChangeMarker *retMarker = NULL;
    int changeId = changeTracker->loadedChangeId(id);
    if (changeId) {
        KDeleteChangeMarker *deleteChangemarker = new KDeleteChangeMarker(changeTracker);
        deleteChangemarker->setChangeId(changeId);
        KChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
        changeElement->setDeleteChangeMarker(deleteChangemarker);
        changeElement->setEnabled(true);
        changeElement->setChangeType(KOdfGenericChange::DeleteChange);
        KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
        if (layout) {
            KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
            textObjectManager->insertInlineObject(cursor, deleteChangemarker);
        }
        retMarker = deleteChangemarker;
    }
    return retMarker;
}

bool KTextLoader::Private::checkForDeleteMerge(QTextCursor &cursor, const QString &id, int startPosition)
{
    bool result = false;

    int changeId = changeTracker->loadedChangeId(id);
    if (changeId) {
        KChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
        //Check if this change is at the beginning of the block and if there is a
        //delete-change at the end of the previous block with the same change-id
        //If both the conditions are true, then merge both these deletions.
        int prevChangeId = 0;
        if ( startPosition == (cursor.block().position())) {
            QTextCursor tempCursor(cursor);
            tempCursor.setPosition(cursor.block().previous().position() + cursor.block().previous().length() - 1);
            prevChangeId = tempCursor.charFormat().property(KCharacterStyle::ChangeTrackerId).toInt();

            if (!prevChangeId) {
                KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
                KInlineObject *inlineObject = layout ? layout->inlineTextObjectManager()->inlineTextObject(tempCursor.charFormat()) : 0;
                KDeleteChangeMarker *deleteChangeMarker = dynamic_cast<KDeleteChangeMarker *>(inlineObject);
                if (deleteChangeMarker) {
                    prevChangeId = deleteChangeMarker->changeId();
                }
            }

        } else {
            QTextCursor tempCursor(cursor);
            tempCursor.setPosition(startPosition - 1);
            prevChangeId = tempCursor.charFormat().property(KCharacterStyle::ChangeTrackerId).toInt();
        }

        if ((prevChangeId) && (prevChangeId == changeId)) {
            QPair<int, int> deleteMarkerRange = deleteChangeMarkerMap.value(changeElement->deleteChangeMarker());
            deleteMarkerRange.second = cursor.position();
            deleteChangeMarkerMap.insert(changeElement->deleteChangeMarker(), deleteMarkerRange);
            result = true;
        }
    }
    return result;
}

void KTextLoader::Private::processDeleteChange(QTextCursor &cursor)
{
    QList<KDeleteChangeMarker *> markersList = deleteChangeMarkerMap.keys();

    KDeleteChangeMarker *marker;
    foreach (marker, markersList) {
        int changeId = marker->changeId();

        KChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
        QPair<int, int> rangeValue = deleteChangeMarkerMap.value(marker);
        int startPosition = rangeValue.first;
        int endPosition = rangeValue.second;

        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);

        //Get the QTextDocumentFragment from the selection and store it in the changeElement
        QTextDocumentFragment deletedFragment = KChangeTracker::generateDeleteFragment(cursor, changeElement->deleteChangeMarker());
        changeElement->setDeleteData(deletedFragment);

        cursor.removeSelectedText();
    }
}

void KTextLoader::loadTable(const KXmlElement &tableElem, QTextCursor &cursor)
{
    //add block before table,
    // **************This Should Be fixed: Just Commenting out for now***************
    // An Empty block before a table would result in a <p></p> before a table
    // After n round-trips we would end-up with n <p></p> before table.
    // ******************************************************************************
    //if (cursor.block().blockNumber() != 0) {
    //    cursor.insertBlock(QTextBlockFormat());
    //}

    QTextTableFormat tableFormat;
    QString tableStyleName = tableElem.attributeNS(KOdfXmlNS::table, "style-name", "");
    if (!tableStyleName.isEmpty()) {
        KTableStyle *tblStyle = d->textSharedData->tableStyle(tableStyleName, d->stylesDotXml);
        if (tblStyle)
            tblStyle->applyStyle(tableFormat);
    }

    // if table has master page style property, copy it to block before table, because this block belongs to table
    // **************This Should Be fixed: Just Commenting out for now***************
    // An Empty block before a table would result in a <p></p> before a table
    // After n round-trips we would end-up with n <p></p> before table.
    // ******************************************************************************
    //QVariant masterStyle = tableFormat.property(KTableStyle::MasterPageName);
    //if (!masterStyle.isNull()) {
    //    QTextBlockFormat textBlockFormat;
    //    textBlockFormat.setProperty(KParagraphStyle::MasterPageName,masterStyle);
    //    cursor.setBlockFormat(textBlockFormat);
    //}

    KTableColumnAndRowStyleManager *tcarManager = new KTableColumnAndRowStyleManager;
    tableFormat.setProperty(KTableStyle::ColumnAndRowStyleManager, QVariant::fromValue(reinterpret_cast<void *>(tcarManager)));
    if (d->changeTracker && d->changeStack.count()) {
        tableFormat.setProperty(KCharacterStyle::ChangeTrackerId, d->changeStack.top());
    }
    QTextTable *tbl = cursor.insertTable(1, 1, tableFormat);
    int rows = 0;
    int columns = 0;
    QList<QRect> spanStore; //temporary list to store spans until the entire table have been created
    KXmlElement tblTag;
    forEachElement(tblTag, tableElem) {
        if (! tblTag.isNull()) {
            const QString tblLocalName = tblTag.localName();
            if (tblTag.namespaceURI() == KOdfXmlNS::table) {
                if (tblLocalName == "table-column") {
                    loadTableColumn(tblTag, tbl, columns);
                } else if (tblLocalName == "table-row") {
                    if (!tblTag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                        d->openChangeRegion(tblTag);

                    loadTableRow(tblTag, tbl, spanStore, cursor, rows);

                    if (!tblTag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                        d->closeChangeRegion(tblTag);
                }
            } else if(tblTag.namespaceURI() == KOdfXmlNS::delta) {
                if (tblLocalName == "removed-content")
                    d->openChangeRegion(tblTag);

                KXmlElement deltaTblTag;
                forEachElement (deltaTblTag, tblTag) {
                    if (!deltaTblTag.isNull() && (deltaTblTag.namespaceURI() == KOdfXmlNS::table)) {
                        const QString deltaTblLocalName = deltaTblTag.localName();
                        if (deltaTblLocalName == "table-column") {
                            loadTableColumn(deltaTblTag, tbl, columns);
                        } else if (deltaTblLocalName == "table-row") {
                            loadTableRow(deltaTblTag, tbl, spanStore, cursor, rows);
                        }
                    }
                }

                if (tblLocalName == "removed-content")
                    d->closeChangeRegion(tblTag);
            }
        }
    }
    // Finally create spans
    foreach(const QRect &span, spanStore) {
        tbl->mergeCells(span.y(), span.x(), span.height(), span.width()); // for some reason Qt takes row, column
    }
    cursor = tbl->lastCursorPosition();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
}

void KTextLoader::loadTableColumn(KXmlElement &tblTag, QTextTable *tbl, int &columns)
{
    KTableColumnAndRowStyleManager *tcarManager = reinterpret_cast<KTableColumnAndRowStyleManager *>
                                                   (tbl->format().property(KTableStyle::ColumnAndRowStyleManager).value<void *>());
    int rows = tbl->rows();
    int repeatColumn = tblTag.attributeNS(KOdfXmlNS::table, "number-columns-repeated", "1").toInt();
    QString columnStyleName = tblTag.attributeNS(KOdfXmlNS::table, "style-name", "");
    if (!columnStyleName.isEmpty()) {
        KTableColumnStyle *columnStyle = d->textSharedData->tableColumnStyle(columnStyleName, d->stylesDotXml);
        if (columnStyle) {
            for (int c = columns; c < columns + repeatColumn; c++) {
                tcarManager->setColumnStyle(c, *columnStyle);
            }
        }
    }

    QString defaultCellStyleName = tblTag.attributeNS(KOdfXmlNS::table, "default-cell-style-name", "");
    if (!defaultCellStyleName.isEmpty()) {
        KTableCellStyle *cellStyle = d->textSharedData->tableCellStyle(defaultCellStyleName, d->stylesDotXml);
        for (int c = columns; c < columns + repeatColumn; c++) {
            tcarManager->setDefaultColumnCellStyle(c, cellStyle);
        }
    }

    columns = columns + repeatColumn;
    tbl->resize(qMax(rows, 1), columns);
}

void KTextLoader::loadTableRow(KXmlElement &tblTag, QTextTable *tbl, QList<QRect> &spanStore, QTextCursor &cursor, int &rows)
{
    KTableColumnAndRowStyleManager *tcarManager = reinterpret_cast<KTableColumnAndRowStyleManager *>
                                                   (tbl->format().property(KTableStyle::ColumnAndRowStyleManager).value<void *>());

    int columns = tbl->columns();
    QString rowStyleName = tblTag.attributeNS(KOdfXmlNS::table, "style-name", "");
    if (!rowStyleName.isEmpty()) {
        KTableRowStyle *rowStyle = d->textSharedData->tableRowStyle(rowStyleName, d->stylesDotXml);
        if (rowStyle) {
            tcarManager->setRowStyle(rows, *rowStyle);
        }
    }

    QString defaultCellStyleName = tblTag.attributeNS(KOdfXmlNS::table, "default-cell-style-name", "");
    if (!defaultCellStyleName.isEmpty()) {
        KTableCellStyle *cellStyle = d->textSharedData->tableCellStyle(defaultCellStyleName, d->stylesDotXml);
        tcarManager->setDefaultRowCellStyle(rows, cellStyle);
    }

    rows++;
    tbl->resize(rows, qMax(columns,1));

    // Added a row
    int currentCell = 0;
    KXmlElement rowTag;
    forEachElement(rowTag, tblTag) {
        if (!rowTag.isNull()) {
            const QString rowLocalName = rowTag.localName();
            if (rowTag.namespaceURI() == KOdfXmlNS::table) {
                if (rowLocalName == "table-cell") {
                    loadTableCell(rowTag, tbl, spanStore, cursor, currentCell);
                    currentCell++;
                } else if (rowLocalName == "covered-table-cell") {
                    currentCell++;
                }
            } else if (rowTag.namespaceURI() == KOdfXmlNS::delta) {
                if (rowLocalName == "removed-content")
                    d->openChangeRegion(rowTag);

                KXmlElement deltaRowTag;
                forEachElement (deltaRowTag, rowTag) {
                    if (!deltaRowTag.isNull() && (deltaRowTag.namespaceURI() == KOdfXmlNS::table)) {
                        const QString deltaRowLocalName = deltaRowTag.localName();
                        if (deltaRowLocalName == "table-cell") {
                            loadTableCell (deltaRowTag, tbl, spanStore, cursor, currentCell);
                            currentCell++;
                        } else if (deltaRowLocalName == "covered-table-cell") {
                            currentCell++;
                        }
                    }
                }

                if (rowLocalName == "removed-content")
                    d->closeChangeRegion(rowTag);
            }
        }
    }
}

void KTextLoader::loadTableCell(KXmlElement &rowTag, QTextTable *tbl, QList<QRect> &spanStore, QTextCursor &cursor, int &currentCell)
{
    KTableColumnAndRowStyleManager *tcarManager = reinterpret_cast<KTableColumnAndRowStyleManager *>
                                                   (tbl->format().property(KTableStyle::ColumnAndRowStyleManager).value<void *>());
    const int currentRow = tbl->rows() - 1;
    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);

    if (!rowTag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
        d->openChangeRegion(rowTag);

    // store spans until entire table have been loaded
    int rowsSpanned = rowTag.attributeNS(KOdfXmlNS::table, "number-rows-spanned", "1").toInt();
    int columnsSpanned = rowTag.attributeNS(KOdfXmlNS::table, "number-columns-spanned", "1").toInt();
    spanStore.append(QRect(currentCell, currentRow, columnsSpanned, rowsSpanned));

    if (cell.isValid()) {
        QString cellStyleName = rowTag.attributeNS(KOdfXmlNS::table, "style-name", "");
        KTableCellStyle *cellStyle = 0;
        if (!cellStyleName.isEmpty()) {
            cellStyle = d->textSharedData->tableCellStyle(cellStyleName, d->stylesDotXml);
        } else if (tcarManager->defaultRowCellStyle(currentRow)) {
            cellStyle = tcarManager->defaultRowCellStyle(currentRow);
        } else if (tcarManager->defaultColumnCellStyle(currentCell)) {
            cellStyle = tcarManager->defaultColumnCellStyle(currentCell);
        }

        QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
        if (cellStyle)
            cellStyle->applyStyle(cellFormat);

        cell.setFormat(cellFormat);

        // handle inline Rdf
        // rowTag is the current table cell.
        if (rowTag.hasAttributeNS(KOdfXmlNS::xhtml, "property") || rowTag.hasAttribute("id")) {
            KTextInlineRdf* inlineRdf = new KTextInlineRdf((QTextDocument*)cursor.block().document(),cell);
            inlineRdf->loadOdf(rowTag);
            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
            cellFormat.setProperty(KTableCellStyle::InlineRdf,QVariant::fromValue(inlineRdf));
            cell.setFormat(cellFormat);
        }

        cursor = cell.firstCursorPosition();
        loadBody(rowTag, cursor);

        if (d->changeTracker && d->changeStack.count()) {
            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
            cellFormat.setProperty(KCharacterStyle::ChangeTrackerId, d->changeStack.top());
            cell.setFormat(cellFormat);
        }
    }

    if (!rowTag.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
        d->closeChangeRegion(rowTag);

}

void KTextLoader::loadShape(const KXmlElement &element, QTextCursor &cursor)
{
    KShape *shape = KShapeRegistry::instance()->createShapeFromOdf(element, d->context);
    if (!shape) {
        kDebug(32500) << "shape '" << element.localName() << "' unhandled";
        return;
    }

    QString anchorType;
    if (shape->hasAdditionalAttribute("text:anchor-type"))
        anchorType = shape->additionalAttribute("text:anchor-type");
    else if (element.hasAttributeNS(KOdfXmlNS::text, "anchor-type"))
        anchorType = element.attributeNS(KOdfXmlNS::text, "anchor-type");
    else
        anchorType = "as-char"; // default value

    // page anchored shapes are handled differently
    if (anchorType != "page") {
        KTextAnchor *anchor = new KTextAnchor(shape);
        anchor->loadOdf(element, d->context);
        d->textSharedData->shapeInserted(shape, element, d->context);

        KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
        if (layout) {
            if (!element.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(element);

            if (d->changeTracker && d->changeStack.count()) {
                QTextCharFormat format;
                format.setProperty(KCharacterStyle::ChangeTrackerId, d->changeStack.top());
                cursor.mergeCharFormat(format);
            } else {
                QTextCharFormat format = cursor.charFormat();
                if (format.hasProperty(KCharacterStyle::ChangeTrackerId)) {
                    format.clearProperty(KCharacterStyle::ChangeTrackerId);
                    cursor.setCharFormat(format);
                }
            }

            KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
            if (textObjectManager) {
                textObjectManager->insertInlineObject(cursor, anchor, cursor.charFormat());
            }

            if(!element.attributeNS(KOdfXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(element);
        }
    } else {
        d->textSharedData->shapeInserted(shape, element, d->context);
    }
}

void KTextLoader::loadTableOfContents(const KXmlElement &element, QTextCursor &cursor)
{
    // Add a frame to the current layout
    QTextFrameFormat tocFormat;
    tocFormat.setProperty(KoText::TableOfContents, true);
    cursor.insertFrame(tocFormat);
    // Get the cursor of the frame
    QTextCursor cursorFrame = cursor.currentFrame()->lastCursorPosition();

    // We'll just try to find displayable elements and add them as paragraphs
    KXmlElement e;
    forEachElement(e, element) {
        if (e.isNull() || e.namespaceURI() != KOdfXmlNS::text)
            continue;

        //TODO look at table-of-content-source

        // We look at the index body now
        if (e.localName() == "index-body") {
            KXmlElement p;
            bool firstTime = true;
            forEachElement(p, e) {
                // All elem will be "p" instead of the title, which is particular
                if (p.isNull() || p.namespaceURI() != KOdfXmlNS::text)
                    continue;

                if (!firstTime) {
                    // use empty formats to not inherit from the prev parag
                    QTextBlockFormat bf;
                    QTextCharFormat cf;
                    cursorFrame.insertBlock(bf, cf);
                }
                firstTime = false;

                QTextBlock current = cursorFrame.block();
                QTextBlockFormat blockFormat;

                if (p.localName() == "p") {
                    loadParagraph(p, cursorFrame);
                } else if (p.localName() == "index-title") {
                    loadBody(p, cursorFrame);
                }

                QTextCursor c(current);
                c.mergeBlockFormat(blockFormat);
            }
        }
    }
    // Get out of the frame
    cursor.movePosition(QTextCursor::End);
}

void KTextLoader::loadVariableDeclarations(const KXmlElement& element, QTextCursor& cursor) {
    KXmlElement e;
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(cursor.block().document()->documentLayout());
    if (layout) {
        KInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
	if (textObjectManager) {
            KVariableManager *variableManager = textObjectManager->variableManager();
	    if (variableManager) {
                forEachElement(e, element) {
		    QString name = e.attributeNS(KOdfXmlNS::text, "name");
		    QString value = e.attributeNS(KOdfXmlNS::office, "string-value");
		    variableManager->setValue(name, value);
		}
	    }
	}
    }
}

void KTextLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KTextLoader::processBody()
{
    d->bodyProgressValue++;
    if (d->dt.elapsed() >= d->nextProgressReportMs) {  // update based on elapsed time, don't saturate the queue
        d->nextProgressReportMs = d->dt.elapsed() + 333; // report 3 times per second
        Q_ASSERT(d->bodyProgressTotal > 0);
        const int percent = d->bodyProgressValue * 100 / d->bodyProgressTotal;
        emit sigProgress(percent);
    }
}

void KTextLoader::endBody()
{
}

void KTextLoader::storeDeleteChanges(KXmlElement &element)
{
    KXmlElement tag;
    forEachElement(tag, element) {
        if (! tag.isNull()) {
            const QString localName = tag.localName();
            if (localName == "changed-region") {
                KXmlElement region;
                forEachElement(region, tag) {
                    if (!region.isNull()) {
                        if (region.localName() == "deletion") {
                            QString id = tag.attributeNS(KOdfXmlNS::text, "id");
                            d->deleteChangeTable.insert(id, region);
                        }
                    }
                }
            }
        }
    }
}

void KTextLoader::markBlocksAsInserted(QTextCursor& cursor,int from, const QString& id)
{
    int to = cursor.position();
    QTextCursor editCursor(cursor);
    QTextDocument *document = cursor.document();

    QTextBlock startBlock = document->findBlock(from);
    QTextBlock endBlock = document->findBlock(to);

    int changeId = d->changeTracker->loadedChangeId(id);

    QTextBlockFormat format;
    format.setProperty(KCharacterStyle::ChangeTrackerId, changeId);

    do {
        startBlock = startBlock.next();
        editCursor.setPosition(startBlock.position());
        editCursor.mergeBlockFormat(format);
    } while(startBlock != endBlock);
}

#include <KTextLoader.moc>
