/* This file is part of the KDE project
 * Copyright (C) 2008,2010 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>
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

#include "ArtworkZoomController.h"

#include <ArtworkCanvas.h>
#include <KoZoomAction.h>
#include <KCanvasController.h>
#include <KCanvasBase.h>
#include <KoZoomHandler.h>
#include <KResourceManager.h>
#include <KShapeManager.h>
#include <KShapeSelection.h>

#include <KActionCollection>
#include <KLocale>

class ArtworkZoomController::Private
{
public:
    Private()
            : canvasController(0), canvas(0), zoomHandler(0), action(0)
            , fitMargin(25)
    {
    }

    ~Private()
    {
        delete action;
    }

    KCanvasController * canvasController;
    ArtworkCanvas * canvas;
    KoZoomHandler * zoomHandler;
    KoZoomAction * action;
    QSizeF pageSize;
    int fitMargin;
};

ArtworkZoomController::ArtworkZoomController(KCanvasController *controller, KActionCollection *actionCollection, QObject *parent)
        : QObject(parent), d(new Private())
{
    d->canvasController = controller;

    d->action = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE, i18n("Zoom"), this);
    d->action->setSpecialButtons(KoZoomAction::ZoomToSelection | KoZoomAction::ZoomToAll);
    connect(d->action, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
            this, SLOT(setZoom(KoZoomMode::Mode, qreal)));
    connect(d->action, SIGNAL(zoomedToSelection()),
            this, SIGNAL(zoomedToSelection()));
    connect(d->action, SIGNAL(zoomedToAll()),
            this, SIGNAL(zoomedToAll()));

    actionCollection->addAction("view_zoom", d->action);
    actionCollection->addAction(KStandardAction::ZoomIn, "zoom_in", d->action, SLOT(zoomIn()));
    actionCollection->addAction(KStandardAction::ZoomOut, "zoom_out", d->action, SLOT(zoomOut()));

    d->canvas = dynamic_cast<ArtworkCanvas*>(d->canvasController->canvas());
    d->zoomHandler = dynamic_cast<KoZoomHandler*>(const_cast<KViewConverter*>(d->canvas->viewConverter()));

    connect(d->canvasController, SIGNAL(sizeChanged(const QSize &)), this, SLOT(setAvailableSize()));
    connect(d->canvasController, SIGNAL(zoomBy(const qreal)), this, SLOT(requestZoomBy(const qreal)));
    connect(d->canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            d->canvas, SLOT(setDocumentOffset(const QPoint&)));

    connect(d->canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(resourceChanged(int, const QVariant &)));
}

ArtworkZoomController::~ArtworkZoomController()
{
    delete d;
}

KoZoomAction * ArtworkZoomController::zoomAction() const
{
    return d->action;
}

void ArtworkZoomController::setZoomMode(KoZoomMode::Mode mode)
{
    setZoom(mode, 1.0);
}

void ArtworkZoomController::setZoom(KoZoomMode::Mode mode, qreal zoom)
{
    if (d->zoomHandler->zoomMode() == mode && d->zoomHandler->zoom() == zoom)
        return; // no change
    d->zoomHandler->setZoomMode(mode);

    if (mode == KoZoomMode::ZOOM_CONSTANT) {
        if (zoom == 0.0) return;
        d->action->setZoom(zoom);
    } else if (mode == KoZoomMode::ZOOM_WIDTH) {
        zoom = (d->canvasController->viewport()->size().width() - 2 * d->fitMargin)
               / (d->zoomHandler->resolutionX() * d->pageSize.width());
        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    } else if (mode == KoZoomMode::ZOOM_PAGE) {
        zoom = (d->canvasController->viewport()->size().width() - 2 * d->fitMargin)
               / (d->zoomHandler->resolutionX() * d->pageSize.width());
        zoom = qMin(zoom, (d->canvasController->viewport()->size().height() - 2 * d->fitMargin)
                    / (d->zoomHandler->resolutionY() * d->pageSize.height()));

        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    }

    // before changing the zoom get the actual document size, document origin
    // and the current preferred center point
    QPoint preferredCenter = d->canvasController->preferredCenter();
    QPointF documentOrigin = d->canvas->documentOrigin();
    QRectF documentRect = d->canvas->documentViewRect();

    // now calculate the preferred center in document coordinates
    QPointF docCenter = d->zoomHandler->viewToDocument(preferredCenter - documentOrigin);
    KShapeSelection * selection = d->canvas->shapeManager()->selection();
    if( selection->count()) {
        docCenter = selection->boundingRect().center();
    }

    d->zoomHandler->setZoom(zoom);

    // Tell the canvasController that the zoom has changed
    // Actually canvasController doesn't know about zoom, but the document in pixels
    // has change as a result of the zoom change
    QSizeF viewSize = d->zoomHandler->documentToView(documentRect).size();
    d->canvasController->setDocumentSize(QSize(qRound(viewSize.width()), qRound(viewSize.height())), true);

    d->canvas->adjustOrigin();

    // Finally ask the canvasController to recenter
    if (mode == KoZoomMode::ZOOM_CONSTANT) {
        QPointF center = d->canvas->documentOrigin() + d->zoomHandler->documentToView(docCenter);
        d->canvasController->setPreferredCenter(center.toPoint());
    } else {
        // center the page rect when change the zoom mode to ZOOM_PAGE or ZOOM_WIDTH
        QRectF pageRect(-documentRect.topLeft(), d->pageSize);
        QPointF center = d->zoomHandler->documentToView(pageRect.center());
        d->canvasController->setPreferredCenter(center.toPoint());
    }
    d->canvas->update();
}

void ArtworkZoomController::setAvailableSize()
{
    if (d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, -1);
    if (d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, -1);
}

void ArtworkZoomController::requestZoomBy(const qreal factor)
{
    qreal zoom = d->zoomHandler->zoom();
    d->action->setZoom(factor*zoom);
    setZoom(KoZoomMode::ZOOM_CONSTANT, factor*zoom);
    d->action->setEffectiveZoom(factor*zoom);
}

void ArtworkZoomController::setPageSize(const QSizeF &pageSize)
{
    if (d->pageSize == pageSize)
        return;
    d->pageSize = pageSize;

    if (d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, -1);
    if (d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, -1);
}

void ArtworkZoomController::resourceChanged(int key, const QVariant &value)
{
    if (key == KCanvasResource::PageSize) {
        setPageSize(value.toSizeF());

        // Tell the canvasController that the document in pixels
        // has changed as a result of the page layout change
        QRectF documentRect = d->canvas->documentViewRect();
        QSizeF viewSize = d->zoomHandler->documentToView(documentRect).size();
        d->canvasController->setDocumentSize(QSize(qRound(viewSize.width()), qRound(viewSize.height())), true);
        d->canvas->adjustOrigin();
        d->canvas->update();
    }
}

#include "ArtworkZoomController.moc"
