/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#ifndef CHANGETRACKEDDELETECOMMAND_H
#define CHANGETRACKEDDELETECOMMAND_H

#include <QUndoStack>
#include "TextCommandBase.h"
#include <KoListStyle.h>
#include <QList>
#include <QTextList>

class TextTool;
class QTextDocument;
class QTextCursor;
class QTextDocumentFragment;
class KoChangeTrackerElement;
class KoDeleteChangeMarker;
class KoInlineTextObjectManager;

class ChangeTrackedDeleteCommand : public TextCommandBase
{
public:
    enum DeleteMode {
        PreviousChar,
        NextChar
    };

    ChangeTrackedDeleteCommand(DeleteMode mode, TextTool *tool, QUndoCommand* parent = 0);
    ~ChangeTrackedDeleteCommand();

    virtual void undo();
    virtual void redo();

    virtual int id() const;
    virtual bool mergeWith ( const QUndoCommand *command);

private:
    TextTool *m_tool;
    bool m_first;
    bool m_undone;
    bool m_canMerge;
    DeleteMode m_mode;
    QList<int> m_removedElements;
    QList<KoListStyle::ListIdType> m_newListIds;
    int m_position, m_length;
    int m_addedChangeElement;

    virtual void deleteChar();
    virtual void deletePreviousChar();
    virtual void deleteSelection(QTextCursor &selection);
    virtual void removeChangeElement(int changeId);
    virtual void insertDeleteFragment(QTextCursor &cursor, KoDeleteChangeMarker *marker);
    virtual int fragmentLength(QTextDocumentFragment &fragment);
    virtual void updateListIds(QTextCursor &cursor);
    virtual void updateListChanges();
    virtual void handleListItemDelete(QTextCursor &cursor);
};

#endif // CHANGETRACKEDDELTECOMMAND_H
