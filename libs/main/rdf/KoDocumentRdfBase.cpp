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

#include "KoDocumentRdfBase.h"
#include "../KoDocument.h"
#include <KoResourceManager.h>
#include <KoCanvasBase.h>
#include <KoText.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>

#include "KoTextSopranoRdfModel_p.h"
#include <kdebug.h>

KoDocumentRdfBase::KoDocumentRdfBase(KoDocument *parent)
        : QObject(parent)
{
}

Soprano::Model *KoDocumentRdfBase::model() const
{
    return 0;
}

KoDocumentRdfBase *KoDocumentRdfBase::fromResourceManager(KoCanvasBase *host)
{
    KoResourceManager *rm = host->resourceManager();
    if (!rm->hasResource(KoText::DocumentRdf)) {
        return 0;
    }
    return static_cast<KoDocumentRdfBase*>(rm->resource(KoText::DocumentRdf).value<void*>());
}

void KoDocumentRdfBase::linkToResourceManager(KoResourceManager *rm)
{
    QVariant variant;
    variant.setValue<void*>(this);
    rm->setResource(KoText::DocumentRdf, variant);
}

void KoDocumentRdfBase::updateInlineRdfStatements(QTextDocument *qdoc)
{
    Q_UNUSED(qdoc);
}

void KoDocumentRdfBase::updateXmlIdReferences(const QMap<QString, QString> &m)
{
    Q_UNUSED(m);
}

bool KoDocumentRdfBase::loadOasis(KoStore *store)
{
    Q_UNUSED(store);
    return true;
}

bool KoDocumentRdfBase::saveOasis(KoStore *store, KoXmlWriter *manifestWriter)
{
    Q_UNUSED(store);
    Q_UNUSED(manifestWriter);
    return true;
}
