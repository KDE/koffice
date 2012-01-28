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

#ifndef SCPRESENTERVIEWWIDGET
#define SCPRESENTERVIEWWIDGET

#include <QtGui/QWidget>

class QStackedLayout;

class KoPACanvas;
class KoPAPage;
class KoPAPageThumbnailModel;

class SCViewModePresentation;

class SCPresenterViewBaseInterface;
class SCPresenterViewInterface;
class SCPresenterViewSlidesInterface;
class SCPresenterViewToolWidget;

class SCPresenterViewWidget : public QWidget
{
    Q_OBJECT
public:
    SCPresenterViewWidget(SCViewModePresentation *viewMode, const QList<KoPAPage *> &pages, KoPACanvas *canvas, QWidget *parent=0);
    ~SCPresenterViewWidget();
    void setActivePage(KoPAPage *page);
    void setActivePage(int pageIndex);
    void updateWidget(const QSize &widgetSize, const QSize &canvasSize);

private slots:
    void showSlideThumbnails(bool show);
    void requestPreviousSlide();
    void requestNextSlide();
    void requestChangePage(int index, bool enableMainView);

private:
    SCViewModePresentation *m_viewMode;
    QList<KoPAPage *> m_pages;
    KoPACanvas *m_canvas;

    QStackedLayout *m_stackedLayout;

    SCPresenterViewInterface *m_mainWidget;
    SCPresenterViewSlidesInterface *m_slidesWidget;
    SCPresenterViewBaseInterface *m_activeWidget;
    SCPresenterViewToolWidget *m_toolWidget;
};

#endif

