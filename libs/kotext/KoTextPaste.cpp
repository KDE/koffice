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

#include "KoTextPaste.h"

#include <KOdfStoreReader.h>
#include <KOdfLoadingContext.h>
#include <KCanvasBase.h>
#include <KShapeLoadingContext.h>
#include <KShapeControllerBase.h>
#include <KShapeController.h>
#include "KoTextShapeData.h"
#include "opendocument/KoTextLoader.h"

#include <kdebug.h>
#ifdef SHOULD_BUILD_RDF
#include "KoTextRdfCore.h"
#include "KoTextRdfCore_p.h"
#endif

class KoTextPaste::Private
{
public:
    Private(KoTextShapeData *shapeData, QTextCursor &cursor,
            KCanvasBase *canvas, Soprano::Model *rdfModel)
            : shapeData(shapeData)
            , cursor(cursor)
            , canvas(canvas)
            , rdfModel(rdfModel) {}

    KoTextShapeData *shapeData;
    QTextCursor &cursor;
    KCanvasBase *canvas;
    Soprano::Model *rdfModel;
};

KoTextPaste::KoTextPaste(KoTextShapeData *shapeData, QTextCursor &cursor,
                         KCanvasBase *canvas, Soprano::Model *rdfModel)
        : d(new Private(shapeData, cursor, canvas, rdfModel))
{
}

KoTextPaste::~KoTextPaste()
{
    delete d;
}

bool KoTextPaste::process(const KXmlElement &body, KOdfStoreReader &odfStore)
{
    bool ok = true;
    KOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
    KShapeLoadingContext context(loadingContext, d->canvas->shapeController()->resourceManager());

    KoTextLoader loader(context);

    kDebug(30015) << "text paste";
    loader.loadBody(body, d->cursor);   // now let's load the body from the ODF KXmlElement.

#ifdef SHOULD_BUILD_RDF
    // RDF: Grab RDF metadata from ODF file if present & load it into rdfModel
    if (d->rdfModel) {
        Soprano::Model *tmpmodel(Soprano::createModel());
        ok = KoTextRdfCore::loadManifest(odfStore.store(), tmpmodel);
        kDebug(30015) << "ok:" << ok << " model.sz:" << tmpmodel->statementCount();
#ifndef NDEBUG
        KoTextRdfCore::dumpModel("RDF from C+P", tmpmodel);
#endif
        d->rdfModel->addStatements(tmpmodel->listStatements().allElements());
        delete tmpmodel;
    }
#endif

    return ok;
}
