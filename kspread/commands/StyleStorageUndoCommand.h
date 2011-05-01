/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_STYLE_STORAGE_UNDO_COMMAND
#define KSPREAD_STYLE_STORAGE_UNDO_COMMAND

// Qt
#include <QList>
#include <QPair>
#include <QUndoCommand>

// KSpread
#include "KCStyleStorage.h"


/**
 * \ingroup Commands
 * \brief An undo command for KCStyleStorage data.
 *
 * Implements undo functionality only. Glue it to another command,
 * that provides the appropriate applying (redoing).
 *
 * Used for recording undo data in KCCellStorage.
 */
class StyleStorageUndoCommand : public QUndoCommand
{
public:
    typedef QPair<QRectF, SharedSubStyle> Pair;

    explicit StyleStorageUndoCommand(KCStyleStorage *storage, QUndoCommand *parent = 0);

    virtual void undo();

    void add(const QList<Pair> &pairs);

    StyleStorageUndoCommand& operator<<(const Pair &pair);
    StyleStorageUndoCommand& operator<<(const QList<Pair> &pairs);

private:
    KCStyleStorage *const m_storage;
    QList<Pair> m_undoData;
};

StyleStorageUndoCommand::StyleStorageUndoCommand(KCStyleStorage *storage, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_storage(storage)
{
}

void StyleStorageUndoCommand::undo()
{
    for (int i = 0; i < m_undoData.count(); ++i) {
        m_storage->insert(m_undoData[i].first.toRect(), m_undoData[i].second);
    }
    QUndoCommand::undo(); // undo possible child commands
}

void StyleStorageUndoCommand::add(const QList<Pair>& pairs)
{
    m_undoData << pairs;
}

StyleStorageUndoCommand& StyleStorageUndoCommand::operator<<(const Pair& pair)
{
    m_undoData << pair;
    return *this;
}

StyleStorageUndoCommand& StyleStorageUndoCommand::operator<<(const QList<Pair>& pairs)
{
    m_undoData << pairs;
    return *this;
}

#endif // KSPREAD_STYLE_STORAGE_UNDO_COMMAND
