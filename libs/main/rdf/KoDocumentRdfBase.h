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

#ifndef KO_DOCUMENT_Rdf_Base_H
#define KO_DOCUMENT_Rdf_Base_H

#include "komain_export.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

#include <KoDataCenterBase.h>

class KoCanvasBase;
class KoResourceManager;
class QTextDocument;
class KoStore;
class KoXmlWriter;
class KoDocument;

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
class KOMAIN_EXPORT KoDocumentRdfBase : public QObject, public KoDataCenterBase
{
    Q_OBJECT

public:
    KoDocumentRdfBase(KoDocument *parent);

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
    static KoDocumentRdfBase *fromResourceManager(KoCanvasBase *host);
    virtual void linkToResourceManager(KoResourceManager *rm);

    virtual void updateInlineRdfStatements(QTextDocument *qdoc);
    virtual void updateXmlIdReferences(const QMap<QString, QString> &m);
    virtual bool loadOasis(KoStore *store);
    virtual bool saveOasis(KoStore *store, KoXmlWriter *manifestWriter);
};

#endif

