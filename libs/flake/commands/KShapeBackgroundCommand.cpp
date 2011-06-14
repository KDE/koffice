/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KShapeBackgroundCommand.h"
#include "KShape.h"
#include "KShapeBackground.h"

#include <klocale.h>

class KShapeBackgroundCommand::Private
{
public:
    Private() {
    }
    ~Private() {
        foreach(KShapeBackground* fill, oldFills) {
            if (fill && !fill->deref())
                delete fill;
        }
        foreach(KShapeBackground* fill, newFills) {
            if (fill && !fill->deref())
                delete fill;
        }
    }

    void addOldFill(KShapeBackground * oldFill)
    {
        if (oldFill)
            oldFill->ref();
        oldFills.append(oldFill);
    }

    void addNewFill(KShapeBackground * newFill)
    {
        if (newFill)
            newFill->ref();
        newFills.append(newFill);
    }

    QList<KShape*> shapes;    ///< the shapes to set background for
    QList<KShapeBackground*> oldFills;
    QList<KShapeBackground*> newFills;
};

KShapeBackgroundCommand::KShapeBackgroundCommand(const QList<KShape*> &shapes, KShapeBackground * fill,
        QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes = shapes;
    foreach(KShape *shape, d->shapes) {
        d->addOldFill(shape->background());
        d->addNewFill(fill);
    }

    setText(i18n("Set Background"));
}

KShapeBackgroundCommand::KShapeBackgroundCommand(KShape * shape, KShapeBackground * fill, QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes.append(shape);
    d->addOldFill(shape->background());
    d->addNewFill(fill);

    setText(i18n("Set Background"));
}

KShapeBackgroundCommand::KShapeBackgroundCommand(const QList<KShape*> &shapes, const QList<KShapeBackground*> &fills, QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes = shapes;
    foreach(KShape *shape, d->shapes) {
        d->addOldFill(shape->background());
    }
    foreach (KShapeBackground * fill, fills) {
        d->addNewFill(fill);
    }

    setText(i18n("Set Background"));
}

void KShapeBackgroundCommand::redo()
{
    QUndoCommand::redo();
    QList<KShapeBackground*>::iterator brushIt = d->newFills.begin();
    foreach(KShape *shape, d->shapes) {
        shape->setBackground(*brushIt);
        shape->update();
        brushIt++;
    }
}

void KShapeBackgroundCommand::undo()
{
    QUndoCommand::undo();
    QList<KShapeBackground*>::iterator brushIt = d->oldFills.begin();
    foreach(KShape *shape, d->shapes) {
        shape->setBackground(*brushIt);
        shape->update();
        brushIt++;
    }
}

KShapeBackgroundCommand::~KShapeBackgroundCommand()
{
    delete d;
}
