/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include <QXmlStreamReader>

#include "KoInlineObject.h"
#include "KoTextAnchor.h"
#include "KoShape.h"
#include "KoVariable.h"
#include "KoInlineTextObjectManager.h"
#include "styles/KoStyleManager.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "styles/KoTableCellStyle.h"
#include "KoTextDocumentLayout.h"
#include "KoTextBlockData.h"
#include "KoTextDocument.h"
#include "KoTextInlineRdf.h"

#include "KoTextMeta.h"
#include "KoBookmark.h"

#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>

#include <opendocument/KoTextSharedSavingData.h>
#include <changetracker/KoChangeTracker.h>
#include <changetracker/KoChangeTrackerElement.h>
#include <changetracker/KoDeleteChangeMarker.h>
#include <KoGenChange.h>
#include <KoGenChanges.h>
#include <rdf/KoDocumentRdfBase.h>

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#endif

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
    tagTypeChangeRegionOpened(false),
    tagTypeChangeEndBlockNumber(-1)
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
    int openChangeRegion(int position, ElementType elementType);
    void closeChangeRegion();

    QString saveParagraphStyle(const QTextBlock &block);
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
    KoXmlWriter *writer;

    KoTextDocumentLayout *layout;
    KoStyleManager *styleManager;
    KoChangeTracker *changeTracker;
    KoDocumentRdfBase *rdfData;
    QTextDocument *document;

    QStack<int> changeStack;
    QMap<int, QString> changeTransTable;
    // Things like bookmarks need to be properly turn down
    // during a cut and paste operation when their end marker
    // is not included in the selection.
    QList<KoInlineObject*> pairedInlineObjectStack;

    //For saving of delete-changes that result in a tag type change
    bool tagTypeChangeRegionOpened;
    int tagTypeChangeEndBlockNumber;
    int checkForTagTypeChanges(QTextBlock &block);
    void openTagTypeChangeRegion();
    void closeTagTypeChangeRegion();

    KoXmlWriter *oldXmlWriter;
    KoXmlWriter *newXmlWriter;
    QByteArray generatedXmlArray;
    QBuffer generatedXmlBuffer;

    void postProcessTagTypeChangeXml();
    void generateFinalXml(QTextStream &outputXmlStream, const KoXmlElement &element);
    
    // For Handling <p> with <h> or <h> with <p> merges
    void handleParagraphWithHeaderMerge(QTextStream &outputXmlStream, const KoXmlElement &element);

    // For handling <p> with <list-item> merges
    void handleParagraphWithListItemMerge(QTextStream &outputXmlStream, const KoXmlElement &element);
    void generateListForPWithListMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                       QString &mergeResultElement, QString &changeId, int &endIdCounter, bool removeLeavingContent);
    void generateListItemForPWithListMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                       QString &mergeResultElement, QString &changeId, int &endIdCounter, bool removeLeavingContent);

    // FOr Handling <list-item> with <p> merges
    int deleteStartDepth;
    void handleListItemWithParagraphMerge(QTextStream &outputXmlStream, const KoXmlElement &element);
    void generateListForListWithPMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool removeLeavingContent);
    void generateListItemForListWithPMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool removeLeavingContent);
    bool checkForDeleteStartInListItem(KoXmlElement &element, bool checkRecursively = true);

    void handleListWithListMerge(QTextStream &outputXmlStream, const KoXmlElement &element);

    // For handling <list-item> with <list-item> merges
    void handleListItemWithListItemMerge(QTextStream &outputXmlStream, const KoXmlElement &element);
    QString findChangeIdForListItemMerge(const KoXmlElement &element);
    void generateListForListItemMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool deleteStart, bool deleteEnd);
    void generateListItemForListItemMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                       QString &changeId, int &endIdCounter, bool deleteStart, bool deleteEnd);
    bool checkForDeleteEndInListItem(KoXmlElement &element, bool checkRecursively = true);

    void writeAttributes(QTextStream &outputXmlStream, KoXmlElement &element);
    void writeNode(QTextStream &outputXmlStream, KoXmlNode &node, bool writeOnlyChildren = false);
};

void KoTextWriter::Private::saveChange(QTextCharFormat format)
{
    if (!changeTracker /*&& changeTracker->isEnabled()*/)
        return;//The change tracker exist and we are allowed to save tracked changes

    int changeId = format.property(KoCharacterStyle::ChangeTrackerId).toInt();
    if (changeId) { //There is a tracked change
        saveChange(changeId);
    }
}

void KoTextWriter::Private::saveChange(int changeId)
{
    if(changeTransTable.value(changeId).length())
        return;
    KoGenChange change;
    changeTracker->saveInlineChange(changeId, change);
    QString changeName = sharedData->genChanges().insert(change);
    changeTransTable.insert(changeId, changeName);
}

int KoTextWriter::Private::openChangeRegion(int position, ElementType elementType)
{
    int changeId = 0;
    QTextCursor cursor(document);
    cursor.setPosition(position + 1);
    QTextCharFormat charFormat = cursor.charFormat();
    QTextBlock block = document->findBlock(position);

    switch (elementType) {
        case KoTextWriter::Private::Span:
            changeId = charFormat.property(KoCharacterStyle::ChangeTrackerId).toInt();
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
            changeId = cursor.currentTable()->cellAt(position).format().property(KoCharacterStyle::ChangeTrackerId).toInt();
            break;
        case KoTextWriter::Private::Table:
            cursor.setPosition(position);
            QTextTableFormat tableFormat = cursor.currentTable()->format();
            changeId = tableFormat.property(KoCharacterStyle::ChangeTrackerId).toInt();
            break;
    }

    if (changeId && (changeStack.top() != changeId)) {
        changeStack.push(changeId);
        saveChange(changeId);
    } else {
        changeId = 0;
    }

    if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
        writer->startElement("delta:removed-content", false);
        writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
    }

    return changeId;
}

void KoTextWriter::Private::closeChangeRegion()
{
    int changeId = changeStack.pop();
    if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
        writer->endElement(); //delta:removed-content
    }
    return;
}

KoTextWriter::KoTextWriter(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData)
    : d(new Private(context))
{
    d->rdfData = rdfData;
    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    if (sharedData) {
        d->sharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }

    if (!d->sharedData) {
        d->sharedData = new KoTextSharedSavingData();
        KoGenChanges *changes = new KoGenChanges();
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
    KoParagraphStyle *defaultParagraphStyle = styleManager->defaultParagraphStyle();

    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat();

    KoParagraphStyle *originalParagraphStyle = styleManager->paragraphStyle(blockFormat.intProperty(KoParagraphStyle::StyleId));
    if (!originalParagraphStyle)
        originalParagraphStyle = defaultParagraphStyle;

    QString generatedName;
    QString displayName = originalParagraphStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    // we'll convert the blockFormat to a KoParagraphStyle to check for local changes.
    KoParagraphStyle paragStyle(blockFormat, charFormat);
    if (paragStyle == (*originalParagraphStyle)) { // This is the real, unmodified character style.
        // TODO zachmann: this could use the name of the saved style without saving it again
        // therefore we would need to store that information in the saving context
        if (originalParagraphStyle != defaultParagraphStyle) {
            KoGenStyle style(KoGenStyle::ParagraphStyle, "paragraph");
            originalParagraphStyle->saveOdf(style, context.mainStyles());
            generatedName = context.mainStyles().insert(style, internalName, KoGenStyles::DontAddNumberToName);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::ParagraphAutoStyle, "paragraph", internalName);
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

QString KoTextWriter::Private::saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    KoCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();

    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(charFormat.intProperty(KoCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    KoCharacterStyle charStyle(charFormat);
    // we'll convert it to a KoCharacterStyle to check for local changes.
    // we remove that properties given by the paragraphstyle char format, these are not present in the saved style (should it really be the case?)
    charStyle.removeDuplicates(blockCharFormat);
    if (charStyle == (*originalCharStyle)) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            if (!charStyle.isEmpty()) {
                KoGenStyle style(KoGenStyle::ParagraphStyle, "text");
                originalCharStyle->saveOdf(style);
                generatedName = context.mainStyles().insert(style, internalName, KoGenStyles::DontAddNumberToName);
            }
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::ParagraphAutoStyle, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
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
static KoListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KoListStyle::ListIdType listId;

    if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KoListStyle::ListId).toUInt();
    else
        listId = format.property(KoListStyle::ListId).toULongLong();

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
        KoListStyle::ListIdType listId = ListId(textList->format());
        if (KoList *list = KoTextDocument(document).list(listId)) {
            if (generatedLists.contains(list)) {
                if (!listStyles.contains(textList))
                    listStyles.insert(textList, generatedLists.value(list));
                continue;
            }
            KoListStyle *listStyle = list->style();
            bool automatic = listStyle->styleId() == 0;
            KoGenStyle style(automatic ? KoGenStyle::ListAutoStyle : KoGenStyle::ListStyle);
            listStyle->saveOdf(style);
            QString generatedName = context.mainStyles().insert(style, listStyle->name(), KoGenStyles::AllowDuplicates);
            listStyles[textList] = generatedName;
            generatedLists.insert(list, generatedName);
        } else {
            if (listStyles.contains(textList))
                continue;
            KoListLevelProperties llp = KoListLevelProperties::fromTextList(textList);
            KoGenStyle style(KoGenStyle::ListAutoStyle);
            KoListStyle listStyle;
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
    const int outlineLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
    int changeId = openChangeRegion(block.position(), KoTextWriter::Private::ParagraphOrHeader);
    if (outlineLevel > 0) {
        writer->startElement("text:h", false);
        writer->addAttribute("text:outline-level", outlineLevel);
    } else {
        writer->startElement("text:p", false);
    }

    if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
        writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
        writer->addAttribute("delta:insertion-type", "insert-with-content");
    }

    QString styleName = saveParagraphStyle(block);
    if (!styleName.isEmpty())
        writer->addAttribute("text:style-name", styleName);

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
            previousCharFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
            compFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
            if (previousCharFormat == compFormat)
                identical = true;
            else
                identical = false;

            saveChange(charFormat);

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

            KoInlineObject *inlineObject = layout ? layout->inlineTextObjectManager()->inlineTextObject(charFormat) : 0;
            if (currentFragment.length() == 1 && inlineObject
                    && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                if (!dynamic_cast<KoDeleteChangeMarker*>(inlineObject)) {
                    bool saveInlineObject = true;

                    if (KoTextMeta* z = dynamic_cast<KoTextMeta*>(inlineObject)) {
                        if (z->position() < from) {
                            //
                            // This <text:meta> starts before the selection, default
                            // to not saving it with special cases to allow saving
                            //
                            saveInlineObject = false;
                            if (z->type() == KoTextMeta::StartBookmark) {
                                if (z->endBookmark()->position() > from) {
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
                        }
                        else {
                            saveSpan = false;
                        }
                    }

                    if (saveInlineObject) {
                        int changeId = openChangeRegion(currentFragment.position(), KoTextWriter::Private::Span);
                        KoTextAnchor *textAnchor = dynamic_cast<KoTextAnchor *>(inlineObject);
                        if (textAnchor && changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                            textAnchor->shape()->setAdditionalAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                            textAnchor->shape()->setAdditionalAttribute("delta:insertion-type", "insert-with-content");
                        }
                        
                        inlineObject->saveOdf(context);
                        
                        if (textAnchor && changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                            textAnchor->shape()->removeAdditionalAttribute("delta:insertion-change-idref");
                            textAnchor->shape()->removeAdditionalAttribute("delta:insertion-type");
                        }
                        
                        if (changeId)
                            closeChangeRegion();
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
                int changeId = openChangeRegion(currentFragment.position(), KoTextWriter::Private::Span);
                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);
                if (charFormat.isAnchor()) {
                    writer->startElement("text:a", false);
                    writer->addAttribute("xlink:type", "simple");
                    writer->addAttribute("xlink:href", charFormat.anchorHref());
                } else if (!styleName.isEmpty() /*&& !identical*/) {
                    writer->startElement("text:span", false);
                    writer->addAttribute("text:style-name", styleName);
                    if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                        writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                        writer->addAttribute("delta:insertion-type", "insert-with-content");
                    } else if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::FormatChange) {
                        writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                        writer->addAttribute("delta:insertion-type", "insert-around-content");
                    }
                }

                QString text = currentFragment.text();
                int spanFrom = fragmentStart >= from ? 0 : from;
                int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                    writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                } else {
                    writer->addTextSpan(text);
                }

                if ((!styleName.isEmpty() /*&& !identical*/) || charFormat.isAnchor()) {
                    writer->endElement();
                }

                if (changeId)
                    closeChangeRegion();
            } // if (inlineObject)

            previousCharFormat = charFormat;
        } // if (fragment.valid())
    } // foreach(fragment)

    //kDebug(30015) << "pairedInlineObjectStack.sz:" << pairedInlineObjectStack.size();
    if (to !=-1 && to < block.position() + block.length()) {
        foreach (KoInlineObject* inlineObject, pairedInlineObjectStack) {
            inlineObject->saveOdf(context);
        }
    }

    if (changeId)
        closeChangeRegion();
    
    writer->endElement();
}

//Check if the whole Block is a part of a single change
//If so return the changeId else return 0 
int KoTextWriter::Private::checkForBlockChange(const QTextBlock &block)
{
    int changeId = 0;
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            int currentChangeId = charFormat.property(KoCharacterStyle::ChangeTrackerId).toInt();
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
        if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
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
   
    int listChangeId = 0;
    do {
        listChangeId = checkForBlockChange(block);
        if (!listChangeId)
            break;
        block = block.next();
    } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));

    return listChangeId;
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
            int currentChangeId = table->cellAt(row,i).format().property(KoCharacterStyle::ChangeTrackerId).toInt();
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
            int currentChangeId = table->cellAt(i,column).format().property(KoCharacterStyle::ChangeTrackerId).toInt();
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
    int changeId = openChangeRegion(table->firstCursorPosition().position(), KoTextWriter::Private::Table);
    writer->startElement("table:table");
    
    if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
        writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
        writer->addAttribute("delta:insertion-type", "insert-with-content");
    }
    
    for (int c = 0 ; c < table->columns() ; c++) {
        int changeId = openChangeRegion(table->cellAt(0,c).firstCursorPosition().position(), KoTextWriter::Private::TableColumn);
        writer->startElement("table:table-column");

        if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
            writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            writer->addAttribute("delta:insertion-type", "insert-with-content");
        }

        writer->endElement(); // table:table-column

        if (changeId)
            closeChangeRegion();
    }
    for (int r = 0 ; r < table->rows() ; r++) {
        int changeId = openChangeRegion(table->cellAt(r,0).firstCursorPosition().position(), KoTextWriter::Private::TableRow);
        writer->startElement("table:table-row");

        if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
            writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            writer->addAttribute("delta:insertion-type", "insert-with-content");
        }

        for (int c = 0 ; c < table->columns() ; c++) {
            QTextTableCell cell = table->cellAt(r, c);
            int changeId = openChangeRegion(table->cellAt(r,c).firstCursorPosition().position(), KoTextWriter::Private::TableCell);
            if ((cell.row() == r) && (cell.column() == c)) {
                writer->startElement("table:table-cell");
                writer->addAttribute("rowSpan", cell.rowSpan());
                writer->addAttribute("columnSpan", cell.columnSpan());
        
                if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                    writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                    writer->addAttribute("delta:insertion-type", "insert-with-content");
                }

                // Save the Rdf for the table cell
                QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
                QVariant v = cellFormat.property(KoTableCellStyle::InlineRdf);
                if (KoTextInlineRdf* inlineRdf = v.value<KoTextInlineRdf*>()) {
                    inlineRdf->saveOdf(context, writer);
                }
                writeBlocks(table->document(), cell.firstPosition(), cell.lastPosition(), listStyles, table);
            } else {
                writer->startElement("table:covered-table-cell");
                if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                    writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                    writer->addAttribute("delta:insertion-type", "insert-with-content");
                }
            }
            writer->endElement(); // table:table-cell
            if (changeId)
                closeChangeRegion();
        }
        writer->endElement(); // table:table-row
        if (changeId)
            closeChangeRegion();
    }
    writer->endElement(); // table:table

    if (changeId)
        closeChangeRegion();
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
    headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
    numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);

    KoTextDocument textDocument(block.document());
    KoList *list = textDocument.list(block);
    int topListLevel = KoList::level(block);

    if ((level == 1) && (!tagTypeChangeRegionOpened) && !headingLevel) {
        QTextBlock listBlock = block;
        do {
            int endBlockNumber = checkForTagTypeChanges(listBlock);
            if (endBlockNumber != -1) {
                tagTypeChangeEndBlockNumber = endBlockNumber;
                tagTypeChangeRegionOpened = true;
                openTagTypeChangeRegion();
                break;
            }
            listBlock = listBlock.next();
        } while(textDocument.list(listBlock) == list);
    }

    bool closeTagChangeRegion = false;
    if ((level == 1) && (tagTypeChangeRegionOpened) && !headingLevel) {
        QTextBlock listBlock = block;
        do {
            if (listBlock.blockNumber() == tagTypeChangeEndBlockNumber) {
                closeTagChangeRegion = true;
            }
            listBlock = listBlock.next();
        } while(textDocument.list(listBlock) == list);
    }

    bool listStarted = false;
    int listChangeId = 0;
    if (!headingLevel && !numberedParagraphLevel) {
        listStarted = true;
        listChangeId = openChangeRegion(block.position(), KoTextWriter::Private::List);
        writer->startElement("text:list", false);
        writer->addAttribute("text:style-name", listStyles[textList]);
        if (textList->format().hasProperty(KoListStyle::ContinueNumbering))
            writer->addAttribute("text:continue-numbering",textList->format().boolProperty(KoListStyle::ContinueNumbering) ? "true" : "false");

        if (listChangeId && changeTracker->elementById(listChangeId)->getChangeType() == KoGenChange::InsertChange) {
            writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(listChangeId));
            writer->addAttribute("delta:insertion-type", "insert-with-content");
        }
    }

    if (!headingLevel) {
        do {
            if (numberedParagraphLevel) {
                int changeId = openChangeRegion(block.position(), KoTextWriter::Private::NumberedParagraph);
                writer->startElement("text:numbered-paragraph", false);
                writer->addAttribute("text:level", numberedParagraphLevel);
                writer->addAttribute("text:style-name", listStyles.value(textList));

                if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                    writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                    writer->addAttribute("delta:insertion-type", "insert-with-content");
                }
                
                writeBlocks(textDocument.document(), block.position(), block.position() + block.length() - 1, listStyles, 0, 0, textList); 
                writer->endElement(); 
                
                if (changeId) {
                    closeChangeRegion();
                }
            } else {
                const bool listHeader = blockFormat.boolProperty(KoParagraphStyle::IsListHeader)|| blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem);
                int listItemChangeId;
                if (textList == topLevelTextList) {
                    listItemChangeId = openChangeRegion(block.position(), KoTextWriter::Private::ListItem);
                } else {
                    // This is a sub-list. So check for a list-change
                    listItemChangeId = openChangeRegion(block.position(), KoTextWriter::Private::List);
                }

                writer->startElement(listHeader ? "text:list-header" : "text:list-item", false);

                if (listItemChangeId && changeTracker->elementById(listItemChangeId)->getChangeType() == KoGenChange::InsertChange) {
                    writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(listItemChangeId));
                    writer->addAttribute("delta:insertion-type", "insert-with-content");
                }

                if (KoListStyle::isNumberingStyle(textList->format().style())) {
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
                        if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
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
                if (listItemChangeId)
                   closeChangeRegion();
                writer->endElement(); 
            }
            block = block.next();
            blockFormat = block.blockFormat();
            headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
            numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);
            textList = block.textList();
        } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));
    }

    if (listStarted) {
        if (listChangeId)
            closeChangeRegion();
        writer->endElement();
    }

    if (closeTagChangeRegion) {
        closeTagTypeChangeRegion();
    }
   
    return block;
}

void KoTextWriter::Private::writeBlocks(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable, QTextFrame *currentFrame, QTextList *currentList)
{
    KoTextDocument textDocument(document);
    QTextBlock block = document->findBlock(from);

    while (block.isValid() && ((to == -1) || (block.position() <= to))) {
        QTextCursor cursor(block);
        QTextFrame *cursorFrame = cursor.currentFrame();
        int blockOutlineLevel = block.blockFormat().property(KoParagraphStyle::OutlineLevel).toInt();

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

        if (!tagTypeChangeRegionOpened && !cursor.currentTable() && (!cursor.currentList() || blockOutlineLevel)) {
            tagTypeChangeEndBlockNumber = checkForTagTypeChanges(block);
            if (tagTypeChangeEndBlockNumber != -1) {
                tagTypeChangeRegionOpened = true;
                openTagTypeChangeRegion();
            }
        }

        saveParagraph(block, from, to);

        if (tagTypeChangeRegionOpened && (block.blockNumber() == tagTypeChangeEndBlockNumber) && (!cursor.currentList() || blockOutlineLevel)) {
            closeTagTypeChangeRegion();
        }

        block = block.next();
    } // while
}

int KoTextWriter::Private::checkForTagTypeChanges(QTextBlock &block)
{
    QTextBlock endBlock = block;
    QTextCursor cursor(block);    
    int endBlockNumber = -1;

    int changeId = 0;
    do {
        if (!endBlock.next().isValid())
            break;
        
        int lastFragmentChangeId = endBlock.end().fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        int nextFragmentChangeId = endBlock.next().begin().fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        
        if ((lastFragmentChangeId) && (nextFragmentChangeId) \
            && (lastFragmentChangeId == nextFragmentChangeId) \
            && (changeTracker->elementById(lastFragmentChangeId)->getChangeType() == KoGenChange::DeleteChange)) {
            changeId = lastFragmentChangeId;
            endBlock = endBlock.next();
        } else {
            changeId = 0;
        }
    } while(changeId);

    if (endBlock.blockNumber() != block.blockNumber()) {
        // Check For <p> and a <h> merge
        bool outlineChange = false;
        if (block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel) \
            != endBlock.blockFormat().intProperty(KoParagraphStyle::OutlineLevel)) {
            outlineChange = true;
        }

        // Check for <p> or a <h> with a <list> merge
        bool pWithListMerge = false;
        if (!QTextCursor(block).currentList() && QTextCursor(endBlock).currentList()) {
            pWithListMerge = true;
        }

        // Check for a <list> merge with <p> or a <h>
        bool listWithPMerge = false;
        if (QTextCursor(block).currentList() && !QTextCursor(endBlock).currentList()) {
            listWithPMerge = true;
        }

        // Check For List-Item merge
        bool listItemMerge = false;
        if (QTextCursor(block).currentList() && QTextCursor(endBlock).currentList()) {
            listItemMerge = true;
        }

        if (outlineChange || pWithListMerge || listWithPMerge || listItemMerge)
            endBlockNumber = endBlock.blockNumber();
    } 

    return endBlockNumber;
}

void KoTextWriter::Private::openTagTypeChangeRegion()
{
    //Save the current writer
    oldXmlWriter = writer;

    //Create a new KoXmlWriter pointing to a QBuffer
    generatedXmlArray.clear();
    generatedXmlBuffer.setBuffer(&generatedXmlArray);
    newXmlWriter = new KoXmlWriter(&generatedXmlBuffer);

    //Set our xmlWriter as the writer to be used
    writer = newXmlWriter;
    context.setXmlWriter(*newXmlWriter);
}

void KoTextWriter::Private::closeTagTypeChangeRegion()
{
    //delete the new writer
    delete newXmlWriter;

    //Restore the actual xml writer
    writer = oldXmlWriter;
    context.setXmlWriter(*oldXmlWriter);

    //Post-Process and save the generated xml in the appropriate way.
    postProcessTagTypeChangeXml();
}

void KoTextWriter::Private::postProcessTagTypeChangeXml()
{
    QString generatedXmlString(generatedXmlArray);

    //Generate the name-space definitions so that it can be parsed. Like what is office:text, office:delta etc
    QString nameSpaceDefinitions;
    QTextStream nameSpacesStream(&nameSpaceDefinitions);
    
    nameSpacesStream << "<generated-xml ";
    nameSpacesStream << "xmlns:office=\"" << KoXmlNS::office << "\" ";
    nameSpacesStream << "xmlns:meta=\"" << KoXmlNS::meta << "\" ";
    nameSpacesStream << "xmlns:config=\"" << KoXmlNS::config << "\" ";
    nameSpacesStream << "xmlns:text=\"" << KoXmlNS::text << "\" ";
    nameSpacesStream << "xmlns:table=\"" << KoXmlNS::table << "\" ";
    nameSpacesStream << "xmlns:draw=\"" << KoXmlNS::draw << "\" ";
    nameSpacesStream << "xmlns:presentation=\"" << KoXmlNS::presentation << "\" ";
    nameSpacesStream << "xmlns:dr3d=\"" << KoXmlNS::dr3d << "\" ";
    nameSpacesStream << "xmlns:chart=\"" << KoXmlNS::chart << "\" ";
    nameSpacesStream << "xmlns:form=\"" << KoXmlNS::form << "\" ";
    nameSpacesStream << "xmlns:script=\"" << KoXmlNS::script << "\" ";
    nameSpacesStream << "xmlns:style=\"" << KoXmlNS::style << "\" ";
    nameSpacesStream << "xmlns:number=\"" << KoXmlNS::number << "\" ";
    nameSpacesStream << "xmlns:math=\"" << KoXmlNS::math << "\" ";
    nameSpacesStream << "xmlns:svg=\"" << KoXmlNS::svg << "\" ";
    nameSpacesStream << "xmlns:fo=\"" << KoXmlNS::fo << "\" ";
    nameSpacesStream << "xmlns:anim=\"" << KoXmlNS::anim << "\" ";
    nameSpacesStream << "xmlns:smil=\"" << KoXmlNS::smil << "\" ";
    nameSpacesStream << "xmlns:koffice=\"" << KoXmlNS::koffice << "\" ";
    nameSpacesStream << "xmlns:officeooo=\"" << KoXmlNS::officeooo << "\" ";
    nameSpacesStream << "xmlns:delta=\"" << KoXmlNS::delta << "\" ";
    nameSpacesStream << "xmlns:split=\"" << KoXmlNS::split << "\" ";
    nameSpacesStream << ">";

    generatedXmlString.prepend(nameSpaceDefinitions);
    generatedXmlString.append("</generated-xml>");

    //Now Parse the generatedXML and if successful generate the final output
    QString errorMsg;
    int errorLine, errorColumn;
    KoXmlDocument doc; 

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

void KoTextWriter::Private::generateFinalXml(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    QString firstChild = element.firstChild().toElement().localName();
    KoXmlElement secondChildElement = element.firstChild().nextSibling().toElement();
    QString secondChild;

    do {
        secondChild = secondChildElement.localName();
        secondChildElement = secondChildElement.nextSibling().toElement();
    } while (secondChild == "removed-content");

    if ((firstChild == "p") && (secondChild == "h")) {
        handleParagraphWithHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "p")) {
        handleParagraphWithHeaderMerge(outputXmlStream, element);
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

void KoTextWriter::Private::handleParagraphWithHeaderMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KoXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KoXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    //Start generating the XML
    outputXmlStream << "<text:" << firstChild << " delta:insertion-change-idref=" << "\"" << changeId << "\"";
    outputXmlStream << " delta:insertion-type=\"insert-around-content\"";
    writeAttributes(outputXmlStream, firstChildElement);
    outputXmlStream << ">";

    //Start a counter for end-element-idref
    int endIdCounter = 1;

    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else {
            outputXmlStream << "<delta:remove-leaving-content-start";
            outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
            outputXmlStream << " delta:end-element-idref=" << "\"end" << endIdCounter << "\">";

            outputXmlStream << "<text:" << childElement.localName();
            writeAttributes(outputXmlStream, childElement);
            outputXmlStream << "/>";
            
            outputXmlStream << "</delta:remove-leaving-content-start>";

            writeNode(outputXmlStream, childElement, true);

            outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << endIdCounter << "\"/>";
            endIdCounter++;
        }
    }

    outputXmlStream << "</text:" << firstChild << ">";

}

void KoTextWriter::Private::handleParagraphWithListItemMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KoXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KoXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    //Start generating the XML
    outputXmlStream << "<text:" << firstChild << " delta:insertion-change-idref=" << "\"" << changeId << "\"";
    outputXmlStream << " delta:insertion-type=\"insert-around-content\"";
    writeAttributes(outputXmlStream, firstChildElement);
    outputXmlStream << ">";

    //Start a counter for end-element-idref
    int endIdCounter = 1;

    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == firstChild) {
            outputXmlStream << "<delta:remove-leaving-content-start";
            outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
            outputXmlStream << " delta:end-element-idref=" << "\"end" << endIdCounter << "\">";

            outputXmlStream << "<text:" << childElement.localName();
            writeAttributes(outputXmlStream, childElement);
            outputXmlStream << "/>";
            
            outputXmlStream << "</delta:remove-leaving-content-start>";

            writeNode(outputXmlStream, childElement, true);

            outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << endIdCounter << "\"/>";
            endIdCounter++;
        } else if (childElement.localName() == "list"){
            generateListForPWithListMerge(outputXmlStream, childElement, firstChild, changeId, endIdCounter, true);
        }
    }
}

void KoTextWriter::Private::generateListForPWithListMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                                          QString &mergeResultElement, QString &changeId, int &endIdCounter, \
                                                          bool removeLeavingContent)
{
    int listEndIdCounter = endIdCounter;
    if (removeLeavingContent) {
        endIdCounter++;
        outputXmlStream << "<delta:remove-leaving-content-start";
        outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
        outputXmlStream << " delta:end-element-idref=" << "\"end" << listEndIdCounter << "\">";

        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << "/>";
            
        outputXmlStream << "</delta:remove-leaving-content-start>";
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    bool tagTypeChangeEnded = false;
    bool listStarted = false;
    KoXmlElement childElement;
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
                    outputXmlStream << "<text:list";
                    outputXmlStream << " delta:insertion-change-idref=" << "\"" << changeId << "\"";
                    outputXmlStream << " delta:insertion-type=\"insert-around-content\"";
                    writeAttributes(outputXmlStream, element);
                    outputXmlStream << ">";
                }
            }
        } else {
            //Not Possible
        }
    }
    if (listStarted)
        outputXmlStream << "</text:list>";
    if (removeLeavingContent) {
        outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << listEndIdCounter << "\"/>";
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::generateListItemForPWithListMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                                              QString &mergeResultElement, QString &changeId, int &endIdCounter, \
                                                              bool removeLeavingContent)
{
    int listItemEndIdCounter = endIdCounter;

    if (removeLeavingContent) {
        endIdCounter++;
        outputXmlStream << "<delta:remove-leaving-content-start";
        outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
        outputXmlStream << " delta:end-element-idref=" << "\"end" << listItemEndIdCounter << "\">";

        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << "/>";
            
        outputXmlStream << "</delta:remove-leaving-content-start>";
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == "p") {
            if (removeLeavingContent) {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                outputXmlStream << "<delta:remove-leaving-content-start";
                outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
                outputXmlStream << " delta:end-element-idref=" << "\"end" << paragraphEndIdCounter << "\">";

                outputXmlStream << "<text:" << childElement.localName();
                writeAttributes(outputXmlStream, childElement);
                outputXmlStream << "/>";
            
                outputXmlStream << "</delta:remove-leaving-content-start>";
                
                writeNode(outputXmlStream, childElement, true);

                outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << paragraphEndIdCounter << "\"/>";
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
        outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << listItemEndIdCounter << "\"/>";
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::handleListItemWithParagraphMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    deleteStartDepth = 0;

    // Find the first <p> so that we can get the change-id
    KoXmlElement paragraphElement;
    forEachElement(paragraphElement, element) {
        if (paragraphElement.localName() == "p") {
            break;
        }
    }

    // Find the Change-id
    KoXmlElement removedContentElement = paragraphElement.firstChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    int endIdCounter = 1;

    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "list") {
            generateListForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, true);
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == "p") {
            int paragraphEndIdCounter = endIdCounter;
            endIdCounter++;

            outputXmlStream << "<delta:remove-leaving-content-start";
            outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
            outputXmlStream << " delta:end-element-idref=" << "\"end" << paragraphEndIdCounter << "\">";

            outputXmlStream << "<text:" << childElement.localName();
            writeAttributes(outputXmlStream, childElement);
            outputXmlStream << "/>";
              
            outputXmlStream << "</delta:remove-leaving-content-start>";

            writeNode(outputXmlStream, childElement, true);
            
            outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << paragraphEndIdCounter << "\"/>";
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

void KoTextWriter::Private::generateListForListWithPMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                                          QString &changeId, int &endIdCounter, \
                                                          bool removeLeavingContent)
{
    static int listDepth = 0;
    listDepth++;

    if (!deleteStartDepth) {
        KoXmlElement childElement;
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
        outputXmlStream << "<delta:remove-leaving-content-start";
        outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
        outputXmlStream << " delta:end-element-idref=" << "\"end" << listEndIdCounter << "\">";

        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << "/>";
            
        outputXmlStream << "</delta:remove-leaving-content-start>";

        outputXmlStream << "<text:list delta:insertion-change-idref=\"" << changeId << "\" delta:insertion-type = \"insert-around-content\"";
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";

    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            bool startOfDeleteMerge = checkForDeleteStartInListItem(childElement);
            generateListItemForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, startOfDeleteMerge);
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        }
    }
    
    if (removeLeavingContent) {
        outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << listEndIdCounter << "\"/>";
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::generateListItemForListWithPMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                                              QString &changeId, int &endIdCounter, bool removeLeavingContent)
{
    if (!removeLeavingContent) {
        writeNode(outputXmlStream, element, false);
    } else {
        int listItemEndIdCounter = endIdCounter;
        endIdCounter++;
        outputXmlStream << "<text:list-item delta:insertion-change-idref=\"" << changeId << "\" delta:insertion-type = \"insert-around-content\"";
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";

        outputXmlStream << "<delta:remove-leaving-content-start";
        outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
        outputXmlStream << " delta:end-element-idref=" << "\"end" << listItemEndIdCounter << "\">";

        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << "/>";
            
        outputXmlStream << "</delta:remove-leaving-content-start>";

        KoXmlElement childElement;
        forEachElement (childElement, element) {
            if (childElement.localName() == "p") {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                outputXmlStream << "<text:p delta:insertion-change-idref=\"" << changeId << "\" delta:insertion-type = \"insert-around-content\"";
                writeAttributes(outputXmlStream, childElement);
                outputXmlStream << ">";

                outputXmlStream << "<delta:remove-leaving-content-start";
                outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
                outputXmlStream << " delta:end-element-idref=" << "\"end" << paragraphEndIdCounter << "\">";

                outputXmlStream << "<text:" << childElement.localName();
                writeAttributes(outputXmlStream, childElement);
                outputXmlStream << "/>";
                
                outputXmlStream << "</delta:remove-leaving-content-start>";

                writeNode(outputXmlStream, childElement, true);
            
                outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << paragraphEndIdCounter << "\"/>";
            } else if(childElement.localName() == "list") {
                generateListForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, removeLeavingContent);
            }
        }
        
        outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << listItemEndIdCounter << "\"/>";
    }
}

bool KoTextWriter::Private::checkForDeleteStartInListItem(KoXmlElement &element, bool checkRecursively)
{
    bool returnValue = false;
    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "p") {
            if (childElement.lastChild().toElement().localName() == "removed-content") {
                returnValue = true;
                break;
            }
        } else if ((childElement.localName() == "list") && (checkRecursively)) {
            KoXmlElement listItem;
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

void KoTextWriter::Private::handleListWithListMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    int endIdCounter = 1;

    KoXmlElement listElement = element.firstChild().toElement();
    QString changeId = findChangeIdForListItemMerge(listElement);

    KoXmlElement firstChildElement = element.firstChild().toElement();
    generateListForListItemMerge(outputXmlStream, firstChildElement, changeId, endIdCounter, true, false);

    KoXmlElement secondChildElement = element.firstChild().nextSibling().toElement();
    QString secondChild = secondChildElement.localName();
    while (secondChild == "removed-content")
    {
        writeNode(outputXmlStream, secondChildElement, false);
        secondChildElement = secondChildElement.nextSibling().toElement();
        secondChild = secondChildElement.localName();
    };

    generateListForListItemMerge(outputXmlStream, secondChildElement, changeId, endIdCounter, false, true);
}

void KoTextWriter::Private::handleListItemWithListItemMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    int endIdCounter = 1;
    KoXmlElement listElement = element.firstChild().toElement();
    QString changeId = findChangeIdForListItemMerge(listElement);
    
    generateListForListItemMerge(outputXmlStream, listElement, changeId, endIdCounter, false, false);
}

void KoTextWriter::Private::generateListForListItemMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                                         QString &changeId, int &endIdCounter, bool deleteStart, bool deleteEnd)
{
    int listEndIdCounter = endIdCounter;
    if (deleteStart || deleteEnd) {
        endIdCounter++;
        outputXmlStream << "<delta:remove-leaving-content-start";
        outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
        outputXmlStream << " delta:end-element-idref=" << "\"end" << listEndIdCounter << "\">";

        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << "/>";
            
        outputXmlStream << "</delta:remove-leaving-content-start>";
        
        if (deleteStart) {
            outputXmlStream << "<text:list delta:insertion-change-idref=\"" << changeId << "\" delta:insertion-type = \"insert-around-content\"";
            writeAttributes(outputXmlStream, element);
            outputXmlStream << ">";
        }

    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KoXmlElement childElement;
    bool deleteRangeStarted = false;
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
    
    if (deleteStart || deleteEnd) {
        outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << listEndIdCounter << "\"/>";
        if (deleteEnd) {
            outputXmlStream << "</text:list>";
        }
    } else {
        outputXmlStream << "</text:list>";
    }
}

void KoTextWriter::Private::generateListItemForListItemMerge(QTextStream &outputXmlStream, KoXmlElement &element, \
                                                             QString &changeId, int &endIdCounter, bool deleteStart, bool deleteEnd)
{
    if (!deleteStart && !deleteEnd) {
        writeNode(outputXmlStream, element, false);
    } else {
        int listItemEndIdCounter = endIdCounter;
        endIdCounter++;

        if (deleteStart) {
            outputXmlStream << "<text:list-item delta:insertion-change-idref=\"" << changeId << "\" delta:insertion-type = \"insert-around-content\"";
            writeAttributes(outputXmlStream, element);
            outputXmlStream << ">";
        }

        outputXmlStream << "<delta:remove-leaving-content-start";
        outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
        outputXmlStream << " delta:end-element-idref=" << "\"end" << listItemEndIdCounter << "\">";

        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << "/>";
            
        outputXmlStream << "</delta:remove-leaving-content-start>";

        KoXmlElement childElement;
        forEachElement (childElement, element) {
            if (childElement.localName() == "p") {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                if (deleteStart) {
                    outputXmlStream << "<text:p delta:insertion-change-idref=\"" << changeId << "\" delta:insertion-type = \"insert-around-content\"";
                    writeAttributes(outputXmlStream, childElement);
                    outputXmlStream << ">";
                }

                outputXmlStream << "<delta:remove-leaving-content-start";
                outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
                outputXmlStream << " delta:end-element-idref=" << "\"end" << paragraphEndIdCounter << "\">";

                outputXmlStream << "<text:" << childElement.localName();
                writeAttributes(outputXmlStream, childElement);
                outputXmlStream << "/>";
                
                outputXmlStream << "</delta:remove-leaving-content-start>";

                writeNode(outputXmlStream, childElement, true);
            
                outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << paragraphEndIdCounter << "\"/>";

                if (deleteEnd) {
                    outputXmlStream << "</text:" << childElement.localName() << ">";
                }
            } else if(childElement.localName() == "list") {
                generateListForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, deleteStart ,deleteEnd);
            }
        }
        
        outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << listItemEndIdCounter << "\"/>";
        if (deleteEnd) {
            outputXmlStream << "</text:list-item>";
        }
    }
}

bool KoTextWriter::Private::checkForDeleteEndInListItem(KoXmlElement &element, bool checkRecursively)
{
    bool returnValue = false;
    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "p") {
            if (childElement.firstChild().toElement().localName() == "removed-content") {
                returnValue = true;
                break;
            }
        } else if ((childElement.localName() == "list") && (checkRecursively)) {
            KoXmlElement listItem;
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

QString KoTextWriter::Private::findChangeIdForListItemMerge(const KoXmlElement &element)
{
    QString changeId;

    KoXmlElement listItemElement;
    forEachElement (listItemElement, element) {
        if ((listItemElement.localName() == "list-item") || (listItemElement.localName() == "list-header")) {
            KoXmlElement childElement;
            forEachElement (childElement, listItemElement) {
                if (childElement.localName() == "p") {
                    KoXmlElement removedContentElement = childElement.lastChild().toElement();
                    if (removedContentElement.localName() == "removed-content") {
                        changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");
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

void KoTextWriter::Private::writeAttributes(QTextStream &outputXmlStream, KoXmlElement &element)
{
    QList<QPair<QString, QString> > attributes = element.attributeNSNames();

    QPair<QString, QString> attributeNamePair;
    foreach (attributeNamePair, attributes) {
        if (attributeNamePair.first == KoXmlNS::text) {
            outputXmlStream << " text:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KoXmlNS::text, attributeNamePair.second) << "\"";    
        } else if (attributeNamePair.first == KoXmlNS::delta) {
            outputXmlStream << " delta:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KoXmlNS::delta, attributeNamePair.second) << "\"";    
        } else {
            //To Be Added when needed
        }
    }
}

void KoTextWriter::Private::writeNode(QTextStream &outputXmlStream, KoXmlNode &node, bool writeOnlyChildren)
{
    if (node.isText()) {
        outputXmlStream  << node.toText().data();
    } else if (node.isElement()) {
        KoXmlElement element = node.toElement();

        if (!writeOnlyChildren) {
            outputXmlStream << "<" << element.prefix() << ":" << element.localName();
            writeAttributes(outputXmlStream,element);
            outputXmlStream << ">";
        }    

        for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
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

    QHash<QTextList *, QString> listStyles = d->saveListStyles(block, to);
    d->writeBlocks(document, from, to, listStyles);
}
