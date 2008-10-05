/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
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

#ifndef KWPAGEINSERTCOMMAND_H
#define KWPAGEINSERTCOMMAND_H

#include "../KWPage.h"

#include <QUndoCommand>

class KWPage;
class KWPageManager;
class KWDocument;
class KoShapeMoveCommand;

/// The undo / redo command for inserting a new page in a kword document.
class KWPageInsertCommand : public QUndoCommand
{
public:
    /**
     * The constuctor for a command to insert a new page.
     * @param document the document that gets a new page.
     * @param afterPageNum we will insert a new page after the page indicated with pagenumber afterPageNum
     * @param parent the parent for command macros
     * @param masterPageName the master page name for the new page
     */
    explicit KWPageInsertCommand(KWDocument *document, int afterPageNum, QUndoCommand *parent = 0, const QString &masterPageName = QString());
    ~KWPageInsertCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

    /// return the page created.
    KWPage page() const {
        return m_page;
    }

private:
    KWDocument *m_document;
    KWPage m_page;
    int m_afterPageNum;
    QString m_masterPageName;
    KoShapeMoveCommand *m_shapeMoveCommand;
};

#endif
