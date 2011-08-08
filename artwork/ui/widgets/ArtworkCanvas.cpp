/* This file is part of the KDE project
   Copyright (C) 2001-2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2001-2002 Rob Buis <buis@kde.org>
   Copyright (C) 2002-2004, 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2004 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2004-2005 David Faure <faure@kde.org>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Thorsten Zachmann <t.zachmann@zagge.de>
   Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>
   
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

#include "ArtworkCanvas.h"
#include "ArtworkDocument.h"
#include "ArtworkPart.h"
#include <ArtworkOutlinePaintingStrategy.h>

#include <KoZoomHandler.h>
#include <KShapeManager.h>
#include <KToolProxy.h>
#include <KShapeManagerPaintingStrategy.h>
#include <KCanvasController.h>
#include <KSelection.h>

#include <kdebug.h>
#include <klocale.h>

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QMenu>
#include <QtCore/QEvent>
#include <QtCore/QSizeF>

class ArtworkCanvas::ArtworkCanvasPrivate {
public:
    ArtworkCanvasPrivate()
            : zoomHandler()
            , document(0)
            , part(0)
            , showMargins(false)
            , documentOffset(0, 0)
            , viewMargin(100) {}

    ~ArtworkCanvasPrivate() {
        delete toolProxy;
        toolProxy = 0;
        delete shapeManager;
    }

    KShapeManager* shapeManager;
    KoZoomHandler zoomHandler;

    KToolProxy *toolProxy;

    ArtworkDocument *document;
    ArtworkPart *part;
    QPoint origin;         ///< the origin of the document page rect
    bool showMargins;      ///< should page margins be shown
    QPoint documentOffset; ///< the offset of the virtual canvas from the viewport
    int viewMargin;        ///< the view margin around the document in pixels
    QRectF documentViewRect; ///< the last calculated document view rect
};

ArtworkCanvas::ArtworkCanvas(ArtworkPart *p)
        : QWidget() , KCanvasBase(p), d(new ArtworkCanvasPrivate())
{
    d->part = p;
    d->document = &p->document();
    d->toolProxy = new KToolProxy(this);
    d->shapeManager = new KShapeManager(this, d->document->shapes());
    connect(d->shapeManager, SIGNAL(selectionChanged()), this, SLOT(updateSizeAndOffset()));

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setBackgroundColor(Qt::white);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus); // allow to receive keyboard input
    updateSizeAndOffset();
    setAttribute(Qt::WA_InputMethodEnabled, true);
}

ArtworkCanvas::~ArtworkCanvas()
{
    delete d;
}

KShapeManager * ArtworkCanvas::shapeManager() const
{
    return d->shapeManager;
}

const KViewConverter * ArtworkCanvas::viewConverter() const
{
    return &d->zoomHandler;
}

KToolProxy * ArtworkCanvas::toolProxy() const
{
    return d->toolProxy;
}

QWidget * ArtworkCanvas::canvasWidget()
{
    return this;
}

const QWidget * ArtworkCanvas::canvasWidget() const
{
    return this;
}

bool ArtworkCanvas::event(QEvent *e)
{
    if(toolProxy()) {
        toolProxy()->processEvent(e);
    }
    return QWidget::event(e);
}

void ArtworkCanvas::paintEvent(QPaintEvent * ev)
{
    QPainter painter(this);
    painter.translate(-d->documentOffset);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect clipRect = ev->rect().translated(d->documentOffset);
    painter.setClipRect(clipRect);

    painter.translate(d->origin.x(), d->origin.y());
    painter.setPen(Qt::black);

    // paint the page rect
    painter.drawRect(d->zoomHandler.documentToView(QRectF(QPointF(0.0, 0.0), d->document->pageSize())));

    // paint the page margins
    paintMargins(painter, d->zoomHandler);

    // get the cliprect in document coordinates
    QRectF updateRect = d->zoomHandler.viewToDocument(widgetToView(clipRect));

    // paint the shapes
    painter.setRenderHint(QPainter::Antialiasing);
    d->shapeManager->paint(painter, d->zoomHandler, false);

    // paint the grid and guides
    painter.setRenderHint(QPainter::Antialiasing, false);
    d->part->gridData().paintGrid(painter, d->zoomHandler, updateRect);
    d->part->guidesData().paintGuides(painter, d->zoomHandler, updateRect);

    // paint the tool decorations
    painter.setRenderHint(QPainter::Antialiasing);
    d->toolProxy->paint(painter, d->zoomHandler);

    painter.end();
}

void ArtworkCanvas::paintMargins(QPainter &painter, const KViewConverter &converter)
{
    if (! d->showMargins)
        return;

    KOdfPageLayoutData pl = d->part->pageLayout();

    QSizeF pageSize = d->document->pageSize();
    QRectF marginRect(pl.leftMargin, pl.topMargin,
                      pageSize.width() - pl.leftMargin - pl.rightMargin,
                      pageSize.height() - pl.topMargin - pl.bottomMargin);

    QPen pen(Qt::blue);
    QVector<qreal> pattern;
    pattern << 5 << 5;
    pen.setDashPattern(pattern);
    painter.setPen(pen);
    painter.drawRect(converter.documentToView(marginRect));
}

void ArtworkCanvas::mouseMoveEvent(QMouseEvent *e)
{
    d->toolProxy->mouseMoveEvent(e, d->zoomHandler.viewToDocument(widgetToView(e->pos() + d->documentOffset)));
}

void ArtworkCanvas::mousePressEvent(QMouseEvent *e)
{
    d->toolProxy->mousePressEvent(e, d->zoomHandler.viewToDocument(widgetToView(e->pos() + d->documentOffset)));
    if (!e->isAccepted() && e->button() == Qt::RightButton) {
        QList<QAction*> actions = d->toolProxy->popupActionList();
        if (!actions.isEmpty()) {
            QMenu menu(this);
            foreach(QAction *action, d->toolProxy->popupActionList()) {
                menu.addAction(action);
            }
            menu.exec(e->globalPos());
            e->setAccepted(true);
        }
    }
}

void ArtworkCanvas::mouseDoubleClickEvent(QMouseEvent *e)
{
    d->toolProxy->mouseDoubleClickEvent(e, d->zoomHandler.viewToDocument(widgetToView(e->pos() + d->documentOffset)));
}

void ArtworkCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    d->toolProxy->mouseReleaseEvent(e, d->zoomHandler.viewToDocument(widgetToView(e->pos() + d->documentOffset)));
}

void ArtworkCanvas::keyReleaseEvent(QKeyEvent *e)
{
    d->toolProxy->keyReleaseEvent(e);
}

void ArtworkCanvas::keyPressEvent(QKeyEvent *e)
{
    d->toolProxy->keyPressEvent(e);
    if (! e->isAccepted()) {
        if (e->key() == Qt::Key_Backtab
                || (e->key() == Qt::Key_Tab && (e->modifiers() & Qt::ShiftModifier)))
            focusNextPrevChild(false);
        else if (e->key() == Qt::Key_Tab)
            focusNextPrevChild(true);
    }
}

void ArtworkCanvas::tabletEvent(QTabletEvent *e)
{
    d->toolProxy->tabletEvent(e, d->zoomHandler.viewToDocument(widgetToView(e->pos() + d->documentOffset)));
}

void ArtworkCanvas::wheelEvent(QWheelEvent *e)
{
    d->toolProxy->wheelEvent(e, d->zoomHandler.viewToDocument(widgetToView(e->pos() + d->documentOffset)));
}

QVariant ArtworkCanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return d->toolProxy->inputMethodQuery(query, *(viewConverter()));
}

void ArtworkCanvas::inputMethodEvent(QInputMethodEvent *event)
{
    d->toolProxy->inputMethodEvent(event);
}

void ArtworkCanvas::resizeEvent(QResizeEvent *)
{
    updateSizeAndOffset();
}

void ArtworkCanvas::gridSize(qreal *horizontal, qreal *vertical) const
{
    if (horizontal)
        *horizontal = d->part->gridData().gridX();
    if (vertical)
        *vertical = d->part->gridData().gridY();
}

bool ArtworkCanvas::snapToGrid() const
{
    return d->part->gridData().snapToGrid();
}

void ArtworkCanvas::addCommand(QUndoCommand *command)
{
    d->part->addCommand(command);
    updateSizeAndOffset();
}

void ArtworkCanvas::updateCanvas(const QRectF& rc)
{
    QRect clipRect(viewToWidget(d->zoomHandler.documentToView(rc).toRect()));
    clipRect.adjust(-2, -2, 2, 2); // grow for anti-aliasing
    clipRect.moveTopLeft(clipRect.topLeft() - d->documentOffset);
    update(clipRect);
}

void ArtworkCanvas::updateSizeAndOffset()
{
    // save the old view rect for comparing
    QRectF oldDocumentViewRect = d->documentViewRect;
    d->documentViewRect = documentViewRect();
    // check if the view rect has changed and emit signal if it has
    if (oldDocumentViewRect != d->documentViewRect) {
        QRectF viewRect = d->zoomHandler.documentToView(d->documentViewRect);
        KCanvasController * controller = canvasController();
        if (controller) {
            // tell canvas controller the new document size in pixel
            controller->setDocumentSize(viewRect.size().toSize(), true);
            // make sure the actual selection is visible
            KSelection * selection = d->shapeManager->selection();
            if (selection->count())
                controller->ensureVisible(d->zoomHandler.documentToView(selection->boundingRect()));
        }
    }
    adjustOrigin();
    update();
}

void ArtworkCanvas::adjustOrigin()
{
    // calculate the zoomed document bounding rect
    QRect documentRect = d->zoomHandler.documentToView(documentViewRect()).toRect();

    // save the old origin to see if it has changed
    QPoint oldOrigin = d->origin;

    // set the origin to the zoom document rect origin
    d->origin = -documentRect.topLeft();

    // the document bounding rect is always centered on the virtual canvas
    // if there are margins left around the zoomed document rect then
    // distribute them evenly on both sides
    int widthDiff = size().width() - documentRect.width();
    if (widthDiff > 0)
        d->origin.rx() += qRound(0.5 * widthDiff);
    int heightDiff = size().height() - documentRect.height();
    if (heightDiff > 0)
        d->origin.ry() += qRound(0.5 * heightDiff);

    // check if the origin has changed and emit signal if it has
    if (d->origin != oldOrigin)
        emit documentOriginChanged(d->origin);
}

void ArtworkCanvas::setDocumentOffset(const QPoint &offset)
{
    d->documentOffset = offset;
}

void ArtworkCanvas::enableOutlineMode(bool on)
{
    if (on)
        new ArtworkOutlinePaintingStrategy(d->shapeManager);
    else {
        d->shapeManager->setPaintingStrategy(new KShapeManagerPaintingStrategy(d->shapeManager));
    }
}

QPoint ArtworkCanvas::widgetToView(const QPoint& p) const
{
    return p - d->origin;
}

QRect ArtworkCanvas::widgetToView(const QRect& r) const
{
    return r.translated(- d->origin);
}

QPoint ArtworkCanvas::viewToWidget(const QPoint& p) const
{
    return p + d->origin;
}

QRect ArtworkCanvas::viewToWidget(const QRect& r) const
{
    return r.translated(d->origin);
}

KUnit ArtworkCanvas::unit() const
{
    return d->part->unit();
}

QPoint ArtworkCanvas::documentOrigin() const
{
    return d->origin;
}

void ArtworkCanvas::setShowPageMargins(bool on)
{
    d->showMargins = on;
}

void ArtworkCanvas::setDocumentViewMargin(int margin)
{
    d->viewMargin = margin;
}

int ArtworkCanvas::documentViewMargin() const
{
    return d->viewMargin;
}

QRectF ArtworkCanvas::documentViewRect()
{
    QRectF bbox = d->document->boundingRect();
    d->documentViewRect = bbox.adjusted(-d->viewMargin, -d->viewMargin, d->viewMargin, d->viewMargin);
    return d->documentViewRect;
}

void ArtworkCanvas::updateInputMethodInfo()
{
    updateMicroFocus();
}

KGuidesData * ArtworkCanvas::guidesData()
{
    return &d->part->guidesData();
}

void ArtworkCanvas::setBackgroundColor(const QColor &color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Normal, backgroundRole(), color);
    pal.setColor(QPalette::Inactive, backgroundRole(), color);
    setPalette(pal);
}

#include "ArtworkCanvas.moc"

