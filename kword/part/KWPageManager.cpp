/* This file is part of the KOffice project
 * Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>
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
#include "KWPageManager.h"
#include "KWPage.h"

#include <KoShape.h>
#include <KoUnit.h>

#include <KDebug>

//#define DEBUG_PAGES

KWPageManager::KWPageManager()
    : m_firstPage(-1),
    m_onlyAllowAppend(false),
    m_preferPageSpread(false)
{
    m_defaultPageLayout = KoPageLayout::standardLayout();
}

KWPageManager::~KWPageManager() {
    qDeleteAll(m_pageList);
}

int KWPageManager::pageNumber(const QPointF &point) const {
    int pageNumber=m_firstPage;
    double startOfpage = 0.0;
    foreach(KWPage *page, m_pageList) {
        if(startOfpage >= point.y())
            break;
        startOfpage += page->height();
        pageNumber = page->pageNumber();
    }
    return pageNumber;
}

int KWPageManager::pageNumber(const KoShape *shape) const {
    return pageNumber(shape->absolutePosition());
}
int KWPageManager::pageNumber(const double y) const {
    return pageNumber(QPointF(0, y));
}
int KWPageManager::pageCount() const {
    if(m_pageList.isEmpty())
        return 0;
    KWPage *first = m_pageList.first();
    KWPage *last = m_pageList.last();
    return 1 + last->pageNumber() + (last->pageSide() == KWPage::PageSpread?1:0) - first->pageNumber();
}

KWPage* KWPageManager::page(int pageNum) const {
    foreach(KWPage *page, m_pageList) {
        if(page->pageNumber() == pageNum ||
                page->pageSide() == KWPage::PageSpread && page->pageNumber()+1 == pageNum)
            return page;
    }
#ifdef DEBUG_PAGES
    kWarning(31001) << "KWPageManager::page(" << pageNum << ") failed; Requested page does not exist ["<< m_firstPage << "-"<< lastPageNumber() << "]"<< endl;
    kDebug(32001) << kBacktrace();
#endif
    return 0;
}
KWPage* KWPageManager::page(const KoShape *shape) const {
    return page(pageNumber(shape));
}
KWPage* KWPageManager::page(const QPointF &point) const {
    return page(pageNumber(point));
}
KWPage* KWPageManager::page(double y) const {
    return page(pageNumber(y));
}

void KWPageManager::setStartPage(int startPage) {
    int offset = startPage - m_firstPage;
    bool switchSides = startPage % 2 != m_firstPage % 2;
    foreach(KWPage *page, m_pageList) {
        page->m_pageNum = page->m_pageNum + offset;
        if(switchSides && page->pageSide() != KWPage::PageSpread)
            page->m_pageSide = page->m_pageSide == KWPage::Left ? KWPage::Right : KWPage::Left;
    }
    m_firstPage = startPage;
}

int KWPageManager::lastPageNumber() const {
    return pageCount() + m_firstPage - 1;
}

KWPage* KWPageManager::insertPage(int pageNumber) {
    if(m_onlyAllowAppend)
        return appendPage();
    // increase the pagenumbers of pages following the pageNumber
    foreach(KWPage *page, m_pageList) {
        if(page->pageNumber() >= pageNumber)
            page->m_pageNum++;
    }
    KWPage *page = new KWPage(this, qMin( qMax(pageNumber, m_firstPage), lastPageNumber()+1 ));
    m_pageList.append(page);
    qSort(m_pageList.begin(), m_pageList.end(), compareItems);
    return page;
}

KWPage* KWPageManager::insertPage(KWPage *page) {
    const int increase = page->pageSide() == KWPage::PageSpread ? 2 : 1;
    foreach(KWPage *page, m_pageList) { // increase the pagenumbers of pages following the pageNumber
        if(page->pageNumber() >= page->pageNumber())
            page->m_pageNum += increase;
    }
    m_pageList.append(page);
    qSort(m_pageList.begin(), m_pageList.end(), compareItems);
    return page;
}

KWPage* KWPageManager::appendPage() {
    KWPage *page = new KWPage(this, lastPageNumber() + 1);
    m_pageList.append(page);
    return page;
}

const KoPageLayout KWPageManager::pageLayout(int pageNumber) const {
    KoPageLayout lay = m_defaultPageLayout;
    if(pageNumber >= m_firstPage && pageNumber <= lastPageNumber()) {
        KWPage *page = this->page(pageNumber);
        lay.height = page->height();
        lay.width = page->width();
        lay.top = page->topMargin();
        lay.left = page->leftMargin();
        lay.bottom = page->bottomMargin();
        lay.right = page->rightMargin();
        lay.bindingSide = page->marginClosestBinding();
        lay.pageEdge = page->pageEdgeMargin();

        lay.orientation = page->orientationHint();
        double w = lay.width;
        if(page->pageSide() == KWPage::PageSpread)
            w /= 2.0;
        double h = lay.height;
        if(lay.orientation == KoPageFormat::Landscape)
            qSwap(w, h);
        lay.format = KoPageFormat::guessFormat(POINT_TO_MM(w), POINT_TO_MM(h));
    }
    return lay;
}

double KWPageManager::topOfPage(int pageNum) const {
    return pageOffset(pageNum, false);
}
double KWPageManager::bottomOfPage(int pageNum) const {
    return pageOffset(pageNum, true);
}

double KWPageManager::pageOffset(int pageNum, bool bottom) const {
    if(pageNum < m_firstPage)
        return 0;
    double offset = 0.0;
    foreach(KWPage *page, m_pageList) {
        if(page->pageNumber() == pageNum) {
            if(bottom)
                offset += page->height();
            break;
        }
        offset += page->height() + m_padding.top + m_padding.bottom;
    }
    return offset;
}

void KWPageManager::removePage(int pageNumber) {
    removePage(page(pageNumber));
}
void KWPageManager::removePage(KWPage *page) {
    if(!page)
        return;
    const int count = page->pageSide() == KWPage::PageSpread ? 2 : 1;
    foreach(KWPage *p, m_pageList) {
        if(p->m_pageNum > page->m_pageNum)
            p->m_pageNum -= count;
    }

    m_pageList.removeAll(page);
    delete page;
}

void KWPageManager::setDefaultPage(const KoPageLayout &layout) {
    m_defaultPageLayout = layout;
    // make sure we have 1 default, either pageBound or left/right bound.
    if(m_defaultPageLayout.left < 0 || m_defaultPageLayout.right < 0) {
        m_defaultPageLayout.left = -1;
        m_defaultPageLayout.right = -1;
    } else {
        m_defaultPageLayout.pageEdge = -1;
        m_defaultPageLayout.bindingSide = -1;
        m_defaultPageLayout.left = qMax(m_defaultPageLayout.left, 0.0);
        m_defaultPageLayout.right = qMax(m_defaultPageLayout.right, 0.0);
    }
    //kDebug(32001) <<"setDefaultPage l:" << m_defaultPageLayout.left <<", r:" << m_defaultPageLayout.right <<", a:" << m_defaultPageLayout.pageEdge <<", b:" << m_defaultPageLayout.bindingSide;
}

QPointF KWPageManager::clipToDocument(const QPointF &point) {
    int page=m_firstPage;
    double startOfpage = 0.0;

    foreach (KWPage *p, m_pageList) {
        startOfpage += p->height();
        if(startOfpage >= point.y())
            break;
        page++;
    }
    page = qMin(page, lastPageNumber());
    QRectF rect = this->page(page)->rect();
    if(rect.contains(point))
        return point;

    QPointF rc(point);
    if(rect.top() > rc.y())
        rc.setY(rect.top());
    else if(rect.bottom() < rc.y())
        rc.setY(rect.bottom());

    if(rect.left() > rc.x())
        rc.setX(rect.left());
    else if(rect.right() < rc.x())
        rc.setX(rect.right());
    return rc;
}

QList<KWPage*> KWPageManager::pages() const {
    return QList<KWPage*> (m_pageList);
}

// **** PageList ****
int KWPageManager::compareItems(KWPage *a, KWPage *b)
{
    return b->pageNumber() - a->pageNumber() > 0;
}
