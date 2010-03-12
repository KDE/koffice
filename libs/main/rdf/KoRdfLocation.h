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

#ifndef __rdf_KoRdfLocation_h__
#define __rdf_KoRdfLocation_h__

#include "rdf/KoRdfSemanticItem.h"
#include <QSharedPointer>

class KoRdfLocationPrivate;

/**
 * @short A Location class which handles ICBM (lat/long) data of various kinds.
 * @author Ben Martin <ben.martin@kogmbh.com>
 *
 * Handling the mapping Location name <-> ICBM is still an open topic.
 *
 * The two schemas below are currently handled as they both essentially
 * store a lat/long but in different ways.
 *
 * http://www.w3.org/2003/01/geo/   WGS84 base
 * http://www.w3.org/TR/rdfcal/     Relates an Rdf "geo" to a list of 2 doubles.
 *
 */
class KoRdfLocation : public KoRdfSemanticItem
{
    Q_OBJECT

public:
    KoRdfLocation(QObject *parent, KoDocumentRdf *m_rdf = 0);
    KoRdfLocation(QObject *parent, KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it, bool isGeo84);
    virtual ~KoRdfLocation();

    // inherited and reimplemented...

    virtual void exportToFile(const QString &fileName = QString()) const;
    virtual void importFromData(const QByteArray &ba, KoDocumentRdf *rdf = 0, KoCanvasBase *host = 0);
    virtual QWidget *createEditor(QWidget *parent);
    virtual void updateFromEditorData();
    virtual KoRdfSemanticTreeWidgetItem *createQTreeWidgetItem(QTreeWidgetItem *parent = 0);
    virtual Soprano::Node linkingSubject() const;
    virtual void setupStylesheetReplacementMapping(QMap<QString, QString> &m);
    virtual void exportToMime(QMimeData *md) const;
    virtual QList<KoSemanticStylesheet*> stylesheets() const;
    virtual QString className() const;

    /**
     * Present the location in some graphical map display
     * like Marble.
     */
    virtual void showInViewer();

    // accessor methods...

    virtual QString name() const;
    double dlat() const;
    double dlong() const;

private:
    Q_DECLARE_PRIVATE(KoRdfLocation);
};
#endif
