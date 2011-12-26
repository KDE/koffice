/* This file is part of the KDE project
 * Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KOPAVIEWBASE_H
#define KOPAVIEWBASE_H

#include "KoPageApp.h"			//krazy:exclude=includes
#include "KoPAViewMode.h"		//krazy:exclude=includes
#include "kopageapp_export.h"

class KoPACanvasBase;
class KViewConverter;
class KoPAPage;
class KoPADocument;
class KoZoomHandler;
class KoZoomController;
class KoPAViewProxyObject;

/**
 * Base class defining the View interface the KoPAViewMode class needs.
 */
class KOPAGEAPP_EXPORT KoPAViewBase
{
public:

    // proxy QObject
    KoPAViewProxyObject *proxyObject;

    explicit KoPAViewBase();
    virtual ~KoPAViewBase();

    /// @return the canvas for the application
    virtual KoPACanvasBase * kopaCanvas() const = 0;

    /// @return the document for the application
    virtual KoPADocument * kopaDocument() const = 0;

    /// XXX
    virtual KViewConverter * viewConverter(KoPACanvasBase * canvas);
    virtual KViewConverter * viewConverter() const;
    virtual KoZoomController * zoomController() const = 0;

    virtual KoZoomHandler * zoomHandler();
    virtual KoZoomHandler *zoomHandler() const;

    /// Set the active page and updates the UI
    virtual void doUpdateActivePage(KoPAPage * page) = 0;

    /// Set page shown in the canvas to @p page
    virtual void setActivePage(KoPAPage * page) = 0;

    /// @return Page that is shown in the canvas
    virtual KoPAPage* activePage() const = 0;

    /// XXX
    virtual void navigatePage(KoPageApp::PageNavigation pageNavigation) = 0;

    /**
     * @brief Enables/Disables the given actions
     *
     * The actions are of Type KoPAAction
     *
     * @param actions which should be enabled/disabled
     * @param enable new state of the actions
     */
    virtual void setActionEnabled(int actions, bool enable) = 0;

    /// XXX
    virtual void updatePageNavigationActions() = 0;

    /**
     * @brief Set the view mode
     *
     * @param mode the new view mode
     */
    virtual void setViewMode(KoPAViewMode* mode);

    /// @return the active viewMode
    virtual KoPAViewMode* viewMode() const;

    /// Insert a new page after the current one
    virtual void insertPage() = 0;

    /// Paste the page if everything is ok
    virtual void pagePaste() = 0;

    /// XXX
    virtual void editPaste() = 0;

    /// XXX
    virtual void setShowRulers(bool show) = 0;

private:

    class Private;
    Private * const d;
};

/**
 * QObject proxy class for handling signals and slots
 */
class KOPAGEAPP_EXPORT KoPAViewProxyObject : public QObject {

    Q_OBJECT

public:

    KoPAViewProxyObject(KoPAViewBase * parent);

    void emitActivePageChanged() { emit activePageChanged(); }

public slots:

    /// Set the active page and updates the UI
    void updateActivePage(KoPAPage * page) { m_view->viewMode()->updateActivePage(page); }

    /// Shows/hides the rulers
    void setShowRulers(bool show) { m_view->setShowRulers(show); }

    /// Insert a new page after the current one
    void insertPage() { m_view->insertPage(); }

    void editPaste() { m_view->editPaste(); }

signals:

    /// Emitted every time the active page is changed
    void activePageChanged();

private:

    KoPAViewBase * m_view;
};

#endif // KOPAVIEWBASE_H
