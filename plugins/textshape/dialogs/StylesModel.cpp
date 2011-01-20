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
#include "StylesModel.h"
#include <QSet>
#include <QDebug>
#include <QSignalMapper>

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

#include <KIcon>

StylesModel::StylesModel(KoStyleManager *manager, QObject *parent)
        : QAbstractItemModel(parent),
        m_styleManager(0),
        m_currentParagraphStyle(0),
        m_currentCharacterStyle(0),
        m_pureParagraphStyle(true),
        m_pureCharacterStyle(true),
        m_styleMapper(new QSignalMapper(this))
{
    setStyleManager(manager);
    m_paragIcon = KIcon("kotext-paragraph");
    m_charIcon = KIcon("kotext-character");
    connect(m_styleMapper, SIGNAL(mapped(int)), this, SLOT(updateName(int)));
}

StylesModel::~StylesModel()
{
}

void StylesModel::recalculate()
{
    m_relations.clear();
    if (m_styleManager == 0) {
        m_styleList.clear();
        reset();
        return;
    }

    /*
        In the model we need two-way iteration. In the styles we already have
        a way to traverse up the tree using the paragraphStyle::parentStyle()
        This method creates a tree of styles and stores the info to traverse
        down the tree in m_relations.
    */

    QSet<int> paragraphStyles; // found paragraphStyles
    QSet<int> characterStyles; // found characterStyles
    QSet<int> treeRoot;

    foreach (KoParagraphStyle *style, m_styleManager->paragraphStyles()) {
        if (style->parentStyle() == 0) {
            treeRoot.insert(style->styleId());
            continue;
        }
        Q_ASSERT(style->parentStyle());
        while (style->parentStyle()) {
            characterStyles << style->characterStyle()->styleId();

            // store the relation style->parentStyle() <-> style
            const int key = style->parentStyle()->styleId();
            const int value = style->styleId();

            if (paragraphStyles.contains(value)) // relationship already present
                break;

            // the multiHash has the nasty habit or returning an inverted list, so lets 'sort in' by inserting them again
            QList<int> prevValues = m_relations.values(key);
            if (!prevValues.contains(value)) {
                m_relations.remove(key);
                m_relations.insert(key, value);
                while (!prevValues.isEmpty())
                    m_relations.insert(key, prevValues.takeLast());
            }

            style = style->parentStyle();
        }
    }

    QList<int> newStyleList;
    if (treeRoot.count() == 1 && (*treeRoot.constBegin()) == 100) { // the default style
        newStyleList = m_relations.values(100);
    } else { // a real list
        newStyleList = treeRoot.toList(); // TODO sort alphabetically on style name?
    }

    foreach (KoCharacterStyle *style, m_styleManager->characterStyles()) {
        const int key = style->styleId();
        if (key != 101 && !characterStyles.contains(key)) // 101 is default style.
            newStyleList << style->styleId();
    }

    int firstChangedRow = -1;
    int index = 0;
    foreach (int rootId, newStyleList) {
        if (index >= m_styleList.count()) {
            if (firstChangedRow == -1)
                firstChangedRow = index;
            break;
        }
        if (m_styleList[index] != rootId) {
            firstChangedRow = index;
            break;
        }
    }

    if (m_styleList.count() == newStyleList.count()) {
        int maxRow = qMax(m_styleList.count(), newStyleList.count()) - 1;
        m_styleList = newStyleList;
        emit dataChanged(createIndex(firstChangedRow, 0, 0), createIndex(maxRow, 1, 0));
    } else {
        m_styleList = newStyleList;
        layoutChanged();
    }
}

QModelIndex StylesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || row < 0 || column > 1)
        return QModelIndex();

    if (! parent.isValid()) {
        if (row >= m_styleList.count())
            return QModelIndex();
        return createIndex(row, column, m_styleList[row]);
    }
    int id = (int) parent.internalId();
    KoParagraphStyle *pstyle = m_styleManager->paragraphStyle(id);
    if (pstyle == 0) { // that means it has to be a charStyle. Thats easy, there is no hierarchy there.
        if (row >= m_styleList.count())
            return QModelIndex();
        return createIndex(row, column, m_styleList[row]);
    }

    if (row == 0) // return child char style
        return createIndex(row, column, pstyle->characterStyle()->styleId());
    if (m_relations.contains(id)) {
        QList<int> children = m_relations.values(id);
        if (row > children.count())
            return QModelIndex();
        return createIndex(row, column, children[row-1]);
    }
    return QModelIndex();
}

QModelIndex StylesModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        int id = (int) child.internalId();
        if (m_styleList.contains(id)) // is the root, parent is invalid.
            return QModelIndex();
        KoParagraphStyle *childStyle = m_styleManager->paragraphStyle(id);
        if (childStyle && childStyle->parentStyle())
            createIndex(0, 0, childStyle->parentStyle()->styleId());

        return parent(id, m_styleList);
    }
    return QModelIndex();
}

QModelIndex StylesModel::parent(int needle, const QList<int> &haystack) const
{
    Q_ASSERT(haystack.count());
    int row = -1;
    foreach (int id, haystack) {
        row++;
        KoParagraphStyle *style = m_styleManager->paragraphStyle(id);
        if (style == 0)
            continue;
        if (style->characterStyle()->styleId() == needle) // found it!
            return createIndex(row, 0, style->styleId());
        QList<int> children = m_relations.values(id);
        if (children.isEmpty())
            continue;
        int index = children.indexOf(needle);
        if (index >= 0)
            return createIndex(row, 0, id);
        children.insert(0, style->characterStyle()->styleId());
        QModelIndex mi = parent(needle, children);
        if (mi.isValid())
            return mi;
    }

    return QModelIndex();
}

int StylesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_styleList.count();
    if (parent.column() == 1)
        return 0;
    int id = (int) parent.internalId();
    const bool isParagStyle = m_styleManager->paragraphStyle(id) != 0;
    if (isParagStyle)
        return m_relations.values(id).count() + 1;
    return 0;
}

int StylesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant StylesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int id = (int) index.internalId();
    switch (role) {
    case Qt::DisplayRole: {
        if (index.column() == 1) {
            if (id == m_currentParagraphStyle) {
                if (m_pureParagraphStyle)
                    return QString(QChar(0x25CF));
                return QString(QChar(0x25D0));
            }
            if (id == m_currentCharacterStyle) {
                if (m_pureCharacterStyle)
                    return QString(QChar(0x25CF));
                return QString(QChar(0x25D0));
            }
            return QString(QChar(0x25CC));
        }
        KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
        if (paragStyle)
            return paragStyle->name();
        KoCharacterStyle *characterStyle =  m_styleManager->characterStyle(id);
        if (characterStyle)
            return characterStyle->name();
        break;
    }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (m_styleManager->paragraphStyle(id))
                return m_paragIcon;
            return m_charIcon;
        }
        break;
    default: break;
    };
    return QVariant();
}

bool StylesModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

Qt::ItemFlags StylesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return (Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void StylesModel::setCurrentParagraphStyle(int styleId, bool unchanged)
{
    if (m_currentParagraphStyle == styleId && unchanged == m_pureParagraphStyle)
        return;

    for (int i = 0; i < 2; ++i) {
        foreach (int parent, m_relations.keys()) {
            int index = m_relations.values(parent).indexOf(m_currentParagraphStyle);
            if (index >= 0) {
                QModelIndex mi = createIndex(index + 1, 1, m_currentParagraphStyle);
                emit dataChanged(mi, mi);
                break;
            }
        }

        m_currentParagraphStyle = styleId;
        m_pureParagraphStyle = unchanged;
    }
}

void StylesModel::setCurrentCharacterStyle(int styleId, bool unchanged)
{
    if (m_currentCharacterStyle == styleId && unchanged == m_pureCharacterStyle)
        return;
    QModelIndex prev = createIndex(0, 1, m_currentCharacterStyle);
    m_currentCharacterStyle = styleId;
    m_pureCharacterStyle = unchanged;
    if (prev.isValid())
        emit dataChanged(prev, prev);
    QModelIndex newCurrent = createIndex(0, 1, styleId);
    emit dataChanged(newCurrent, newCurrent);
}

KoParagraphStyle *StylesModel::paragraphStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->paragraphStyle(index.internalId());
}

KoCharacterStyle *StylesModel::characterStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->characterStyle(index.internalId());
}

void StylesModel::setStyleManager(KoStyleManager *sm)
{
    if (sm == m_styleManager)
        return;
    if (m_styleManager) {
        disconnect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    }
    m_styleManager = sm;

    if (m_styleManager == 0) {
        recalculate();
        return;
    }

    connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));

    foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles())
        addParagraphStyle(style, false);
    foreach(KoCharacterStyle *style, m_styleManager->characterStyles())
        addCharacterStyle(style, false);

    recalculate();
}

// called when the stylemanager adds a style
void StylesModel::addParagraphStyle(KoParagraphStyle *style, bool recalc)
{
    Q_ASSERT(style);
    if (recalc)
        recalculate();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager adds a style
void StylesModel::addCharacterStyle(KoCharacterStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeParagraphStyle(KoParagraphStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeCharacterStyle(KoCharacterStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

void StylesModel::updateName(int styleId)
{
    // TODO, no idea how to do this more correct for children...
    int row = m_styleList.indexOf(styleId);
    if (row >= 0) {
        QModelIndex index = createIndex(row, 0, styleId);
        emit dataChanged(index, index);
    }
}
