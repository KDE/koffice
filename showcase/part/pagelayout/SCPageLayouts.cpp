/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCPageLayouts.h"

#include <KoPageLayout.h>
#include <KOdfLoadingContext.h>
#include <KOdfStylesReader.h>
#include <KoPALoadingContext.h>
#include <KoPASavingContext.h>
#include <KoPAMasterPage.h>

#include "SCPageLayout.h"
#include "SCPageLayoutSharedSavingData.h"

class SCPageLayoutWrapper
{
public:
    explicit SCPageLayoutWrapper(SCPageLayout * pageLayout)
    : layout(pageLayout)
    {}

    bool operator<(const SCPageLayoutWrapper &other) const
    {
        return *layout < *(other.layout);
    }

    SCPageLayout * layout;
};

SCPageLayouts::SCPageLayouts(QObject *parent)
    : QObject(parent)
{
}

SCPageLayouts::~SCPageLayouts()
{
    QMap<SCPageLayoutWrapper, SCPageLayout *>::iterator it(m_pageLayouts.begin());
    for (; it != m_pageLayouts.end(); ++it) {
        delete it.value();
    }
}

bool SCPageLayouts::saveOdf(KoPASavingContext &context)
{
    SCPageLayoutSharedSavingData * sharedData = new SCPageLayoutSharedSavingData();

    QMap<SCPageLayoutWrapper, SCPageLayout *>::iterator it(m_pageLayouts.begin());
    for (; it != m_pageLayouts.end(); ++it) {
        QString style = it.value()->saveOdf(context);
        sharedData->addPageLayoutStyle(it.value(), style);
    }

    context.addSharedData(KPR_PAGE_LAYOUT_SHARED_SAVING_ID, sharedData);
    return true;
}

bool compareLayouts(const SCPageLayout * p1, const SCPageLayout * p2)
{
    return SCPageLayout::compareByContent(*p1,* p2);
}

bool SCPageLayouts::loadOdf(KoPALoadingContext &context)
{
    QHash<QString, KoXmlElement*> layouts = context.odfLoadingContext().stylesReader().presentationPageLayouts();
    QHash<QString, KoXmlElement*>::iterator it(layouts.begin());

    // TODO need to use the correct page size
    // we should check which layouts are already loaded
    const QMap<QString, KoPAMasterPage *> &masterPages = context.masterPages();
    if (! masterPages.isEmpty()) {
        KoPageLayout &layout = masterPages.begin().value()->pageLayout();
        QRectF pageRect(0, 0, layout.width, layout.height);
        for (; it != layouts.end(); ++it) {
            SCPageLayout * pageLayout = new SCPageLayout();
            if (pageLayout->loadOdf(*(it.value()), pageRect)) {
                QMap<SCPageLayoutWrapper, SCPageLayout *>::const_iterator it(m_pageLayouts.constFind(SCPageLayoutWrapper(pageLayout)));
                if (it != m_pageLayouts.constEnd()) {
                    delete pageLayout;
                }
                else {
                    m_pageLayouts.insert(SCPageLayoutWrapper(pageLayout), pageLayout);
                }
            }
            else {
                delete pageLayout;
            }
        }
    }

    // handel default styles
    layouts = context.odfLoadingContext().defaultStylesReader().presentationPageLayouts();
    it = layouts.begin();
    QList<SCPageLayout *> defaultLayouts;
    for (; it != layouts.end(); ++it) {
        SCPageLayout * pageLayout = new SCPageLayout();
        // this is not used but needed
        QRectF pageRect(0, 0, 800, 600);
        if (pageLayout->loadOdf(*(it.value()), pageRect)) {
            defaultLayouts.append(pageLayout);
        }
        else {
            delete pageLayout;
        }
    }
    QList<SCPageLayout *> documentLayouts = m_pageLayouts.values();

    qSort(documentLayouts.begin(), documentLayouts.end(), compareLayouts);
    qSort(defaultLayouts.begin(), defaultLayouts.end(), compareLayouts);

    QList<SCPageLayout *>::const_iterator docIt = documentLayouts.constBegin();
    QList<SCPageLayout *>::const_iterator defaultIt = defaultLayouts.constBegin();
    while (defaultIt != defaultLayouts.constEnd()) {
        if (docIt == documentLayouts.constEnd() || compareLayouts(*defaultIt, *docIt)) {
            m_pageLayouts.insert(SCPageLayoutWrapper(*defaultIt), *defaultIt);
            ++defaultIt;
        }
        else if (compareLayouts(*docIt, *defaultIt)) {
            ++docIt;
        }
        else {
            // it already exist
            ++docIt;
            ++defaultIt;
        }
    }

    return true;
}

SCPageLayout *SCPageLayouts::pageLayout(const QString &name, KoPALoadingContext &loadingContext, const QRectF &pageRect)
{
    SCPageLayout *pageLayout = 0;

    QHash<QString, KoXmlElement*> layouts = loadingContext.odfLoadingContext().stylesReader().presentationPageLayouts();
    QHash<QString, KoXmlElement*>::iterator it(layouts.find(name));

    if (it != layouts.end()) { // Found the xmlelement
        pageLayout = new SCPageLayout();
        if (pageLayout->loadOdf(*(it.value()), pageRect)) {
            QMap<SCPageLayoutWrapper, SCPageLayout *>::const_iterator iter(m_pageLayouts.constFind(SCPageLayoutWrapper(pageLayout)));
            if (iter != m_pageLayouts.constEnd()) { // found it in our m_pageLayouts map
                delete pageLayout;
                pageLayout = *iter;
            } else {
                m_pageLayouts.insert(SCPageLayoutWrapper(pageLayout), pageLayout);
            }
        } else { // load failed
            delete pageLayout;
            pageLayout = 0;
        }
    }
    return pageLayout;
}

const QList<SCPageLayout *> SCPageLayouts::layouts() const
{
    return m_pageLayouts.values();
}
