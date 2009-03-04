/* This file is part of the KDE project
 * Copyright (C) 2001 David Faure <faure@kde.org>
 * Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
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
 * Boston, MA 02110-1301, USA
 */

#include "KWGui.h"
#include "KWView.h"
#include "KWDocument.h"
#include "KWCanvas.h"
#include "KWPage.h"

#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoFlake.h>
#include <KoSelection.h>
#include <KoToolDocker.h>
#include <KoDockerManager.h>
#include <KoRuler.h>
#include <KoToolBoxFactory.h>
#include <KoRulerController.h>
#include <KActionCollection>

#include <QGridLayout>
#include <QTimer>
#include <QAction>
#include <QScrollBar>

KWGui::KWGui(const QString& viewMode, KWView *parent)
        : QWidget(parent),
        m_view(parent)
{
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);

    // Ruler
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, m_view->viewConverter());
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_view->kwdocument()->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, m_view->viewConverter());
    m_verticalRuler->setUnit(m_view->kwdocument()->unit());
    m_verticalRuler->setShowMousePosition(true);

    m_canvas = new KWCanvas(viewMode, static_cast<KWDocument*>(m_view->koDocument()), m_view, this);
    m_canvasController = new KoCanvasController(this);
    m_canvasController->setMargin(10);
    m_canvasController->setCanvas(m_canvas);
    m_canvasController->setCanvasMode(KoCanvasController::AlignTop);
    KoToolManager::instance()->addController(m_canvasController);
    KoToolManager::instance()->registerTools(m_view->actionCollection(), m_canvasController);

    KoToolBoxFactory toolBoxFactory(m_canvasController, "KWord");
    m_view->createDockWidget(&toolBoxFactory);

    KoDockerManager *dockerMng = m_view->dockerManager();
    if (!dockerMng) {
        dockerMng = new KoDockerManager(m_view);
        m_view->setDockerManager(dockerMng);
    }
    connect(m_canvasController, SIGNAL(toolOptionWidgetsChanged(const QMap<QString, QWidget *> &, KoView *)),
            dockerMng, SLOT(newOptionWidgets(const  QMap<QString, QWidget *> &, KoView *)));

    gridLayout->addWidget(m_horizontalRuler->tabChooser(), 0, 0);
    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget(m_canvasController, 1, 1);

    new KoRulerController(m_horizontalRuler, m_canvas->resourceProvider());

    connect(m_view->kwdocument(), SIGNAL(unitChanged(KoUnit)), m_horizontalRuler, SLOT(setUnit(KoUnit)));
    connect(m_view->kwdocument(), SIGNAL(unitChanged(KoUnit)), m_verticalRuler, SLOT(setUnit(KoUnit)));
    connect(m_view->kwdocument(), SIGNAL(pageSetupChanged()), this, SLOT(pageSetupChanged()));

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)), m_horizontalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)), m_verticalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(canvasMousePositionChanged(const QPoint &)),
            this, SLOT(updateMousePos(const QPoint&)));

    connect(m_canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            m_canvas, SLOT(setDocumentOffset(const QPoint&)));

    connect(m_canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(shapeSelectionChanged()));

    pageSetupChanged();

    QTimer::singleShot(0, this, SLOT(setupUnitActions()));
}

KWGui::~KWGui()
{
    KoToolManager::instance()->removeCanvasController(m_canvasController);
}

int KWGui::visibleWidth() const
{
    return m_canvasController->visibleWidth();
}

int KWGui::visibleHeight() const
{
    return m_canvasController->visibleHeight();
}

QSize KWGui::viewportSize() const
{
    return m_canvasController->viewport()->size();
}


bool KWGui::horizontalScrollBarVisible()
{
    return m_canvasController->horizontalScrollBar() &&
           m_canvasController->horizontalScrollBar()->isVisible();
}

void KWGui::pageSetupChanged()
{
    const KWPageManager *pm = m_view->kwdocument()->pageManager();
    const KWPage firstPage = pm->begin();
    const KWPage lastPage = pm->last();
    int height = 0;
    if (lastPage.isValid())
        height = lastPage.offsetInDocument() + lastPage.height();
    m_verticalRuler->setRulerLength(height);
    updateRulers();
    int width = 0;
    if (firstPage.isValid())
        width = firstPage.width();
    m_horizontalRuler->setRulerLength(width);
    updateRulers();
}

void KWGui::updateMousePos(const QPoint &point)
{
    QPoint canvasOffset(m_canvasController->canvasOffsetX(), m_canvasController->canvasOffsetY() );
    // the offset is positive it the canvas is shown fully visible
    canvasOffset.setX(canvasOffset.x() < 0 ? canvasOffset.x(): 0);
    canvasOffset.setY(canvasOffset.y() < 0 ? canvasOffset.y(): 0);
    QPoint viewPos = point - canvasOffset;
    m_horizontalRuler->updateMouseCoordinate(viewPos.x());
    m_verticalRuler->updateMouseCoordinate(viewPos.y());
}

void KWGui::updateRulers() const
{
    m_verticalRuler->setVisible(m_view->kwdocument()->config().viewRulers());
    m_horizontalRuler->setVisible(m_view->kwdocument()->config().viewRulers());
}

void KWGui::shapeSelectionChanged()
{
    QPointF start, end;
    bool first = true;
    foreach (KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection)) {
        QRectF rect = shape->boundingRect();
        if (first) {
            start = rect.topLeft();
            end = rect.bottomRight();
            first = false;
        } else {
            start.setX(qMin(start.x(), rect.left()));
            start.setY(qMin(start.y(), rect.top()));
            end.setX(qMax(end.x(), rect.right()));
            end.setY(qMax(end.y(), rect.bottom()));
        }
    }
    m_horizontalRuler->setActiveRange(start.x(), end.x());
    m_verticalRuler->setActiveRange(start.y(), end.y());
}

void KWGui::setupUnitActions()
{
    QList<QAction*> unitActions = m_view->createChangeUnitActions();
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    unitActions.append(separator);
    unitActions.append(m_view->actionCollection()->action("format_page"));
    m_horizontalRuler->setPopupActionList(unitActions);
}

