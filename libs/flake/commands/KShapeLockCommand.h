/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KSHAPELOCKCOMMAND_H
#define KSHAPELOCKCOMMAND_H

#include <QUndoCommand>
#include <QList>

#include "flake_export.h"

class KShape;
class KShapeLockCommandPrivate;

/// The undo / redo command to lock a set of shapes position and size
class FLAKE_EXPORT KShapeLockCommand : public QUndoCommand
{
public:
    /**
     * Command to lock a set of shapes position and size
     * @param shapes a set of shapes that should change lock state
     * @param oldLock list of old lock states the same length as @p shapes
     * @param newLock list of new lock states the same length as @p shapes
     * @param parent the parent command used for macro commands
     */
    KShapeLockCommand(const QList<KShape*> &shapes, const QList<bool> &oldLock,
            const QList<bool> &newLock, QUndoCommand *parent = 0);
    ~KShapeLockCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KShapeLockCommandPrivate *d;
};

#endif
