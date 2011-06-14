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
#include "SCSoundCollection.h"
#include "SCSoundData.h"

#include <KOdfStorageDevice.h>
#include <KXmlWriter.h>

#include <QList>
#include <KDebug>
#include <kmimetype.h>

class SCSoundCollection::Private {
public:
    QList<SCSoundData*> sounds;
};

SCSoundCollection::SCSoundCollection(QObject *parent)
    : QObject(parent),
    d(new Private())
{
}

SCSoundCollection::~SCSoundCollection() {
    delete d;
}

void SCSoundCollection::addSound(SCSoundData *image) {
    d->sounds.append(new SCSoundData(*image));
}

void SCSoundCollection::removeSound(SCSoundData *image) {
    foreach (SCSoundData *data, d->sounds) {
        if(data->operator==(*image)) {
            d->sounds.removeAll(data);
            delete data;
        }
    }
}

SCSoundData *SCSoundCollection::findSound(QString title)
{
    for (int i = 0; i < d->sounds.size(); ++i) {
        if (d->sounds.at(i)->title() == title)
            return d->sounds[i];
    }
    return 0;
}

QStringList SCSoundCollection::titles()
{
    QStringList list;

    for (int i = 0; i < d->sounds.size(); ++i) {
        list << d->sounds.at(i)->title();
    }
    return list;
}

// TODO move to loading of the actual element using the sound
bool SCSoundCollection::completeLoading(KOdfStore *store)
{
    foreach (SCSoundData *sound, d->sounds) {
        if(! store->open(sound->storeHref()))
            return false;
        bool ok = sound->loadFromFile(new KOdfStorageDevice(store));
        store->close();
        if(! ok) {
            return false;
        }
    }
    return true;
}

// use a KSharedSavingData in the context to save which sounds need to be saved
bool SCSoundCollection::completeSaving(KOdfStore *store, KXmlWriter * manifestWriter, KShapeSavingContext * context)
{
    Q_UNUSED(context);
    foreach (SCSoundData *sound, d->sounds) {
        if(sound->isTaggedForSaving())
        {
            if(! store->open(sound->storeHref()))
                return false;
            bool ok = sound->saveToFile(new KOdfStorageDevice(store));
            store->close();
            if(! ok)
                return false;
            const QString mimetype(KMimeType::findByPath(sound->storeHref(), 0 ,true)->name());
            manifestWriter->addManifestEntry(sound->storeHref(), mimetype);
        }
    }
    return true;
}

