/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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

#include "ShapeMoveStrategy.h"
#include "SelectionDecorator.h"

#include <KCanvasBase.h>
#include <KShapeManager.h>
#include <KShapeContainer.h>
#include <KShapeContainerModel.h>
#include <KResourceManager.h>
#include <commands/KShapeMoveCommand.h>
#include <KSnapGuide.h>
#include <KPointerEvent.h>
#include <KToolBase.h>
#include <KLocale>

ShapeMoveStrategy::ShapeMoveStrategy(KToolBase *tool, const QPointF &clicked)
    : KInteractionStrategy(tool),
    m_start(clicked)
{
    QList<KShape*> selectedShapes = tool->canvas()->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection);
    QRectF boundingRect;
    foreach(KShape *shape, selectedShapes) {
        if (! shape->isEditable())
            continue;
        m_selectedShapes << shape;
        m_previousPositions << shape->position();
        m_newPositions << shape->position();
        boundingRect = boundingRect.unite( shape->boundingRect() );
    }
    KSelection * selection = tool->canvas()->shapeManager()->selection();
    m_initialOffset = selection->absolutePosition( SelectionDecorator::hotPosition() ) - m_start;
    m_initialSelectionPosition = selection->position();
    tool->canvas()->snapGuide()->setIgnoredShapes( selection->selectedShapes( KoFlake::FullSelection ) );

    setStatusText(i18n("Press ALT to hold x- or y-position."));
}

void ShapeMoveStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if (m_selectedShapes.isEmpty())
        return;
    QPointF diff = point - m_start;

    if (modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
        // keep x or y position unchanged
        if (qAbs(diff.x()) < qAbs(diff.y()))
            diff.setX(0);
        else
            diff.setY(0);
    } else {
        QPointF positionToSnap = point + m_initialOffset;
        tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
        QPointF snappedPosition = tool()->canvas()->snapGuide()->snap(positionToSnap, modifiers);
        tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
        diff = snappedPosition - m_initialOffset - m_start;
    }

    m_diff = diff;

    moveSelection();
}

void ShapeMoveStrategy::moveSelection()
{
    Q_ASSERT(m_newPositions.count());

    int i=0;
    foreach(KShape *shape, m_selectedShapes) {
        QPointF delta = m_previousPositions.at(i) + m_diff - shape->position();
        if (shape->parent())
            shape->parent()->model()->proposeMove(shape, delta);
        tool()->canvas()->clipToDocument(shape, delta);
        QPointF newPos (shape->position() + delta);
        m_newPositions[i] = newPos;
        shape->update();
        shape->setPosition(newPos);
        shape->update();
        i++;
    }
    tool()->canvas()->shapeManager()->selection()->setPosition(m_initialSelectionPosition + m_diff);
}

QUndoCommand* ShapeMoveStrategy::createCommand(QUndoCommand *parent)
{
    tool()->canvas()->snapGuide()->reset();
    if (m_diff.x() == 0 && m_diff.y() == 0)
        return 0;
    return new KShapeMoveCommand(m_selectedShapes, m_previousPositions, m_newPositions, parent);
}

void ShapeMoveStrategy::paint( QPainter &painter, const KoViewConverter &converter)
{
    SelectionDecorator decorator (KoFlake::NoHandle, false, false);
    decorator.setSelection(tool()->canvas()->shapeManager()->selection());
    decorator.setHandleRadius( tool()->canvas()->resourceManager()->handleRadius() );
    decorator.paint(painter, converter);
}
