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

#include "KWSetFrameSetCommand.h"
#include <KWDocument.h>
#include <frames/KWTextFrame.h>
#include <frames/KWTextFrameSet.h>

KWSetFrameSetCommand::KWSetFrameSetCommand(KWTextFrame *frame, KWTextFrameSet *newFs, QUndoCommand *parent)
    : QUndoCommand(i18n("Move shape to storyline"), parent),
    m_frame(frame),
    m_oldFrameSet(static_cast<KWTextFrameSet*>(frame->frameSet())),
    m_newFrameSet(newFs)
{
}

KWSetFrameSetCommand::~KWSetFrameSetCommand()
{
    if (m_ownFrameSet)
        delete m_oldFrameSet;
}

void KWSetFrameSetCommand::redo()
{
    QUndoCommand::redo();
    m_frame->setFrameSet(m_newFrameSet);

    if (m_oldFrameSet->frameCount() == 0 && m_oldFrameSet->kwordDocument()) {
        const_cast<KWDocument*>(m_oldFrameSet->kwordDocument())->removeFrameSet(m_oldFrameSet);
        m_ownFrameSet = true;
    }
}

void KWSetFrameSetCommand::undo()
{
    m_frame->setFrameSet(m_oldFrameSet);
    if (m_ownFrameSet) {
        const_cast<KWDocument*>(m_oldFrameSet->kwordDocument())->addFrameSet(m_oldFrameSet);
        m_oldFrameSet = false;
    }
    QUndoCommand::undo();
}

