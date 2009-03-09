/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KWSelectBookmarkDialog.h"

#include <kmessagebox.h>
#include <kinputdialog.h>

static QString lastBookMarkItem;

KWSelectBookmark::KWSelectBookmark(QList<QString> nameList, QWidget *parent)
        : QWidget(parent),
        parentWidget(parent)
{
    widget.setupUi(this);
    widget.bookmarkList->addItems(nameList);
    widget.bookmarkList->setFocus(Qt::ActiveWindowFocusReason);

    const int count = widget.bookmarkList->count();
    if (count > 0) {
        int row = 0;
        if (! lastBookMarkItem.isNull()) {
            QList<QListWidgetItem *> items = widget.bookmarkList->findItems(lastBookMarkItem, Qt::MatchExactly);
            if (items.count() > 0)
                row = widget.bookmarkList->row(items[0]);
        }
        widget.bookmarkList->setCurrentRow(row);
    }

    connect(widget.bookmarkList, SIGNAL(currentRowChanged(int)), this, SLOT(selectionChanged(int)));
    connect(widget.buttonRename, SIGNAL(clicked()), this, SLOT(slotBookmarkRename()));
    connect(widget.buttonDelete, SIGNAL(clicked()), this, SLOT(slotBookmarkDelete()));
    connect(widget.bookmarkList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(slotBookmarkItemActivated(QListWidgetItem *)));
    selectionChanged(bookmarkRow());
}

QString KWSelectBookmark::bookmarkName() const
{
    const QListWidgetItem* item = widget.bookmarkList->currentItem();
    return item ? item->text() : QString();
}

int KWSelectBookmark::bookmarkRow() const
{
    return widget.bookmarkList->currentRow();
}

void KWSelectBookmark::selectionChanged(int currentRow)
{
    widget.buttonRename->setEnabled(currentRow != -1);
    widget.buttonDelete->setEnabled(currentRow != -1);
    emit bookmarkSelectionChanged(currentRow);
}

void KWSelectBookmark::slotBookmarkRename()
{
    QListWidgetItem* item = widget.bookmarkList->currentItem();
    Q_ASSERT(item);
    QString curName = item->text();
    QString newName = item->text();
    while (true) {
        newName = KInputDialog::getText(i18n("Rename Bookmark"),
                                        i18n("Please provide a new name for the bookmark"),
                                        newName,
                                        0,
                                        parentWidget);
        if (curName != newName && ! newName.isNull()) {
            if (newName.isEmpty())
                continue;
            QList<QListWidgetItem *> items = widget.bookmarkList->findItems(newName, Qt::MatchExactly);
            if (items.count() > 0) {
                KMessageBox::error(parentWidget, i18n("A bookmark with the name \"%1\" already exists.", newName));
                continue;
            }
            item->setText(newName);
            emit bookmarkNameChanged(curName, newName);
        }
        break;
    }
}

void KWSelectBookmark::slotBookmarkDelete()
{
    int currentRow = widget.bookmarkList->currentRow();
    Q_ASSERT(currentRow >= 0);
    QListWidgetItem *deletedItem = widget.bookmarkList->takeItem(currentRow);
    QString deletedName = deletedItem->text();
    emit bookmarkItemDeleted(deletedName);
    delete deletedItem;
}

void KWSelectBookmark::slotBookmarkItemActivated(QListWidgetItem *item)
{
    Q_ASSERT(item);
    lastBookMarkItem = item->text();
    emit bookmarkItemDoubleClicked(item);
}

KWSelectBookmarkDialog::KWSelectBookmarkDialog(QList<QString> nameList, QWidget *parent)
        : KDialog(parent)
{
    ui = new KWSelectBookmark(nameList, this);
    setMainWidget(ui);
    setCaption(i18n("Select Bookmark"));
    setModal(true);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    showButtonSeparator(true);
    connect(ui, SIGNAL(bookmarkSelectionChanged(int)), this, SLOT(selectionChanged(int)));
    connect(ui, SIGNAL(bookmarkNameChanged(const QString &, const QString &)),
            this, SIGNAL(nameChanged(const QString &, const QString &)));
    connect(ui, SIGNAL(bookmarkItemDeleted(const QString &)),
            this, SIGNAL(bookmarkDeleted(const QString &)));
    connect(ui, SIGNAL(bookmarkItemDoubleClicked(QListWidgetItem *)),
            this, SLOT(bookmarkDoubleClicked(QListWidgetItem *)));
    selectionChanged(ui->bookmarkRow());
}

QString KWSelectBookmarkDialog::selectedBookmarkName()
{
    return ui->bookmarkName();
}

void KWSelectBookmarkDialog::selectionChanged(int currentRow)
{
    enableButtonOk(currentRow != -1);
}

void KWSelectBookmarkDialog::bookmarkDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    accept();
}

#include <KWSelectBookmarkDialog.moc>

