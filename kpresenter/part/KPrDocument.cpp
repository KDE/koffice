/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KPrDocument.h"


#include "KPrView.h"
#include "KPrPage.h"
#include "KPrMasterPage.h"
#include "KPrShapeApplicationData.h"
#include "KPrFactory.h"
#include "KPrViewModeNotes.h"
#include "KPrPlaceholderShapeFactory.h"
#include "KPrSoundCollection.h"
#include "pagelayout/KPrPageLayouts.h"
#include "tools/KPrPlaceholderToolFactory.h"
#include "commands/KPrSetCustomSlideShowsCommand.h"
#include <KoPACanvas.h>
#include <KoPAViewModeNormal.h>
#include <KoShapeManager.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>

class InitOnce
{
public:
    InitOnce()
    {
        KoToolRegistry * toolRegistry = KoToolRegistry::instance();
        toolRegistry->add( new KPrPlaceholderToolFactory( toolRegistry ) );
        KoShapeRegistry * registry = KoShapeRegistry::instance();
        registry->addFactory( new KPrPlaceholderShapeFactory( registry ) );
    }
};

KPrDocument::KPrDocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoPADocument( parentWidget, parent, singleViewMode )
, m_customSlideShows(new KPrCustomSlideShows())
{
    K_GLOBAL_STATIC( InitOnce, s_initOnce );
    InitOnce * initOnce = s_initOnce;
    // have this is as otherwise we get a warning from the compiler
    // the variable is used and the way it is done is to only call it once
    Q_UNUSED( initOnce );

    setComponentData(KPrFactory::componentData(), false);
    setTemplateType( "kpresenter_template" );

    KoShapeLoadingContext::addAdditionalAttributeData( KoShapeLoadingContext::AdditionalAttributeData(
                                                       KoXmlNS::presentation, "placeholder",
                                                       "presentation:placeholder" ) );

    KoShapeLoadingContext::addAdditionalAttributeData( KoShapeLoadingContext::AdditionalAttributeData(
                                                       KoXmlNS::presentation, "class",
                                                       "presentation:class" ) );

    KPrSoundCollection *soundCol = new KPrSoundCollection();
    insertIntoDataCenterMap("SoundCollection", soundCol);

    insertIntoDataCenterMap( PageLayouts, new KPrPageLayouts() );

    loadKPrConfig();
}

KPrDocument::~KPrDocument()
{
    saveKPrConfig();
    delete m_customSlideShows;
}

KoView * KPrDocument::createViewInstance( QWidget *parent )
{
    return new KPrView( this, parent );
}

const char * KPrDocument::odfTagName( bool withNamespace )
{
    return withNamespace ? "office:presentation": "presentation";
}

bool KPrDocument::saveOdfEpilogue( KoPASavingContext & context )
{
    context.xmlWriter().startElement( "presentation:settings" );
    if ( !m_activeCustomSlideShow.isEmpty() && m_customSlideShows->names().contains(  m_activeCustomSlideShow ) ) {
        context.xmlWriter().addAttribute( "presentation:show", m_activeCustomSlideShow );
    }
    m_customSlideShows->saveOdf( context );
    context.xmlWriter().endElement();//presentation:settings
    return true;
}

void KPrDocument::saveOdfDocumentStyles( KoPASavingContext & context )
{
    KoPADocument::saveOdfDocumentStyles( context );
    KPrPageLayouts * layouts = dynamic_cast<KPrPageLayouts *>( dataCenterMap().value( PageLayouts ) );
    Q_ASSERT( layouts );
    if ( layouts ) {
        layouts->saveOdf( context );
    }
}

bool KPrDocument::loadOdfEpilogue( const KoXmlElement & body, KoPALoadingContext & context )
{
    const KoXmlElement & presentationSettings( KoXml::namedItemNS( body, KoXmlNS::presentation, "settings" ) );
    if ( !presentationSettings.isNull() ) {
        m_customSlideShows->loadOdf( presentationSettings, context );
    }

    m_activeCustomSlideShow = QString( "" );
    if ( presentationSettings.hasAttributeNS( KoXmlNS::presentation, "show" ) ) {
        QString show = presentationSettings.attributeNS( KoXmlNS::presentation, "show" );
        if ( m_customSlideShows->names().contains( show ) ) {
            m_activeCustomSlideShow = show;
        }
    }

    return true;
}

bool KPrDocument::loadOdfDocumentStyles( KoPALoadingContext & context )
{
    KPrPageLayouts * layouts = dynamic_cast<KPrPageLayouts *>( dataCenterMap().value( PageLayouts ) );
    Q_ASSERT( layouts );
    if ( layouts ) {
        layouts->loadOdf( context );
    }
    return true;
}

KoPAPage * KPrDocument::newPage( KoPAMasterPage * masterPage )
{
    return new KPrPage( masterPage, this );
}

KoPAMasterPage * KPrDocument::newMasterPage()
{
    return new KPrMasterPage();
}

KoOdf::DocumentType KPrDocument::documentType() const
{
    return KoOdf::Presentation;
}

void KPrDocument::addAnimation( KPrShapeAnimation * animation )
{
    KoShape * shape = animation->shape();

    KPrShapeAnimations & animations( animationsByPage( pageByShape( shape ) ) );

    // add animation to the list of animations
    animations.add( animation );

    // add animation to the shape animation data so that it can be regenerated on delete shape and undo
    KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
    if ( applicationData == 0 ) {
        applicationData = new KPrShapeApplicationData();
        shape->setApplicationData( applicationData );
    }
    applicationData->animations().insert( animation );
}

void KPrDocument::removeAnimation( KPrShapeAnimation * animation, bool removeFromApplicationData )
{
    KoShape * shape = animation->shape();

    KPrShapeAnimations & animations( animationsByPage( pageByShape( shape ) ) );

    // remove animation from the list of animations
    animations.remove( animation );

    if ( removeFromApplicationData ) {
        // remove animation from the shape animation data
        KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
        Q_ASSERT( applicationData );
        applicationData->animations().remove( animation );
    }
}

void KPrDocument::postAddShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
    if ( applicationData ) {
        // reinsert animations. this is needed on undo of a delete shape that had a animations
        QSet<KPrShapeAnimation *> animations = applicationData->animations();
        for ( QSet<KPrShapeAnimation *>::const_iterator it( animations.begin() ); it != animations.end(); ++it ) {
            addAnimation( *it );
        }
    }
}

void KPrDocument::postRemoveShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
    if ( applicationData ) {
        QSet<KPrShapeAnimation *> animations = applicationData->animations();
        for ( QSet<KPrShapeAnimation *>::const_iterator it( animations.begin() ); it != animations.end(); ++it ) {
            // remove animations, don't remove from shape application data so that it can be reinserted on undo.
            removeAnimation( *it, false );
        }
    }
}

void KPrDocument::pageRemoved( KoPAPageBase * page, QUndoCommand * parent )
{
    // only normal pages can be part of a slide show
    if ( dynamic_cast<KPrPage *>( page ) ) {
        KPrCustomSlideShows * slideShows = new KPrCustomSlideShows( *customSlideShows() );
        slideShows->removeSlideFromAll( page );
        // maybe we should check if old and new are different and only than create the command
        new KPrSetCustomSlideShowsCommand( this, slideShows, parent );
    }
}

void KPrDocument::loadKPrConfig()
{
    KSharedConfigPtr config = componentData().config();

    if ( config->hasGroup( "SlideShow" ) ) {
        KConfigGroup configGroup = config->group( "SlideShow" );
        m_presentationMonitor = configGroup.readEntry<int>( "PresentationMonitor", 0 );
        m_presenterViewEnabled = configGroup.readEntry<bool>( "PresenterViewEnabled", false );
    }
}

void KPrDocument::saveKPrConfig()
{
    KSharedConfigPtr config = componentData().config();
    KConfigGroup configGroup = config->group( "SlideShow" );

    configGroup.writeEntry( "PresentationMonitor", m_presentationMonitor );
    configGroup.writeEntry( "PresenterViewEnabled", m_presenterViewEnabled );
}

KoPageApp::PageType KPrDocument::pageType() const
{
    return KoPageApp::Slide;
}

KPrShapeAnimations & KPrDocument::animationsByPage( KoPAPageBase * page )
{
    KPrPageData * pageData = dynamic_cast<KPrPageData *>( page );
    Q_ASSERT( pageData );
    return pageData->animations();
}

KPrCustomSlideShows* KPrDocument::customSlideShows()
{
    return m_customSlideShows;
}

void KPrDocument::setCustomSlideShows( KPrCustomSlideShows* replacement )
{
    m_customSlideShows = replacement;
}

int KPrDocument::presentationMonitor()
{
    return m_presentationMonitor;
}

void KPrDocument::setPresentationMonitor( int monitor )
{
    m_presentationMonitor = monitor;
}

bool KPrDocument::isPresenterViewEnabled()
{
    return m_presenterViewEnabled;
}

void KPrDocument::setPresenterViewEnabled( bool enabled )
{
    m_presenterViewEnabled = enabled;
}

QList<KoPAPageBase*> KPrDocument::slideShow() const
{
    if ( !m_activeCustomSlideShow.isEmpty() &&
            m_customSlideShows->names().contains( m_activeCustomSlideShow ) ) {
        return m_customSlideShows->getByName( m_activeCustomSlideShow );
    }

    return pages();
}

QString KPrDocument::activeCustomSlideShow() const
{
    return m_activeCustomSlideShow;
}

void KPrDocument::setActiveCustomSlideShow( const QString &customSlideShow )
{
    m_activeCustomSlideShow = customSlideShow;
}

#include "KPrDocument.moc"

