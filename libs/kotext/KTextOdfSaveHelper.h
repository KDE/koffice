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

#ifndef KOTEXTODFSAVEHELPER_H
#define KOTEXTODFSAVEHELPER_H

#include <KDragOdfSaveHelper.h>

#include "kotext_export.h"

class KoTextShapeData;
class KXmlWriter;
namespace Soprano
{
    class Model;
}

class KOTEXT_EXPORT KTextOdfSaveHelper : public KDragOdfSaveHelper
{
public:
    KTextOdfSaveHelper(KoTextShapeData * shapeData, int from, int to);
    virtual ~KTextOdfSaveHelper();

    /// reimplemented
    virtual bool writeBody();

    virtual KShapeSavingContext *context(KXmlWriter *bodyWriter, KOdfGenericStyles &mainStyles, KOdfEmbeddedDocumentSaver &embeddedSaver);

    /**
     * The Rdf Model ownership is not taken, you must still delete it,
     * and you need to ensure that it lives longer than this object
     * unless you reset the model to 0.
     */
    void setRdfModel(Soprano::Model *m);
    Soprano::Model *rdfModel() const;

private:
    struct Private;
    Private * const d;
};

#endif /* KOTEXTODFSAVEHELPER_H */
