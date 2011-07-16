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

#ifndef K_DOCUMENT_Rdf_Base_H
#define K_DOCUMENT_Rdf_Base_H

#include "kotext_export.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

#include <KDataCenterBase.h>

class KCanvasBase;
class KResourceManager;
class QTextDocument;
class KOdfStore;
class KXmlWriter;

namespace Soprano
{
    class Model;
}

/**
 * A base class that provides the interface to many RDF features
 * but will not do anything if Soprano support is not built.
 * By having this "Base" class, code can call methods at points
 * where RDF handling is desired and can avoid #ifdef conditionals
 * because the base class interface is here and will be valid, even
 * if impotent when Soprano support is not built.
 */
class KOTEXT_EXPORT KDocumentRdfBase : public QObject, public KDataCenterBase
{
    Q_OBJECT

public:
    KDocumentRdfBase(QObject *parent = 0);

    /**
     * Get the Soprano::Model that contains all the Rdf
     * You do not own the model, do not delete it.
     */
    virtual Soprano::Model *model() const;

    /**
     * Convenience method to get the KoDocumentRdf given a CanvasBase
     * pointer. The resource manager is the canvas is used to get back
     * the KoDoucmentRdf if there is one for the canvas.
     *
     * Note that this method can return either a valid KoDocumentRdf
     * pointer or a NULL pointer if there is no Rdf for the canvas.
     */
    static KDocumentRdfBase *fromResourceManager(KCanvasBase *host);
    virtual void linkToResourceManager(KResourceManager *rm);

    virtual void updateInlineRdfStatements(QTextDocument *qdoc);
    virtual void updateXmlIdReferences(const QMap<QString, QString> &m);
    virtual bool loadOasis(KOdfStore *store);
    virtual bool saveOasis(KOdfStore *store, KXmlWriter *manifestWriter);
};

#endif

