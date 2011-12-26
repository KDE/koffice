/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPACanvasBase.h"

#include <KShapeManager.h>
#include <KToolProxy.h>
#include <KOdfText.h>

#include "KoPADocument.h"
#include "KoPAView.h"
#include "KoPAPageProvider.h"

class KoPACanvasBase::Private
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

KoPACanvasBase::KoPACanvasBase(KoPADocument * doc)
    : KCanvasBase(doc)
    , d(new Private(doc))
{
    d->shapeManager = new KShapeManager(this);
    d->masterShapeManager = new KShapeManager(this);
    d->toolProxy = new KToolProxy(this);
}

KoPACanvasBase::~KoPACanvasBase()
{
    delete d;
}

void KoPACanvasBase::setView(KoPAViewBase *view)
{
    d->view = view;
}

KoPADocument* KoPACanvasBase::document() const
{
    return d->doc;
}

KToolProxy* KoPACanvasBase::toolProxy() const
{
    return d->toolProxy;
}

KoPAViewBase* KoPACanvasBase::koPAView() const
{
    return d->view;
}

QPoint KoPACanvasBase::documentOrigin() const
{
    return viewConverter()->documentToView(d->origin()).toPoint();
}

void KoPACanvasBase::setDocumentOrigin(const QPointF &o)
{
    d->view->viewMode()->setOrigin(o);
}

void KoPACanvasBase::gridSize(qreal *horizontal, qreal *vertical) const
{
    *horizontal = d->doc->gridData().gridX();
    *vertical = d->doc->gridData().gridY();
}

bool KoPACanvasBase::snapToGrid() const
{
    return d->doc->gridData().snapToGrid();
}

void KoPACanvasBase::addCommand(QUndoCommand *command)
{
    d->doc->addCommand(command);
}

KShapeManager * KoPACanvasBase::shapeManager() const
{
    return d->shapeManager;
}

KShapeManager * KoPACanvasBase::masterShapeManager() const
{
    return d->masterShapeManager;
}

const KViewConverter * KoPACanvasBase::viewConverter() const
{
    return d->view->viewMode()->viewConverter(const_cast<KoPACanvasBase *>(this));
}

KUnit KoPACanvasBase::unit() const
{
    return d->doc->unit();
}

const QPoint &KoPACanvasBase::documentOffset() const
{
    return d->documentOffset;
}

void KoPACanvasBase::setDocumentOffset(const QPoint &offset) {
    d->documentOffset = offset;
}

QPoint KoPACanvasBase::widgetToView(const QPoint&p) const
{
    return p - viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvasBase::widgetToView(const QRect &r) const
{
    return r.translated(viewConverter()->documentToView(-d->origin()).toPoint());
}

QPoint KoPACanvasBase::viewToWidget(const QPoint &p) const
{
    return p + viewConverter()->documentToView(d->origin()).toPoint();
}

QRect KoPACanvasBase::viewToWidget(const QRect &r) const
{
    return r.translated(viewConverter()->documentToView(d->origin()).toPoint());
}

KGuidesData * KoPACanvasBase::guidesData()
{
    return &d->doc->guidesData();
}

void KoPACanvasBase::paint(QPainter &painter, const QRectF paintRect) {

    KoPAPage *activePage(d->view->activePage());
    if (d->view->activePage()) {
        int pageNumber = d->doc->pageIndex(d->view->activePage()) + 1;
        QVariant var = d->doc->resourceManager()->resource(KOdfText::PageProvider);
        static_cast<KoPAPageProvider*>(var.value<void*>())->setPageData(pageNumber, activePage);
        d->view->viewMode()->paint(this, painter, paintRect);
    }
}
