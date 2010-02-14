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

#ifndef __rdf_RdfPrefixMapping_h__
#define __rdf_RdfPrefixMapping_h__

#include "komain_export.h"
#include "RdfForward.h"

#include <QObject>
#include <QMap>
#include <QString>

/**
 * @short Supports bidirectional prefix:lname -> uri/lname translation
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoDocumentRdf
 *
 * Both Rdf and XML support namespaces. This class is intended to allow
 * the advanced user to use such namespace mappings in the KOffice suite
 * when dealing with Rdf.
 *
 * For example, to be able to say dc:author for the dublin core author uri
 *
 */
class KOMAIN_EXPORT RdfPrefixMapping : public QObject
{
    Q_OBJECT
public:
    RdfPrefixMapping(KoDocumentRdf *rdf);
    ~RdfPrefixMapping();

    /**
     * Convert a URI to a prefix:rest string
     * For example, given:
     * http://www.example.com/foo/bar
     * you might get
     * foo:bar
     * as the return value
     */
    QString URItoPrefexedLocalname(const QString &uri) const;

    /**
     * Opposite of URItoPrefexedLocalname(). Given foo:bar
     * you get http://www.example.com/foo/bar
     */
    QString PrefexedLocalnameToURI(const QString &pname) const;

    /**
     * Lookup the URI associated with a prefix.
     * given foo: you might get http://www.example.com/foo/
     */
    QString prefexToURI(const QString &pname) const;

    /**
     * Insert a new mapping prefix -> uri
     */
    void insert(const QString &prefix, const QString &url);

    /**
     * Delete the mapping for prefix
     */
    void remove(const QString &prefix);

    /**
     * Load the prefix mapping information from the given model.
     *
     * @see save()
     */
    void load(Soprano::Model *model);

    /**
     * Save the prefix mapping into the given Rdf model. If there is
     * already a mapping in the model then those triples are delete
     * and fresh triples inserted to represet the this prefix mapping
     * state.
     *
     * @see load()
     */
    void save(Soprano::Model *model, Soprano::Node context) const;

    /**
     * Debug method to capture the data structure in the logs.
     */
    void dump() const;

private:
    friend class KoDocumentRdfEditWidget;
    KoDocumentRdf *m_rdf;

    /**
     * prefix -> uri map.
     * gives speed preference to prefix resolution over
     * turning a uri into a prefixed shorter string
         */
    QMap<QString, QString> m_mappings;

    QString canonPrefix(const QString &pname) const;
};

#endif
