/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPAODFPAGESAVEHELPER_H
#define KOPAODFPAGESAVEHELPER_H

#include <KDragOdfSaveHelper.h>

#include <QList>

#include "kopageapp_export.h"

class KoPADocument;
class KoPAPage;

class KOPAGEAPP_TEST_EXPORT KoPAOdfPageSaveHelper : public KDragOdfSaveHelper
{
public:
    /**
     * Use only one type of pages e.g. only master pages or only normal pages
     * if you mix the master pages will only be saved if they are needed for a normal page.
     */
    KoPAOdfPageSaveHelper(KoPADocument * doc, QList<KoPAPage *> pages);
    virtual ~KoPAOdfPageSaveHelper();

    /// reimplemented
    virtual KShapeSavingContext * context(KXmlWriter * bodyWriter, KOdfGenericStyles &mainStyles, KOdfEmbeddedDocumentSaver &embeddedSaver);

    /// reimplemented
    virtual bool writeBody();

private:
    KoPADocument * m_doc;
    KShapeSavingContext  *m_context;
    QList<KoPAPage *> m_pages;
    QList<KoPAPage *> m_masterPages;
};

#endif /* KOPAODFPAGESAVEHELPER_H */
