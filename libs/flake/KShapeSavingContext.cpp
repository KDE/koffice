/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "KShapeSavingContext.h"
#include "KDataCenterBase.h"
#include "KShapeConnection.h"
#include "KShapeLayer.h"
#include "KImageData.h"

#include <KXmlWriter.h>
#include <KOdfStore.h>
#include <KOdfStorageDevice.h>

#include <kmimetype.h>
#include <kdebug.h>
#include <QMap>

class KShapeSavingContextPrivate {
public:
    KShapeSavingContextPrivate(KXmlWriter&, KOdfGenericStyles&, KOdfEmbeddedDocumentSaver&);

    KXmlWriter *xmlWriter;
    KShapeSavingContext::ShapeSavingOptions savingOptions;
    QMap<const KShape *, QString> drawIds;
    QMap<const QTextBlockUserData*, QString> subIds;
    QList<KShapeLayer*> layers;
    QSet<KDataCenterBase *> dataCenter;
    int drawId;
    int subId;
    QMap<QString, KoSharedSavingData*> sharedData;
    QMap<qint64, QString> imageNames;
    int imageId;
    QMap<QString, QImage> images;
    QHash<const KShape *, QTransform> shapeOffsets;
    QSet<KShapeConnection*> unsavedConnections;

    KOdfGenericStyles& mainStyles;
    KOdfEmbeddedDocumentSaver& embeddedSaver;
};

KShapeSavingContextPrivate::KShapeSavingContextPrivate(KXmlWriter &w,
        KOdfGenericStyles &s, KOdfEmbeddedDocumentSaver &e)
        : xmlWriter(&w),
        savingOptions(0),
        drawId(0),
        subId(0),
        imageId(0),
        mainStyles(s),
        embeddedSaver(e)
{
}

KShapeSavingContext::KShapeSavingContext(KXmlWriter &xmlWriter, KOdfGenericStyles &mainStyles,
        KOdfEmbeddedDocumentSaver &embeddedSaver)
    : d(new KShapeSavingContextPrivate(xmlWriter, mainStyles, embeddedSaver))
{
    // by default allow saving of draw:id
    addOption(KShapeSavingContext::DrawId);
}

KShapeSavingContext::~KShapeSavingContext()
{
}

KXmlWriter & KShapeSavingContext::xmlWriter()
{
    return *d->xmlWriter;
}

void KShapeSavingContext::setXmlWriter(KXmlWriter &xmlWriter)
{
    d->xmlWriter = &xmlWriter;
}

KOdfGenericStyles & KShapeSavingContext::mainStyles()
{
    return d->mainStyles;
}

KOdfEmbeddedDocumentSaver &KShapeSavingContext::embeddedSaver()
{
    return d->embeddedSaver;
}

bool KShapeSavingContext::isSet(ShapeSavingOption option) const
{
    return d->savingOptions & option;
}

void KShapeSavingContext::setOptions(ShapeSavingOptions options)
{
    d->savingOptions = options;
}

KShapeSavingContext::ShapeSavingOptions KShapeSavingContext::options() const
{
    return d->savingOptions;
}

void KShapeSavingContext::addOption(ShapeSavingOption option)
{
    d->savingOptions = d->savingOptions | option;
}

void KShapeSavingContext::removeOption(ShapeSavingOption option)
{
    if (isSet(option))
        d->savingOptions = d->savingOptions ^ option; // xor to remove it.
}

QString KShapeSavingContext::drawId(const KShape *shape, bool insert)
{
    QMap<const KShape *, QString>::iterator it(d->drawIds.find(shape));
    if (it == d->drawIds.end()) {
        if (insert == true) {
            it = d->drawIds.insert(shape, QString("shape%1").arg(++d->drawId));
        } else {
            return QString();
        }
    }
    return it.value();
}

void KShapeSavingContext::clearDrawIds()
{
    d->drawIds.clear();
    d->drawId = 0;
}

QString KShapeSavingContext::subId(const QTextBlockUserData *subItem, bool insert)
{
    QMap<const QTextBlockUserData*, QString>::iterator it(d->subIds.find(subItem));
    if (it == d->subIds.end()) {
        if (insert == true) {
            it = d->subIds.insert(subItem, QString("subitem%1").arg(++d->subId));
        } else {
            return QString();
        }
    }
    return it.value();
}

void KShapeSavingContext::addLayerForSaving(KShapeLayer *layer)
{
    if (layer && ! d->layers.contains(layer))
        d->layers.append(layer);
}

void KShapeSavingContext::saveLayerSet(KXmlWriter &xmlWriter)
{
    int unnamed = 0;
    QSet<QString> names;
    foreach(KShapeLayer *layer, d->layers)
        names.insert(layer->name());

    xmlWriter.startElement("draw:layer-set");
    foreach(KShapeLayer *layer, d->layers) {
        xmlWriter.startElement("draw:layer");
        if (layer->name().isEmpty()) {
            while (true) {
                QString newName = QString("Unnamed Layer %1").arg(++unnamed);
                if (!names.contains(newName)) {
                    layer->setName(newName);
                    break;
                }
            }
        }
        xmlWriter.addAttribute("draw:name", layer->name());
        if (layer->isGeometryProtected())
            xmlWriter.addAttribute("draw:protected", "true");
        if (! layer->isVisible())
            xmlWriter.addAttribute("draw:display", "none");
        xmlWriter.endElement();  // draw:layer
    }
    xmlWriter.endElement();  // draw:layer-set
}

void KShapeSavingContext::clearLayers()
{
    d->layers.clear();
}

QString KShapeSavingContext::imageHref(KImageData * image)
{
    QMap<qint64, QString>::iterator it(d->imageNames.find(image->key()));
    if (it == d->imageNames.end()) {
        QString suffix = image->suffix();
        if (suffix.isEmpty()) {
            it = d->imageNames.insert(image->key(), QString("Pictures/image%1").arg(++d->imageId));
        }
        else {
            it = d->imageNames.insert(image->key(), QString("Pictures/image%1.%2").arg(++d->imageId).arg(suffix));
        }
    }
    return it.value();
}

QString KShapeSavingContext::imageHref(QImage &image)
{
    // TODO this can be optimized to recocnice images which have the same content
    // Also this can use quite a lot of memeory as the qimage are all kept until
    // the they are saved to the store in memory
    QString href = QString("Pictures/image%1.png").arg(++d->imageId);
    d->images.insert(href, image);
    return href;
}

QMap<qint64, QString> KShapeSavingContext::imagesToSave()
{
    return d->imageNames;
}

void KShapeSavingContext::addDataCenter(KDataCenterBase * dataCenter)
{
    d->dataCenter.insert(dataCenter);
}

bool KShapeSavingContext::saveDataCenter(KOdfStore *store, KXmlWriter* manifestWriter)
{
    bool ok = true;
    foreach(KDataCenterBase *dataCenter, d->dataCenter) {
        ok = ok && dataCenter->completeSaving(store, manifestWriter, this);
        //kDebug() << "ok" << ok;
    }
    for (QMap<QString, QImage>::iterator it(d->images.begin()); it != d->images.end(); ++it) {
        if (store->open(it.key())) {
            KOdfStorageDevice device(store);
            ok = ok && it.value().save(&device, "PNG");
            store->close();
            // TODO error handling
            if (ok) {
                const QString mimetype(KMimeType::findByPath(it.key(), 0 , true)->name());
                manifestWriter->addManifestEntry(it.key(), mimetype);
            }
            else {
                kWarning(30006) << "saving image failed";
            }
        }
        else {
            ok = false;
            kWarning(30006) << "saving image failed: open store failed";
        }
    }
    return ok;
}

void KShapeSavingContext::addSharedData(const QString &id, KoSharedSavingData * data)
{
    QMap<QString, KoSharedSavingData*>::iterator it(d->sharedData.find(id));
    // data will not be overwritten
    if (it == d->sharedData.end()) {
        d->sharedData.insert(id, data);
    } else {
        kWarning(30006) << "The id" << id << "is already registered. Data not inserted";
        Q_ASSERT(it == d->sharedData.end());
    }
}

KoSharedSavingData * KShapeSavingContext::sharedData(const QString &id) const
{
    KoSharedSavingData * data = 0;
    QMap<QString, KoSharedSavingData*>::const_iterator it(d->sharedData.constFind(id));
    if (it != d->sharedData.constEnd()) {
        data = it.value();
    }
    return data;
}

void KShapeSavingContext::addShapeOffset(const KShape *shape, const QTransform &m)
{
    d->shapeOffsets.insert(shape, m);
}

void KShapeSavingContext::removeShapeOffset(const KShape *shape)
{
    d->shapeOffsets.remove(shape);
}

QTransform KShapeSavingContext::shapeOffset(const KShape *shape) const
{
    return d->shapeOffsets.value(shape, QTransform());
}

void KShapeSavingContext::addForWriting(const QList<KShapeConnection*> &connections)
{
    foreach (KShapeConnection *c, connections)
        d->unsavedConnections.insert(c);
}

void KShapeSavingContext::writeConnectors()
{
    foreach (KShapeConnection *con, d->unsavedConnections)
        con->saveOdf(*this);
    d->unsavedConnections.clear();
}
