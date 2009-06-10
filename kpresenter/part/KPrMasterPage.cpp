/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "KPrMasterPage.h"

#include "pagelayout/KPrPageLayouts.h"
#include <KoXmlNS.h>
#include <KoPALoadingContext.h>
#include <kdebug.h>

KPrMasterPage::KPrMasterPage()
{
}

KPrMasterPage::~KPrMasterPage()
{
}

KoPageApp::PageType KPrMasterPage::pageType() const
{
    return KoPageApp::Slide;
}

void KPrMasterPage::loadOdfPageExtra( const KoXmlElement &element, KoPALoadingContext & loadingContext )
{
    // the layout needs to be loaded after the shapes are already loaded so the initialization of the data works
    KPrPageLayout * layout = 0;
    if ( element.hasAttributeNS( KoXmlNS::presentation, "presentation-page-layout-name" ) ) {
        KPrPageLayouts * layouts = dynamic_cast<KPrPageLayouts *>( loadingContext.dataCenter( PageLayouts ) );
        Q_ASSERT( layouts );
        if ( layouts ) {
            QString layoutName = element.attributeNS( KoXmlNS::presentation, "presentation-page-layout-name" );
            QRectF pageRect( 0, 0, pageLayout().width, pageLayout().height );
            layout = layouts->pageLayout( layoutName, loadingContext, pageRect );
            kDebug(33001) << "page layout" << layoutName << layout;
        }
    }
    placeholders().init( layout, childShapes() );
}
