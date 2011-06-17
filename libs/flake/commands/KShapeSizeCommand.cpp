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

#include "KShapeSizeCommand.h"

#include <klocale.h>

class KShapeSizeCommand::Private
{
public:
    QList<KShape*> shapes;
    QList<QSizeF> previousSizes, newSizes;
};

KShapeSizeCommand::KShapeSizeCommand(const QList<KShape*> &shapes, const QList<QSizeF> &previousSizes, const QList<QSizeF> &newSizes, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private())
{
    d->previousSizes = previousSizes;
    d->newSizes = newSizes;
    d->shapes = shapes;
    Q_ASSERT(d->shapes.count() == d->previousSizes.count());
    Q_ASSERT(d->shapes.count() == d->newSizes.count());

    setText(i18n("Resize Shapes"));
}

KShapeSizeCommand::~KShapeSizeCommand()
{
    delete d;
}

void KShapeSizeCommand::redo()
{
    QUndoCommand::redo();
    int i = 0;
    foreach(KShape *shape, d->shapes) {
        shape->update();
        shape->setSize(d->newSizes[i++]);
        shape->update();
    }
}

void KShapeSizeCommand::undo()
{
    QUndoCommand::undo();
    int i = 0;
    foreach(KShape *shape, d->shapes) {
        shape->update();
        shape->setSize(d->previousSizes[i++]);
        shape->update();
    }
}
