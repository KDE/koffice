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

#ifndef KWSETFRAMESETCOMMAND_H
#define KWSETFRAMESETCOMMAND_H

#include <QUndoCommand>

class KWFrame;
class KWTextFrame;
class KWTextFrameSet;

/// The undo / redo command for deleting frames
class KWSetFrameSetCommand : public QUndoCommand
{
public:
    /**
     * Constructor for a command to delete a frame.
     * @param shapeController the shape controller to remove / add the shapes from/to.
     * @param frame the frame to delete.
     * @param parent the parent for macro command functionality
     */
    explicit KWSetFrameSetCommand(KWTextFrame *frame, KWTextFrameSet *newFs, QUndoCommand *parent = 0);
    ~KWSetFrameSetCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KWFrame *m_frame;
    KWTextFrameSet *m_oldFrameSet;
    KWTextFrameSet *m_newFrameSet;
    bool m_ownFrameSet;
};

#endif
