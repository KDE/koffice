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

#ifndef __rdf_RdfFoaF_h__
#define __rdf_RdfFoaF_h__

#include "../komain_export.h"
#include "RdfSemanticItem.h"
#include <QSharedPointer>

/**
 * @short Contact information from the FOAF vocabulary.
 * @author Ben Martin <ben.martin@kogmbh.com>
 */
class KOMAIN_EXPORT RdfFoaF : public RdfSemanticItem
{
    Q_OBJECT
public:

    RdfFoaF(QObject *parent, KoDocumentRdf *m_rdf = 0);
    RdfFoaF(QObject *parent, KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it);
    virtual ~RdfFoaF();

    // inherited and reimplemented...

    /**
     * Export to a VCard format file
     * Prompt for a filename if none is given
     */
    void exportToFile(const QString& fileName = QString());
    /**
     * Import from VCard data contained in ba.
     */
    virtual void importFromData(const QByteArray &ba, KoDocumentRdf *rdf = 0, KoCanvasBase *host = 0);
    virtual QWidget *createEditor(QWidget *parent);
    virtual void updateFromEditorData();
    virtual RdfSemanticTreeWidgetItem *createQTreeWidgetItem(QTreeWidgetItem *parent = 0);
    virtual Soprano::Node linkingSubject() const;
    virtual void setupStylesheetReplacementMapping(QMap<QString, QString> &m);
    virtual void exportToMime(QMimeData *md);
    virtual QList<SemanticStylesheet*> &stylesheets();
    virtual QList<SemanticStylesheet*> &userStylesheets();

    /**
     * Export the contact to your current KDE addressbook.
     */
    void saveToKABC();

    // accessor methods...

    virtual QString name() const;

private:
    class Private;
    QSharedPointer<Private> d;

};
#endif
