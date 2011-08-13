/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#ifndef KTOOL_H
#define KTOOL_H

#include <QString>
#include <QObject>
#include <QCursor>
#include <QStringList>
#include <QtCore/QRectF>

#include "flake_export.h"

class KShape;
class KCanvasBase;
class KPointerEvent;
class KViewConverter;
class KToolSelection;
class KToolBasePrivate;

class KAction;
class QAction;
class QKeyEvent;
class QWidget;
class QPainter;
class QInputMethodEvent;

/**
 * Abstract base class for all tools.
 * In its simplest form a tool is a class that gets routed all the user input (mouse / keybard / tablet)
 * and it can handle that to implement your application behavior.
 *
 * Whereas in most applications this is done in the user interface, in flake the content the user has
 * on screen is used to select a tool and a tool implementation can therefor be much more specific and limited
 * in what kind of user input it will handle.
 *
 * As an example of how the tools concept fits in the whole of the application concept, consider this example.
 * A PDF reader allows the user to navigate over the pages either by pressing the cursor keys
 * or by clicking-and-dragging the mouse.
 * There is a special button that when instead of draging the page, the cursor will select the text on screen
 * and puts it in the clipboard.  From a interaction design point of view this means that the application has
 * two modes, a navigation mode and a selection mode.  Because based on the mode the mouse handling will have
 * different effects.
 * If we map this onto Flake and tools we will end up with 2 tools. One per mode.  The tool that is active will
 * handle the mouse events and take the actions.   The advantage of using two tools is that its got a better
 * separation of concerns.  The two ways of working are put into different classes.
 *
 * The interaction design done for flake using applications doesn't just stop at modes, it goes a step further
 * by making the content shown on screen (image, text, etc) determine which tools will be presented to the user.
 * If there are no images, the tool that handles images will never be seen by the user and thus make choosing
 * between tools easier.
 * When we mention 'content' in this context we are specifically refering to KShape inheriting classes that
 * display the content.
 *
 * The KToolBase class goes together with the KToolFactoryBase in configuring which tool to show for which
 * type of content.  Specifically the setting of the KToolFactoryBase::activationShapeId() to the relevant
 * choice.  Its also relevant to notice that a tool is matched to a factory using the toolId().
 *
 * Each tool is, as mentioned above, a mode. The user will specifically switch between those modes and although
 * there are various features that will make that as painless as possible, there is the chance that we want
 * to have different behavior while not switching away from the current tool.  For example if we are interacting
 * with a virtual bicycle then when the user clicks and drags one of the tires we may want to show an animation.
 * But if the user clicks on the chassis we may want to change the viewpoint and rotate the whole bike around.
 * This kind of situation where the actual interaction is decided only at the point of the user clicking somewhere
 * is made easy using the KInteractionTool subclass. The different sub-modes are implemented using the strategy
 * design pattern.
 *
 * Tools handle low level user input like mouse events by reimplementing methods like mousePressEvent(),
 * keyPressEvent() and wheelEvent().  A tool by default will only receive mouse events and using the Flag enums
 * and the setFlags() methods can enable more types. See for example the ToolHandleKeyEvents enum value.
 *
 * Tools handle higher level input using KAction objects and addAction() which allows tools to have keyboard
 * shortcuts and menu entries using a common implementation pattern.
 * Default actions already pre-provided are deleteSelection(), copy() and paste(), those methods can be reimplemented
 * by tools.  They go together with a KToolSelection class which allows the tool to make sure the cut/copy
 * actions are only enabled when the tool actually has a selection.
 *
 * Next to handling user input, the tool handles some more functions that allow it to be useful as well as
 * powerful.  One of those features is that the tool is allowed to paint on top of the canvas with complete
 * freedom.  This allows tools to draw hints for user interaction.  For example a circle where the user can
 * start a drag on a vector shape.
 *
 * Tools have full freedom on what they do with the user input.  Typically a tool provides a combination of
 * navigation and editing functionlity.  A text tool will allow the cursor keys to navigate but typing text
 * will modify.   When the application using the tool opens a document that can not be modified (for example
 * a document that is opened from a remote website over http) it may be disirable to disallow editing
 * the document until the user choose to save it locally.  This avoids the user putting in time in editing
 * only later to find out she can't use those modifications.
 * In KToolBase this usecase is called ReadWrite and tool implementors can use this by calling isReadWrite()
 * in their reimplementations of the event handling methods and by using the ReadWrite enum when adding
 * actions.
 *
 * There exists an instance of every tool for every pointer device.
 * These instances are managed by the KToolManager..
 */
class FLAKE_EXPORT KToolBase : public QObject
{
    Q_OBJECT
public:
    /// Option for activate()
    enum ToolActivation {
        TemporaryActivation, ///< The tool is activated temporarily and works 'in-place' of another one.
        DefaultActivation   ///< The tool is activated normally and emitting 'done' goes to the defaultTool
    };
    /// Action information
    enum ReadWrite {
        ReadOnlyAction, ///< Used for actions that do not alter the content
        ReadWriteAction ///< Used to signify actions that can change document content
    };

    /** This enum describes different flags that you can set on a tool to toggle different features in
     * the tools's behavior.
     *
     * All flags are disabled by default.
     */
    enum Flag {
        /**
         * The tool will stop automatically scrolling the viewport when the input events
         * coming into the tool cause the cursor to hit the edge of the visible area.
         */
        ToolDoesntAutoScroll = 1,
        /**
         * The tool will handle mouse move events even if there was no button depressed.
         */
        ToolMouseTracking = 2,
        /**
         * The tool will get key events when the user uses the keyboard.
         */
        ToolHandleKeyEvents = 4,
        /**
         * The tool does not want mouse events.
         */
        ToolDoesntHandleMouseEvents = 8,
        /**
         * The tool will get a call on shortcutOverride() which will allow the tool to steal any keyboard
         * event including application-defined keyboard shortcuts by accepting the event.
         */
        ToolHandleShortcutOverride = 0x10

        /*  These are ideas for expansion;
        ToolDrawsNoContent,
        ToolResourceChangeListener,
        */
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    /**
     * Constructor, normally only called by the factory (see KToolFactoryBase)
     * @param canvas the canvas interface this tool will work for.
     */
    explicit KToolBase(KCanvasBase *canvas);
    virtual ~KToolBase();

    /**
     * request a repaint of the decorations to be made. This triggers
     * an update call on the canvas, but does not paint directly.
     */
    virtual void repaintDecorations();

    /**
     * Return if dragging (moving with the mouse down) to the edge of a canvas should scroll the
     * canvas (default is true).
     * @return if this tool wants mouse events to cause scrolling of canvas.
     */
    bool wantsAutoScroll() const;

    /**
     * Called by the canvas to paint any decorations that the tool deems needed.
     * The painter has the top left of the canvas as its origin.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paint(QPainter &painter, const KViewConverter &converter) = 0;

    /**
     * Return the option widgets for this tool. Create them if they
     * do not exist yet. If the tool does not have an option widget,
     * this method return an empty list.
     *
     * @see createOptionWidgets();
     */
    QMap<QString, QWidget *> optionWidgets();

    /**
     * Returns the internal selection option of this tool.
     * Each tool can have a selection which is private to that tool and the specified shape that it comes with.
     * The default returns 0.
     */
    virtual KToolSelection *selection(); // TODO add setter and remove virtual

    /**
     * Retrieves the entire collection of actions for the tool.
     */
    QHash<QString, KAction*> actions(ReadWrite readWrite = ReadWriteAction) const;

    /**
     * Retrieve an action by name.
     * @returns the action object, or null if the name is unknown.
     */
    KAction *action(const QString &name) const;

    /**
     * Set the identifier code from the KToolFactoryBase that created this tool.
     * @param id the identifier code
     * @see KToolFactoryBase::id()
     */
    void setToolId(const QString &id);

    /**
     * get the identifier code from the KToolFactoryBase that created this tool.
     * @return the toolId.
     * @see KToolFactoryBase::id()
     */
    QString toolId() const;

    /// return the last emitted cursor
    QCursor cursor() const;

    /**
     * copies the tools selection to the clipboard.
     * The default implementation is empty to aid tools that don't have any selection.
     * @see selection()
     */
    virtual void copy() const;

    /**
     * Delete the tools selection.
     * The default implementation is empty to aid tools that don't have any selection.
     * @see selection()
     */
    virtual void deleteSelection();

    /**
     * Cut the tools selection and copy it to the clipboard.
     * The default implementation calls copy() and then deleteSelection()
     * @see copy()
     * @see deleteSelection()
     */
    virtual void cut();

    /**
     * Paste the clipboard selection.
     * A tool typically has one or more shapes selected and pasting should do something meaningful
     * for this specific shape and tool combination.  Inserting text in a text tool, for example.
     * If you reimplement this function make sure to also reimplement supportedPasteMimeTypes().
     * @return will return true if pasting succeeded. False if nothing happened.
     */
    virtual bool paste();

    /**
     * Returns the mimetypes that this tool's paste() function can handle
     * @return QStringList containing the mimetypes that's supported by paste()
     * @see setSupportedPasteMimeTypes()
     */
    QStringList supportedPasteMimeTypes() const;

    /**
     * @return A list of actions to be used for a popup.
     * @see setPopupActionList()
     */
    QList<QAction*> popupActionList() const;

    /// Returns the canvas the tool is working on
    KCanvasBase *canvas() const;

    /**
     * Calling this will turn the tool into a read/write or a read-only tool.
     * Note that upon calling this method no actions will be enabled/disabled
     * as that is the responsibility of the KToolManager. The KToolManager
     * should use this variable at next tool switch.
     *
     * @param readWrite if true all available actions will be enabled, if false
     *   only actions that do not change the content will be available.
     * @see KToolManager::updateReadWrite(), isReadWrite()
     */
    void setReadWrite(bool readWrite);

    /**
     * @return returns true if all available actions are be enabled, if false
     *   only actions that do not change the content are be available.
     * @see setReadWrite()
     */
    bool isReadWrite() const;


    /**
     * This method is called when this tool instance is activated.
     * For any main window there is only one tool active at a time, which then gets all
     * user input.  Switching between tools will call deactivate on one and activate on the
     * new tool allowing the tool to flush items (like a selection)
     * when it is not in use.
     *
     * <p>There is one case where two tools are activated at the same.  This is the case
     * where one tool delegates work to another temporarily.  For example, while shift is
     * being held down.  The second tool will get activated with temporary=true and
     * it should emit done() when the state that activated it is ended.
     * <p>One of the important tasks of activate is to call setCursor()
     *
     * @param shapes the set of shapes that are selected or suggested for editing by a
     *      selected shape for the tool to work on.  Not all shapes will be meant for this
     *      tool.
     * @param toolActivation if TemporaryActivation, this tool is only temporarily actived
     *                  and should emit done when it is done.
     * @see deactivate()
     */
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes) = 0;

    /**
     * This method is called whenever this tool is no longer the
     * active tool
     * @see activate()
     */
    virtual void deactivate();

    /**
     * Returns this tools's flags. The flags describe what configurable features of the tool are enabled and not.
     * For example, if the flags include ToolHandleKeyEvents, the item can accept key events.
     * By default, no flags are enabled.
     * @see setFlags() and setFlag().
     */
    Flags flags() const;

    /// \internal
    KToolBasePrivate *priv();

public slots:
    /**
     * This method is called whenever a property in the resource
     * provider associated with the canvas this tool belongs to
     * changes. An example is currently selected foreground color.
     */
    virtual void resourceChanged(int key, const QVariant &res);

signals:
    /**
     * Emitted when this tool wants itself to be replaced by another tool.
     *
     * @param id the identification of the desired tool
     * @see toolId(), KToolFactoryBase::id()
     */
    void activateTool(const QString &id);

    /**
     * Emitted when this tool wants itself to temporarily be replaced by another tool.
     * For instance, a paint tool could desire to be
     * temporarily replaced by a pan tool which could be temporarily
     * replaced by a colorpicker.
     * @param id the identification of the desired tool
     */
    void activateTemporary(const QString &id);

    /**
     * Emitted when the tool has been temporarily activated and wants
     * to notify the world that it's done.
     */
    void done();

    /**
     * Emitted by setCursor() when the cursor to display on the canvas is changed.
     * The KToolManager should connect to this signal to handle cursors further.
     */
    void cursorChanged(const QCursor &cursor);

    /**
     * A tool can have a selection that is copy-able, this signal is emitted when that status changes.
     * @param hasSelection is true when the tool holds selected data.
     */
    void selectionChanged(bool hasSelection);

    /**
     * Emitted when the tool wants to display a different status text
     * @param statusText the new status text
     */
    void statusTextChanged(const QString &statusText);

protected:
    /**
     * Classes inheriting from this one can call this method to signify which cursor
     * the tool wants to display at this time.  Logical place to call it is after an
     * incoming event has been handled.
     * @param cursor the new cursor.
     */
    void setCursor(const QCursor &cursor);

    /**
     * This method just relays the given text via the tools statusTextChanged signal.
     * @param statusText the new status text
     */
    void setStatusText(const QString &statusText);

    /**
     * Reimplement this if your tool actually has an option widget.
     * Sets the option widget to 0 by default.
     */
    virtual QWidget *createOptionWidget();

    /**
     * Reimplement this if your tool actually has an option widget.
     */
    virtual QMap<QString, QWidget *> createOptionWidgets();

    /**
     * Add an action under the given name to the collection.
     *
     * Inserting an action under a name that is already used for another action will replace
     * the other action in the collection.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param action The action to add.
     * @param readWrite set this to ReadOnlyAction to keep the action available on
     *      read-only documents
     */
    void addAction(const QString &name, KAction *action, ReadWrite readWrite = ReadWriteAction);

    /**
     * Set the list of actions to be used as popup menu.
     * @param list the list of actions.
     * @see popupActionList
     */
    void setPopupActionList(const QList<QAction*> &list);

    /**
    * Returns a handle grab rect at the given position.
    *
    * The position is expected to be in document coordinates. The grab sensitivity
    * canvas resource is used for the dimension of the rectangle.
    *
    * @return the handle rectangle in document coordinates
    */
    QRectF handleGrabRect(const QPointF &position) const;

    /**
    * Returns a handle paint rect at the given position.
    *
    * The position is expected to be in document coordinates. The handle radius
    * canvas resource is used for the dimension of the rectangle.
    *
    * @return the handle rectangle in document coordinates
    */
    QRectF handlePaintRect(const QPointF &position) const;

    /**
     * On every keypress the tool is allowed to steal it before anyone else sees it.
     *
     * On every keypress this method is called on the active tool to allow to tool to handle
     * the key and / or shortcut before the normal chain of event handling kicks in.
     * If the tool decides to accept the event this will cause none of the actions to be
     * matched in the whole application.
     *
     * The main reason tools may want to enable this is when they suspect an application having a KAction
     * that is triggered by a keyboard shortcut and the tool listens to the same key and never receives it.
     * For example if an application registers 'select-all' as a default shortcut and the tool does the same.
     * Another example is when the application has single-key shortcuts to activate functionality but the
     * tool is meant to be used to type text.
     * In both cases the action being registered in the application prevent the tool from getting it via
     * the normal way and the tool can override this default behavior by accepting the specic key in this
     * method.
     *
     * Tools that enable ToolHandleShortcutOverride in setFlag() will get these events offered,
     * otherwise they risk actions set in the application to steal key events it would normally get.
     */
    virtual void shortcutOverride(QKeyEvent *event);

    /**
     * Called when (one of) the mouse or stylus buttons is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus press
     */
    virtual void mousePressEvent(KPointerEvent *event);

    /**
     * Called when (one of) the mouse or stylus buttons is double clicked.
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this mouse or stylus press
     */
    virtual void mouseDoubleClickEvent(KPointerEvent *event);

    /**
     * Called when the mouse or stylus moved over the canvas.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus move
     */
    virtual void mouseMoveEvent(KPointerEvent *event);

    /**
     * Called when (one of) the mouse or stylus buttons is released.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus release
     */
    virtual void mouseReleaseEvent(KPointerEvent *event);

    /**
     * Called when a key is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this key press
     */
    virtual void keyPressEvent(QKeyEvent *event);

    /**
     * Called when a key is released
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this key release
     */
    virtual void keyReleaseEvent(QKeyEvent *event);

    /**
     * Called when the scrollwheel is used
     * Implementors should call event->ignore() if they do not actually use the event
     * @param event state of this wheel event
     */
    virtual void wheelEvent(KPointerEvent *event);

    /**
     * This method is used to query a set of properties of the tool to be
     * able to support complex input method operations as support for surrounding
     * text and reconversions.
     * Default implementation returns simple defaults, for tools that want to provide
     * a more responsive text entry experience for CJK languages it would be good to reimplemnt.
     * @param query specifies which property is queried.
     * @param converter the view converter for the current canvas.
     */
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query, const KViewConverter &converter) const;

    /**
     * Text entry of complex text, like CJK, can be made more interactive if a tool
     * implements this and the InputMethodQuery() methods.
     * Reimplementing this only provides the user with a more responsive text experience, since the
     * default implementation forwards the typed text as key pressed events.
     * @param event the input method event.
     */
    virtual void inputMethodEvent(QInputMethodEvent *event);

    /**
     * Sets the shape flags to flags.
     * All flags in flags are enabled; all flags not in flags are disabled.
     * By default, no flags are enabled. (KInteractionTool enables the ToolHandleKeyEvents flag by default)
     * @see flags() and setFlag().
     */
    void setFlags(Flags flags);

    /**
     * If enabled is true, the item flag flag is enabled; otherwise, it is disabled.
     * @see flags() and setFlags().
     */
    void setFlag(Flag flag, bool enabled = true);

    /**
     * Set the mimetypes that this tool's paste() function can handle.
     * Only if the clipboard contains a mime type that is listed here will the paste action be enabled.
     * @param mimes  QStringList containing the mimetypes that's supported by paste()
     * @see supportedPasteMimeTypes()
     */
    void setSupportedPasteMimeTypes(const QStringList &mimes);

    /// \internal
    KToolBase(KToolBasePrivate &dd);

    /// \internal
    KToolBasePrivate *d_ptr;

private:
    KToolBase();
    KToolBase(const KToolBase&);
    KToolBase& operator=(const KToolBase&);

    Q_DECLARE_PRIVATE(KToolBase)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KToolBase::Flags)

#endif
