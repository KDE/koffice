/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KZoomTool_p.h"
#include "KZoomStrategy_p.h"
#include "KoZoomToolWidget_p.h"
#include "KPointerEvent.h"
#include "KCanvasBase.h"
#include "KCanvasController.h"

#include <kstandarddirs.h>
#include <kdebug.h>

KZoomTool::KZoomTool(KCanvasBase *canvas)
        : KInteractionTool(canvas),
        m_temporary(false), m_zoomInMode(true)
{
    QPixmap inPixmap, outPixmap;
    inPixmap.load(KStandardDirs::locate("data", "koffice/icons/zoom_in_cursor.png"));
    outPixmap.load(KStandardDirs::locate("data", "koffice/icons/zoom_out_cursor.png"));
    m_inCursor = QCursor(inPixmap, 4, 4);
    m_outCursor = QCursor(outPixmap, 4, 4);
}

void KZoomTool::wheelEvent(KPointerEvent *event)
{
    // Let KCanvasController handle this
    event->ignore();
}

void KZoomTool::mouseReleaseEvent(KPointerEvent *event)
{
    KInteractionTool::mouseReleaseEvent(event);
    if (m_temporary) {
        emit KToolBase::done();
    }
}

void KZoomTool::mouseMoveEvent(KPointerEvent *event)
{
    updateCursor(event->modifiers() & Qt::ControlModifier);

    if (currentStrategy()) {
        currentStrategy()->handleMouseMove(event->point, event->modifiers());
    }
}

void KZoomTool::keyPressEvent(QKeyEvent *event)
{
    event->ignore();

    updateCursor(event->modifiers() & Qt::ControlModifier);
}

void KZoomTool::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();

    updateCursor(event->modifiers() & Qt::ControlModifier);

    KInteractionTool::keyReleaseEvent(event);
}

void KZoomTool::activate(ToolActivation toolActivation, const QSet<KShape*> &)
{
    m_temporary = toolActivation == TemporaryActivation;
    updateCursor(false);
}

void KZoomTool::mouseDoubleClickEvent(KPointerEvent *event)
{
    mousePressEvent(event);
}

KInteractionStrategy *KZoomTool::createStrategy(KPointerEvent *event)
{
    KZoomStrategy *zs = new KZoomStrategy(this, m_controller, event->point);
    if (event->button() == Qt::RightButton) {
        if (m_zoomInMode) {
            zs->forceZoomOut();
        } else {
            zs->forceZoomIn();
        }
    } else {
        if (m_zoomInMode) {
            zs->forceZoomIn();
        } else {
            zs->forceZoomOut();
        }
    }
    return zs;
}

QWidget *KZoomTool::createOptionWidget()
{
    return new KoZoomToolWidget(this);
}

void KZoomTool::setZoomInMode(bool zoomIn)
{
    m_zoomInMode = zoomIn;
    updateCursor(false);
}

void KZoomTool::updateCursor(bool swap)
{
    bool setZoomInCursor = m_zoomInMode;
    if (swap) {
        setZoomInCursor = !setZoomInCursor;
    }

    if (setZoomInCursor) {
        setCursor(m_inCursor);
    } else {
        setCursor(m_outCursor);
    }
}
