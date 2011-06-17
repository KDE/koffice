/* This file is part of the KDE project

   Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>

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

#include "KInteractionTool.h"
#include "KInteractionTool_p.h"
#include "KToolBase_p.h"
#include "KPointerEvent.h"
#include "KCanvasBase.h"
#include "KPanTool_p.h"

KInteractionTool::KInteractionTool(KCanvasBase *canvas)
    : KToolBase(*(new KInteractionToolPrivate(this, canvas)))
{
}

KInteractionTool::~KInteractionTool()
{
}

void KInteractionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KInteractionTool);
    if (d->currentStrategy)
        d->currentStrategy->paint(painter, converter);
}

void KInteractionTool::mousePressEvent(KPointerEvent *event)
{
    Q_D(KInteractionTool);
    if (d->currentStrategy) { // possible if the user presses an extra mouse button
        cancelCurrentStrategy();
        return;
    }
    d->currentStrategy = createStrategy(event);
    if (d->currentStrategy == 0)
        event->ignore();
}

void KInteractionTool::mouseMoveEvent(KPointerEvent *event)
{
    Q_D(KInteractionTool);
    d->lastPoint = event->point;
    if (d->currentStrategy)
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    else
        event->ignore();
}

void KInteractionTool::mouseReleaseEvent(KPointerEvent *event)
{
    Q_D(KInteractionTool);
    if (d->currentStrategy) {
        d->currentStrategy->finishInteraction(event->modifiers());
        QUndoCommand *command = d->currentStrategy->createCommand();
        if (command)
            d->canvas->addCommand(command);
        delete d->currentStrategy;
        d->currentStrategy = 0;
        repaintDecorations();
    } else
        event->ignore();
}

void KInteractionTool::keyPressEvent(QKeyEvent *event)
{
    Q_D(KInteractionTool);
    event->ignore();
    if (d->currentStrategy &&
            (event->key() == Qt::Key_Control ||
             event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
             event->key() == Qt::Key_Meta)) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
        event->accept();
    }
}

void KInteractionTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(KInteractionTool);
    if (d->currentStrategy == 0) { // catch all cases where no current strategy is needed
        if (event->key() == Qt::Key_Space)
            emit activateTemporary(KoPanTool_ID);
    } else if (event->key() == Qt::Key_Escape) {
        cancelCurrentStrategy();
        event->accept();
    } else if (event->key() == Qt::Key_Control ||
               event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
               event->key() == Qt::Key_Meta) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    }
}

KInteractionStrategy *KInteractionTool::currentStrategy()
{
    Q_D(KInteractionTool);
    return d->currentStrategy;
}

void KInteractionTool::cancelCurrentStrategy()
{
    Q_D(KInteractionTool);
    if (d->currentStrategy) {
        d->currentStrategy->cancelInteraction();
        delete d->currentStrategy;
        d->currentStrategy = 0;
    }
}

KInteractionTool::KInteractionTool(KInteractionToolPrivate &dd)
    : KToolBase(dd)
{
}

#include <KInteractionTool.moc>
