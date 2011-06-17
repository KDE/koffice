/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#ifndef KO_TOOL_MANAGER_P
#define KO_TOOL_MANAGER_P

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QHash>

#include <kshortcut.h>

#include "KInputDevice.h"
#include "KToolManager.h"

class KToolFactoryBase;
class KShapeManager;
class KCanvasBase;
class KToolBase;
class KShape;
class KToolManager;
class KCanvasController;
class KShapeLayer;
class ToolHelper;
class CanvasData;
class QToolButton;
class KoToolProxy;

class KToolManager::Private
{
public:
    Private(KToolManager *qq);
    ~Private();

    void setup();
    void switchTool(KToolBase *tool, bool temporary);
    void switchTool(const QString &id, bool temporary);
    void postSwitchTool(bool temporary);
    bool eventFilter(QObject *object, QEvent *event);
    void toolActivated(ToolHelper *tool);

    void detachCanvas(KCanvasController *controller);
    void attachCanvas(KCanvasController *controller);
    void movedFocus(QWidget *from, QWidget *to);
    void updateCursor(const QCursor &cursor);
    void switchBackRequested();
    void selectionChanged(QList<KShape*> shapes);
    void currentLayerChanged(const KShapeLayer *layer);
    void switchToolTemporaryRequested(const QString &id);
    CanvasData *createCanvasData(KCanvasController *controller, KInputDevice device);
    bool toolCanBeUsed(const QString &activationShapeId);

    /**
     * Request a switch from to the param input device.
     * This will cause the tool for that device to be selected.
     */
    void switchInputDevice(const KInputDevice &device);

    /**
     * Whenever a new tool proxy class is instantiated, it will use this method to register itself
     * so the toolManager can update it to the latest active tool.
     * @param proxy the proxy to register.
     * @param canvas which canvas the proxy is associated with; whenever a new tool is selected for that canvas,
     *        the proxy gets an update.
     */
    void registerToolProxy(KoToolProxy *proxy, KCanvasBase *canvas);

    void switchToolByShortcut(QKeyEvent *event);


    KToolManager *q;

    QList<ToolHelper*> tools; // list of all available tools via their factories.

    QHash<KToolBase*, int> uniqueToolIds; // for the changedTool signal
    QHash<KCanvasController*, QList<CanvasData*> > canvasses;
    QHash<KCanvasBase*, KoToolProxy*> proxies;

    CanvasData *canvasData; // data about the active canvas.

    KInputDevice inputDevice;
    QTimer tabletEventTimer; // Runs for a short while after any tablet event is
    // received to help correct input device detection.

    bool layerEnabled;

};

/// \internal
class ToolHelper : public QObject
{
    Q_OBJECT
public:
    explicit ToolHelper(KToolFactoryBase *tool);
    QToolButton *createButton();
    /// wrapper around KToolFactoryBase::id();
    QString id() const;
    /// wrapper around KToolFactoryBase::toolTip();
    QString toolTip() const;
    /// wrapper around KToolFactoryBase::toolType();
    QString toolType() const;
    /// wrapper around KToolFactoryBase::activationShapeId();
    QString activationShapeId() const;
    /// wrapper around KToolFactoryBase::priority();
    int priority() const;
    KToolBase *createTool(KCanvasBase *canvas) const;
    int uniqueId() const {
        return m_uniqueId;
    }
    /// wrapper around KToolFactoryBase::shortcut()
    KShortcut shortcut() const;
    /// wrapper around KToolFactoryBase::inputDeviceAgnostic()
    bool inputDeviceAgnostic() const;
    /// returns true if the factory will create a tool, false if it decided to not create one in createTool().
    bool canCreateTool(KCanvasBase *canvas) const;

signals:
    /// emitted when one of the generated buttons was pressed.
    void toolActivated(ToolHelper *tool);

private slots:
    void buttonPressed();

private:
    KToolFactoryBase *m_toolFactory;
    int m_uniqueId;
};

/// \internal
/// Helper class to transform a simple signal selection changed into a signal with a parameter
class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(KShapeManager *parent);

public slots:
    void selectionChanged();

signals:
    void selectionChanged(QList<KShape*> shape);

private:
    KShapeManager *m_shapeManager;
};

#endif
