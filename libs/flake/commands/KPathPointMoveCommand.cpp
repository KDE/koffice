/* This file is part of the KDE project
 * Copyright (C) 2006,2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KPathPointMoveCommand.h"
#include "KPathPoint.h"
#include <klocale.h>

class KPathPointMoveCommandPrivate
{
public:
    KPathPointMoveCommandPrivate() : undoCalled(true) { }
    void applyOffset(qreal factor);

    bool undoCalled; // this command stores diffs; so calling undo twice will give wrong results. Guard against that.
    QMap<KPathPointData, QPointF > points;
    QSet<KoPathShape*> paths;
};


KPathPointMoveCommand::KPathPointMoveCommand(const QList<KPathPointData> &pointData, const QPointF &offset, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KPathPointMoveCommandPrivate())
{
    setText(i18n("Move points"));

    foreach (const KPathPointData &data, pointData) {
        if (!d->points.contains(data)) {
            d->points[data] = offset;
            d->paths.insert(data.pathShape);
        }
    }
}

KPathPointMoveCommand::KPathPointMoveCommand(const QList<KPathPointData> &pointData, const QList<QPointF> &offsets, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KPathPointMoveCommandPrivate())
{
    Q_ASSERT(pointData.count() == offsets.count());

    setText(i18n("Move points"));

    uint dataCount = pointData.count();
    for (uint i = 0; i < dataCount; ++i) {
        const KPathPointData & data = pointData[i];
        if (!d->points.contains(data)) {
            d->points[data] = offsets[i];
            d->paths.insert(data.pathShape);
        }
    }
}

KPathPointMoveCommand::~KPathPointMoveCommand()
{
    delete d;
}

void KPathPointMoveCommand::redo()
{
    QUndoCommand::redo();
    if (! d->undoCalled)
        return;

    d->applyOffset(1.0);
    d->undoCalled = false;
}

void KPathPointMoveCommand::undo()
{
    QUndoCommand::undo();
    if (d->undoCalled)
        return;

    d->applyOffset(-1.0);
    d->undoCalled = true;
}

void KPathPointMoveCommandPrivate::applyOffset(qreal factor)
{
    foreach (KoPathShape *path, paths) {
        // repaint old bounding rect
        path->update();
    }

    QMap<KPathPointData, QPointF>::iterator it(points.begin());
    for (; it != points.end(); ++it) {
        KoPathShape *path = it.key().pathShape;
        // transform offset from document to shape coordinate system
        QPointF shapeOffset = path->documentToShape(factor*it.value()) - path->documentToShape(QPointF());
        QTransform matrix;
        matrix.translate(shapeOffset.x(), shapeOffset.y());

        KPathPoint *p = path->pointByIndex(it.key().pointIndex);
        if (p)
            p->map(matrix, true);
    }

    foreach (KoPathShape *path, paths) {
        path->normalize();
        // repaint new bounding rect
        path->update();
    }
}
