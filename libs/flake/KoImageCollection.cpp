/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <QList>
#include <KDebug>
#include <kmimetype.h>

class KoImageCollection::Private
{
public:
    QList<KoImageData*> images;
};

KoImageCollection::KoImageCollection()
    : d(new Private())
{
}

KoImageCollection::~KoImageCollection()
{
    delete d;
}

void KoImageCollection::addImage(KoImageData *image)
{
    d->images.append(new KoImageData(*image));
}

void KoImageCollection::removeImage(KoImageData *image)
{
    foreach(KoImageData *data, d->images) {
        if (data->operator==(*image)) {
            d->images.removeAll(data);
            delete data;
        }
    }
}

bool KoImageCollection::completeLoading(KoStore *store)
{
    foreach(KoImageData *image, d->images) {
        if (! store->open(image->storeHref())) {
            kWarning(30006) << "open image " << image->storeHref() << "failed";
            return false;
        }
        bool ok = image->loadFromFile(new KoStoreDevice(store));
        store->close();
        if (! ok) {
            kWarning(30006) << "load image " << image->storeHref() << "failed";
            return false;
        }
    }
    return true;
}

bool KoImageCollection::completeSaving(KoStore *store, KoXmlWriter * manifestWriter )
{
    foreach(KoImageData *image, d->images) {
        if (image->isTaggedForSaving())
        {
            if (! store->open(image->storeHref()))
                return false;
            bool ok = image->saveToFile(new KoStoreDevice(store));
            store->close();
            if (! ok)
                return false;
            const QString mimetype( KMimeType::findByPath( image->storeHref(), 0 ,true )->name() );
            manifestWriter->addManifestEntry( image->storeHref(), mimetype );
        }
    }
    return true;
}

