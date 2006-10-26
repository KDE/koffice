/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeSelector.h"

#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoShapeRegistry.h>
#include <KoToolManager.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoInteractionTool.h>
#include <KoShapeMoveStrategy.h>
#include <KoCreateShapesTool.h>
#include <KoCanvasController.h>

#include <QKeyEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QPointF>
#include <QToolTip>
#include <QTimer>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>

// ******** IconShape *********
/// \internal
class IconShape : public KoShape {
public:
    IconShape(const QString &icon) {
        m_icon = KIcon(icon).pixmap(22);
        resize(m_icon.size());
    }

    virtual void visit(KoCreateShapesTool *tool) = 0;
    virtual QString toolTip() = 0;

    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(converter);
        painter.drawPixmap(QRect( QPoint(0,0), m_icon.size()), m_icon);
    }

private:
    QPixmap m_icon;
};


// ******** TemplateShape *********
/// \internal
class TemplateShape : public IconShape {
public:
    TemplateShape(KoShapeTemplate shapeTemplate) : IconShape(shapeTemplate.icon) {
        m_shapeTemplate = shapeTemplate;
    }

    void visit(KoCreateShapesTool *tool) {
        tool->setShapeId(m_shapeTemplate.id);
        tool->setShapeProperties(m_shapeTemplate.properties);
    }

    QString toolTip() {
        return m_shapeTemplate.toolTip;
    }

private:
    KoShapeTemplate m_shapeTemplate;
};


// ******** GroupShape *********
/// \internal
class GroupShape : public IconShape {
public:
    GroupShape(KoShapeFactory *shapeFactory) : IconShape(shapeFactory->icon()) {
        m_shapeFactory = shapeFactory;
    }

    void visit(KoCreateShapesTool *tool) {
        tool->setShapeId(m_shapeFactory->shapeId());
        tool->setShapeProperties(0);
    }

    QString toolTip() {
        return m_shapeFactory->toolTip();
    }

private:
    KoShapeFactory *m_shapeFactory;
};


// ******* MoveTool *********
/// \internal
class MoveTool : public KoInteractionTool {
public:
    MoveTool(KoCanvasBase *canvas) : KoInteractionTool(canvas) {};
    void mousePressEvent( KoPointerEvent *event ) {
        KoShape *clickedShape = m_canvas->shapeManager()->shapeAt(event->point);
        repaintDecorations();
        m_canvas->shapeManager()->selection()->deselectAll();
        if(clickedShape) {
            m_canvas->shapeManager()->selection()->select(clickedShape);
        }
        m_currentStrategy = new KoShapeMoveStrategy(this, m_canvas, event->point);
    }
    void mouseMoveEvent (KoPointerEvent *event) {
        KoInteractionTool::mouseMoveEvent(event);
    }
};


// ************** KoShapeSelector ************
KoShapeSelector::KoShapeSelector(QWidget *parent, const QString & regExp)
: QDockWidget(i18n("Shapes"), parent)
{
    setObjectName("ShapeSelector");
    m_canvas = new Canvas(this);
    setWidget(m_canvas);
    m_shapeManager = new KoShapeManager(m_canvas);
    setMinimumSize(30, 200);
    setFeatures(DockWidgetMovable|DockWidgetFloatable);

    QTimer::singleShot(0, this, SLOT(loadShapeTypes()));
}

KoShapeSelector::~KoShapeSelector() {
    delete m_shapeManager;
    delete m_canvas;
}

void KoShapeSelector::loadShapeTypes() {
    foreach(KoID id, KoShapeRegistry::instance()->listKeys()) {
        KoShapeFactory *factory = KoShapeRegistry::instance()->get(id);
        bool oneAdded=false;
        foreach(KoShapeTemplate shapeTemplate, factory->templates()) {
            oneAdded=true;
            TemplateShape *shape = new TemplateShape(shapeTemplate);
            add(shape);
        }
        if(!oneAdded)
            add(new GroupShape(factory));
    }

    connect(m_shapeManager->selection(), SIGNAL(selectionChanged()), this, SLOT(itemSelected()));
}

void KoShapeSelector::itemSelected() {
    QList<KoShape*> allSelected = m_shapeManager->selection()->selectedShapes().toList();
    if(allSelected.isEmpty())
        return;
    IconShape *shape= static_cast<IconShape*> (allSelected.first());
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController) {
        KoCreateShapesTool* createTool = KoToolManager::instance()->shapeCreatorTool(canvasController->canvas());
        shape->visit(createTool);
        KoToolManager::instance()->switchToolRequested(KoCreateShapesTool_ID);
    }
}

void KoShapeSelector::add(KoShape *shape) {
    int x=5, y=5; // 5 = gap
    int w = (int) shape->size().width();
    bool ok=true; // lets be optimistic ;)
    do {
        int rowHeight=0;
        ok=true;
        foreach(const KoShape *shape, m_shapeManager->shapes()) {
            if(shape->position().y() > y || shape->position().y() + shape->size().height() < y)
                continue; // other row.
            rowHeight = qMax(rowHeight, qRound(shape->size().height()));
            x = qMax(x, qRound(shape->position().x() + shape->size().width()) + 5); // 5=gap
            if(x + w > width()) { // next row
                y += rowHeight + 5; // 5 = gap
                x = 5;
                ok=false;
                break;
            }
        }
    } while(! ok);
    shape->setPosition(QPointF(x, y));

    m_shapeManager->add(shape);
}


// ************ DummyViewConverter **********
QPointF KoShapeSelector::DummyViewConverter::documentToView (const QPointF &documentPoint) const {
    return documentPoint;
}

QPointF KoShapeSelector::DummyViewConverter::viewToDocument (const QPointF &viewPoint) const {
    return viewPoint;
}

QRectF KoShapeSelector::DummyViewConverter::documentToView (const QRectF &documentRect) const {
    return documentRect;
}

QRectF KoShapeSelector::DummyViewConverter::viewToDocument (const QRectF &viewRect) const {
    return viewRect;
}

void KoShapeSelector::DummyViewConverter::zoom (double *zoomX, double *zoomY) const {
    *zoomX = 1.0;
    *zoomY = 1.0;
}

double KoShapeSelector::DummyViewConverter::documentToViewX (double documentX) const {
    return documentX;
}

double KoShapeSelector::DummyViewConverter::documentToViewY (double documentY) const {
    return documentY;
}

double KoShapeSelector::DummyViewConverter::viewToDocumentX (double viewX) const {
    return viewX;
}

double KoShapeSelector::DummyViewConverter::viewToDocumentY (double viewY) const {
    return viewY;
}


// ********* Canvas **********
KoShapeSelector::Canvas::Canvas(KoShapeSelector *parent)
: QWidget(parent), m_parent(parent)
{
    m_toolProxy = KoToolManager::instance()->toolProxy();
    m_tool = new MoveTool(this);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

void KoShapeSelector::Canvas::gridSize (double *horizontal, double *vertical) const {
    Q_UNUSED(horizontal);
    Q_UNUSED(vertical);
}

void KoShapeSelector::Canvas::updateCanvas (const QRectF &rc) {
    QRect rect = rc.toRect();
    rect.adjust(-2, -2, 2, 2); // grow for to anti-aliasing
    update(rect);
}

void  KoShapeSelector::Canvas::addCommand (KCommand *command, bool execute) {
    if(execute)
        command->execute();
    delete command;
}

// event handlers
void KoShapeSelector::Canvas::mouseMoveEvent(QMouseEvent *e) {
    KoPointerEvent ev(e, QPointF( viewConverter()->viewToDocument(e->pos()) ));
    m_tool->mouseMoveEvent( &ev );
}

void KoShapeSelector::Canvas::mousePressEvent(QMouseEvent *e) {
    KoPointerEvent ev(e, QPointF( viewConverter()->viewToDocument(e->pos()) ));
    m_tool->mousePressEvent( &ev );
}

void KoShapeSelector::Canvas::mouseReleaseEvent(QMouseEvent *e) {
    KoPointerEvent ev(e, QPointF( viewConverter()->viewToDocument(e->pos()) ));
    m_tool->mouseReleaseEvent( &ev );
}

void KoShapeSelector::Canvas::keyReleaseEvent (QKeyEvent *e) {
    m_tool->keyReleaseEvent(e);
}

void KoShapeSelector::Canvas::keyPressEvent( QKeyEvent *e ) {
    m_tool->keyPressEvent(e);
}

void KoShapeSelector::Canvas::paintEvent(QPaintEvent * e) {
    QPainter painter( this );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(e->rect());

    m_parent->m_shapeManager->paint( painter, *(viewConverter()), false );
    painter.end();
}

bool KoShapeSelector::Canvas::event(QEvent *e) {
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

        const QPointF pos(helpEvent->x(), helpEvent->y());
        KoShape *shape = m_parent->m_shapeManager->shapeAt(pos);
        if(shape) {
            IconShape *is = static_cast<IconShape*> (shape);
            QToolTip::showText(helpEvent->globalPos(), is->toolTip());
        }
        else
            QToolTip::showText(helpEvent->globalPos(), "");
    }
    return QWidget::event(e);
}

#include "KoShapeSelector.moc"
