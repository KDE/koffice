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

#include "KOdfStorageDevice.h"

/// Note: KOdfStore::open() should be called before calling this.
KOdfStorageDevice::KOdfStorageDevice(KOdfStore * store) : m_store(store)
{
    // koffice-1.x behavior compat: a KOdfStorageDevice is automatically open
    setOpenMode(m_store->mode() == KOdfStore::Read ? QIODevice::ReadOnly : QIODevice::WriteOnly);
}

KOdfStorageDevice::~KOdfStorageDevice()
{
}

bool KOdfStorageDevice::isSequential() const
{
    return true;
}

bool KOdfStorageDevice::open(OpenMode m)
{
    setOpenMode(m);
    if (m & QIODevice::ReadOnly)
        return (m_store->mode() == KOdfStore::Read);
    if (m & QIODevice::WriteOnly)
        return (m_store->mode() == KOdfStore::Write);
    return false;
}

void KOdfStorageDevice::close()
{
}

qint64 KOdfStorageDevice::size() const
{
    if (m_store->mode() == KOdfStore::Read)
        return m_store->size();
    else
        return 0xffffffff;
}

qint64 KOdfStorageDevice::pos() const
{
    return m_store->pos();
}

bool KOdfStorageDevice::seek(qint64 pos)
{
    return m_store->seek(pos);
}

bool KOdfStorageDevice::atEnd() const
{
    return m_store->atEnd();
}

qint64 KOdfStorageDevice::readData(char *data, qint64 maxlen)
{
    return m_store->read(data, maxlen);
}

qint64 KOdfStorageDevice::writeData(const char *data, qint64 len)
{
    return m_store->write(data, len);
}
