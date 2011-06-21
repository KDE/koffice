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

#ifndef KWSETFSNAMECOMMAND_H
#define KWSETFSNAMECOMMAND_H

#include <QUndoCommand>

class KWFrameSet;

/// The undo / redo command for deleting frames
class KWSetFSNameCommand : public QUndoCommand
{
public:
    /**
     * Constructor for a command to delete a frame.
     * @param shapeController the shape controller to remove / add the shapes from/to.
     * @param frame the frame to delete.
     * @param parent the parent for macro command functionality
     */
    explicit KWSetFSNameCommand(KWFrameSet *frameSet, const QString &newName, QUndoCommand *parent = 0);
    ~KWSetFSNameCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KWFrameSet *m_frameSet;
    QString m_oldName;
    QString m_newName;
};

#endif
