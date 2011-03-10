/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPRPAGELAYOUTCOMMAND_H
#define KPRPAGELAYOUTCOMMAND_H

#include <QUndoCommand>

class SCPageLayout;
class SCPlaceholders;

class SCPageLayoutCommand : public QUndoCommand
{
public:
    SCPageLayoutCommand(SCPlaceholders * placeholders, SCPageLayout * layout, QUndoCommand *parent = 0);
    virtual ~SCPageLayoutCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    SCPlaceholders * m_placeholders;
    SCPageLayout * m_oldLayout;
    SCPageLayout * m_newLayout;
};

#endif /* KPRPAGELAYOUTCOMMAND_H */
