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

#include "DeleteCommand.h"
#include <klocale.h>
#include <TextTool.h>
#include <QUndoCommand>
#include <KoTextEditor.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextAnchor.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>

DeleteCommand::DeleteCommand(DeleteMode mode, TextTool *tool, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_tool(tool),
    m_first(true),
    m_undone(false),
    m_mode(mode)
{
    setText(i18n("Delete"));
}

void DeleteCommand::undo()
{
    foreach (QUndoCommand *command, m_shapeDeleteCommands)
        command->undo(); 

    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    updateListChanges();
    m_undone = true;
}

void DeleteCommand::redo()
{
    m_undone = false;
    if (!m_first) {
        foreach (QUndoCommand *command, m_shapeDeleteCommands)
            command->redo();        

        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
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

void DeleteCommand::deleteChar()
{
    QTextCursor *caret = m_tool->m_textEditor->cursor();

    if (caret->atEnd() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void DeleteCommand::deletePreviousChar()
{
    QTextCursor *caret = m_tool->m_textEditor->cursor();

    if (caret->atStart() && !caret->hasSelection())
        return;

    if (!caret->hasSelection())
        caret->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

    deleteSelection(*caret);
}

void DeleteCommand::deleteSelection(QTextCursor &selection)
{
    QTextCursor cursor(selection);

    //Store the position and length. Will be used in checkMerge
    m_position = (cursor.anchor() < cursor.position()) ? cursor.anchor():cursor.position();
    m_length = qAbs(cursor.anchor() - cursor.position());

    //Store the charFormat. If the selection has multiple charFormats set m_multipleFormatDeletion to true.Will be used in checkMerge
    QTextCharFormat currFormat;
    QTextCharFormat firstFormat;
    
    m_multipleFormatDeletion = false;

    for (int i = m_position; i < (m_position + m_length); i++) {
        cursor.setPosition(i+1);
        currFormat = cursor.charFormat();
        
        if ( i == m_position ) {
            firstFormat = currFormat;
            continue;
        }

        if ( currFormat != firstFormat) {
            m_multipleFormatDeletion = true;
            break;
        }
    }

    if (!m_multipleFormatDeletion)
        m_format = firstFormat;

    //Delete any inline objects present within the selection
    deleteInlineObjects(selection);
    
    //Now finally Delete the selected text
    selection.deleteChar();
}

void DeleteCommand::deleteInlineObjects(QTextCursor &selection)
{
    QTextCursor cursor(selection);
    QTextDocument *document = m_tool->m_textEditor->document();
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);

    KoInlineTextObjectManager *manager = layout->inlineTextObjectManager();
    KoInlineObject *object;

    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        cursor.setPosition(cursor.selectionStart() + 1);
        int position = cursor.position();
        const QChar *data = selected.constData();
        for (int i = 0; i < selected.length(); i++) {
            if (data->unicode() == QChar::ObjectReplacementCharacter) {
                cursor.setPosition(position);
                object = manager->inlineTextObject(cursor);
                deleteTextAnchor(object);
                m_invalidInlineObjects.append(object);
            } else {
                position++;
            }
            data++;
        }
    } else {
        if (!(m_mode == PreviousChar))
            cursor.movePosition(QTextCursor::Right);

        object = manager->inlineTextObject(cursor);
        deleteTextAnchor(object);
        m_invalidInlineObjects.append(object);
    }
}

void DeleteCommand::deleteTextAnchor(KoInlineObject *object)
{
    if (object) {
        KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
        if (anchor) {
                KoShape *shape = anchor->shape();
                QUndoCommand *shapeDeleteCommand = m_tool->canvas()->shapeController()->removeShape(shape);
                shapeDeleteCommand->redo();
                m_shapeDeleteCommands.push_back(shapeDeleteCommand);
        }
    }
}

int DeleteCommand::id() const
{
    // Should be an enum declared somewhere. TextCommandBase.h ???
    return 56789;
}

bool DeleteCommand::mergeWith( const QUndoCommand *command)
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

    if (!checkMerge(command))
        return false;

    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));
    
    m_shapeDeleteCommands += other->m_shapeDeleteCommands;
    other->m_shapeDeleteCommands.clear();

    m_invalidInlineObjects += other->m_invalidInlineObjects;
    other->m_invalidInlineObjects.clear();

    //Normally one would expect the code below to be present. But in this case we reach here only if checkMerge return true.
    //And checkMerge checks the same conditions that QTextDocument checks before it merges commands in its internal undo stack.
    //So for 4.6.0 and above the number of children would be 0. So the loop even if present would be useless
    //And for Qt 4.5.x because of a bug in Qt there will be a child present. But it should not be processed. So no loop needed.
    //for(int i=0; i < command->childCount(); i++)
    //    new UndoTextCommand(m_tool->m_textEditor->document(), this);

    return true;
}

bool DeleteCommand::checkMerge( const QUndoCommand *command )
{
    DeleteCommand *other = const_cast<DeleteCommand *>(static_cast<const DeleteCommand *>(command));
    
    if (m_multipleFormatDeletion || other->m_multipleFormatDeletion)
        return false;

    if (m_position == other->m_position
        && m_format == other->m_format) {
        m_length += other->m_length;
        return true;
    }

    
    if ( (other->m_position + other->m_length == m_position)
        && (m_format == other->m_format)) {
        m_position = other->m_position;
        m_length += other->m_length;
        return true;
    }
    return false;
}

void DeleteCommand::updateListChanges()
{
    QTextDocument *document = m_tool->m_textEditor->document();
    QTextCursor tempCursor(document);
    QTextBlock startBlock = document->findBlock(m_position);
    QTextBlock endBlock = document->findBlock(m_position + m_length);
    QTextList *currentList;

    for (QTextBlock currentBlock = startBlock; currentBlock != endBlock; currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        currentList = tempCursor.currentList();
        if (currentList) {
            KoListStyle::ListIdType listId;
            if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
                listId = currentList->format().property(KoListStyle::ListId).toUInt();
            else
                listId = currentList->format().property(KoListStyle::ListId).toULongLong();
            
            if (!KoTextDocument(document).list(currentBlock)) {
                KoList *list = KoTextDocument(document).list(listId);
                list->updateStoredList(currentBlock);
            }
        }
    }
}

DeleteCommand::~DeleteCommand()
{
    if (!m_undone) {
        foreach (KoInlineObject *object, m_invalidInlineObjects) {
            QTextDocument *document = m_tool->m_textEditor->document();
            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
            KoInlineTextObjectManager *manager = layout->inlineTextObjectManager();
            manager->removeInlineObject(object);
            delete object;
        }

        foreach (QUndoCommand *command, m_shapeDeleteCommands)
            delete command;
    }
}

