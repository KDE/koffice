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

#ifndef KC_MAP_MODEL
#define KC_MAP_MODEL

#include <QAbstractListModel>

class QUndoCommand;

class KCMap;
class KCSheet;

/**
 * A model for the 'embedded document'.
 * Actually, a model for a sheet list.
 * \ingroup Model
 */
class KCMapModel : public QAbstractListModel
{
    Q_OBJECT
public:
    KCMapModel(KCMap* map);
    virtual ~KCMapModel();

    // QAbstractItemModel interface
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public Q_SLOTS:
    /**
     * Hides \p sheet, if \p hidden is \c true and \p sheet is visible.
     * Shows \p sheet, if \p hidden is \c false and \p sheet is hidden.
     * \return \c true on success; \c false on failure
     */
    bool setHidden(KCSheet* sheet, bool hidden = true);

Q_SIGNALS:
    void addCommandRequested(QUndoCommand* command);

protected:
    KCMap* map() const;

protected Q_SLOTS:
    virtual void addSheet(KCSheet *sheet);
    virtual void removeSheet(KCSheet *sheet);

private:
    class Private;
    Private * const d;
};

#endif // KC_MAP_MODEL
