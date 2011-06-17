/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include <QTextDocument>
#include <QTextCursor>
#include <QUrl>
#include <QVariant>

#include <kdebug.h>

#include "KTextDocument.h"
#include "KoTextEditor.h"
#include "styles/KStyleManager.h"
#include "KInlineTextObjectManager.h"
#include "KoTextDocumentLayout.h"
#include "styles/KParagraphStyle.h"
#include "changetracker/KChangeTracker.h"
#include "KoList.h"
#include <KUndoStack>

const QUrl KTextDocument::StyleManagerURL = QUrl("kotext://stylemanager");
const QUrl KTextDocument::ListsURL = QUrl("kotext://lists");
const QUrl KTextDocument::InlineObjectTextManagerURL = QUrl("kotext://inlineObjectTextManager");
const QUrl KTextDocument::UndoStackURL = QUrl("kotext://undoStack");
const QUrl KTextDocument::ChangeTrackerURL = QUrl("kotext://changetracker");
const QUrl KTextDocument::TextEditorURL = QUrl("kotext://textEditor");
const QUrl KTextDocument::HeadingListURL = QUrl("kotext://headingList");

KTextDocument::KTextDocument(QTextDocument *document)
    : m_document(document)
{
    Q_ASSERT(m_document);
}

KTextDocument::KTextDocument(const QTextDocument *document)
    : m_document(const_cast<QTextDocument *>(document))
{
    Q_ASSERT(m_document);
}

KTextDocument::~KTextDocument()
{
}

QTextDocument *KTextDocument::document() const
{
    return m_document;
}

void KTextDocument::setTextEditor (KoTextEditor* textEditor)
{
    QVariant v;
    v.setValue(textEditor);
    m_document->addResource(KTextDocument::TextEditor, TextEditorURL, v);
}

KoTextEditor* KTextDocument::textEditor()
{
    QVariant resource = m_document->resource(KTextDocument::TextEditor, TextEditorURL);
    return resource.value<KoTextEditor *>();
}

void KTextDocument::setStyleManager(KStyleManager *sm)
{
    QVariant v;
    v.setValue(sm);
    m_document->addResource(KTextDocument::StyleManager, StyleManagerURL, v);
    if (sm)
        sm->add(m_document);
}

void KTextDocument::setInlineTextObjectManager(KInlineTextObjectManager *manager)
{
    QVariant v;
    v.setValue(manager);
    m_document->addResource(KTextDocument::InlineTextManager, InlineObjectTextManagerURL, v);

    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    if (lay)
        lay->setInlineTextObjectManager(manager);
}

KStyleManager *KTextDocument::styleManager() const
{
    QVariant resource = m_document->resource(KTextDocument::StyleManager, StyleManagerURL);
    return resource.value<KStyleManager *>();
}

void KTextDocument::setChangeTracker(KChangeTracker *changeTracker)
{
    QVariant v;
    v.setValue(changeTracker);
    m_document->addResource(KTextDocument::ChangeTrackerResource, ChangeTrackerURL, v);
}

KChangeTracker *KTextDocument::changeTracker() const
{
    QVariant resource = m_document->resource(KTextDocument::ChangeTrackerResource, ChangeTrackerURL);
    return resource.value<KChangeTracker *>();
}

void KTextDocument::setHeadingList(KoList *headingList)
{
    QVariant v;
    v.setValue(headingList);
    m_document->addResource(KTextDocument::HeadingList, HeadingListURL, v);
}

KoList *KTextDocument::headingList() const
{
    QVariant resource = m_document->resource(KTextDocument::HeadingList, HeadingListURL);
    return resource.value<KoList *>();
}

void KTextDocument::setUndoStack(KUndoStack *undoStack)
{
    QVariant v;
    v.setValue<void*>(undoStack);
    m_document->addResource(KTextDocument::UndoStack, UndoStackURL, v);
}

KUndoStack *KTextDocument::undoStack() const
{
    QVariant resource = m_document->resource(KTextDocument::UndoStack, UndoStackURL);
    return static_cast<KUndoStack*>(resource.value<void*>());
}

void KTextDocument::setLists(const QList<KoList *> &lists)
{
    QVariant v;
    v.setValue(lists);
    m_document->addResource(KTextDocument::Lists, ListsURL, v);
}

QList<KoList *> KTextDocument::lists() const
{
    QVariant resource = m_document->resource(KTextDocument::Lists, ListsURL);
    return resource.value<QList<KoList *> >();
}

void KTextDocument::addList(KoList *list)
{
    Q_ASSERT(list);
    list->setParent(m_document);
    QList<KoList *> l = lists();
    if (l.contains(list))
        return;
    l.append(list);
    setLists(l);
}

void KTextDocument::removeList(KoList *list)
{
    QList<KoList *> l = lists();
    if (l.contains(list)) {
        l.removeAll(list);
        setLists(l);
    }
}

KoList *KTextDocument::list(const QTextBlock &block) const
{
    QTextList *textList = block.textList();
    if (!textList)
        return 0;
    return list(textList);
}

KoList *KTextDocument::list(QTextList *textList) const
{
    // FIXME: this is horrible.
    foreach(KoList *l, lists()) {
        if (l->textLists().contains(textList))
            return l;
    }
    return 0;
}

KoList *KTextDocument::list(KListStyle::ListIdType listId) const
{
    foreach(KoList *l, lists()) {
        if (l->textListIds().contains(listId))
            return l;
    }
    return 0;
}

void KTextDocument::clearText()
{
    QTextCursor cursor(m_document);
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
}

KInlineTextObjectManager *KTextDocument::inlineTextObjectManager() const
{
    QVariant resource = m_document->resource(KTextDocument::InlineTextManager,
            InlineObjectTextManagerURL);
    return resource.value<KInlineTextObjectManager *>();
}

void KTextDocument::setResizeMethod(KTextDocument::ResizeMethod method)
{
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    Q_ASSERT(layout);
    layout->setResizeMethod(method);
}

KTextDocument::ResizeMethod KTextDocument::resizeMethod() const
{
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    Q_ASSERT(layout);
    return layout->resizeMethod();
}
