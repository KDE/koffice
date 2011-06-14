/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#include "KDocumentRdfBase.h"
#include "KoTextSopranoRdfModel_p.h"
#include "KoText.h"

#include <KResourceManager.h>
#include <KCanvasBase.h>
#include <KXmlWriter.h>
#include <KOdfStorageDevice.h>

#include <kdebug.h>

KDocumentRdfBase::KDocumentRdfBase(QObject *parent)
        : QObject(parent)
{
}

Soprano::Model *KDocumentRdfBase::model() const
{
    return 0;
}

KDocumentRdfBase *KDocumentRdfBase::fromResourceManager(KCanvasBase *host)
{
    KResourceManager *rm = host->resourceManager();
    if (!rm->hasResource(KoText::DocumentRdf)) {
        return 0;
    }
    return static_cast<KDocumentRdfBase*>(rm->resource(KoText::DocumentRdf).value<void*>());
}

void KDocumentRdfBase::linkToResourceManager(KResourceManager *rm)
{
    QVariant variant;
    variant.setValue<void*>(this);
    rm->setResource(KoText::DocumentRdf, variant);
}

void KDocumentRdfBase::updateInlineRdfStatements(QTextDocument *qdoc)
{
    Q_UNUSED(qdoc);
}

void KDocumentRdfBase::updateXmlIdReferences(const QMap<QString, QString> &m)
{
    Q_UNUSED(m);
}

bool KDocumentRdfBase::loadOasis(KOdfStore *store)
{
    Q_UNUSED(store);
    return true;
}

bool KDocumentRdfBase::saveOasis(KOdfStore *store, KXmlWriter *manifestWriter)
{
    Q_UNUSED(store);
    Q_UNUSED(manifestWriter);
    return true;
}
