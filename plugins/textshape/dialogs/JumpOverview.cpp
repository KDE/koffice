/* This file is part of the KDE project
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "JumpOverview.h"

#include <KTextDocument.h>
#include <KoTextEditor.h>
#include <KInlineTextObjectManager.h>
#include <KoBookmarkManager.h>
#include <KoBookmark.h>
#include <KTextPage.h>
#include <KTextLocator.h>
#include <KTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KShape.h>

#include <KLocale>
#include <KDebug>

JumpOverview::JumpOverview(QTextDocument *doc, QWidget *parent)
    : KDialog(parent),
    m_doc(doc)
{
    Q_ASSERT(doc);
    QWidget *root = new QWidget();
    widget.setupUi(root);
    setMainWidget(root);
    setCaption(i18n("Insert Bookmark"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);

    KInlineTextObjectManager *itom = m_doc.inlineTextObjectManager();
    if (itom == 0)
        return;
    KoBookmarkManager *bookmarkManager = itom->bookmarkManager();
    foreach (KoBookmark *bookmark, bookmarkManager->bookmarks()) {
        // kDebug() << "bookmark" << bookmark->name() << bookmark->textPosition();
        JumpEntry entry;
        entry.type = JumpEntry::Bookmark;
        entry.name = bookmark->name();
        entry.position = bookmark->textPosition();
        entry.objId = bookmark->id();
        KoBookmark *end = bookmark->endBookmark();
        if (end)
            entry.length = end->textPosition() - entry.position;
        m_entries << entry;
    }

    foreach (const KTextLocator *locator, itom->textLocators()) {
        // kDebug() << "locator" << locator->word() << "on page" << locator->pageNumber();
        JumpEntry entry;
        entry.type = JumpEntry::Bookmark;
        entry.name = locator->word();
        entry.position = locator->textPosition();
        entry.objId = locator->id();
        m_entries << entry;
    }

    KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(doc->documentLayout());
    if (lay) {
        foreach (KShape *shape, lay->shapes()) {
            KoTextShapeData *textData = qobject_cast<KoTextShapeData*>(shape->userData());
            if (textData) {
                KTextPage *page = textData->page();
                if (page) {
                    JumpEntry entry;
                    entry.type = JumpEntry::Page;
                    entry.pageNumber = page->pageNumber();
                    entry.name = i18n("Page %1", entry.pageNumber);
                    entry.position = textData->position();
                    m_entries << entry;
                }
            }
        }
    }

    qSort(m_entries.begin(), m_entries.end(), JumpEntry::sortJumpEntries);

    for (int i = 0; i < m_entries.count(); ++i) {
        const JumpEntry &entry = m_entries[i];
        widget.items->addItem(entry.name);
    }

    connect(widget.items, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentRow(int)));
    // TODO delete and rename
}

void JumpOverview::setCurrentRow(int row)
{
    Q_ASSERT(m_entries.count() > row);
    const JumpEntry &entry = m_entries[row];

    switch (entry.type) {
    case JumpEntry::Bookmark:
        widget.type->setText(i18n("Bookmark"));
        break;
    case JumpEntry::Page:
        widget.type->setText(i18n("Page"));
        break;
    case JumpEntry::TextLocator:
        widget.type->setText(i18n("Index locator"));
        break;
    }
    widget.bDelete->setEnabled(entry.type != JumpEntry::Page);

    widget.name->setText(entry.name);
    if (entry.pageNumber == -1) {
        KInlineObject *object = m_doc.inlineTextObjectManager()->inlineTextObject(entry.objId);
        KTextPage *page = 0;
        if (object)
            page = object->page();
        if (page)
            m_entries[row].pageNumber = page->pageNumber();
        else
            m_entries[row].pageNumber = -2;
    }
    if (entry.pageNumber == -2) {
        widget.pageLabel->setVisible(false);
        widget.page->setVisible(false);
    } else {
        widget.pageLabel->setVisible(true);
        widget.page->setVisible(true);
        widget.page->setText(QString::number(entry.pageNumber));
    }

    if (entry.length > 0) {
        QTextCursor cursor(m_doc.document());
        cursor.setPosition(entry.position);
        cursor.setPosition(entry.position + entry.length, QTextCursor::KeepAnchor);
        widget.textContent->setText(cursor.selectedText());
    } else if (entry.type == JumpEntry::Page) {
        QTextBlock block = m_doc.document()->findBlock(entry.position);
        QString txt;
        if (block.isValid())
            txt = block.text();
        widget.textContent->setText(txt);
    } else {
        widget.textContent->setText(QString());
    }
    emit cursorPositionSelected(entry.position);
}
