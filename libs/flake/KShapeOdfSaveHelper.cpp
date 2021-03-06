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

#include "KShapeOdfSaveHelper.h"
#include "KDragOdfSaveHelper_p.h"

#include <KXmlWriter.h>
#include <KOdf.h>
#include <KShape.h>

class KShapeOdfSaveHelperPrivate : public KDragOdfSaveHelperPrivate
{
public:
    KShapeOdfSaveHelperPrivate(QList<KShape *> shapes)
    : shapes(shapes) {}

    QList<KShape *> shapes;
};

KShapeOdfSaveHelper::KShapeOdfSaveHelper(QList<KShape *> shapes)
        : KDragOdfSaveHelper(*(new KShapeOdfSaveHelperPrivate(shapes)))
{
}

bool KShapeOdfSaveHelper::writeBody()
{
    Q_D(KShapeOdfSaveHelper);
    d->context->addOption(KShapeSavingContext::DrawId);

    KXmlWriter &bodyWriter = d->context->xmlWriter();
    bodyWriter.startElement("office:body");
    bodyWriter.startElement(KOdf::bodyContentElement(KOdf::TextDocument, true));

    qSort(d->shapes.begin(), d->shapes.end(), KShape::compareShapeZIndex);
    foreach (KShape *shape, d->shapes) {
        shape->saveOdf(*d->context);
    }
    d->context->writeConnectors();

    bodyWriter.endElement(); // office:element
    bodyWriter.endElement(); // office:body

    return true;
}
