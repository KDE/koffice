/* This file is part of the KDE project
 *  Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
 *  Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#ifndef KOPAVIEW_H
#define KOPAVIEW_H

#include "KoPageApp.h"                  //krazy:exclude=includes
#include "kopageapp_export.h"

#include <KoZoomMode.h>
#include <KoZoomHandler.h>
#include <KoView.h>

class KoPACanvas;
class KViewConverter;
class KoPAPage;
class KoPADocument;
class KoZoomController;
class KoRuler;
class KShapeManager;
class KoPADocumentStructureDocker;
class KCanvasController;
class KActionMenu;
class KoZoomAction;
class KoFind;
class QLabel;
class KToggleAction;
class KoPAViewMode;

/// A view with a KoPACanvas and rulers
class KOPAGEAPP_EXPORT KoPAView : public KoView
{
    Q_OBJECT
public:
    enum KoPAAction
    {
        ActionInsertPage = 1,
        ActionCopyPage   = 2,
        ActionDeletePage = 4,
        ActionViewShowMasterPages = 8,
        ActionFormatMasterPage = 16,
        AllActions       = 0xFF
    };

    /**
     * Constructor
     * @param document the document of this view
     * @param parent the parent widget
     */
    explicit KoPAView(KoPADocument * document, QWidget * parent = 0);
    virtual ~KoPAView();

    /// @return the canvas for the application
    KoPACanvas *kopaCanvas() const { return m_canvas; }

    /// @return the document for the application
    KoPADocument *kopaDocument() const { return m_doc; }

    KViewConverter *viewConverter(KoPACanvas *canvas);
    virtual KoZoomController *zoomController() const;
    virtual KoZoomHandler *zoomHandler();

    /**
     * @brief Set the view mode
     *
     * @param mode the new view mode
     */
    virtual void setViewMode(KoPAViewMode* mode);

    /// @return the active viewMode
    virtual KoPAViewMode* viewMode() const;

    void updateReadWrite(bool readwrite);

    KoRuler *horizontalRuler();
    KoRuler *verticalRuler();

    /// @return Page that is shown in the canvas
    KoPAPage* activePage() const;

    /// Set page shown in the canvas to @p page
    void setActivePage(KoPAPage * page);

    void navigatePage(KoPageApp::PageNavigation pageNavigation);

    /// @return the shape manager used for this view
    KShapeManager* shapeManager() const; // TODO remove

    /// @return the master shape manager used for this view
    KShapeManager* masterShapeManager() const; // TODO remove

    /**
     * @brief Enables/Disables the given actions
     *
     * The actions are of Type KoPAAction
     *
     * @param actions which should be enabled/disabled
     * @param enable new state of the actions
     */
    void setActionEnabled(int actions, bool enable);

    /**
     * Set the active page and updates the UI
     */
    void doUpdateActivePage(KoPAPage * page);

    /**
     * Paste the page if everything is ok
     */
    void pagePaste();

    /// reimplemented
    virtual KoPrintJob * createPrintJob();

    /**
     * Get the thumbnail for the page.
     *
     * Us this method instead the on in the pages directly
     */
    QPixmap pageThumbnail(KoPAPage* page, const QSize &size);

    /**
     * Save thumbnail to an image file.
     *
     * Export a thumbnail to disk using a pixmap file like e.g. PNG
     * This method uses a QPixmap::save() call.
     *
     * @param page page to get thumbnail of
     * @param url the url of the image file to be created
     * @param size the desired image size in px
     * @param format the format of the image file (see QPixmap::save())
     * @param quality the quality of the image in [0,100] or -1 to use default (see QPixmap::save())
     *
     * @returns whether the image was successfully saved
     */
    bool exportPageThumbnail(KoPAPage * page, const KUrl &url, const QSize &size = QSize(512, 512),
                              const char * format = 0, int quality = -1);

    /// Update page navigation actions
    void updatePageNavigationActions();


public slots:
    /// Insert a new page after the current one
    void insertPage();

private:

    /// creates the widgets (called from the constructor)
    void initGUI();
    /// creates the actions (called from the constructor)
    void initActions();

    /// Returns the document structure docker
    KoPADocumentStructureDocker* documentStructureDocker() const;

    /// Called when receiving a PartActivateEvent
    virtual void partActivateEvent(KParts::PartActivateEvent* event);

    bool isMasterUsed(KoPAPage * page);

private slots:
    void editPaste();

    /// Shows/hides the rulers
    void setShowRulers(bool show);
    void updateActivePage(KoPAPage*);

protected slots:
    void viewSnapToGrid(bool snap);
    void viewGuides(bool show);
    void slotZoomChanged(KoZoomMode::Mode mode, qreal zoom);

    void editDeleteSelection();
    void editSelectAll();
    void editDeselectAll();

    void formatMasterPage();

    void formatPageLayout();

    /// Change the current view mode to work on master pages
    void setMasterMode(bool master);

    // update the rulers
    void pageOffsetChanged();

    /// Called when the mouse position changes on the canvas
    virtual void updateMousePosition(const QPoint &position);

    /// Called when the selection changed
    virtual void selectionChanged();

    /// Copy Page
    void copyPage();

    /// Delete the current page
    void deletePage();

    /// Called when the clipboard changed
    virtual void clipboardDataChanged();

    /// Go to the previous page
    void goToPreviousPage();
    /// Go to the next page
    void goToNextPage();
    /// Go to the first page
    void goToFirstPage();
    /// Go to the last page
    void goToLastPage();

    /**
     * Set the next document that should be used in find
     *
     * @param document The current document
     */
    void findDocumentSetNext(QTextDocument * document);

    /**
     * Set the previous document that should be used in find
     *
     * @param document The current document
     */
    void findDocumentSetPrevious(QTextDocument * document);

    /**
     * Re-initialize the document structure docker after active document in this
     * view has been changed
     */
    void reinitDocumentDocker();

    /**
     * Import slideshow
     */
    void importDocument();

    /**
     * Configure kopapage apps
     */
    void configure();

public slots:
    void variableChanged();

signals:
    /// Emitted every time the active page is changed
    void activePageChanged();


private:
    // These were originally private in the .h file
    KoPADocumentStructureDocker *m_documentStructureDocker;

    KCanvasController *m_canvasController;
    KoZoomController *m_zoomController;

    KAction *m_editPaste;
    KAction *m_deleteSelectionAction;

    KToggleAction *m_actionViewSnapToGrid;
    KToggleAction *m_actionViewShowMasterPages;

    KAction *m_actionInsertPage;
    KAction *m_actionCopyPage;
    KAction *m_actionDeletePage;

    KAction *m_actionMasterPage;
    KAction *m_actionPageLayout;

    KAction *m_actionConfigure;

    KActionMenu *m_variableActionMenu;

    KoRuler *m_horizontalRuler;
    KoRuler *m_verticalRuler;
    KToggleAction *m_viewRulers;

    KoZoomAction  *m_zoomAction;

    KoFind *m_find;

    KoPAViewMode *m_viewModeNormal;

    // status bar
    QLabel *m_status;       ///< ordinary status
    QWidget *m_zoomActionWidget;

    KoPADocument *m_doc;
    KoPACanvas *m_canvas;
    KoPAPage *m_activePage;
    KoZoomHandler m_zoomHandler;
    KoPAViewMode *m_viewMode;
};

#endif
