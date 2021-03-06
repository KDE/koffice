/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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

#include "KImageData_p.h"
#include "KImageCollection.h"

#include <KTemporaryFile>
#include <QImageWriter>
#include <QCryptographicHash>
#include <QFileInfo>
#include <KDebug>
#include <QBuffer>

KImageDataPrivate::KImageDataPrivate(KImageData *q)
    : collection(0),
    errorCode(KImageData::Success),
    key(0),
    refCount(0),
    dataStoreState(StateEmpty),
    temporaryFile(0)
{
    cleanCacheTimer.setSingleShot(true);
    cleanCacheTimer.setInterval(1000);
    QObject::connect(&cleanCacheTimer, SIGNAL(timeout()), q, SLOT(cleanupImageCache()));
}

KImageDataPrivate::~KImageDataPrivate()
{
    if (collection)
        collection->removeOnKey(key);
    delete temporaryFile;
}

// called from the collection
bool KImageDataPrivate::saveData(QIODevice &device)
{
    switch (dataStoreState) {
    case KImageDataPrivate::StateEmpty:
        return false;
    case KImageDataPrivate::StateNotLoaded:
        // spool directly.
        Q_ASSERT(temporaryFile); // otherwise the collection should not have called this
        if (temporaryFile) {
            if (!temporaryFile->open()) {
                kWarning(30006) << "Read file from temporary store failed";
                return false;
            }
            char buf[4096];
            while (true) {
                temporaryFile->waitForReadyRead(-1);
                qint64 bytes = temporaryFile->read(buf, sizeof(buf));
                if (bytes <= 0)
                    break; // done!
                do {
                    qint64 nWritten = device.write(buf, bytes);
                    if (nWritten == -1) {
                        temporaryFile->close();
                        return false;
                    }
                    bytes -= nWritten;
                } while (bytes > 0);
            }
            temporaryFile->close();
        }
        return true;
    case KImageDataPrivate::StateImageLoaded:
    case KImageDataPrivate::StateImageOnly: {
        // save image
        QBuffer buffer;
        QImageWriter writer(&buffer, suffix.toLatin1());
        bool result = writer.write(image);
        device.write(buffer.data(), buffer.size());
        return result;
      }
    }
    return false;
}

void KImageDataPrivate::setSuffix(const QString &name)
{
    QRegExp rx("\\.([^/]+$)");
    if (rx.indexIn(name) != -1) {
        suffix = rx.cap(1);
    }
}

void KImageDataPrivate::copyToTemporary(QIODevice &device)
{
    delete temporaryFile;
    temporaryFile = new KTemporaryFile();
    temporaryFile->setPrefix("KImageData");
    if (!temporaryFile->open()) {
        kWarning(30006) << "open temporary file for writing failed";
        errorCode = KImageData::StorageFailed;
        return;
    }
    QCryptographicHash md5(QCryptographicHash::Md5);
    char buf[8096];
    while (true) {
        device.waitForReadyRead(-1);
        qint64 bytes = device.read(buf, sizeof(buf));
        if (bytes <= 0)
            break; // done!
        md5.addData(buf, bytes);
        do {
            bytes -= temporaryFile->write(buf, bytes);
        } while (bytes > 0);
    }
    key = KImageDataPrivate::generateKey(md5.result());
    temporaryFile->close();

    QFileInfo fi(*temporaryFile);
    dataStoreState = StateNotLoaded;
}

void KImageDataPrivate::cleanupImageCache()
{
    if (dataStoreState == KImageDataPrivate::StateImageLoaded) {
        image = QImage();
        dataStoreState = KImageDataPrivate::StateNotLoaded;
    }
}

void KImageDataPrivate::clear()
{
    errorCode = KImageData::Success;
    dataStoreState = StateEmpty;
    imageLocation.clear();
    imageSize = QSizeF();
    key = 0;
    image = QImage();
}

qint64 KImageDataPrivate::generateKey(const QByteArray &bytes)
{
    qint64 answer = 1;
    const int max = qMin(8, bytes.count());
    for (int x = 0; x < max; ++x)
        answer += bytes[x] << (8 * x);
    return answer;
}
