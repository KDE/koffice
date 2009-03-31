/* This file is part of the KDE project
 *
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoCanvasController.h"

#include "KoCanvasController_p.h"
#include "KoShape.h"
#include "KoViewConverter.h"
#include "KoCanvasBase.h"
#include "KoCanvasObserver.h"
#include <KoMainWindow.h>
#include <KoView.h>
#include "tools/KoGuidesTool.h"
#include "KoToolManager.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QScrollBar>
#include <QtCore/QEvent>
#include <QtGui/QDockWidget>
#include <QtCore/QTimer>

#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <QtOpenGL/QGLWidget>
#endif

class KoCanvasController::Private
{
public:
    Private() : canvas(0), canvasMode(Centered), margin(0)
            , ignoreScrollSignals(false) {}
    KoCanvasBase * canvas;
    CanvasMode canvasMode;
    int margin; // The viewport margin around the document // TODO can we remove this one? The viewport has a copy...
    QSize documentSize;
    QPoint documentOffset;
    Viewport * viewportWidget;
    qreal preferredCenterFractionX;
    qreal preferredCenterFractionY;
    bool ignoreScrollSignals;
};

KoCanvasController::KoCanvasController(QWidget *parent)
        : QAbstractScrollArea(parent),
        d(new Private())
{
    setFrameShape(NoFrame);
    d->viewportWidget = new Viewport(this);
    setViewport(d->viewportWidget);

    setAutoFillBackground(false);
    /*
      Fixes:   apps starting at zero zoom.
      Details: Since the document is set on the mainwindow before loading commences the inial show/layout can choose
          to set the document to be very small, even to be zero pixels tall.  Setting a sane minimum size on the
          widget means we no loger get rounding errors in zooming and we no longer end up with zero-zoom.
      Note: KoPage apps should probably startup with a sane document size; for Krita that's impossible
     */
    setMinimumSize(QSize(50, 50));
    setMouseTracking(true);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));
    connect(this, SIGNAL(moveDocumentOffset(const QPoint&)), d->viewportWidget, SLOT(documentOffsetMoved(const QPoint&)));
}

KoCanvasController::~KoCanvasController()
{
    delete d;
}

void KoCanvasController::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx);
    Q_UNUSED(dy);
    setDocumentOffset();
}

void KoCanvasController::setDrawShadow(bool drawShadow)
{
    d->viewportWidget->setDrawShadow(drawShadow);
}


void KoCanvasController::resizeEvent(QResizeEvent * resizeEvent)
{
    emit sizeChanged(resizeEvent->size());

    // XXX: When resizing, keep the area we're looking at now in the
    // center of the resized view.
    resetScrollBars();
    setDocumentOffset();
}

void KoCanvasController::setCanvas(KoCanvasBase *canvas)
{
    Q_ASSERT(canvas); // param is not null
    if (d->canvas) {
        emit canvasRemoved(this);
        canvas->setCanvasController(0);
        d->canvas->canvasWidget()->removeEventFilter(this);
    }
    d->viewportWidget->setCanvas(canvas->canvasWidget());
    d->canvas = canvas;
    d->canvas->canvasWidget()->installEventFilter(this);
    d->canvas->canvasWidget()->setMouseTracking(true);
    canvas->setCanvasController(this);
    setFocusProxy(d->canvas->canvasWidget());

    emit canvasSet(this);
    QTimer::singleShot(0, this, SLOT(activate()));
}

KoCanvasBase* KoCanvasController::canvas() const
{
    return d->canvas;
}

void KoCanvasController::changeCanvasWidget(QWidget *widget)
{
    Q_ASSERT(d->viewportWidget->canvas());
    widget->setCursor(d->viewportWidget->canvas()->cursor());
    d->viewportWidget->canvas()->removeEventFilter(this);
    d->viewportWidget->setCanvas(widget);
    widget->installEventFilter(this);
    widget->setMouseTracking(true);
}

int KoCanvasController::visibleHeight() const
{
    if (d->canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int height1;
    if (canvasWidget == 0)
        height1 = viewport()->height();
    else
        height1 = qMin(viewport()->height(), canvasWidget->height());
    int height2 = height();
    return qMin(height1, height2);
}

int KoCanvasController::visibleWidth() const
{
    if (d->canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int width1;
    if (canvasWidget == 0)
        width1 = viewport()->width();
    else
        width1 = qMin(viewport()->width(), canvasWidget->width());
    int width2 = width();
    return qMin(width1, width2);
}

void KoCanvasController::setCanvasMode(CanvasMode mode)
{
    d->canvasMode = mode;
    switch (mode) {
    case AlignTop:
        d->preferredCenterFractionX = 0;
        d->preferredCenterFractionY = 0.5;
        break;
    case Centered:
        d->preferredCenterFractionX = 0.5;
        d->preferredCenterFractionY = 0.5;
        break;
    case Infinite:
    case Presentation:
        d->preferredCenterFractionX = 0;
        d->preferredCenterFractionY = 0;
        break;
    };
}

KoCanvasController::CanvasMode KoCanvasController::canvasMode() const
{
    return d->canvasMode;
}

int KoCanvasController::canvasOffsetX() const
{
    int offset = 0;

    if (d->canvas) {
        offset = d->canvas->canvasWidget()->x() + frameWidth();
    }

    if (horizontalScrollBar()) {
        offset -= horizontalScrollBar()->value();
    }

    return offset;
}

int KoCanvasController::canvasOffsetY() const
{
    int offset = 0;

    if (d->canvas) {
        offset = d->canvas->canvasWidget()->y() + frameWidth();
    }

    if (verticalScrollBar()) {
        offset -= verticalScrollBar()->value();
    }

    return offset;
}

void KoCanvasController::updateCanvasOffsetX()
{
    emit canvasOffsetXChanged(canvasOffsetX());
    if (d->ignoreScrollSignals)
        return;
    d->preferredCenterFractionX = (horizontalScrollBar()->value() + horizontalScrollBar()->pageStep() / 2.0) / d->documentSize.width();
}

void KoCanvasController::updateCanvasOffsetY()
{
    emit canvasOffsetYChanged(canvasOffsetY());
    if (d->ignoreScrollSignals)
        return;
    d->preferredCenterFractionY = (verticalScrollBar()->value() + verticalScrollBar()->pageStep() / 2.0) / d->documentSize.height();
}

bool KoCanvasController::eventFilter(QObject* watched, QEvent* event)
{
    if (d->canvas && d->canvas->canvasWidget() && (watched == d->canvas->canvasWidget())) {
        if ((event->type() == QEvent::Resize) || event->type() == QEvent::Move) {
            updateCanvasOffsetX();
            updateCanvasOffsetY();
        } else if (event->type() == QEvent::MouseMove || event->type() == QEvent::TabletMove) {
            emitPointerPositionChangedSignals(event);
        }
    }
    return false;
}

void KoCanvasController::emitPointerPositionChangedSignals(QEvent *event)
{
    if (!d->canvas) return;
    if (!d->canvas->viewConverter()) return;

    QPoint pointerPos;
    QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (mouseEvent) {
        pointerPos = mouseEvent->pos();
    } else {
        QTabletEvent * tabletEvent = dynamic_cast<QTabletEvent*>(event);
        if (tabletEvent) {
            pointerPos = tabletEvent->pos();
        }
    }

    QPoint pixelPos = (pointerPos - d->canvas->documentOrigin()) + d->documentOffset;
    QPointF documentPos = d->canvas->viewConverter()->viewToDocument(pixelPos);

    emit documentMousePositionChanged(documentPos);
    emit canvasMousePositionChanged(pointerPos);
}

void KoCanvasController::ensureVisible(KoShape *shape)
{
    Q_ASSERT(shape);
    ensureVisible(shape->boundingRect());
}

void KoCanvasController::ensureVisible(const QRectF &rect, bool smooth)
{
    QRect currentVisible(qMax(0, -canvasOffsetX()), qMax(0, -canvasOffsetY()), visibleWidth(), visibleHeight());

    // convert the document based rect into a canvas based rect
    QRect viewRect = d->canvas->viewConverter()->documentToView(rect).toRect();
    viewRect.translate(d->canvas->documentOrigin());
    if (! viewRect.isValid() || currentVisible.contains(viewRect))
        return; // its visible. Nothing to do.

    // if we move, we move a little more so the amount of times we have to move is less.
    int jumpWidth = smooth ? 0 : currentVisible.width() / 5;
    int jumpHeight = smooth ? 0 : currentVisible.height() / 5;
    if (!smooth && viewRect.width() + jumpWidth > currentVisible.width())
        jumpWidth = 0;
    if (!smooth && viewRect.height() + jumpHeight > currentVisible.height())
        jumpHeight = 0;

    int horizontalMove = 0;
    if (currentVisible.width() <= viewRect.width())      // center view
        horizontalMove = viewRect.center().x() - currentVisible.center().x();
    else if (currentVisible.x() > viewRect.x())          // move left
        horizontalMove = viewRect.x() - currentVisible.x() - jumpWidth;
    else if (currentVisible.right() < viewRect.right())  // move right
        horizontalMove = viewRect.right() - qMax(0, currentVisible.right() - jumpWidth);

    int verticalMove = 0;
    if (currentVisible.height() <= viewRect.height())       // center view
        verticalMove = viewRect.center().y() - currentVisible.center().y();
    if (currentVisible.y() > viewRect.y())               // move up
        verticalMove = viewRect.y() - currentVisible.y() - jumpHeight;
    else if (currentVisible.bottom() < viewRect.bottom()) // move down
        verticalMove = viewRect.bottom() - qMax(0, currentVisible.bottom() - jumpHeight);

    pan(QPoint(horizontalMove, verticalMove));
}

void KoCanvasController::recenterPreferred()
{
    if (viewport()->width() >= d->documentSize.width()
            && viewport()->height() >= d->documentSize.height())
        return; // no need to center when image is smaller than viewport
    const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
    d->ignoreScrollSignals = true;

    QPoint center = QPoint(int(d->documentSize.width() * d->preferredCenterFractionX),
                           int(d->documentSize.height() * d->preferredCenterFractionY));

    // convert into a viewport based point
    center.rx() += d->canvas->canvasWidget()->x() + frameWidth();
    center.ry() += d->canvas->canvasWidget()->y() + frameWidth();

    // calculate the difference to the viewport centerpoint
    QPoint topLeft = center - 0.5 * QPoint(viewport()->width(), viewport()->height());

    QScrollBar *hBar = horizontalScrollBar();
    // try to centralize the centerpoint which we want to make visible
    topLeft.rx() = qMax(topLeft.x(), hBar->minimum());
    topLeft.rx() = qMin(topLeft.x(), hBar->maximum());
    hBar->setValue(topLeft.x());

    QScrollBar *vBar = verticalScrollBar();
    topLeft.ry() = qMax(topLeft.y(), vBar->minimum());
    topLeft.ry() = qMin(topLeft.y(), vBar->maximum());
    vBar->setValue(topLeft.y());
    d->ignoreScrollSignals = oldIgnoreScrollSignals;
}

void KoCanvasController::zoomIn(const QPoint &center)
{
    zoomBy(center, sqrt(2.0));
}

void KoCanvasController::zoomOut(const QPoint &center)
{
    zoomBy(center, sqrt(0.5));
}

void KoCanvasController::zoomBy(const QPoint &center, qreal zoom)
{
    d->preferredCenterFractionX = 1.0 * center.x() / d->documentSize.width();
    d->preferredCenterFractionY = 1.0 * center.y() / d->documentSize.height();

    const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
    d->ignoreScrollSignals = true;
    emit zoomBy(zoom);
    d->ignoreScrollSignals = oldIgnoreScrollSignals;
    recenterPreferred();
    d->canvas->canvasWidget()->update();
}

void KoCanvasController::zoomTo(const QRect &viewRect)
{
    qreal scale;

    if (1.0 * viewport()->width() / viewRect.width() > 1.0 * viewport()->height() / viewRect.height())
        scale = 1.0 * viewport()->height() / viewRect.height();
    else
        scale = 1.0 * viewport()->width() / viewRect.width();

    const qreal preferredCenterFractionX = 1.0 * viewRect.center().x() / d->documentSize.width();
    const qreal preferredCenterFractionY = 1.0 * viewRect.center().y() / d->documentSize.height();

    emit zoomBy(scale);

    d->preferredCenterFractionX = preferredCenterFractionX;
    d->preferredCenterFractionY = preferredCenterFractionY;
    recenterPreferred();
    d->canvas->canvasWidget()->update();
}

void KoCanvasController::setToolOptionWidgets(const QMap<QString, QWidget *>&widgetMap)
{
    KoView *view=0;
    QWidget *w = this;
    while (w->parentWidget()) {
        view = qobject_cast<KoView*>(w);
        if(view) {
            emit toolOptionWidgetsChanged(widgetMap, view);
            break;
        }
        w = w->parentWidget();
    }
}

void KoCanvasController::setDocumentSize(const QSize & sz, bool recalculateCenter)
{
    if (!recalculateCenter) {
        // assume the distance from the top stays equal and recalculate the center.
        d->preferredCenterFractionX = d->documentSize.width() * d->preferredCenterFractionX / sz.width();
        d->preferredCenterFractionY = d->documentSize.height() * d->preferredCenterFractionY / sz.height();
    }

    const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
    d->ignoreScrollSignals = true;
    d->documentSize = sz;
    d->viewportWidget->setDocumentSize(sz);
    resetScrollBars();
    d->ignoreScrollSignals = oldIgnoreScrollSignals;

    // in case the document got so small a slider dissapeared; emit the new offset.
    if (!horizontalScrollBar()->isVisible())
        updateCanvasOffsetX();
    if (!verticalScrollBar()->isVisible())
        updateCanvasOffsetY();
}

void KoCanvasController::setDocumentOffset()
{
    // The margins scroll the canvas widget inside the viewport, not
    // the document. The documentOffset is meant the be the value that
    // the canvas must add to the update rect in its paint event, to
    // compensate.

    QPoint pt(horizontalScrollBar()->value(), verticalScrollBar()->value());
    if (pt.x() < d->margin) pt.setX(0);
    if (pt.y() < d->margin) pt.setY(0);
    if (pt.x() > d->documentSize.width()) pt.setX(d->documentSize.width());
    if (pt.y() > d->documentSize.height()) pt.setY(d->documentSize.height());
    emit(moveDocumentOffset(pt));

    QWidget *canvasWidget = d->canvas->canvasWidget();

    if (canvasWidget) {
        if (!canvasIsOpenGL()) {
            QPoint diff = d->documentOffset - pt;
            canvasWidget->scroll(diff.x(), diff.y());
        }
    }

    d->documentOffset = pt;
}

bool KoCanvasController::canvasIsOpenGL() const
{
    QWidget *canvasWidget = d->canvas->canvasWidget();

    if (canvasWidget) {
#ifdef HAVE_OPENGL
        if (qobject_cast<QGLWidget*>(canvasWidget) != 0) {
            return true;
        }
#endif
    }

    return false;
}

void KoCanvasController::resetScrollBars()
{
    // The scrollbar value always points at the top-left corner of the
    // bit of image we paint.

    int docH = d->documentSize.height() + d->margin;
    int docW = d->documentSize.width() + d->margin;
    int drawH = d->viewportWidget->height();
    int drawW = d->viewportWidget->width();

    QScrollBar * hScroll = horizontalScrollBar();
    QScrollBar * vScroll = verticalScrollBar();

    if (docH <= drawH && docW <= drawW) {
        // we need no scrollbars
        vScroll->setRange(0, 0);
        hScroll->setRange(0, 0);
    } else if (docH <= drawH) {
        // we need a horizontal scrollbar only
        vScroll->setRange(0, 0);
        hScroll->setRange(0, docW - drawW);
    } else if (docW <= drawW) {
        // we need a vertical scrollbar only
        hScroll->setRange(0, 0);
        vScroll->setRange(0, docH - drawH);
    } else {
        // we need both scrollbars
        vScroll->setRange(0, docH - drawH);
        hScroll->setRange(0, docW - drawW);
    }

    int fontheight = QFontMetrics(font()).height();

    vScroll->setPageStep(drawH);
    vScroll->setSingleStep(fontheight);
    hScroll->setPageStep(drawW);
    hScroll->setSingleStep(fontheight);

}

void KoCanvasController::pan(const QPoint &distance)
{
    QScrollBar *hBar = horizontalScrollBar();
    if (hBar && hBar->isVisible())
        hBar->setValue(hBar->value() + distance.x());
    QScrollBar *vBar = verticalScrollBar();
    if (vBar && vBar->isVisible())
        vBar->setValue(vBar->value() + distance.y());
}

void KoCanvasController::setPreferredCenter(const QPoint &viewPoint)
{
    d->preferredCenterFractionX = 1.0 * viewPoint.x() / d->documentSize.width();
    d->preferredCenterFractionY = 1.0 * viewPoint.y() / d->documentSize.height();
    recenterPreferred();
}

QPoint KoCanvasController::preferredCenter() const
{
    QPoint center;
    center.setX(qRound(d->preferredCenterFractionX * d->documentSize.width()));
    center.setY(qRound(d->preferredCenterFractionY * d->documentSize.height()));
    return center;
}

void KoCanvasController::paintEvent(QPaintEvent * event)
{
    QPainter gc(viewport());
    d->viewportWidget->handlePaintEvent(gc, event);
}

void KoCanvasController::dragEnterEvent(QDragEnterEvent * event)
{
    d->viewportWidget->handleDragEnterEvent(event);
}

void KoCanvasController::dropEvent(QDropEvent *event)
{
    d->viewportWidget->handleDropEvent(event);
}

void KoCanvasController::dragMoveEvent(QDragMoveEvent *event)
{
    d->viewportWidget->handleDragMoveEvent(event);
}

void KoCanvasController::dragLeaveEvent(QDragLeaveEvent *event)
{
    d->viewportWidget->handleDragLeaveEvent(event);
}

void KoCanvasController::keyPressEvent(QKeyEvent *event)
{
    KoToolManager::instance()->switchToolByShortcut(event);
}

void KoCanvasController::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
        const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
        d->ignoreScrollSignals = true;

        const QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
        const QPoint mousePos(event->pos() + offset);
        const qreal zoomLevel = event->delta() > 0 ? sqrt(2.0) : sqrt(0.5);

        QPointF oldCenter = preferredCenter();
        if (visibleWidth() >= d->documentSize.width())
            oldCenter.rx() = d->documentSize.width() * 0.5;
        if (visibleHeight() >= d->documentSize.height())
            oldCenter.ry() = d->documentSize.height() * 0.5;

        const QPointF newCenter = mousePos - (1.0 / zoomLevel) * (mousePos - oldCenter);

        if (event->delta() > 0)
            zoomIn(newCenter.toPoint());
        else
            zoomOut(newCenter.toPoint());
        event->accept();

        d->ignoreScrollSignals = oldIgnoreScrollSignals;
    } else
        QAbstractScrollArea::wheelEvent(event);
}

bool KoCanvasController::focusNextPrevChild(bool)
{
    // we always return false meaning the canvas takes keyboard focus, but never gives it away.
    return false;
}

void KoCanvasController::setMargin(int margin)
{
    d->margin = margin;
    Q_ASSERT(d->viewportWidget);
    d->viewportWidget->setMargin(margin);
}

int KoCanvasController::margin() const
{
    return d->margin;
}

void KoCanvasController::activate()
{
    QWidget *parent = this;
    while (parent->parentWidget())
        parent = parent->parentWidget();

    KoMainWindow *mw = dynamic_cast<KoMainWindow*>(parent);
    if (! mw)
        return;

    foreach(QDockWidget *docker, mw->dockWidgets()) {
        KoCanvasObserver *observer = dynamic_cast<KoCanvasObserver*>(docker);
        if (observer)
            observer->setCanvas(canvas());
    }
}

void KoCanvasController::addGuideLine(Qt::Orientation orientation, int viewPosition)
{
    KoGuidesTool * guidesTool = KoToolManager::instance()->guidesTool(d->canvas);
    if (! guidesTool)
        return;
    // check if the canvas does provide access to guides data
    if (! d->canvas->guidesData())
        return;

    if (orientation == Qt::Horizontal)
        guidesTool->addGuideLine(orientation, d->canvas->viewConverter()->viewToDocumentY(viewPosition));
    else
        guidesTool->addGuideLine(orientation, d->canvas->viewConverter()->viewToDocumentX(viewPosition));

    KoToolManager::instance()->switchToolTemporaryRequested(guidesTool->toolId());
}

#include "KoCanvasController.moc"
