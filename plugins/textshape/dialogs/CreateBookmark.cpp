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

#include "CreateBookmark.h"

#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoInlineTextObjectManager.h>
#include <KoBookmarkManager.h>

#include <KLocale>
#include <KDebug>

CreateBookmark::CreateBookmark(KoTextEditor *editor, QWidget *parent)
    : KDialog(parent),
    m_editor(editor),
    m_manager(0)
{
    KoTextDocument doc(editor->document());
    KoInlineTextObjectManager *itom = doc.inlineTextObjectManager();
    if (itom)
        m_manager = itom->bookmarkManager();
    if (m_manager)
        m_existingBookmarks = m_manager->bookmarkNames();
qDebug() << m_existingBookmarks;

    QWidget *root = new QWidget();
    widget.setupUi(root);
    setMainWidget(root);
    setCaption(i18n("Insert Bookmark"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);
    showButtonSeparator(true);

    widget.inUseWarning->setVisible(false);
    widget.name->setFocus();

    connect(widget.name, SIGNAL(textChanged(const QString&)),
            this, SLOT(nameChanged(const QString&)));
    QString x = m_editor->selectedText();
    for (int i = 0; i < x.length(); ++i)
        qDebug() << i << x[i] << x[i].unicode();
    widget.name->setText(m_editor->selectedText().remove(65532)); // remember to remove bookmark markers

    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
}

void CreateBookmark::okClicked()
{
    m_editor->addBookmark(widget.name->text());

    if (m_manager)
        qDebug() << m_manager->bookmarkNames();
}

void CreateBookmark::nameChanged(const QString &name)
{
    bool inUse = false;
    if (name.isEmpty()) {
        enableButtonOk(false);
    } else {
        if (m_existingBookmarks.contains(name))
            inUse = true;
        enableButtonOk(!inUse);
    }
    widget.inUseWarning->setVisible(inUse);
}
