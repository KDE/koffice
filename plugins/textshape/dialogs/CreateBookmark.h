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

#ifndef KWCREATEBOOKMARK_H
#define KWCREATEBOOKMARK_H

#include <KDialog>
#include <QList>
#include <QString>

#include <ui_CreateBookmark.h>

class KoBookmarkManager;
class KoTextEditor;
class KoBookmarkManager;

class CreateBookmark : public KDialog
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param editor the text editor used to insert the bookmark
     * @param parent a parent widget for the purpose of centering the dialog
     */
    CreateBookmark(KoTextEditor *editor, QWidget *parent = 0);

private slots:
    void okClicked();
    void nameChanged(const QString &name);

private:
    Ui::CreateBookmark widget;
    KoTextEditor *m_editor;
    KoBookmarkManager *m_manager;
    QList<QString> m_existingBookmarks;
};

#endif
