/* This file is part of the KDE project
 *  Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
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
#ifndef K_TOOL_MANAGER
#define K_TOOL_MANAGER

#include "KInputDevice.h"
#include "flake_export.h"

#include <QObject>
#include <QCursor>
#include <QList>

class KCanvasController;
class KCanvasBase;
class KToolBase;
class KCreateShapesTool;
class KActionCollection;
class KShape;
class QToolButton;
class KShapeLayer;

/**
 * This class manages the activation and deactivation of tools for
 * each input device.
 *
 * Managing the active tool and switching tool based on various variables.
 *
 * The state of the toolbox will be the same for all views in the process so practically
 * you can say we have one toolbox per application instance (process).  Implementation
 * does not allow one widget to be in more then one view, so we just make sure the toolbox
 * is hidden in not-in-focus views.
 *
 * The ToolManager is a singleton and will manage all views in all applications that
 * are loaded in this process. This means you will have to register and unregister your view.
 * When creating your new view you should use a KCanvasController() and register that
 * with the ToolManager like this:
@code
    MyGuiWidget::MyGuiWidget() {
        m_canvasController = new KCanvasController(this);
        m_canvasController->setCanvas(m_canvas);
        KToolManager::instance()->addControllers(m_canvasController));
    }
    MyGuiWidget::~MyGuiWidget() {
        KToolManager::instance()->removeCanvasController(m_canvasController);
    }
@endcode
 *
 * For a new view that extends KoView all you need to do is implement KoView::createToolBox()
 *
 * KToolManager also keeps track of the current tool based on a
   complex set of conditions and heuristics:

   - there is one active tool per KCanvasController (and there is one KCanvasController
     per view, because this is a class with scrollbars and a zoomlevel and so on)
   - for every pointing device (determined by the unique id of tablet,
     or 0 for mice -- you may have more than one mouse attached, but
     Qt cannot distinquish between them, there is an associated tool.
   - depending on things like tablet leave/enter proximity, incoming
     mouse or tablet events and a little timer (that gets stopped when
     we know what is what), the active pointing device is determined,
     and the active tool is set accordingly.

   Nota bene: if you use KToolManager and register your canvases with
   it you no longer have to manually implement methods to route mouse,
   tablet, key or wheel events to the active tool. In fact, it's no
   longer interesting to you which tool is active; you can safely
   route the paint event through KToolProxy::paint().

   (The reason the input events are handled completely by the
   toolmanager and the paint events not is that, generally speaking,
   it's okay if the tools get the input events first, but you want to
   paint your shapes or other canvas stuff first and only then paint
   the tool stuff.)

 */
class FLAKE_EXPORT KToolManager : public QObject
{
    Q_OBJECT

public:
    /// Return the toolmanager singleton
    static KToolManager* instance();
    ~KToolManager();

    /**
     * Register actions for switching to tools at the actionCollection parameter.
     * The actions will have the text / shortcut as stated by the toolFactory.
     * If the application calls this in their KoView extending class they will have all the benefits
     * from allowing this in the menus and to allow the use to configure the shortcuts used.
     * @param ac the actionCollection that will be the parent of the actions.
     * @param controller tools registered with this controller will have all their actions added as well.
     */
    void registerTools(KActionCollection *ac, KCanvasController *controller = 0);

    /**
     * Register a new canvas controller
     * @param controller the view controller that this toolmanager will manage the tools for
     */
    void addController(KCanvasController *controller);

    /**
     * Remove a set of controllers
     * When the controller is no longer used it should be removed so all tools can be
     * deleted and stop eating memory.
     * @param controller the controller that is removed
     */
    void removeCanvasController(KCanvasController *controller);

    /// @return the active canvas controller
    KCanvasController *activeCanvasController() const;

    /**
     * Return the tool that is able to create shapes for this param canvas.
     * This is typically used by the KoShapeSelector to set which shape to create next.
     * @param canvas the canvas that is a child of a previously registered controller
     *    who's tool you want.
     * @see addController()
     */
    KCreateShapesTool *shapeCreatorTool(KCanvasBase *canvas) const;

    /**
     * Returns the tool that is able to add/edit guides for this param canvas.
     * @param canvas the canvas that is a child of a previously registered controller
     *    who's tool you want.
     * @see addController()
     */
    KToolBase *toolById(KCanvasBase *canvas, const QString &id) const;

    /// @return the currently active pointing device
    KInputDevice currentInputDevice() const;

    /**
     * For the list of shapes find out which tool is the highest priorty tool that can handle it.
     * @returns the toolId for the shapes.
     * @param shapes a list of shapes, a selection for example, that is used to look for the tool.
     */
    QString preferredToolForSelection(const QList<KShape*> &shapes);

    /// Struct for the createToolList return type.
    struct Button {
        QToolButton *button;///< a newly created button.
        QString section;        ///< The section the button wants to be in.
        int priority;           ///< Lower number (higher priority) means coming first in the section.
        int buttonGroupId;      ///< An unique ID for this button as passed by changedTool()
        QString visibilityCode; ///< This button should become visible when we emit this string in toolCodesSelected()
    };

    /**
     * Create a list of buttons to represent all the tools.
     * @returns a list of Buttons.
     * This is a factory method for buttons and meta information on the button to better display the button.
     */
    QList<Button> createToolList(KCanvasBase *canvas) const;

    /// Request tool activation for the given canvas controller
    void requestToolActivation(KCanvasController *controller);

    /// Returns the toolId of the currently active tool
    QString activeToolId() const;

    void updateReadWrite(KCanvasController *cc, bool readWrite);

    class Private;
    /**
     * \internal return the private object for the toolmanager.
     */
    KToolManager::Private *priv();

    /// reimplemented from QObject
    virtual bool eventFilter(QObject *object, QEvent *event);

public slots:
    /**
     * Request switching tool
     * @param id the id of the tool
     */
    void switchToolRequested(const QString &id);

signals:
    /**
     * Emitted when a new tool was selected or became active.
     * @param canvas the currently active canvas.
     * @param uniqueToolId a random but unique code for the new tool.
     */
    void changedTool(KCanvasController *canvas, int uniqueToolId);

    /**
     * Emitted after the selection changed to state which unique shape-types are now
     * in the selection.
     * @param canvas the currently active canvas.
     * @param types a list of string that are the shape types of the selected objects.
     */
    void toolCodesSelected(const KCanvasController *canvas, const QList<QString> &types);

    /**
     * Emitted after the current layer changed either its properties or to a new layer.
     * @param canvas the currently active canvas.
     * @param layer the layer that is selected.
     */
    void currentLayerChanged(const KCanvasController *canvas, const KShapeLayer *layer);

    /**
     * Every time a new input device gets used by a tool, this event is emitted.
     * @param device the new input device that the user picked up.
     */
    void inputDeviceChanged(const KInputDevice &device);

    /**
     * Emitted whenever the active canvas changed.
     * @param canvas the new activated canvas (might be 0)
     */
    void changedCanvas(const KCanvasBase *canvas);

    /**
     * Emitted whenever the active tool changes the status text.
     * @param statusText the new status text
     */
    void changedStatusText(const QString &statusText);

private:
    KToolManager();
    KToolManager(const KToolManager&);
    KToolManager operator=(const KToolManager&);

    Q_PRIVATE_SLOT(d, void toolActivated(ToolHelper *tool))
    Q_PRIVATE_SLOT(d, void detachCanvas(KCanvasController *controller))
    Q_PRIVATE_SLOT(d, void attachCanvas(KCanvasController *controller))
    Q_PRIVATE_SLOT(d, void movedFocus(QWidget *from, QWidget *to))
    Q_PRIVATE_SLOT(d, void updateCursor(const QCursor &cursor))
    Q_PRIVATE_SLOT(d, void switchBackRequested())
    Q_PRIVATE_SLOT(d, void selectionChanged(QList<KShape*> shapes))
    Q_PRIVATE_SLOT(d, void currentLayerChanged(const KShapeLayer *layer))
    Q_PRIVATE_SLOT(d, void switchToolTemporaryRequested(const QString &id))

    Private *const d;
};

#endif
