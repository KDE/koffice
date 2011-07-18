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

#ifndef STYLE_STORAGE_COMMAND_H
#define STYLE_STORAGE_COMMAND_H

// Qt
#include <QList>
#include <QPair>
#include <QUndoCommand>

// KCells
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
class StyleStorageCommand : public QUndoCommand
{
public:
    typedef QPair<QRectF, KCSharedSubStyle> Pair;

    explicit StyleStorageCommand(KCStyleStorage *storage, QUndoCommand *parent = 0);

    virtual void undo();

    void add(const QList<Pair> &pairs);

    StyleStorageCommand& operator<<(const Pair &pair);
    StyleStorageCommand& operator<<(const QList<Pair> &pairs);

private:
    KCStyleStorage *const m_storage;
    QList<Pair> m_undoData;
};

StyleStorageCommand::StyleStorageCommand(KCStyleStorage *storage, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_storage(storage)
{
}

void StyleStorageCommand::undo()
{
    for (int i = 0; i < m_undoData.count(); ++i) {
        m_storage->insert(m_undoData[i].first.toRect(), m_undoData[i].second);
    }
    QUndoCommand::undo(); // undo possible child commands
}

void StyleStorageCommand::add(const QList<Pair>& pairs)
{
    m_undoData << pairs;
}

StyleStorageCommand& StyleStorageCommand::operator<<(const Pair& pair)
{
    m_undoData << pair;
    return *this;
}

StyleStorageCommand& StyleStorageCommand::operator<<(const QList<Pair>& pairs)
{
    m_undoData << pairs;
    return *this;
}

#endif // STYLE_STORAGE_COMMAND_H
