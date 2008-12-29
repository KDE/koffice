/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef __koView_h__
#define __koView_h__

#include <QtGui/QWidget>
#include <QtCore/QPointer>
#include <QtCore/QCustomEvent>

#include <kparts/part.h>

#include "KoChild.h"
#include "komain_export.h"
#include "KoDocumentChild.h"
#include "KoFrame.h"
#include "KoViewChild.h"

class KoDocument;
class KoMainWindow;
class KoPrintJob;
class KoViewPrivate;
class KoViewChild;
class KoDocumentChild;
class KoDockFactory;

// KOffice class but not in main module
class KoDockerManager;

// KDE classes
class KStatusBar;
class KXmlGuiWindow;
class KAction;
class KActionCollection;
namespace KParts
{
class PartManager;
class PartActivateEvent;
class PartSelectEvent;
}

// Qt classes
class QDockWidget;
class QCustomEvent;
class QToolBar;

/**
 * This class is used to display a @ref KoDocument.
 *
 * Multiple views can be attached to one document at a time.
 */
class KOMAIN_EXPORT KoView : public QWidget, public KParts::PartBase
{
    Q_OBJECT
public:
    /**
     * Creates a new view for the document. Usually you don't create views yourself
     * since the KOffice components come with their own view classes which inherit
     * KoView.
     *
     * The standard way to retrieve a KoView is to call @ref KoDocument::createView.
     *
     * @param document is the document which should be displayed in this view. This pointer
     *                 must not be zero.
     * @param parent   parent widget for this view.
     */
    explicit KoView(KoDocument *document, QWidget *parent = 0);
    /**
     * Destroys the view and unregisters at the document.
     */
    virtual ~KoView();

    /**
     *  Retrieves the document object of this view.
     */
    KoDocument *koDocument() const;

    /**
     * Tells this view that its document has got deleted (called internally)
     */
    void setDocumentDeleted();
    /**
     * @return true if the document has already got deleted.
     * This can be useful for the view destructor to know if it can
     * access the document or not.
     */
    bool documentDeleted() const;

    virtual void setPartManager(KParts::PartManager *manager);
    virtual KParts::PartManager *partManager() const;

    /**
     * Returns the action described action object. In fact only the "name" attribute
     * of @p element is of interest here. The method searches in the
     * KActionCollection of this view.
     *
     * Please notice that KoView indirectly inherits KXMLGUIClient.
     *
     * @see KXMLGUIClient
     * @see KXMLGUIClient::actionCollection
     * @see KoDocument::action
     */
    virtual QAction *action(const QDomElement &element) const;

    /**
     *  Retrieves the document that is hit. This can be an embedded document.
     *
     *  The default implementation asks @ref KoDocument::hitTest. This
     *  will iterate over all child documents to detect a hit.
     *
     *  If your koffice component has multiple pages, like for example KSpread, then the hittest
     *  may not succeed for a child that is not on the visible page. In those
     *  cases you need to reimplement this method.
     */
    virtual KoDocument *hitTest(const QPoint &pos);

    /**
     * Retrieves the left border width that is displayed around the content if
     * the view is active.
     *
     * In a spread sheet this border is for example used to display the
     * rows, while a top border is used to display the names of the cells
     * and a right and bottom border is used to display scrollbars. If the view
     * becomes inactive, then this stuff is not displayed anymore.
     *
     * KoFrame uses this border information. If an embedded document becomes active
     * then it is resized so that it has enough space to display the borders and to
     * display the same content as before the activation.
     * So if for example all of your borders are 20 pixels, then activating the embedded
     * document causes the KoView to move 20 pixels up/left and the size and width increases
     * by 20+20 pixels each.
     *
     * The default border is 0.
     */
    virtual int leftBorder() const;
    /**
     * @see #leftBorder
     */
    virtual int rightBorder() const;
    /**
     * @see #leftBorder
     */
    virtual int topBorder() const;
    /**
     * @see #leftBorder
     */
    virtual int bottomBorder() const;

    /**
     * Scales the view on the content. This does not affect the contents
     * data structures. You can use this mechanism to implement a zoom
     * for example.
     *
     * The method calls QWidget::update so that the scaled content
     * is automatically displayed.
     *
     * The default scaling is 1.0 in both orientations.
     */
    virtual void setZoom(qreal zoom);
    /**
     * @return the current scaling factor (zoom level)
     *
     * @see #setZoom
     */
    virtual qreal zoom() const;

    /**
     * Overload this function if the content will be displayed
     * on some child widget instead of the view directly.
     *
     * By default this function returns a pointer to the view.
     */
    virtual QWidget *canvas() const;

    /**
     * Overload this function if the content will be displayed
     * with an offset relative to the upper left corner
     * of the canvas widget.
     *
     * By default this function returns 0.
     */
    virtual int canvasXOffset() const;

    /**
     * Overload this function if the content will be displayed
     * with an offset relative to the upper left corner
     * of the canvas widget.
     *
     * By default this function returns 0.
     */
    virtual int canvasYOffset() const;

    /**
     * Overload this function if you need to perform some actions
     * after KoView (the part widget) is inserted into canvas.
     * You should call for example addChild(QWidget*) method
     * of QScrollView here, if canvas is a viewport of QScrollView.
     *
     * By default this function does nothing.
     */
    virtual void canvasAddChild(KoViewChild *child);

    /**
     * @return the selected child. The function returns 0 if
     *         no direct child is currently selected.
     */
    virtual KoDocumentChild *selectedChild();

    /**
     * @return the active child. The function returns 0 if
     *         no direct child is currently active.
     */
    virtual KoDocumentChild *activeChild();

    /**
     * Sets up so that autoScroll signals are emitted when the mouse pointer is outside the view
     */
    void enableAutoScroll();

    /**
     * Stops the emitting of autoScroll signals
     */
    void disableAutoScroll();

    /**
     * calls KoDocument::paintEverything()
     */
    virtual void paintEverything(QPainter &painter, const QRect &rect);

    /**
     * @return TRUE if the document @p doc is represented in this view by
     *         some KoViewChild.
     *
     * This is just a convenience function for @ref #child.
     */
    bool hasDocumentInWindow(KoDocument *doc);

    /**
     * Returns the matrix which is used by the view to transform the content.
     * Currently only scaling is supported.
     *
     * The matrix changes when calling @ref #setZoom.
     *
     * @deprecated, use applyViewTransformations / reverseViewTransformations instead.
     */
    virtual KDE_DEPRECATED QMatrix matrix() const;

    /**
     * Apply the transformations that the view makes to its contents.
     * This is used for embedded objects.
     * By default this simply applies the zoom().
     * Reimplement to add some translation if needed (e.g. to center the page)
     */
    virtual QPoint applyViewTransformations(const QPoint&) const;

    /**
     * Reverse the transformations that the view makes to its contents,
     * i.e. undo the transformations done by applyViewTransformations().
     * This is used for embedded objects.
     * By default this simply unzooms the point.
     * Reimplement to add some translation if needed (e.g. to center the page)
     */
    virtual QPoint reverseViewTransformations(const QPoint&) const;

    /**
     * Overload for QRect, usually it's not needed to reimplement this one.
     */
    virtual QRect applyViewTransformations(const QRect&) const;

    /**
     * Overload for QRect, usually it's not needed to reimplement this one.
     */
    virtual QRect reverseViewTransformations(const QRect&) const;

    /**
     * @return the KoViewChild which is responsible for the @p view or 0.
     *
     * This method does no recursion.
     */
    KoViewChild *child(KoView *view);
    /**
     * A convenience function which returns the KoViewChild which in turn holds the
     * @ref KoView that in turn holds the @p document.
     */
    KoViewChild *child(KoDocument *document);

    /**
     * In order to print the document represented by this view a new print job should
     * be constructed that is capable of doing the printing.
     * The default implementation returns 0, which silently cancels printing.
     */
    virtual KoPrintJob * createPrintJob();

    /**
     * @return the KoDockerManager which is assigned to the @ref KoMainWindow of this view
     * WARNING: this could be 0, if the main window isn't a koffice main window.
     * (e.g. it can be any KParts application).
     * or if no docker have been assigned yet. In that case create one and then use
     * Note KoDockerManager is not actually defined in the main module but rather in guiutils
     * @ref setDockerManager to assign it.
     */
    KoDockerManager * dockerManager() const;

    /**
     * use this to assign a KoDockerManager to the KoMainWindow of this view.
     * This function may not do anything if the main window isn't a koffice main window.
     * (e.g. it can be any KParts application)
     * Note KoDockerManager is not actually defined in the main module but rather in guiutils
     * @ref dockerManager to retrieve it.
     */
    void setDockerManager(KoDockerManager *);

    /**
     * @return the KoMainWindow in which this view is currently.
     * WARNING: this could be 0L, if the main window isn't a koffice main window.
     * (e.g. it can be any KParts application).
     */
    KoMainWindow * shell() const;

    /**
     * @return the KXmlGuiWindow in which this view is currently.
     * This one should never return 0L, in a KDE app.
     */
    KXmlGuiWindow* mainWindow() const;

    /**
     * @return the statusbar of the KoMainWindow in which this view is currently.
     * WARNING: this could be 0L, if the main window isn't a koffice main window.
     * (e.g. it can be any KParts application).
     */
    KStatusBar * statusBar() const;

    /**
     * This adds a widget to the statusbar for this view.
     * If you use this method instead of using statusBar() directly,
     * KoView will take care of removing the items when the view GUI is deactivated
     * and readding them when it is reactivated.
     * The parameters are the same as QStatusBar::addWidget().
     *
     * Note that you can't use KStatusBar methods (inserting text items by id).
     * But you can create a KStatusBarLabel with a dummy id instead, and use
     * it directly, to get the same look and feel.
     */
    void addStatusBarItem(QWidget * widget, int stretch = 0, bool permanent = false);

    /**
     * Remove a widget from the statusbar for this view.
     */
    void removeStatusBarItem(QWidget * widget);

    /**
     * Show or hide all statusbar items. Used by KoMainWindow during saving.
     */
    void showAllStatusBarItems(bool show);

    /**
     * You have to implement this method and disable/enable certain functionality (actions for example) in
     * your view to allow/disallow editing of the document.
     */
    virtual void updateReadWrite(bool readwrite) = 0;

    /**
     * Creates a dockwidget if needed from @p factory if the mainwindow is a KoMainWindow by calling
     * KoMainWindow::createDockWidget()
     *
     * @param factory the factory used to the create the dockwidget
     * @return the created dockwidget or NULL if the mainwindow isn't a KoMainWindow
     */
    QDockWidget *createDockWidget(KoDockFactory* factory);

    /**
     * Programatically closes (but doesn't delete) a dockwidget if the mainwindow is a
     * KoMainWindow by calling KoMainWindow::removeDockWidget()
     *
     * @param dock the docker that you want to close
     */
    void removeDockWidget(QDockWidget *dock);

    /**
     * Programatically restore a (previously removed) dockwidget if the mainwindow is a
     * KoMainWindow by calling KoMainWindow::restoreDockWidget()
     *
     * @param dock the docker that you want to close
     */
    void restoreDockWidget(QDockWidget *dock);

    /**
     * Check to see if the view is currently in the middle of an operation which means
     * that there will be no screen refreshes until a signal from the document hits
     * the @ref #endOperation slot
     */
    bool isInOperation() const;

    /**
     * @return the view bar. The bar is created only if this function is called.
     */
    QToolBar* viewBar();

    /// create a list of actions that when activated will change the unit on the document.
    QList<QAction*> createChangeUnitActions();

public Q_SLOTS:
    /**
     * Slot to create a new view around the contained @ref #koDocument.
     */
    virtual void newView();

    /**
     * Slot to allow code to signal the beginning of an operation where the screen should
     * not update until it is done.
     *
     * @see #endOperation
     */
    virtual void beginOperation();

    /**
     * Slot to allow code to signal the end of an operation where the screen should
     * not have been updating. So now it will update.
     *
     * @see #beginOperation
     */
    virtual void endOperation();

    /**
     * Display a message in the status bar (calls QStatusBar::message())
     * @todo rename to something more generic
     */
    void slotActionStatusText(const QString &text);

    /**
     * End of the message in the status bar (calls QStatusBar::clear())
     * @todo rename to something more generic
     */
    void slotClearStatusText();

protected:
    /**
     * This method handles three events: KParts::PartActivateEvent, KParts::PartSelectEvent
     * and KParts::GUIActivateEvent.
     * The respective handlers are called if such an event is found.
     */
    virtual void customEvent(QEvent *ev);

    /**
     * Handles the event KParts::PartActivateEvent.
     */
    virtual void partActivateEvent(KParts::PartActivateEvent *event);
    /**
     * Handles the event KParts::PartSelectEvent.
     */
    virtual void partSelectEvent(KParts::PartSelectEvent *event);
    /**
     * Handles the event KParts::GUIActivateEvent.
     */
    virtual void guiActivateEvent(KParts::GUIActivateEvent *);


    /**
       Generate a name for this view.
    */
    QString newObjectName();

Q_SIGNALS:
    void activated(bool active);
    void selected(bool select);

    void autoScroll(const QPoint &scrollDistance);

    void childSelected(KoDocumentChild *child);
    void childUnselected(KoDocumentChild *child);

    void childActivated(KoDocumentChild *child);
    void childDeactivated(KoDocumentChild *child);

    void regionInvalidated(const QRegion &region, bool erase);

    void invalidated();

// KDE invents public signals :)
#undef signals
#define signals public
signals:

    /**
      * Make it possible for plugins to request
      * the embedding of an image into the current
      * document. Used e.g. by the scan-plugin
    */
    void embedImage(const QString &filename);

#undef signals
#define signals protected

protected Q_SLOTS:
    virtual void slotChildActivated(bool a);
    virtual void slotChildChanged(KoDocumentChild *child);
    virtual void slotAutoScroll();

private:
    KAction *actionNewView;
    virtual void setupGlobalActions(void);
    KoViewPrivate * const d;
    int autoScrollAcceleration(int offset) const;

};

#endif
