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
#include <KoRuler.h>
#include <KoToolBoxFactory.h>
#include <KoToolDockerFactory.h>

#include <QGridLayout>
#include <QVariant>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextLayout>

KWGui::KWGui( const QString& viewMode, KWView *parent )
  : QWidget( parent),
    m_view(parent)
{
    QGridLayout *gridLayout = new QGridLayout( this );
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);

    // Ruler
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, m_view->viewConverter());
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_view->kwdocument()->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, m_view->viewConverter());
    m_verticalRuler->setUnit(m_view->kwdocument()->unit());
    m_verticalRuler->setShowMousePosition(true);

    m_canvas = new KWCanvas( viewMode, static_cast<KWDocument*>(m_view->koDocument()), m_view, this );
    m_canvasController = new KoCanvasController(this);
    m_canvasController->setCanvas(m_canvas);
    m_canvasController->setCanvasMode( KoCanvasController::AlignTop );
    KoToolManager::instance()->addController(m_canvasController);
    KoToolManager::instance()->registerTools(m_view->actionCollection(), m_canvasController);

    KoToolBoxFactory toolBoxFactory(m_canvasController, "KWord");
    m_view->createDockWidget( &toolBoxFactory );

    KoToolDockerFactory toolDockerFactory;
    KoToolDocker *td =  dynamic_cast<KoToolDocker*>( m_view->createDockWidget( &toolDockerFactory ) );
    connect(m_canvasController, SIGNAL(toolOptionWidgetChanged(QWidget*)), td, SLOT(newOptionWidget(QWidget*)));

    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget( m_canvasController, 1, 1 );

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
    connect(m_canvas->resourceProvider(), SIGNAL(sigResourceChanged(int, const QVariant &)),
        this, SLOT(canvasResourceChanged(int)));

    pageSetupChanged();
}
KWGui::~KWGui() {
    KoToolManager::instance()->removeCanvasController(m_canvasController);
}

int KWGui::visibleWidth() const {
    return m_canvasController->visibleWidth();
}

int KWGui::visibleHeight() const {
    return m_canvasController->visibleHeight();
}

QSize KWGui::viewportSize() const {
    return m_canvasController->viewport()->size();
}


bool KWGui::horizontalScrollBarVisible() {
    return m_canvasController->horizontalScrollBar() &&
        m_canvasController->horizontalScrollBar()->isVisible();
}

void KWGui::pageSetupChanged() {
    const KWPageManager *pm = m_view->kwdocument()->pageManager();
    m_verticalRuler->setRulerLength( pm->bottomOfPage(pm->lastPageNumber()) );
    updateRulers();
    KWPage *firstPage = pm->page(pm->startPage());
    if(firstPage)
        m_horizontalRuler->setRulerLength( firstPage->width() );
    updateRulers();
}

void KWGui::updateMousePos(const QPoint &point) {
    m_horizontalRuler->updateMouseCoordinate(point.x());
    m_verticalRuler->updateMouseCoordinate(point.y());
}

void KWGui::updateRulers() const {
    m_verticalRuler->setVisible( m_view->kwdocument()->config().viewRulers() );
    m_horizontalRuler->setVisible( m_view->kwdocument()->config().viewRulers() );
}

void KWGui::shapeSelectionChanged() {
    QPointF start, end;
    bool first = true;
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection)) {
        QRectF rect = shape->boundingRect();
        if(first) {
            start = rect.topLeft();
            end = rect.bottomRight();
            first = false;
        }
        else {
            start.setX(qMin(start.x(), rect.left()));
            start.setY(qMin(start.y(), rect.top()));
            end.setX(qMax(end.x(), rect.right()));
            end.setY(qMax(end.y(), rect.bottom()));
        }
    }
    m_horizontalRuler->setActiveRange(start.x(), end.x());
    m_verticalRuler->setActiveRange(start.y(), end.y());
}

void KWGui::canvasResourceChanged(int key) {
    if(key != KoText::CurrentTextPosition && key != KoText::CurrentTextDocument)
       return;

    class Setter {
    public:
        Setter(KoRuler *ruler) : m_ruler(ruler) {}
        ~Setter() {
            if(m_block.isValid()) {
                QTextBlockFormat format = m_block.blockFormat();
                m_ruler->setShowIndents(true);
                m_ruler->setParagraphIndent(format.leftMargin());
                m_ruler->setFirstLineIndent(format.textIndent());
                m_ruler->setEndIndent(format.rightMargin());
                m_ruler->setRightToLeft(m_block.layout()->textOption().textDirection() == Qt::RightToLeft);
            }
            else
                m_ruler->setShowIndents(false);
        }
        void setBlock(const QTextBlock &block) {
            m_block = block;
        }

    private:
        QTextBlock m_block;
        KoRuler *m_ruler;
    };
    Setter setter(m_horizontalRuler);

    QVariant docVar = m_canvas->resourceProvider()->resource(KoText::CurrentTextDocument);
    if(docVar.isNull())
        return;
    QTextDocument *doc = static_cast<QTextDocument*> (docVar.value<void*>());
    if(doc == 0)
        return;
    setter.setBlock(doc->findBlock(m_canvas->resourceProvider()->intResource(KoText::CurrentTextPosition)));

}


#include "KWGui.moc"
