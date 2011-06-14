/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include "KChangeTracker.h"
#include "KFormatChangeInformation_p.h"
#include "KDeleteChangeMarker.h"

//KOffice includes
#include "styles/KCharacterStyle.h"
#include "KChangeTrackerElement.h"
#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KInlineTextObjectManager.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoList.h>
#include <KListStyle.h>
#include <KParagraphStyle.h>
#include <KStyleManager.h>
#include <KFormatChangeInformation_p.h>
#include <KDeletedRowColumnDataStore_p.h>

//KDE includes
#include <KDebug>
#include <KDateTime>
#include <KGlobal>
#include <KLocale>

//Qt includes
#include <QColor>
#include <QList>
#include <QString>
#include <QHash>
#include <QMultiHash>
#include <QTextCursor>
#include <QTextFormat>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextList>

class KChangeTracker::Private
{
public:
    Private()
      : changeId(1),
        recordChanges(false),
        displayChanges(false),
        insertionBgColor(101,255,137),
        deletionBgColor(255,185,185),
        formatChangeBgColor(195,195,255)
    {
        deletedRowColumnData = new KDeletedRowColumnDataStore();
    }
    ~Private() { }

    QMultiHash<int, int> children;
    QMultiHash<int, int> duplicateIds;
    QHash<int, int> parents;
    QHash<int, KChangeTrackerElement *> changes;
    QHash<QString, int> loadedChanges;
    QHash<int, KFormatChangeInformation *> changeInformation;
    QList<int> saveChanges;
    QList<int> acceptedRejectedChanges;
    int changeId;
    bool recordChanges;
    bool displayChanges;
    QColor insertionBgColor, deletionBgColor, formatChangeBgColor;
    QString changeAuthorName;
    KChangeTracker::ChangeSaveFormat changeSaveFormat;
    KDeletedRowColumnDataStore *deletedRowColumnData;

    static bool checkListDeletion(QTextList *list, const QTextCursor &cursor);
};

KChangeTracker::KChangeTracker(QObject *parent)
    : QObject(parent),
    d(new Private())
{
    d->changeId = 1;
}

KChangeTracker::~KChangeTracker()
{
    delete d;
}

void KChangeTracker::setRecordChanges(bool enabled)
{
    d->recordChanges = enabled;
}

bool KChangeTracker::recordChanges() const
{
    return d->recordChanges;
}

void KChangeTracker::setDisplayChanges(bool enabled)
{
    d->displayChanges = enabled;
}

bool KChangeTracker::displayChanges() const
{
    return d->displayChanges;
}

QString KChangeTracker::authorName()
{
    return d->changeAuthorName;
}

void KChangeTracker::setAuthorName(const QString &authorName)
{
    d->changeAuthorName = authorName;
}

KChangeTracker::ChangeSaveFormat KChangeTracker::saveFormat()
{
    return d->changeSaveFormat;
}

void KChangeTracker::setSaveFormat(ChangeSaveFormat saveFormat)
{
    d->changeSaveFormat = saveFormat;
}

int KChangeTracker::formatChangeId(const QString &title, const QTextFormat &format, const QTextFormat &prevFormat, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KChangeTrackerElement *changeElement = new KChangeTrackerElement(title, KOdfGenericChange::FormatChange);
    changeElement->setChangeFormat(format);
    changeElement->setPrevFormat(prevFormat);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));

    changeElement->setCreator(d->changeAuthorName);

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

int KChangeTracker::insertChangeId(const QString &title, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KChangeTrackerElement *changeElement = new KChangeTrackerElement(title, KOdfGenericChange::InsertChange);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));
//    changeElement->setDate(KDateTime::currentLocalDateTime().toString("Y-m-dTH:M:Sz")); //i must have misunderstood the API doc but it doesn't work.
    changeElement->setCreator(d->changeAuthorName);

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

int KChangeTracker::deleteChangeId(const QString &title, const QTextDocumentFragment &selection, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KChangeTrackerElement *changeElement = new KChangeTrackerElement(title, KOdfGenericChange::DeleteChange);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));
    changeElement->setCreator(d->changeAuthorName);
    changeElement->setDeleteData(selection);

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

KChangeTrackerElement* KChangeTracker::elementById(int id)
{
    if (isDuplicateChangeId(id)) {
        id = originalChangeId(id);
    }
    return d->changes.value(id);
}

bool KChangeTracker::removeById(int id, bool freeMemory)
{
    if (freeMemory) {
      KChangeTrackerElement *temp = d->changes.value(id);
      delete temp;
    }
    return d->changes.remove(id);
}

bool KChangeTracker::containsInlineChanges(const QTextFormat &format)
{
    if (format.property(KCharacterStyle::ChangeTrackerId).toInt())
        return true;

    return false;
}

int KChangeTracker::mergeableId(KOdfGenericChange::Type type, const QString &title, int existingId) const
{
    if (!existingId || !d->changes.value(existingId))
        return 0;

    if (d->changes.value(existingId)->changeType() == type && d->changes.value(existingId)->changeTitle() == title)
        return existingId;
    else if (d->parents.contains(existingId))
        return mergeableId(type, title, d->parents.value(existingId));
    else
        return 0;
}

int KChangeTracker::split(int changeId)
{
    KChangeTrackerElement *element = new KChangeTrackerElement(*d->changes.value(changeId));
    d->changes.insert(d->changeId, element);
    return d->changeId++;
}

bool KChangeTracker::isParent(int testedParentId, int testedChildId) const
{
    if ((testedParentId == testedChildId) && !d->acceptedRejectedChanges.contains(testedParentId))
        return true;
    else if (d->parents.contains(testedChildId))
        return isParent(testedParentId, d->parents.value(testedChildId));
    else
        return false;
}

void KChangeTracker::setParent(int child, int parent)
{
    if (!d->children.values(parent).contains(child)) {
        d->children.insert(parent, child);
    }
    if (!d->parents.contains(child)) {
        d->parents.insert(child, parent);
    }
}

int KChangeTracker::parent(int changeId) const
{
    if (!d->parents.contains(changeId))
        return 0;
    if (d->acceptedRejectedChanges.contains(d->parents.value(changeId)))
        return parent(d->parents.value(changeId));
    return d->parents.value(changeId);
}

int KChangeTracker::createDuplicateChangeId(int existingChangeId)
{
    int duplicateChangeId = d->changeId;
    d->changeId++;

    d->duplicateIds.insert(existingChangeId, duplicateChangeId);

    return duplicateChangeId;
}

bool KChangeTracker::isDuplicateChangeId(int duplicateChangeId)
{
    bool isDuplicate = d->duplicateIds.values().contains(duplicateChangeId);
    return isDuplicate;
}

int KChangeTracker::originalChangeId(int duplicateChangeId)
{
    int originalChangeId = 0;
    QMultiHash<int, int>::const_iterator i = d->duplicateIds.constBegin();

    while (i != d->duplicateIds.constEnd()) {
        if (duplicateChangeId == i.value()) {
            originalChangeId = i.key();
            break;
        }
        ++i;
    }

    return originalChangeId;
}

void KChangeTracker::acceptRejectChange(int changeId, bool set)
{
    if (set) {
        if (!d->acceptedRejectedChanges.contains(changeId))
            d->acceptedRejectedChanges.append(changeId);
    }
    else {
        if (d->acceptedRejectedChanges.contains(changeId))
            d->acceptedRejectedChanges.removeAll(changeId);
    }

    d->changes.value(changeId)->setAcceptedRejected(set);
}

bool KChangeTracker::saveInlineChange(int changeId, KOdfGenericChange &change)
{
    if (!d->changes.contains(changeId))
        return false;

    change.setType(d->changes.value(changeId)->changeType());
    change.addChangeMetaData("dc-creator", d->changes.value(changeId)->creator());
    change.addChangeMetaData("dc-date", d->changes.value(changeId)->date());
    if (d->changes.value(changeId)->hasExtraMetaData())
        change.addChildElement("changeMetaData", d->changes.value(changeId)->extraMetaData());

    return true;
}

void KChangeTracker::setFormatChangeInformation(int formatChangeId, KFormatChangeInformation *formatInformation)
{
    d->changeInformation.insert(formatChangeId, formatInformation);
}

KFormatChangeInformation *KChangeTracker::formatChangeInformation(int formatChangeId)
{
    return d->changeInformation.value(formatChangeId);
}

void KChangeTracker::loadOdfChanges(const KXmlElement& element)
{
    if (element.namespaceURI() == KOdfXmlNS::text) {
        KXmlElement tag;
        forEachElement(tag, element) {
            if (! tag.isNull()) {
                const QString localName = tag.localName();
                if (localName == "changed-region") {
                    KChangeTrackerElement *changeElement = 0;
                    KXmlElement region;
                    forEachElement(region, tag) {
                        if (!region.isNull()) {
                            if (region.localName() == "insertion") {
                                changeElement = new KChangeTrackerElement(tag.attributeNS(KOdfXmlNS::text,"id"),KOdfGenericChange::InsertChange);
                            } else if (region.localName() == "format-change") {
                                changeElement = new KChangeTrackerElement(tag.attributeNS(KOdfXmlNS::text,"id"),KOdfGenericChange::FormatChange);
                            } else if (region.localName() == "deletion") {
                                changeElement = new KChangeTrackerElement(tag.attributeNS(KOdfXmlNS::text,"id"),KOdfGenericChange::DeleteChange);
                            }
                            KXmlElement metadata = region.namedItemNS(KOdfXmlNS::office,"change-info").toElement();
                            if (!metadata.isNull()) {
                                KXmlElement date = metadata.namedItem("dc:date").toElement();
                                if (!date.isNull()) {
                                    changeElement->setDate(date.text());
                                }
                                KXmlElement creator = metadata.namedItem("dc:creator").toElement();
                                if (!date.isNull()) {
                                    changeElement->setCreator(creator.text());
                                }
                                //TODO load comments
/*                              KXmlElement extra = metadata.namedItem("dc-").toElement();
                                if (!date.isNull()) {
                                    kDebug() << "creator: " << creator.text();
                                    changeElement->setCreator(creator.text());
                                }*/
                            }
                            changeElement->setEnabled(d->recordChanges);
                            d->changes.insert( d->changeId, changeElement);
                            d->loadedChanges.insert(tag.attributeNS(KOdfXmlNS::text,"id"), d->changeId++);
                        }
                    }
                }
            }
        }
    } else {
        //This is the ODF 1.2 Change Format
        KXmlElement tag;
        forEachElement(tag, element) {
            if (! tag.isNull()) {
                const QString localName = tag.localName();
                if (localName == "change-transaction") {
                    KChangeTrackerElement *changeElement = 0;
                    //Set the change element as an insertion element for now
                    //Will be changed to the correct type when actual changes referencing this change-id are encountered
                    changeElement = new KChangeTrackerElement(tag.attributeNS(KOdfXmlNS::delta,"change-id"),KOdfGenericChange::InsertChange);
                    KXmlElement metadata = tag.namedItemNS(KOdfXmlNS::delta,"change-info").toElement();
                    if (!metadata.isNull()) {
                           KXmlElement date = metadata.namedItem("dc:date").toElement();
                           if (!date.isNull()) {
                                changeElement->setDate(date.text());
                            }
                            KXmlElement creator = metadata.namedItem("dc:creator").toElement();
                            if (!creator.isNull()) {
                                changeElement->setCreator(creator.text());
                            }
                    }
                    changeElement->setEnabled(d->recordChanges);
                    d->changes.insert( d->changeId, changeElement);
                    d->loadedChanges.insert(tag.attributeNS(KOdfXmlNS::delta,"change-id"), d->changeId++);
               }
           }
        }
    }
}

int KChangeTracker::loadedChangeId(QString odfId)
{
    return d->loadedChanges.value(odfId);
}

int KChangeTracker::deletedChanges(QVector<KChangeTrackerElement *> &deleteVector) const
{
    int numAppendedItems = 0;
    foreach (KChangeTrackerElement *element, d->changes.values()) {
        if (element->changeType() == KOdfGenericChange::DeleteChange && !element->acceptedRejected()) {
          deleteVector << element;
          numAppendedItems++;
        }
    }

    return numAppendedItems;
}

int KChangeTracker::allChangeIds(QVector<int> &changesVector) const
{
    int numAppendedItems = 0;
    foreach (int changeId, d->changes.keys()) {
        changesVector << changeId;
        numAppendedItems++;
    }

    return numAppendedItems;
}

QColor KChangeTracker::insertionBgColor() const
{
    return d->insertionBgColor;
}

QColor KChangeTracker::deletionBgColor() const
{
    return d->deletionBgColor;
}

QColor KChangeTracker::formatChangeBgColor() const
{
    return d->formatChangeBgColor;
}

void KChangeTracker::setInsertionBgColor(const QColor& bgColor)
{
    d->insertionBgColor = bgColor;
}

void KChangeTracker::setDeletionBgColor(const QColor& bgColor)
{
    d->deletionBgColor = bgColor;
}

void KChangeTracker::setFormatChangeBgColor(const QColor& bgColor)
{
    d->formatChangeBgColor = bgColor;
}

KDeletedRowColumnDataStore *KChangeTracker::deletedRowColumnData()
{
    return d->deletedRowColumnData;
}

//A convenience function to get a ListIdType from a format
static KListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KListStyle::ListIdType listId;

    if (sizeof(KListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KListStyle::ListId).toUInt();
    else
        listId = format.property(KListStyle::ListId).toULongLong();

    return listId;
}

QTextDocumentFragment KChangeTracker::generateDeleteFragment(QTextCursor &cursor, KDeleteChangeMarker *marker)
{
    int changeId = marker->changeId();
    QTextCursor editCursor(cursor);
    QTextDocument *document = cursor.document();

    QTextDocument deletedDocument;
    QTextDocument deleteCursor(&deletedDocument);

    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());

    for (int i = cursor.anchor();i <= cursor.position(); i++) {
        if (document->characterAt(i) == QChar::ObjectReplacementCharacter) {
            editCursor.setPosition(i+1);
            KDeleteChangeMarker *testMarker = dynamic_cast<KDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(editCursor));
            if (testMarker)
                editCursor.deletePreviousChar();
        }
    }

    QTextBlock currentBlock = document->findBlock(cursor.anchor());
    QTextBlock startBlock = currentBlock;
    QTextBlock endBlock = document->findBlock(cursor.position()).next();

    // First remove any left-over DeletedList set from previous deletes
    for (;currentBlock != endBlock; currentBlock = currentBlock.next()) {
        editCursor.setPosition(currentBlock.position());
        if (editCursor.currentList()) {
            if (editCursor.currentList()->format().hasProperty(KDeleteChangeMarker::DeletedList)) {
                QTextListFormat format = editCursor.currentList()->format();
                format.clearProperty(KDeleteChangeMarker::DeletedList);
                editCursor.currentList()->setFormat(format);
            }
        }
    }

    currentBlock = document->findBlock(cursor.anchor());
    startBlock = currentBlock;
    endBlock = document->findBlock(cursor.position()).next();

    for (;currentBlock != endBlock; currentBlock = currentBlock.next()) {
        editCursor.setPosition(currentBlock.position());
        if (editCursor.currentList()) {
            if (!editCursor.currentList()->format().hasProperty(KDeleteChangeMarker::DeletedList)) {
                bool fullyDeletedList = Private::checkListDeletion(editCursor.currentList(), cursor);
                QTextListFormat format = editCursor.currentList()->format();
                format.setProperty(KDeleteChangeMarker::DeletedList, fullyDeletedList);
                if (fullyDeletedList) {
                    KListStyle::ListIdType listId = ListId(format);
                    KoList *list = KoTextDocument(document).list(currentBlock);
                    marker->setDeletedListStyle(listId, list->style());
                }
                editCursor.currentList()->setFormat(format);
            }
            if (cursor.anchor() <= (currentBlock.position() - 1)) {
                //Then the list-item has been deleted. Set the block-format to indicate that this is a deleted list-item.
                QTextBlockFormat blockFormat;
                blockFormat.setProperty(KDeleteChangeMarker::DeletedListItem, true);
                editCursor.mergeBlockFormat(blockFormat);
            } else {
                QTextBlockFormat blockFormat;
                blockFormat.setProperty(KDeleteChangeMarker::DeletedListItem, false);
                editCursor.mergeBlockFormat(blockFormat);
            }
        }

        if (editCursor.currentTable()) {
            QTextTableFormat tableFormat = editCursor.currentTable()->format();
            tableFormat.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
            editCursor.currentTable()->setFormat(tableFormat);
        }

        if (currentBlock != startBlock) {
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(KCharacterStyle::ChangeTrackerId, changeId);
            editCursor.mergeBlockFormat(blockFormat);
        }
    }

    return cursor.selection();
}

bool KChangeTracker::Private::checkListDeletion(QTextList *list, const QTextCursor &cursor)
{
    int startOfList = (list->item(0).position() - 1);
    int endOfList = list->item(list->count() -1).position() + list->item(list->count() -1).length() - 1;
    if ((cursor.anchor() <= startOfList) && (cursor.position() >= endOfList))
        return true;
    else {
        /***************************************************************************************************/
        /*                                    Qt Quirk Work-Around                                         */
        /***************************************************************************************************/
        if ((cursor.anchor() == (startOfList + 1)) && (cursor.position() > endOfList)) {
            return true;
        /***************************************************************************************************/
        } else if((cursor.anchor() <= startOfList) && (list->count() == 1)) {
            return true;
        } else {
            return false;
        }
    }
}

void KChangeTracker::insertDeleteFragment(QTextCursor &cursor, KDeleteChangeMarker *marker)
{
    QTextDocumentFragment fragment =  KoTextDocument(cursor.document()).changeTracker()->elementById(marker->changeId())->deleteData();
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);

    bool deletedListItem = false;

    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        //This condition is for the work-around for a Qt behaviour
        //Even if a delete ends at the end of a table, the fragment will have an empty block after the table
        //If such a block is detected then, just ignore it
        if ((currentBlock.next() == tempDoc.end()) && (currentBlock.text().length() == 0) && (QTextCursor(currentBlock.previous()).currentTable())) {
            continue;
        }

        tempCursor.setPosition(currentBlock.position());
        QTextList *textList = tempCursor.currentList();
        int outlineLevel = currentBlock.blockFormat().property(KParagraphStyle::OutlineLevel).toInt();

        KoList *currentList = KoTextDocument(cursor.document()).list(cursor.block());
        int docOutlineLevel = cursor.block().blockFormat().property(KParagraphStyle::OutlineLevel).toInt();
        if (docOutlineLevel) {
            //Even though we got a list, it is actually a list for storing headings. So don't consider it
            currentList = NULL;
        }

        QTextList *previousTextList = currentBlock.previous().isValid() ? QTextCursor(currentBlock.previous()).currentList():NULL;
        if (textList && previousTextList && (textList != previousTextList) && (KoList::level(currentBlock) == KoList::level(currentBlock.previous()))) {
            //Even though we are already in a list, the QTextList* of the current block is differnt from that of the previous block
            //Also the levels of the list-items ( previous and current ) are the same.
            //This can happen only when two lists are merged together without any intermediate content.
            //So we need to create a new list.
            currentList = NULL;
        }

        if (textList) {
            if (textList->format().property(KDeleteChangeMarker::DeletedList).toBool() && !currentList) {
                //Found a Deleted List in the fragment. Create a new KoList.
                KListStyle::ListIdType listId;
                if (sizeof(KListStyle::ListIdType) == sizeof(uint))
                    listId = textList->format().property(KListStyle::ListId).toUInt();
                else
                    listId = textList->format().property(KListStyle::ListId).toULongLong();
                KListStyle *style = marker->deletedListStyle(listId);
                currentList = new KoList(cursor.document(), style);
            }

            deletedListItem = currentBlock.blockFormat().property(KDeleteChangeMarker::DeletedListItem).toBool();
            if (deletedListItem && currentBlock != tempDoc.begin()) {
                // Found a deleted list item in the fragment. So insert a new list-item
                int deletedListItemLevel = KoList::level(currentBlock);

                if (!(QTextCursor(currentBlock.previous()).currentTable())) {
                    cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
                } else {
                    cursor.mergeBlockFormat(currentBlock.blockFormat());
                }

                if(!currentList) {
                    if (!outlineLevel) {
                        //This happens when a part of a paragraph and a succeeding list-item are deleted together
                        //So go to the next block and insert it in the list there.
                        QTextCursor tmp(cursor);
                        tmp.setPosition(tmp.block().next().position());
                        currentList = KoTextDocument(tmp.document()).list(tmp.block());
                    } else {
                        // This is a heading. So find the KoList for heading and add the block there
                        KoList *headingList = KoTextDocument(cursor.document()).headingList();
                        currentList = headingList;
                    }
                }
                currentList->add(cursor.block(), deletedListItemLevel);
            }
        } else if (tempCursor.currentTable()) {
            QTextTable *deletedTable = tempCursor.currentTable();
            int numRows = deletedTable->rows();
            int numColumns = deletedTable->columns();
            QTextTable *insertedTable = cursor.insertTable(numRows, numColumns, deletedTable->format());
            for (int i=0; i<numRows; i++) {
                for (int j=0; j<numColumns; j++) {
                    tempCursor.setPosition(deletedTable->cellAt(i,j).firstCursorPosition().position());
                    tempCursor.setPosition(deletedTable->cellAt(i,j).lastCursorPosition().position(), QTextCursor::KeepAnchor);
                    insertedTable->cellAt(i,j).setFormat(deletedTable->cellAt(i,j).format().toTableCellFormat());
                    cursor.setPosition(insertedTable->cellAt(i,j).firstCursorPosition().position());
                    cursor.insertFragment(tempCursor.selection());
                }
            }
            tempCursor.setPosition(deletedTable->cellAt(numRows-1,numColumns-1).lastCursorPosition().position());
            currentBlock = tempCursor.block();
            //Move the cursor outside of table
            cursor.setPosition(cursor.position() + 1);
            continue;
        } else {
            // This block does not contain a list. So no special work here.
            if ((currentBlock != tempDoc.begin()) && !(QTextCursor(currentBlock.previous()).currentTable())) {
                cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
            }

            if (QTextCursor(currentBlock.previous()).currentTable()) {
                cursor.mergeBlockFormat(currentBlock.blockFormat());
            }
        }

        /********************************************************************************************************************/
        /*This section of code is a work-around for a bug in the Qt. This work-around is safe. If and when the bug is fixed */
        /*the if condition would never be true and the code would never get executed                                        */
        /********************************************************************************************************************/
        if ((KoList::level(cursor.block()) != KoList::level(currentBlock)) && currentBlock.text().length()) {
            if (!currentList) {
                QTextCursor tmp(cursor);
                tmp.setPosition(tmp.block().previous().position());
                currentList = KoTextDocument(tmp.document()).list(tmp.block());
            }
            currentList->add(cursor.block(), KoList::level(currentBlock));
        }
        /********************************************************************************************************************/

        // Finally insert all the contents of the block into the main document.
        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid()) {
                cursor.insertText(currentFragment.text(), currentFragment.charFormat());
            }
        }
    }
}

int KChangeTracker::fragmentLength(QTextDocumentFragment fragment)
{
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);
    int length = 0;
    bool deletedListItem = false;
    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        if (tempCursor.currentList()) {
            deletedListItem = currentBlock.blockFormat().property(KDeleteChangeMarker::DeletedListItem).toBool();
            if (currentBlock != tempDoc.begin() && deletedListItem)
                length += 1; //For the Block separator
        } else if (tempCursor.currentTable()) {
            QTextTable *deletedTable = tempCursor.currentTable();
            int numRows = deletedTable->rows();
            int numColumns = deletedTable->columns();
            for (int i=0; i<numRows; i++) {
                for (int j=0; j<numColumns; j++) {
                    length += 1;
                    length += (deletedTable->cellAt(i,j).lastCursorPosition().position() - deletedTable->cellAt(i,j).firstCursorPosition().position());
                }
            }
            tempCursor.setPosition(deletedTable->cellAt(numRows-1,numColumns-1).lastCursorPosition().position());
            currentBlock = tempCursor.block();
            length += 1;
            continue;
        } else {
            if ((currentBlock != tempDoc.begin()) && !(QTextCursor(currentBlock.previous()).currentTable()))
                length += 1; //For the Block Separator
        }


        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid())
                length += currentFragment.text().length();
        }
    }

    return length;
}

#include <KChangeTracker.moc>
