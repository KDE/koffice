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
    int margin; // The viewport margin around the document
    QSize documentSize;
    QPoint documentOffset;
    Viewport * viewportWidget;
    qreal preferredCenterFractionX;
    qreal preferredCenterFractionY;
    bool ignoreScrollSignals;
};

KoCanvasController::KoCanvasController(QWidget *parent)
        : QAbstractScrollArea(parent),
        m_d(new Private())
{
    setFrameShape(NoFrame);
    m_d->viewportWidget = new Viewport(this);
    setViewport(m_d->viewportWidget);

    setAutoFillBackground(false);
    /*
      Fixes:   apps starting at zero zoom.
      Details: Since the document is set on the mainwindow before loading commences the inial show/layout can choose
          to set the document to be very small, even to be zero pixels tall.  Setting a sane minimum size on the
          widget means we no loger get rounding errors in zooming and we no longer end up with zero-zoom.
      Note: KoPage apps should probably startup with a sane document size; for Krita that's impossible
     */
    setMinimumSize(QSize(50, 50));

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));

    setMouseTracking(true);

    KConfigGroup cfg = KGlobal::config()->group("");
    m_d->margin = cfg.readEntry("canvasmargin",  0);

    connect(this, SIGNAL(moveDocumentOffset(const QPoint&)), m_d->viewportWidget, SLOT(documentOffsetMoved(const QPoint&)));
}

KoCanvasController::~KoCanvasController()
{
    delete m_d;
}

void KoCanvasController::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx);
    Q_UNUSED(dy);
    setDocumentOffset();
}

void KoCanvasController::setDrawShadow(bool drawShadow)
{
    m_d->viewportWidget->setDrawShadow(drawShadow);
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
    if (m_d->canvas) {
        emit canvasRemoved(this);
        canvas->setCanvasController(0);
        m_d->canvas->canvasWidget()->removeEventFilter(this);
    }
    m_d->viewportWidget->setCanvas(canvas->canvasWidget());
    m_d->canvas = canvas;
    m_d->canvas->canvasWidget()->installEventFilter(this);
    m_d->canvas->canvasWidget()->setMouseTracking(true);
    canvas->setCanvasController(this);

    emit canvasSet(this);
    QTimer::singleShot(0, this, SLOT(activate()));
}

KoCanvasBase* KoCanvasController::canvas() const
{
    return m_d->canvas;
}

void KoCanvasController::changeCanvasWidget(QWidget *widget)
{
    Q_ASSERT(m_d->viewportWidget->canvas());
    widget->setCursor(m_d->viewportWidget->canvas()->cursor());
    m_d->viewportWidget->canvas()->removeEventFilter(this);
    m_d->viewportWidget->setCanvas(widget);
    widget->installEventFilter(this);
    widget->setMouseTracking(true);
}

int KoCanvasController::visibleHeight() const
{
    if (m_d->canvas == 0)
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
    if (m_d->canvas == 0)
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
    m_d->canvasMode = mode;
    switch (mode) {
    case AlignTop:
        m_d->preferredCenterFractionX = 0;
        m_d->preferredCenterFractionY = 0.5;
        break;
    case Centered:
        m_d->preferredCenterFractionX = 0.5;
        m_d->preferredCenterFractionY = 0.5;
        break;
    case Infinite:
    case Presentation:
        m_d->preferredCenterFractionX = 0;
        m_d->preferredCenterFractionY = 0;
        break;
    };
}

KoCanvasController::CanvasMode KoCanvasController::canvasMode() const
{
    return m_d->canvasMode;
}

int KoCanvasController::canvasOffsetX() const
{
    int offset = 0;

    if (m_d->canvas) {
        offset = m_d->canvas->canvasWidget()->x() + frameWidth();
    }

    if (horizontalScrollBar()) {
        offset -= horizontalScrollBar()->value();
    }

    return offset;
}

int KoCanvasController::canvasOffsetY() const
{
    int offset = 0;

    if (m_d->canvas) {
        offset = m_d->canvas->canvasWidget()->y() + frameWidth();
    }

    if (verticalScrollBar()) {
        offset -= verticalScrollBar()->value();
    }

    return offset;
}

void KoCanvasController::updateCanvasOffsetX()
{
    if (m_d->ignoreScrollSignals)
        return;
    emit canvasOffsetXChanged(canvasOffsetX());
    if (m_d->viewportWidget && m_d->viewportWidget->canvas())
        m_d->viewportWidget->canvas()->setFocus(); // workaround ugly bug in Qt that the focus is transferred to the sliders
    m_d->preferredCenterFractionX = (horizontalScrollBar()->value() + horizontalScrollBar()->pageStep() / 2.0) / m_d->documentSize.width();
}

void KoCanvasController::updateCanvasOffsetY()
{
    if (m_d->ignoreScrollSignals)
        return;
    emit canvasOffsetYChanged(canvasOffsetY());
    if (m_d->viewportWidget && m_d->viewportWidget->canvas())
        m_d->viewportWidget->canvas()->setFocus(); // workaround ugly bug in Qt that the focus is transferred to the sliders
    m_d->preferredCenterFractionY = (verticalScrollBar()->value() + verticalScrollBar()->pageStep() / 2.0) / m_d->documentSize.height();
}

bool KoCanvasController::eventFilter(QObject* watched, QEvent* event)
{
    if (m_d->canvas && m_d->canvas->canvasWidget() && (watched == m_d->canvas->canvasWidget())) {
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
    if (!m_d->canvas) return;
    if (!m_d->canvas->viewConverter()) return;

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

    QPoint pixelPos = (pointerPos - m_d->canvas->documentOrigin()) + m_d->documentOffset;
    QPointF documentPos = m_d->canvas->viewConverter()->viewToDocument(pixelPos);

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
    QRect viewRect = m_d->canvas->viewConverter()->documentToView(rect).toRect();
    viewRect.translate(m_d->canvas->documentOrigin());
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
    if (viewport()->width() >= m_d->documentSize.width()
            && viewport()->height() >= m_d->documentSize.height())
        return; // no need to center when image is smaller than viewport
    const bool oldIgnoreScrollSignals = m_d->ignoreScrollSignals;
    m_d->ignoreScrollSignals = true;

    QPoint center = QPoint(int(m_d->documentSize.width() * m_d->preferredCenterFractionX),
                           int(m_d->documentSize.height() * m_d->preferredCenterFractionY));

    // convert into a viewport based point
    center.rx() += m_d->canvas->canvasWidget()->x() + frameWidth();
    center.ry() += m_d->canvas->canvasWidget()->y() + frameWidth();

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
    m_d->ignoreScrollSignals = oldIgnoreScrollSignals;
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
    m_d->preferredCenterFractionX = 1.0 * center.x() / m_d->documentSize.width();
    m_d->preferredCenterFractionY = 1.0 * center.y() / m_d->documentSize.height();

    const bool oldIgnoreScrollSignals = m_d->ignoreScrollSignals;
    m_d->ignoreScrollSignals = true;
    emit zoomBy(zoom);
    m_d->ignoreScrollSignals = oldIgnoreScrollSignals;
    recenterPreferred();
    m_d->canvas->canvasWidget()->update();
}

void KoCanvasController::zoomTo(const QRect &viewRect)
{
    qreal scale;

    if (1.0 * viewport()->width() / viewRect.width() > 1.0 * viewport()->height() / viewRect.height())
        scale = 1.0 * viewport()->height() / viewRect.height();
    else
        scale = 1.0 * viewport()->width() / viewRect.width();

    const qreal preferredCenterFractionX = 1.0 * viewRect.center().x() / m_d->documentSize.width();
    const qreal preferredCenterFractionY = 1.0 * viewRect.center().y() / m_d->documentSize.height();

    emit zoomBy(scale);

    m_d->preferredCenterFractionX = preferredCenterFractionX;
    m_d->preferredCenterFractionY = preferredCenterFractionY;
    recenterPreferred();
    m_d->canvas->canvasWidget()->update();
}

void KoCanvasController::setToolOptionWidgets(const QMap<QString, QWidget *>&widgetMap)
{
    emit toolOptionWidgetsChanged(widgetMap);
}

void KoCanvasController::setDocumentSize(const QSize & sz, bool recalculateCenter)
{
    if (!recalculateCenter) {
        // assume the distance from the top stays equal and recalculate the center.
        m_d->preferredCenterFractionX = m_d->documentSize.width() * m_d->preferredCenterFractionX / sz.width();
        m_d->preferredCenterFractionY = m_d->documentSize.height() * m_d->preferredCenterFractionY / sz.height();
    }

    const bool oldIgnoreScrollSignals = m_d->ignoreScrollSignals;
    m_d->ignoreScrollSignals = true;
    m_d->documentSize = sz;
    m_d->viewportWidget->setDocumentSize(sz);
    resetScrollBars();
    m_d->ignoreScrollSignals = oldIgnoreScrollSignals;

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
    if (pt.x() < m_d->margin) pt.setX(0);
    if (pt.y() < m_d->margin) pt.setY(0);
    if (pt.x() > m_d->documentSize.width()) pt.setX(m_d->documentSize.width());
    if (pt.y() > m_d->documentSize.height()) pt.setY(m_d->documentSize.height());
    emit(moveDocumentOffset(pt));

    QWidget *canvasWidget = m_d->canvas->canvasWidget();

    if (canvasWidget) {
        if (!canvasIsOpenGL()) {
            QPoint diff = m_d->documentOffset - pt;
            canvasWidget->scroll(diff.x(), diff.y());
        }
    }

    m_d->documentOffset = pt;
}

bool KoCanvasController::canvasIsOpenGL() const
{
    QWidget *canvasWidget = m_d->canvas->canvasWidget();

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

    int docH = m_d->documentSize.height() + m_d->margin;
    int docW = m_d->documentSize.width() + m_d->margin;
    int drawH = m_d->viewportWidget->height();
    int drawW = m_d->viewportWidget->width();

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
    m_d->preferredCenterFractionX = 1.0 * viewPoint.x() / m_d->documentSize.width();
    m_d->preferredCenterFractionY = 1.0 * viewPoint.y() / m_d->documentSize.height();
    recenterPreferred();
}

QPoint KoCanvasController::preferredCenter() const
{
    QPoint center;
    center.setX(qRound(m_d->preferredCenterFractionX * m_d->documentSize.width()));
    center.setY(qRound(m_d->preferredCenterFractionY * m_d->documentSize.height()));
    return center;
}

void KoCanvasController::paintEvent(QPaintEvent * event)
{
    QPainter gc(viewport());
    m_d->viewportWidget->handlePaintEvent(gc, event);
}

void KoCanvasController::dragEnterEvent(QDragEnterEvent * event)
{
    m_d->viewportWidget->handleDragEnterEvent(event);
}

void KoCanvasController::dropEvent(QDropEvent *event)
{
    m_d->viewportWidget->handleDropEvent(event);
}

void KoCanvasController::dragMoveEvent(QDragMoveEvent *event)
{
    m_d->viewportWidget->handleDragMoveEvent(event);
}

void KoCanvasController::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_d->viewportWidget->handleDragLeaveEvent(event);
}

void KoCanvasController::keyPressEvent(QKeyEvent *event)
{
    KoToolManager::instance()->switchToolByShortcut(event);
}

void KoCanvasController::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
        const bool oldIgnoreScrollSignals = m_d->ignoreScrollSignals;
        m_d->ignoreScrollSignals = true;

        const QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
        const QPoint mousePos(event->pos() + offset);
        const qreal zoomLevel = event->delta() > 0 ? sqrt(2.0) : sqrt(0.5);

        QPointF oldCenter = preferredCenter();
        if (visibleWidth() >= m_d->documentSize.width())
            oldCenter.rx() = m_d->documentSize.width() * 0.5;
        if (visibleHeight() >= m_d->documentSize.height())
            oldCenter.ry() = m_d->documentSize.height() * 0.5;

        const QPointF newCenter = mousePos - (1.0 / zoomLevel) * (mousePos - oldCenter);

        if (event->delta() > 0)
            zoomIn(newCenter.toPoint());
        else
            zoomOut(newCenter.toPoint());
        event->accept();

        m_d->ignoreScrollSignals = oldIgnoreScrollSignals;
    } else
        QAbstractScrollArea::wheelEvent(event);
}

bool KoCanvasController::focusNextPrevChild(bool next)
{
    // we always return false meaning the canvas takes keyboard focus, but never gives it away.
    return false;
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
    KoGuidesTool * guidesTool = KoToolManager::instance()->guidesTool(m_d->canvas);
    if (! guidesTool)
        return;
    // check if the canvas does provide access to guides data
    if (! m_d->canvas->guidesData())
        return;

    if (orientation == Qt::Horizontal)
        guidesTool->addGuideLine(orientation, m_d->canvas->viewConverter()->viewToDocumentY(viewPosition));
    else
        guidesTool->addGuideLine(orientation, m_d->canvas->viewConverter()->viewToDocumentX(viewPosition));

    KoToolManager::instance()->switchToolTemporaryRequested(guidesTool->toolId());
}

#include "KoCanvasController.moc"
