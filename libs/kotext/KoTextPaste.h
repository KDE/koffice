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

#ifndef KOTEXTPASTE_H
#define KOTEXTPASTE_H

#include <KOdfPasteBase.h>
#include "kotext_export.h"

class QTextCursor;
class KoTextShapeData;
class KoCanvasBase;

#include <QSharedPointer>
namespace Soprano
{
    class Model;
}

class KOTEXT_EXPORT KoTextPaste : public KOdfPasteBase
{
public:
    /**
     * Note: RdfModel ownership is not taken. You must ensure that it remains
     * valid for the lifetime of the object.
     */
    KoTextPaste(KoTextShapeData *shapeData, QTextCursor &cursor,
                KoCanvasBase *canvas, Soprano::Model *rdfModel);
    virtual ~KoTextPaste();

protected:
    /// reimplemented
    virtual bool process(const KoXmlElement &body, KOdfStoreReader &odfStore);

private:
    class Private;
    Private * const d;
};

#endif /* KOTEXTPASTE_H */
