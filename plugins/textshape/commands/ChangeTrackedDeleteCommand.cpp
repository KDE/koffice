/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
 * Boston, MA 02110-1301, USA.*/

#include "ChangeTrackedDeleteCommand.h"
#include <KoTextEditor.h>
#include <TextTool.h>
#include <klocale.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KAction>
#include <KoTextAnchor.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoList.h>
#include <KoParagraphStyle.h>
#include <QTextDocumentFragment>
#include <QUndoCommand>

#include <KDebug>
//#include <iostream>
#include <QDebug>

//A convenience function to get a ListIdType from a format

static KoListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KoListStyle::ListIdType listId;

    if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KoListStyle::ListId).toUInt();
    else
        listId = format.property(KoListStyle::ListId).toULongLong();

    return listId;
}

using namespace std;
ChangeTrackedDeleteCommand::ChangeTrackedDeleteCommand(DeleteMode mode, TextTool *tool, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_tool(tool),
    m_first(true),
    m_undone(false),
    m_mode(mode),
    m_removedElements()
{
      setText(i18n("Delete"));
}

void ChangeTrackedDeleteCommand::undo()
{
    foreach(QUndoCommand *shapeDeleteCommand, m_shapeDeleteCommands)
        shapeDeleteCommand->undo();

    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);

    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocument(document).changeTracker()->elementById(m_addedChangeElement)->setValid(false);
    foreach (int changeId, m_removedElements) {
      KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(true);
    }
    updateListChanges();
    m_undone = true;
}

void ChangeTrackedDeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        foreach(QUndoCommand *shapeDeleteCommand, m_shapeDeleteCommands)
            shapeDeleteCommand->redo();
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        QTextDocument *document = m_tool->m_textEditor->document();
        KoTextDocument(document).changeTracker()->elementById(m_addedChangeElement)->setValid(true);
        foreach (int changeId, m_removedElements) {
          KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
        }
    } else {
        m_first = false;
        m_tool->m_textEditor->beginEditBlock();
        if(m_mode == PreviousChar)
            deletePreviousChar();
        else
            deleteChar();
        m_tool->m_textEditor->endEditBlock();
    }
}

void ChangeTrackedDeleteCommand::deleteChar()
{
    QTextCursor *caret = m_tool->m_textEditor->cursor();

    if (caret->atEnd() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void ChangeTrackedDeleteCommand::deletePreviousChar()
{
    QTextCursor *caret = m_tool->m_textEditor->cursor();

    if (caret->atStart() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void ChangeTrackedDeleteCommand::deleteSelection(QTextCursor &selection)
{
    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

    QTextCursor checker = QTextCursor(selection);
    KoDeleteChangeMarker *deleteChangemarker = 0;
    KoDeleteChangeMarker *testMarker;

    bool backwards = (checker.anchor() > checker.position());
    int selectionBegin = qMin(checker.anchor(), checker.position());
    int selectionEnd = qMax(checker.anchor(), checker.position());
    int changeId;

    QList<KoShape *> shapesInSelection;

    checker.setPosition(selectionBegin);

    while ((checker.position() < selectionEnd) && (!checker.atEnd())) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QTextDocumentFragment inter = KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
                if (!KoTextDocument(document).changeTracker()->displayChanges()) {
                    insertDeleteFragment(checker, testMarker);
                    selectionEnd = selectionEnd + fragmentLength(inter);
                }
                checker.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, fragmentLength(inter));
                KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
           } else {
                KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(layout->inlineTextObjectManager()->inlineTextObject(checker));
                if (anchor)
                    shapesInSelection.push_back(anchor->shape());
           } 
        }
        checker.setPosition(checker.position());
    }

    checker.setPosition(selectionBegin);
    
    if (!KoTextDocument(document).changeTracker()->displayChanges()) {
        QChar charAtPos = document->characterAt(checker.position() - 1);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker)
                insertDeleteFragment(checker, testMarker);
        }
    }

    if (KoTextDocument(document).changeTracker()->containsInlineChanges(checker.charFormat())) {
        int changeId = checker.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        if (KoTextDocument(document).changeTracker()->elementById(changeId)->getChangeType() == KoGenChange::deleteChange) {
            QTextDocumentFragment prefix =  KoTextDocument(document).changeTracker()->elementById(changeId)->getDeleteData();
            selectionBegin -= (fragmentLength(prefix) + 1 );
            KoTextDocument(document).changeTracker()->elementById(changeId)->setValid(false);
            m_removedElements.push_back(changeId);
        }
    }

    checker.setPosition(selectionEnd);
    if (!checker.atEnd()) {
        QChar charAtPos = document->characterAt(checker.position());
        checker.movePosition(QTextCursor::NextCharacter);
        if (layout->inlineTextObjectManager()->inlineTextObject(checker) && charAtPos == QChar::ObjectReplacementCharacter) {
            testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(checker));
            if (testMarker) {
                QTextDocumentFragment sufix =  KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->getDeleteData();
                if (!KoTextDocument(document).changeTracker()->displayChanges())
                    insertDeleteFragment(checker, testMarker);
                selectionEnd += fragmentLength(sufix) + 1;
                KoTextDocument(document).changeTracker()->elementById(testMarker->changeId())->setValid(false);
                m_removedElements.push_back(testMarker->changeId());
            }
        }
    }

    selection.setPosition(selectionBegin);
    selection.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    QTextDocumentFragment deletedFragment;
    changeId = KoTextDocument(document).changeTracker()->getDeleteChangeId(i18n("Delete"), deletedFragment, selection.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
    KoChangeTrackerElement *element = KoTextDocument(document).changeTracker()->elementById(changeId);
    deleteChangemarker = new KoDeleteChangeMarker(KoTextDocument(document).changeTracker());
    deleteChangemarker->setChangeId(changeId);
    element->setDeleteChangeMarker(deleteChangemarker);
    deletedFragment = generateDeleteFragment(selection, deleteChangemarker);
    element->setDeleteData(deletedFragment);

    //Store the position and length. Will be used in updateListChanges()
    m_position = (selection.anchor() < selection.position()) ? selection.anchor():selection.position();
    m_length = qAbs(selection.anchor() - selection.position());

    updateListIds(selection);
    layout->inlineTextObjectManager()->insertInlineObject(selection, deleteChangemarker);

    m_addedChangeElement = changeId;
    
    //Insert the deleted data again after the marker with the charformat set to the change-id
    if (KoTextDocument(document).changeTracker()->displayChanges()) {
        int startPosition = selection.position();
        insertDeleteFragment(selection, deleteChangemarker);
        QTextCursor tempCursor(selection);
        tempCursor.setPosition(startPosition);
        tempCursor.setPosition(selection.position(), QTextCursor::KeepAnchor);
        //tempCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, fragmentLength(deletedFragment));
        updateListIds(tempCursor);
        if (backwards)
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, fragmentLength(deletedFragment) + 1);
    } else {
        if (backwards)
            selection.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor,1);

        foreach (KoShape *shape, shapesInSelection) {
            QUndoCommand *shapeDeleteCommand = m_tool->canvas()->shapeController()->removeShape(shape);
            m_shapeDeleteCommands.push_back(shapeDeleteCommand);
            shapeDeleteCommand->redo();
        }
    }
}


QTextDocumentFragment ChangeTrackedDeleteCommand::generateDeleteFragment(QTextCursor &cursor, KoDeleteChangeMarker *marker)
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
            KoDeleteChangeMarker *testMarker = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(editCursor));
            if (testMarker)
                editCursor.deletePreviousChar();
        }
    }

    QTextCharFormat format;
    format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    cursor.mergeCharFormat(format);

    QTextBlock currentBlock = document->findBlock(cursor.anchor());
    QTextBlock endBlock = document->findBlock(cursor.position()).next();

    for (;currentBlock != endBlock; currentBlock = currentBlock.next()) {
        editCursor.setPosition(currentBlock.position());
        if (editCursor.currentList()) {
            if (!editCursor.currentList()->format().hasProperty(KoDeleteChangeMarker::DeletedList)) {
                bool fullyDeletedList = checkListDeletion(editCursor.currentList(), cursor);
                QTextListFormat format = editCursor.currentList()->format();
                format.setProperty(KoDeleteChangeMarker::DeletedList, fullyDeletedList);
                if (fullyDeletedList) {
                    KoListStyle::ListIdType listId = ListId(format);
                    KoList *list = KoTextDocument(document).list(currentBlock);
                    marker->setDeletedListStyle(listId, list->style());
                }
                editCursor.currentList()->setFormat(format);
            }
            if (cursor.anchor() <= (currentBlock.position() - 1)) {
                //Then the list-item has been deleted. Set the block-format to indicate that this is a deleted list-item.
                QTextBlockFormat blockFormat;
                blockFormat.setProperty(KoDeleteChangeMarker::DeletedListItem, true);
                editCursor.mergeBlockFormat(blockFormat);
            } else {
                QTextBlockFormat blockFormat;
                blockFormat.setProperty(KoDeleteChangeMarker::DeletedListItem, false);
                editCursor.mergeBlockFormat(blockFormat);
            }
        }
    }
    
    return cursor.selection();
}

bool ChangeTrackedDeleteCommand::checkListDeletion(QTextList *list, QTextCursor &cursor)
{
    int startOfList = (list->item(0).position() - 1);
    int endOfList = list->item(list->count() -1).position() + list->item(list->count() -1).length() - 1;
    if ((cursor.anchor() <= startOfList) && (cursor.position() >= endOfList))
        return true;
    else
        return false;
} 

void ChangeTrackedDeleteCommand::insertDeleteFragment(QTextCursor &cursor, KoDeleteChangeMarker *marker)
{
    QTextDocumentFragment fragment =  KoTextDocument(cursor.document()).changeTracker()->elementById(marker->changeId())->getDeleteData();
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);

    bool deletedListItem = false;
    
    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        QTextList *textList = tempCursor.currentList();
        KoList *currentList = KoTextDocument(cursor.document()).list(cursor.block());

        if (textList) {
            if (textList->format().property(KoDeleteChangeMarker::DeletedList).toBool() && !currentList) {
                //Found a Deleted List in the fragment. Create a new KoList.
                KoListStyle::ListIdType listId = ListId(textList->format());
                KoListStyle *style = marker->getDeletedListStyle(listId);
                currentList = new KoList(cursor.document(), style);    
            }
            
            deletedListItem = currentBlock.blockFormat().property(KoDeleteChangeMarker::DeletedListItem).toBool();
            if (deletedListItem && currentBlock != tempDoc.begin()) {
                // Found a deleted list item in the fragment. So insert a new list-item
                int deletedListItemLevel = KoList::level(currentBlock);
                cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
                if(!currentList) {
                    //This happens when a part of a paragraph and a succeeding list-item are deleted together
                    //So go to the next block and insert it in the list there.
                    QTextCursor tmp(cursor);
                    tmp.setPosition(tmp.block().next().position());
                    currentList = KoTextDocument(tmp.document()).list(tmp.block());
                } 
                currentList->add(cursor.block(), deletedListItemLevel);
            }
        } else {
            // This block does not contain a list. So no special work here. 
            if (currentBlock != tempDoc.begin())
                cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
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
            if (currentFragment.isValid())
                cursor.insertText(currentFragment.text(), currentFragment.charFormat());
        }
        
    }
}

int ChangeTrackedDeleteCommand::fragmentLength(QTextDocumentFragment &fragment)
{
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);
    int length = 0;
    bool deletedListItem = false;
    
    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        if (tempCursor.currentList()) {
            deletedListItem = currentBlock.blockFormat().property(KoDeleteChangeMarker::DeletedListItem).toBool();
            if (currentBlock != tempDoc.begin() && deletedListItem)
                length += 1; //For the Block separator
        } else {
            if (currentBlock != tempDoc.begin())
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

int ChangeTrackedDeleteCommand::id() const
{
    return 98765;
}

bool ChangeTrackedDeleteCommand::mergeWith( const QUndoCommand *command)
{
    class UndoTextCommand : public QUndoCommand
    {
    public:
        UndoTextCommand(QTextDocument *document, QUndoCommand *parent = 0)
        : QUndoCommand(i18n("Text"), parent),
        m_document(document)
        {}

        void undo() {
            if (m_document.isNull())
                return;
            m_document->undo(KoTextDocument(m_document).textEditor()->cursor());
        }

        void redo() {
            if (m_document.isNull())
                return;
            m_document->redo(KoTextDocument(m_document).textEditor()->cursor());
        }

        QPointer<QTextDocument> m_document;
    };

    if (command->id() != id())
        return false;

    ChangeTrackedDeleteCommand *other = const_cast<ChangeTrackedDeleteCommand *>(static_cast<const ChangeTrackedDeleteCommand *>(command));
    if (other->m_removedElements.contains(m_addedChangeElement)) {
        removeChangeElement(m_addedChangeElement);
        other->m_removedElements.removeAll(m_addedChangeElement);
        m_addedChangeElement = other->m_addedChangeElement;

        m_removedElements += other->m_removedElements;
        other->m_removedElements.clear();

        m_shapeDeleteCommands += other->m_shapeDeleteCommands;
        other->m_shapeDeleteCommands.clear();

        m_newListIds = other->m_newListIds;

        m_position = other->m_position;
        m_length = other->m_length;

        for(int i=0; i < command->childCount(); i++)
            new UndoTextCommand(m_tool->m_textEditor->document(), this);

        return true;
    }
    return false;
}

void ChangeTrackedDeleteCommand::updateListIds(QTextCursor &cursor)
{
    m_newListIds.clear();
    QTextDocument *document = m_tool->m_textEditor->document();
    QTextCursor tempCursor(document);
    QTextBlock startBlock = document->findBlock(cursor.anchor());
    QTextBlock endBlock = document->findBlock(cursor.position());
    QTextList *currentList;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock.next(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KoListStyle::ListIdType listId = ListId(currentList->format());
            m_newListIds.push_back(listId);
        }
    }
}
void ChangeTrackedDeleteCommand::updateListChanges()
{
    QTextDocument *document = m_tool->m_textEditor->document();
    QTextCursor tempCursor(document);
    QTextBlock startBlock = document->findBlock(m_position);
    QTextBlock endBlock = document->findBlock(m_position + m_length);
    QTextList *currentList;
    int newListIdsCounter = 0;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock.next(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KoListStyle::ListIdType listId = m_newListIds[newListIdsCounter];
            if (!KoTextDocument(document).list(currentBlock)) {
                KoList *list = KoTextDocument(document).list(listId);
                list->updateStoredList(currentBlock);
            }
            newListIdsCounter++;
        }
    }
}

ChangeTrackedDeleteCommand::~ChangeTrackedDeleteCommand()
{
    if (m_undone) {
        removeChangeElement(m_addedChangeElement);
    } else {
        foreach (int changeId, m_removedElements) {
           removeChangeElement(changeId);
        }
        
        foreach(QUndoCommand *shapeDeleteCommand, m_shapeDeleteCommands)
            delete shapeDeleteCommand;
    }
}

void ChangeTrackedDeleteCommand::removeChangeElement(int changeId)
{
    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    KoChangeTrackerElement *element = KoTextDocument(document).changeTracker()->elementById(changeId);
    KoDeleteChangeMarker *marker = element->getDeleteChangeMarker();
    layout->inlineTextObjectManager()->removeInlineObject(marker);
    KoTextDocument(document).changeTracker()->removeById(changeId);
}
