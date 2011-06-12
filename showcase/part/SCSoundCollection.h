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
#ifndef KPRSOUNDCOLLECTION_H
#define KPRSOUNDCOLLECTION_H

#include <KoDataCenterBase.h>
#include <QStringList>
#include <QObject>
#include <QVariant>

#include "showcase_export.h"

class SCSoundData;
class KOdfStore;

/**
 * An collection of SCSoundData objects to allow loading and saving them all together to the KOdfStore.
 */
class SHOWCASE_EXPORT SCSoundCollection : public QObject, public KoDataCenterBase {
public:
    /// constructor
    SCSoundCollection(QObject *parent = 0);
    ~SCSoundCollection();

    /**
     * Load all sounds from the store which have a recognized SCSoundData::storeHref().
     * @return returns true if load was successful (no sounds failed).
     */
    bool completeLoading(KOdfStore *store);

    /**
     * Save all sounds to the store which are tagged for saving
     * and have a recognized SCSoundData::storeHref().
     * @return returns true if save was successful (no sounds failed).
     */
    bool completeSaving(KOdfStore *store, KoXmlWriter * manifestWriter, KoShapeSavingContext * context);

    SCSoundData *findSound(QString title);

    QStringList titles();

protected:
    friend class SCSoundData;
    void addSound(SCSoundData *image);
    void removeSound(SCSoundData *image);


private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(SCSoundCollection*)
#endif
