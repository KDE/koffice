/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOdfReadStore.h"

#include <kdebug.h>
#include <klocale.h>

#include <KoStore.h>
#include <KoXmlReader.h>

#include "KoOdfStylesReader.h"
#include "KoXmlNS.h"

struct KoOdfReadStore::Private {
    Private(KoStore * store)
            : store(store) {}

    KoStore * store;
    KoOdfStylesReader stylesReader;
    // it is needed to keep the stylesDoc around so that you can access the styles
    KoXmlDocument stylesDoc;
    KoXmlDocument contentDoc;
    KoXmlDocument settingsDoc;
};

KoOdfReadStore::KoOdfReadStore(KoStore* store)
        : d(new Private(store))
{
}

KoOdfReadStore::~KoOdfReadStore()
{
    delete d;
}

void KoOdfReadStore::setupXmlReader(QXmlSimpleReader& reader, bool namespaceProcessing)
{
    if (namespaceProcessing) {
        reader.setFeature("http://xml.org/sax/features/namespaces", true);
        reader.setFeature("http://xml.org/sax/features/namespace-prefixes", false);
    } else {
        reader.setFeature("http://xml.org/sax/features/namespaces", false);
        reader.setFeature("http://xml.org/sax/features/namespace-prefixes", true);
    }
    reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", true);
}

KoStore * KoOdfReadStore::store() const
{
    return d->store;
}

KoOdfStylesReader & KoOdfReadStore::styles()
{
    return d->stylesReader;
}

const KoXmlDocument & KoOdfReadStore::contentDoc() const
{
    return d->contentDoc;
}

const KoXmlDocument & KoOdfReadStore::settingsDoc() const
{
    return d->settingsDoc;
}

bool KoOdfReadStore::loadAndParse(QString & errorMessage)
{
    if (!loadAndParse("content.xml", d->contentDoc, errorMessage)) {
        return false;
    }

    loadAndParse("styles.xml", d->stylesDoc, errorMessage);
    // Load styles from style.xml
    d->stylesReader.createStyleMap(d->stylesDoc, true);
    // Also load styles from content.xml
    d->stylesReader.createStyleMap(d->contentDoc, false);

    // TODO post 1.4, pass manifestDoc to the apps so that they don't have to do it themselves
    // (when calling KoDocumentChild::loadOasisDocument)
    //QDomDocument manifestDoc;
    //KoOdfReadStore oasisStore( store );
    //if ( !oasisStore.loadAndParse( "tar:/META-INF/manifest.xml", manifestDoc, d->lastErrorMessage ) )
    //    return false;

    if (d->store->hasFile("settings.xml")) {
        loadAndParse("settings.xml", d->settingsDoc, errorMessage);
    }
    return true;
}

bool KoOdfReadStore::loadAndParse(const QString& fileName, KoXmlDocument& doc, QString& errorMessage)
{
    //kDebug(30003) <<"loadAndParse: Trying to open" << fileName;
    if (!d->store) {
        kWarning(30003) << "No store backend";
        errorMessage = i18n("No store backend");
        return false;
    }

    if (!d->store->open(fileName)) {
        kDebug(30003) << "Entry " << fileName << " not found!"; // not a warning as embedded stores don't have to have all files
        errorMessage = i18n("Could not find %1", fileName);
        return false;
    }

    bool ok = loadAndParse(d->store->device(), doc, errorMessage, fileName);
    d->store->close();
    return ok;
}

bool KoOdfReadStore::loadAndParse(QIODevice* fileDevice, KoXmlDocument& doc, QString& errorMessage, const QString& fileName)
{
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;

    // We need to be able to see the space in <text:span> </text:span>, this is why
    // we activate the "report-whitespace-only-CharData" feature.
    // Unfortunately this leads to lots of whitespace text nodes in between real
    // elements in the rest of the document, watch out for that.
    QXmlInputSource source(fileDevice);
    // Copied from QDomDocumentPrivate::setContent, to change the whitespace thing
    QXmlSimpleReader reader;
    setupXmlReader(reader, true /*namespaceProcessing*/);

    bool ok = doc.setContent(&source, &reader, &errorMsg, &errorLine, &errorColumn);
    if (!ok) {
        kError(30003) << "Parsing error in " << fileName << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        errorMessage = i18n("Parsing error in the main document at line %1, column %2\nError message: %3"
                            , errorLine , errorColumn , i18n("QXml", errorMsg));
    } else {
        kDebug(30003) << "File" << fileName << " loaded and parsed";
    }
    return ok;
}

QString KoOdfReadStore::mimeForPath(const KoXmlDocument& doc, const QString& fullPath)
{
    KoXmlElement docElem = doc.documentElement();
    KoXmlElement elem;
    forEachElement(elem, docElem) {
        if (elem.localName() == "file-entry" && elem.namespaceURI() == KoXmlNS::manifest) {
            if (elem.attributeNS(KoXmlNS::manifest, "full-path", QString()) == fullPath)
                return elem.attributeNS(KoXmlNS::manifest, "media-type", QString());
        }
    }
    return QString();
}
