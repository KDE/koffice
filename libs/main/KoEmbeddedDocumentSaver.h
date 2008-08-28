/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOEMBEDDEDDOCUMENTSAVER_H
#define KOEMBEDDEDDOCUMENTSAVER_H

#include <QList>

#include "KoDocument.h"
#include "komain_export.h"

class KoStore;
class KoXmlWriter;

/**
 * This class is used to save embedded objects in ODF documents.
 */
class KOMAIN_EXPORT KoEmbeddedDocumentSaver
{
public:
    KoEmbeddedDocumentSaver();
    ~KoEmbeddedDocumentSaver();

    /**
     * Adds the object specific attibutes to the tag, but does NOT
     * write the content of the embedded document. Saving of the
     * embedded documents themselves is done in @ref save. This
     * function should be called from within KoDocument::saveOasis.
     */
    void embedDocument(KoXmlWriter &writer, KoDocument * doc);

    /**
     * Save all embedded documents to the store.
     */
    bool saveEmbeddedDocuments(KoDocument::SavingContext & documentContext);

private:
    QList<KoDocument *> m_documents;
    int m_objectId;
};

#endif /* KOEMBEDDEDDOCUMENTSAVER_H */
