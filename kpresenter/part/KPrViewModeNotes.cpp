/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KPrViewModeNotes.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include <KDebug>

#include <KoCanvasResourceProvider.h>
#include <KoRuler.h>
#include <KoSelection.h>
#include <KoShapeLayer.h>
#include <KoShapeManager.h>
#include <KoText.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>
#include <KoZoomController.h>

#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAPageBase.h>
#include <KoPAMasterPage.h>
#include <KoPAView.h>

#include "KPrNotes.h"
#include "KPrPage.h"

KPrViewModeNotes::KPrViewModeNotes(KoPAView *view, KoPACanvas *canvas)
    : KoPAViewMode( view, canvas )
{
}

KPrViewModeNotes::~KPrViewModeNotes()
{
}

void KPrViewModeNotes::paintEvent( KoPACanvas * canvas, QPaintEvent* event )
{
    Q_ASSERT( m_canvas == canvas );

    QPainter painter(m_canvas);
    painter.translate(-m_canvas->documentOffset());
    painter.setRenderHint( QPainter::Antialiasing );
    QRectF clipRect = event->rect().translated(m_canvas->documentOffset());
    painter.setClipRect( clipRect );

    KoViewConverter *converter = m_view->viewConverter( m_canvas );
    m_canvas->shapeManager()->paint(painter, *converter, false);
    m_toolProxy->paint(painter, *converter);
}

void KPrViewModeNotes::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    m_toolProxy->tabletEvent(event, point);
}

void KPrViewModeNotes::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mousePressEvent(event, point);
}

void KPrViewModeNotes::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseDoubleClickEvent(event, point);
}

void KPrViewModeNotes::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseMoveEvent(event, point);
}

void KPrViewModeNotes::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseReleaseEvent(event, point);
}

void KPrViewModeNotes::keyPressEvent(QKeyEvent *event)
{
    m_toolProxy->keyPressEvent(event);
    KoPageApp::PageNavigation pageNavigation;

    if (!event->isAccepted()) {
        event->accept();

        switch (event->key()) {
            case Qt::Key_Home:
                pageNavigation = KoPageApp::PageFirst;
                break;
            case Qt::Key_PageUp:
                pageNavigation = KoPageApp::PagePrevious;
                break;
            case Qt::Key_PageDown:
                pageNavigation = KoPageApp::PageNext;
                break;
            case Qt::Key_End:
                pageNavigation = KoPageApp::PageLast;
                break;
            default:
                event->ignore();
                return;
        }

        KoPAPageBase *activePage = m_view->activePage();
        KoPAPageBase *newPage = m_view->kopaDocument()->pageByNavigation(activePage, pageNavigation);

        if (newPage != activePage) {
            updateActivePage( newPage );
        }
    }
}

void KPrViewModeNotes::keyReleaseEvent(QKeyEvent *event)
{
    m_toolProxy->keyReleaseEvent(event);
}

void KPrViewModeNotes::wheelEvent(QWheelEvent *event, const QPointF &point)
{
    m_toolProxy->wheelEvent(event, point);
}

void KPrViewModeNotes::activate(KoPAViewMode *previousViewMode)
{
    Q_UNUSED( previousViewMode );
    m_canvas->resourceProvider()->setResource(KoText::ShowTextFrames, true);
    m_view->setActionEnabled( KoPAView::AllActions, false );
    updateActivePage( m_view->activePage() );
}

void KPrViewModeNotes::deactivate()
{
    m_canvas->resourceProvider()->setResource(KoText::ShowTextFrames, false);
    m_view->setActionEnabled( KoPAView::AllActions, true );
    m_view->doUpdateActivePage(m_view->activePage());
}

void KPrViewModeNotes::updateActivePage( KoPAPageBase *page )
{
    if ( m_view->activePage() != page ) {
        m_view->setActivePage( page );
    }

    KPrPage *prPage = dynamic_cast<KPrPage *>( page );
    if ( !prPage ) return;

    KPrNotes *notes = prPage->pageNotes();
    notes->updatePageThumbnail();
    KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>( notes->iterator().last() );

    KoPageLayout &layout = notes->pageLayout();
    QSize size(layout.width, layout.height);
    
    m_view->horizontalRuler()->setRulerLength(layout.width);
    m_view->verticalRuler()->setRulerLength(layout.height);
    m_view->horizontalRuler()->setActiveRange(layout.left, layout.width - layout.right);
    m_view->verticalRuler()->setActiveRange(layout.top, layout.height - layout.bottom);

    m_view->zoomController()->setPageSize(size);
    m_view->zoomController()->setDocumentSize(size);
    m_canvas->update();

    m_canvas->shapeManager()->setShapes( layer->iterator() );
    m_canvas->masterShapeManager()->setShapes(QList<KoShape*>());

    KoSelection *selection = m_canvas->shapeManager()->selection();
    selection->select(notes->textShape());
    selection->setActiveLayer( layer );
    QString tool = KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes());
    KoToolManager::instance()->switchToolRequested(tool);
}

void KPrViewModeNotes::addShape( KoShape *shape )
{
    KoShape *parent = shape;
    KPrNotes *notes = 0;
    // similar to KoPADocument::pageByShape()
    while ( !notes && ( parent = parent->parent() ) ) {
        notes = dynamic_cast<KPrNotes *>( parent );
    }

    if ( notes ) {
        KPrPage *activePage = static_cast<KPrPage *>( m_view->activePage() );
        if ( notes == activePage->pageNotes() ) {
            m_view->kopaCanvas()->shapeManager()->add( shape );
        }
    }
}

void KPrViewModeNotes::removeShape( KoShape *shape )
{
    KoShape *parent = shape;
    KPrNotes *notes = 0;
    while ( !notes && ( parent = parent->parent() ) ) {
        notes = dynamic_cast<KPrNotes *>( parent );
    }

    if ( notes ) {
        KPrPage *activePage = static_cast<KPrPage *>( m_view->activePage() );
        if ( notes == activePage->pageNotes() ) {
            m_view->kopaCanvas()->shapeManager()->remove( shape );
        }
    }
}

