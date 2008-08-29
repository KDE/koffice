/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KWFRAMECREATECOMMAND_H
#define KWFRAMECREATECOMMAND_H

#include <QUndoCommand>

class KWFrame;
class KWFrameSet;
class KoShapeControllerBase;

/// The undo / redo command for creating frames
class KWFrameCreateCommand : public QUndoCommand
{
public:
    /**
     * Constructor for a command to create a frame.
     * @param shapeController the shape controller to remove / add the shapes from/to.
     * @param frame the frame to create.
     * @param parent the parent for macro command functionality
     */
    explicit KWFrameCreateCommand(KoShapeControllerBase *shapeController, KWFrame *frame, QUndoCommand *parent = 0);

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KWFrame *m_frame;
    KWFrameSet *m_frameSet;
};

#endif
