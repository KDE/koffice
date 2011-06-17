/* This file is part of the KDE project
*
* Copyright (C) 2010 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#include "SCViewModeSlidesSorter.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QVariant>
#include <QScrollBar>
#include <QListWidget>

#include <KResourceManager.h>
#include <KoRuler.h>
#include <KSelection.h>
#include <KShapeLayer.h>
#include <KShapeManager.h>
#include <KoText.h>
#include <KToolManager.h>
#include <KoToolProxy.h>
#include <KoZoomController.h>

#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAPageBase.h>
#include <KoPAMasterPage.h>
#include <KoPAView.h>

#include <KoPAPageMoveCommand.h>

#include <KDebug>

    /**
     * This class manage the QListWidget itself.
     * Use all the getters and setters of the SCViewModeSlidesSorter.
     * Most of the functions are Qt overrides to have the wished comportment.
     */
class SCViewModeSlidesSorter::SCSlidesSorter : public QListWidget {
    public:
        SCSlidesSorter (SCViewModeSlidesSorter * viewModeSlidesSorter, QWidget * parent = 0)
            : QListWidget(parent)
            , m_viewModeSlidesSorter(viewModeSlidesSorter)
            , m_movingPageNumber(-1)
        {
            setViewMode(QListView::IconMode);
            setResizeMode(QListView::Adjust);
            setDragDropMode(QAbstractItemView::DragDrop);
        };
        ~SCSlidesSorter(){};

        virtual Qt::DropActions supportedDropActions() const
        {
            return Qt::MoveAction;
        }

        virtual void paintEvent (QPaintEvent * ev);

        virtual void startDrag (Qt::DropActions supportedActions);

        virtual void dropEvent(QDropEvent* ev);

        virtual void dragMoveEvent(QDragMoveEvent* ev);

        virtual QStringList mimeTypes() const;

        virtual QMimeData* mimeData(const QList<QListWidgetItem*> items) const;

        int pageBefore(QPoint point);

    private:
        SCViewModeSlidesSorter * m_viewModeSlidesSorter;
        int m_movingPageNumber;
};


SCViewModeSlidesSorter::SCViewModeSlidesSorter(KoPAView *view, KoPACanvas *canvas)
    : KoPAViewMode(view, canvas)
    , m_slidesSorter(new SCSlidesSorter(this, view->parentWidget()))
    , m_iconSize(QSize(200, 200))
    , m_itemSize(QRect(0, 0, 0, 0))
    , m_sortNeeded(false)
    , m_pageCount(m_view->kopaDocument()->pages().count())
    , m_dragingFlag(false)
    , m_lastItemNumber(-1)
{
    m_slidesSorter->hide();
    m_slidesSorter->setIconSize(m_iconSize);
}

SCViewModeSlidesSorter::~SCViewModeSlidesSorter()
{
}

void SCViewModeSlidesSorter::paint(KoPACanvasBase* /*canvas*/, QPainter &/*painter*/, const QRectF &/*paintRect*/)
{
}

void SCViewModeSlidesSorter::SCSlidesSorter::paintEvent(QPaintEvent* event)
{
    event->accept();
    QListWidget::paintEvent(event);

    // Paint the line where the slide should go
    //bool before = true;
    int lastItemNumber = m_viewModeSlidesSorter->lastItemNumber();
    int currentItemNumber = lastItemNumber;
    /* The page is going to the beginning */
    if (lastItemNumber <= m_movingPageNumber) {
        currentItemNumber = lastItemNumber - 1;
    }

    if (m_viewModeSlidesSorter->isDraging() && currentItemNumber >= 0) {
        QSize size(m_viewModeSlidesSorter->itemSize().width(), m_viewModeSlidesSorter->itemSize().height());

        int numberMod = currentItemNumber%4;
        /* The page is going to the end */
        if (lastItemNumber > m_movingPageNumber) {
            numberMod = currentItemNumber%4 > 0 ? currentItemNumber%4 : 4;
        }
        int verticalValue = (currentItemNumber - numberMod) / 4 * size.height() - verticalScrollBar()->value();
        QPoint point1(numberMod * size.width(), verticalValue);
        QPoint point2(numberMod * size.width(), verticalValue + size.height());
        QLineF line(point1, point2);

        QPainter painter(this->viewport());
        painter.drawLine(line);
    }

}

void SCViewModeSlidesSorter::paintEvent(KoPACanvas * canvas, QPaintEvent* event)
{
    Q_UNUSED(canvas);
    Q_UNUSED(event);
    Q_ASSERT(m_canvas == canvas);
}

void SCViewModeSlidesSorter::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void SCViewModeSlidesSorter::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void SCViewModeSlidesSorter::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void SCViewModeSlidesSorter::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void SCViewModeSlidesSorter::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void SCViewModeSlidesSorter::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void SCViewModeSlidesSorter::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void SCViewModeSlidesSorter::wheelEvent(QWheelEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void SCViewModeSlidesSorter::activate(KoPAViewMode *previousViewMode)
{
    Q_UNUSED(previousViewMode);
    populate();
    KoPAView *view = dynamic_cast<KoPAView *>(m_view);
    if (view) {
        view->hide();
    }
    m_slidesSorter->show();
    m_slidesSorter->setFocus(Qt::ActiveWindowFocusReason);
}

void SCViewModeSlidesSorter::deactivate()
{
    m_slidesSorter->hide();
    // Give the ressources back to the canvas
    m_canvas->resourceManager()->setResource(KoText::ShowTextFrames, 0);
    // Active the view as a basic but active one
    m_view->setActionEnabled(KoPAView::AllActions, true);
    m_view->doUpdateActivePage(m_view->activePage());
    KoPAView *view = dynamic_cast<KoPAView *>(m_view);
    if (view) {
        view->show();
    }
}

void SCViewModeSlidesSorter::updateActivePage(KoPAPageBase *page)
{
    Q_UNUSED(page);
}

void SCViewModeSlidesSorter::addShape(KShape *shape)
{
    Q_UNUSED(shape);
}

void SCViewModeSlidesSorter::removeShape(KShape *shape)
{
    Q_UNUSED(shape);
}

void SCViewModeSlidesSorter::SCSlidesSorter::startDrag (Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions);
    QAbstractItemView::startDrag(Qt::MoveAction);
}


void SCViewModeSlidesSorter::SCSlidesSorter::dragMoveEvent(QDragMoveEvent* ev)
{
    ev->accept();
    m_viewModeSlidesSorter->setDragingFlag();
    pageBefore(ev->pos());
    viewport()->update();
}

void SCViewModeSlidesSorter::SCSlidesSorter::dropEvent(QDropEvent* ev)
{
    m_viewModeSlidesSorter->setDragingFlag(false);
    ev->setDropAction(Qt::IgnoreAction);
    ev->accept();

    int newIndex;
    QByteArray ssData = ev->mimeData()->data("application/x-koffice-sliderssorter");
    int oldIndex = ssData.toInt();

    QListWidgetItem * itemNew = itemAt(ev->pos());
    if (itemNew)
    {
        // Normal case
        newIndex = row(itemNew);
    } else {
        // In case you point the end (no slides under the pointer)
        newIndex = m_viewModeSlidesSorter->pageCount() - 1;
    }

    if (oldIndex != newIndex) {
        if (oldIndex > newIndex) {
            m_viewModeSlidesSorter->movePage(oldIndex, newIndex - 1);
        } else {
            m_viewModeSlidesSorter->movePage(oldIndex, newIndex);
        }

        QListWidgetItem *sourceItem = takeItem(oldIndex);
        insertItem(newIndex, sourceItem);
        // This selection helps the user
        clearSelection();
        item(newIndex)->setSelected(true);
    }

    m_movingPageNumber = -1;
}

QMimeData* SCViewModeSlidesSorter::SCSlidesSorter::mimeData(const QList<QListWidgetItem*> items) const
{
    QListWidgetItem* page = items.first();

    QByteArray ssData = QVariant(row(page)).toByteArray();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-koffice-sliderssorter", ssData);

    return mimeData;
}

QStringList SCViewModeSlidesSorter::SCSlidesSorter::mimeTypes() const
{
    return QStringList() << "application/x-koffice-sliderssorter";
}

int SCViewModeSlidesSorter::SCSlidesSorter::pageBefore(QPoint point)
{
    QListWidgetItem *item = itemAt(point);
    int pageBeforeNumber = -1;
    if (item) {
        pageBeforeNumber = row(item) + 1;
    } else {
        pageBeforeNumber = m_viewModeSlidesSorter->pageCount();
    }
    if (m_movingPageNumber == -1) {
        m_movingPageNumber = pageBeforeNumber;
    }
    m_viewModeSlidesSorter->setLastItemNumber(pageBeforeNumber);
    return pageBeforeNumber;
}

void SCViewModeSlidesSorter::populate()
{
    int currentPage = 0;
    m_slidesSorter->clear();

    QListWidgetItem * item = 0;

    //Load the available slides
    foreach (KoPAPageBase *page, m_view->kopaDocument()->pages())
    {
        currentPage++;
        QString slideName = page->name().isEmpty() ? i18n("Slide %1", currentPage) : page->name();
        item = new QListWidgetItem(QIcon(page->thumbnail(m_iconSize)), slideName, m_slidesSorter);
        item->setFlags((item->flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
    }
    if (item) {
        setItemSize(m_slidesSorter->visualItemRect(item));
    }
}

void SCViewModeSlidesSorter::movePage(int pageNumber, int pageAfterNumber)
{
    KoPAPageBase * page = 0;
    KoPAPageBase * pageAfter = 0;

    if (pageNumber >= 0) {
        page = m_view->kopaDocument()->pageByIndex(pageNumber,false);
    }
    if (pageAfterNumber >= 0) {
        pageAfter = m_view->kopaDocument()->pageByIndex(pageAfterNumber,false);
    }

    if (page) {
        KoPAPageMoveCommand *command = new KoPAPageMoveCommand(m_view->kopaDocument(), page, pageAfter);
        m_view->kopaDocument()->addCommand(command);
    }
}

int SCViewModeSlidesSorter::pageCount() const
{
    return m_pageCount;
}

QSize SCViewModeSlidesSorter::iconSize() const
{
    return m_iconSize;
}

QRect SCViewModeSlidesSorter::itemSize() const
{
    return m_itemSize;
}

void SCViewModeSlidesSorter::setItemSize(QRect size)
{
    m_itemSize = size;
}

bool SCViewModeSlidesSorter::isDraging() const
{
    return m_dragingFlag;
}

void SCViewModeSlidesSorter::setDragingFlag(bool flag)
{
    m_dragingFlag = flag;
}

int SCViewModeSlidesSorter::lastItemNumber() const
{
    return m_lastItemNumber;
}

void SCViewModeSlidesSorter::setLastItemNumber(int number)
{
    m_lastItemNumber = number;
}

