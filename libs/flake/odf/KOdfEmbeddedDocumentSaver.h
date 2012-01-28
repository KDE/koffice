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

#ifndef KODFEMBEDDEDDOCUMENTSAVER_H
#define KODFEMBEDDEDDOCUMENTSAVER_H

#include "KOdfDocumentBase.h"				//krazy:exclude=includes
#include "flake_export.h"

class KXmlWriter;

/**
 * This class is used to save embedded objects in ODF documents.
 */
class FLAKE_EXPORT KOdfEmbeddedDocumentSaver
{
public:
    KOdfEmbeddedDocumentSaver();
    ~KOdfEmbeddedDocumentSaver();

    /**
     * Adds the object specific attributes to the tag, but does NOT
     * write the content of the embedded document. Saving of the
     * embedded documents themselves is done in @ref save. This
     * function should be called from within KOdfDocumentBase::saveOdf.
     */
    void embedDocument(KXmlWriter &writer, KOdfDocumentBase *doc);

    /**
     * Save all embedded documents to the store.
     */
    bool saveEmbeddedDocuments(KOdfDocumentBase::SavingContext &documentContext);

private:
    class Private;
    Private *d;
    Q_DISABLE_COPY(KOdfEmbeddedDocumentSaver)
};

#endif /* KOEMBEDDEDDOCUMENTSAVER_H */
