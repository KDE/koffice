/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (c) 2001 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "koClipartCollection.h"
#include <qfileinfo.h>
#include "qwmf.h"
#include <qdom.h>
#include <kdebug.h>
#include <koStore.h>
#include <koStoreDevice.h>
#include <ktempfile.h>

// Can't do that yet
//#define SAVE_IN_SVG_FORMAT

KoClipart KoClipartCollection::findClipart( const KoClipartKey &key ) const
{
    kdDebug(30003) << "KoClipartCollection::findClipart " << key.toString() << endl;
    ConstIterator it = find( key );
    if ( it == end() )
        return KoClipart();

    return *it;
}

KoClipart KoClipartCollection::insertClipart( const KoClipartKey &key, const QPicture &picture )
{
    kdDebug(30003) << "KoClipartCollection::insertClipart " << key.toString() << endl;
    KoClipart c = findClipart( key );
    if ( c.isNull() )
    {
        kdDebug(30003) << "KoClipartCollection::insertClipart not found -> inserting" << endl;
        c = KoClipart( key, picture );
        insert( key, c );
    }
    return c;
}

KoClipart KoClipartCollection::loadClipart( const QString &fileName )
{
    // Check the last modified date of the file, as it is now.
    QFileInfo inf( fileName );
    KoClipartKey key( fileName, inf.lastModified() );

    KoClipart c = findClipart( key );
    if ( c.isNull() )
    {
        kdDebug(30003) << " loading clipart from file " << fileName << endl;
        QPicture pic;
        loadFromFile( fileName, &pic );
        if ( !pic.isNull() )
            c = insertClipart( key, pic );
        else
            kdWarning(30003) << "Couldn't build WMF QPicture from " << fileName << endl;
    }
    return c;
}

//static
bool KoClipartCollection::loadFromFile( const QString& fileName, QPicture *pic )
{
    if ( fileName.lower().endsWith( "wmf" ) || fileName.lower().endsWith( "emf" ))
    {
        // WMF
        QWinMetaFile wmf;
        if ( wmf.load( fileName ) )
        {
            wmf.paint( pic );
            return true;
        }
        else
            return false;
    }
    else
    {
        // assume SVG
        return pic->load( fileName, "svg" );
    }
}

void KoClipartCollection::saveToStore( KoStore *store, QValueList<KoClipartKey> keys, const QString & prefix )
{
    int i = 0;
    QValueList<KoClipartKey>::Iterator it = keys.begin();
    for ( ; it != keys.end(); ++it )
    {
        KoClipart c = findClipart( *it );
        if ( c.isNull() )
            kdWarning(30003) << "Picture " << (*it).toString() << " not found in collection !" << endl;
        else
        {
#ifdef SAVE_IN_SVG_FORMAT
            QString format = "svg";
#else
            QString format = "wmf";
#endif
            QString storeURL = QString( "cliparts/clipart%1.%2" ).arg( ++i ).arg( format.lower() );
            storeURL.prepend( prefix );

            if ( store->open( storeURL ) ) {
		KoStoreDevice dev( store );
                QPicture * pic = c.picture();

#ifdef SAVE_IN_SVG_FORMAT
                KTempFile tmpFile;
                tmpFile.setAutoDelete( true );
                if ( pic->save( tmpFile.name(), "svg" ) )
                {
                    // Argl, it would be so much simpler if QPicture::save would take a QIODevice * !
                    QByteArray array( 1024 );
                    int n;
                    while ( ( n = tmpFile.file()->readBlock( array.data(), array.size() ) ) )
                        dev.writeBlock( array.data(), n );
                }
#else
                dev.writeBlock( pic->data(), pic->size() );
#endif
		store->close();
            }
        }
    }
}

QDomElement KoClipartCollection::saveXML( QDomDocument &doc, QValueList<KoClipartKey> keys, const QString & prefix )
{
    QDomElement cliparts = doc.createElement( "CLIPARTS" );
    int i = 0;
    QValueList<KoClipartKey>::Iterator it = keys.begin();
    for ( ; it != keys.end(); ++it )
    {
        KoClipart picture = findClipart( *it );
        if ( picture.isNull() )
            kdWarning(30003) << "Picture " << (*it).toString() << " not found in collection !" << endl;
        else
        {
#ifdef SAVE_IN_SVG_FORMAT
            QString format = "svg";
#else
            QString format = "wmf";
#endif
            QString pictureName = QString( "cliparts/clipart%1.%2" ).arg( ++i ).arg( format.lower() );
            pictureName.prepend( prefix );

            QDomElement keyElem = doc.createElement( "KEY" );
            cliparts.appendChild( keyElem );
            (*it).saveAttributes( keyElem );
            keyElem.setAttribute( "name", pictureName );
        }
    }
    return cliparts;
}

KoClipartCollection::StoreMap
KoClipartCollection::readXML( QDomElement &pixmapsElem, const QDateTime & defaultDateTime )
{
    StoreMap map;
    QDomElement keyElement = pixmapsElem.firstChild().toElement();
    while( !keyElement.isNull() )
    {
        if (keyElement.tagName()=="KEY")
        {
            KoClipartKey key;
            key.loadAttributes( keyElement, defaultDateTime.date(), defaultDateTime.time() );
            QString n = keyElement.attribute( "name" );
            map.insert( key, n );
        }
        keyElement = keyElement.nextSibling().toElement();
    }
    return map;
}

void KoClipartCollection::readFromStore( KoStore * store, const StoreMap & storeMap, const QString &prefix )
{
    StoreMap::ConstIterator it = storeMap.begin();
    for ( ; it != storeMap.end(); ++it )
    {
        QString u;
        if ( ! it.data().isEmpty() )
            u = it.data();
        else {
            // old naming I suppose ?
            u = prefix + it.key().toString();
        }

        QPicture pic;

        if ( store->open( u ) ) {
            readFromStore( store, u, &pic );
            store->close();
        } else {
            u.prepend( "file:" );
            if ( store->open( u ) ) {
                readFromStore( store, u, &pic );
                store->close();
            }
        }

        if ( !pic.isNull() )
            insertClipart( it.key(), pic );
    }
}

void KoClipartCollection::readFromStore( KoStore * store, const QString &u, QPicture * pic )
{
    KoStoreDevice dev( store );
    int size = store->size();
    char * data = new char[size];
    dev.readBlock( data, size );
    if ( u.endsWith( "svg" ) )
    {
        // Argl, why doesn't QPicture::load take a QIODevice * ?
        KTempFile tmpFile;
        tmpFile.setAutoDelete( true );
        tmpFile.file()->writeBlock( data, size );
        tmpFile.close();
        (void)pic->load( tmpFile.name(), "svg" );
    }
    else // raw QPicture data (compat for KOffice <= 1.1)
        pic->setData( data, size );
    delete data;
}
