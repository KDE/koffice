/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "KPrViewModePresentation.h"

#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QtGui/QDesktopWidget>

#include <kdebug.h>

#include <KoPointerEvent.h>
#include <KoCanvasController.h>
#include <KoPageApp.h>
#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAView.h>
#include <KoZoomHandler.h>

#include "KPrDocument.h"
#include "KPrPresentationTool.h"
#include "KPrPresenterViewWidget.h"
#include "KPrEndOfSlideShowPage.h"

KPrViewModePresentation::KPrViewModePresentation( KoPAView * view, KoPACanvas * canvas )
: KoPAViewMode( view, canvas )
, m_savedParent( 0 )
, m_tool( new KPrPresentationTool( *this ) )
, m_animationDirector( 0 )
, m_pvAnimationDirector( 0 )
, m_presenterViewCanvas( 0 )
, m_presenterViewWidget( 0 )
, m_endOfSlideShowPage( 0 )
{
}

KPrViewModePresentation::~KPrViewModePresentation()
{
    delete m_animationDirector;
    delete m_tool;
}

KoViewConverter * KPrViewModePresentation::viewConverter( KoPACanvas * canvas )
{
    if ( m_animationDirector && m_canvas == canvas ) {
        return m_animationDirector->viewConverter();
    }
    else if ( m_pvAnimationDirector && m_presenterViewCanvas == canvas ) {
        return m_pvAnimationDirector->viewConverter();
    }
    else {
        return 0;
    }
}

void KPrViewModePresentation::paintEvent( KoPACanvas * canvas,  QPaintEvent* event )
{
    if ( m_canvas == canvas && m_animationDirector ) {
        m_animationDirector->paintEvent( event );
    } else if ( m_presenterViewCanvas == canvas && m_pvAnimationDirector ) {
        m_pvAnimationDirector->paintEvent( event );
    }
}

void KPrViewModePresentation::tabletEvent( QTabletEvent *event, const QPointF &point )
{

}

void KPrViewModePresentation::mousePressEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );

    m_tool->mousePressEvent( &ev );
}

void KPrViewModePresentation::mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );

    m_tool->mouseDoubleClickEvent( &ev );
}

void KPrViewModePresentation::mouseMoveEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );

    m_tool->mouseMoveEvent( &ev );
}

void KPrViewModePresentation::mouseReleaseEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );

    m_tool->mouseReleaseEvent( &ev );
}

void KPrViewModePresentation::keyPressEvent( QKeyEvent *event )
{
    m_tool->keyPressEvent( event );
}

void KPrViewModePresentation::keyReleaseEvent( QKeyEvent *event )
{
    m_tool->keyReleaseEvent( event );
}

void KPrViewModePresentation::wheelEvent( QWheelEvent * event, const QPointF &point )
{
    KoPointerEvent ev( event, point );

    m_tool->wheelEvent( &ev );
}

void KPrViewModePresentation::closeEvent( QCloseEvent * event )
{
    activateSavedViewMode();
    event->ignore();
}

void KPrViewModePresentation::activate( KoPAViewMode * previousViewMode )
{
    m_savedViewMode = previousViewMode;               // store the previous view mode
    m_savedParent = m_canvas->parentWidget();
    m_canvas->setParent( ( QWidget* )0, Qt::Window ); // set parent to 0 and

    QDesktopWidget desktop;

    KPrDocument *document = static_cast<KPrDocument *>( m_view->kopaDocument() );
    bool presenterViewEnabled = document->isPresenterViewEnabled();
    int presentationscreen = document->presentationMonitor();

    // add end off slideshow page
    m_endOfSlideShowPage = new KPrEndOfSlideShowPage( desktop.screenGeometry( presentationscreen ), document );
    QList<KoPAPageBase*> pages = document->slideShow();
    pages.append( m_endOfSlideShowPage );

    QRect presentationRect = desktop.screenGeometry( presentationscreen );

    m_canvas->setWindowState( m_canvas->windowState() | Qt::WindowFullScreen ); // detach widget to make
    m_canvas->show();
    m_canvas->setFocus();                             // it shown full screen
    // move and resize now as otherwise it is not set when we call activate on the tool.
    m_canvas->move( presentationRect.topLeft() );
    m_canvas->resize( presentationRect.size() );

    // the main animation director needs to be created first since it will set the active page
    // of the presentation
    m_animationDirector = new KPrAnimationDirector( m_view, m_canvas, pages, m_view->activePage() );

    if ( presenterViewEnabled ) {
        if ( desktop.numScreens() > 1 ) {
            int newscreen = desktop.numScreens() - presentationscreen - 1; // What if we have > 2 screens?
            QRect pvRect = desktop.screenGeometry( newscreen );

            m_presenterViewCanvas = new KoPACanvas( m_view, document );
            m_presenterViewWidget = new KPrPresenterViewWidget( this, pages, m_presenterViewCanvas );
            m_presenterViewWidget->setParent( ( QWidget* )0, Qt::Window );
            m_presenterViewWidget->setWindowState(
                    m_presenterViewWidget->windowState() | Qt::WindowFullScreen );
            m_presenterViewWidget->move( pvRect.topLeft() );
            m_presenterViewWidget->updateWidget( pvRect.size(), presentationRect.size() );
            m_presenterViewWidget->show();
            m_presenterViewWidget->setFocus();                             // it shown full screen

            m_pvAnimationDirector = new KPrAnimationDirector( m_view,
                    m_presenterViewCanvas, pages, m_view->activePage() );
        }
        else {
            kWarning() << "Presenter View is enabled but only found one monitor";
            document->setPresenterViewEnabled( false );
        }
    }

    m_tool->activate( false );
}

void KPrViewModePresentation::deactivate()
{
    KoPAPageBase * page = m_view->activePage();
    if ( m_endOfSlideShowPage ) {
        if ( page == m_endOfSlideShowPage ) {
            page = m_view->kopaDocument()->pages().last();
        }
    }
    m_tool->deactivate();

    m_canvas->setParent( m_savedParent, Qt::Widget );
    m_canvas->setFocus();
    m_canvas->setWindowState( m_canvas->windowState() & ~Qt::WindowFullScreen ); // reset
    m_canvas->show();
    m_view->updateActivePage( page );

    // only delete after the new page has been set
    delete m_endOfSlideShowPage;
    m_endOfSlideShowPage = 0;

    delete m_animationDirector;
    m_animationDirector = 0;

    if ( m_presenterViewWidget ) {
        m_presenterViewWidget->setWindowState(
            m_presenterViewWidget->windowState() & ~Qt::WindowFullScreen );
        delete m_pvAnimationDirector;
        m_pvAnimationDirector = 0;

        delete m_presenterViewWidget;
        m_presenterViewWidget = 0;
        m_presenterViewCanvas = 0;
    }
}

void KPrViewModePresentation::updateActivePage( KoPAPageBase *page )
{
    m_view->setActivePage( page );
    if ( m_presenterViewWidget ) {
        if ( 0 != m_animationDirector ) {
            m_presenterViewWidget->setActivePage( m_animationDirector->currentPage() );
        }
        else {
            m_presenterViewWidget->setActivePage( page );
        }
    }
}

void KPrViewModePresentation::activateSavedViewMode()
{
    m_view->setViewMode( m_savedViewMode );
}

KPrAnimationDirector * KPrViewModePresentation::animationDirector()
{
    return m_animationDirector;
}

void KPrViewModePresentation::navigate( KPrAnimationDirector::Navigation navigation )
{
    bool finished = m_animationDirector->navigate( navigation );
    if ( m_pvAnimationDirector ) {
        finished = m_pvAnimationDirector->navigate( navigation ) && finished;
    }

    if ( finished ) {
        activateSavedViewMode();
    }
}

void KPrViewModePresentation::navigateToPage( int index )
{
    m_animationDirector->navigateToPage( index );
    if ( m_pvAnimationDirector ) {
        m_pvAnimationDirector->navigateToPage( index );
    }
}
