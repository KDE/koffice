/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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
// flake
#include "KToolManager.h"
#include "KToolManager_p.h"
#include "KToolRegistry.h"
#include "KToolProxy.h"
#include "KToolProxy_p.h"
#include "KSelection.h"
#include "KCanvasController.h"
#include "KCanvasController_p.h"
#include "KShape.h"
#include "KShapeLayer.h"
#include "KShapeRegistry.h"
#include "KShapeManager.h"
#include "KCanvasBase.h"
#include "KPointerEvent.h"
#include "tools/KCreateShapesTool.h"
#include "tools/KZoomTool_p.h"
#include "tools/KPanTool_p.h"

// Qt + kde
#include <QWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTabletEvent>
#include <QKeyEvent>
#include <QGridLayout>
#include <QDockWidget>
#include <QGraphicsWidget>
#include <QStringList>
#include <QAbstractButton>
#include <QApplication>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kaction.h>
#include <QStack>
#include <QLabel>

class CanvasData
{
public:
    CanvasData(KCanvasController *cc, const KInputDevice &id)
            : activeTool(0),
            canvas(cc),
            inputDevice(id),
            dummyToolWidget(0),
            dummyToolLabel(0) {
    }

    ~CanvasData() {
        // the dummy tool widget does not necessarily have a parent and we create it, so we delete it.
        delete dummyToolWidget;
    }

    KToolBase *activeTool;     // active Tool
    QString activeToolId;   // the id of the active Tool
    QString activationShapeId; // the shape-type (KShape::shapeId()) the activeTool 'belongs' to.
    QHash<QString, KToolBase*> allTools; // all the tools that are created for this canvas.
    QStack<QString> stack; // stack of temporary tools
    KCanvasController *const canvas;
    const KInputDevice inputDevice;
    QWidget *dummyToolWidget;  // the widget shown in the toolDocker.
    QLabel *dummyToolLabel;
};

KToolManager::Private::Private(KToolManager *qq)
    : q(qq),
    canvasData(0),
    layerEnabled(true)
{
    tabletEventTimer.setSingleShot(true);
}

KToolManager::Private::~Private()
{
    qDeleteAll(tools);
}
    // helper method.
CanvasData *KToolManager::Private::createCanvasData(KCanvasController *controller, KInputDevice device)
{
    QHash<QString, KToolBase*> origHash;
    if (canvasses.contains(controller))
        origHash = canvasses.value(controller).first()->allTools;

    bool readWrite = true;
    if (controller->canvas())
        readWrite = controller->canvas()->isReadWrite();

    QHash<QString, KToolBase*> toolsHash;
    foreach(ToolHelper *tool, tools) {
        if (tool->inputDeviceAgnostic() && origHash.contains(tool->id())) {
            // reuse ones that are marked as inputDeviceAgnostic();
            toolsHash.insert(tool->id(), origHash.value(tool->id()));
            continue;
        }
        if (! tool->canCreateTool(controller->canvas())) {
            kDebug(30006) << "Skipping the creation of tool" << tool->id();
            continue;
        }
        kDebug(30006) << "Creating tool" << tool->id() << ". Activated on:" << tool->activationShapeId() << ", prio:" << tool->priority();
        KToolBase *tl = tool->createTool(controller->canvas());
        Q_ASSERT(tl);
        tl->setReadWrite(readWrite);
        uniqueToolIds.insert(tl, tool->uniqueId());
        toolsHash.insert(tool->id(), tl);
        tl->setObjectName(tool->id());
        foreach(KAction *action, tl->actions())
            action->setEnabled(false);
        KZoomTool *zoomTool = dynamic_cast<KZoomTool*>(tl);
        if (zoomTool)
            zoomTool->setCanvasController(controller);
        KPanTool *panTool = dynamic_cast<KPanTool*>(tl);
        if (panTool)
            panTool->setCanvasController(controller);
    }
    KCreateShapesTool *createTool = dynamic_cast<KCreateShapesTool*>(toolsHash.value(KoCreateShapesTool_ID));
    Q_ASSERT(createTool);
    QString id = KShapeRegistry::instance()->keys()[0];
    createTool->setShapeId(id);

    CanvasData *cd = new CanvasData(controller, device);
    cd->allTools = toolsHash;
    return cd;
}

bool KToolManager::Private::toolCanBeUsed(const QString &activationShapeId)
{
    if (layerEnabled)
        return true;
    if (activationShapeId.endsWith(QLatin1String("/always")))
        return true;
    return false;
}

void KToolManager::Private::setup()
{
    if (tools.size() > 0)
        return;

    KShapeRegistry::instance();
    KToolRegistry *registry = KToolRegistry::instance();
    for (KGenericRegistry<KToolFactoryBase*>::const_iterator it = registry->constBegin(); 
		    it != registry->constEnd(); ++it) {
        ToolHelper *t = new ToolHelper(it.value());
        tools.append(t);
    }

    // connect to all tools so we can hear their button-clicks
    foreach(ToolHelper *tool, tools)
        connect(tool, SIGNAL(toolActivated(ToolHelper*)), q, SLOT(toolActivated(ToolHelper*)));
}

void KToolManager::Private::switchTool(KToolBase *tool, bool temporary)
{
    Q_ASSERT(tool);
    if (canvasData == 0)
        return;

    if (canvasData->activeTool == tool && tool->toolId() != KoInteractionTool_ID)
        return;

    bool newActiveTool = canvasData->activeTool != 0;

    if (newActiveTool) {
        canvasData->activeTool->repaintDecorations();
        // check if this tool is inputDeviceAgnostic and used by other devices, in which case we should not deactivate.
        QList<CanvasData*> items = canvasses[canvasData->canvas];
        foreach(CanvasData *cd, items) {
            if (cd == canvasData) continue;
            if (cd->activeTool == canvasData->activeTool) {
                newActiveTool = false;
                break;
            }
        }
    }

    if (newActiveTool) {
        foreach(KAction *action, canvasData->activeTool->actions(
                    canvasData->activeTool->isReadWrite() ? KToolBase::ReadWriteAction
                    : KToolBase::ReadOnlyAction)) {
            action->setEnabled(false);
        }
        // repaint the decorations before we deactivate the tool as it might deleted
        // data needed for the repaint
        canvasData->activeTool->deactivate();
        disconnect(canvasData->activeTool, SIGNAL(cursorChanged(const QCursor&)),
                   q, SLOT(updateCursor(const QCursor&)));
        disconnect(canvasData->activeTool, SIGNAL(activateTool(const QString &)),
                   q, SLOT(switchToolRequested(const QString &)));
        disconnect(canvasData->activeTool, SIGNAL(activateTemporary(const QString &)),
                   q, SLOT(switchToolTemporaryRequested(const QString &)));
        disconnect(canvasData->activeTool, SIGNAL(done()), q, SLOT(switchBackRequested()));
        disconnect(canvasData->activeTool, SIGNAL(statusTextChanged(const QString &)),
                   q, SIGNAL(changedStatusText(const QString &)));
    }

    canvasData->activeTool = tool;

    connect(canvasData->activeTool, SIGNAL(cursorChanged(const QCursor &)),
            q, SLOT(updateCursor(const QCursor &)));
    connect(canvasData->activeTool, SIGNAL(activateTool(const QString &)),
            q, SLOT(switchToolRequested(const QString &)));
    connect(canvasData->activeTool, SIGNAL(activateTemporary(const QString &)),
            q, SLOT(switchToolTemporaryRequested(const QString &)));
    connect(canvasData->activeTool, SIGNAL(done()), q, SLOT(switchBackRequested()));
    connect(canvasData->activeTool, SIGNAL(statusTextChanged(const QString &)),
            q, SIGNAL(changedStatusText(const QString &)));

    // emit a empty status text to clear status text from last active tool
    emit q->changedStatusText(QString());

    // we expect the tool to emit a cursor on activation.
    updateCursor(Qt::ForbiddenCursor);

    foreach(KAction *action, canvasData->activeTool->actions()) {
        action->setEnabled(true);
        // XXX: how to handle actions for non-qwidget-based canvases?
        KCanvasController *canvasControllerWidget = dynamic_cast<KCanvasController*>(canvasData->canvas);
        if (canvasControllerWidget) {
            canvasControllerWidget->addAction(action);
        }
    }

    postSwitchTool(temporary);
}

void KToolManager::Private::switchTool(const QString &id, bool temporary)
{
    Q_ASSERT(canvasData);
    if (!canvasData) return;

    if (canvasData->activeTool && temporary)
        canvasData->stack.push(canvasData->activeToolId);
    canvasData->activeToolId = id;
    KToolBase *tool = canvasData->allTools.value(id);
    if (! tool) {
        kWarning(30006) << "KToolManager::switchTool() " << (temporary ? "temporary" : "") << " got request to unknown tool: '" << id << "'";
        return;
    }

    foreach(ToolHelper *th, tools) {
        if (th->id() == id) {
            if (!toolCanBeUsed(th->activationShapeId()))
                return;
            canvasData->activationShapeId = th->activationShapeId();
            break;
        }
    }

    switchTool(tool, temporary);
}

void KToolManager::Private::postSwitchTool(bool temporary)
{
#ifndef NDEBUG
    int canvasCount = 1;
    foreach(QList<CanvasData*> list, canvasses) {
        bool first = true;
        foreach(CanvasData *data, list) {
            if (first) {
                kDebug(30006) << "Canvas" << canvasCount++;
            }
            kDebug(30006) << "  +- Tool:" << data->activeToolId  << (data == canvasData ? " *" : "");
            first = false;
        }
    }
#endif
    Q_ASSERT(canvasData);
    if (!canvasData) return;

    KToolBase::ToolActivation toolActivation;
    if (temporary)
        toolActivation = KToolBase::TemporaryActivation;
    else
        toolActivation = KToolBase::DefaultActivation;
    QSet<KShape*> shapesToOperateOn;
    if (canvasData->activeTool
            && canvasData->activeTool->canvas()
            && canvasData->activeTool->canvas()->shapeManager()) {
        KSelection *selection = canvasData->activeTool->canvas()->shapeManager()->selection();
        Q_ASSERT(selection);

        foreach(KShape *shape, selection->selectedShapes()) {
            QSet<KShape*> delegates = shape->toolDelegates();
            if (delegates.isEmpty()) { // no delegates, just the orig shape
                shapesToOperateOn << shape;
            } else {
                shapesToOperateOn += delegates;
            }
        }
    }

    Q_ASSERT(canvasData->canvas);
    if (canvasData->canvas->canvas()) {
        KCanvasBase *canvas = canvasData->canvas->canvas();
        // Caller of postSwitchTool expect this to be called to update the selected tool
        KToolProxy *tp = proxies.value(canvas);
        if (tp)
            tp->setActiveTool(canvasData->activeTool);
        canvasData->activeTool->activate(toolActivation, shapesToOperateOn);
        canvas->updateInputMethodInfo();
    } else {
        canvasData->activeTool->activate(toolActivation, shapesToOperateOn);
    }

    QMap<QString, QWidget *> optionWidgetMap = canvasData->activeTool->optionWidgets();
    if (optionWidgetMap.empty()) { // no option widget.
        QWidget *toolWidget = canvasData->dummyToolWidget;
        if (toolWidget == 0) {
            toolWidget = new QWidget();
            toolWidget->setObjectName("DummyToolWidget");
            QVBoxLayout *layout = new QVBoxLayout(toolWidget);
            layout->setMargin(3);
            canvasData->dummyToolLabel = new QLabel(toolWidget);
            layout->addWidget(canvasData->dummyToolLabel);
            layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
            toolWidget->setLayout(layout);
            canvasData->dummyToolWidget = toolWidget;
        }
        Q_ASSERT(canvasData->dummyToolLabel);
        QString title;
        foreach(ToolHelper *tool, tools) {
            if (tool->id() == canvasData->activeTool->toolId()) {
                title = tool->toolTip();
                break;
            }
        }
        canvasData->dummyToolLabel->setText(i18n("Active tool: %1", title));
        optionWidgetMap.insert(i18n("Tool Options"), toolWidget);
    }

    // Activate the actions for the currently active tool
    foreach(KAction *action, canvasData->activeTool->actions()) {
        action->setEnabled(true);
    }

    canvasData->canvas->setToolOptionWidgets(optionWidgetMap);
    emit q->changedTool(canvasData->canvas, uniqueToolIds.value(canvasData->activeTool));
}

void KToolManager::Private::toolActivated(ToolHelper *tool)
{
    Q_ASSERT(tool);
    if (!toolCanBeUsed(tool->activationShapeId()))
        return;

    Q_ASSERT(canvasData);
    if (!canvasData) return;
    KToolBase *t = canvasData->allTools.value(tool->id());
    Q_ASSERT(t);

    canvasData->activeToolId = tool->id();
    canvasData->activationShapeId = tool->activationShapeId();

    switchTool(t, false);
}

void KToolManager::Private::detachCanvas(KCanvasController *controller)
{
    Q_ASSERT(controller);
    // check if we are removing the active canvas controller
    if (canvasData && canvasData->canvas == controller) {
        KCanvasController *newCanvas = 0;
        // try to find another canvas controller beside the one we are removing
	for (QHash<KCanvasController*, QList<CanvasData*> >::const_iterator it = canvasses.constBegin(); it != canvasses.constEnd(); ++it) {
            KCanvasController *canvas = it.key();
            if (canvas != controller) {
                // yay found one
                newCanvas = canvas;
                break;
            }
        }
        if (newCanvas) {
            // activate the found canvas controller
            canvasData = canvasses.value(newCanvas).first();
            inputDevice = canvasData->inputDevice;
            canvasData->canvas->priv()->activate();
        } else {
            canvasData->canvas->setToolOptionWidgets(QMap<QString, QWidget *>());
            // as a last resort just set a blank one
            canvasData = 0;
            // and stop the event filter
            QApplication::instance()->removeEventFilter(q);
        }
    }

    KToolProxy *proxy = proxies.value(controller->canvas());
    if (proxy)
        proxy->setActiveTool(0);

    QList<KToolBase *> tools;
    foreach(CanvasData *canvasData, canvasses.value(controller)) {
        foreach(KToolBase *tool, canvasData->allTools) {
            if (! tools.contains(tool)) {
                tools.append(tool);
            }
        }
        delete canvasData;
    }
    foreach(KToolBase *tool, tools) {
        uniqueToolIds.remove(tool);
        delete tool;
    }
    canvasses.remove(controller);
    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KToolManager::Private::attachCanvas(KCanvasController *controller)
{
    Q_ASSERT(controller);
    CanvasData *cd = createCanvasData(controller, KInputDevice::mouse());
    // switch to new canvas as the active one.
    if (canvasData == 0) {
        QApplication::instance()->installEventFilter(q);
    }
    canvasData = cd;
    inputDevice = cd->inputDevice;
    QList<CanvasData*> canvasses_;
    canvasses_.append(cd);
    canvasses[controller] = canvasses_;

    KToolProxy *tp = proxies[controller->canvas()];
    if (tp)
        tp->priv()->setCanvasController(controller);

    if (cd->activeTool == 0) {
        // no active tool, so we activate the highest priority main tool
        int highestPriority = INT_MAX;
        ToolHelper * helper = 0;
        foreach(ToolHelper * th, tools) {
            if (th->toolType() == KToolFactoryBase::mainToolType()) {
                if (th->priority() < highestPriority) {
                    highestPriority = qMin(highestPriority, th->priority());
                    helper = th;
                }
            }
        }
        if (helper)
            toolActivated(helper);
    }

    Connector *connector = new Connector(controller->canvas()->shapeManager());
    connect(connector, SIGNAL(selectionChanged(QList<KShape*>)), q,
            SLOT(selectionChanged(QList<KShape*>)));
    connect(controller->canvas()->shapeManager()->selection(),
            SIGNAL(currentLayerChanged(const KShapeLayer*)),
            q, SLOT(currentLayerChanged(const KShapeLayer*)));

    canvasData->canvas->priv()->activate();

    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KToolManager::Private::movedFocus(QWidget *from, QWidget *to)
{
    Q_UNUSED(from);
    // XXX: Focus handling for non-qwidget based canvases!
    if (!canvasData) {
        return;
    }

    KCanvasController *canvasControllerWidget = dynamic_cast<KCanvasController*>(canvasData->canvas);
    if (!canvasControllerWidget) {
        return;
    }

    if (to == 0 || to == canvasControllerWidget) {
        return;
    }

    KCanvasController *newCanvas = 0;
    // if the 'to' is one of our canvasses, or one of its children, then switch.
    for (QHash<KCanvasController*, QList<CanvasData*> >::const_iterator it = canvasses.constBegin(); it != canvasses.constEnd(); ++it) {
        KCanvasController *canvas = it.key();
        if (canvasControllerWidget == to || canvas->canvas()->canvasWidget() == to) {
            newCanvas = canvas;
            break;
        }
    }

    if (newCanvas == 0)
        return;
    if (canvasData && newCanvas == canvasData->canvas)
        return;

    if (! canvasses.contains(newCanvas))
        return;
    foreach(CanvasData *data, canvasses.value(newCanvas)) {
        if (data->inputDevice == inputDevice) {
            if (canvasData) { // deactivate the old one.
                updateCursor(Qt::ArrowCursor);
                if (canvasData->activeTool) {
                    canvasData->activeTool->deactivate();
                    KToolProxy *proxy = proxies.value(canvasData->canvas->canvas());
                    Q_ASSERT(proxy);
                    proxy->setActiveTool(0);
                }
            }

            canvasData = data;
            updateCursor(canvasData->activeTool->cursor());
            canvasData->canvas->priv()->activate();
            postSwitchTool(false);
            emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
            return;
        }
    }
    // no such inputDevice for this canvas...
    canvasData = canvasses.value(newCanvas).first();
    inputDevice = canvasData->inputDevice;
    canvasData->canvas->priv()->activate();
    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KToolManager::Private::updateCursor(const QCursor &cursor)
{
    Q_ASSERT(canvasData);
    Q_ASSERT(canvasData->canvas);
    Q_ASSERT(canvasData->canvas->canvas());
    if (canvasData->canvas->canvas()->canvasWidget())
        canvasData->canvas->canvas()->canvasWidget()->setCursor(cursor);
}

void KToolManager::Private::switchBackRequested()
{
    Q_ASSERT(canvasData);
    if (!canvasData) return;

    if (canvasData->stack.isEmpty()) {
        // default to changing to the interactionTool
        switchTool(KoInteractionTool_ID, false);
        return;
    }
    switchTool(canvasData->stack.pop(), false);
}

void KToolManager::Private::selectionChanged(QList<KShape*> shapes)
{
    QList<QString> types;
    foreach(KShape *shape, shapes) {
        QSet<KShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) { // no delegates, just the orig shape
            delegates << shape;
        }

        foreach (KShape *shape2, delegates) {
            Q_ASSERT(shape2);
            if (! types.contains(shape2->shapeId())) {
                types.append(shape2->shapeId());
            }
        }
    }

    // check if there is still a shape selected the active tool can work on
    // there needs to be at least one shape for a tool without an activationShapeId
    // to work
    // if not change the current tool to the default tool
    if (!(canvasData->activationShapeId.isNull() && shapes.size() > 0)
        && canvasData->activationShapeId != "flake/always"
        && canvasData->activationShapeId != "flake/edit"
        && ! types.contains(canvasData->activationShapeId)) {
        switchTool(KoInteractionTool_ID, false);
    }

    emit q->toolCodesSelected(canvasData->canvas, types);
}

void KToolManager::Private::currentLayerChanged(const KShapeLayer *layer)
{
    kDebug(30006) << "layer changed to" << layer;

    /*
      This method implements a feature of locked layers.
      Essentially it disables all tools from doing anything on a layer that is either not editable or
      not visible.
      While this is a great feature, it makes parts of koffice unuasable due to the fact that all apps
      support layers (due to it being part of Flake and ODF) but not all apps can handle them.
      So you can end up in a situation where showcase or kword has shapes that are uneditable and
      there is no GUI to show why and no GUI to fix the situation.
      So disabling this feature until we solve this usability bug.
    */
return;

    emit q->currentLayerChanged(canvasData->canvas, layer);
    layerEnabled = layer == 0 || (layer->isEditable() && layer->isVisible());

    kDebug(30006) << "and the layer enabled is" << (layerEnabled ? "true" : "false");

    KToolProxy *proxy = proxies.value(canvasData->canvas->canvas());
    kDebug(30006) << " and the proxy is" << proxy;
    if (proxy) {
        kDebug(30006) << " set" << canvasData->activeTool << (layerEnabled ? "enabled" : "disabled");
        proxy->setActiveTool(toolCanBeUsed(canvasData->activationShapeId) ? canvasData->activeTool : 0);
    }
}

#define MSECS_TO_IGNORE_SWITCH_TO_MOUSE_AFTER_TABLET_EVENT_RECEIVED 100

void KToolManager::Private::switchInputDevice(const KInputDevice &device)
{
    Q_ASSERT(canvasData);
    if (!canvasData) return;
    if (!device.isMouse()) {
        tabletEventTimer.start(MSECS_TO_IGNORE_SWITCH_TO_MOUSE_AFTER_TABLET_EVENT_RECEIVED);
    }
    if (inputDevice == device) return;
    if (device.isMouse() && tabletEventTimer.isActive()) {
        // Ignore switch to mouse for a short time after a tablet event
        // is received, as this is likely to be either the mouse event sent
        // to a widget that doesn't accept the tablet event, or, on X11,
        // a core event sent after the tablet event.
        return;
    }
    inputDevice = device;
    QList<CanvasData*> items = canvasses[canvasData->canvas];

    // disable all actions for all tools in the all canvasdata objects for this canvas.
    foreach(CanvasData *cd, items) {
        foreach(KToolBase* tool, cd->allTools) {
            foreach(KAction* action, tool->actions()) {
                action->setEnabled(false);
            }
        }
    }

    // search for a canvasdata object for the current input device
    foreach(CanvasData *cd, items) {
        if (cd->inputDevice == device) {
            canvasData = cd;
            if (cd->activeTool == 0)
                switchTool(KoInteractionTool_ID, false);
            else {
                postSwitchTool(false);
                updateCursor(canvasData->activeTool->cursor());
            }
            canvasData->canvas->priv()->activate();
            emit q->inputDeviceChanged(device);
            emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
            return;
        }
    }

    // still here?  That means we need to create a new CanvasData instance with the current InputDevice.
    CanvasData *cd = createCanvasData(canvasData->canvas, device);
    // switch to new canvas as the active one.
    QString oldTool = canvasData->activeToolId;

    canvasData = cd;
    items.append(cd);
    canvasses[canvasData->canvas] = items;

    q->switchToolRequested(oldTool);
    emit q->inputDeviceChanged(device);
    canvasData->canvas->priv()->activate();
    emit q->changedCanvas(canvasData ? canvasData->canvas->canvas() : 0);
}

void KToolManager::Private::registerToolProxy(KToolProxy *proxy, KCanvasBase *canvas)
{
    proxies.insert(canvas, proxy);
    for (QHash<KCanvasController*, QList<CanvasData*> >::const_iterator it = canvasses.constBegin(); it != canvasses.constEnd(); ++it) {
        KCanvasController *controller = it.key();
        if (controller->canvas() == canvas) {
            proxy->priv()->setCanvasController(controller);
            break;
        }
    }
}

void KToolManager::Private::switchToolByShortcut(QKeyEvent *event)
{
    QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));

    foreach (ToolHelper *th, tools) {
        if (th->shortcut().contains(item)) {
            event->accept();
            switchTool(th->id(), false);
            return;
        }
    }
    if (event->key() == Qt::Key_Space && event->modifiers() == 0) {
        switchTool(KoPanTool_ID, true);
    } else if (event->key() == Qt::Key_Escape && event->modifiers() == 0) {
        switchTool(KoInteractionTool_ID, false);
    }
}

void KToolManager::Private::switchToolTemporaryRequested(const QString &id)
{
    switchTool(id, true);
}


// ******** KToolManager **********
KToolManager::KToolManager()
    : QObject(),
    d(new Private(this))
{
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(movedFocus(QWidget*, QWidget*)));
}

KToolManager::~KToolManager()
{
    delete d;
}

QList<KToolManager::Button> KToolManager::createToolList(KCanvasBase *canvas) const
{
    QList<KToolManager::Button> answer;
    foreach(ToolHelper *tool, d->tools) {
        if (tool->id() == KoCreateShapesTool_ID)
            continue; // don't show this one.
        if (!tool->canCreateTool(canvas))
            continue;
        Button button;
        button.button = tool->createButton();
        button.section = tool->toolType();
        button.priority = tool->priority();
        button.buttonGroupId = tool->uniqueId();
        button.visibilityCode = tool->activationShapeId();
        answer.append(button);
    }
    return answer;
}

void KToolManager::requestToolActivation(KCanvasController * controller)
{
    if (d->canvasses.contains(controller)) {
        QString activeToolId = d->canvasses.value(controller).first()->activeToolId;
        foreach(ToolHelper * th, d->tools) {
            if (th->id() == activeToolId) {
                d->toolActivated(th);
                break;
            }
        }
    }
}

KInputDevice KToolManager::currentInputDevice() const
{
    return d->inputDevice;
}

void KToolManager::registerTools(KActionCollection *ac, KCanvasController *controller)
{
    Q_ASSERT(controller);
    Q_ASSERT(ac);

    d->setup();

    if (! d->canvasses.contains(controller)) {
        kWarning(30006) << "registerTools called on a canvasController that has not been registered (yet)!";
        return;
    }
    CanvasData *cd = d->canvasses.value(controller).first();
    foreach(KToolBase *tool, cd->allTools) {
        QHash<QString, KAction*> actions = tool->actions();
        QHash<QString, KAction*>::const_iterator it(actions.constBegin());
        for (; it != actions.constEnd(); ++it) {
            ac->addAction(it.key(), it.value());
        }
    }
}

void KToolManager::addController(KCanvasController *controller)
{
    Q_ASSERT(controller);
    if (d->canvasses.keys().contains(controller))
        return;
    d->setup();
    d->attachCanvas(controller);
    connect(controller, SIGNAL(canvasRemoved(KCanvasController*)), this, SLOT(detachCanvas(KCanvasController*)));
    connect(controller, SIGNAL(canvasSet(KCanvasController*)), this, SLOT(attachCanvas(KCanvasController*)));
}

void KToolManager::removeCanvasController(KCanvasController *controller)
{
    Q_ASSERT(controller);
    d->detachCanvas(controller);
    disconnect(controller, SIGNAL(canvasRemoved(KCanvasController*)), this, SLOT(detachCanvas(KCanvasController*)));
    disconnect(controller, SIGNAL(canvasSet(KCanvasController*)), this, SLOT(attachCanvas(KCanvasController*)));
}

void KToolManager::switchToolRequested(const QString & id)
{
    Q_ASSERT(d->canvasData);
    if (!d->canvasData) return;

    while (!d->canvasData->stack.isEmpty()) // switching means to flush the stack
        d->canvasData->stack.pop();
    d->switchTool(id, false);
}

KCreateShapesTool * KToolManager::shapeCreatorTool(KCanvasBase *canvas) const
{
    Q_ASSERT(canvas);
    for (QHash<KCanvasController*, QList<CanvasData*> >::const_iterator it = d->canvasses.constBegin(); it != d->canvasses.constEnd(); ++it) {
        KCanvasController *controller = it.key();
        if (controller->canvas() == canvas) {
            KCreateShapesTool *createTool = dynamic_cast<KCreateShapesTool*>
                                             (d->canvasData->allTools.value(KoCreateShapesTool_ID));
            Q_ASSERT(createTool /* ID changed? */);
            return createTool;
        }
    }
    Q_ASSERT(0);   // this should not happen
    return 0;
}

KToolBase *KToolManager::toolById(KCanvasBase *canvas, const QString &id) const
{
    Q_ASSERT(canvas);
    for (QHash<KCanvasController*, QList<CanvasData*> >::const_iterator it = d->canvasses.constBegin(); it != d->canvasses.constEnd(); ++it) {
        KCanvasController *controller = it.key();
        if (controller->canvas() == canvas)
            return d->canvasData->allTools.value(id);
    }
    return 0;
}

KCanvasController *KToolManager::activeCanvasController() const
{
    if (! d->canvasData) return 0;
    return d->canvasData->canvas;
}

QString KToolManager::preferredToolForSelection(const QList<KShape*> &shapes)
{
    QList<QString> types;
    foreach(KShape *shape, shapes)
        if (! types.contains(shape->shapeId()))
            types.append(shape->shapeId());

    QString toolType = KoInteractionTool_ID;
    int prio = INT_MAX;
    foreach(ToolHelper *helper, d->tools) {
        if (helper->priority() >= prio)
            continue;
        if (helper->toolType() == KToolFactoryBase::mainToolType())
            continue;
        if (types.contains(helper->activationShapeId())) {
            toolType = helper->id();
            prio = helper->priority();
        }
    }
    return toolType;
}

bool KToolManager::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletEnterProximity:
    case QEvent::TabletLeaveProximity: {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent *>(event);

        KInputDevice id(tabletEvent->device(), tabletEvent->pointerType(), tabletEvent->uniqueId());
        d->switchInputDevice(id);
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        d->switchInputDevice(KInputDevice::mouse());
        break;
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

KToolManager* KToolManager::instance()
{
    K_GLOBAL_STATIC(KToolManager, s_instance)
    return s_instance;
}

QString KToolManager::activeToolId() const
{
    if (!d->canvasData) return QString();
    return d->canvasData->activeToolId;
}

void KToolManager::updateReadWrite(KCanvasController *cc, bool readWrite)
{
    if (d->canvasData && d->canvasData->activeTool
            && d->canvasData->activeTool->isReadWrite() != readWrite) {
        KToolBase *tl = d->canvasData->activeTool;
        if (readWrite) { // enable all
            foreach (KAction *action, tl->actions())
                action->setEnabled(true);
        } else { // disable all destructive actions
            QList<KAction*> actionsToEnable = tl->actions(KToolBase::ReadOnlyAction).values();
            foreach (KAction *action, tl->actions()) {
                action->setEnabled(actionsToEnable.contains(action));
            }
        }
    }
    foreach (CanvasData *data, d->canvasses.value(cc)) {
        foreach (KToolBase *tool, data->allTools) {
            tool->setReadWrite(readWrite);
        }
    }
}

KToolManager::Private *KToolManager::priv()
{
    return d;
}


#include <KToolManager.moc>
