/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KOPAVIEW_H
#define KOPAVIEW_H

#include <QObject>

#include <KoView.h>
#include <KoZoomHandler.h>
#include <KoRuler.h>
#include "KoPageApp.h"
#include "kopageapp_export.h"

class KoCanvasController;
class KoFind;
class KoPACanvas;
class KoPADocument;
class KToggleAction;
class KoPAPageBase;
class KoPAViewMode;
class KoPADocumentStructureDocker;
class KoShapeManager;
class KoZoomAction;
class KoZoomController;
class QTextDocument;

/// Creates a view with a KoPACanvas and rulers
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
    explicit KoPAView( KoPADocument * document, QWidget * parent = 0 );
    virtual ~KoPAView();

    void updateReadWrite( bool readwrite );

    virtual KoViewConverter * viewConverter( KoPACanvas * canvas ) { Q_UNUSED( canvas ); return &m_zoomHandler; }

    KoZoomHandler* zoomHandler() { return &m_zoomHandler; }

    KoZoomController *zoomController() { return m_zoomController; }

    KoRuler *horizontalRuler() { return m_horizontalRuler; }
    KoRuler *verticalRuler() { return m_verticalRuler; }

    KoPACanvas * kopaCanvas() { return m_canvas; }
    KoPACanvas * kopaCanvas() const { return m_canvas; }

    KoPADocument * kopaDocument() { return m_doc; }
    /// @return Page that is shown in the canvas
    KoPAPageBase* activePage() const;

    /// Set page shown in the canvas to @p page
    void setActivePage( KoPAPageBase * page );

    void navigatePage( KoPageApp::PageNavigation pageNavigation );

    /// @return the shape manager used for this view
    KoShapeManager* shapeManager() const;

    /// @return the master shape manager used for this view
    KoShapeManager* masterShapeManager() const;

    /// @return the acvive viewMode
    KoPAViewMode* viewMode() const;

    /**
     * @brief Set the view mode
     *
     * @param mode the new view mode
     */
    void setViewMode( KoPAViewMode* mode );

    /**
     * @brief Enables/Disables the given actions
     *
     * The actions are of Type KoPAAction
     *
     * @param actions which should be enabled/disabled
     * @param enable new state of the actions
     */
    void setActionEnabled( int actions, bool enable );

    /**
     * Set the active page and updates the UI
     */
    void doUpdateActivePage( KoPAPageBase * page );

    /// reimplemented
    virtual KoPrintJob * createPrintJob();

public slots:
    /// Set the active page and updates the UI
    void updateActivePage( KoPAPageBase * page );

    /// Shows/hides the rulers
    void setShowRulers(bool show);

    /// Insert a new page after the current one
    void insertPage();

signals:
    /// Emitted every time the active page is changed
    void activePageChanged();

protected:
    /// creates the widgets (called from the constructor)
    void initGUI();
    /// creates the actions (called from the constructor)
    void initActions();

    /// Returns the document structure docker
    KoPADocumentStructureDocker* documentStructureDocker() const;

    /// Called when receiving a PartActivateEvent
    virtual void partActivateEvent(KParts::PartActivateEvent* event);

    /// Update page navigation actions
    void updatePageNavigationActions();

    bool isMasterUsed( KoPAPageBase * page );

protected slots:
    void viewSnapToGrid(bool snap);
    void viewGuides(bool show);
    void slotZoomChanged( KoZoomMode::Mode mode, qreal zoom );

    void editPaste();
    void editDeleteSelection();
    void editSelectAll();
    void editDeselectAll();

    void formatMasterPage();

    /// Change the current view mode to work on master pages
    void setMasterMode( bool master );

    /// Called when the canvas controller is resized
    virtual void canvasControllerResized();

    /// Called when the mouse position changes on the canvas
    virtual void updateMousePosition(const QPoint& position);

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
    void findDocumentSetNext( QTextDocument * document );

    /**
     * Set the previous document that should be used in find
     *
     * @param document The current document
     */
    void findDocumentSetPrevious( QTextDocument * document );

    /**
     * Re-initialize the document structure docker after active document in this
     * view has been changed
     */
    void reinitDocumentDocker();

protected:
    KoPADocument *m_doc;
    KoPACanvas *m_canvas;
    KoPAPageBase *m_activePage;
    KoPAViewMode *m_viewMode;

private:

    KoPADocumentStructureDocker * m_documentStructureDocker;

    KoCanvasController * m_canvasController;
    KoZoomController * m_zoomController;
    KoZoomHandler m_zoomHandler;

    KAction *m_editPaste;
    KAction *m_deleteSelectionAction;

    KToggleAction *m_actionViewSnapToGrid;
    KToggleAction *m_actionViewShowGuides;
    KToggleAction *m_actionViewShowMasterPages;

    KAction * m_actionInsertPage;
    KAction * m_actionCopyPage;
    KAction * m_actionDeletePage;

    KAction *m_actionMasterPage;

    KoRuler *m_horizontalRuler;
    KoRuler *m_verticalRuler;
    KToggleAction* m_viewRulers;

    KoZoomAction *m_zoomAction;

    KoFind * m_find;

    KoPAViewMode *m_viewModeNormal;
};

#endif /* KOPAVIEW_H */
