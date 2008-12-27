/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef LISTITEMNUMBERINGCOMMAND
#define LISTITEMNUMBERINGCOMMAND

#include "TextCommandBase.h"

#include <QTextBlock>

/**
 * This command is useful to mark a block as numbered or unnumbered list-item.
 */
class ListItemNumberingCommand : public TextCommandBase
{
public:
    /**
     * Change the list property of 'block'.
     * @param block the paragraph to change the list property of
     * @param numbered indicates if the block is an numbered list item
     * @param parent the parent undo command for macro functionality
     */
    ListItemNumberingCommand(const QTextBlock &block, bool numbered, QUndoCommand *parent = 0);

    ~ListItemNumberingCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

    /// reimplemnted from QUndoCommand
    virtual int id() const {
        return 58450688;
    }
    /// reimplemnted from QUndoCommand
    virtual bool mergeWith(const QUndoCommand *other);

private:
    void setNumbered(bool numbered);

    QTextBlock m_block;
    bool m_numbered;
    bool m_wasNumbered;
    bool m_first;
};

#endif
