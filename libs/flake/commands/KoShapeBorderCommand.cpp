/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeBorderCommand.h"
#include "KShape.h"
#include "KoShapeBorderBase.h"

#include <klocale.h>

class KoShapeBorderCommand::Private
{
public:
    Private() {}
    ~Private()
    {
        foreach(KoShapeBorderBase* border, oldBorders) {
            if (border && !border->deref())
                delete border;
        }
    }

    void addOldBorder(KoShapeBorderBase * oldBorder)
    {
        if (oldBorder)
            oldBorder->ref();
        oldBorders.append(oldBorder);
    }

    void addNewBorder(KoShapeBorderBase * newBorder)
    {
        if (newBorder)
            newBorder->ref();
        newBorders.append(newBorder);
    }

    QList<KShape*> shapes;                ///< the shapes to set border for
    QList<KoShapeBorderBase*> oldBorders; ///< the old borders, one for each shape
    QList<KoShapeBorderBase*> newBorders; ///< the new borders to set
};

KoShapeBorderCommand::KoShapeBorderCommand(const QList<KShape*> &shapes, KoShapeBorderBase *border, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes = shapes;

    // save old borders
    foreach(KShape *shape, d->shapes) {
        d->addOldBorder(shape->border());
        d->addNewBorder(border);
    }

    setText(i18n("Set Border"));
}

KoShapeBorderCommand::KoShapeBorderCommand(const QList<KShape*> &shapes,
        const QList<KoShapeBorderBase*> &borders,
        QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    Q_ASSERT(shapes.count() == borders.count());

    d->shapes = shapes;

    // save old borders
    foreach(KShape *shape, shapes)
        d->addOldBorder(shape->border());
    foreach (KoShapeBorderBase * border, borders)
        d->addNewBorder(border);

    setText(i18n("Set Border"));
}

KoShapeBorderCommand::KoShapeBorderCommand(KShape* shape, KoShapeBorderBase *border, QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes.append(shape);
    d->addNewBorder(border);
    d->addOldBorder(shape->border());

    setText(i18n("Set Border"));
}

KoShapeBorderCommand::~KoShapeBorderCommand()
{
    delete d;
}

void KoShapeBorderCommand::redo()
{
    QUndoCommand::redo();
    QList<KoShapeBorderBase*>::iterator borderIt = d->newBorders.begin();
    foreach(KShape *shape, d->shapes) {
        shape->update();
        shape->setBorder(*borderIt);
        shape->update();
        ++borderIt;
    }
}

void KoShapeBorderCommand::undo()
{
    QUndoCommand::undo();
    QList<KoShapeBorderBase*>::iterator borderIt = d->oldBorders.begin();
    foreach(KShape *shape, d->shapes) {
        shape->update();
        shape->setBorder(*borderIt);
        shape->update();
        ++borderIt;
    }
}
