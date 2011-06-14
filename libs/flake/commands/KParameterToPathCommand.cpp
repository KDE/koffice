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

class KoParameterToPathCommandPrivate
{
public:
    ~KoParameterToPathCommandPrivate() {
        qDeleteAll(copies);
    }
    void initialize();
    void copyPath(KoPathShape *destination, KoPathShape *source);
    QList<KParameterShape*> shapes;
    QList<KoPathShape*> copies;
};

KParameterToPathCommand::KParameterToPathCommand(KParameterShape *shape, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoParameterToPathCommandPrivate())
{
    d->shapes.append(shape);
    d->initialize();
    setText(i18n("Convert to Path"));
}

KParameterToPathCommand::KParameterToPathCommand(const QList<KParameterShape*> &shapes, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoParameterToPathCommandPrivate())
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

void KoParameterToPathCommandPrivate::initialize()
{
    foreach(KParameterShape *shape, shapes) {
        KoPathShape *p = new KoPathShape();
        copyPath(p, shape);
        copies.append(p);
    }
}

void KoParameterToPathCommandPrivate::copyPath(KoPathShape *destination, KoPathShape *source)
{
    destination->clear();

    int subpathCount = source->subpathCount();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        int pointCount = source->subpathPointCount(subpathIndex);
        if (! pointCount)
            continue;

        KoSubpath * subpath = new KoSubpath;
        for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
            KoPathPoint * p = source->pointByIndex(KoPathPointIndex(subpathIndex, pointIndex));
            KoPathPoint * c = new KoPathPoint(*p);
            c->setParent(destination);
            subpath->append(c);
        }
        destination->addSubpath(subpath, subpathIndex);
    }
    destination->setTransformation(source->transformation());
}
