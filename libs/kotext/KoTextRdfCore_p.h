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
#ifndef KO_TEXT_RDF_CORE_P_H
#define KO_TEXT_RDF_CORE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KoText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// this file can only be used by code that is built
// with soprano enabled.
#include <Soprano/Soprano>

class KOdfStore;
class KXmlWriter;

namespace KoTextRdfCore
{
/**
 * Save the RDF selected triples from model to the store with the
 * given RDF/XML filename
 */
bool saveRdf(Soprano::Model *model, Soprano::StatementIterator triples,
        KOdfStore *store, KXmlWriter *manifestWriter, const QString &fileName);

/**
 * Save the given RDF model to the manifest.rdf file. The idmap is used
 * to maintain xml:id links from the model so they will be valid with
 * the content.xml that generated the idmap.
 */
bool createAndSaveManifest(Soprano::Model *model,
        const QMap<QString, QString> &idmap, KOdfStore *store, KXmlWriter *manifestWriter);

/**
 * Load the manifest.rdf file from the ODF container store
 * into the model provided.
 */
bool loadManifest(KOdfStore *store, Soprano::Model *model);

/**
 * For debugging, dump the model to kDebug() along with the
 * given header message for identification
 */
void dumpModel(const QString &message, Soprano::Model *model);

}
#endif

