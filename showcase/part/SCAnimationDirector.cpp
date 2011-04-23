/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
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

#include "SCAnimationDirector.h"
#include "SCShapeAnimations.h"
#include "animations/SCAnimationCache.h"
#include "animations/SCAnimationStep.h"
#include "SCEndOfSlideShowPage.h"
#include "SCPage.h"
#include "SCMasterPage.h"
#include "SCPageApplicationData.h"
#include "SCShapeManagerAnimationStrategy.h"
#include "SCShapeManagerDisplayMasterStrategy.h"
#include "SCPageSelectStrategyActive.h"
#include "pageeffects/SCPageEffectRunner.h"
#include "pageeffects/SCPageEffect.h"

#include <KoShapeLayer.h>
#include <KoPAMasterPage.h>
#include <KoSelection.h>

#include <QList>
#include <QPainter>
#include <QPaintEvent>
#include <QWidget>
#include <QVariant>
#include <kapplication.h>
#include <kdebug.h>
#include <KoPageLayout.h>
#include <KoShapeManager.h>
#include <KoShapeManagerPaintingStrategy.h>
#include <KoViewConverter.h>
#include <KoPAViewMode.h>
#include <KoPACanvas.h>
#include <KoPAPageBase.h>
#include <KoPAView.h>
#include <KoPAUtil.h>

SCAnimationDirector::SCAnimationDirector(KoPAView * view, KoPACanvas * canvas, const QList<KoPAPageBase*> &pages, KoPAPageBase* currentPage)
: m_view(view)
, m_canvas(canvas)
, m_pages(pages)
, m_pageEffectRunner(0)
, m_stepIndex(0)
, m_maxShapeDuration(0)
, m_hasAnimation(false)
, m_animationCache(0)
{
    Q_ASSERT(!m_pages.empty());
    m_animationCache = new SCAnimationCache();
    if(!currentPage || !pages.contains(currentPage))
        updateActivePage(m_pages[0]);
    else
        updateActivePage(currentPage);

    m_pageIndex = m_pages.indexOf(m_view->activePage());

    // updatePageAnimation was called from updateZoom() [updateActivePage()]

    connect(&m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animate()));
    // this is needed as after a call to m_canvas->showFullScreen the canvas is not made fullscreen right away
    connect(m_canvas, SIGNAL(sizeChanged(const QSize &)), this, SLOT(updateZoom(const QSize &)));
    m_timeLine.setCurveShape(QTimeLine::LinearCurve);
    m_timeLine.setUpdateInterval(20);
    // set the animation strategy in the KoShapeManagers
    m_canvas->shapeManager()->setPaintingStrategy(new SCShapeManagerAnimationStrategy(m_canvas->shapeManager(), m_animationCache,
                                                       new SCPageSelectStrategyActive(m_view->kopaCanvas())));
    m_canvas->masterShapeManager()->setPaintingStrategy(new SCShapeManagerAnimationStrategy(m_canvas->masterShapeManager(), m_animationCache,
                                                             new SCPageSelectStrategyActive(m_view->kopaCanvas())));

    if (hasAnimation()) {
        startTimeLine(m_animations.at(m_stepIndex)->totalDuration());
    }
}

SCAnimationDirector::~SCAnimationDirector()
{
    // free used resources
    delete m_pageEffectRunner;
    delete m_animationCache;
    //set the KoShapeManagerPaintingStrategy in the KoShapeManagers
    m_canvas->shapeManager()->setPaintingStrategy(new KoShapeManagerPaintingStrategy(m_canvas->shapeManager()));
    m_canvas->masterShapeManager()->setPaintingStrategy(new SCShapeManagerDisplayMasterStrategy(m_canvas->masterShapeManager(),
                                                             new SCPageSelectStrategyActive(m_view->kopaCanvas())));
}



void SCAnimationDirector::paint(QPainter &painter, const QRectF &paintRect)
{
    if (m_pageEffectRunner)
    {
        bool finished = m_pageEffectRunner->isFinished();
        if (!m_pageEffectRunner->paint(painter))
        {
            delete m_pageEffectRunner;
            m_pageEffectRunner = 0;

            // check if there where a animation to start
            if (hasAnimation()) {
                if (finished) {
                    QRect clipRect = m_pageRect.intersected(paintRect.toRect());
                    painter.setClipRect(clipRect);
                    painter.setRenderHint(QPainter::Antialiasing);
                    paintStep(painter);
                }
                else {
                    // start the animations
                    startTimeLine(m_animations.at(m_stepIndex)->totalDuration());
                }
            }
        }
    }
    else {
        QRect clipRect = m_pageRect.intersected(paintRect.toRect());
        painter.setClipRect(clipRect);
        painter.setRenderHint(QPainter::Antialiasing);
        paintStep(painter);
    }
    // This is needed as otherwise on some ATI graphic cards it leads to
    // 100% CPU load of the x server and no more key events are received
    // until the page effect is finished. With it is made sure that key
    // events still get through so that it is possible to cancel the
    // events. It looks like this is not a problem with nvidia graphic
    // cards.
    KApplication::kApplication()->syncX();
}

void SCAnimationDirector::paintEvent(QPaintEvent* event)
{
    QPainter painter(m_canvas);
    paint(painter, event->rect());
}

KoViewConverter * SCAnimationDirector::viewConverter()
{
    return &m_zoomHandler;
}

int SCAnimationDirector::numPages() const
{
    return m_pages.size();
}

int SCAnimationDirector::currentPage() const
{
    return m_pageIndex;
}

int SCAnimationDirector::numStepsInPage() const
{
    return m_animations.size();
}

int SCAnimationDirector::currentStep() const
{
    return m_stepIndex;
}

bool SCAnimationDirector::navigate(Navigation navigation)
{
    bool finished = false;
    if (m_pageEffectRunner) {
        m_pageEffectRunner->finish();
        finishAnimations();
        // finish on first step
        m_timeLine.stop();
        finished = true;
    }
    else if (m_timeLine.state() == QTimeLine::Running) { // there are still shape animations running
        finishAnimations();
        m_timeLine.stop();
        finished = true;
    }

    bool presentationFinished = false;

    switch (navigation)
    {
        case FirstPage:
        case PreviousPage:
        case NextPage:
        case LastPage:
            presentationFinished = changePage(navigation);
            break;
        case PreviousStep:
            previousStep();
            break;
        case NextStep:
            if (!finished) {
                presentationFinished = nextStep();
            }
            break;
        default:
            break;
    }

    return presentationFinished;
}

void SCAnimationDirector::navigateToPage(int index)
{
    if (m_pageEffectRunner) {
        m_pageEffectRunner->finish();
        finishAnimations();
        // finish on first step
        m_timeLine.stop();
    }
    else if (m_timeLine.state() == QTimeLine::Running) { // there are still shape animations running
        finishAnimations();
        m_timeLine.stop();
    }

    m_pageIndex = index;
    KoPAPageBase *page = m_pages[m_pageIndex];

    m_stepIndex = 0;

    updateActivePage(page);
    updatePageAnimation();
    updateStepAnimation();
    // trigger a repaint
    m_canvas->update();

    if (hasAnimation()) {
        startTimeLine(m_animations.at(m_stepIndex)->totalDuration());
    }
}

void SCAnimationDirector::updateActivePage(KoPAPageBase * page)
{
    deactivate();

    if (m_canvas == m_view->kopaCanvas()) {
        m_view->viewMode()->updateActivePage(page);
    }
    else {
        QList<KoShape*> shapes = page->shapes();
        m_canvas->shapeManager()->setShapes(shapes, KoShapeManager::AddWithoutRepaint);
        //Make the top most layer active
        if (!shapes.isEmpty()) {
            KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>(shapes.last());
            m_canvas->shapeManager()->selection()->setActiveLayer(layer);
        }

        // if the page is not a master page itself set shapes of the master page
        KoPAPage * paPage = dynamic_cast<KoPAPage *>(page);

        Q_ASSERT(paPage);
        KoPAMasterPage * masterPage = paPage->masterPage();
        QList<KoShape*> masterShapes = masterPage->shapes();
        m_canvas->masterShapeManager()->setShapes(masterShapes, KoShapeManager::AddWithoutRepaint);
        // Make the top most layer active
        if (!masterShapes.isEmpty()) {
            KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>(masterShapes.last());
            m_canvas->masterShapeManager()->selection()->setActiveLayer(layer);
        }
    }

    SCPage * kprPage = dynamic_cast<SCPage *>(page);
    Q_ASSERT(kprPage);
    m_pageIndex = m_pages.indexOf(page);
    m_animations = kprPage->animations().steps();

    // it can be that the pages have different sizes. So we need to recalulate
    // the zoom when we change the page
    updateZoom(m_canvas->size());
}

void SCAnimationDirector::updatePageAnimation()
{
    m_animationCache->clear();

    m_animationCache->setPageSize(m_pages[m_pageIndex]->size());
    qreal zoom;
    m_zoomHandler.zoom(&zoom, &zoom);
    m_animationCache->setZoom(zoom);
    int i = 0;
    foreach (SCAnimationStep *step, m_animations) {
        step->init(m_animationCache, i);
        i++;
    }
}

void SCAnimationDirector::updateStepAnimation()
{
    m_animationCache->startStep(m_stepIndex);
}


bool SCAnimationDirector::changePage(Navigation navigation)
{
    switch (navigation)
    {
        case FirstPage:
            m_pageIndex = 0;
            break;
        case PreviousPage:
            m_pageIndex = m_pageIndex > 0 ? m_pageIndex - 1 : 0;
            break;
        case NextPage:
            if (m_pageIndex < m_pages.size() -1) {
                ++m_pageIndex;
            }
            else {
                return true;
            }
            break;
        case LastPage:
            m_pageIndex = m_pages.size() - 1;
            if (dynamic_cast<SCEndOfSlideShowPage *>(m_pages[m_pageIndex]) && m_pageIndex > 0) {
                m_pageIndex--;
            }
            break;
        case PreviousStep:
        case NextStep:
        default:
            // this should not happen
            Q_ASSERT(0);
            break;
    }
    m_stepIndex = 0;

    updateActivePage(m_pages[m_pageIndex]);
    updatePageAnimation();
    updateStepAnimation();

    // trigger a repaint
    m_canvas->update();

    if (hasAnimation()) {
        startTimeLine(m_animations.at(m_stepIndex)->totalDuration());
    }

    return false;
}

void SCAnimationDirector::updateZoom(const QSize &size)
{
    KoPageLayout pageLayout = m_view->activePage()->pageLayout();
    KoPAUtil::setZoom(pageLayout, size, m_zoomHandler);
    m_pageRect = KoPAUtil::pageRect(pageLayout, size, m_zoomHandler);
    m_canvas->setDocumentOffset(-m_pageRect.topLeft());

    // reinit page animation, because somi init method contain zoom
    updatePageAnimation();
    updateStepAnimation();
}

void SCAnimationDirector::paintStep(QPainter &painter)
{
    painter.translate(m_pageRect.topLeft());
    m_view->activePage()->paintBackground(painter, m_zoomHandler);

    if (m_view->activePage()->displayMasterShapes()) {
        foreach (KoShape *shape, m_canvas->masterShapeManager()->shapes()) {
            shape->waitUntilReady(m_zoomHandler, false);
        }

        m_canvas->masterShapeManager()->paint(painter, m_zoomHandler, true);
    }
    foreach (KoShape *shape, m_canvas->shapeManager()->shapes()) {
        shape->waitUntilReady(m_zoomHandler, false);
    }
    m_canvas->shapeManager()->paint(painter, m_zoomHandler, true);
}

bool SCAnimationDirector::nextStep()
{
    if (m_stepIndex < numStepsInPage() - 1) {
        // if there are sub steps go to the next substep
        ++m_stepIndex;
        updateStepAnimation();
        startTimeLine(m_animations.at(m_stepIndex)->totalDuration());
    }
    else {
        // if there are no more sub steps go to the next page
        // The active page and the substeps are updated later as
        // first the current page has to be painted again for the page effect
        if (m_pageIndex < m_pages.size() -1) {
            ++m_pageIndex;
        }
        else {
            return true;
        }
        m_stepIndex = 0;

        SCPageEffect * effect = SCPage::pageData(m_pages[m_pageIndex])->pageEffect();

        // run page effect if there is one
        if (effect) {
            QPixmap oldPage(m_canvas->size());
            m_canvas->render(&oldPage);

            updateActivePage(m_pages[m_pageIndex]);
            updatePageAnimation();
            updateStepAnimation();
            QPixmap newPage(m_canvas->size());
            newPage.fill(Qt::white); // TODO
            QPainter newPainter(&newPage);
            newPainter.setClipRect(m_pageRect);
            newPainter.setRenderHint(QPainter::Antialiasing);
            paintStep(newPainter);

            m_pageEffectRunner = new SCPageEffectRunner(oldPage, newPage, m_canvas, effect);
            startTimeLine(effect->duration());
        }
        else {
            updateActivePage(m_pages[m_pageIndex]);
            updatePageAnimation();
            updateStepAnimation();
            m_canvas->update();
            if (hasAnimation()) {
                startTimeLine(m_animations.at(m_stepIndex)->totalDuration());
            }
        }
    }
    return false;
}

void SCAnimationDirector::previousStep()
{
    if (m_stepIndex > 0) {
        --m_stepIndex;
    }
    else {
        if (m_pageIndex > 0) {
            --m_pageIndex;
            updateActivePage(m_pages[m_pageIndex]);
            if(hasAnimation()) {
                m_stepIndex = m_animations.size() - 1;
            }
            else {
                m_stepIndex = m_animations.size();
            }
            updatePageAnimation();
            // trigger repaint
            m_canvas->update();
            // cancel a running page effect
            delete m_pageEffectRunner;
            m_pageEffectRunner = 0;
        }
    }
    // when going back you allway go to the end of the effect
    finishAnimations();
}


bool SCAnimationDirector::hasAnimation()
{
    return m_animations.size() > 0;
}

void SCAnimationDirector::animate()
{
    if (m_pageEffectRunner) {
        m_pageEffectRunner->next(m_timeLine.currentTime());
    }
    else if (hasAnimation()) { //if there are animnations
        // set current time, to the current step
        m_animationCache->next();
        m_animations.at(m_stepIndex)->setCurrentTime(m_timeLine.currentTime());
        m_canvas->update();
    }
}

void SCAnimationDirector::finishAnimations()
{
    m_animationCache->endStep(m_stepIndex);
    m_canvas->update();
}

void SCAnimationDirector::startTimeLine(int duration)
{
    if (duration == 0) {
        m_timeLine.setDuration(1);
    }
    else {
       m_timeLine.setDuration(duration);
    }
    m_timeLine.setCurrentTime(0);
    m_timeLine.start();
}

void SCAnimationDirector::deactivate()
{
    foreach (SCAnimationStep *step, m_animations) {
        step->deactivate();
    }
}

#include "SCAnimationDirector.moc"
