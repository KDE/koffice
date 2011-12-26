/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@kde.org>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCViewModePreviewPageEffect.h"

#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QtGui/QDesktopWidget>

#include <kdebug.h>

#include <KPointerEvent.h>
#include <KoPageApp.h>
#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAView.h>
#include <KoZoomHandler.h>

#include "SCDocument.h"
#include "SCEndOfSlideShowPage.h"
#include "SCPage.h"
#include "pageeffects/SCPageEffect.h"
#include "pageeffects/SCPageEffectRunner.h"

SCViewModePreviewPageEffect::SCViewModePreviewPageEffect(KoPAViewBase * view, KoPACanvasBase * canvas)
: KoPAViewMode(view, canvas)
, m_savedViewMode(0)
, m_pageEffect(0)
, m_pageEffectRunner(0)
, m_page(0)
, m_prevpage(0)
{
    connect(&m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animate()));
    connect(&m_timeLine, SIGNAL(finished()), this, SLOT(activateSavedViewMode()));
}

SCViewModePreviewPageEffect::~SCViewModePreviewPageEffect()
{
}


void SCViewModePreviewPageEffect::paint(KoPACanvasBase* canvas, QPainter &painter, const QRectF &paintRect)
{
    Q_UNUSED(canvas);
    Q_UNUSED(paintRect);
    //QRect framerect = canvas->rect();

    if (m_pageEffectRunner && m_timeLine.state() == QTimeLine::Running) {
        m_pageEffectRunner->paint(painter);
    }
}


void SCViewModePreviewPageEffect::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}


void SCViewModePreviewPageEffect::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}


void SCViewModePreviewPageEffect::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}


void SCViewModePreviewPageEffect::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}


void SCViewModePreviewPageEffect::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}


void SCViewModePreviewPageEffect::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}


void SCViewModePreviewPageEffect::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}


void SCViewModePreviewPageEffect::wheelEvent(QWheelEvent * event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}


void SCViewModePreviewPageEffect::activate(KoPAViewMode * previousViewMode)
{
    m_savedViewMode = previousViewMode;               // store the previous view mode

    // the update of the canvas is needed so that the old page gets drawn fully before the effect starts
    canvas()->repaint();
    m_timeLine.setDuration(m_pageEffect->duration());
    m_timeLine.setCurrentTime(0);
    m_timeLine.start();
}


void SCViewModePreviewPageEffect::deactivate()
{
    m_savedViewMode = 0;               // store the previous view mode
}


void SCViewModePreviewPageEffect::updateActivePage(KoPAPage *page)
{
    m_view->setActivePage(page);
}


void SCViewModePreviewPageEffect::activateSavedViewMode()
{
    m_view->setViewMode(m_savedViewMode);
    //canvas()->update();
}


void SCViewModePreviewPageEffect::setPageEffect(SCPageEffect* pageEffect, SCPage* page, SCPage* prevpage)
{
    if(m_savedViewMode) {//stop the previous playing
        activateSavedViewMode();
    }

    delete m_pageEffect;
    m_pageEffect = pageEffect;
    delete m_pageEffectRunner;
    m_pageEffectRunner = 0;

    m_page = page;
    m_prevpage = prevpage;

    if(m_page) {
        updatePixmaps();

        if(m_pageEffect) {
            m_pageEffectRunner = new SCPageEffectRunner(m_oldPage, m_newPage, canvas()->canvasWidget(), m_pageEffect);
        }
    }
}

void SCViewModePreviewPageEffect::animate()
{
    if (m_pageEffectRunner) {
        m_pageEffectRunner->next(m_timeLine.currentTime());
    }
}

void SCViewModePreviewPageEffect::updatePixmaps()
{
    if(!m_page)
        return;

    QSize size = canvas()->canvasWidget()->size(); // TODO wrong this should be page/document size

    m_newPage = m_page->thumbnail(size);

    if(m_newPage.isNull())
        return;

    if(m_prevpage && m_prevpage != m_page)
    {
        m_oldPage = m_prevpage->thumbnail(size);
    }
    else
    {
        QPixmap oldPage(size);
        oldPage.fill(QColor(Qt::black));
        m_oldPage = oldPage;
    }
}

#include "SCViewModePreviewPageEffect.moc"
