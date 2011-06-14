/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeTraversal.h"

#include <kdebug.h>

#include <KShape.h>
#include <KShapeContainer.h>

KShape * KoShapeTraversal::nextShape(const KShape * current)
{
    return nextShapeStep(current, 0);
}

KShape * KoShapeTraversal::nextShape(const KShape * current, const QString &shapeId)
{
    KShape * next = nextShapeStep(current, 0);

    while (next != 0 && next->shapeId() != shapeId) {
        next = nextShapeStep(next, 0);
    }

    return next;
}

KShape * KoShapeTraversal::previousShape(const KShape * current)
{
    return previousShapeStep(current, 0);
}

KShape * KoShapeTraversal::previousShape(const KShape * current, const QString &shapeId)
{
    KShape * previous = previousShapeStep(current, 0);

    while (previous != 0 && previous->shapeId() != shapeId) {
        previous = previousShapeStep(previous, 0);
    }

    return previous;
}

KShape * KoShapeTraversal::last(KShape * current)
{
    KShape * last = current;
    while (const KShapeContainer * container = dynamic_cast<const KShapeContainer *>(last)) {
        QList<KShape*> shapes = container->shapes();
        if (!shapes.isEmpty()) {
            last = shapes.last();
        }
        else {
            break;
        }
    }
    return last;
}

KShape * KoShapeTraversal::nextShapeStep(const KShape * current, const KShapeContainer * parent)
{
    Q_ASSERT(current);
    if (!current) {
        return 0;
    }

    KShape * next = 0;

    if (parent) {
        const QList<KShape*> shapes = parent->shapes();
        QList<KShape*>::const_iterator it(qFind(shapes, current));
        Q_ASSERT(it != shapes.end());

        if (it == shapes.end()) {
            kWarning(30010) << "the shape is not in the list of children of his parent";
            return 0;
        }

        ++it;
        if (it != shapes.end()) {
            next = *it;
        }
        else {
            KShapeContainer * currentParent = parent->parent();
            next = currentParent ? nextShapeStep(parent, currentParent) : 0;
        }
    }
    else {
        if (const KShapeContainer * container = dynamic_cast<const KShapeContainer *>(current)) {
            QList<KShape*> shapes = container->shapes();
            if (!shapes.isEmpty()) {
                next = shapes[0];
            }
        }

        if (next == 0) {
            KShapeContainer * currentParent = current->parent();
            next = currentParent ? nextShapeStep(current, currentParent) : 0;
        }
    }

    return next;
}

KShape * KoShapeTraversal::previousShapeStep(const KShape * current, const KShapeContainer * parent)
{
    Q_ASSERT(current);
    if (!current) {
        return 0;
    }

    KShape * previous = 0;

    if (parent) {
        if (previous == 0) {
            const QList<KShape*> shapes = parent->shapes();
            QList<KShape*>::const_iterator it(qFind(shapes, current));
            Q_ASSERT(it != shapes.end());

            if (it == shapes.end()) {
                kWarning(30010) << "the shape is not in the list of children of his parent";
                return 0;
            }

            if (it != shapes.begin()) {
                --it;
                previous = last(*it);
            }
            else {
                previous = current->parent();
            }
        }
    }
    else {
        KShapeContainer * currentParent = current->parent();
        previous = currentParent ? previousShapeStep(current, currentParent) : 0;
    }

    return previous;
}
