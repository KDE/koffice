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

#ifndef JUMPOVERVIEW_H
#define JUMPOVERVIEW_H

#include <KDialog>
#include <QString>

#include <KTextDocument.h>

#include <ui_JumpOverview.h>

class KTextDocument;
class QTextDocument;

struct JumpEntry {
    JumpEntry() : position(-1), length(0), pageNumber(-1), objId(-1) { }
    enum Type {
        Bookmark,
        Page,
        TextLocator // i.e. Indexing marker
    };
    Type type;
    QString name;
    int position;
    int length;
    int pageNumber;
    int objId;

    static bool sortJumpEntries(const JumpEntry &first, const JumpEntry &second) {
        return first.position < second.position;
    }
};

class JumpOverview : public KDialog
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param editor the text editor used to insert the bookmark
     * @param parent a parent widget for the purpose of centering the dialog
     */
    JumpOverview(QTextDocument *doc, QWidget *parent = 0);

signals:
    void cursorPositionSelected(int pos);

private slots:
    void setCurrentRow(int row);

private:
    Ui::JumpOverview widget;
    KTextDocument m_doc;
    QList<JumpEntry> m_entries;
};

#endif
