/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

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

#include "KoQueryTrader.h"

#include "KoDocument.h"
#include "KoFilter.h"

#include <kparts/factory.h>

#include <kservicetype.h>
#include <kdebug.h>
#include <kservicetypetrader.h>
#include <QFile>

#include <limits.h> // UINT_MAX

/**
 * Port from KOffice Trader to KTrader/KActivator (kded) by Simon Hausmann
 * (c) 1999 Simon Hausmann <hausmann@kde.org>
 * Port to KService and simplifications by David Faure <faure@kde.org>
 */


/*******************************************************************
 *
 * KoDocumentEntry
 *
 *******************************************************************/

KoDocumentEntry::KoDocumentEntry()
        : m_service(0)
{
}

KoDocumentEntry::KoDocumentEntry(const KService::Ptr& service)
        : m_service(service)
{
}

KoDocumentEntry::~KoDocumentEntry()
{
}

KoDocument* KoDocumentEntry::createDoc(QString* errorMsg, KoDocument* parent) const
{
    // TODO use m_service->createInstance() to get better error handling,
    // and use of non-deprecated API.
    KLibFactory* factory = KLibLoader::self()->factory(QFile::encodeName(m_service->library()));

    if (!factory) {
        if (errorMsg)
            *errorMsg = KLibLoader::self()->lastErrorMessage();
        kWarning(30003) << KLibLoader::self()->lastErrorMessage();
        return 0;
    }

    QObject* obj;
    if (factory->inherits("KParts::Factory"))
        obj = static_cast<KParts::Factory *>(factory)->createPart(0L, parent, "KoDocument");
    else {
        kWarning(30003) << "factory doesn't inherit KParts::Factory ! It is a " << factory->metaObject()->className(); // This shouldn't happen...
        obj = factory->create(parent, "KoDocument");
    }

    if (!obj || !obj->inherits("KoDocument")) {
        // TODO
        //if ( errorMsg )
        //    *errorMsg = i18n( "Document could not be created" );
        delete obj;
        return 0;
    }

    return static_cast<KoDocument*>(obj);
}

KoDocumentEntry KoDocumentEntry::queryByMimeType(const QString & mimetype)
{
    QString constr = QString::fromLatin1("[X-KDE-NativeMimeType] == '%1' or '%2' in [X-KDE-ExtraNativeMimeTypes]").arg(mimetype).arg(mimetype);

    QList<KoDocumentEntry> vec = query(AllEntries, constr);
    if (vec.isEmpty()) {
        kWarning(30003) << "Got no results with " << constr;
        // Fallback to the old way (which was probably wrong, but better be safe)
        QString constr = QString::fromLatin1("'%1' in ServiceTypes").arg(mimetype);
        vec = query(AllEntries, constr);
        if (vec.isEmpty()) {
            // Still no match. Either the mimetype itself is unknown, or we have no service for it.
            // Help the user debugging stuff by providing some more diagnostics
            if (KServiceType::serviceType(mimetype).isNull()) {
                kError(30003) << "Unknown KOffice MimeType " << mimetype << "." << endl;
                kError(30003) << "Check your installation (for instance, run 'kde4-config --path mime' and check the result)." << endl;
            } else {
                kError(30003) << "Found no KOffice part able to handle " << mimetype << "!" << endl;
                kError(30003) << "Check your installation (does the desktop file have X-KDE-NativeMimeType and KOfficePart, did you install KOffice in a different prefix than KDE, without adding the prefix to /etc/kderc ?)" << endl;
            }
            return KoDocumentEntry();
        }
    }

    return KoDocumentEntry(vec[0]);
}

QList<KoDocumentEntry> KoDocumentEntry::query(QueryFlags flags, const QString & _constr)
{

    QList<KoDocumentEntry> lst;
    QString constr;
    if (!_constr.isEmpty()) {
        constr = "(";
        constr += _constr;
        constr += ") and ";
    }
    constr += " exist Library";

    const bool onlyDocEmb = flags & OnlyEmbeddableDocuments;

    // Query the trader
    const KService::List offers = KServiceTypeTrader::self()->query("KOfficePart", constr);

    KService::List::ConstIterator it = offers.begin();
    unsigned int max = offers.count();
    for (unsigned int i = 0; i < max; i++, ++it) {
        //kDebug(30003) <<"   desktopEntryPath=" << (*it)->desktopEntryPath()
        //               << "   library=" << (*it)->library() << endl;

        if ((*it)->noDisplay())
            continue;
        // Maybe this could be done as a trader constraint too.
        if ((!onlyDocEmb) || ((*it)->property("X-KDE-NOTKoDocumentEmbeddable").toString() != "1")) {
            KoDocumentEntry d(*it);
            // Append converted offer
            lst.append(d);
            // Next service
        }
    }

    if (lst.count() > 1 && !_constr.isEmpty())
        kWarning(30003) << "KoDocumentEntry::query " << constr << " got " << max << " offers!";

    return lst;
}




/*******************************************************************
 *
 * KoFilterEntry
 *
 *******************************************************************/

KoFilterEntry::KoFilterEntry(const KService::Ptr& service)
        : m_service(service)
{
    import = service->property("X-KDE-Import").toStringList();
    export_ = service->property("X-KDE-Export").toStringList();
    int w = service->property("X-KDE-Weight").toInt();
    weight = w < 0 ? UINT_MAX : static_cast<unsigned int>(w);
    available = service->property("X-KDE-Available").toString();
}

QList<KoFilterEntry::Ptr> KoFilterEntry::query(const QString & _constr)
{
    kDebug(30500) << "KoFilterEntry::query(" << _constr << " )";
    QList<KoFilterEntry::Ptr> lst;

    KService::List offers = KServiceTypeTrader::self()->query("KOfficeFilter", _constr);

    KService::List::ConstIterator it = offers.constBegin();
    unsigned int max = offers.count();
    //kDebug(30500) <<"Query returned" << max <<" offers";
    for (unsigned int i = 0; i < max; i++) {
        //kDebug(30500) <<"   desktopEntryPath=" << (*it)->desktopEntryPath()
        //               << "   library=" << (*it)->library() << endl;
        // Append converted offer
        lst.append(KoFilterEntry::Ptr(new KoFilterEntry(*it)));
        // Next service
        it++;
    }

    return lst;
}

KoFilter* KoFilterEntry::createFilter(KoFilterChain* chain, QObject* parent)
{
    KLibFactory* factory = KLibLoader::self()->factory(QFile::encodeName(m_service->library()));

    if (!factory) {
        kWarning(30003) << KLibLoader::self()->lastErrorMessage();
        return 0;
    }

    QObject* obj = factory->create(parent, "KoFilter");
    if (!obj || !obj->inherits("KoFilter")) {
        delete obj;
        return 0;
    }

    KoFilter* filter = static_cast<KoFilter*>(obj);
    filter->m_chain = chain;
    return filter;
}

