/***************************************************************************
  KoScriptingOdf.cpp
 * This file is part of the KDE project
 * copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptingOdf.h"

#include <QPointer>
#include <QIODevice>
#include <QBuffer>
#include <QRegExp>
#include <KoStore.h>
#include <KoOdfWriteStore.h>
#include <KoDocumentAdaptor.h>
#include <KoDocument.h>
#include <KoEmbeddedDocumentSaver.h>

#include <KDebug>

/************************************************************************************************
 * KoScriptingOdfReader
 */

/// \internal d-pointer class.
class KoScriptingOdfReader::Private
{
public:
    KoScriptingOdfStore *store;
    KoXmlDocument doc;
    KoXmlElement currentElement;
    int level;
    QString filter;
    QRegExp filterRegExp;
    Private(KoScriptingOdfStore *store, const KoXmlDocument &doc)
        : store(store),
        doc(doc),
        level(0),
        filterRegExp(false)
    {
    }
};

KoScriptingOdfReader::KoScriptingOdfReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : QObject(store),
    d(new Private(store, doc))
{
}

KoScriptingOdfReader::~KoScriptingOdfReader()
{
    delete d;
}

void KoScriptingOdfReader::start()
{
    KoXmlElement elem = d->doc.documentElement();
    handleElement(elem);
    setCurrentElement(KoXmlElement());
    setLevel(0);
}

QString KoScriptingOdfReader::nameFilter() const
{
    return d->filter;
}

void KoScriptingOdfReader::setNameFilter(const QString &name, bool regularExpression) const
{
    d->filter = name.isEmpty() ? QString() : name;
    d->filterRegExp = regularExpression ? QRegExp(name, Qt::CaseInsensitive) : QRegExp();
}

KoScriptingOdfStore *KoScriptingOdfReader::store() const
{
    return d->store;
}

KoXmlDocument KoScriptingOdfReader::doc() const
{
    return d->doc;
}

KoXmlElement KoScriptingOdfReader::currentElement() const
{
    return d->currentElement;
}

QString KoScriptingOdfReader::name() const
{
    return d->currentElement.tagName(); /*.nodeName();*/
}

QString KoScriptingOdfReader::namespaceURI() const
{
    return d->currentElement.namespaceURI();
}

int KoScriptingOdfReader::level() const
{
    return d->level;
}

#ifndef KOXML_USE_QDOM
QStringList KoScriptingOdfReader::attributeNames()
{
    return d->currentElement.attributeNames();
}
#endif

QString KoScriptingOdfReader::attribute(const QString &name, const QString &defaultValue) const
{
    return d->currentElement.attribute(name, defaultValue);
}

QString KoScriptingOdfReader::attributeNS(const QString &namespaceURI, const QString &localName, const QString &defaultValue) const
{
    return d->currentElement.attributeNS(namespaceURI, localName, defaultValue);
}

bool KoScriptingOdfReader::hasAttribute(const QString &name) const
{
    return d->currentElement.hasAttribute(name);
}

bool KoScriptingOdfReader::hasAttributeNS(const QString &namespaceURI, const QString &localName) const
{
    return d->currentElement.hasAttributeNS(namespaceURI, localName);
}

bool KoScriptingOdfReader::isNull() const
{
    return d->currentElement.isNull();
}

bool KoScriptingOdfReader::isElement() const
{
    return d->currentElement.isElement();
}

QString KoScriptingOdfReader::text() const
{
    return d->currentElement.text();
}

bool KoScriptingOdfReader::hasChildren() const {
#ifdef KOXML_USE_QDOM
    const int count = d->currentElement.childNodes().count();
#else
    const int count = d->currentElement.childNodesCount();
#endif
    if (count < 1)
        return false;
    if (count == 1 && d->currentElement.firstChild().isText())
        return false;
    return true;
}

void KoScriptingOdfReader::emitOnElement()
{
emit onElement();
}

void KoScriptingOdfReader::setCurrentElement(const KoXmlElement &elem)
{
    d->currentElement = elem;
}

void KoScriptingOdfReader::setLevel(int level)
{
    d->level = level;
}

void KoScriptingOdfReader::handleElement(KoXmlElement &elem, int level)
{
    bool ok = d->filter.isNull();
    if (! ok) {
        if (d->filterRegExp.isEmpty())
            ok = d->filter == elem.tagName();
        else
            ok = d->filterRegExp.exactMatch(elem.tagName());
    }
    if (ok) {
        setCurrentElement(elem);
        setLevel(level);
        emitOnElement();
    }
    level++;
    KoXmlElement e;
    forEachElement(e, elem)
        handleElement(e, level); // recursive
}

KoScriptingOdfManifestReader::KoScriptingOdfManifestReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : KoScriptingOdfReader(store, doc)
{
    KoXmlElement elem = doc.documentElement();
    KoXmlElement e;
    forEachElement(e, elem)
        if (e.tagName() == "manifest:file-entry")
            m_entries << QPair<QString,QString>(e.attribute("manifest:media-type"), e.attribute("manifest:full-path"));
}

QStringList KoScriptingOdfManifestReader::paths(const QString &type)
{
    QStringList list;
    for (QList<QPair<QString,QString> >::Iterator it = m_entries.begin(); it != m_entries.end(); ++it)
        if (type.isEmpty() || type == (*it).first )
            list << (*it).second;
    return list;
}

#ifndef NDEBUG
void dumpElem(KoXmlElement elem, int level=0)
{
    QString prefix;
    for (int i = 0; i < level; ++i)
        prefix+="  ";
    kDebug(32010)  << QString("%1  %2").arg(prefix).arg(elem.tagName());
#ifndef KOXML_USE_QDOM
    foreach (const QString &s, elem.attributeNames())
        kDebug(32010)  << QString("%1    %2 = %3").arg(prefix).arg(s).arg(elem.attribute(s));
#endif
    level++;
    KoXmlElement e;
    forEachElement(e, elem)
        dumpElem(e,level);
}
#endif

KoScriptingOdfStylesReader::KoScriptingOdfStylesReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : KoScriptingOdfReader(store, doc)
{
    //dumpElem( doc.documentElement() );
}

KoScriptingOdfContentReader::KoScriptingOdfContentReader(KoScriptingOdfStore *store, const KoXmlDocument &doc)
    : KoScriptingOdfReader(store, doc)
{
    //dumpElem( doc.documentElement() );
}

/************************************************************************************************
 * KoScriptingOdfStore
 */

/// \internal d-pointer class.
class KoScriptingOdfStore::Private
{
public:
    QPointer<KoDocument> document;
    QPointer<KoDocumentAdaptor> documentAdaptor;

    KoStore *readStore;
    QIODevice *readDevice;
    KoScriptingOdfReader *reader;
    QByteArray byteArray;

    explicit Private(KoDocument *doc) : document(doc), documentAdaptor(0), readStore(0), readDevice(0), reader(0) {}
    ~Private() { delete readStore; delete readDevice; delete reader; }

    KoStore *getReadStore() {
        QByteArray ba = getByteArray();
        if (ba.isNull()) {
            kWarning(32010)  << "KoScriptingOdfStore::getReadStore() Failed to fetch ByteArray";
            return 0;
        }
        if (readStore ) {
            //kDebug(32010) <<"KoScriptingOdfStore::getReadStore() Return cached store";
            Q_ASSERT(readDevice);
            return readStore;
        }
        //kDebug(32010) <<"KoScriptingOdfStore::getReadStore() Return new store";
        Q_ASSERT(!readDevice);
        readDevice = new QBuffer(&byteArray);
        readStore = KoStore::createStore(readDevice, KoStore::Read, "KrossScript", KoStore::Tar);
        return readStore;
    }

    QByteArray getByteArray() {
        if (! byteArray.isNull())
            return byteArray;
        if (readStore) {
            //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Cleaning prev cached store up.";
            if (readStore->isOpen() ) {
                //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Closing prev cached store.";
                readStore->close();
            }
            delete readStore;
            readStore = 0;
        }
        if (readDevice) {
            //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Cleaning prev cached device up.";
            delete readDevice;
            readDevice = 0;
        }
        if (! document) {
            //kWarning(32010)  << "KoScriptingOdfStore::getByteArray() No document defined.";
            return QByteArray();
        }

        //kDebug(32010)  << "KoScriptingOdfStore::getByteArray() Reading ByteArray.";
        QBuffer buffer(&byteArray);
        KoStore *store = KoStore::createStore(&buffer, KoStore::Write, "KrossScript", KoStore::Tar);
        KoOdfWriteStore odfStore(store);
        odfStore.manifestWriter("");
        KoEmbeddedDocumentSaver embeddedSaver;
        KoDocument::SavingContext documentContext(odfStore, embeddedSaver);
        QByteArray mime = getMimeType();
        if (! document->saveOdf(documentContext)) {
            kWarning(32010)  << "KoScriptingOdfStore::open() Failed to save Oasis to ByteArray";
            byteArray = QByteArray();
        }
        //odfStore.closeContentWriter();
        odfStore.closeManifestWriter();
        delete store;
        return byteArray;
    }

    QByteArray getMimeType() const {
        return "application/vnd.oasis.opendocument.text"; //odt
        //return "application/vnd.oasis.opendocument.spreadsheet"; //ods
        //return "application/vnd.oasis.opendocument.presentation"; //odp
        //return "pplication/vnd.oasis.opendocument.graphics"; //odg
        //return "application/vnd.oasis.opendocument.chart"; //odc
        //return "application/vnd.oasis.opendocument.formula"; //odf
        //return "application/vnd.oasis.opendocument.image"; //odi
    }

};

KoScriptingOdfStore::KoScriptingOdfStore(QObject *parent, KoDocument *doc)
    : QObject(parent)
    , d(new Private(doc))
{
}

KoScriptingOdfStore::~KoScriptingOdfStore()
{
    delete d;
}

//KoStore *KoScriptingOdfStore::readStore() const { return d->getReadStore(); }
//QIODevice *KoScriptingOdfStore::readDevice() const { return d->readDevice; }

bool KoScriptingOdfStore::hasFile(const QString &fileName)
{
    KoStore *store = d->getReadStore();
    return store ? store->hasFile(fileName) : false;
}

bool KoScriptingOdfStore::isOpen() const
{
    return d->readStore && d->readStore->isOpen();
}

QObject *KoScriptingOdfStore::open(const QString &fileName)
{
    delete d->reader; d->reader = 0;
    KoStore *store = d->getReadStore();
    if (! store)
        return 0;
    if (store->isOpen())
        store->close();
    if (! store->open(fileName)) {
        kWarning(32010) <<"KoScriptingOdfStore::openFile() Failed to open file:"<<fileName;
        return 0;
    }
    //kDebug(32010) <<"KoScriptingOdfStore::openFile() fileName="<<fileName<<" store->isOpen="<<store->isOpen()<<endl;
    Q_ASSERT(store->device());

    //KoOasisStore oasisStore(store);
    KoXmlDocument doc;

    QString errorMsg;
    int errorLine, errorColumn;
    if (! doc.setContent(store->device(), &errorMsg, &errorLine, &errorColumn)) {
       kWarning(32010) << "Parse-Error message" << errorMsg << "line" << errorLine << "col" << errorColumn;
       return 0;
    }

    const QString tagName = doc.documentElement().tagName();
    kDebug(32010) <<"KoScriptingOdfStore::open documentElement.tagName="<<tagName;
    if (tagName == "office:document-content")
        d->reader = new KoScriptingOdfContentReader(this, doc);
    if (tagName == "office:document-styles")
        d->reader = new KoScriptingOdfStylesReader(this, doc);
    else if (tagName == "manifest:manifest")
        d->reader = new KoScriptingOdfManifestReader(this, doc);
    else
        d->reader = new KoScriptingOdfReader(this, doc);
    return d->reader;
}

bool KoScriptingOdfStore::close()
{
    if (! d->readStore || ! d->readStore->isOpen())
        return true;
    return d->readStore->close();
}

QByteArray KoScriptingOdfStore::extract(const QString &fileName)
{
    KoStore *store = d->getReadStore();
    if (! store)
        return QByteArray();
    if (store->isOpen())
        store->close();
    QByteArray data;
    bool ok = store->extractFile(fileName, data);
    return ok ? data : QByteArray();
}

bool KoScriptingOdfStore::extractToFile(const QString &fileName, const QString &toFileName)
{
    KoStore *store = d->getReadStore();
    if (! store)
        return false;
    if (store->isOpen())
        store->close();
    return store->extractFile(fileName, toFileName);
}

QObject *KoScriptingOdfStore::document() const
{
    if (d->documentAdaptor)
        return d->documentAdaptor;
    return d->document;
}

bool KoScriptingOdfStore::setDocument(QObject *document)
{
    //d->clear();
    bool ok = true;
    d->documentAdaptor = dynamic_cast<KoDocumentAdaptor*>(document);
    if (d->documentAdaptor) {
        d->document = dynamic_cast<KoDocument*>(d->documentAdaptor->parent());
        Q_ASSERT(d->document);
    } else {
        if (KoDocument *doc = dynamic_cast<KoDocument*>(document)) {
            d->document = doc;
        } else {
            d->document = 0;
            ok = false;
        }
        d->documentAdaptor = 0;
    }
    return ok;
}

#include "KoScriptingOdf.moc"
