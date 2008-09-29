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

#include "KoZoomStrategy.h"
#include "KoZoomTool.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"

#include <kdebug.h>

KoZoomStrategy::KoZoomStrategy(KoZoomTool *tool, KoCanvasController *controller, const QPointF &clicked)
        : KoShapeRubberSelectStrategy(tool, controller->canvas(), clicked, false),
        m_controller(controller),
        m_forceZoomOut(false)
{
}

void KoZoomStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    QRect pixelRect = m_controller->canvas()->viewConverter()->documentToView(selectRect()).toRect();
    pixelRect.translate(m_controller->canvas()->documentOrigin());
    if (m_forceZoomOut || modifiers & Qt::ControlModifier)
        m_controller->zoomOut(pixelRect.center());
    else if (pixelRect.width() > 5 && pixelRect.height() > 5)
        m_controller->zoomTo(pixelRect);
    else
        m_controller->zoomIn(pixelRect.center());
}

void KoZoomStrategy::cancelInteraction()
{
    m_parent->repaintDecorations();
    m_canvas->updateCanvas(selectRect().toRect().normalized());
}

void KoZoomStrategy::forceZoomOut()
{
    m_forceZoomOut = true;
}
