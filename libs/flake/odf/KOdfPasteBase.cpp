/* This file is part of the KDE project
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

#include "KOdfPasteBase.h"

#include <QBuffer>
#include <QByteArray>
#include <QMimeData>
#include <QString>

#include <kdebug.h>

#include <KOdfStore.h>
#include <KOdfStoreReader.h>
#include <KXmlReader.h>
#include <KOdfXmlNS.h>

KOdfPasteBase::KOdfPasteBase()
{
}

KOdfPasteBase::~KOdfPasteBase()
{
}

bool KOdfPasteBase::paste(KOdf::DocumentType documentType, const QMimeData *data)
{
    QByteArray arr = data->data(KOdf::mimeType(documentType));
    return paste(documentType, arr);
}

bool KOdfPasteBase::paste(KOdf::DocumentType documentType, const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return false;

    QBuffer buffer;
    buffer.setData(bytes);
    KOdfStore * store = KOdfStore::createStore(&buffer, KOdfStore::Read);
    store->disallowNameExpansion();
    KOdfStoreReader odfStore(store);

    QString errorMessage;
    if (! odfStore.loadAndParse(errorMessage)) {
        kWarning(30002) << "loading and parsing failed:" << errorMessage;
        return false;
    }

    KXmlElement content = odfStore.contentDoc().documentElement();
    KXmlElement realBody(KoXml::namedItemNS(content, KOdfXmlNS::office, "body"));

    if (realBody.isNull()) {
        kWarning(30002) << "No body tag found";
        return false;
    }

    KXmlElement body = KoXml::namedItemNS(realBody, KOdfXmlNS::office, KOdf::bodyContentElement(documentType, false));

    if (body.isNull()) {
        kWarning(30002) << "No" << KOdf::bodyContentElement(documentType, true) << "tag found";
        return false;
    }

    return process(body, odfStore);
}
