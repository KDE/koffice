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

#include "KShapeLockCommand.h"
#include "KShape.h"

#include <klocale.h>

class KShapeLockCommandPrivate
{
public:
    QList<KShape*> shapes;    /// the shapes to lock
    QList<bool> oldLock;       /// old lock states
    QList<bool> newLock;       /// new lock states
};

KShapeLockCommand::KShapeLockCommand(const QList<KShape*> &shapes, const QList<bool> &oldLock, const QList<bool> &newLock, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new KShapeLockCommandPrivate())
{
    d->shapes = shapes;
    d->oldLock = oldLock;
    d->newLock = newLock;

    Q_ASSERT(d->shapes.count() == d->oldLock.count());
    Q_ASSERT(d->shapes.count() == d->newLock.count());

    setText(i18n("Lock Shapes"));
}

KShapeLockCommand::~KShapeLockCommand()
{
}

void KShapeLockCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < d->shapes.count(); ++i) {
        d->shapes[i]->setGeometryProtected(d->newLock[i]);
    }
}

void KShapeLockCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->shapes.count(); ++i) {
        d->shapes[i]->setGeometryProtected(d->oldLock[i]);
    }
}
