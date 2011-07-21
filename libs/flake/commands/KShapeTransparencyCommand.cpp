/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KShapeTransparencyCommand.h"
#include "KShape.h"
#include "KShapeBackgroundBase.h"

#include <klocale.h>

class KShapeTransparencyCommand::Private
{
public:
    Private() {
    }
    ~Private() {
    }

    QList<KShape*> shapes;    ///< the shapes to set background for
    QList<qreal> oldTransparencies; ///< the old transparencies
    QList<qreal> newTransparencies; ///< the new transparencies
};

KShapeTransparencyCommand::KShapeTransparencyCommand(const QList<KShape*> &shapes, qreal transparency, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes = shapes;
    foreach(KShape *shape, d->shapes) {
        d->oldTransparencies.append(shape->transparency());
        d->newTransparencies.append(transparency);
    }

    setText(i18n("Set Opacity"));
}

KShapeTransparencyCommand::KShapeTransparencyCommand(KShape * shape, qreal transparency, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes.append(shape);
    d->oldTransparencies.append(shape->transparency());
    d->newTransparencies.append(transparency);

    setText(i18n("Set Opacity"));
}

KShapeTransparencyCommand::KShapeTransparencyCommand(const QList<KShape*> &shapes, const QList<qreal> &transparencies, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes = shapes;
    foreach(KShape *shape, d->shapes) {
        d->oldTransparencies.append(shape->transparency());
    }
    d->newTransparencies = transparencies;

    setText(i18n("Set Opacity"));
}

KShapeTransparencyCommand::~KShapeTransparencyCommand()
{
    delete d;
}

void KShapeTransparencyCommand::redo()
{
    QUndoCommand::redo();
    QList<qreal>::iterator transparencyIt = d->newTransparencies.begin();
    foreach(KShape *shape, d->shapes) {
        shape->setTransparency(*transparencyIt);
        shape->update();
        transparencyIt++;
    }
}

void KShapeTransparencyCommand::undo()
{
    QUndoCommand::undo();
    QList<qreal>::iterator transparencyIt = d->oldTransparencies.begin();
    foreach(KShape *shape, d->shapes) {
        shape->setTransparency(*transparencyIt);
        shape->update();
        transparencyIt++;
    }
}
