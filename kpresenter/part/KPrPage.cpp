/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KPrPage.h"

#include <QString>

#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoStyleStack.h>
#include <KoGenStyle.h>
#include <KoPALoadingContext.h>

#include "KPrDocument.h"
#include "KPrPageApplicationData.h"
#include "KPrNotes.h"
#include "pagelayout/KPrPageLayout.h"
#include "pagelayout/KPrPageLayouts.h"
#include "pageeffects/KPrPageEffectRegistry.h"
#include "pageeffects/KPrPageEffect.h"

#include <kdebug.h>

class KPrPage::Private
{
public:
    Private( KPrPage * page, KPrDocument * document )
    : pageNotes( new KPrNotes( page, document ) )
    , layout( 0 )
    {}

    ~Private()
    {
        delete pageNotes;
    }

    KPrNotes * pageNotes;
    KPrPageLayout * layout;
};

KPrPage::KPrPage( KoPAMasterPage * masterPage, KPrDocument * document )
: KoPAPage( masterPage )
, d( new Private( this, document ) )
{
    setApplicationData( new KPrPageApplicationData() );
}

KPrPage::~KPrPage()
{
    delete d;
}

KPrPageApplicationData * KPrPage::pageData( KoPAPageBase * page )
{
    KPrPageApplicationData * data = dynamic_cast<KPrPageApplicationData *>( page->applicationData() );
    Q_ASSERT( data );
    return data;
}

KPrNotes *KPrPage::pageNotes()
{
    return d->pageNotes;
}

void KPrPage::addShape( KoShape * shape )
{
    Q_ASSERT( shape );
    // TODO
}

void KPrPage::removeShape( KoShape * shape )
{
    Q_ASSERT( shape );
    // TODO
}

void KPrPage::saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const
{
    KoPAPage::saveOdfPageStyleData( style, paContext );
    style.addProperty( "presentation:background-visible", ( m_pageProperties & DisplayMasterBackground ) == DisplayMasterBackground );
    style.addProperty( "presentation:background-objects-visible", ( m_pageProperties & DisplayMasterShapes ) == DisplayMasterShapes );

    KPrPageApplicationData * data = dynamic_cast<KPrPageApplicationData *>( applicationData() );
    Q_ASSERT( data );
    KPrPageEffect * pageEffect = data->pageEffect();

    if ( pageEffect ) {
        pageEffect->saveOdfSmilAttributes( style );
    }
}

void KPrPage::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
    KoPAPage::loadOdfPageTag( element, loadingContext );

    if ( element.hasAttributeNS( KoXmlNS::presentation, "presentation-page-layout-name" ) ) {
        KPrPageLayouts * layouts = dynamic_cast<KPrPageLayouts *>( loadingContext.dataCenter( PageLayouts ) );
        Q_ASSERT( layouts );
        if ( layouts ) {
            kDebug(33001) << "layout" << element.attributeNS( KoXmlNS::presentation, "presentation-page-layout-name" );
            QString layoutName = element.attributeNS( KoXmlNS::presentation, "presentation-page-layout-name" );
            QRectF pageRect( 0, 0, pageLayout().width, pageLayout().height );
            d->layout = layouts->pageLayout( layoutName, loadingContext, pageRect );
        }
    }

    KoStyleStack& styleStack = loadingContext.odfLoadingContext().styleStack();

    int pageProperties = m_pageProperties & UseMasterBackground;
    if ( styleStack.property( KoXmlNS::presentation, "background-objects-visible" ) == "true" ) {
        pageProperties = pageProperties | DisplayMasterShapes;
    }
    if ( styleStack.property( KoXmlNS::presentation, "background-visible" ) == "true" ) {
        pageProperties = pageProperties | DisplayMasterBackground;
    }
    m_pageProperties = pageProperties;

    KPrPageApplicationData * data = dynamic_cast<KPrPageApplicationData *>( applicationData() );
    Q_ASSERT( data );

    KoOdfStylesReader& stylesReader = loadingContext.odfLoadingContext().stylesReader();
    const KoXmlElement * styleElement = stylesReader.findContentAutoStyle( element.attributeNS( KoXmlNS::draw, "style-name" ), "drawing-page" );
    KoXmlNode node = styleElement->namedItemNS( KoXmlNS::style, "drawing-page-properties" );
    if ( node.isElement() ) {
        data->setPageEffect( KPrPageEffectRegistry::instance()->createPageEffect( node.toElement() ) );
    }

    node = element.namedItemNS(KoXmlNS::presentation, "notes");
    if ( node.isElement() ) {
        d->pageNotes->loadOdf(node.toElement(), loadingContext);
    }
}

bool KPrPage::saveOdfPresentationNotes(KoPASavingContext &paContext) const
{
    d->pageNotes->saveOdf(paContext);
    return true;
}

