/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Jim Courtiau <jeremy.courtiau@gmail.com>
 * Copyright (C) 2009 Alexia Allanic <alexia_allanic@yahoo.fr>
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * Copyright (C) 2009 Jérémy Lugagne <jejewindsurf@hotmail.com>
 * Copyright (C) 2009 Johann Hingue <yoan1703@hotmail.fr>
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

#include "SCPresentationTool.h"

#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPainter>
#include <QKeyEvent>
#include <qabstracttextdocumentlayout.h>
#include <qcursor.h>
#include <qdesktopservices.h>
#include <qurl.h>

#include <KShape.h>
#include <KShapeManager.h>
#include <KPointerEvent.h>
#include <KEventAction.h>
#include <KoPACanvas.h>
#include <KTextShapeData.h>

#include "SCViewModePresentation.h"
#include "SCPresentationStrategy.h"
#include "SCPresentationHighlightStrategy.h"
#include "SCPresentationDrawStrategy.h"
#include "SCPresentationBlackStrategy.h"
#include "ui/SCPresentationToolWidget.h"
#include "SCPresentationToolAdaptor.h"
#include "SCViewModePresentation.h"


SCPresentationTool::SCPresentationTool(SCViewModePresentation &viewMode)
: KToolBase(viewMode.canvas())
, m_viewMode(viewMode)
, m_strategy(new SCPresentationStrategy(this))
, m_bus (new SCPresentationToolAdaptor(this))
{
    QDBusConnection::sessionBus().registerObject("/showcase/PresentationTools", this);

    // tool box
    m_frame = new QFrame(m_viewMode.canvas()->canvasWidget());

    QVBoxLayout *frameLayout = new QVBoxLayout();

    m_presentationToolWidget = new SCPresentationToolWidget(m_viewMode.canvas()->canvasWidget());
    frameLayout->addWidget(m_presentationToolWidget, 0, Qt::AlignLeft | Qt::AlignBottom);
    m_frame->setLayout(frameLayout);
    m_frame->show();

    m_presentationToolWidget->raise();
    m_presentationToolWidget->setVisible(false);
    m_presentationToolWidget->installEventFilter(this);

    // Connections of button clicked to slots
    connect(m_presentationToolWidget->presentationToolUi().penButton, SIGNAL(clicked()), this, SLOT(drawOnPresentation()));
    connect(m_presentationToolWidget->presentationToolUi().highLightButton, SIGNAL(clicked()), this, SLOT(highlightPresentation()));
    connect(m_presentationToolWidget->presentationToolUi().blackButton, SIGNAL(clicked()), this, SLOT(blackPresentation()));

}

SCPresentationTool::~SCPresentationTool()
{
    delete m_strategy;
}

bool SCPresentationTool::wantsAutoScroll() const
{
    return false;
}

void SCPresentationTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void SCPresentationTool::mousePressEvent(KPointerEvent *event)
{
    if (event->button() & Qt::LeftButton) {
        event->accept();
        finishEventActions();
        KShape * shapeClicked = canvas()->shapeManager()->shapeAt(event->point);
        if (shapeClicked) {
            QString link;
            if (checkHyperlink(event, shapeClicked, link)) {
                runHyperlink(link);
                return;
            }

            m_eventActions = shapeClicked->eventActions();
            if (!m_eventActions.isEmpty()) {
                foreach (KEventAction * eventAction, m_eventActions) {
                    eventAction->start();
                }
                // don't do next step if a action was executed
                return;
            }
        }
        m_viewMode.navigate(SCAnimationDirector::NextStep);
    }
    else if (event->button() & Qt::RightButton) {
        event->accept();
        finishEventActions();
        m_viewMode.navigate(SCAnimationDirector::PreviousStep);
    }
}

void SCPresentationTool::mouseDoubleClickEvent(KPointerEvent *event)
{
    Q_UNUSED(event);
}

void SCPresentationTool::mouseMoveEvent(KPointerEvent *event)
{
    KShape * shape = canvas()->shapeManager()->shapeAt(event->point);

    QString link;
    if (checkHyperlink(event, shape, link)) {
        setCursor(Qt::PointingHandCursor);
        return;
    }

    setCursor(Qt::ArrowCursor);
}

void SCPresentationTool::mouseReleaseEvent(KPointerEvent *event)
{
    Q_UNUSED(event);
}

void SCPresentationTool::keyPressEvent(QKeyEvent *event)
{
    finishEventActions();
    // first try to handle the event in the strategy if it is done there no need to use the default action
    if (! m_strategy->keyPressEvent(event)) {
        switch (event->key())
        {
            case Qt::Key_Escape:
                m_viewMode.activateSavedViewMode();
                break;
            case Qt::Key_Home:
                m_viewMode.navigate(SCAnimationDirector::FirstPage);
                break;
            case Qt::Key_Up:
            case Qt::Key_PageUp:
                m_viewMode.navigate(SCAnimationDirector::PreviousPage);
                break;
            case Qt::Key_Backspace:
            case Qt::Key_Left:
                m_viewMode.navigate(SCAnimationDirector::PreviousStep);
                break;
            case Qt::Key_Right:
            case Qt::Key_Space:
                m_viewMode.navigate(SCAnimationDirector::NextStep);
                break;
            case Qt::Key_Down:
            case Qt::Key_PageDown:
                m_viewMode.navigate(SCAnimationDirector::NextPage);
                break;
            case Qt::Key_End:
                m_viewMode.navigate(SCAnimationDirector::LastPage);
                break;
            default:
                event->ignore();
                break;
        }
    }
}

void SCPresentationTool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void SCPresentationTool::wheelEvent(KPointerEvent * event)
{
    Q_UNUSED(event);
}

void SCPresentationTool::activate(ToolActivation toolActivation, const QSet<KShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    Q_UNUSED(shapes);
    m_frame->setGeometry(canvas()->canvasWidget()->geometry());
    m_presentationToolWidget->setVisible(false);
    // redirect event to tool widget
    m_frame->installEventFilter(this);
    // activate tracking for show/hide tool buttons
    m_frame->setMouseTracking(true);
}

void SCPresentationTool::deactivate()
{
    switchStrategy(new SCPresentationStrategy(this));
    finishEventActions();
}

void SCPresentationTool::finishEventActions()
{
    foreach (KEventAction * eventAction, m_eventActions) {
        eventAction->finish();
    }
}

void SCPresentationTool::switchStrategy(SCPresentationStrategyBase * strategy)
{
    Q_ASSERT(strategy);
    Q_ASSERT(m_strategy != strategy);
    delete m_strategy;
    m_strategy = strategy;
}

// SLOTS
void SCPresentationTool::highlightPresentation()
{
    SCPresentationStrategyBase * strategy;
    if (dynamic_cast<SCPresentationHighlightStrategy *>(m_strategy)) {
        strategy = new SCPresentationStrategy(this);
    }
    else {
        strategy = new SCPresentationHighlightStrategy(this);
    }
    switchStrategy(strategy);
}

void SCPresentationTool::drawOnPresentation()
{
    SCPresentationStrategyBase * strategy;
    if (dynamic_cast<SCPresentationDrawStrategy*>(m_strategy)) {
        strategy = new SCPresentationStrategy(this);
    }
    else {
        strategy = new SCPresentationDrawStrategy(this);
    }
    switchStrategy(strategy);
}

void SCPresentationTool::blackPresentation()
{
    SCPresentationStrategyBase * strategy;
    if (dynamic_cast<SCPresentationBlackStrategy*>(m_strategy)) {
        strategy = new SCPresentationStrategy(this);
    }
    else {
        strategy = new SCPresentationBlackStrategy(this);
    }
    switchStrategy(strategy);
}

bool SCPresentationTool::eventFilter(QObject *obj, QEvent * event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QWidget *source = static_cast<QWidget*>(obj);
        QPoint pos = source->mapFrom(m_viewMode.canvas()->canvasWidget(), mouseEvent->pos());

        QSize buttonSize = m_presentationToolWidget->size() + QSize(20, 20);
        QRect geometry = QRect(0, m_frame->height() - buttonSize.height(), buttonSize.width(), buttonSize.height());
        if (geometry.contains(pos)) {
            m_presentationToolWidget->setVisible(true);
        }
        else {
            m_presentationToolWidget->setVisible(false);
        }
    }
    return false;
}

bool SCPresentationTool::checkHyperlink(KPointerEvent *event, KShape *shape, QString &hyperLink)
{
    if (!shape) {
        return false;
    }

    KTextShapeData *textShapeData = qobject_cast<KTextShapeData*>(shape->userData());
    if (textShapeData) {
        QPointF p = shape->absoluteTransformation(0).inverted().map(event->point);
        p = p + QPointF(0.0, textShapeData->documentOffset());

        int caretPos = textShapeData->document()->documentLayout()->hitTest(p, Qt::ExactHit);

        if (textShapeData->endPosition() != -1 && caretPos != -1) {
            QTextCursor mouseOver(textShapeData->document());
            mouseOver.setPosition(caretPos);

            QTextCharFormat fmt = mouseOver.charFormat();
            hyperLink = fmt.anchorHref();
            if (!hyperLink.isEmpty()) {
                return true;
            }
        }
    }
    return false;
}

void SCPresentationTool::runHyperlink(QString hyperLink)
{
    QUrl url = QUrl::fromUserInput(hyperLink);

    QDesktopServices::openUrl(url);
}

SCPresentationStrategyBase *SCPresentationTool::strategy()
{
    return m_strategy;
}

SCViewModePresentation &SCPresentationTool::viewModePresentation()
{
    return m_viewMode;
}


void SCPresentationTool::normalPresentation()
{
   switchStrategy(new SCPresentationStrategy(this));
}

#include "SCPresentationTool.moc"
