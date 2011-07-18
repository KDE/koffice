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

#ifndef KC_RECT_STORAGE_UNDO_COMMAND
#define KC_RECT_STORAGE_UNDO_COMMAND

// Qt
#include <QList>
#include <QPair>
#include <QUndoCommand>

// KCells
#include "ModelSupport.h"
#include "KCSheetModel.h"

/**
 * \ingroup Commands
 * \brief An undo command for KCRectStorage data.
 *
 * Implements undo functionality only. Glue it to another command,
 * that provides the appropriate applying (redoing).
 *
 * Used for recording undo data in KCCellStorage.
 */
template<typename T>
class KCRectStorageUndoCommand : public QUndoCommand
{
public:
    typedef QPair<QRectF, T> Pair;

    KCRectStorageUndoCommand(QAbstractItemModel *const model, int role, QUndoCommand *parent = 0);

    virtual void undo();

    void add(const QList<Pair> &pairs);

    KCRectStorageUndoCommand& operator<<(const Pair &pair);
    KCRectStorageUndoCommand& operator<<(const QList<Pair> &pairs);

private:
    QAbstractItemModel *const m_model;
    const int m_role;
    QList<Pair> m_undoData;
};

template<typename T>
KCRectStorageUndoCommand<T>::KCRectStorageUndoCommand(QAbstractItemModel *const model,
        int role, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_model(model)
        , m_role(role)
{
}

template<typename T>
void KCRectStorageUndoCommand<T>::undo()
{
    KCSheetModel *const model = static_cast<KCSheetModel*>(m_model);
    for (int i = 0; i < m_undoData.count(); ++i) {
        QVariant data;
        data.setValue(m_undoData[i].second);
        model->setData(fromRange(m_undoData[i].first.toRect(), model), data, m_role);
    }
    QUndoCommand::undo(); // undo possible child commands
}

template<typename T>
void KCRectStorageUndoCommand<T>::add(const QList<Pair>& pairs)
{
    m_undoData << pairs;
}

template<typename T>
KCRectStorageUndoCommand<T>& KCRectStorageUndoCommand<T>::operator<<(const Pair& pair)
{
    m_undoData << pair;
    return *this;
}

template<typename T>
KCRectStorageUndoCommand<T>& KCRectStorageUndoCommand<T>::operator<<(const QList<Pair>& pairs)
{
    m_undoData << pairs;
    return *this;
}

#endif // KC_RECT_STORAGE_UNDO_COMMAND
