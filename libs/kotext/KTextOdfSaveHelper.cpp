/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KTextOdfSaveHelper.h"

#include <KXmlWriter.h>
#include <KOdf.h>
#include "KTextShapeData.h"
#include <KOdfGenericChanges.h>
#include <KShapeSavingContext.h>

#include <opendocument/KoTextSharedSavingData.h>
#include "KoTextSopranoRdfModel_p.h"

struct KTextOdfSaveHelper::Private {
    Private(KTextShapeData *shapeData, int from, int to)
        : shapeData(shapeData),
        from(from),
        to(to),
        rdfModel(0)
    {
    }

    KShapeSavingContext *context;
    KTextShapeData *shapeData;

    int from;
    int to;

    Soprano::Model *rdfModel; //< This is so cut/paste can serialize the relevant RDF to the clipboard
};


KTextOdfSaveHelper::KTextOdfSaveHelper(KTextShapeData * shapeData, int from, int to)
        : d(new Private(shapeData, from, to))
{
}

KTextOdfSaveHelper::~KTextOdfSaveHelper()
{
    delete d;
}

bool KTextOdfSaveHelper::writeBody()
{
    if (d->to < d->from)
        qSwap(d->to, d->from);

    KXmlWriter & bodyWriter = d->context->xmlWriter();
    bodyWriter.startElement("office:body");
    bodyWriter.startElement(KOdf::bodyContentElement(KOdf::TextDocument, true));

    d->shapeData->saveOdf(*d->context, 0, d->from, d->to);
    d->context->writeConnectors();

    bodyWriter.endElement(); // office:element
    bodyWriter.endElement(); // office:body
    return true;
}

KShapeSavingContext * KTextOdfSaveHelper::context(KXmlWriter * bodyWriter, KOdfGenericStyles & mainStyles, KOdfEmbeddedDocumentSaver & embeddedSaver)
{
//    Q_ASSERT(d->context == 0);

    d->context = new KShapeSavingContext(*bodyWriter, mainStyles, embeddedSaver);
    return d->context;
}

void KTextOdfSaveHelper::setRdfModel(Soprano::Model *m)
{
    d->rdfModel = m;
}

Soprano::Model *KTextOdfSaveHelper::rdfModel() const
{
    return d->rdfModel;
}

