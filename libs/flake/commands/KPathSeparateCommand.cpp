/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KPathSeparateCommand.h"
#include "KoShapeControllerBase.h"
#include "KPathShape.h"
#include <klocale.h>

class KPathSeparateCommand::Private
{
public:
    Private(KoShapeControllerBase *c, const QList<KPathShape*> &p)
            : controller(c),
            paths(p),
            isSeparated(false) {
    }

    ~Private() {
        if (isSeparated && controller) {
            foreach(KPathShape* p, paths)
                delete p;
        } else {
            foreach(KPathShape* p, separatedPaths)
                delete p;
        }
    }

    KoShapeControllerBase *controller;
    QList<KPathShape*> paths;
    QList<KPathShape*> separatedPaths;
    bool isSeparated;
};


KPathSeparateCommand::KPathSeparateCommand(KoShapeControllerBase *controller, const QList<KPathShape*> &paths, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private(controller, paths))
{
    setText(i18n("Separate Paths"));
}

KPathSeparateCommand::~KPathSeparateCommand()
{
    delete d;
}

void KPathSeparateCommand::redo()
{
    QUndoCommand::redo();
    if (d->separatedPaths.isEmpty()) {
        foreach(KPathShape* p, d->paths) {
            QList<KPathShape*> separatedPaths;
            if (p->separate(separatedPaths))
                d->separatedPaths << separatedPaths;
        }
    }

    d->isSeparated = true;

    if (d->controller) {
        foreach(KPathShape* p, d->paths)
            d->controller->removeShape(p);
        foreach(KPathShape *p, d->separatedPaths)
            d->controller->addShape(p);
    }
    foreach(KPathShape* p, d->paths)
        p->update();
}

void KPathSeparateCommand::undo()
{
    QUndoCommand::undo();
    if (d->controller) {
        foreach(KPathShape *p, d->separatedPaths)
            d->controller->removeShape(p);
        foreach(KPathShape* p, d->paths)
            d->controller->addShape(p);
    }

    d->isSeparated = false;

    foreach(KPathShape* p, d->paths)
        p->update();
}
