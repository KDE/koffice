/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#ifndef KODFSTORAGEDEVICE_H
#define KODFSTORAGEDEVICE_H

#include "flake_export.h"
#include "KOdfStore.h"		//krazy:exclude=includes

/**
 * This class implements a QIODevice around KOdfStore, so that
 * it can be used to create a QDomDocument from it, to be written or read
 * using QDataStream or to be written using QTextStream
 */
class FLAKE_EXPORT KOdfStorageDevice : public QIODevice
{
public:
    /// Note: KOdfStore::open() should be called before calling this.
    explicit KOdfStorageDevice(KOdfStore *store);
    ~KOdfStorageDevice();

    // Reimplemented from QIODevice
    virtual bool isSequential() const;
    // Reimplemented from QIODevice
    virtual bool open(OpenMode m);
    // Reimplemented from QIODevice
    virtual void close();
    // Reimplemented from QIODevice
    virtual qint64 size() const;
    // Reimplemented from QIODevice
    virtual qint64 pos() const;
    // Reimplemented from QIODevice
    virtual bool seek(qint64 pos);
    // Reimplemented from QIODevice
    virtual bool atEnd() const;

protected:
    // Reimplemented from QIODevice
    virtual qint64 readData(char *data, qint64 maxlen);
    // Reimplemented from QIODevice
    virtual qint64 writeData(const char *data, qint64 len);

private:
    KOdfStore *m_store;

};

#endif
