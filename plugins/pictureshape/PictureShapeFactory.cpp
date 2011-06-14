/* This file is part of the KDE project
 * Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "PictureShapeFactory.h"

#include "PictureShape.h"
#include "PictureShapeConfigWidget.h"

#include <KOdfXmlNS.h>
#include "KShapeControllerBase.h"
#include <KShapeLoadingContext.h>
#include "KImageCollection.h"

#include <klocale.h>
#include <kdebug.h>

PictureShapeFactory::PictureShapeFactory(QObject *parent)
    : KShapeFactoryBase(parent, PICTURESHAPEID, i18n("Image"))
{
    setToolTip(i18n("Image shape that can display jpg, png etc."));
    setIcon("x-shape-image");
    setOdfElementNames(KOdfXmlNS::draw, QStringList("image"));
    setLoadingPriority(1);
}

KShape *PictureShapeFactory::createDefaultShape(KResourceManager *documentResources) const
{
    PictureShape * defaultShape = new PictureShape();
    defaultShape->setShapeId(PICTURESHAPEID);
    if (documentResources) {
        defaultShape->setImageCollection(documentResources->imageCollection());
    }
    return defaultShape;
}

bool PictureShapeFactory::supports(const KXmlElement &e, KShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return e.localName() == "image" && e.namespaceURI() == KOdfXmlNS::draw;
}

void PictureShapeFactory::newDocumentResourceManager(KResourceManager *manager)
{
    manager->setLazyResourceSlot(KoDocumentResource::ImageCollection,
            this, "createImageCollection");
}

void PictureShapeFactory::createImageCollection(KResourceManager *manager)
{
    manager->setImageCollection(new KImageCollection(manager));
}

