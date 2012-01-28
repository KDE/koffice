/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAViewMode.h"

#include "KoPACanvas.h"
#include "KoPAMasterPage.h"
#include "KoPAView.h"
#include "KoPADocument.h"

#include <KCanvasController.h>
#include <KOdfPageLayoutData.h>
#include <KShapeManager.h>

#include <KDebug>

#include <QCloseEvent>

KoPAViewMode::KoPAViewMode(KoPAView * view, KoPACanvas * canvas)
: m_canvas(canvas)
, m_toolProxy(canvas->toolProxy())
, m_view(view)
{
}

KoPAViewMode::~KoPAViewMode()
{
}

void KoPAViewMode::closeEvent(QCloseEvent * event)
{
    event->ignore();
}

void KoPAViewMode::setMasterMode(bool master)
{
    Q_UNUSED(master);
}

bool KoPAViewMode::masterMode()
{
    return false;
}

void KoPAViewMode::activate(KoPAViewMode * previousViewMode)
{
    Q_UNUSED(previousViewMode);
    m_canvas->updateSize();
    updateActivePage(m_view->activePage());
    // this is done to set the preferred center
    m_canvas->canvasController()->setCanvasMode(KCanvasController::Centered);
    m_canvas->canvasController()->recenterPreferred();
}

void KoPAViewMode::deactivate()
{
}

KoPACanvas * KoPAViewMode::canvas() const
{
    return m_canvas;
}

KoPAView * KoPAViewMode::view() const
{
    return m_view;
}

KViewConverter * KoPAViewMode::viewConverter(KoPACanvas * canvas)
{
    return m_view->KoPAView::viewConverter(canvas);
}

void KoPAViewMode::updateActivePage(KoPAPage *page)
{
    m_view->doUpdateActivePage(page);
}

void KoPAViewMode::addShape(KShape *shape)
{
    // the KShapeController sets the active layer as parent
    KoPAPage *page(m_view->kopaDocument()->pageByShape(shape));
    Q_ASSERT(page); // otherwise there is a bug in our view
    if (page == m_view->activePage() || page == m_view->activePage()->masterPage()) {
        m_view->kopaCanvas()->shapeManager()->addShape(shape);
    }
}

void KoPAViewMode::removeShape(KShape *shape)
{
    m_view->kopaCanvas()->shapeManager()->remove(shape);
}

void KoPAViewMode::changePageLayout(const KOdfPageLayoutData &pageLayout, bool applyToDocument, QUndoCommand *parent)
{
    Q_UNUSED(pageLayout);
    Q_UNUSED(applyToDocument);
    Q_UNUSED(parent);
}

QPointF KoPAViewMode::origin() const
{
    return m_origin;
}

void KoPAViewMode::setOrigin(const QPointF &o)
{
    m_origin = o;
}

#include <KoPAViewMode.moc>

