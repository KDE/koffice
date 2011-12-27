/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
    Copyright (C) 2011 Thomas Zander <zander@kde.org>

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

#include "KoPACanvas.h"

#include <KShapeManager.h>
#include <KoZoomHandler.h>
#include <KToolProxy.h>
#include <KOdfText.h>

#include "KoPADocument.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAPageProvider.h"

#include <kxmlguifactory.h>
#include <QMenu>

class KoPACanvas::Private
{
public:
    Private(KoPADocument * doc)
    : view(0)
    , doc(doc)
    , shapeManager(0)
    , masterShapeManager(0)
    , toolProxy(0)
    {}

    ~Private()
    {
        delete toolProxy;
        delete masterShapeManager;
        delete shapeManager;
    }

    ///< the origin of the page rect inside the canvas in document points
    QPointF origin() const
    {
        return view->viewMode()->origin();
    }

    KoPAViewBase * view;
    KoPADocument * doc;
    KShapeManager * shapeManager;
    KShapeManager * masterShapeManager;
    KToolProxy * toolProxy;
    QPoint documentOffset;
};

KoPACanvas::KoPACanvas(KoPAViewBase *view, KoPADocument *doc, QWidget *parent)
    : QWidget(parent),
    KCanvasBase(doc),
    d(new Private(doc))
{
    d->shapeManager = new KShapeManager(this);
    d->masterShapeManager = new KShapeManager(this);
    d->toolProxy = new KToolProxy(this);

    setView(view);
    setFocusPolicy(Qt::StrongFocus);
    // this is much faster than painting it in the paintevent
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    updateSize();
    setAttribute(Qt::WA_InputMethodEnabled, true);
}

KoPACanvas::~KoPACanvas()
{
    delete d;
}

void KoPACanvas::setView(KoPAViewBase *view)
{
    d->view = view;
}

KoPADocument* KoPACanvas::document() const
{
    return d->doc;
}

KToolProxy* KoPACanvas::toolProxy() const
{
    return d->toolProxy;
}

KoPAViewBase* KoPACanvas::koPAView() const
{
    return d->view;
}

QPoint KoPACanvas::documentOrigin() const
{
    return viewConverter()->documentToView(d->origin()).toPoint();
}

void KoPACanvas::setDocumentOrigin(const QPointF &o)
{
    d->view->viewMode()->setOrigin(o);
}

void KoPACanvas::gridSize(qreal *horizontal, qreal *vertical) const
{
    *horizontal = d->doc->gridData().gridX();
    *vertical = d->doc->gridData().gridY();
}

bool KoPACanvas::snapToGrid() const
{
    return d->doc->gridData().snapToGrid();
}

void KoPACanvas::addCommand(QUndoCommand *command)
{
    d->doc->addCommand(command);
}

KShapeManager * KoPACanvas::shapeManager() const
{
    return d->shapeManager;
}

KShapeManager * KoPACanvas::masterShapeManager() const
{
    return d->masterShapeManager;
}

const KViewConverter * KoPACanvas::viewConverter() const
{
    return d->view->viewMode()->viewConverter(const_cast<KoPACanvas *>(this));
}

KUnit KoPACanvas::unit() const
{
    return d->doc->unit();
}

const QPoint &KoPACanvas::documentOffset() const
{
    return d->documentOffset;
}

void KoPACanvas::setDocumentOffset(const QPoint &offset) {
    d->documentOffset = offset;
}

QPoint KoPACanvas::widgetToView(const QPoint&p) const
{
    return p - viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvas::widgetToView(const QRect &r) const
{
    return r.translated(viewConverter()->documentToView(-d->origin()).toPoint());
}

QPoint KoPACanvas::viewToWidget(const QPoint &p) const
{
    return p + viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvas::viewToWidget(const QRect &r) const
{
    return r.translated(viewConverter()->documentToView(d->origin()).toPoint());
}

KGuidesData * KoPACanvas::guidesData()
{
    return &d->doc->guidesData();
}

void KoPACanvas::paint(QPainter &painter, const QRectF paintRect) {

    KoPAPage *activePage(d->view->activePage());
    if (d->view->activePage()) {
        int pageNumber = d->doc->pageIndex(d->view->activePage()) + 1;
        QVariant var = d->doc->resourceManager()->resource(KOdfText::PageProvider);
        static_cast<KoPAPageProvider*>(var.value<void*>())->setPageData(pageNumber, activePage);
        d->view->viewMode()->paint(this, painter, paintRect);
    }
}

QWidget* KoPACanvas::canvasWidget()
{
    return this;
}

const QWidget* KoPACanvas::canvasWidget() const
{
    return this;
}

void KoPACanvas::updateSize()
{
    QSize size;

    if (koPAView()->activePage()) {
        KOdfPageLayoutData pageLayout = koPAView()->activePage()->pageLayout();
        size.setWidth(qRound(koPAView()->zoomHandler()->zoomItX(pageLayout.width)));
        size.setHeight(qRound(koPAView()->zoomHandler()->zoomItX(pageLayout.height)));
    }

    emit documentSize(size);
}

void KoPACanvas::updateCanvas(const QRectF &rc)
{
    QRect clipRect(viewToWidget(viewConverter()->documentToView(rc).toRect()));
    clipRect.adjust(-2, -2, 2, 2); // Resize to fit anti-aliasing
    clipRect.moveTopLeft(clipRect.topLeft() - documentOffset());
    update(clipRect);

    emit canvasUpdated();
}

bool KoPACanvas::event(QEvent *e)
{
    if (toolProxy()) {
        toolProxy()->processEvent(e);
    }
    return QWidget::event(e);
}

void KoPACanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    paint(painter, event->rect());
    painter.end();
}

void KoPACanvas::tabletEvent(QTabletEvent *event)
{
    koPAView()->viewMode()->tabletEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::mousePressEvent(QMouseEvent *event)
{
    koPAView()->viewMode()->mousePressEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));

    if (!event->isAccepted() && event->button() == Qt::RightButton)
    {
        showContextMenu(event->globalPos(), toolProxy()->popupActionList());
        event->setAccepted(true);
    }
}

void KoPACanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    koPAView()->viewMode()->mouseDoubleClickEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::mouseMoveEvent(QMouseEvent *event)
{
    koPAView()->viewMode()->mouseMoveEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::mouseReleaseEvent(QMouseEvent *event)
{
    koPAView()->viewMode()->mouseReleaseEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::keyPressEvent(QKeyEvent *event)
{
    koPAView()->viewMode()->keyPressEvent(event);
    if (! event->isAccepted()) {
        if (event->key() == Qt::Key_Backtab
                || (event->key() == Qt::Key_Tab && (event->modifiers() & Qt::ShiftModifier)))
            focusNextPrevChild(false);
        else if (event->key() == Qt::Key_Tab)
            focusNextPrevChild(true);
    }
}

void KoPACanvas::keyReleaseEvent(QKeyEvent *event)
{
    koPAView()->viewMode()->keyReleaseEvent(event);
}

void KoPACanvas::wheelEvent (QWheelEvent * event)
{
    koPAView()->viewMode()->wheelEvent(event, viewConverter()->viewToDocument(widgetToView(event->pos() + documentOffset())));
}

void KoPACanvas::closeEvent(QCloseEvent * event)
{
    koPAView()->viewMode()->closeEvent(event);
}

void KoPACanvas::updateInputMethodInfo()
{
    updateMicroFocus();
}

QVariant KoPACanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return toolProxy()->inputMethodQuery(query, *(viewConverter()));
}

void KoPACanvas::inputMethodEvent(QInputMethodEvent *event)
{
    toolProxy()->inputMethodEvent(event);
}

void KoPACanvas::resizeEvent(QResizeEvent * event)
{
    emit sizeChanged(event->size());
}

void KoPACanvas::showContextMenu(const QPoint &globalPos, const QList<QAction*> &actionList)
{
    KoPAView *view = dynamic_cast<KoPAView*>(koPAView());
    if (!view || !view->factory()) return;

    view->unplugActionList("toolproxy_action_list");
    view->plugActionList("toolproxy_action_list", actionList);


    QMenu *menu = dynamic_cast<QMenu*>(view->factory()->container("default_canvas_popup", view));

    if (menu)
        menu->exec(globalPos);
}

