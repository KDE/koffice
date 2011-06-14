/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
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

#include "KoTextWriter.h"

#include <QMap>
#include <QTextDocument>
#include <QTextTable>
#include <QStack>
#include <QTextTableCellFormat>
#include <QBuffer>
#include <QUuid>
#include <QTextDocumentFragment>
#include <QXmlStreamReader>

#include "KInlineObject.h"
#include "KoTextAnchor.h"
#include "KoShape.h"
#include "KoVariable.h"
#include "KInlineTextObjectManager.h"
#include "styles/KoStyleManager.h"
#include "styles/KCharacterStyle.h"
#include "styles/KParagraphStyle.h"
#include "styles/KListStyle.h"
#include "styles/KListLevelProperties.h"
#include "styles/KoTableCellStyle.h"
#include "KoTextDocumentLayout.h"
#include "KoTextBlockData.h"
#include "KoTextDocument.h"
#include "KoTextInlineRdf.h"
#include "../KDocumentRdfBase.h"

#include "KoTextMeta.h"
#include "KoBookmark.h"

#include <KoShapeSavingContext.h>
#include <KXmlWriter.h>
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KOdfXmlNS.h>

#include <opendocument/KoTextSharedSavingData.h>
#include <changetracker/KChangeTracker.h>
#include <changetracker/KChangeTrackerElement.h>
#include <changetracker/KDeleteChangeMarker.h>
#include <KFormatChangeInformation_p.h>
#include <KOdfGenericChange.h>
#include <KOdfGenericChanges.h>

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#endif

class KoTextWriter::TagInformation
{
    public:
        TagInformation() :m_tagName(0), attributeList()
        {
        }

        void setTagName(const char *tagName)
        {
            m_tagName = tagName;
        }

        void addAttribute(const QString& attributeName, const QString& attributeValue)
        {
            attributeList.push_back(QPair<QString,QString>(attributeName, attributeValue));
        }

        void addAttribute(const QString& attributeName, int value)
        {
            addAttribute(attributeName, QString::number(value));
        }

        void clear()
        {
            m_tagName = 0;
            attributeList.clear();
        }

        const char *name() const
        {
            return m_tagName;
        }

        const QVector<QPair<QString, QString> >& attributes() const
        {
            return attributeList;
        }

        void write(KXmlWriter &writer)
        {
            if (m_tagName == 0)
                return;
            writer.startElement(m_tagName, false);
            typedef QPair<QString, QString> Attribute;
            foreach(const Attribute &attribute, attributeList) {
                writer.addAttribute(attribute.first.toAscii(), attribute.second.toAscii());
            }
        }

    private:
        const char *m_tagName;
        QVector<QPair<QString, QString> > attributeList;
};

class KoTextWriter::Private
{
public:
    explicit Private(KoShapeSavingContext &context)
    : context(context),
    sharedData(0),
    writer(0),
    layout(0),
    styleManager(0),
    changeTracker(0),
    rdfData(0),
    splitEndBlockNumber(-1),
    splitRegionOpened(false),
    splitIdCounter(1),
    deleteMergeRegionOpened(false),
    deleteMergeEndBlockNumber(-1)
    {
        writer = &context.xmlWriter();
        changeStack.push(0);
    }

    ~Private() {}

    enum ElementType {
        Span,
        ParagraphOrHeader,
        ListItem,
        List,
        NumberedParagraph,
        Table,
        TableRow,
        TableColumn,
        TableCell
    };

    void saveChange(QTextCharFormat format);
    void saveChange(int changeId);
    void saveODF12Change(QTextCharFormat format);
    QString generateDeleteChangeXml(KDeleteChangeMarker *marker);
    int openTagRegion(int position, ElementType elementType, KoTextWriter::TagInformation& tagInformation);
    void closeTagRegion(int changeId);
    QStack<const char *> openedTagStack;

    QString saveParagraphStyle(const QTextBlock &block);
    QString saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat);
    QString saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat);
    QHash<QTextList *, QString> saveListStyles(QTextBlock block, int to);
    void saveParagraph(const QTextBlock &block, int from, int to);
    void saveTable(QTextTable *table, QHash<QTextList *, QString> &listStyles);
    QTextBlock& saveList(QTextBlock &block, QHash<QTextList *, QString> &listStyles, int level);
    void saveTableOfContents(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable, QTextFrame *toc);
    void writeBlocks(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable = 0, QTextFrame *currentFrame = 0, QTextList *currentList = 0);
    int checkForBlockChange(const QTextBlock &block);
    int checkForListItemChange(const QTextBlock &block);
    int checkForListChange(const QTextBlock &block);
    int checkForTableRowChange(int position);
    int checkForTableColumnChange(int position);
    KoShapeSavingContext &context;
    KoTextSharedSavingData *sharedData;
    KXmlWriter *writer;

    KoTextDocumentLayout *layout;
    KoStyleManager *styleManager;
    KChangeTracker *changeTracker;
    KDocumentRdfBase *rdfData;
    QTextDocument *document;

    QStack<int> changeStack;
    QMap<int, QString> changeTransTable;
    QList<int> savedDeleteChanges;

    // Things like bookmarks need to be properly turn down
    // during a cut and paste operation when their end marker
    // is not included in the selection.
    QList<KInlineObject*> pairedInlineObjectStack;

    // For saving of paragraph or header splits
    int checkForSplit(const QTextBlock &block);
    int splitEndBlockNumber;
    bool splitRegionOpened;
    bool splitIdCounter;

    //For saving of delete-changes that result in a merge between two elements
    bool deleteMergeRegionOpened;
    int deleteMergeEndBlockNumber;
    int checkForDeleteMerge(const QTextBlock &block);
    void openSplitMergeRegion();
    void closeSplitMergeRegion();

    //For List Item Splits
    void postProcessListItemSplit(int changeId);

    //Method used by both split and merge
    int checkForMergeOrSplit(const QTextBlock &block, KOdfGenericChange::Type changeType);
    void addNameSpaceDefinitions(QString &generatedXmlString);

    KXmlWriter *oldXmlWriter;
    KXmlWriter *newXmlWriter;
    QByteArray generatedXmlArray;
    QBuffer generatedXmlBuffer;

    void postProcessDeleteMergeXml();
    void generateFinalXml(QTextStream &outputXmlStream, const KXmlElement &element);

    // For Handling <p> with <p> or <h> with <h> merges
    void handleParagraphOrHeaderMerge(QTextStream &outputXmlStream, const KXmlElement &element);

    // For Handling <p> with <h> or <h> with <p> merges
    void handleParagraphWithHeaderMerge(QTextStream &outputXmlStream, const KXmlElement &element);

    // For handling <p> with <list-item> merges
    void handleParagraphWithListItemMerge(QTextStream &outputXmlStream, const KXmlElement &element);
    void generateListForPWithListMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                       QString &mergeResultElement, QString &changeId, int &endIdCounter, bool removeLeavingContent);
    void generateListItemForPWithListMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                       QString &mergeResultElement, QString &changeId, int &endIdCounter, bool removeLeavingContent);

    // FOr Handling <list-item> with <p> merges
    int deleteStartDepth;
    void handleListItemWithParagraphMerge(QTextStream &outputXmlStream, const KXmlElement &element);
    void generateListForListWithPMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool removeLeavingContent);
    void generateListItemForListWithPMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool removeLeavingContent);
    bool checkForDeleteStartInListItem(KXmlElement &element, bool checkRecursively = true);

    void handleListWithListMerge(QTextStream &outputXmlStream, const KXmlElement &element);

    // For handling <list-item> with <list-item> merges
    void handleListItemWithListItemMerge(QTextStream &outputXmlStream, const KXmlElement &element);
    QString findChangeIdForListItemMerge(const KXmlElement &element);
    void generateListForListItemMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool listMergeStart, bool listMergeEnd);
    void generateListItemForListItemMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool listItemMergeStart, bool listItemMergeEnd);
    bool checkForDeleteEndInListItem(KXmlElement &element, bool checkRecursively = true);

    // Common methods
    void writeAttributes(QTextStream &outputXmlStream, KXmlElement &element);
    void writeNode(QTextStream &outputXmlStream, KXmlNode &node, bool writeOnlyChildren = false);
    void removeLeavingContentStart(QTextStream &outputXmlStream, KXmlElement &element, QString &changeId, int endIdCounter);
    void removeLeavingContentEnd(QTextStream &outputXmlStream, int endIdCounter);
    void insertAroundContent(QTextStream &outputXmlStream, KXmlElement &element, QString &changeId);
};

void KoTextWriter::Private::saveChange(QTextCharFormat format)
{
    if (!changeTracker /*&& changeTracker->isEnabled()*/)
        return;//The change tracker exist and we are allowed to save tracked changes

    int changeId = format.property(KCharacterStyle::ChangeTrackerId).toInt();
    if (changeId) { //There is a tracked change
        saveChange(changeId);
    }
}

void KoTextWriter::Private::saveChange(int changeId)
{
    Q_ASSERT(changeTracker);
    if (changeTransTable.value(changeId).length())
        return;

    if ((changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) &&
         (changeTracker->saveFormat() == KChangeTracker::ODF_1_2)) {
        return;
    }

    KOdfGenericChange change;
    if (changeTracker->saveFormat() == KChangeTracker::ODF_1_2) {
        change.setChangeFormat(KOdfGenericChange::ODF_1_2);
    } else {
        change.setChangeFormat(KOdfGenericChange::DELTAXML);
    }

    changeTracker->saveInlineChange(changeId, change);
    QString changeName = sharedData->genChanges().insert(change);
    changeTransTable.insert(changeId, changeName);
}

void KoTextWriter::Private::saveODF12Change(QTextCharFormat format)
{
    if (!changeTracker /*&& changeTracker->isEnabled()*/)
        return;//The change tracker exist and we are allowed to save tracked changes

    int changeId = format.property(KCharacterStyle::ChangeTrackerId).toInt();

    //First we need to check if the eventual already opened change regions are still valid
    while (int change = changeStack.top()) {
        if (!changeId || !changeTracker->isParent(change, changeId)) {
            writer->startElement("text:change-end", false);
            writer->addAttribute("text:change-id", changeTransTable.value(change));
            writer->endElement();
            changeStack.pop();
        }
    }

    if (changeId) { //There is a tracked change
        if (changeTracker->elementById(changeId)->changeType() != KOdfGenericChange::DeleteChange) {
            //Now start a new change region if not already done
            if (!changeStack.contains(changeId)) {
                QString changeName = changeTransTable.value(changeId);
                writer->startElement("text:change-start", false);
                writer->addAttribute("text:change-id",changeName);
                writer->endElement();
                changeStack.push(changeId);
            }
        }
    }

    KDeleteChangeMarker *changeMarker;
    if (layout && (changeMarker = dynamic_cast<KDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(format)))) {
        if (!savedDeleteChanges.contains(changeMarker->changeId())) {
            QString deleteChangeXml = generateDeleteChangeXml(changeMarker);
            changeMarker->setDeleteChangeXml(deleteChangeXml);
            changeMarker->saveOdf(context);
            savedDeleteChanges.append(changeMarker->changeId());
        }
    }
}

QString KoTextWriter::Private::generateDeleteChangeXml(KDeleteChangeMarker *marker)
{
    //Create a QTextDocument from the Delete Fragment
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertFragment(changeTracker->elementById(marker->changeId())->deleteData());

    //Save the current writer
    KXmlWriter &oldWriter = context.xmlWriter();

    //Create a new KXmlWriter pointing to a QBuffer
    QByteArray xmlArray;
    QBuffer xmlBuffer(&xmlArray);
    KXmlWriter newXmlWriter(&xmlBuffer);

    //Set our xmlWriter as the writer to be used
    writer = &newXmlWriter;
    context.setXmlWriter(newXmlWriter);

    //Call writeBlocks to generate the xml
    QHash<QTextList *,QString> listStyles = saveListStyles(doc.firstBlock(), doc.characterCount());
    writeBlocks(&doc, 0, doc.characterCount(),listStyles);

    //Restore the actual xml writer
    writer = &oldWriter;
    context.setXmlWriter(oldWriter);

    QString generatedXmlString(xmlArray);
    return generatedXmlString;
}

int KoTextWriter::Private::openTagRegion(int position, ElementType elementType, KoTextWriter::TagInformation& tagInformation)
{
    if (changeTracker == 0) {
        openedTagStack.push(tagInformation.name());
        tagInformation.write(*writer);
        return -1;
    }
    int changeId = 0, returnChangeId = 0;
    QTextCursor cursor(document);
    QTextBlock block = document->findBlock(position);

    openedTagStack.push(tagInformation.name());

    KChangeTracker::ChangeSaveFormat changeSaveFormat = changeTracker->saveFormat();

    if (changeSaveFormat == KChangeTracker::DELTAXML) {
        switch (elementType) {
            case KoTextWriter::Private::Span:
                cursor.setPosition(position + 1);
                changeId = cursor.charFormat().property(KCharacterStyle::ChangeTrackerId).toInt();
                break;
            case KoTextWriter::Private::ParagraphOrHeader:
                changeId = checkForBlockChange(block);
                break;
            case KoTextWriter::Private::NumberedParagraph:
                changeId = checkForBlockChange(block);
                break;
            case KoTextWriter::Private::ListItem:
                changeId = checkForListItemChange(block);
                break;
            case KoTextWriter::Private::List:
                changeId = checkForListChange(block);
                break;
            case KoTextWriter::Private::TableRow:
                changeId = checkForTableRowChange(position);
                break;
            case KoTextWriter::Private::TableColumn:
                changeId = checkForTableColumnChange(position);
                break;
            case KoTextWriter::Private::TableCell:
                cursor.setPosition(position);
                changeId = cursor.currentTable()->cellAt(position).format().property(KCharacterStyle::ChangeTrackerId).toInt();
                break;
            case KoTextWriter::Private::Table:
                cursor.setPosition(position);
                QTextTableFormat tableFormat = cursor.currentTable()->format();
                changeId = tableFormat.property(KCharacterStyle::ChangeTrackerId).toInt();
                break;
        }
    }

    if (!changeId || (changeStack.top() == changeId)) {
        changeId = 0;
    } else if ((changeTracker->isDuplicateChangeId(changeId)) && (changeTracker->originalChangeId(changeId) == changeStack.top())) {
        QVectorIterator<int> changeStackIterator(changeStack);
        changeStackIterator.toBack();

        while ((changeStackIterator.peekPrevious()) && (changeStackIterator.peekPrevious() == changeTracker->originalChangeId(changeId))) {
            changeStackIterator.previous();
            changeId = changeTracker->parent(changeId);
        }
    } else if ((changeTracker->isDuplicateChangeId(changeId)) && (changeTracker->isParent(changeStack.top(), changeId))) {
        changeId = 0;
    }

    returnChangeId = changeId;

    //Navigate through the change history and push into a stack so that they can be processed in the reverse order (i.e starting from earliest)
    QStack<int> changeHistory;
    while (changeId && (changeId != changeStack.top())) {
        changeHistory.push(changeId);
        changeId = changeTracker->parent(changeId);
    }

    if (returnChangeId) {
        changeStack.push(returnChangeId);
    }

    while(changeHistory.size()) {
        int changeId = changeHistory.pop();
        if (changeTracker->isDuplicateChangeId(changeId)) {
            changeId = changeTracker->originalChangeId(changeId);
        }

        if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
            writer->startElement("delta:removed-content", false);
            writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
        }else if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::InsertChange) {
            tagInformation.addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            tagInformation.addAttribute("delta:insertion-type", "insert-with-content");
        } else if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::FormatChange && elementType == KoTextWriter::Private::Span) {
            KFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);

            if (formatChangeInformation && formatChangeInformation->formatType() == KFormatChangeInformation::eTextStyleChange) {
                writer->startElement("delta:remove-leaving-content-start", false);
                writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
                writer->addAttribute("delta:end-element-idref", QString("end%1").arg(changeId));

                cursor.setPosition(position);
                KoTextStyleChangeInformation *textStyleChangeInformation = static_cast<KoTextStyleChangeInformation *>(formatChangeInformation);
                QString styleName = saveCharacterStyle(textStyleChangeInformation->previousCharFormat(), cursor.blockCharFormat());
                if (!styleName.isEmpty()) {
                   writer->startElement("text:span", false);
                   writer->addAttribute("text:style-name", styleName);
                }
                writer->endElement();
                writer->endElement();

            }

            tagInformation.addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            tagInformation.addAttribute("delta:insertion-type", "insert-around-content");
        } else if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::FormatChange
                            && elementType == KoTextWriter::Private::ParagraphOrHeader) {
            KFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);
            if (formatChangeInformation && formatChangeInformation->formatType() == KFormatChangeInformation::eParagraphStyleChange) {
                KParagraphStyleChangeInformation *paraStyleChangeInformation = static_cast<KParagraphStyleChangeInformation *>(formatChangeInformation);
                QString styleName = saveParagraphStyle(paraStyleChangeInformation->previousBlockFormat(), QTextCharFormat());
                QString attributeChangeRecord = changeTransTable.value(changeId) + QString(",") + QString("modify")
                                                                                 + QString(",") + QString("text:style-name")
                                                                                 + QString(",") + styleName;
                tagInformation.addAttribute("ac:change001", attributeChangeRecord);
            }
        } else if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::FormatChange
                            && elementType == KoTextWriter::Private::ListItem) {
            KFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);
            if (formatChangeInformation && formatChangeInformation->formatType() == KFormatChangeInformation::eListItemNumberingChange) {
                KListItemNumChangeInformation *listItemChangeInfo = static_cast<KListItemNumChangeInformation *>(formatChangeInformation);

                if (listItemChangeInfo->listItemNumChangeType() == KListItemNumChangeInformation::eNumberingRestarted) {
                    QString attributeChangeRecord = changeTransTable.value(changeId) + QString(",") + QString("insert")
                                                                                     + QString(",") + QString("text:start-value");
                    tagInformation.addAttribute("ac:change001", attributeChangeRecord);
                } else if (listItemChangeInfo->listItemNumChangeType() == KListItemNumChangeInformation::eRestartRemoved) {
                    QString attributeChangeRecord = changeTransTable.value(changeId) + QString(",") + QString("remove")
                                                                                     + QString(",") + QString("text:start-value")
                                                                                     + QString(",") + QString::number(listItemChangeInfo->previousStartNumber());
                    tagInformation.addAttribute("ac:change001", attributeChangeRecord);
                }
            }
        }
    }

    tagInformation.write(*writer);

    return returnChangeId;
}

void KoTextWriter::Private::closeTagRegion(int changeId)
{
    const char *tagName = openedTagStack.pop();
    if (tagName) {
        writer->endElement(); // close the tag
    }
    if (changeTracker == 0)
        return;

    if (changeId)
        changeStack.pop();

    if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
        writer->endElement(); //delta:removed-content
    } else if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::FormatChange) {
        KFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);
        if (formatChangeInformation && formatChangeInformation->formatType() == KFormatChangeInformation::eTextStyleChange) {
            writer->startElement("delta:remove-leaving-content-end", false);
            writer->addAttribute("delta:end-element-id", QString("end%1").arg(changeId));
            writer->endElement();
        }
    }
    return;
}

KoTextWriter::KoTextWriter(KoShapeSavingContext &context, KDocumentRdfBase *rdfData)
    : d(new Private(context))
{
    d->rdfData = rdfData;
    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    if (sharedData) {
        d->sharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }

    if (!d->sharedData) {
        d->sharedData = new KoTextSharedSavingData();
        KOdfGenericChanges *changes = new KOdfGenericChanges();
        d->sharedData->setGenChanges(*changes);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_SAVING_ID, d->sharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }
}

KoTextWriter::~KoTextWriter()
{
    delete d;
}

QString KoTextWriter::saveParagraphStyle(const QTextBlock &block, KoStyleManager *styleManager, KoShapeSavingContext &context)
{
    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat();
    return saveParagraphStyle(blockFormat, charFormat, styleManager, context);
}

QString KoTextWriter::saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat, KoStyleManager * styleManager, KoShapeSavingContext &context)
{
    KParagraphStyle *defaultParagraphStyle = styleManager->defaultParagraphStyle();
    KParagraphStyle *originalParagraphStyle = styleManager->paragraphStyle(blockFormat.intProperty(KParagraphStyle::StyleId));
    if (!originalParagraphStyle)
        originalParagraphStyle = defaultParagraphStyle;

    QString generatedName;
    QString displayName = originalParagraphStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    // we'll convert the blockFormat to a KParagraphStyle to check for local changes.
    KParagraphStyle paragStyle(blockFormat, charFormat);
    if (paragStyle == (*originalParagraphStyle)) { // This is the real, unmodified character style.
        // TODO zachmann: this could use the name of the saved style without saving it again
        // therefore we would need to store that information in the saving context
        if (originalParagraphStyle != defaultParagraphStyle) {
            KOdfGenericStyle style(KOdfGenericStyle::ParagraphStyle, "paragraph");
            originalParagraphStyle->saveOdf(style, context.mainStyles());
            generatedName = context.mainStyles().insert(style, internalName, KOdfGenericStyles::DontAddNumberToName);
        }
    } else { // There are manual changes... We'll have to store them then
        KOdfGenericStyle style(KOdfGenericStyle::ParagraphAutoStyle, "paragraph", internalName);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        if (originalParagraphStyle)
            paragStyle.removeDuplicates(*originalParagraphStyle);
        paragStyle.saveOdf(style, context.mainStyles());
        generatedName = context.mainStyles().insert(style, "P");
    }
    return generatedName;
}

QString KoTextWriter::Private::saveParagraphStyle(const QTextBlock &block)
{
    return KoTextWriter::saveParagraphStyle(block, styleManager, context);
}

QString KoTextWriter::Private::saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat)
{
    return KoTextWriter::saveParagraphStyle(blockFormat, charFormat, styleManager, context);
}

QString KoTextWriter::Private::saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    KCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();

    KCharacterStyle *originalCharStyle = styleManager->characterStyle(charFormat.intProperty(KCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    KCharacterStyle charStyle(charFormat);
    // we'll convert it to a KCharacterStyle to check for local changes.
    // we remove that properties given by the paragraphstyle char format, these are not present in the saved style (should it really be the case?)
    charStyle.removeDuplicates(blockCharFormat);
    if (charStyle == (*originalCharStyle)) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            if (!charStyle.isEmpty()) {
                KOdfGenericStyle style(KOdfGenericStyle::ParagraphStyle, "text");
                originalCharStyle->saveOdf(style);
                generatedName = context.mainStyles().insert(style, internalName, KOdfGenericStyles::DontAddNumberToName);
            }
        }
    } else { // There are manual changes... We'll have to store them then
        KOdfGenericStyle style(KOdfGenericStyle::ParagraphAutoStyle, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        charStyle.removeDuplicates(*originalCharStyle);
        if (!charStyle.isEmpty()) {
            charStyle.saveOdf(style);
            generatedName = context.mainStyles().insert(style, "T");
        }
    }

    return generatedName;
}

// A convinience function to get a listId from a list-format
static KListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KListStyle::ListIdType listId;

    if (sizeof(KListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KListStyle::ListId).toUInt();
    else
        listId = format.property(KListStyle::ListId).toULongLong();

    return listId;
}

QHash<QTextList *, QString> KoTextWriter::Private::saveListStyles(QTextBlock block, int to)
{
    QHash<KoList *, QString> generatedLists;
    QHash<QTextList *, QString> listStyles;

    for (;block.isValid() && ((to == -1) || (block.position() < to)); block = block.next()) {
        QTextList *textList = block.textList();
        if (!textList)
            continue;
        KListStyle::ListIdType listId = ListId(textList->format());
        if (KoList *list = KoTextDocument(document).list(listId)) {
            if (generatedLists.contains(list)) {
                if (!listStyles.contains(textList))
                    listStyles.insert(textList, generatedLists.value(list));
                continue;
            }
            KListStyle *listStyle = list->style();
            bool automatic = listStyle->styleId() == 0;
            KOdfGenericStyle style(automatic ? KOdfGenericStyle::ListAutoStyle : KOdfGenericStyle::ListStyle);
            listStyle->saveOdf(style);
            QString generatedName = context.mainStyles().insert(style, listStyle->name(), KOdfGenericStyles::AllowDuplicates);
            listStyles[textList] = generatedName;
            generatedLists.insert(list, generatedName);
        } else {
            if (listStyles.contains(textList))
                continue;
            KListLevelProperties llp = KListLevelProperties::fromTextList(textList);
            KOdfGenericStyle style(KOdfGenericStyle::ListAutoStyle);
            KListStyle listStyle;
            listStyle.setLevelProperties(llp);
            listStyle.saveOdf(style);
            QString generatedName = context.mainStyles().insert(style, listStyle.name());
            listStyles[textList] = generatedName;
        }
    }
    return listStyles;
}

void KoTextWriter::Private::saveParagraph(const QTextBlock &block, int from, int to)
{
    QTextCursor cursor(block);
    QTextBlockFormat blockFormat = block.blockFormat();
    const int outlineLevel = blockFormat.intProperty(KParagraphStyle::OutlineLevel);

    TagInformation blockTagInformation;
    if (outlineLevel > 0) {
        blockTagInformation.setTagName("text:h");
        blockTagInformation.addAttribute("text:outline-level", outlineLevel);
    } else {
        blockTagInformation.setTagName("text:p");
    }

    int changeId = openTagRegion(block.position(), KoTextWriter::Private::ParagraphOrHeader, blockTagInformation);

    if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
        if (!deleteMergeRegionOpened && !splitRegionOpened && !cursor.currentTable() && (!cursor.currentList() || outlineLevel)) {
            splitEndBlockNumber = checkForSplit(block);
            if (splitEndBlockNumber != -1) {
                splitRegionOpened = true;
                QString splitId = QString("split") + QString::number(splitIdCounter);
                writer->addAttribute("split:split001-idref", splitId);
            }
        }

        if (splitRegionOpened && (block.blockNumber() == splitEndBlockNumber)) {
            splitRegionOpened = false;
            splitEndBlockNumber = -1;
            QString splitId = QString("split") + QString::number(splitIdCounter);
            writer->addAttribute("delta:split-id", splitId);
            int changeId = block.blockFormat().intProperty(KCharacterStyle::ChangeTrackerId);
            writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            writer->addAttribute("delta:insertion-type", "split");
            splitIdCounter++;
        }
    }

    QString styleName = saveParagraphStyle(block);
    if (!styleName.isEmpty())
        writer->addAttribute("text:style-name", styleName);

    if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
        QTextBlock previousBlock = block.previous();
        if (previousBlock.isValid()) {
            QTextBlockFormat blockFormat = block.blockFormat();
            int changeId = blockFormat.intProperty(KCharacterStyle::ChangeTrackerId);
            if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
                QTextFragment firstFragment = (block.begin()).fragment();
                QTextCharFormat firstFragmentFormat = firstFragment.charFormat();
                int firstFragmentChangeId = firstFragmentFormat.intProperty(KCharacterStyle::ChangeTrackerId);
                if (changeTracker->isDuplicateChangeId(firstFragmentChangeId)) {
                    firstFragmentChangeId = changeTracker->originalChangeId(firstFragmentChangeId);
                }
                if (firstFragmentChangeId != changeId) {
                    QString outputXml("<delta:removed-content delta:removal-change-idref=\"" + changeTransTable.value(changeId) + "\"/>");
                    writer->addCompleteElement(outputXml.toUtf8());
                }
            }
        }
    }

    // Write the fragments and their formats
    QTextCharFormat blockCharFormat = cursor.blockCharFormat();
    QTextCharFormat previousCharFormat;
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        const int fragmentStart = currentFragment.position();
        const int fragmentEnd = fragmentStart + currentFragment.length();
        if (to != -1 && fragmentStart >= to)
            break;
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            QTextCharFormat compFormat = charFormat;
            bool identical;
            previousCharFormat.clearProperty(KCharacterStyle::ChangeTrackerId);
            compFormat.clearProperty(KCharacterStyle::ChangeTrackerId);
            if (previousCharFormat == compFormat)
                identical = true;
            else
                identical = false;

            if (changeTracker && changeTracker->saveFormat() == KChangeTracker::ODF_1_2) {
                saveODF12Change(charFormat);
            }

            const KoTextBlockData *blockData = dynamic_cast<const KoTextBlockData *>(block.userData());
            if (blockData && (it == block.begin())) {
                writer->addAttribute("text:id", context.subId(blockData));
            }
            //kDebug(30015) << "from:" << from << " to:" << to;
            KoTextInlineRdf* inlineRdf;
            if ((inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(charFormat)) && (it == block.begin())) {
                // Write xml:id here for Rdf
                kDebug(30015) << "have inline rdf xmlid:" << inlineRdf->xmlId();
                inlineRdf->saveOdf(context, writer);
            }

            KInlineObject *inlineObject = layout ? layout->inlineTextObjectManager()->inlineTextObject(charFormat) : 0;
            if (currentFragment.length() == 1 && inlineObject
                    && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                if (!dynamic_cast<KDeleteChangeMarker*>(inlineObject)) {
                    bool saveInlineObject = true;

                    if (KoTextMeta* z = dynamic_cast<KoTextMeta*>(inlineObject)) {
                        if (z->textPosition() < from) {
                            //
                            // This <text:meta> starts before the selection, default
                            // to not saving it with special cases to allow saving
                            //
                            saveInlineObject = false;
                            if (z->type() == KoTextMeta::StartBookmark) {
                                if (z->endBookmark()->textPosition() > from) {
                                    //
                                    // They have selected something starting after the
                                    // <text:meta> opening but before the </text:meta>
                                    //
                                    saveInlineObject = true;
                                }
                            }
                        }
                    }

                    bool saveSpan = dynamic_cast<KoVariable*>(inlineObject) != 0;

                    if (saveSpan) {
                        QString styleName = saveCharacterStyle(charFormat, blockCharFormat);
                        if (!styleName.isEmpty()) {
                            writer->startElement("text:span", false);
                            writer->addAttribute("text:style-name", styleName);
                        } else {
                            saveSpan = false;
                        }
                    }

                    if (saveInlineObject) {
                        int changeId = charFormat.intProperty(KCharacterStyle::ChangeTrackerId);
                        KoTextAnchor *textAnchor = dynamic_cast<KoTextAnchor *>(inlineObject);
                        if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
                            if (textAnchor && changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::InsertChange) {
                                textAnchor->shape()->setAdditionalAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                                textAnchor->shape()->setAdditionalAttribute("delta:insertion-type", "insert-with-content");
                            } else if (textAnchor && changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
                                writer->startElement("delta:removed-content", false);
                                writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
                            }
                        }

                        inlineObject->saveOdf(context);

                        if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
                            if (textAnchor && changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::InsertChange) {
                                textAnchor->shape()->removeAdditionalAttribute("delta:insertion-change-idref");
                                textAnchor->shape()->removeAdditionalAttribute("delta:insertion-type");
                            } else if (textAnchor && changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
                                writer->endElement();
                            }
                        }
                    }

                    if (saveSpan) {
                        writer->endElement();
                    }
                    //
                    // Track the end marker for matched pairs so we produce valid
                    // ODF
                    //
                    if (KoTextMeta* z = dynamic_cast<KoTextMeta*>(inlineObject)) {
                        kDebug(30015) << "found kometa, type:" << z->type();
                        if (z->type() == KoTextMeta::StartBookmark)
                            pairedInlineObjectStack.append(z->endBookmark());
                        if (z->type() == KoTextMeta::EndBookmark
                                && !pairedInlineObjectStack.isEmpty())
                            pairedInlineObjectStack.removeLast();
                    } else if (KoBookmark* z = dynamic_cast<KoBookmark*>(inlineObject)) {
                        if (z->type() == KoBookmark::StartBookmark)
                            pairedInlineObjectStack.append(z->endBookmark());
                        if (z->type() == KoBookmark::EndBookmark
                                && !pairedInlineObjectStack.isEmpty())
                            pairedInlineObjectStack.removeLast();
                    }
                }
            } else {
                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);

                TagInformation fragmentTagInformation;
                if (charFormat.isAnchor()) {
                    fragmentTagInformation.setTagName("text:a");
                    fragmentTagInformation.addAttribute("xlink:type", "simple");
                    fragmentTagInformation.addAttribute("xlink:href", charFormat.anchorHref());
                } else if (!styleName.isEmpty() /*&& !identical*/) {
                    fragmentTagInformation.setTagName("text:span");
                    fragmentTagInformation.addAttribute("text:style-name", styleName);
                }

                int changeId = openTagRegion(currentFragment.position(), KoTextWriter::Private::Span, fragmentTagInformation);

                QString text = currentFragment.text();
                int spanFrom = fragmentStart >= from ? 0 : from;
                int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                    writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                } else {
                    writer->addTextSpan(text);
                }

                closeTagRegion(changeId);
            } // if (inlineObject)

            previousCharFormat = charFormat;
        } // if (fragment.valid())
    } // foreach(fragment)

    //kDebug(30015) << "pairedInlineObjectStack.sz:" << pairedInlineObjectStack.size();
    if (to !=-1 && to < block.position() + block.length()) {
        foreach (KInlineObject* inlineObject, pairedInlineObjectStack) {
            inlineObject->saveOdf(context);
        }
    }

    if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
        QTextBlock nextBlock = block.next();
        if (nextBlock.isValid() && deleteMergeRegionOpened) {
            QTextBlockFormat nextBlockFormat = nextBlock.blockFormat();
            int changeId = nextBlockFormat.intProperty(KCharacterStyle::ChangeTrackerId);
            if (changeId && changeTracker->elementById(changeId)->changeType() == KOdfGenericChange::DeleteChange) {
                QTextFragment lastFragment = (--block.end()).fragment();
                QTextCharFormat lastFragmentFormat = lastFragment.charFormat();
                int lastFragmentChangeId = lastFragmentFormat.intProperty(KCharacterStyle::ChangeTrackerId);
                if (changeTracker->isDuplicateChangeId(lastFragmentChangeId)) {
                    lastFragmentChangeId = changeTracker->originalChangeId(lastFragmentChangeId);
                }
                if (lastFragmentChangeId != changeId) {
                    QString outputXml("<delta:removed-content delta:removal-change-idref=\"" + changeTransTable.value(changeId) + "\"/>");
                    writer->addCompleteElement(outputXml.toUtf8());
                }
            }
        }
    }

    if (changeTracker && changeTracker->saveFormat() == KChangeTracker::ODF_1_2) {
        while (int change = changeStack.top()) {
            writer->startElement("text:change-end", false);
            writer->addAttribute("text:change-id", changeTransTable.value(change));
            writer->endElement();
            changeStack.pop();
        }
    }

    closeTagRegion(changeId);
}

//Check if the whole Block is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForBlockChange(const QTextBlock &block)
{
    int changeId = 0;
    QTextBlock::iterator it = block.begin();

    if (it.atEnd()) {
        //This is a empty block. So just return the change-id of the block
        changeId = block.blockFormat().property(KCharacterStyle::ChangeTrackerId).toInt();
    }

    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            int currentChangeId = charFormat.property(KCharacterStyle::ChangeTrackerId).toInt();

            KInlineObject *inlineObject = layout ? layout->inlineTextObjectManager()->inlineTextObject(charFormat) : 0;
            if (currentFragment.length() == 1 && inlineObject && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                continue;
            }

            if (!currentChangeId) {
                // Encountered a fragment that is not a change
                // So break out of loop and return 0
                changeId = 0;
                break;
            } else {
                // This Fragment is a change fragment. Continue further.
                if (changeId == 0) {
                    //First fragment and it is a change-fragment
                    //Store it and continue
                    changeId = currentChangeId;
                    continue;
                } else {
                    if (currentChangeId == changeId) {
                        //Change Fragment and it is the same as the first change.
                        //continue looking
                        continue;
                    } else if (changeTracker->isParent(currentChangeId, changeId)) {
                        //The currentChangeId is a parent of changeId
                        changeId = currentChangeId;
                        continue;
                    } else if (changeTracker->isParent(changeId, currentChangeId)) {
                        //The current change id is a child of change-id
                        continue;
                    } else {
                        //A Change Fragment but not same as the first change fragment
                        //Break-out of loop and return 0
                        changeId = 0;
                        break;
                    }
                }
            }
        }
    }
    return changeId;
}

//Check if the whole list-item is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForListItemChange(const QTextBlock &block)
{
    QTextBlock listItemBlock = block;
    int listItemChangeId = checkForBlockChange(listItemBlock);
    while (listItemChangeId) {
        QTextBlock nextBlock = listItemBlock.next();
        if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KParagraphStyle::UnnumberedListItem))
            break;
        listItemBlock = nextBlock;
        listItemChangeId = checkForBlockChange(listItemBlock);
    }
    return listItemChangeId;
}

//Check if the whole list is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForListChange(const QTextBlock &listBlock)
{
    QTextBlock block(listBlock);
    QTextList *textList;
    textList = block.textList();

    KoTextDocument textDocument(block.document());
    KoList *list = textDocument.list(block);
    int topListLevel = KoList::level(block);

    int changeId = 0;
    do {
        int currentChangeId = checkForBlockChange(block);
        if (changeTracker->isDuplicateChangeId(currentChangeId)) {
            currentChangeId = changeTracker->originalChangeId(currentChangeId);
        }

        if (!currentChangeId) {
            // Encountered a list-item that is not a change
            // So break out of loop and return 0
            changeId = 0;
            break;
        } else {
            // This list-item is a changed cell. Continue further.
            if (changeId == 0) {
                //First list-item and it is a changed list-item
                //Store it and continue
                changeId = currentChangeId;
                block = block.next();
                continue;
            } else {
                if (currentChangeId == changeId) {
                    //Change found and it is the same as the first change.
                    //continue looking
                    block = block.next();
                    continue;
                } else if (changeTracker->isParent(currentChangeId, changeId)) {
                    //The currentChangeId is a parent of changeId
                    changeId = currentChangeId;
                    block = block.next();
                    continue;
                } else if (changeTracker->isParent(changeId, currentChangeId)) {
                    //The current change id is a child of change-id
                    block = block.next();
                    continue;
                } else {
                    //A Change found but not same as the first change
                    //Break-out of loop and return 0
                    changeId = 0;
                    break;
                }
            }
        }
    } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));
    return changeId;
}

//Check if the whole of table row is a part of a singke change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForTableRowChange(int position)
{
    int changeId = 0;
    QTextCursor cursor(document);
    cursor.setPosition(position);
    QTextTable *table = cursor.currentTable();

    if (table) {
        int row = table->cellAt(position).row();
        for (int i=0; i<table->columns(); i++) {
            int currentChangeId = table->cellAt(row,i).format().property(KCharacterStyle::ChangeTrackerId).toInt();
            if (!currentChangeId) {
                // Encountered a cell that is not a change
                // So break out of loop and return 0
                changeId = 0;
                break;
            } else {
                // This cell is a changed cell. Continue further.
                if (changeId == 0) {
                    //First cell and it is a changed-cell
                    //Store it and continue
                    changeId = currentChangeId;
                    continue;
                } else {
                    if (currentChangeId == changeId) {
                        //Change found and it is the same as the first change.
                        //continue looking
                        continue;
                    } else {
                        //A Change found but not same as the first change
                        //Break-out of loop and return 0
                        changeId = 0;
                        break;
                    }
                }
            }
        }
    }
    return changeId;
}

//Check if the whole of table column is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForTableColumnChange(int position)
{
    int changeId = 0;
    QTextCursor cursor(document);
    cursor.setPosition(position);
    QTextTable *table = cursor.currentTable();

    if (table) {
        int column = table->cellAt(position).column();
        for (int i=0; i<table->rows(); i++) {
            int currentChangeId = table->cellAt(i,column).format().property(KCharacterStyle::ChangeTrackerId).toInt();
            if (!currentChangeId) {
                // Encountered a cell that is not a change
                // So break out of loop and return 0
                changeId = 0;
                break;
            } else {
                // This cell is a changed cell. Continue further.
                if (changeId == 0) {
                    //First cell and it is a changed-cell
                    //Store it and continue
                    changeId = currentChangeId;
                    continue;
                } else {
                    if (currentChangeId == changeId) {
                        //Change found and it is the same as the first change.
                        //continue looking
                        continue;
                    } else {
                        //A Change found but not same as the first change
                        //Break-out of loop and return 0
                        changeId = 0;
                        break;
                    }
                }
            }
        }
    }
    return changeId;
}

void KoTextWriter::Private::saveTable(QTextTable *table, QHash<QTextList *, QString> &listStyles)
{
    TagInformation tableTagInformation;
    tableTagInformation.setTagName("table:table");
    int changeId = openTagRegion(table->firstCursorPosition().position(), KoTextWriter::Private::Table, tableTagInformation);

    for (int c = 0 ; c < table->columns() ; c++) {
        TagInformation tableColumnInformation;
        tableColumnInformation.setTagName("table:table-column");
        int changeId = openTagRegion(table->cellAt(0,c).firstCursorPosition().position(), KoTextWriter::Private::TableColumn, tableColumnInformation);
        closeTagRegion(changeId);
    }
    for (int r = 0 ; r < table->rows() ; r++) {
        TagInformation tableRowInformation;
        tableRowInformation.setTagName("table:table-row");
        int changeId = openTagRegion(table->cellAt(r,0).firstCursorPosition().position(), KoTextWriter::Private::TableRow, tableRowInformation);

        for (int c = 0 ; c < table->columns() ; c++) {
            QTextTableCell cell = table->cellAt(r, c);
            int changeId = 0;

            if ((cell.row() == r) && (cell.column() == c)) {
                TagInformation tableCellInformation;
                tableCellInformation.setTagName("table:table-cell");
                tableCellInformation.addAttribute("rowSpan", cell.rowSpan());
                tableCellInformation.addAttribute("columnSpan", cell.columnSpan());
                changeId = openTagRegion(table->cellAt(r,c).firstCursorPosition().position(), KoTextWriter::Private::TableCell, tableCellInformation);

                // Save the Rdf for the table cell
                QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
                QVariant v = cellFormat.property(KoTableCellStyle::InlineRdf);
                if (KoTextInlineRdf* inlineRdf = v.value<KoTextInlineRdf*>()) {
                    inlineRdf->saveOdf(context, writer);
                }
                writeBlocks(table->document(), cell.firstPosition(), cell.lastPosition(), listStyles, table);
            } else {
                TagInformation tableCellInformation;
                tableCellInformation.setTagName("table:covered-table-cell");
                changeId = openTagRegion(table->cellAt(r,c).firstCursorPosition().position(), KoTextWriter::Private::TableCell, tableCellInformation);
            }

            closeTagRegion(changeId);
        }
        closeTagRegion(changeId);
    }
    closeTagRegion(changeId);
}

void KoTextWriter::Private::saveTableOfContents(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable, QTextFrame *toc)
{

    writer->startElement("text:table-of-content");
        //TODO TOC styles
        writer->startElement("text:index-body");
            // write the title (one p block)
            QTextCursor localBlock = toc->firstCursorPosition();
            localBlock.movePosition(QTextCursor::NextBlock);
            int endTitle = localBlock.position();
            writer->startElement("text:index-title");
                writeBlocks(document, from, endTitle, listStyles, currentTable, toc);
            writer->endElement(); // text:index-title
        from = endTitle;

        QTextBlock block = toc->lastCursorPosition().block();
        writeBlocks(document, from, to, listStyles, currentTable, toc);


    writer->endElement(); // table:index-body
    writer->endElement(); // table:table-of-content
}

QTextBlock& KoTextWriter::Private::saveList(QTextBlock &block, QHash<QTextList *, QString> &listStyles, int level)
{
    QTextList *textList, *topLevelTextList;
    topLevelTextList = textList = block.textList();

    int headingLevel = 0, numberedParagraphLevel = 0;
    QTextBlockFormat blockFormat = block.blockFormat();
    headingLevel = blockFormat.intProperty(KParagraphStyle::OutlineLevel);
    numberedParagraphLevel = blockFormat.intProperty(KParagraphStyle::ListLevel);

    KoTextDocument textDocument(block.document());
    KoList *list = textDocument.list(block);
    int topListLevel = KoList::level(block);
    const bool saveChangeTrackerDeltaXml = changeTracker &&
        changeTracker->saveFormat() == KChangeTracker::DELTAXML;

    if (saveChangeTrackerDeltaXml) {
        if ((level == 1) && (!deleteMergeRegionOpened) && !headingLevel) {
            QTextBlock listBlock = block;
            do {
                int endBlockNumber = checkForDeleteMerge(listBlock);
                if (endBlockNumber != -1) {
                    deleteMergeEndBlockNumber = endBlockNumber;
                    deleteMergeRegionOpened = true;
                    openSplitMergeRegion();
                    break;
                }
                listBlock = listBlock.next();
            } while(textDocument.list(listBlock) == list);
        }
    }

    bool closeDelMergeRegion = false;
    if (saveChangeTrackerDeltaXml) {
        if ((level == 1) && (deleteMergeRegionOpened) && !headingLevel) {
            QTextBlock listBlock = block;
            do {
                if (listBlock.blockNumber() == deleteMergeEndBlockNumber) {
                    closeDelMergeRegion = true;
                }
                listBlock = listBlock.next();
            } while(textDocument.list(listBlock) == list);
        }
    }

    bool splitRegionOpened = false;
    int splitEndBlockNumber = -1;

    bool listStarted = false;
    int listChangeId = 0;
    if (!headingLevel && !numberedParagraphLevel) {
        listStarted = true;

        TagInformation listTagInformation;
        listTagInformation.setTagName("text:list");
        listTagInformation.addAttribute("text:style-name", listStyles[textList]);
        if (textList->format().hasProperty(KListStyle::ContinueNumbering))
            listTagInformation.addAttribute("text:continue-numbering",textList->format().boolProperty(KListStyle::ContinueNumbering) ? "true" : "false");

        listChangeId = openTagRegion(block.position(), KoTextWriter::Private::List, listTagInformation);
    }

    if (!headingLevel) {
        do {
            if (numberedParagraphLevel) {
                TagInformation paraTagInformation;
                paraTagInformation.setTagName("text:numbered-paragraph");
                paraTagInformation.addAttribute("text:level", numberedParagraphLevel);
                paraTagInformation.addAttribute("text:style-name", listStyles.value(textList));

                int changeId = openTagRegion(block.position(), KoTextWriter::Private::NumberedParagraph, paraTagInformation);
                writeBlocks(textDocument.document(), block.position(), block.position() + block.length() - 1, listStyles, 0, 0, textList);
                closeTagRegion(changeId);
            } else {
                if (saveChangeTrackerDeltaXml) {
                    int endBlockNumber = checkForSplit(block);
                    if (!deleteMergeRegionOpened && !splitRegionOpened && (endBlockNumber != -1)) {
                        openSplitMergeRegion();
                        splitRegionOpened = true;
                        splitEndBlockNumber = endBlockNumber;
                    }
                }

                const bool listHeader = blockFormat.boolProperty(KParagraphStyle::IsListHeader)|| blockFormat.boolProperty(KParagraphStyle::UnnumberedListItem);
                int listItemChangeId;
                TagInformation listItemTagInformation;
                listItemTagInformation.setTagName(listHeader ? "text:list-header" : "text:list-item");
                if (block.blockFormat().hasProperty(KParagraphStyle::ListStartValue)) {
                    int startValue = block.blockFormat().intProperty(KParagraphStyle::ListStartValue);
                    listItemTagInformation.addAttribute("text:start-value", startValue);
                }
                if (textList == topLevelTextList) {
                    listItemChangeId = openTagRegion(block.position(), KoTextWriter::Private::ListItem, listItemTagInformation);
                } else {
                    // This is a sub-list. So check for a list-change
                    listItemChangeId = openTagRegion(block.position(), KoTextWriter::Private::List, listItemTagInformation);
                }

                if (KListStyle::isNumberingStyle(textList->format().style())) {
                    if (KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData())) {
                        writer->startElement("text:number", false);
                        writer->addTextSpan(blockData->counterText());
                        writer->endElement();
                    }
                }

                if (textList == topLevelTextList) {
                    writeBlocks(textDocument.document(), block.position(), block.position() + block.length() - 1, listStyles, 0, 0, textList);
                    // we are generating a text:list-item. Look forward and generate unnumbered list items.
                    while (true) {
                        QTextBlock nextBlock = block.next();
                        if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KParagraphStyle::UnnumberedListItem))
                            break;
                        block = nextBlock;
                        saveParagraph(block, block.position(), block.position() + block.length() - 1);
                    }
                } else {
                    //This is a sub-list
                    block = saveList(block, listStyles, ++level);
                    //saveList will return a block one-past the last block of the list.
                    //Since we are doing a block.next() below, we need to go one back.
                    block = block.previous();
                }

                closeTagRegion(listItemChangeId);

                if (saveChangeTrackerDeltaXml) {
                    if (splitRegionOpened && (block.blockNumber() == splitEndBlockNumber)) {
                        splitRegionOpened = false;
                        splitEndBlockNumber = -1;
                        closeSplitMergeRegion();
                        postProcessListItemSplit(block.blockFormat().intProperty(KCharacterStyle::ChangeTrackerId));
                    }
                }
            }
            block = block.next();
            blockFormat = block.blockFormat();
            headingLevel = blockFormat.intProperty(KParagraphStyle::OutlineLevel);
            numberedParagraphLevel = blockFormat.intProperty(KParagraphStyle::ListLevel);
            textList = block.textList();
        } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));
    }

    if (listStarted) {
        closeTagRegion(listChangeId);
    }

    if (closeDelMergeRegion && saveChangeTrackerDeltaXml) {
        closeSplitMergeRegion();
        deleteMergeRegionOpened = false;
        deleteMergeEndBlockNumber = -1;
        postProcessDeleteMergeXml();
    }

    return block;
}

void KoTextWriter::Private::postProcessListItemSplit(int changeId)
{
    QString change = changeTransTable.value(changeId);

    QString generatedXmlString(generatedXmlArray);

    //Add the name-space definitions so that this can be parsed
    addNameSpaceDefinitions(generatedXmlString);

    //Now Parse the generatedXML and if successful generate the final output
    QString errorMsg;
    int errorLine, errorColumn;
    KXmlDocument doc;

    QXmlStreamReader reader(generatedXmlString);
    reader.setNamespaceProcessing(true);

    bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);

    if (!ok)
        return;

    QString outputXml;
    QTextStream outputXmlStream(&outputXml);

    KXmlElement rootElement = doc.documentElement();
    KXmlElement listItemElement = rootElement.firstChild().toElement();
    removeLeavingContentStart(outputXmlStream, listItemElement, change, 1);

    KXmlElement pElement = rootElement.firstChild().firstChild().toElement();
    removeLeavingContentStart(outputXmlStream, pElement, change, 2);

    KXmlElement childElement;
    forEachElement(childElement, rootElement) {
        if (childElement.localName() == "list-item") {
            insertAroundContent(outputXmlStream, childElement, change);
            KXmlElement pElement = childElement.firstChild().toElement();
            insertAroundContent(outputXmlStream, pElement, change);
            writeNode(outputXmlStream, pElement, true);
            outputXmlStream << "</text:p>";
            outputXmlStream << "</text:list-item>";
        } else {
            writeNode(outputXmlStream, pElement);
        }
    }

    removeLeavingContentEnd(outputXmlStream, 2);
    removeLeavingContentEnd(outputXmlStream, 1);
    writer->addCompleteElement(outputXml.toUtf8());
}

void KoTextWriter::Private::writeBlocks(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable, QTextFrame *currentFrame, QTextList *currentList)
{
    KoTextDocument textDocument(document);
    QTextBlock block = document->findBlock(from);

    while (block.isValid() && ((to == -1) || (block.position() <= to))) {
        QTextCursor cursor(block);
        QTextFrame *cursorFrame = cursor.currentFrame();
        int blockOutlineLevel = block.blockFormat().property(KParagraphStyle::OutlineLevel).toInt();

        if (cursorFrame != currentFrame && cursorFrame->format().hasProperty(KoText::TableOfContents)) {
            int frameBegin = cursorFrame->firstPosition();
            int frameEnd = cursorFrame->lastPosition();
            saveTableOfContents(document, frameBegin, frameEnd, listStyles, currentTable, cursor.currentFrame());
            block = cursorFrame->lastCursorPosition().block();
            block = block.next();
            continue;
        }
        if (cursor.currentTable() != currentTable) {
            // Call the code to save the table....
            saveTable(cursor.currentTable(), listStyles);
            // We skip to the end of the table.
            block = cursor.currentTable()->lastCursorPosition().block();
            block = block.next();
            continue;
        }

        if (cursor.currentList() != currentList) {
            int previousBlockNumber = block.blockNumber();
            block = saveList(block, listStyles, 1);
            int blockNumberToProcess = block.blockNumber();
            if (blockNumberToProcess != previousBlockNumber)
                continue;
        }

        if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
            if (!deleteMergeRegionOpened && !cursor.currentTable() && (!cursor.currentList() || blockOutlineLevel)) {
                deleteMergeEndBlockNumber = checkForDeleteMerge(block);
                if (deleteMergeEndBlockNumber != -1) {
                    deleteMergeRegionOpened = true;
                    openSplitMergeRegion();
                }
            }
        }

        saveParagraph(block, from, to);

        if (changeTracker && changeTracker->saveFormat() == KChangeTracker::DELTAXML) {
            if (deleteMergeRegionOpened && (block.blockNumber() == deleteMergeEndBlockNumber) && (!cursor.currentList() || blockOutlineLevel)) {
                closeSplitMergeRegion();
                deleteMergeRegionOpened = false;
                deleteMergeEndBlockNumber = -1;
                postProcessDeleteMergeXml();
            }
        }

        block = block.next();
    } // while
}

int KoTextWriter::Private::checkForSplit(const QTextBlock &block)
{
    return checkForMergeOrSplit(block, KOdfGenericChange::InsertChange);
}

int KoTextWriter::Private::checkForDeleteMerge(const QTextBlock &block)
{
    Q_ASSERT(changeTracker);
    return checkForMergeOrSplit(block, KOdfGenericChange::DeleteChange);
}

int KoTextWriter::Private::checkForMergeOrSplit(const QTextBlock &block, KOdfGenericChange::Type changeType)
{
    Q_ASSERT(changeTracker);
    QTextBlock endBlock = block;
    QTextCursor cursor(block);
    int endBlockNumber = -1;

    int splitMergeChangeId = 0, changeId = 0;
    do {
        if (!endBlock.next().isValid())
            break;

        int nextBlockChangeId;
        QTextTable *currentTable;

        if ((currentTable = QTextCursor(endBlock.next()).currentTable())) {
            nextBlockChangeId = currentTable->format().intProperty(KCharacterStyle::ChangeTrackerId);
        } else {
            nextBlockChangeId = endBlock.next().blockFormat().property(KCharacterStyle::ChangeTrackerId).toInt();
        }

        if (changeTracker->isDuplicateChangeId(nextBlockChangeId)) {
            nextBlockChangeId = changeTracker->originalChangeId(nextBlockChangeId);
        }

        if (!changeId) {
            splitMergeChangeId = changeId = nextBlockChangeId;
            if ((changeId) && (changeTracker->elementById(nextBlockChangeId)->changeType() == changeType)) {
                endBlock = endBlock.next();
            } else {
                changeId = 0;
            }
        } else {
            if (nextBlockChangeId == changeId) {
                endBlock = endBlock.next();
            } else {
                changeId = 0;
            }
        }
    } while(changeId);

    if ((endBlock.blockNumber() != block.blockNumber()) && (endBlock.text().length()) && !(QTextCursor(endBlock).currentTable())) {
        //Check that the last fragment of this block is not a part of this change. If so, it is not a merge or a split
        QTextFragment lastFragment = (--(endBlock.end())).fragment();
        QTextCharFormat lastFragmentFormat = lastFragment.charFormat();
        int lastFragmentChangeId = lastFragmentFormat.intProperty(KCharacterStyle::ChangeTrackerId);
        if (changeTracker->isDuplicateChangeId(lastFragmentChangeId)) {
            lastFragmentChangeId = changeTracker->originalChangeId(lastFragmentChangeId);
        }

        if (lastFragmentChangeId != splitMergeChangeId) {
            endBlockNumber = endBlock.blockNumber();
        }
    }
    return endBlockNumber;
}

void KoTextWriter::Private::openSplitMergeRegion()
{
    //Save the current writer
    oldXmlWriter = writer;

    //Create a new KXmlWriter pointing to a QBuffer
    generatedXmlArray.clear();
    generatedXmlBuffer.setBuffer(&generatedXmlArray);
    newXmlWriter = new KXmlWriter(&generatedXmlBuffer);

    //Set our xmlWriter as the writer to be used
    writer = newXmlWriter;
    context.setXmlWriter(*newXmlWriter);
}

void KoTextWriter::Private::closeSplitMergeRegion()
{
    //delete the new writer
    delete newXmlWriter;

    //Restore the actual xml writer
    writer = oldXmlWriter;
    context.setXmlWriter(*oldXmlWriter);
}

void KoTextWriter::Private::postProcessDeleteMergeXml()
{
    QString generatedXmlString(generatedXmlArray);

    //Add the name-space definitions so that this can be parsed
    addNameSpaceDefinitions(generatedXmlString);

    //Now Parse the generatedXML and if successful generate the final output
    QString errorMsg;
    int errorLine, errorColumn;
    KXmlDocument doc;

    QXmlStreamReader reader(generatedXmlString);
    reader.setNamespaceProcessing(true);

    bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
    if (ok) {
        //Generate the final XML output and save it
        QString outputXml;
        QTextStream outputXmlStream(&outputXml);
        generateFinalXml(outputXmlStream, doc.documentElement());
        writer->addCompleteElement(outputXml.toUtf8());
    }
}

void KoTextWriter::Private::addNameSpaceDefinitions(QString &generatedXmlString)
{
    //Generate the name-space definitions so that it can be parsed. Like what is office:text, office:delta etc
    QString nameSpaceDefinitions;
    QTextStream nameSpacesStream(&nameSpaceDefinitions);

    nameSpacesStream << "<generated-xml ";
    nameSpacesStream << "xmlns:office=\"" << KOdfXmlNS::office << "\" ";
    nameSpacesStream << "xmlns:meta=\"" << KOdfXmlNS::meta << "\" ";
    nameSpacesStream << "xmlns:config=\"" << KOdfXmlNS::config << "\" ";
    nameSpacesStream << "xmlns:text=\"" << KOdfXmlNS::text << "\" ";
    nameSpacesStream << "xmlns:table=\"" << KOdfXmlNS::table << "\" ";
    nameSpacesStream << "xmlns:draw=\"" << KOdfXmlNS::draw << "\" ";
    nameSpacesStream << "xmlns:presentation=\"" << KOdfXmlNS::presentation << "\" ";
    nameSpacesStream << "xmlns:dr3d=\"" << KOdfXmlNS::dr3d << "\" ";
    nameSpacesStream << "xmlns:chart=\"" << KOdfXmlNS::chart << "\" ";
    nameSpacesStream << "xmlns:form=\"" << KOdfXmlNS::form << "\" ";
    nameSpacesStream << "xmlns:script=\"" << KOdfXmlNS::script << "\" ";
    nameSpacesStream << "xmlns:style=\"" << KOdfXmlNS::style << "\" ";
    nameSpacesStream << "xmlns:number=\"" << KOdfXmlNS::number << "\" ";
    nameSpacesStream << "xmlns:math=\"" << KOdfXmlNS::math << "\" ";
    nameSpacesStream << "xmlns:svg=\"" << KOdfXmlNS::svg << "\" ";
    nameSpacesStream << "xmlns:fo=\"" << KOdfXmlNS::fo << "\" ";
    nameSpacesStream << "xmlns:anim=\"" << KOdfXmlNS::anim << "\" ";
    nameSpacesStream << "xmlns:smil=\"" << KOdfXmlNS::smil << "\" ";
    nameSpacesStream << "xmlns:koffice=\"" << KOdfXmlNS::koffice << "\" ";
    nameSpacesStream << "xmlns:officeooo=\"" << KOdfXmlNS::officeooo << "\" ";
    nameSpacesStream << "xmlns:delta=\"" << KOdfXmlNS::delta << "\" ";
    nameSpacesStream << "xmlns:split=\"" << KOdfXmlNS::split << "\" ";
    nameSpacesStream << "xmlns:ac=\"" << KOdfXmlNS::ac << "\" ";
    nameSpacesStream << ">";

    generatedXmlString.prepend(nameSpaceDefinitions);
    generatedXmlString.append("</generated-xml>");
}

void KoTextWriter::Private::generateFinalXml(QTextStream &outputXmlStream, const KXmlElement &element)
{
    QString firstChild = element.firstChild().toElement().localName();
    KXmlElement secondChildElement = element.firstChild().nextSibling().toElement();
    QString secondChild;

    do {
        secondChild = secondChildElement.localName();
        secondChildElement = secondChildElement.nextSibling().toElement();
    } while (secondChild == "removed-content");

    if ((firstChild == "p") && (secondChild == "h")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "p")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "p") && (secondChild == "p")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "h")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "p") && (secondChild == "list")) {
        handleParagraphWithListItemMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "list")) {
        handleParagraphWithListItemMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "p")) {
        handleListItemWithParagraphMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "h")) {
        handleListItemWithParagraphMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "list")) {
        handleListWithListMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "")) {
        handleListItemWithListItemMerge(outputXmlStream, element);
    } else {
        //Not Possible
    }

}

void KoTextWriter::Private::removeLeavingContentStart(QTextStream &outputXmlStream, KXmlElement &element, QString &changeId, int endIdCounter)
{
    outputXmlStream << "<delta:remove-leaving-content-start";
    outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
    outputXmlStream << " delta:end-element-idref=" << "\"end" << endIdCounter << "\">";

    outputXmlStream << "<text:" << element.localName();
    writeAttributes(outputXmlStream, element);
    outputXmlStream << "/>";

    outputXmlStream << "</delta:remove-leaving-content-start>";
}

void KoTextWriter::Private::removeLeavingContentEnd(QTextStream &outputXmlStream, int endIdCounter)
{
    outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << endIdCounter << "\"/>";
}

void KoTextWriter::Private::insertAroundContent(QTextStream &outputXmlStream, KXmlElement &element, QString &changeId)
{
    outputXmlStream << "<text:" << element.localName() << " delta:insertion-change-idref=" << "\"" << changeId << "\"";
    outputXmlStream << " delta:insertion-type=\"insert-around-content\"";
    writeAttributes(outputXmlStream, element);
    outputXmlStream << ">";
}

void KoTextWriter::Private::handleParagraphOrHeaderMerge(QTextStream &outputXmlStream, const KXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KOdfXmlNS::delta, "removal-change-idref");

    outputXmlStream << "<text:" << firstChild;
    writeAttributes(outputXmlStream, firstChildElement);
    outputXmlStream << ">";

    for (KXmlNode node = firstChildElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.toElement().localName() == "removed-content" && node.nextSibling().isNull()) {
            outputXmlStream << "<delta:merge delta:removal-change-idref=\"" << changeId << "\">";
            outputXmlStream << "<delta:leading-partial-content>";
            writeNode(outputXmlStream, node, true);
            outputXmlStream << "</delta:leading-partial-content>";
        } else {
            writeNode(outputXmlStream, node);
        }
    }

    outputXmlStream << "<delta:intermediate-content>";
    KXmlElement mergeEndElement = firstChildElement.nextSibling().toElement();
    while (mergeEndElement.localName() == "removed-content") {
        writeNode(outputXmlStream, mergeEndElement, true);
        mergeEndElement = mergeEndElement.nextSibling().toElement();
    }
    outputXmlStream << "</delta:intermediate-content>";

    for (KXmlNode node = mergeEndElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.toElement().localName() == "removed-content" && node.previousSibling().isNull()) {
            outputXmlStream << "<delta:trailing-partial-content>";
            outputXmlStream << "<text:" << mergeEndElement.localName();
            writeAttributes(outputXmlStream, mergeEndElement);
            outputXmlStream << ">";
            writeNode(outputXmlStream, node, true);
            outputXmlStream << "</text:" << mergeEndElement.localName() << ">";
            outputXmlStream << "</delta:trailing-partial-content>";
            outputXmlStream << "</delta:merge>";
        } else {
            writeNode(outputXmlStream, node);
        }
    }

    outputXmlStream << "</text:" << firstChild << ">";
}

void KoTextWriter::Private::handleParagraphWithHeaderMerge(QTextStream &outputXmlStream, const KXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KOdfXmlNS::delta, "removal-change-idref");

    //Start generating the XML
    insertAroundContent(outputXmlStream, firstChildElement, changeId);

    //Start a counter for end-element-idref
    int endIdCounter = 1;

    KXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else {
            removeLeavingContentStart(outputXmlStream, childElement, changeId, endIdCounter);
            writeNode(outputXmlStream, childElement, true);
            removeLeavingContentEnd(outputXmlStream, endIdCounter);
            endIdCounter++;
        }
    }

    outputXmlStream << "</text:" << firstChild << ">";

}

void KoTextWriter::Private::handleParagraphWithListItemMerge(QTextStream &outputXmlStream, const KXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KOdfXmlNS::delta, "removal-change-idref");

    //Start generating the XML
    insertAroundContent(outputXmlStream, firstChildElement, changeId);

    //Start a counter for end-element-idref
    int endIdCounter = 1;

    KXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == firstChild) {
            removeLeavingContentStart(outputXmlStream, childElement, changeId, endIdCounter);
            writeNode(outputXmlStream, childElement, true);
            removeLeavingContentEnd(outputXmlStream, endIdCounter);
            endIdCounter++;
        } else if (childElement.localName() == "list"){
            generateListForPWithListMerge(outputXmlStream, childElement, firstChild, changeId, endIdCounter, true);
        }
    }
}

void KoTextWriter::Private::generateListForPWithListMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                                          QString &mergeResultElement, QString &changeId, int &endIdCounter, \
                                                          bool removeLeavingContent)
{
    int listEndIdCounter = endIdCounter;
    if (removeLeavingContent) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listEndIdCounter);
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    bool tagTypeChangeEnded = false;
    bool listStarted = false;
    KXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            generateListItemForPWithListMerge(outputXmlStream, childElement, mergeResultElement,\
                                              changeId, endIdCounter, !tagTypeChangeEnded);
            if (!tagTypeChangeEnded) {
                tagTypeChangeEnded = true;
                if (childElement != element.lastChild().toElement()) {
                    listStarted = true;
                    insertAroundContent(outputXmlStream, element, changeId);
                }
            }
        } else {
            //Not Possible
        }
    }
    if (listStarted)
        outputXmlStream << "</text:list>";
    if (removeLeavingContent) {
        removeLeavingContentEnd(outputXmlStream, listEndIdCounter);
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::generateListItemForPWithListMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                                              QString &mergeResultElement, QString &changeId, int &endIdCounter, \
                                                              bool removeLeavingContent)
{
    int listItemEndIdCounter = endIdCounter;

    if (removeLeavingContent) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listItemEndIdCounter);
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == "p") {
            if (removeLeavingContent) {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
                writeNode(outputXmlStream, childElement, true);
                removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);
                outputXmlStream << "</text:" << mergeResultElement << ">";
            } else {
                writeNode(outputXmlStream, childElement, false);
            }
        } else if (childElement.localName() == "list") {
            generateListForPWithListMerge(outputXmlStream, childElement, mergeResultElement, changeId, endIdCounter, removeLeavingContent);
        } else {
            //Not Possible
        }
    }

    if (removeLeavingContent) {
        removeLeavingContentEnd(outputXmlStream, listItemEndIdCounter);
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::handleListItemWithParagraphMerge(QTextStream &outputXmlStream, const KXmlElement &element)
{
    deleteStartDepth = 0;

    // Find the first <p> so that we can get the change-id
    KXmlElement paragraphElement;
    forEachElement(paragraphElement, element) {
        if (paragraphElement.localName() == "p") {
            break;
        }
    }

    // Find the Change-id
    KXmlElement removedContentElement = paragraphElement.firstChild().toElement();
    QString changeId = removedContentElement.attributeNS(KOdfXmlNS::delta, "removal-change-idref");

    int endIdCounter = 1;

    KXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "list") {
            generateListForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, true);
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == "p") {
            int paragraphEndIdCounter = endIdCounter;
            endIdCounter++;
            removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
            writeNode(outputXmlStream, childElement, true);
            removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);
            outputXmlStream << "</text:p>";

            for (int i=0; i < deleteStartDepth; i++) {
                outputXmlStream << "</text:list-item>";
                outputXmlStream << "</text:list>";
            }
            break;
        } else {
        }
    }

}

void KoTextWriter::Private::generateListForListWithPMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                                          QString &changeId, int &endIdCounter, \
                                                          bool removeLeavingContent)
{
    static int listDepth = 0;
    listDepth++;

    if (!deleteStartDepth) {
        KXmlElement childElement;
        forEachElement(childElement, element) {
            if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
                bool startOfDeleteMerge = checkForDeleteStartInListItem(childElement, false);
                if (startOfDeleteMerge) {
                    deleteStartDepth = listDepth;
                }
            }
        }
    }

    int listEndIdCounter = endIdCounter;
    if (removeLeavingContent) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listEndIdCounter);
        insertAroundContent(outputXmlStream, element, changeId);
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KXmlElement childElement;
    forEachElement(childElement, element) {
        if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            bool startOfDeleteMerge = checkForDeleteStartInListItem(childElement);
            generateListItemForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, startOfDeleteMerge);
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        }
    }

    if (removeLeavingContent) {
        removeLeavingContentEnd(outputXmlStream, listEndIdCounter);
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::generateListItemForListWithPMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                                              QString &changeId, int &endIdCounter, bool removeLeavingContent)
{
    if (!removeLeavingContent) {
        writeNode(outputXmlStream, element, false);
    } else {
        int listItemEndIdCounter = endIdCounter;
        endIdCounter++;
        insertAroundContent(outputXmlStream, element, changeId);
        removeLeavingContentStart(outputXmlStream, element, changeId, listItemEndIdCounter);

        KXmlElement childElement;
        forEachElement (childElement, element) {
            if (childElement.localName() == "p") {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                insertAroundContent(outputXmlStream, childElement, changeId);
                removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
                writeNode(outputXmlStream, childElement, true);
                removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);
            } else if(childElement.localName() == "list") {
                generateListForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, removeLeavingContent);
            }
        }
        removeLeavingContentEnd(outputXmlStream, listItemEndIdCounter);
    }
}

bool KoTextWriter::Private::checkForDeleteStartInListItem(KXmlElement &element, bool checkRecursively)
{
    bool returnValue = false;
    KXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "p") {
            if (childElement.lastChild().toElement().localName() == "removed-content") {
                returnValue = true;
                break;
            }
        } else if ((childElement.localName() == "list") && (checkRecursively)) {
            KXmlElement listItem;
            forEachElement(listItem, childElement) {
                returnValue = checkForDeleteStartInListItem(listItem);
                if (returnValue)
                    break;
            }
        } else {
        }

        if (returnValue)
            break;
    }

    return returnValue;
}

void KoTextWriter::Private::handleListWithListMerge(QTextStream &outputXmlStream, const KXmlElement &element)
{
    int endIdCounter = 1;

    KXmlElement listElement = element.firstChild().toElement();
    QString changeId = findChangeIdForListItemMerge(listElement);

    KXmlElement firstChildElement = element.firstChild().toElement();
    generateListForListItemMerge(outputXmlStream, firstChildElement, changeId, endIdCounter, true, false);

    KXmlElement secondChildElement = element.firstChild().nextSibling().toElement();
    QString secondChild = secondChildElement.localName();
    while (secondChild == "removed-content")
    {
        writeNode(outputXmlStream, secondChildElement, false);
        secondChildElement = secondChildElement.nextSibling().toElement();
        secondChild = secondChildElement.localName();
    };

    generateListForListItemMerge(outputXmlStream, secondChildElement, changeId, endIdCounter, false, true);
}

void KoTextWriter::Private::handleListItemWithListItemMerge(QTextStream &outputXmlStream, const KXmlElement &element)
{
    int endIdCounter = 1;
    KXmlElement listElement = element.firstChild().toElement();
    QString changeId = findChangeIdForListItemMerge(listElement);

    generateListForListItemMerge(outputXmlStream, listElement, changeId, endIdCounter, false, false);
}

void KoTextWriter::Private::generateListForListItemMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                                         QString &changeId, int &endIdCounter, bool listMergeStart, bool listMergeEnd)
{
    int listEndIdCounter = endIdCounter;
    if (listMergeStart || listMergeEnd) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listEndIdCounter);

        if (listMergeStart) {
            insertAroundContent(outputXmlStream, element, changeId);
        }

    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KXmlElement childElement;
    bool deleteRangeStarted = false;

    if (listMergeEnd) {
        deleteRangeStarted = true;
    }

    forEachElement(childElement, element) {
        if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            if (!deleteRangeStarted) {
                deleteRangeStarted = checkForDeleteStartInListItem(childElement);
                generateListItemForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, deleteRangeStarted, false);
            } else {
                bool endOfDeleteRange = checkForDeleteEndInListItem(childElement);
                deleteRangeStarted = !endOfDeleteRange;
                generateListItemForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, false, endOfDeleteRange);
            }
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        }
    }

    if (listMergeStart || listMergeEnd) {
        removeLeavingContentEnd(outputXmlStream, listEndIdCounter);
        if (listMergeEnd) {
            outputXmlStream << "</text:list>";
        }
    } else {
        outputXmlStream << "</text:list>";
    }
}

void KoTextWriter::Private::generateListItemForListItemMerge(QTextStream &outputXmlStream, KXmlElement &element, \
                                                             QString &changeId, int &endIdCounter, bool listItemMergeStart, bool listItemMergeEnd)
{
    if (!listItemMergeStart && !listItemMergeEnd) {
        writeNode(outputXmlStream, element, false);
    } else {
        int listItemEndIdCounter = endIdCounter;
        endIdCounter++;

        if (listItemMergeStart) {
            insertAroundContent(outputXmlStream, element, changeId);
        }

        removeLeavingContentStart(outputXmlStream, element, changeId, listItemEndIdCounter);

        KXmlElement childElement;
        forEachElement (childElement, element) {
            if (childElement.localName() == "p") {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                if (listItemMergeStart) {
                    insertAroundContent(outputXmlStream, childElement, changeId);
                }

                removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
                writeNode(outputXmlStream, childElement, true);
                removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);

                if (listItemMergeEnd) {
                    outputXmlStream << "</text:" << childElement.localName() << ">";
                }
            } else if(childElement.localName() == "list") {
                generateListForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, listItemMergeStart ,listItemMergeEnd);
            }
        }

        removeLeavingContentEnd(outputXmlStream, listItemEndIdCounter);
        if (listItemMergeEnd) {
            outputXmlStream << "</text:list-item>";
        }
    }
}

bool KoTextWriter::Private::checkForDeleteEndInListItem(KXmlElement &element, bool checkRecursively)
{
    bool returnValue = false;
    KXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "p") {
            if (childElement.firstChild().toElement().localName() == "removed-content") {
                returnValue = true;
                break;
            }
        } else if ((childElement.localName() == "list") && (checkRecursively)) {
            KXmlElement listItem;
            forEachElement(listItem, childElement) {
                returnValue = checkForDeleteStartInListItem(listItem);
                if (returnValue)
                    break;
            }
        } else {
        }

        if (returnValue)
            break;
    }

    return returnValue;
}

QString KoTextWriter::Private::findChangeIdForListItemMerge(const KXmlElement &element)
{
    QString changeId;

    KXmlElement listItemElement;
    forEachElement (listItemElement, element) {
        if ((listItemElement.localName() == "list-item") || (listItemElement.localName() == "list-header")) {
            KXmlElement childElement;
            forEachElement (childElement, listItemElement) {
                if (childElement.localName() == "p") {
                    KXmlElement removedContentElement = childElement.lastChild().toElement();
                    if (removedContentElement.localName() == "removed-content") {
                        changeId = removedContentElement.attributeNS(KOdfXmlNS::delta, "removal-change-idref");
                        break;
                    }
                } else if (childElement.localName() == "list") {
                    changeId = findChangeIdForListItemMerge(childElement);
                    break;
                } else {
                    // Not Needed
                }
            }
        }
    }

    return changeId;
}

void KoTextWriter::Private::writeAttributes(QTextStream &outputXmlStream, KXmlElement &element)
{
    QList<QPair<QString, QString> > attributes = element.attributeNSNames();

    QPair<QString, QString> attributeNamePair;
    foreach (attributeNamePair, attributes) {
        if (attributeNamePair.first == KOdfXmlNS::text) {
            outputXmlStream << " text:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KOdfXmlNS::text, attributeNamePair.second) << "\"";
        } else if (attributeNamePair.first == KOdfXmlNS::delta) {
            outputXmlStream << " delta:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KOdfXmlNS::delta, attributeNamePair.second) << "\"";
        } else {
            //To Be Added when needed
        }
    }
}

void KoTextWriter::Private::writeNode(QTextStream &outputXmlStream, KXmlNode &node, bool writeOnlyChildren)
{
    if (node.isText()) {
        outputXmlStream  << node.toText().data();
    } else if (node.isElement()) {
        KXmlElement element = node.toElement();
        if ((element.localName() == "removed-content") && !element.childNodesCount()) {
            return;
        }

        if (!writeOnlyChildren) {
            outputXmlStream << "<" << element.prefix() << ":" << element.localName();
            writeAttributes(outputXmlStream,element);
            outputXmlStream << ">";
        }

        for (KXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            writeNode(outputXmlStream, node);
        }

        if (!writeOnlyChildren) {
            outputXmlStream << "</" << element.prefix() << ":" << element.localName() << ">";
        }
    }
}

void KoTextWriter::write(QTextDocument *document, int from, int to)
{
    d->document = document;
    d->styleManager = KoTextDocument(document).styleManager();
    d->layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());

    d->changeTracker = KoTextDocument(document).changeTracker();

    if (d->layout) Q_ASSERT(d->layout->inlineTextObjectManager());

    QTextBlock block = document->findBlock(from);

    QVector<int> changesVector;
    if (d->changeTracker) {
        d->changeTracker->allChangeIds(changesVector);
        foreach (int changeId, changesVector) {
            d->saveChange(changeId);
        }
    }

    QHash<QTextList *, QString> listStyles = d->saveListStyles(block, to);
    d->writeBlocks(document, from, to, listStyles);
}
