/* This file is part of the KDE project
 * Copyright (C) 2008-2011 Thomas Zander <zander@kde.org>
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
#ifndef MODEL_H
#define MODEL_H

#include <QAbstractItemModel>
#include <QMultiHash>
#include <QIcon>

class KStyleManager;
class KParagraphStyle;
class KCharacterStyle;
class QSignalMapper;

class StylesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit StylesModel(KStyleManager *styleManager, QObject *parent = 0);
    ~StylesModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    KParagraphStyle *paragraphStyleForIndex(const QModelIndex &index) const;
    KCharacterStyle *characterStyleForIndex(const QModelIndex &index) const;

    void setStyleManager(KStyleManager *manager);

signals:
    /// emitted on recalculate
    /// @param multiLevel if true then the root nodes have children.
    void isMultiLevel(bool multiLevel);

public slots:
    /**
        Sets the paragraph style that is to be marked as the 'active' one.
        @param styleId the id from KParagraphStyle::styleId()
        @param unchanged if true the icon will display the paragraph style in the text has no local modifications.
        @return the QModelIndex of the new paragraph style
    */
    QModelIndex setCurrentParagraphStyle(int styleId, bool unchanged);
    /**
        Sets the character style that is to be marked as the 'active' one.
        @param styleId the id from KCharacterStyle::styleId()
        @param unchanged if true the icon will display the character style in the text has no local modifications.
    */
    void setCurrentCharacterStyle(int styleId, bool unchanged);

private slots:
    void addParagraphStyle(KParagraphStyle*, bool recalc = true);
    void addCharacterStyle(KCharacterStyle*, bool recalc = true);
    void removeParagraphStyle(KParagraphStyle*, bool recalc = true);
    void removeCharacterStyle(KCharacterStyle*, bool recalc = true);
    void updateName(int styleId);

protected:
    void recalculate();

    QList<int> m_styleList; // top level list of items
    QMultiHash<int, int> m_relations; // parent-child relations.

private:
    QModelIndex indexForStyle(int styleId);
    QModelIndex parent(int needle, const QList<int> &haystack) const;

    KStyleManager *m_styleManager;

    int m_currentParagraphStyle;
    int m_currentCharacterStyle;
    bool m_pureParagraphStyle;
    bool m_pureCharacterStyle;

    QIcon m_charIcon;

    QSignalMapper *m_styleMapper;
};

#endif
