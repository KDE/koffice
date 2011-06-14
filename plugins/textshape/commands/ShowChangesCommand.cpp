/*
 * This file is part of the KDE project
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
 * Boston, MA 02110-1301, USA.
*/
#include <iostream>
#include "ShowChangesCommand.h"

#include <KChangeTracker.h>
#include <KChangeTrackerElement.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextEditor.h>
#include <KoTextAnchor.h>
#include <KInlineTextObjectManager.h>
#include <KCanvasBase.h>
#include <KoShapeController.h>
#include <KShapeContainer.h>
#include <KDeleteChangeMarker.h>

#include <KAction>
#include <klocale.h>

#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QtAlgorithms>
#include <QList>

ShowChangesCommand::ShowChangesCommand(bool showChanges, QTextDocument *document, KCanvasBase *canvas, QUndoCommand *parent) :
    TextCommandBase (parent),
    m_document(document),
    m_first(true),
    m_showChanges(showChanges),
    m_canvas(canvas)
{
    Q_ASSERT(document);
    m_changeTracker = KoTextDocument(m_document).changeTracker();
    m_textEditor = KoTextDocument(m_document).textEditor();
    if (showChanges)
      setText(i18n("Show Changes"));
    else
      setText(i18n("Hide Changes"));
}

void ShowChangesCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    foreach (QUndoCommand *shapeCommand, m_shapeCommands)
        shapeCommand->undo();
    emit toggledShowChange(!m_showChanges);
    enableDisableStates(!m_showChanges);
}

void ShowChangesCommand::redo()
{
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        foreach (QUndoCommand *shapeCommand, m_shapeCommands)
            shapeCommand->redo();
        emit toggledShowChange(m_showChanges);
        enableDisableStates(m_showChanges);
    } else {
        m_first = false;
        enableDisableChanges();
    }
}

void ShowChangesCommand::enableDisableChanges()
{
    if (m_changeTracker) {
        enableDisableStates(m_showChanges);

        if(m_showChanges)
          insertDeletedChanges();
        else
          removeDeletedChanges();

        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
        if (lay)
          lay->scheduleLayout();
    }
}

void ShowChangesCommand::enableDisableStates(bool showChanges)
{
    m_changeTracker->setDisplayChanges(showChanges);

    QTextCharFormat format = m_textEditor->charFormat();
    format.clearProperty(KCharacterStyle::ChangeTrackerId);
    m_textEditor->setCharFormat(format);
}

bool isPositionLessThan(KChangeTrackerElement *element1, KChangeTrackerElement *element2)
{
    return element1->deleteChangeMarker()->position() < element2->deleteChangeMarker()->position();
}

void ShowChangesCommand::insertDeletedChanges()
{
    int numAddedChars = 0;
    QVector<KChangeTrackerElement *> elementVector;
    KoTextDocument(m_textEditor->document()).changeTracker()->deletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach (KChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->deleteChangeMarker()) {
            QTextCursor caret(element->deleteChangeMarker()->document());
            caret.setPosition(element->deleteChangeMarker()->position() + numAddedChars +  1);
            QTextCharFormat f = caret.charFormat();
            f.setProperty(KCharacterStyle::ChangeTrackerId, element->deleteChangeMarker()->changeId());
            f.clearProperty(KCharacterStyle::InlineInstanceId);
            caret.setCharFormat(f);
            int insertPosition = caret.position();
            KChangeTracker::insertDeleteFragment(caret, element->deleteChangeMarker());
            checkAndAddAnchoredShapes(insertPosition, KChangeTracker::fragmentLength(element->deleteData()));
            numAddedChars += KChangeTracker::fragmentLength(element->deleteData());
        }
    }
}

void ShowChangesCommand::checkAndAddAnchoredShapes(int position, int length)
{
    QTextCursor cursor(m_textEditor->document());
    for (int i=position;i < (position + length);i++) {
        if (m_textEditor->document()->characterAt(i) == QChar::ObjectReplacementCharacter) {
            cursor.setPosition(i+1);
            KInlineObject *object = KoTextDocument(m_textEditor->document()).inlineTextObjectManager()->inlineTextObject(cursor);
            if (!object)
                continue;

            KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
            if (!anchor)
                continue;
           
            KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
            KShapeContainer *container = dynamic_cast<KShapeContainer *>(lay->shapeForPosition(i));
            
            // a very ugly hack. Since this class is going away soon, it should be okay
            if (!container)
                container = dynamic_cast<KShapeContainer *>((lay->shapes()).at(0));

            if (container) {
                container->addShape(anchor->shape());
                QUndoCommand *shapeCommand = m_canvas->shapeController()->addShape(anchor->shape());
                shapeCommand->redo();
                m_shapeCommands.push_front(shapeCommand);
            }
        }
    }
}

void ShowChangesCommand::removeDeletedChanges()
{
    int numDeletedChars = 0;
    QVector<KChangeTrackerElement *> elementVector;
    m_changeTracker->deletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach(KChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->deleteChangeMarker()) {
            QTextCursor caret(element->deleteChangeMarker()->document());
            QTextCharFormat f;
            int deletePosition = element->deleteChangeMarker()->position() + 1 - numDeletedChars;
            caret.setPosition(deletePosition);
            int deletedLength = KChangeTracker::fragmentLength(element->deleteData());
            caret.setPosition(deletePosition + deletedLength, QTextCursor::KeepAnchor);
            checkAndRemoveAnchoredShapes(deletePosition, KChangeTracker::fragmentLength(element->deleteData()));
            caret.removeSelectedText();
            numDeletedChars += KChangeTracker::fragmentLength(element->deleteData());
        }
    }
}

void ShowChangesCommand::checkAndRemoveAnchoredShapes(int position, int length)
{
    QTextCursor cursor(m_textEditor->document());
    for (int i=position;i < (position + length);i++) {
        if (m_textEditor->document()->characterAt(i) == QChar::ObjectReplacementCharacter) {
            cursor.setPosition(i+1);
            KInlineObject *object = KoTextDocument(m_textEditor->document()).inlineTextObjectManager()->inlineTextObject(cursor);
            if (!object)
                continue;

            KoTextAnchor *anchor = dynamic_cast<KoTextAnchor *>(object);
            if (!anchor)
                continue;
            
            QUndoCommand *shapeCommand = m_canvas->shapeController()->removeShape(anchor->shape());
            shapeCommand->redo();
            m_shapeCommands.push_front(shapeCommand);
        }
    }
}

ShowChangesCommand::~ShowChangesCommand()
{
}

#include <ShowChangesCommand.moc>
