/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 
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

#include "KoShapeSavingContext.h"
#include "KoShapeLayer.h"
#include "KoDataCenter.h"

#include <KoGenStyles.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kmimetype.h>

#include <QtCore/QTime>
#include <kdebug.h>

KoShapeSavingContext::KoShapeSavingContext( KoXmlWriter &xmlWriter, KoGenStyles& mainStyles,
                                            KoEmbeddedDocumentSaver& embeddedSaver, SavingMode savingMode )
: m_xmlWriter( &xmlWriter )
, m_savingOptions( 0 )
, m_drawId( 0 )
, m_mainStyles( mainStyles )
, m_embeddedSaver( embeddedSaver )
, m_savingMode( savingMode )
{
}

KoShapeSavingContext::~KoShapeSavingContext()
{
}

KoXmlWriter & KoShapeSavingContext::xmlWriter()
{
    return *m_xmlWriter;
}

void KoShapeSavingContext::setXmlWriter( KoXmlWriter &_xmlWriter )
{
    m_xmlWriter = &_xmlWriter;
}

KoGenStyles & KoShapeSavingContext::mainStyles()
{
    return m_mainStyles;
}

KoEmbeddedDocumentSaver & KoShapeSavingContext::embeddedSaver()
{
    return m_embeddedSaver;
}

bool KoShapeSavingContext::isSet( ShapeSavingOption option ) const
{
    return m_savingOptions & option;
}

void KoShapeSavingContext::setOptions( KoShapeSavingOptions options )
{
    m_savingOptions = options;
}

KoShapeSavingContext::KoShapeSavingOptions KoShapeSavingContext::options() const
{
    return m_savingOptions;
}

void KoShapeSavingContext::addOption( ShapeSavingOption option) {
    m_savingOptions = m_savingOptions | option;
}

void KoShapeSavingContext::removeOption( ShapeSavingOption option) {
    if(isSet(option))
        m_savingOptions = m_savingOptions ^ option; // xor to remove it.
}

const QString KoShapeSavingContext::drawId( const KoShape * shape, bool insert )
{
    QMap<const KoShape *, QString>::const_iterator it( m_drawIds.find( shape ) );
    if ( it == m_drawIds.constEnd() ) {
        if ( insert == true ) {
            it = m_drawIds.insert( shape, QString( "shape%1" ).arg( ++m_drawId ) );
        }
        else {
            return QString();
        }
    }
    return it.value();
}

void KoShapeSavingContext::clearDrawIds()
{
    m_drawIds.clear();
    m_drawId = 0;
}

void KoShapeSavingContext::addLayerForSaving( const KoShapeLayer * layer )
{
    if( layer && ! m_layers.contains( layer ) )
        m_layers.append( layer );
}

void KoShapeSavingContext::saveLayerSet( KoXmlWriter * xmlWriter ) const
{
    Q_ASSERT( xmlWriter );

    xmlWriter->startElement( "draw:layer-set" );
    foreach( const KoShapeLayer * layer, m_layers )
    {
        xmlWriter->startElement( "draw:layer" );
        xmlWriter->addAttribute( "draw:name", layer->name() );
        if( layer->isLocked() )
            xmlWriter->addAttribute( "draw:protected", "true" );
        if( ! layer->isVisible() )
            xmlWriter->addAttribute( "draw:display", "none" );
        xmlWriter->endElement();  // draw:layer
    }
    xmlWriter->endElement();  // draw:layer-set
}

QString KoShapeSavingContext::addImageForSaving( const QPixmap &pixmap )
{
    QString filename = "Pictures/image" + QTime::currentTime().toString( "_hh_mm_ss_zzz.png" );
    if( m_pixmaps.contains( filename ) )
    {
        int i = 1;
        while( m_pixmaps.contains( filename + QString("_%1").arg( i ) ) )
            i++;
        filename += QString("_%1").arg( i );
    }

    m_pixmaps[filename] = pixmap;
    return filename;
}


bool KoShapeSavingContext::saveImages( KoStore * store, KoXmlWriter * manifestWriter ) const
{
    QString fileName("/tmp/temp.png");
    // Find the mimetype only by the extension, not by file content (as the file is empty!)
    const QString mimetype( KMimeType::findByPath( fileName, 0 ,true )->name() );
    QMapIterator<QString, QPixmap> i( m_pixmaps );
    while( i.hasNext() )
    {
        i.next();
        if( store->open( i.key() ) )
        {
            KoStoreDevice dev(store);
            if ( ! i.value().save(&dev, "PNG" ) )
                return false; // e.g. bad image?
            if ( !store->close() )
                return false; // e.g. disk full
            manifestWriter->addManifestEntry( i.key(), mimetype );
        }
    }
    return true;
}

void KoShapeSavingContext::addDataCenter( KoDataCenter * dataCenter )
{
    m_dataCenter.insert( dataCenter );
}

bool KoShapeSavingContext::saveDataCenter( KoStore *store, KoXmlWriter* manifestWriter )
{
    bool ok = true;
    foreach( KoDataCenter *dataCenter, m_dataCenter )
    {
        ok = ok && dataCenter->completeSaving( store, manifestWriter );
        kDebug() << "ok" << ok;
    }
    return ok;
}
