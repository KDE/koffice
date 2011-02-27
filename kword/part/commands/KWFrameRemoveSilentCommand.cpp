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

#include "KWFrameRemoveSilentCommand.h"
#include <KWDocument.h>
#include <frames/KWFrame.h>
#include <frames/KWFrameSet.h>

KWFrameRemoveSilentCommand::KWFrameRemoveSilentCommand(KWDocument *doc, KWFrame *frame, QUndoCommand *parent)
    : QUndoCommand(QString(), parent),
    m_frame(frame),
    m_frameSet(frame->frameSet()),
    m_shape(frame->shape()),
    m_doc(doc),
    m_ownFrame(false),
    m_ownFrameSet(false)
{
}

KWFrameRemoveSilentCommand::~KWFrameRemoveSilentCommand()
{
    if (m_ownFrameSet)
        delete m_frameSet;
    if (m_ownFrame)
        delete m_frame;
}

void KWFrameRemoveSilentCommand::redo()
{
    QUndoCommand::redo();
    m_shape->setApplicationData(0);
    if (m_frameSet->frameCount() == 1) {
        m_doc->m_frameSets.removeAt(m_doc->m_frameSets.indexOf(m_frameSet));
        m_ownFrameSet = true;
    } else {
        m_frameSet->removeFrame(m_frame);
        m_ownFrame = true;
    }
}

void KWFrameRemoveSilentCommand::undo()
{
    if (m_ownFrame) {
        m_frameSet->addFrame(m_frame);
        m_ownFrame = false;
    } else if (m_ownFrameSet) {
        m_doc->m_frameSets.append(m_frameSet);
        m_ownFrameSet = false;
    }
    QUndoCommand::undo();
}

