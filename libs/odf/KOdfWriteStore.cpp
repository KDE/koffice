/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KOdfWriteStore.h"

#include <QBuffer>

#include <ktemporaryfile.h>
#include <kdebug.h>
#include <klocale.h>

#include <KOdfStore.h>
#include <KOdfStorageDevice.h>
#include <KoXmlWriter.h>

#include "KOdfXmlNS.h"

struct KOdfWriteStore::Private {
    Private(KOdfStore * store)
            : store(store)
            , storeDevice(0)
            , contentWriter(0)
            , bodyWriter(0)
            , manifestWriter(0)
            , contentTmpFile(0) {}


    ~Private() {
        // If all the right close methods were called, nothing should remain,
        // so those deletes are really just in case.
        Q_ASSERT(!contentWriter);
        delete contentWriter;
        Q_ASSERT(!bodyWriter);
        delete bodyWriter;
        Q_ASSERT(!storeDevice);
        delete storeDevice;
        Q_ASSERT(!manifestWriter);
        delete manifestWriter;
    }

    KOdfStore * store;
    KOdfStorageDevice * storeDevice;
    KoXmlWriter * contentWriter;

    KoXmlWriter * bodyWriter;
    KoXmlWriter * manifestWriter;
    KTemporaryFile * contentTmpFile;
};

KOdfWriteStore::KOdfWriteStore(KOdfStore* store)
        : d(new Private(store))
{
}

KOdfWriteStore::~KOdfWriteStore()
{
    delete d;
}

KoXmlWriter* KOdfWriteStore::createOasisXmlWriter(QIODevice* dev, const char* rootElementName)
{
    KoXmlWriter* writer = new KoXmlWriter(dev);
    writer->startDocument(rootElementName);
    writer->startElement(rootElementName);

    if (qstrcmp(rootElementName, "VL:version-list") == 0) {
        writer->addAttribute("xmlns:VL", KOdfXmlNS::VL);
        writer->addAttribute("xmlns:dc", KOdfXmlNS::dc);
        return writer;
    }

    writer->addAttribute("xmlns:office", KOdfXmlNS::office);
    writer->addAttribute("xmlns:meta", KOdfXmlNS::meta);

    if (qstrcmp(rootElementName, "office:document-meta") != 0) {
        writer->addAttribute("xmlns:config", KOdfXmlNS::config);
        writer->addAttribute("xmlns:text", KOdfXmlNS::text);
        writer->addAttribute("xmlns:table", KOdfXmlNS::table);
        writer->addAttribute("xmlns:draw", KOdfXmlNS::draw);
        writer->addAttribute("xmlns:presentation", KOdfXmlNS::presentation);
        writer->addAttribute("xmlns:dr3d", KOdfXmlNS::dr3d);
        writer->addAttribute("xmlns:chart", KOdfXmlNS::chart);
        writer->addAttribute("xmlns:form", KOdfXmlNS::form);
        writer->addAttribute("xmlns:script", KOdfXmlNS::script);
        writer->addAttribute("xmlns:style", KOdfXmlNS::style);
        writer->addAttribute("xmlns:number", KOdfXmlNS::number);
        writer->addAttribute("xmlns:math", KOdfXmlNS::math);
        writer->addAttribute("xmlns:svg", KOdfXmlNS::svg);
        writer->addAttribute("xmlns:fo", KOdfXmlNS::fo);
        writer->addAttribute("xmlns:anim", KOdfXmlNS::anim);
        writer->addAttribute("xmlns:smil", KOdfXmlNS::smil);
        writer->addAttribute("xmlns:koffice", KOdfXmlNS::koffice);
        writer->addAttribute("xmlns:officeooo", KOdfXmlNS::officeooo);
        writer->addAttribute("xmlns:delta", KOdfXmlNS::delta);
        writer->addAttribute("xmlns:split", KOdfXmlNS::split);
        writer->addAttribute("xmlns:ac", KOdfXmlNS::ac);
    }
    writer->addAttribute("office:version", "1.2");

    writer->addAttribute("xmlns:dc", KOdfXmlNS::dc);
    writer->addAttribute("xmlns:xlink", KOdfXmlNS::xlink);
    return writer;
}

KOdfStore* KOdfWriteStore::store() const
{
    return d->store;
}

KoXmlWriter* KOdfWriteStore::contentWriter()
{
    if (!d->contentWriter) {
        if (!d->store->open("content.xml")) {
            return 0;
        }
        d->storeDevice = new KOdfStorageDevice(d->store);
        d->contentWriter = createOasisXmlWriter(d->storeDevice, "office:document-content");
    }
    return d->contentWriter;
}

KoXmlWriter* KOdfWriteStore::bodyWriter()
{
    if (!d->bodyWriter) {
        Q_ASSERT(!d->contentTmpFile);
        d->contentTmpFile = new KTemporaryFile;
        d->contentTmpFile->open();
        d->bodyWriter = new KoXmlWriter(d->contentTmpFile, 1);
    }
    return d->bodyWriter;
}

bool KOdfWriteStore::closeContentWriter()
{
    Q_ASSERT(d->contentWriter);
    Q_ASSERT(d->bodyWriter);
    Q_ASSERT(d->contentTmpFile);

    delete d->bodyWriter; d->bodyWriter = 0;

    // copy over the contents from the tempfile to the real one
    d->contentTmpFile->close();
    d->contentWriter->addCompleteElement(d->contentTmpFile);
    d->contentTmpFile->close();
    delete d->contentTmpFile; d->contentTmpFile = 0;

    d->contentWriter->endElement(); // document-content
    d->contentWriter->endDocument();

    delete d->contentWriter; d->contentWriter = 0;
    delete d->storeDevice; d->storeDevice = 0;
    if (!d->store->close()) {   // done with content.xml
        return false;
    }
    return true;
}

KoXmlWriter* KOdfWriteStore::manifestWriter(const char* mimeType)
{
    if (!d->manifestWriter) {
        // the pointer to the buffer is already stored in the KoXmlWriter, no need to store it here as well
        QBuffer *manifestBuffer = new QBuffer;
        manifestBuffer->open(QIODevice::WriteOnly);
        d->manifestWriter = new KoXmlWriter(manifestBuffer);
        d->manifestWriter->startDocument("manifest:manifest");
        d->manifestWriter->startElement("manifest:manifest");
        d->manifestWriter->addAttribute("xmlns:manifest", KOdfXmlNS::manifest);
        d->manifestWriter->addAttribute("manifest:version", "1.2");
        d->manifestWriter->addManifestEntry("/", mimeType);
    }
    return d->manifestWriter;
}

KoXmlWriter* KOdfWriteStore::manifestWriter()
{
    Q_ASSERT(d->manifestWriter);
    return d->manifestWriter;
}

bool KOdfWriteStore::closeManifestWriter()
{
    Q_ASSERT(d->manifestWriter);
    d->manifestWriter->endElement();
    d->manifestWriter->endDocument();
    QBuffer* buffer = static_cast<QBuffer *>(d->manifestWriter->device());
    delete d->manifestWriter; d->manifestWriter = 0;
    bool ok = false;
    if (d->store->open("META-INF/manifest.xml")) {
        qint64 written = d->store->write(buffer->buffer());
        ok = (written == (qint64) buffer->buffer().size() && d->store->close());
    }
    delete buffer;
    return ok;
}
