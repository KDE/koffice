/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2006-2010 Boudewijn Rempt <boud@valdyas.org>
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
#include "KToolProxy.h"
#include "KToolProxy_p.h"

#include "KToolBase.h"
#include "KToolBase_p.h"
#include "KPointerEvent.h"
#include "KInputDevice.h"
#include "KToolManager_p.h"
#include "KToolSelection.h"
#include "KCanvasBase.h"
#include "KCanvasController.h"
#include "KShapeManager.h"
#include "KShapeManager_p.h"
#include "KSelection.h"
#include "KShapeLayer.h"

#include <kdebug.h>
#include <QTimer>

KToolProxyPrivate::KToolProxyPrivate(KToolProxy *p)
    : activeTool(0),
    tabletPressed(false),
    hasSelection(false),
    controller(0),
    parent(p)
{
    scrollTimer.setInterval(100);
    mouseLeaveWorkaround = false;
}

void KToolProxyPrivate::timeout() // Auto scroll the canvas
{
    Q_ASSERT(controller);
    int offsetX = controller->canvasOffsetX();
    int offsetY = controller->canvasOffsetY();
    QRectF mouseArea(scrollEdgePoint, QSizeF(10, 10));
    mouseArea.setTopLeft(mouseArea.center());

    controller->ensureVisible(mouseArea, true);

    QPoint moved(offsetX - controller->canvasOffsetX(), offsetY - controller->canvasOffsetY());
    if (moved.x() == 0 && moved.y() == 0)
        return;
    scrollEdgePoint += moved;

    QMouseEvent event(QEvent::MouseMove, scrollEdgePoint, Qt::LeftButton, Qt::LeftButton, 0);
    KPointerEvent ev(&event, controller->canvas()->viewConverter()->viewToDocument(scrollEdgePoint));
    activeTool->priv()->mouseMoveEvent(&ev);
}

void KToolProxyPrivate::checkAutoScroll(const KPointerEvent &event)
{
    if (controller == 0) return;
    if (!activeTool) return;
    if (!activeTool->wantsAutoScroll()) return;
    if (!event.isAccepted()) return;
    if (event.buttons() != Qt::LeftButton) return;
    scrollEdgePoint = controller->canvas()->viewConverter()->documentToView(event.point).toPoint();
    if (! scrollTimer.isActive())
        scrollTimer.start();
}

void KToolProxyPrivate::selectionChanged(bool newSelection)
{
    if (hasSelection == newSelection)
        return;
    hasSelection = newSelection;
    emit parent->selectionChanged(hasSelection);
}

bool KToolProxyPrivate::isActiveLayerEditable()
{
    if (!activeTool)
        return false;

    KShapeManager * shapeManager = activeTool->canvas()->shapeManager();
    KShapeLayer * activeLayer = shapeManager->selection()->activeLayer();
    if (activeLayer && !activeLayer->isEditable())
        return false;
    return true;
}

KToolProxy::KToolProxy(KCanvasBase *canvas, QObject *parent)
        : QObject(parent),
        d(new KToolProxyPrivate(this))
{
    KToolManager::instance()->priv()->registerToolProxy(this, canvas);

    connect(&d->scrollTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

KToolProxy::~KToolProxy()
{
    delete d;
}

void KToolProxy::paint(QPainter &painter, const KViewConverter &converter)
{
    if (d->activeTool) d->activeTool->paint(painter, converter);
}

void KToolProxy::repaintDecorations()
{
    if (d->activeTool) d->activeTool->repaintDecorations();
}

void KToolProxy::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    // don't process tablet events for stylus middle and right mouse button
    // they will be re-send as mouse events with the correct button. there is no possibility to get the button from the QTabletEvent.
    if (qFuzzyIsNull(event->pressure()) && d->tabletPressed==false && event->type()!=QEvent::TabletMove) {
        //kDebug()<<"don't accept tablet event: "<< point;
        return;
    }
    else {
        // Accept the tablet events as they are useless to parent widgets and they will
        // get re-send as mouseevents if we don't accept them.
        //kDebug()<<"accept tablet event: "<< point;
        event->accept();
    }

    KInputDevice id(event->device(), event->pointerType(), event->uniqueId());
    KToolManager::instance()->priv()->switchInputDevice(id);

    KPointerEvent ev(event, point);
    switch (event->type()) {
    case QEvent::TabletPress:
        ev.setTabletButton(Qt::LeftButton);
        if (!d->tabletPressed && d->activeTool)
            d->activeTool->priv()->mousePressEvent(&ev);
        d->tabletPressed = true;
        break;
    case QEvent::TabletRelease:
        ev.setTabletButton(Qt::LeftButton);
        d->tabletPressed = false;
        d->scrollTimer.stop();
        if (d->activeTool)
            d->activeTool->priv()->mouseReleaseEvent(&ev);
        break;
    case QEvent::TabletMove:
        if (d->tabletPressed)
            ev.setTabletButton(Qt::LeftButton);
        if (d->activeTool)
            d->activeTool->priv()->mouseMoveEvent(&ev);
        d->checkAutoScroll(ev);
    default:
        ; // ignore the rest.
    }

    d->mouseLeaveWorkaround = true;
}

void KToolProxy::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KInputDevice id;
    KToolManager::instance()->priv()->switchInputDevice(id);
    d->mouseDownPoint = event->pos();

    if (d->tabletPressed) // refuse to send a press unless there was a release first.
        return;

    KPointerEvent ev(event, point);
    if (d->activeTool)
        d->activeTool->priv()->mousePressEvent(&ev);
    else
        event->ignore();
}

void KToolProxy::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KInputDevice id;
    KToolManager::instance()->priv()->switchInputDevice(id);
    if (d->activeTool == 0) {
        event->ignore();
        return;
    }

    KPointerEvent ev(event, point);
    d->activeTool->priv()->mouseDoubleClickEvent(&ev);
    if (! event->isAccepted() && event->button() == Qt::LeftButton)
        d->activeTool->canvas()->shapeManager()->priv()->suggestChangeTool(&ev);
}

void KToolProxy::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    if (d->mouseLeaveWorkaround) {
        d->mouseLeaveWorkaround = false;
        return;
    }
    KInputDevice id;
    KToolManager::instance()->priv()->switchInputDevice(id);
    if (d->activeTool == 0) {
        event->ignore();
        return;
    }

    KPointerEvent ev(event, point);
    d->activeTool->priv()->mouseMoveEvent(&ev);

    d->checkAutoScroll(ev);
}

void KToolProxy::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KInputDevice id;
    KToolManager::instance()->priv()->switchInputDevice(id);
    d->scrollTimer.stop();

    KPointerEvent ev(event, point);
    if (d->activeTool) {
        d->activeTool->priv()->mouseReleaseEvent(&ev);

        if (! event->isAccepted() && event->button() == Qt::LeftButton && event->modifiers() == 0
                && qAbs(d->mouseDownPoint.x() - event->x()) < 5
                && qAbs(d->mouseDownPoint.y() - event->y()) < 5) {
            // we potentially will change the selection
            Q_ASSERT(d->activeTool->canvas());
            KShapeManager *manager = d->activeTool->canvas()->shapeManager();
            Q_ASSERT(manager);
            // only change the selection if that will not lead to losing a complex selection
            if (manager->selection()->count() <= 1) {
                KShape *shape = manager->shapeAt(point);
                if (shape && !manager->selection()->isSelected(shape)) { // make the clicked shape the active one
                    manager->selection()->deselectAll();
                    manager->selection()->select(shape);
                    QList<KShape*> shapes;
                    shapes << shape;
                    QString tool = KToolManager::instance()->preferredToolForSelection(shapes);
                    KToolManager::instance()->switchToolRequested(tool);
                }
            }
        }
    } else {
        event->ignore();
    }
}

void KToolProxy::keyPressEvent(QKeyEvent *event)
{
    if (d->activeTool)
        d->activeTool->priv()->keyPressEvent(event);
    else
        event->ignore();
}

void KToolProxy::keyReleaseEvent(QKeyEvent *event)
{
    if (d->activeTool)
        d->activeTool->priv()->keyReleaseEvent(event);
    else
        event->ignore();
}

void KToolProxy::wheelEvent(QWheelEvent *event, const QPointF &point)
{
    KPointerEvent ev(event, point);
    if (d->activeTool)
        d->activeTool->priv()->wheelEvent(&ev);
    else
        event->ignore();
}

QVariant KToolProxy::inputMethodQuery(Qt::InputMethodQuery query, const KViewConverter &converter) const
{
    if (d->activeTool)
        return d->activeTool->priv()->inputMethodQuery(query, converter);
    return QVariant();
}

void KToolProxy::inputMethodEvent(QInputMethodEvent *event)
{
    if (d->activeTool) d->activeTool->priv()->inputMethodEvent(event);
}

KToolSelection* KToolProxy::selection()
{
    if (d->activeTool)
        return d->activeTool->selection();
    return 0;
}

void KToolProxy::setActiveTool(KToolBase *tool)
{
    if (d->activeTool)
        disconnect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    d->activeTool = tool;
    if (tool) {
        connect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
        d->selectionChanged(d->activeTool->selection() && d->activeTool->selection()->hasSelection());
        emit toolChanged(tool->toolId());
    }
}

void KToolProxyPrivate::setCanvasController(KCanvasController *c)
{
    controller = c;
}

QHash<QString, KAction*> KToolProxy::actions() const
{
    return d->activeTool ? d->activeTool->actions() : QHash<QString, KAction*>();
}

void KToolProxy::cut()
{
    // TODO maybe move checking the active layer to KPasteController ?
    if (d->activeTool && d->isActiveLayerEditable())
        d->activeTool->cut();
}

void KToolProxy::copy() const
{
    if (d->activeTool)
        d->activeTool->copy();
}

bool KToolProxy::paste()
{
    // TODO maybe move checking the active layer to KPasteController ?
    if (d->activeTool && d->isActiveLayerEditable())
        return d->activeTool->paste();
    return false;
}

QStringList KToolProxy::supportedPasteMimeTypes() const
{
    if (d->activeTool)
        return d->activeTool->supportedPasteMimeTypes();

    return QStringList();
}

QList<QAction*> KToolProxy::popupActionList() const
{
    if (d->activeTool)
        return d->activeTool->popupActionList();
    return QList<QAction*>();
}

void KToolProxy::deleteSelection()
{
    if (d->activeTool)
        return d->activeTool->deleteSelection();
}

void KToolProxy::processEvent(QEvent *e) const
{
    if (e->type() == QEvent::ShortcutOverride && d->activeTool) {
        d->activeTool->priv()->shortcutOverride(static_cast<QKeyEvent*>(e));
    }
}

KToolProxyPrivate *KToolProxy::priv()
{
    return d;
}

#include <KToolProxy.moc>
