/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#ifndef KIMAGECOLLECTION_H
#define KIMAGECOLLECTION_H

#include "flake_export.h"

#include <QObject>
#include <KDataCenterBase.h>

class QImage;
class KUrl;
class KOdfStore;
class KImageData;

/**
 * An collection of KImageData objects to allow loading and saving them all together to the KOdfStore.
 * It also makes sure that if the same image is added to the collection that they share the internal data structure.
 */
class FLAKE_EXPORT KImageCollection : public QObject, public KDataCenterBase
{
    Q_OBJECT
public:
    /// constructor
    KImageCollection(QObject *parent = 0);
    virtual ~KImageCollection();

    /// reimplemented
    bool completeLoading(KOdfStore *store);

    /**
     * Save all images to the store which are in the context
     * @return returns true if save was successful (no images failed).
     */
    bool completeSaving(KOdfStore *store, KXmlWriter *manifestWriter, KShapeSavingContext *context);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KImageData will
     * share its data.
     * @param image a valid image which will be represented by the imageData.
     * @see KImageData::isValid()
     */
    KImageData *createImageData(const QImage &image);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KImageData will
     * share its data.
     * @param url a valid, local url to point to an image on the filesystem.
     * @see KImageData::isValid()
     */
    KImageData *createExternalImageData(const KUrl &url);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KImageData will
     * share its data.
     * @param localPath a valid, local path to point to an image on the filesystem.
     * @see KImageData::isValid()
     */
    KImageData *createExternalImageData(const QString &localPath);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KImageData will
     * share its data.
     * @param href the name of the image inside the store.
     * @param store the KOdfStore object.
     * @see KImageData::isValid()
     */
    KImageData *createImageData(const QString &href, KOdfStore *store);

    /**
     * Create a data object for the image data.
     * The collection will create an image data in a way that if there is an
     * existing data object with the same image the returned KImageData will
     * share its data.
     * @param imageData the bytes that represent the image in a format like png.
     * @see KImageData::isValid()
     */
    KImageData *createImageData(const QByteArray &imageData);

    void add(const KImageData &data);
    void remove(const KImageData &data);
    void removeOnKey(qint64 imageDataKey);

    bool fillFromKey(KImageData &idata, qint64 imageDataKey);

    /**
     * Get the number of images inside the collection
     */
    int size() const;
    /**
     * Get the number of images inside the collection
     */
    int count() const;

private:
    KImageData *cacheImage(KImageData *data);

    class Private;
    Private * const d;
};

#endif // KOIMAGECOLLECTION_H
