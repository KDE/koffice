/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>
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

#include "KPanTool_p.h"
#include "KToolBase_p.h"
#include "KPointerEvent.h"
#include "KCanvasBase.h"
#include "KCanvasController.h"
#include "KoViewConverter.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <kdebug.h>

KPanTool::KPanTool(KCanvasBase *canvas)
        : KToolBase(canvas),
        m_controller(0),
        m_temporary(false)
{
}

bool KPanTool::wantsAutoScroll() const
{
    return false;
}

void KPanTool::mousePressEvent(KPointerEvent *event)
{
    m_lastPosition = documentToViewport(event->point);
    event->accept();
    setCursor(QCursor(Qt::ClosedHandCursor));
}

void KPanTool::mouseMoveEvent(KPointerEvent *event)
{
    Q_ASSERT(m_controller);
    if (event->buttons() == 0)
        return;
    event->accept();

    QPointF actualPosition = documentToViewport(event->point);
    QPointF distance(m_lastPosition - actualPosition);
    m_controller->pan(distance.toPoint());

    m_lastPosition = actualPosition;
}

void KPanTool::mouseReleaseEvent(KPointerEvent *event)
{
    event->accept();
    setCursor(QCursor(Qt::OpenHandCursor));
    if (m_temporary)
        emit done();
}

void KPanTool::keyPressEvent(QKeyEvent *event)
{
    KCanvasController *canvasControllerWidget = dynamic_cast<KCanvasController*>(m_controller);
    if (!canvasControllerWidget) {
        return;
    }
    switch (event->key()) {
        case Qt::Key_Up:
            m_controller->pan(QPoint(0, -canvasControllerWidget->verticalScrollBar()->singleStep()));
            break;
        case Qt::Key_Down:
            m_controller->pan(QPoint(0, canvasControllerWidget->verticalScrollBar()->singleStep()));
            break;
        case Qt::Key_Left:
            m_controller->pan(QPoint(-canvasControllerWidget->horizontalScrollBar()->singleStep(), 0));
            break;
        case Qt::Key_Right:
            m_controller->pan(QPoint(canvasControllerWidget->horizontalScrollBar()->singleStep(), 0));
            break;
    }
    event->accept();
}

void KPanTool::activate(ToolActivation toolActivation, const QSet<KShape*> &)
{
    if (m_controller == 0) {
        emit done();
        return;
    }
    m_temporary = toolActivation == TemporaryActivation;
    setCursor(QCursor(Qt::OpenHandCursor));
}

void KPanTool::customMoveEvent(KPointerEvent * event)
{
    m_controller->pan(QPoint(-event->x(), -event->y()));
    event->accept();
}

QPointF KPanTool::documentToViewport(const QPointF &p)
{
    Q_D(KToolBase);
    QPointF viewportPoint = d->canvas->viewConverter()->documentToView(p);
    viewportPoint += d->canvas->documentOrigin();
    viewportPoint += QPoint(m_controller->canvasOffsetX(), m_controller->canvasOffsetY());

    return viewportPoint;
}
