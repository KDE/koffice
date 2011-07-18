/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus stefan.nikolaus@kdemail.net

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KC_VALIDITY_STORAGE
#define KC_VALIDITY_STORAGE

#include "KCRectStorage.h"
#include "KCValidity.h"


/**
 * \class KCValidityStorage
 * \ingroup Storage
 * \ingroup KCValue
 * Stores cell validations.
 */
class KCValidityStorage : public QObject, public KCRectStorage<KCValidity>
{
    Q_OBJECT
public:
    explicit KCValidityStorage(KCMap* map) : QObject(map), KCRectStorage<KCValidity>(map) {}
    KCValidityStorage(const KCValidityStorage& other) : QObject(other.parent()), KCRectStorage<KCValidity>(other) {}

protected Q_SLOTS:
    virtual void triggerGarbageCollection() {
        QTimer::singleShot(g_garbageCollectionTimeOut, this, SLOT(garbageCollection()));
    }
    virtual void garbageCollection() {
        KCRectStorage<KCValidity>::garbageCollection();
    }
};

#endif // KC_VALIDITY_STORAGE
