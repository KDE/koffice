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

#include "KWSetFSNameCommand.h"
#include <frames/KWTextFrameSet.h>

KWSetFSNameCommand::KWSetFSNameCommand(KWFrameSet *frameSet, const QString &newName, QUndoCommand *parent)
    : QUndoCommand(QString(), parent),
    m_frameSet(frameSet),
    m_oldName(frameSet->name()),
    m_newName(newName)
{
}

KWSetFSNameCommand::~KWSetFSNameCommand()
{
}

void KWSetFSNameCommand::redo()
{
    QUndoCommand::redo();
    m_frameSet->setName(m_newName);
}

void KWSetFSNameCommand::undo()
{
    m_frameSet->setName(m_oldName);
    QUndoCommand::undo();
}

