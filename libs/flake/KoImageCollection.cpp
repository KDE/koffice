/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoImageCollection.h"
#include "KoImageData.h"
#include "KoImageData_p.h"
#include "KoShapeSavingContext.h"

#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <QMap>
#include <kdebug.h>
#include <kmimetype.h>

class KoImageCollection::Private
{
public:
    QMap<QByteArray, KoImageDataPrivate *> images;
    ~Private()
    {
        foreach(KoImageDataPrivate * image, images)
            image->collection = 0;
    }
};

KoImageCollection::KoImageCollection()
: d(new Private())
{
}

KoImageCollection::~KoImageCollection()
{
    delete d;
}

bool KoImageCollection::completeLoading(KoStore *store)
{
    Q_UNUSED( store );
    return true;
}

bool KoImageCollection::completeSaving(KoStore *store, KoXmlWriter * manifestWriter, KoShapeSavingContext * context)
{
    QMap<QByteArray, QString> images(context->imagesToSave());
    QMap<QByteArray, QString>::iterator it(images.begin());

    QMap<QByteArray, KoImageDataPrivate *>::iterator dataIt(d->images.begin());

    while (it != images.end()) {
        if ( dataIt == d->images.end() ) {
            // this should not happen
            kWarning(30006) << "image not found";
            Q_ASSERT(0);
            break;
        }
        else if ( dataIt.key() == it.key() ) {
            if (store->open(it.value())) {
                KoStoreDevice device(store);
                bool ok = dataIt.value()->saveToFile(device);
                store->close();
                // TODO error handling
                if ( ok ) {
                    const QString mimetype(KMimeType::findByPath(it.value(), 0 , true)->name());
                    manifestWriter->addManifestEntry(it.value(), mimetype);
                }
                else {
                    kWarning(30006) << "saving image failed";
                }
            }
            else {
                kWarning(30006) << "saving image failed: open store failed";
            }
            ++dataIt;
            ++it;
        }
        else if ( dataIt.key() < it.key() ) {
            ++dataIt;
        }
        else {
            // this should not happen
            kWarning(30006) << "image not found";
            Q_ASSERT(0);
        }
    }

    return true;
}

KoImageData * KoImageCollection::getImage(const QImage & image)
{
    KoImageData * data = new KoImageData(this, image);
    lookup( data );
    return data;
}

KoImageData * KoImageCollection::getImage(const KUrl & url)
{
    KoImageData * data = new KoImageData(this, url);
    if ( data->errorCode() == KoImageData::Success ) {
        lookup( data );
    }
    else {
        data = 0;
    }
    return data;
}

KoImageData * KoImageCollection::getImage(const QString & href, KoStore * store)
{
    KoImageData * data = 0;
    if (KUrl::isRelativeUrl(href)) {
        data = new KoImageData(this, href, store);
        if ( data->errorCode() == KoImageData::Success ) {
            lookup( data );
        }
        else {
            data = 0;
        }
    }
    else {
        data = getImage(KUrl(href));
    }
    return data;
}

int KoImageCollection::size() const
{
    return d->images.size();
}

void KoImageCollection::lookup( KoImageData * image )
{
    KoImageDataPrivate * found = d->images.value(image->key(), 0);
    if (found != 0) {
        image->d = found;
    }
    else {
        d->images.insert(image->key(), image->d.data());
    }
}

void KoImageCollection::removeImage(KoImageDataPrivate * image)
{
    QMap<QByteArray, KoImageDataPrivate *>::iterator it( d->images.find(image->key) );
    if ( it != d->images.end() ) {
        if ( it.value() == image ) {
            d->images.erase(it);
        }
    }
}
