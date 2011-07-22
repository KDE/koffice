/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
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

#include "KImageData.h"
#include "KImageData_p.h"

#include "KImageCollection.h"

#include <KUnit.h>
#include <KOdfStore.h>
#include <KOdfStorageDevice.h>

#include <KDE/KDebug>
#include <KDE/KTemporaryFile>

#include <QtCore/QBuffer>
#include <QtCore/QCryptographicHash>
#include <QtGui/QPainter>

/// the maximum amount of bytes the image can be while we store it in memory instead of
/// spooling it to disk in a temp-file.
#define MAX_MEMORY_IMAGESIZE 90000

KImageData::KImageData()
    : d(0)
{
}

KImageData::KImageData(const KImageData &imageData)
    : KShapeUserData(),
    d(imageData.d)
{
    if (d)
        d->refCount.ref();
}

KImageData::KImageData(KImageDataPrivate *priv)
    : d(priv)
{
    d->refCount.ref();
}

KImageData::~KImageData()
{
    if (d && !d->refCount.deref())
        delete d;
}

QPixmap KImageData::pixmap(const QSize &size)
{
    if (!d) return QPixmap();
    QSize wantedSize = size;
    if (! wantedSize.isValid()) {
        if (d->pixmap.isNull()) // we have a problem, Houston..
            wantedSize = QSize(100, 100);
        else
            wantedSize = d->pixmap.size();
    }
    if (d->pixmap.isNull() || d->pixmap.size() != wantedSize) {
        switch (d->dataStoreState) {
        case KImageDataPrivate::StateEmpty: {
            d->pixmap = QPixmap(1, 1);
            QPainter p(&d->pixmap);
            p.setPen(QPen(Qt::gray));
            p.drawPoint(0, 0);
            p.end();
            break;
        }
        case KImageDataPrivate::StateNotLoaded:
            image(); // forces load
            // fall through
        case KImageDataPrivate::StateImageLoaded:
        case KImageDataPrivate::StateImageOnly:
            if (!d->image.isNull()) {
                // create pixmap from image.
                // this is the highest quality and lowest memory usage way of doing the conversion.
                d->pixmap = QPixmap::fromImage(d->image.scaled(wantedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
        }

        if (d->dataStoreState == KImageDataPrivate::StateImageLoaded) {
            if (d->cleanCacheTimer.isActive())
                d->cleanCacheTimer.stop();
            // schedule an auto-unload of the big QImage in a second.
            d->cleanCacheTimer.start();
        }
    }
    return d->pixmap;
}

bool KImageData::hasCachedPixmap() const
{
    return d && !d->pixmap.isNull();
}

QSizeF KImageData::imageSize()
{
    if (!d->imageSize.isValid()) {
        // The imagesize have not yet been calculated
        if (image().isNull()) // auto loads the image
            return QSizeF(100, 100);

        if (d->image.dotsPerMeterX())
            d->imageSize.setWidth(DM_TO_POINT(d->image.width() / (qreal) d->image.dotsPerMeterX() * 10.0));
        else
            d->imageSize.setWidth(d->image.width() / 72.0);

        if (d->image.dotsPerMeterY())
            d->imageSize.setHeight(DM_TO_POINT(d->image.height() / (qreal) d->image.dotsPerMeterY() * 10.0));
        else
            d->imageSize.setHeight(d->image.height() / 72.0);
    }
    return d->imageSize;
}

QImage KImageData::image() const
{
    if (d->dataStoreState == KImageDataPrivate::StateNotLoaded) {
        // load image
        if (d->temporaryFile) {
            d->temporaryFile->open();
            if (d->errorCode == Success && !d->image.load(d->temporaryFile, 0))
                d->errorCode = OpenFailed;
            d->temporaryFile->close();
        } else {
            if (d->errorCode == Success && !d->image.load(d->imageLocation.toLocalFile()))
                d->errorCode = OpenFailed;
        }
        if (d->errorCode == Success)
            d->dataStoreState = KImageDataPrivate::StateImageLoaded;
    }
    return d->image;
}

bool KImageData::hasCachedImage() const
{
    return d && !d->image.isNull();
}

void KImageData::setImage(const QImage &image, KImageCollection *collection)
{
    Q_ASSERT(!image.isNull());
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KImageData *other = collection->createImageData(image);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KImageDataPrivate(this);
            d->refCount.ref();
        }
        delete d->temporaryFile;
        d->clear();
        d->suffix = "png"; // good default for non-lossy storage.
        if (image.byteCount() > MAX_MEMORY_IMAGESIZE) {
            // store image
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            if (!image.save(&buffer, d->suffix.toLatin1())) {
                kWarning(30006) << "Write temporary file failed";
                d->errorCode = StorageFailed;
                delete d->temporaryFile;
                return;
            }
            buffer.close();
            buffer.open(QIODevice::ReadOnly);
            d->copyToTemporary(buffer);
        } else {
            d->image = image;
            d->dataStoreState = KImageDataPrivate::StateImageOnly;

            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG"); // use .png for images we get as QImage
            QCryptographicHash md5(QCryptographicHash::Md5);
            md5.addData(ba);
            d->key = KImageDataPrivate::generateKey(md5.result());
        }
    }
}

void KImageData::setExternalImage(const QString &location, KImageCollection *collection)
{
    setExternalImage(KUrl::fromUserInput(location), collection);
}

void KImageData::setExternalImage(const KUrl &location, KImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KImageData *other = collection->createExternalImageData(location);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KImageDataPrivate(this);
            d->refCount.ref();
        } else {
            d->clear();
        }
        d->imageLocation = location;
        d->setSuffix(location.toEncoded());
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(location.toEncoded());
        d->key = KImageDataPrivate::generateKey(md5.result());
        d->dataStoreState = KImageDataPrivate::StateNotLoaded;
    }
}

void KImageData::setImage(const QString &url, KOdfStore *store, KImageCollection *collection)
{
    if (collection) {
        // Let the collection first check if it already has one. If it
        // doesn't it'll call this method again and we'll go to the
        // other clause.
        KImageData *other = collection->createImageData(url, store);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KImageDataPrivate(this);
            d->refCount.ref();
        } else {
            d->clear();
        }
        d->setSuffix(url);

        if (store->open(url)) {
            struct Finalizer {
                ~Finalizer() { store->close(); }
                KOdfStore *store;
            };
            Finalizer closer;
            closer.store = store;
            KOdfStorageDevice device(store);
            const bool lossy =url.toLower().endsWith(".jpg");
            if (!lossy && device.size() < MAX_MEMORY_IMAGESIZE) {
                QByteArray data = device.readAll();
                if (d->image.loadFromData(data)) {
                    QCryptographicHash md5(QCryptographicHash::Md5);
                    md5.addData(data);
                    d->key = KImageDataPrivate::generateKey(md5.result());
                    d->dataStoreState = KImageDataPrivate::StateImageOnly;
                    return;
                }
            }
            if (!device.open(QIODevice::ReadOnly)) {
                kWarning(30006) << "open file from store " << url << "failed";
                d->errorCode = OpenFailed;
                return;
            }
            d->copyToTemporary(device);
        } else {
            kWarning(30006) << "Find file in store " << url << "failed";
            d->errorCode = OpenFailed;
            return;
        }
    }
}

void KImageData::setImage(const QByteArray &imageData, KImageCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        KImageData *other = collection->createImageData(imageData);
        this->operator=(*other);
        delete other;
    } else {
        if (d == 0) {
            d = new KImageDataPrivate(this);
            d->refCount.ref();
        }
        delete d->temporaryFile;
        d->clear();
        d->suffix = "png"; // good default for non-lossy storage.
        if (imageData.size() <= MAX_MEMORY_IMAGESIZE) {
            QImage image;
            if (!image.loadFromData(imageData)) {
                // mark the image as invalid, but keep the data in memory
                // even if KOffice cannot handle the format, the data should
                // be retained
                d->errorCode = OpenFailed;
            }
            d->image = image;
            d->dataStoreState = KImageDataPrivate::StateImageOnly;
        }
        if (imageData.size() > MAX_MEMORY_IMAGESIZE
                || d->errorCode == OpenFailed) {
            d->image = QImage();
            // store image data
            QBuffer buffer;
            buffer.setData(imageData);
            buffer.open(QIODevice::ReadOnly);
            d->copyToTemporary(buffer);
        }
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(imageData);
        d->key = KImageDataPrivate::generateKey(md5.result());
    }
}

bool KImageData::isValid() const
{
    return d && d->dataStoreState != KImageDataPrivate::StateEmpty
        && d->errorCode == Success;
}

bool KImageData::operator==(const KImageData &other) const
{
    return other.d == d;
}

KImageData &KImageData::operator=(const KImageData &other)
{
    if (other.d)
        other.d->refCount.ref();
    if (d && !d->refCount.deref())
        delete d;
    d = other.d;
    return *this;
}

qint64 KImageData::key() const
{
    return d->key;
}

QString KImageData::suffix() const
{
    return d->suffix;
}

KImageData::ErrorCode KImageData::errorCode() const
{
    return d->errorCode;
}

bool KImageData::saveData(QIODevice &device)
{
    return d->saveData(device);
}

#include <KImageData.moc>
