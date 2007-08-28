/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPADocument.h"

#include <KTemporaryFile>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoOasisStyles.h>
#include <KoSavingContext.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeManager.h>
#include <KoShapeLayer.h>
#include <KoPathShape.h>
#include <KoLineBorder.h>
#include <KoDom.h>
#include <KoXmlNS.h>

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPAStyles.h"
#include "KoPALoadingContext.h"

#include <typeinfo>

KoPADocument::KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode )
{
//    KoPAMasterPage * masterPage = new KoPAMasterPage();
//    m_masterPages.append( masterPage );
#ifndef NDEBUG
    //TODO This produces some pages to be used for testing remove when we have loading support
/*    KoPathShape *pathShape = new KoPathShape();
    pathShape->lineTo( QPointF( 100, 100 ) );
    pathShape->setPosition( QPointF( 100, 100 ) );
    pathShape->setBorder( new KoLineBorder( 1.0 ) );
    KoShapeLayer * parentShape = dynamic_cast<KoShapeLayer *>( masterPage->iterator()[0] );
    if ( parentShape )
    {
        parentShape->addChild( pathShape );
    }
    insertPage( new KoPAPage(  masterPage ), 1 );
    insertPage(  new KoPAPage(  masterPage ), 2 );*/
#endif
}

KoPADocument::~KoPADocument()
{
    qDeleteAll( m_pages );
    qDeleteAll( m_masterPages );
}

void KoPADocument::paintContent( QPainter &painter, const QRect &rect)
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}

bool KoPADocument::loadXML( QIODevice *, const KoXmlDocument & doc )
{
    Q_UNUSED( doc );

    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

bool KoPADocument::loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                              const KoXmlDocument & settings, KoStore* store )
{
    Q_UNUSED( doc );
    Q_UNUSED( settings );

    emit sigProgress( 0 );
    KoOasisLoadingContext loadingContext( this, oasisStyles, store );
    KoPALoadingContext paContext( loadingContext );

    KoXmlElement content = doc.documentElement();
    KoXmlElement realBody ( KoDom::namedItemNS( content, KoXmlNS::office, "body" ) );

    if ( realBody.isNull() ) {
        kError() << "No body tag found!" << endl;
        return false;
    }

    KoXmlElement body = KoDom::namedItemNS(realBody, KoXmlNS::office, odfTagName( false ));

    if ( body.isNull() ) {
        kError() << "No office:" << odfTagName( false ) << " tag found!" << endl;
        return false;
    }

    //load master pages
    const QHash<QString, KoXmlElement*> masterStyles( oasisStyles.masterPages() );
    QHash<QString, KoXmlElement*>::const_iterator it( masterStyles.constBegin() );
    for ( ; it != masterStyles.constEnd(); ++it )
    {
        qDebug() << "Master:" << it.key();
        KoPAMasterPage * masterPage = newMasterPage();
        masterPage->loadOdf( *( it.value() ), paContext );
        m_masterPages.append( masterPage );
        paContext.addMasterPage (it.key(), masterPage);
    }

    KoXmlElement element;
    forEachElement( element, body )
    {
        if ( element.tagName() == "page" && element.namespaceURI() == KoXmlNS::draw ) {
            KoPAPage* page = newPage();
            page->loadOdf( element, paContext );

            insertPage( page, -1 );
        }
    }

    emit sigProgress( 100 );
    return true;
}

bool KoPADocument::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    if ( !store->open( "content.xml" ) )
        return false;

    KoStoreDevice contentDev( store );
    KoXmlWriter* contentWriter = createOasisXmlWriter( &contentDev, "office:document-content" );

    KoGenStyles mainStyles;
    KoSavingContext savingContext( mainStyles, KoSavingContext::Store );

    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter( &masterStyles, 1 );

    KoPASavingContext paContext( masterStylesTmpWriter, savingContext, 1 );

    paContext.setOptions( KoPASavingContext::DrawId | KoPASavingContext::AutoStyleInStyleXml );

    masterStylesTmpWriter.startElement( "office:master-styles" );

    // save master pages
    foreach( KoPAPageBase *page, m_masterPages )
    {
        page->saveOdf( paContext );
    }
    masterStylesTmpWriter.endElement();

    masterStyles.close();

    // for office:body
    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter( &contentTmpFile, 1 );

    contentTmpWriter.startElement( "office:body" );
    contentTmpWriter.startElement( odfTagName( true ) );

    paContext.setXmlWriter( contentTmpWriter );
    paContext.setOptions( KoPASavingContext::DrawId );

    // save pages
    foreach ( KoPAPageBase *page, m_pages )
    {
        page->saveOdf( paContext );
        paContext.incrementPage();
    }

    contentTmpWriter.endElement(); // office:odfTagName()
    contentTmpWriter.endElement(); // office:body

    contentTmpFile.close();

    contentWriter->startElement( "office:automatic-styles" );
    saveOdfAutomaticStyles( *contentWriter, mainStyles, false );
    contentWriter->endElement();

    // And now we can copy over the contents from the tempfile to the real one
    contentWriter->addCompleteElement( &contentTmpFile );

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;

    if ( !store->close() ) // done with content.xml
        return false;

    //add manifest line for content.xml
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    if ( !store->open( "styles.xml" ) )
        return false;

    manifestWriter->addManifestEntry( "styles.xml", "text/xml" );
    saveOdfDocumentStyles( store, mainStyles, &masterStyles );

    if ( !store->close() ) // done with styles.xml
        return false;

    return true;
}

void KoPADocument::saveOdfAutomaticStyles( KoXmlWriter& contentWriter, KoGenStyles& mainStyles, bool stylesDotXml )
{
    // test style writing
    QList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::StyleGraphicAuto, stylesDotXml );
    QList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        //qDebug() << "style:style" << ( *it ).name;
        ( *it ).style->writeStyle( &contentWriter, mainStyles, "style:style", ( *it ).name , "style:graphic-properties" );
    }

    styles = mainStyles.styles( KoPAStyles::STYLE_PAGE, stylesDotXml );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        //qDebug() << "style:style" << ( *it ).name;
        ( *it ).style->writeStyle( &contentWriter, mainStyles, "style:style", ( *it ).name , "style:drawing-page-properties" );
    }

    styles = mainStyles.styles( KoGenStyle::StylePageLayout, stylesDotXml );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        //qDebug() << "style:style" << ( *it ).name;
        (*it).style->writeStyle( &contentWriter, mainStyles, "style:page-layout", (*it).name, "style:page-layout-properties" );
    }
}

void KoPADocument::saveOdfDocumentStyles( KoStore * store, KoGenStyles& mainStyles, QFile *masterStyles )
{
    KoStoreDevice stylesDev( store );
    KoXmlWriter* stylesWriter = createOasisXmlWriter( &stylesDev, "office:document-styles" );

    stylesWriter->startElement( "office:styles" );
    stylesWriter->endElement(); // office:styles

    stylesWriter->startElement( "office:automatic-styles" );
    saveOdfAutomaticStyles( *stylesWriter, mainStyles, true );
    stylesWriter->endElement(); // office:automatic-styles

    stylesWriter->addCompleteElement( masterStyles );

    stylesWriter->endElement(); // root element (office:document-styles)
    stylesWriter->endDocument();
    delete stylesWriter;
}

KoPAPageBase* KoPADocument::pageByIndex( int index, bool masterPage ) const
{
    if ( masterPage )
    {
        return m_masterPages.at( index );
    }
    else
    {
        return m_pages.at( index );
    }
}

KoPAPageBase* KoPADocument::pageByNavigation( KoPAPageBase * currentPage, KoPageApp::PageNavigation pageNavigation ) const
{
    const QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( currentPage ) ? m_masterPages : m_pages;

    Q_ASSERT( ! pages.isEmpty() );

    KoPAPageBase * newPage = currentPage;

    switch ( pageNavigation )
    {
        case KoPageApp::PageFirst:
            newPage = pages.first();
            break;
        case KoPageApp::PageLast:
            newPage = pages.last();
            break;
        case KoPageApp::PagePrevious:
        {
            int index = pages.indexOf( currentPage ) - 1;
            if ( index >= 0 )
            {
                newPage = pages.at( index );
            }
        }   break;
        case KoPageApp::PageNext:
            // fall through
        default:
        {
            int index = pages.indexOf( currentPage ) + 1;
            if ( index < pages.size() )
            {
                newPage = pages.at( index );
            }
            break;
        }
    }

    return newPage;
}

void KoPADocument::addShape( KoShape * shape )
{
    if(!shape)
        return;

    // the KoShapeController sets the active layer as parent
    KoPAPageBase * page( pageByShape( shape ) );

    bool isMaster = dynamic_cast<KoPAMasterPage*>( page ) != 0; 

    foreach( KoView *view, views() )
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        KoPAPage * p;
        if ( page == kopaView->activePage() ) {
            kopaView->kopaCanvas()->shapeManager()->add( shape );
        }
        else if ( isMaster && ( p = dynamic_cast<KoPAPage*>( kopaView->activePage() ) ) != 0 ) {
            if ( p->masterPage() == page ) {
                kopaView->kopaCanvas()->masterShapeManager()->add( shape );
            }
        }
    }

    postAddShape( page, shape );
}

void KoPADocument::postAddShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

void KoPADocument::removeShape( KoShape *shape )
{
    if(!shape)
        return;

    KoPAPageBase * page( pageByShape( shape ) );

    bool isMaster = dynamic_cast<KoPAMasterPage*>( page ) != 0; 

    foreach( KoView *view, views() ) 
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        KoPAPage * p;
        if ( page == kopaView->activePage() ) {
            kopaView->kopaCanvas()->shapeManager()->remove( shape );
        }
        else if ( isMaster && ( p = dynamic_cast<KoPAPage*>( kopaView->activePage() ) ) != 0 ) {
            if ( p->masterPage() == page ) {
                kopaView->kopaCanvas()->masterShapeManager()->remove( shape );
            }
        }
    }

    postRemoveShape( page, shape );
}

void KoPADocument::postRemoveShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

KoPAPageBase * KoPADocument::pageByShape( KoShape * shape ) const
{
    KoShape * parent = shape;
    KoPAPageBase * page = 0;
    while ( !page && ( parent = parent->parent() ) )
    {
        page = dynamic_cast<KoPAPageBase*>( parent );
    }
    return page;
}

void KoPADocument::setActionEnabled( int actions, bool enable )
{
    foreach( KoView *view, views() ) 
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        kopaView->setActionEnabled( actions, enable );
    }
}

void KoPADocument::insertPage( KoPAPageBase* page, int index )
{
    if ( !page )
        return;

    QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? m_masterPages : m_pages;

    if ( index > pages.size() || index < 0 )
    {
        index = pages.size();
    }

    pages.insert( index, page );
    
    if ( pages.size() == 2 ) {
        setActionEnabled( KoPAView::ActionDeletePage, true );
    }
}

void KoPADocument::insertPage( KoPAPageBase* page, KoPAPageBase* after )
{
    if ( !page )
        return;

    QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? m_masterPages : m_pages;

    int index = 0;

    if ( after != 0 )
    {
        index = pages.indexOf( after ) + 1;

        // Append the page if after wasn't found in pages
        if ( index == 0 )
            index = pages.count();
    }

    pages.insert( index, page );

    if ( pages.size() == 2 ) {
        setActionEnabled( KoPAView::ActionDeletePage, true );
    }
    
    // move active view to new page
}

int KoPADocument::takePage( KoPAPageBase *page )
{
    Q_ASSERT( page );

    QList<KoPAPageBase *>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? m_masterPages : m_pages;

    int index = pages.indexOf( page );

    // it should not be possible to delete the last page
    Q_ASSERT( pages.size() > 1 );

    if ( index != -1 ) {
        pages.removeAt( index );

        // change to previous page when the page is the active one if the first one is delete go to the next one
        int newIndex = index == 0 ? 0 : index - 1;
        KoPAPageBase * newActivePage = pages.at( newIndex );
        foreach( KoView *view, views() )
        {
            KoPAView * kopaView = static_cast<KoPAView*>( view );
            if ( page == kopaView->activePage() ) {
                kopaView->setActivePage( newActivePage );
            }
        }
    }

    if ( pages.size() == 1 ) {
        setActionEnabled( KoPAView::ActionDeletePage, false );
    }
    return index;
}

QList<KoPAPageBase*> KoPADocument::pages() const
{
    return m_pages;
}

KoPAPage * KoPADocument::newPage( KoPAMasterPage * masterPage )
{
    return new KoPAPage( masterPage );
}

KoPAMasterPage * KoPADocument::newMasterPage()
{
    return new KoPAMasterPage();
}


#include "KoPADocument.moc"
