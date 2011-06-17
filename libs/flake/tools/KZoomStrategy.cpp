/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
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

#include "KZoomStrategy_p.h"
#include "KShapeRubberSelectStrategy_p.h"
#include "KZoomTool_p.h"
#include "KCanvasBase.h"
#include "KCanvasController.h"
#include "KViewConverter.h"

#include <kdebug.h>

KZoomStrategy::KZoomStrategy(KZoomTool *tool, KCanvasController *controller, const QPointF &clicked)
        : KShapeRubberSelectStrategy(tool, clicked, false),
        m_controller(controller),
        m_forceZoomOut(false)
{
}

void KZoomStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_D(KShapeRubberSelectStrategy);
    QRect pixelRect = m_controller->canvas()->viewConverter()->documentToView(d->selectedRect()).toRect();

    bool m_zoomOut = m_forceZoomOut;
    if (modifiers & Qt::ControlModifier) {
        m_zoomOut = !m_zoomOut;
    }
    if (m_zoomOut) {
        m_controller->zoomOut(pixelRect.center());
    } else if (pixelRect.width() > 5 && pixelRect.height() > 5) {
        m_controller->zoomTo(pixelRect);
    } else {
        m_controller->zoomIn(pixelRect.center());
    }
}

void KZoomStrategy::cancelInteraction()
{
    Q_D(KShapeRubberSelectStrategy);
    d->tool->repaintDecorations();
    d->tool->canvas()->updateCanvas(d->selectedRect().toRect().normalized());
}

void KZoomStrategy::forceZoomOut()
{
    m_forceZoomOut = true;
}

void KZoomStrategy::forceZoomIn()
{
    m_forceZoomOut = false;
}
