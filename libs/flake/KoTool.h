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
#ifndef KOTOOL_H
#define KOTOOL_H

#include <QString>
#include <QObject>
#include <QCursor>

#include <flake_export.h>

#include <KoCanvasResourceProvider.h>

class KoCanvasBase;
class KoPointerEvent;
class KoViewConverter;
class KoToolSelection;

class QKeyEvent;
class QWidget;
class QPainter;

/**
 * Abstract base class for all tools. Tools can create or manipulate
 * flake shapes, canvas state or any other thing that a user may wish
 * to do to his document or his view on a document with a pointing
 * device.
 *
 * There exists an instance of every tool for every pointer device.
 * These instances are managed by the toolmanager..
 */
class FLAKE_EXPORT KoTool : public QObject
{
    Q_OBJECT
public:

    /**
     * Constructor, normally only called by the factory (see KoToolFactory)
     * @param canvas the canvas interface this tool will work for.
     */
    explicit KoTool(KoCanvasBase *canvas );
    virtual ~KoTool();

public:

    /**
     * request a repaint of the decorations to be made. This triggers
     * an update call on the canvas, but does not paint directly.
     */
    virtual void repaintDecorations() {};

public slots:
    /**
     * This method is called when this tool instance is activated.
     * For any main window there is only one tool active at a time, which then gets all
     * user input.  Switching between tools will call deactivate on one and activate on the
     * new tool allowing the tool to flush items (like a selection) when it is not in use.
     * <p>There is one case where two tools are activated at the same.  This is the case
     * where one tool delegates work to another temporarily.  For example, while shift is
     * being held down.  The second tool will get activated with temporary=true and
     * it should emit sigDone() when the state that activated it is ended.
     * <p>One of the important tasks of activate is to call useCursor()
     *
     * @param temporary if true, this tool is only temporarily actived
     *                  and should emit sigDone when it is done.
     * @see deactivate()
     */
    virtual void activate(bool temporary = false);

    /**
     * This method is called whenever this tool is no longer the
     * active tool
     * @see activate()
     */
    virtual void deactivate();

    /**
     * This method is called whenever a property in the resource
     * provider associated with the canvas this tool belongs to
     * changes. An example is currently selected foreground color.
     */
    virtual void resourceChanged( KoCanvasResource::EnumCanvasResource key, const QVariant & res );

public:

    /**
     * Return if dragging (moving with the mouse down) to the edge of a canvas should scroll the
     * canvas (default is true).
     * @return if this tool wants mouse events to cause scrolling of canvas.
     */
    virtual bool wantsAutoScroll();

    /**
     * Called by the canvas to paint any decorations that the tool deems needed.
     * The painter has the top left of the canvas as its origin.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     */
    virtual void paint( QPainter &painter, KoViewConverter &converter ) = 0;

    /**
     * Return the option widget for this tool. Create it if it
     * does not exist yet. If the tool does not have an option widget,
     * this method return 0. (After discussion with Thomas, who prefers
     * the toolmanager to handle that case.)
     *
     * @see m_optionWidget
     */
    QWidget * optionWidget();

    /**
     * Quick help is a short help text about the way the tool functions.
     */
    virtual QString quickHelp() const { return ""; }

    /**
     * Returns the internal selection option of this tool.
     * Each tool can have a selection which is private to that tool and the specified shape that it comes with.
     * The default returns 0.
     */
    virtual KoToolSelection* selection() { return 0; }

public: // Events

    /**
     * Called when (one of) the mouse or stylus buttons is pressed.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus press
     */
    virtual void mousePressEvent( KoPointerEvent *event ) = 0;

    /**
     * Called when (one of) the mouse or stylus buttons is double clicked.
     * Implementors should call event->ignore() if they do not actually use the event.
     * Default implementation ignores this event.
     * @param event state and reason of this mouse or stylus press
     */
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );

    /**
     * Called when the mouse or stylus moved over the canvas.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus move
     */
    virtual void mouseMoveEvent( KoPointerEvent *event ) = 0;

    /**
     * Called when (one of) the mouse or stylus buttons is released.
     * Implementors should call event->ignore() if they do not actually use the event.
     * @param event state and reason of this mouse or stylus release
     */
    virtual void mouseReleaseEvent( KoPointerEvent *event ) = 0;

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
    virtual void wheelEvent ( KoPointerEvent * event );

signals:

    /**
     * Emitted when this tool wants itself to be replaced by another tool.
     * The id it gives is the 'id' part of a KoID instance that is linked to the
     * specified tool.
     *
     * @param id the identification of the desired tool
     */
    void sigActivateTool(const QString &id );

    /**
     * Emitted when this tool wants itself to temporarily be replaced by another tool.
     * For instance, a paint tool could desire to be
     * temporarily replaced by a pan tool which could be temporarily
     * replaced by a colorpicker.
     * @param id the identification of the desired tool
     */
    void sigActivateTemporary(const QString & id);

    /**
     * Emitted when the tool has been temporarily activated and wants
     * to notify the world that it's done.
     */
    void sigDone();

    /**
     * Emitted by useCursor() when the cursor to display on the canvas is changed.
     * The KoToolManager should connect to this signal to handle cursors further.
     */
    void sigCursorChanged(QCursor cursor);



protected:
    /**
     * Classes inheriting from this one can call this method to signify which cursor
     * the tool wants to display at this time.  Logical place to call it is after an
     * incoming event has been handled.
     * @param cursor the new cursor. If this is the same as the previously set cursor
     *   this call will not do anything.
     * @param force if true the cursor will be set no matter what.
     */
    void useCursor(QCursor cursor, bool force=false);

    /**
     * Reimplement this if your tool actually has an option widget.
     * Sets the option widget to 0 by default.
     */
    virtual QWidget *  createOptionWidget();

protected:

    KoCanvasBase * const m_canvas; ///< the canvas interface this tool will work for.

private:
    KoTool();
    KoTool(const KoTool&);
    KoTool& operator=(const KoTool&);

    QWidget * m_optionWidget; ///< the optionwidget this tool will show in the option widget palette
    QCursor m_previousCursor;
};

#endif /* KOTOOL_H */
