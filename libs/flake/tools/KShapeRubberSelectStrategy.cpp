/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#include "KShapeRubberSelectStrategy.h"
#include "KShapeRubberSelectStrategy_p.h"
#include "KoViewConverter.h"

#include <QPainter>

#include "KShapeManager.h"
#include "KSelection.h"
#include "KCanvasBase.h"

KShapeRubberSelectStrategy::KShapeRubberSelectStrategy(KoToolBase *tool, const QPointF &clicked, bool useSnapToGrid)
    : KInteractionStrategy(*(new KShapeRubberSelectStrategyPrivate(tool)))
{
    Q_D(KShapeRubberSelectStrategy);
    d->snapGuide->enableSnapStrategies(KSnapGuide::GridSnapping);
    d->snapGuide->enableSnapping(useSnapToGrid);

    d->selectRect = QRectF(d->snapGuide->snap(clicked, 0), QSizeF(0, 0));
}

void KShapeRubberSelectStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KShapeRubberSelectStrategy);
    painter.setRenderHint(QPainter::Antialiasing, false);

    QColor selectColor(Qt::blue);   // TODO make configurable
    selectColor.setAlphaF(0.5);
    QBrush sb(selectColor, Qt::SolidPattern);
    painter.setPen(QPen(sb, 0));
    painter.setBrush(sb);
    QRectF paintRect = converter.documentToView(d->selectedRect());
    paintRect = paintRect.normalized();
    paintRect.adjust(0., -0.5, 0.5, 0.);
    if (painter.hasClipping())
        paintRect = paintRect.intersect(painter.clipRegion().boundingRect());
    painter.drawRect(paintRect);
}

void KShapeRubberSelectStrategy::handleMouseMove(const QPointF &p, Qt::KeyboardModifiers modifiers)
{
    Q_D(KShapeRubberSelectStrategy);
    QPointF point = d->snapGuide->snap(p, modifiers);
    if ((modifiers & Qt::AltModifier) != 0) {
        d->tool->canvas()->updateCanvas(d->selectedRect());
        d->selectRect.moveTopLeft(d->selectRect.topLeft() - (d->lastPos - point));
        d->lastPos = point;
        d->tool->canvas()->updateCanvas(d->selectedRect());
        return;
    }
    d->lastPos = point;
    QPointF old = d->selectRect.bottomRight();
    d->selectRect.setBottomRight(point);
    /*
        +---------------|--+
        |               |  |    We need to figure out rects A and B based on the two points. BUT
        |          old  | A|    we need to do that even if the points are switched places
        |             \ |  |    (i.e. the rect got smaller) and even if the rect is mirrored
        +---------------+  |    in either the horizontal or vertical axis.
        |       B          |
        +------------------+
                            `- point
    */
    QPointF x1 = old;
    x1.setY(d->selectRect.topLeft().y());
    qreal h1 = point.y() - x1.y();
    qreal h2 = old.y() - x1.y();
    QRectF A(x1, QSizeF(point.x() - x1.x(), point.y() < d->selectRect.top() ? qMin(h1, h2) : qMax(h1, h2)));
    A = A.normalized();
    d->tool->canvas()->updateCanvas(A);

    QPointF x2 = old;
    x2.setX(d->selectRect.topLeft().x());
    qreal w1 = point.x() - x2.x();
    qreal w2 = old.x() - x2.x();
    QRectF B(x2, QSizeF(point.x() < d->selectRect.left() ? qMin(w1, w2) : qMax(w1, w2), point.y() - x2.y()));
    B = B.normalized();
    d->tool->canvas()->updateCanvas(B);
}

void KShapeRubberSelectStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_D(KShapeRubberSelectStrategy);
    Q_UNUSED(modifiers);
    KSelection * selection = d->tool->canvas()->shapeManager()->selection();
    QList<KShape *> shapes(d->tool->canvas()->shapeManager()->shapesAt(d->selectRect));
    foreach(KShape * shape, shapes) {
        if (!(shape->isSelectable() && shape->isVisible()))
            continue;
        selection->select(shape);
    }
    d->tool->repaintDecorations();
    d->tool->canvas()->updateCanvas(d->selectedRect());
}

QUndoCommand *KShapeRubberSelectStrategy::createCommand(QUndoCommand *)
{
    return 0;
}
