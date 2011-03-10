/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "SCPageEffectRunner.h"
#ifdef HAVE_OPENGL
#include <QGLFormat>
#endif
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include "SCPageEffect.h"

SCPageEffectRunner::SCPageEffectRunner(const QPixmap &oldPage, const QPixmap &newPage, QWidget *w, SCPageEffect *effect)
: m_effect(effect)
, m_data(oldPage, newPage, w)
{
    if (m_effect->useGraphicsView()) {
        m_data.m_scene = new QGraphicsScene();
        m_data.m_graphicsView = new QGraphicsView(m_data.m_scene, m_data.m_widget);
        m_data.m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_data.m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_data.m_graphicsView->resize(m_data.m_widget->size());
        m_data.m_graphicsView->setFrameShape(QFrame::Panel);
        m_data.m_graphicsView->setLineWidth(0);

        m_data.m_oldPageItem = new QGraphicsPixmapItem(m_data.m_oldPage, 0, m_data.m_scene);
        m_data.m_newPageItem = new QGraphicsPixmapItem(m_data.m_newPage, 0, m_data.m_scene);
        m_data.m_oldPageItem->hide();
        m_data.m_newPageItem->hide();

        // set a black background
        m_data.m_graphicsView->setBackgroundBrush(Qt::black);

        m_data.m_graphicsView->show();

        // Do some optimization
        m_data.m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_data.m_graphicsView->setOptimizationFlag(QGraphicsView::DontClipPainter, true);
        m_data.m_graphicsView->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
        m_data.m_graphicsView->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
        m_data.m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

        // If computer support openGL
#ifdef HAVE_OPENGL
        if (QGLFormat::hasOpenGL()) {
            m_data.m_graphicsView->setRenderHints(QPainter::SmoothPixmapTransform);
            m_data.m_graphicsView->setRenderHints(QPainter::Antialiasing);
            m_data.m_graphicsView->setRenderHints(QPainter::HighQualityAntialiasing);
            QGLWidget *openGLWidget = new QGLWidget();
            openGLWidget->format().setSwapInterval(0);
            m_data.m_graphicsView->setViewport(openGLWidget);
        }
#endif
    }
    m_effect->setup(m_data, m_data.m_timeLine);
}

SCPageEffectRunner::~SCPageEffectRunner()
{
    delete m_data.m_graphicsView;
    delete m_data.m_scene;
}

bool SCPageEffectRunner::paint(QPainter &painter)
{
    return m_effect->paint(painter, m_data);
}

void SCPageEffectRunner::next(int currentTime)
{
    m_data.m_lastTime = m_data.m_currentTime;
    m_data.m_currentTime = currentTime;

    m_effect->next(m_data);
}

void SCPageEffectRunner::finish()
{
    m_data.m_finished = true;
    m_effect->finish(m_data);
}

bool SCPageEffectRunner::isFinished()
{
    return m_data.m_finished;
}

const QPixmap & SCPageEffectRunner::oldPage() const
{
    return m_data.m_oldPage;
}

const QPixmap & SCPageEffectRunner::newPage() const
{
    return m_data.m_newPage;
}

void SCPageEffectRunner::setOldPage(const QPixmap & oldPage)
{
    m_data.m_oldPage = oldPage;
}

void SCPageEffectRunner::setNewPage(const QPixmap & newPage)
{
    m_data.m_newPage = newPage;
}


