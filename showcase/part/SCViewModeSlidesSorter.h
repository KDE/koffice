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

#ifndef SCVIEWMODESLIDESSORTER_H
#define SCVIEWMODESLIDESSORTER_H

#include <QSize>
#include <QRect>

#include <KoPAViewMode.h>

class KoPAView;
class KoPACanvas;
class KoPAPage;

class SCViewModeSlidesSorter : public KoPAViewMode
{
    Q_OBJECT
public:
    SCViewModeSlidesSorter(KoPAView *view, KoPACanvas *canvas);
    ~SCViewModeSlidesSorter();

    void paint(KoPACanvas* canvas, QPainter &painter, const QRectF &paintRect);
    void paintEvent(KoPACanvas * canvas, QPaintEvent* event);
    void tabletEvent(QTabletEvent *event, const QPointF &point);
    void mousePressEvent(QMouseEvent *event, const QPointF &point);
    void mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point);
    void mouseMoveEvent(QMouseEvent *event, const QPointF &point);
    void mouseReleaseEvent(QMouseEvent *event, const QPointF &point);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent * event, const QPointF &point);

    void activate(KoPAViewMode *previousViewMode);
    void deactivate();

    void updateActivePage(KoPAPage *page);

    void addShape(KShape *shape);
    void removeShape(KShape *shape);

protected:

    /**
     * Fills the editor with presentation slides and ordored them in the SCSlidesSorter
     */
    void populate();

    /**
     * Moves a page from pageNumber to pageAfterNumber
     *
     * @param pageNumber the number of the page to move
     * @param pageAfterNumber the number of the place the page should move to
     */
    void movePage(int pageNumber, int pageAfterNumber);

    /**
     * The count of the page
     *
     * @return the count of the page
     */
    int pageCount() const;

    /**
     * The icon size
     *
     * @return the icon size defined before
     */
    QSize iconSize() const;

    /**
     * The rect of an items, essentialy used to have the size of the full icon
     *
     * @return the rect of the item
     */
    QRect itemSize() const;

    /**
     * Setter of the size with a rect
     *
     * @param size which is a QRect
     */
    void setItemSize(QRect size);

    /**
     * Permit to know if a slide is draging
     *
     * @return boolean
     */
    bool isDraging() const;

    /**
     * Setter for the draging flag
     *
     * @param flag boolean
     */
    void setDragingFlag(bool flag = true);

    /**
     * Return the last item number it were on
     *
     * @return the last item number it was on
     */
    int lastItemNumber() const;

    /**
     * Setter of the last item number it were on
     *
     * @param number of the item number it is on
     */
    void setLastItemNumber(int number);

private:
    class SCSlidesSorter;
    SCSlidesSorter * m_slidesSorter;
    QSize m_iconSize;
    QRect m_itemSize;
    bool m_sortNeeded;
    const int m_pageCount;
    bool m_dragingFlag;
    int m_lastItemNumber;
};

#endif // SCVIEWMODESLIDESSORTER_H
