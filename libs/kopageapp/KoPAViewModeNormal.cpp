/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAViewModeNormal.h"

#include <QEvent>
#include <QKeyEvent>
#include <QPainter>

#include <KDebug>

#include <KToolProxy.h>
#include <KShapeManager.h>
#include "KoPACanvas.h"
#include "KoPADocument.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPAView.h"
#include "commands/KoPAChangePageLayoutCommand.h"

KoPAViewModeNormal::KoPAViewModeNormal(KoPAView * view, KoPACanvas * canvas)
: KoPAViewMode(view, canvas)
, m_masterMode(false)
, m_savedPage(0)
{
}

KoPAViewModeNormal::~KoPAViewModeNormal()
{
}

void KoPAViewModeNormal::paint(KoPACanvas *, QPainter &painter, const QRectF &paintRect)
{
    // apply origin and offset.
    painter.translate(-m_canvas->documentOffset());
    QRectF clipRect = paintRect.translated(m_canvas->documentOffset());
    painter.setClipRect(clipRect);
    painter.translate(m_canvas->documentOrigin().x(), m_canvas->documentOrigin().y());

    KViewConverter *converter = m_view->viewConverter(m_canvas);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    m_canvas->shapeManager()->paint(painter, *converter, false);
    painter.restore();

    // paint the page margins
    const KOdfPageLayoutData pl = m_view->activePage()->pageLayout();
    const QRectF marginRect(pl.leftMargin, pl.topMargin,
                       pl.width - pl.leftMargin - pl.rightMargin,
                       pl.height - pl.topMargin - pl.bottomMargin);
    painter.save();
    painter.setPen(Qt::gray);
    painter.drawRect(converter->documentToView(marginRect));
    painter.restore();

    m_canvas->document()->gridData().paintGrid(painter, *converter, paintRect.intersected(marginRect));
    m_canvas->document()->guidesData().paintGuides(painter, *converter, paintRect.intersected(marginRect));

    painter.setRenderHint(QPainter::Antialiasing);
    m_toolProxy->paint(painter, *converter);
}

void KoPAViewModeNormal::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    m_toolProxy->tabletEvent(event, point);
}

void KoPAViewModeNormal::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mousePressEvent(event, point);
}

void KoPAViewModeNormal::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseDoubleClickEvent(event, point);
}

void KoPAViewModeNormal::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseMoveEvent(event, point);
}

void KoPAViewModeNormal::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    m_toolProxy->mouseReleaseEvent(event, point);
}

void KoPAViewModeNormal::keyPressEvent(QKeyEvent *event)
{
    m_toolProxy->keyPressEvent(event);

    if (! event->isAccepted()) {
        event->accept();

        switch (event->key())
        {
        case Qt::Key_Home:
            m_view->navigatePage(KoPageApp::PageFirst);
            break;
        case Qt::Key_PageUp:
            m_view->navigatePage(KoPageApp::PagePrevious);
            break;
        case Qt::Key_PageDown:
            m_view->navigatePage(KoPageApp::PageNext);
            break;
        case Qt::Key_End:
            m_view->navigatePage(KoPageApp::PageLast);
            break;
        default:
            event->ignore();
            break;
        }
    }
}

void KoPAViewModeNormal::keyReleaseEvent(QKeyEvent *event)
{
    m_toolProxy->keyReleaseEvent(event);
}

void KoPAViewModeNormal::wheelEvent(QWheelEvent * event, const QPointF &point)
{
    m_toolProxy->wheelEvent(event, point);
}

void KoPAViewModeNormal::setMasterMode(bool master)
{
    m_masterMode = master;
    KoPAPage * page = dynamic_cast<KoPAPage *>(m_view->activePage());
    if (m_masterMode) {
        if (page) {
            m_view->doUpdateActivePage(page->masterPage());
            m_savedPage = page;
        }
    }
    else if (m_savedPage) {
        m_view->doUpdateActivePage(m_savedPage);
        m_savedPage = 0;
    }
}

bool KoPAViewModeNormal::masterMode()
{
    return m_masterMode;
}

void KoPAViewModeNormal::changePageLayout(const KOdfPageLayoutData &pageLayout, bool applyToDocument, QUndoCommand *parent)
{
    KoPAPage *page = m_view->activePage();
    KoPAMasterPage *masterPage = dynamic_cast<KoPAMasterPage *>(page);
    if (!masterPage) {
        masterPage = static_cast<KoPAPage *>(page)->masterPage();
    }

    new KoPAChangePageLayoutCommand(m_canvas->document(), masterPage, pageLayout, applyToDocument, parent);
}

