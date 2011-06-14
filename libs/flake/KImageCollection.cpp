/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#include "KImageCollection.h"
#include "KImageData.h"
#include "KImageData_p.h"
#include "KoShapeSavingContext.h"

#include <KOdfStorageDevice.h>
#include <QCryptographicHash>
#include <KXmlWriter.h>

#include <QMap>
#include <kdebug.h>
#include <kmimetype.h>

class KImageCollection::Private
{
public:
    ~Private()
    {
        foreach(KoImageDataPrivate *id, images)
            id->collection = 0;
    }

    QMap<qint64, KoImageDataPrivate*> images;
    // an extra map to find all dataObjects based on the key of a store.
    QMap<QByteArray, KoImageDataPrivate*> storeImages;
};

KImageCollection::KImageCollection(QObject *parent)
    : QObject(parent),
    d(new Private())
{
}

KImageCollection::~KImageCollection()
{
    delete d;
}

bool KImageCollection::completeLoading(KOdfStore *store)
{
    Q_UNUSED(store);
    d->storeImages.clear();
    return true;
}

bool KImageCollection::completeSaving(KOdfStore *store, KXmlWriter *manifestWriter, KoShapeSavingContext *context)
{
    QMap<qint64, QString> images(context->imagesToSave());
    QMap<qint64, QString>::iterator it(images.begin());

    QMap<qint64, KoImageDataPrivate *>::iterator dataIt(d->images.begin());

    while (it != images.end()) {
        if (dataIt == d->images.end()) {
            // this should not happen
            kWarning(30006) << "image not found";
            Q_ASSERT(0);
            break;
        }
        else if (dataIt.key() == it.key()) {
            KoImageDataPrivate *imageData = dataIt.value();
            if (imageData->imageLocation.isValid()) {
                // TODO store url
                Q_ASSERT(0); // not impleented yet
            }
            else if (store->open(it.value())) {
                KOdfStorageDevice device(store);
                bool ok = imageData->saveData(device);
                store->close();
                // TODO error handling
                if (ok) {
                    const QString mimetype(KMimeType::findByPath(it.value(), 0 , true)->name());
                    manifestWriter->addManifestEntry(it.value(), mimetype);
                } else {
                    kWarning(30006) << "saving image failed";
                }
            } else {
                kWarning(30006) << "saving image failed: open store failed";
            }
            ++dataIt;
            ++it;
        } else if (dataIt.key() < it.key()) {
            ++dataIt;
        } else {
            // this should not happen
            kWarning(30006) << "image not found";
            Q_ASSERT(0);
        }
    }
    return true;
}

KImageData *KImageCollection::createImageData(const QImage &image)
{
    Q_ASSERT(!image.isNull());
    KImageData *data = new KImageData();
    data->setImage(image);

    data = cacheImage(data);
    return data;
}

KImageData *KImageCollection::createExternalImageData(const QString &localPath)
{
    return createExternalImageData(QUrl::fromUserInput(localPath));
}

KImageData *KImageCollection::createExternalImageData(const QUrl &url)
{
    Q_ASSERT(!url.isEmpty() && url.isValid());

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(url.toEncoded());
    qint64 key = KoImageDataPrivate::generateKey(md5.result());
    if (d->images.contains(key))
        return new KImageData(d->images.value(key));
    KImageData *data = new KImageData();
    data->setExternalImage(url);
    data->priv()->collection = this;
    Q_ASSERT(data->key() == key);
    d->images.insert(key, data->priv());
    return data;
}

KImageData *KImageCollection::createImageData(const QString &href, KOdfStore *store)
{
    // the tricky thing with a 'store' is that we need to read the data now
    // as the store will no longer be readable after the loading completed.
    //
    // The solution we use is to read the data, store it in a KTemporaryFile
    // and read and parse it on demand when the image data is actually needed.
    // This leads to having two keys, one for the store and one for the
    // actual image data. We need the latter so if someone else gets the same
    // image data he can find this data and share (insert warm fuzzy feeling here).
    //
    QByteArray storeKey = (QString::number((qint64) store) + href).toLatin1();
    if (d->storeImages.contains(storeKey))
        return new KImageData(d->storeImages.value(storeKey));

    KImageData *data = new KImageData();
    data->setImage(href, store);

    data = cacheImage(data);
    d->storeImages.insert(storeKey, data->priv());
    return data;
}

KImageData *KImageCollection::createImageData(const QByteArray &imageData)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(imageData);
    qint64 key = KoImageDataPrivate::generateKey(md5.result());
    if (d->images.contains(key))
        return new KImageData(d->images.value(key));
    KImageData *data = new KImageData();
    data->setImage(imageData);
    data->priv()->collection = this;
    Q_ASSERT(data->key() == key);
    d->images.insert(key, data->priv());
    return data;
}

KImageData *KImageCollection::cacheImage(KImageData *data)
{
    QMap<qint64, KoImageDataPrivate*>::const_iterator it(d->images.constFind(data->key()));
    if (it == d->images.constEnd()) {
        d->images.insert(data->key(), data->priv());
        data->priv()->collection = this;
    }
    else {
        delete data;
        data = new KImageData(it.value());
    }
    return data;
}

bool KImageCollection::fillFromKey(KImageData &idata, qint64 key)
{
    if (d->images.contains(key)) {
        idata = KImageData(d->images.value(key));
        return true;
    }
    return false;
}

int KImageCollection::size() const
{
    return d->images.count();
}

int KImageCollection::count() const
{
    return d->images.count();
}

void KImageCollection::removeOnKey(qint64 imageDataKey)
{
    d->images.remove(imageDataKey);
}
