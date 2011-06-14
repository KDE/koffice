/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCMasterPage.h"

#include "pagelayout/SCPageLayouts.h"
#include <KOdfXmlNS.h>
//#include <KResourceManager.h>
#include "Showcase.h"
#include <KoPALoadingContext.h>
#include <KoOdfWorkaround.h>
#include <kdebug.h>

SCMasterPage::SCMasterPage()
{
}

SCMasterPage::~SCMasterPage()
{
}

KoPageApp::PageType SCMasterPage::pageType() const
{
    return KoPageApp::Slide;
}

bool SCMasterPage::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
#ifndef NWORKAROUND_ODF_BUGS
    KoOdfWorkaround::setFixPresentationPlaceholder(true, context);
#endif
    bool retval = KoPAPageBase::loadOdf(element, context);
#ifndef NWORKAROUND_ODF_BUGS
    KoOdfWorkaround::setFixPresentationPlaceholder(false, context);
#endif
    return retval;
}

void SCMasterPage::loadOdfPageExtra(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    // the layout needs to be loaded after the shapes are already loaded so the initialization of the data works
    SCPageLayout * layout = 0;
    if (element.hasAttributeNS(KOdfXmlNS::presentation, "presentation-page-layout-name")) {
        SCPageLayouts *layouts = loadingContext.documentResourceManager()->resource(Showcase::PageLayouts).value<SCPageLayouts*>();
        Q_ASSERT(layouts);
        if (layouts) {
            QString layoutName = element.attributeNS(KOdfXmlNS::presentation, "presentation-page-layout-name");
            QRectF pageRect(0, 0, pageLayout().width, pageLayout().height);
            layout = layouts->pageLayout(layoutName, loadingContext, pageRect);
            kDebug(33001) << "page layout" << layoutName << layout;
        }
    }
    placeholders().init(layout, shapes());
}
