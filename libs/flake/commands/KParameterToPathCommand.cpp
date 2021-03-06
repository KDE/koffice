/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KParameterToPathCommand.h"
#include "KParameterShape.h"
#include <klocale.h>

class KParameterToPathCommandPrivate
{
public:
    ~KParameterToPathCommandPrivate() {
        qDeleteAll(copies);
    }
    void initialize();
    void copyPath(KPathShape *destination, KPathShape *source);
    QList<KParameterShape*> shapes;
    QList<KPathShape*> copies;
};

KParameterToPathCommand::KParameterToPathCommand(KParameterShape *shape, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KParameterToPathCommandPrivate())
{
    d->shapes.append(shape);
    d->initialize();
    setText(i18n("Convert to Path"));
}

KParameterToPathCommand::KParameterToPathCommand(const QList<KParameterShape*> &shapes, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KParameterToPathCommandPrivate())
{
    d->shapes = shapes;
    d->initialize();
    setText(i18n("Convert to Path"));
}

KParameterToPathCommand::~KParameterToPathCommand()
{
    delete d;
}

void KParameterToPathCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < d->shapes.size(); ++i) {
        KParameterShape *parameterShape = d->shapes.at(i);
        parameterShape->update();
        parameterShape->setParametricShape(false);
        parameterShape->update();
    }
}

void KParameterToPathCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->shapes.size(); ++i) {
        KParameterShape * parameterShape = d->shapes.at(i);
        parameterShape->update();
        parameterShape->setParametricShape(true);
        d->copyPath(parameterShape, d->copies[i]);
        parameterShape->update();
    }
}

void KParameterToPathCommandPrivate::initialize()
{
    foreach(KParameterShape *shape, shapes) {
        KPathShape *p = new KPathShape();
        copyPath(p, shape);
        copies.append(p);
    }
}

void KParameterToPathCommandPrivate::copyPath(KPathShape *destination, KPathShape *source)
{
    destination->clear();

    int subpathCount = source->subpathCount();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        int pointCount = source->subpathPointCount(subpathIndex);
        if (! pointCount)
            continue;

        KoSubpath * subpath = new KoSubpath;
        for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
            KPathPoint * p = source->pointByIndex(KoPathPointIndex(subpathIndex, pointIndex));
            KPathPoint * c = new KPathPoint(*p);
            c->setParent(destination);
            subpath->append(c);
        }
        destination->addSubpath(subpath, subpathIndex);
    }
    destination->setTransformation(source->transformation());
}
