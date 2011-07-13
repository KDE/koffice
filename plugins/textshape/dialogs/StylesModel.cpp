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

#include <KStyleManager.h>
#include <KParagraphStyle.h>
#include <KCharacterStyle.h>

#include <KIcon>
#include <KDebug>

StylesModel::StylesModel(KStyleManager *manager, QObject *parent)
        : QAbstractItemModel(parent),
        m_styleManager(0),
        m_currentParagraphStyle(0),
        m_currentCharacterStyle(0),
        m_pureParagraphStyle(true),
        m_pureCharacterStyle(true),
        m_styleMapper(new QSignalMapper(this))
{
    setStyleManager(manager);
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
    const int DefaultParagStyleId = m_styleManager->defaultParagraphStyle() ?
        m_styleManager->defaultParagraphStyle()->styleId() : -1;
    const int DefaultCharStyleId = DefaultParagStyleId == -1 ? -1 :
        (m_styleManager->defaultParagraphStyle()->characterStyle() ?
         m_styleManager->defaultParagraphStyle()->characterStyle()->styleId(): -1);

    /*
        In the model we need two-way iteration. In the styles we already have
        a way to traverse up the tree using the paragraphStyle::parentStyle()
        This method creates a tree of styles and stores the info to traverse
        down the tree in m_relations.
    */

    QSet<int> paragraphStyles; // found paragraphStyles
    QSet<int> characterStyles; // found characterStyles
    QSet<int> treeRoot;
    QHash<int, QList<int> > tree;

    foreach (KParagraphStyle *style, m_styleManager->paragraphStyles()) {
        if (style->parentStyle() == 0) {
            treeRoot.insert(style->styleId());
            continue;
        }
        Q_ASSERT(style->parentStyle());
        while (style->parentStyle()) {
            // store the relation style->parentStyle() <-> style
            const int key = style->parentStyle()->styleId();
            const int value = style->styleId();

            if (paragraphStyles.contains(value)) // relationship already present
                break;
            characterStyles << style->characterStyle()->styleId();
            const QString styleName = style->name();
            QList<int> prevValues = tree.value(key);
            // insert value in prevValues according to i18n sorting.
            bool inserted = false;
            for (QList<int>::Iterator iter = prevValues.begin(); iter != prevValues.end(); ++iter) {
                if ((*iter) == value) {
                    inserted = true; // already present.
                    break;
                }
                KParagraphStyle *i = m_styleManager->paragraphStyle(*iter);
                Q_ASSERT(i);
                if (styleName.compare(i->name(), Qt::CaseInsensitive) > 0) {
                    prevValues.insert(iter, value);
                    inserted = true;
                    break;
                }
            }
            if (!inserted)
                prevValues.append(value);
            tree.insert(key, prevValues);

            style = style->parentStyle();
        }
    }

    // now copy our 'tree' to m_relations. Probably should refactor to use this datastructure..
    for (QHash<int, QList<int> >::const_iterator it = tree.constBegin(); it != tree.constEnd(); ++it) {
    	int key = it.key();
        foreach (int x, it.value()) {
            m_relations.insert(key, x);
        }
    }


    QList<int> newStyleList;
    if (treeRoot.count() == 1 && (*treeRoot.constBegin()) == DefaultParagStyleId) {
        newStyleList = m_relations.values(DefaultParagStyleId);
    } else { // a real list
        newStyleList = treeRoot.toList(); // TODO sort alphabetically on style name?
    }
    emit isMultiLevel(m_relations.count() != newStyleList.count());

    foreach (KCharacterStyle *style, m_styleManager->characterStyles()) {
        const int key = style->styleId();
        if (key != DefaultCharStyleId && !characterStyles.contains(key))
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
        int maxRow = qMax(m_styleList.count(), newStyleList.count());
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
    KParagraphStyle *pstyle = m_styleManager->paragraphStyle(id);
    if (pstyle == 0) { // that means it has to be a charStyle. Thats easy, there is no hierarchy there.
        if (row >= m_styleList.count())
            return QModelIndex();
        return createIndex(row, column, m_styleList[row]);
    }

    if (m_relations.contains(id)) {
        QList<int> children = m_relations.values(id);
        if (row >= children.count())
            return QModelIndex();
        return createIndex(row, column, children[row]);
    }
    return QModelIndex();
}

QModelIndex StylesModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        int id = (int) child.internalId();
        if (m_styleList.contains(id)) // is the root, parent is invalid.
            return QModelIndex();
        KParagraphStyle *childStyle = m_styleManager->paragraphStyle(id);
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
        KParagraphStyle *style = m_styleManager->paragraphStyle(id);
        if (style == 0)
            continue;
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
        return m_relations.values(id).count();
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
        KParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
        if (paragStyle)
            return paragStyle->name();
        KCharacterStyle *characterStyle =  m_styleManager->characterStyle(id);
        if (characterStyle)
            return characterStyle->name();
        break;
    }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (m_styleManager->characterStyle(id))
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

QModelIndex StylesModel::setCurrentParagraphStyle(int styleId, bool unchanged)
{
    if (m_currentParagraphStyle == styleId && unchanged == m_pureParagraphStyle)
        return QModelIndex();

    QModelIndex mi;
    for (int i = 0; i < 2; ++i) {
        mi = indexForStyle(m_currentParagraphStyle);
        emit dataChanged(mi, mi);

        m_currentParagraphStyle = styleId;
        m_pureParagraphStyle = unchanged;
    }
    return mi;
}

QModelIndex StylesModel::indexForStyle(int styleId)
{
    for (QMultiHash<int,int>::const_iterator it = m_relations.constBegin(); it != m_relations.constEnd(); ++it) {
	int key = it.key();
        int index = m_relations.values(key).indexOf(styleId);
        if (index >= 0)
            return createIndex(index, 0, styleId);
    }
    return QModelIndex();
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

KParagraphStyle *StylesModel::paragraphStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->paragraphStyle(index.internalId());
}

KCharacterStyle *StylesModel::characterStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->characterStyle(index.internalId());
}

void StylesModel::setStyleManager(KStyleManager *sm)
{
    if (sm == m_styleManager)
        return;
    if (m_styleManager) {
        disconnect(sm, SIGNAL(styleAdded(KParagraphStyle*)), this, SLOT(addParagraphStyle(KParagraphStyle*)));
        disconnect(sm, SIGNAL(styleAdded(KCharacterStyle*)), this, SLOT(addCharacterStyle(KCharacterStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KParagraphStyle*)), this, SLOT(removeParagraphStyle(KParagraphStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KCharacterStyle*)), this, SLOT(removeCharacterStyle(KCharacterStyle*)));
    }
    m_styleManager = sm;

    if (m_styleManager == 0) {
        recalculate();
        return;
    }

    connect(sm, SIGNAL(styleAdded(KCharacterStyle*)), this, SLOT(addCharacterStyle(KCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KCharacterStyle*)), this, SLOT(removeCharacterStyle(KCharacterStyle*)));
    connect(sm, SIGNAL(styleAdded(KParagraphStyle*)), this, SLOT(addParagraphStyle(KParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KParagraphStyle*)), this, SLOT(removeParagraphStyle(KParagraphStyle*)));

    foreach(KParagraphStyle *style, m_styleManager->paragraphStyles())
        addParagraphStyle(style, false);
    foreach(KCharacterStyle *style, m_styleManager->characterStyles())
        addCharacterStyle(style, false);

    recalculate();
}

// called when the stylemanager adds a style
void StylesModel::addParagraphStyle(KParagraphStyle *style, bool recalc)
{
    Q_ASSERT(style);
    if (recalc)
        recalculate();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager adds a style
void StylesModel::addCharacterStyle(KCharacterStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeParagraphStyle(KParagraphStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeCharacterStyle(KCharacterStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

void StylesModel::updateName(int styleId)
{
    QModelIndex mi = indexForStyle(styleId);
    emit dataChanged(mi, mi);
}
