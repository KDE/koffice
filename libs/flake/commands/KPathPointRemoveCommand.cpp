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

#include "KPathPointRemoveCommand.h"
#include "KoSubpathRemoveCommand.h"
#include "KoShapeControllerBase.h"
#include "KoShapeController.h"
#include <klocale.h>

class KPathPointRemoveCommandPrivate
{
public:
    KPathPointRemoveCommandPrivate() : deletePoints(false) { }
    ~KPathPointRemoveCommandPrivate() {
        if (deletePoints)
            qDeleteAll(points);
    }
    QList<KPathPointData> pointDataList;
    QList<KPathPoint*> points;
    bool deletePoints;
};

QUndoCommand *KPathPointRemoveCommand::createCommand(
    const QList<KPathPointData> &pointDataList,
    KoShapeController *shapeController,
    QUndoCommand *parent)
{
    /*
     * We want to decide if we have to:
     * 1. delete only some points of a path or
     * 2. delete one or more complete subpath or
     * 3. delete a complete path
     */

    QList<KPathPointData> sortedPointData(pointDataList);
    qSort(sortedPointData);

    KPathPointData last(0, KoPathPointIndex(-1, -1));
    // add last at the end so that the point date before last will also be put in
    // the right places.
    sortedPointData.append(last);

    QList<KPathPointData> pointsOfSubpath; // points of current subpath
    QList<KPathPointData> subpathsOfPath;  // subpaths of current path
    QList<KPathPointData> pointsToDelete;  // single points to delete
    QList<KPathPointData> subpathToDelete; // single subpaths to delete
    QList<KoShape*> shapesToDelete;         // single paths to delete

    last = sortedPointData.first();

    QList<KPathPointData>::const_iterator it(sortedPointData.constBegin());
    for (; it != sortedPointData.constEnd(); ++it) {
        // check if we have come to the next subpath of the same or another path
        if (last.pathShape != it->pathShape || last.pointIndex.first != it->pointIndex.first) {
            // check if all points of the last subpath should be deleted
            if (last.pathShape->subpathPointCount(last.pointIndex.first) == pointsOfSubpath.size()) {
                // all points of subpath to be deleted -> mark subpath as to be deleted
                subpathsOfPath.append(pointsOfSubpath.first());
            } else {
                // not all points of subpath to be deleted -> add them to the delete point list
                pointsToDelete += pointsOfSubpath;
            }
            // clear the suboath point list
            pointsOfSubpath.clear();
        }

        // check if we have come to the next shape
        if (last.pathShape != it->pathShape) {
            // check if all subpath of the shape should be deleted
            if (last.pathShape->subpathCount() == subpathsOfPath.size()) {
                // all subpaths of path to be deleted -> add shape to delete shape list
                shapesToDelete.append(last.pathShape);
            } else {
                // not all subpaths of path to be deleted -> add them to delete subpath list
                subpathToDelete += subpathsOfPath;
            }
            subpathsOfPath.clear();
        }
        if (! it->pathShape)
            continue;
        // keep reference to last point
        last = *it;
        // add this point to the current subpath point list
        pointsOfSubpath.append(*it);
    }

    QUndoCommand *cmd = new QUndoCommand(i18n("Remove Points"), parent);

    if (pointsToDelete.size() > 0) {
        new KPathPointRemoveCommand(pointsToDelete, cmd);
    }
    foreach(const KPathPointData & pd, subpathToDelete) {
        new KoSubpathRemoveCommand(pd.pathShape, pd.pointIndex.first, cmd);
    }
    if (shapesToDelete.size() > 0) {
        shapeController->removeShapes(shapesToDelete, cmd);
    }

    return cmd;
}

KPathPointRemoveCommand::KPathPointRemoveCommand(const QList<KPathPointData> & pointDataList,
        QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KPathPointRemoveCommandPrivate())
{
    QList<KPathPointData>::const_iterator it(pointDataList.begin());
    for (; it != pointDataList.end(); ++it) {
        KPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
        if (point) {
            d->pointDataList.append(*it);
            d->points.append(0);
        }
    }
    qSort(d->pointDataList);
    setText(i18n("Remove Points"));
}

KPathPointRemoveCommand::~KPathPointRemoveCommand()
{
    delete d;
}

void KPathPointRemoveCommand::redo()
{
    QUndoCommand::redo();
    KPathShape * lastPathShape = 0;
    int updateBefore = d->pointDataList.size();
    for (int i = d->pointDataList.size() - 1; i >= 0; --i) {
        const KPathPointData &pd = d->pointDataList.at(i);
        pd.pathShape->update();
        d->points[i] = pd.pathShape->removePoint(pd.pointIndex);

        if (lastPathShape != pd.pathShape) {
            if (lastPathShape) {
                QPointF offset = lastPathShape->normalize();

                QTransform matrix;
                matrix.translate(-offset.x(), -offset.y());
                for (int j = i + 1; j < updateBefore; ++j) {
                    d->points.at(j)->map(matrix);
                }
                lastPathShape->update();
                updateBefore = i + 1;
            }
            lastPathShape = pd.pathShape;
        }
    }

    if (lastPathShape) {
        QPointF offset = lastPathShape->normalize();

        QTransform matrix;
        matrix.translate(-offset.x(), -offset.y());
        for (int j = 0; j < updateBefore; ++j) {
            d->points.at(j)->map(matrix);
        }
        lastPathShape->update();
    }

    d->deletePoints = true;
}

void KPathPointRemoveCommand::undo()
{
    QUndoCommand::undo();
    KPathShape * lastPathShape = 0;
    for (int i = 0; i < d->pointDataList.size(); ++i) {
        const KPathPointData &pd = d->pointDataList.at(i);
        if (lastPathShape && lastPathShape != pd.pathShape) {
            lastPathShape->normalize();
            lastPathShape->update();
        }
        pd.pathShape->insertPoint(d->points[i], pd.pointIndex);
        lastPathShape = pd.pathShape;
    }
    if (lastPathShape) {
        lastPathShape->normalize();
        lastPathShape->update();
    }
    d->deletePoints = false;
}
