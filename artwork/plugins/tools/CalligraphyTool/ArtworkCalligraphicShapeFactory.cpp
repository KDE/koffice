/* This file is part of the KDE project
 * Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>
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

#include "ArtworkCalligraphicShapeFactory.h"
#include "ArtworkCalligraphicShape.h"

#include <klocale.h>
#include <KShapeLoadingContext.h>


ArtworkCalligraphicShapeFactory::ArtworkCalligraphicShapeFactory(QObject *parent)
        : KShapeFactoryBase(parent, ArtworkCalligraphicShapeId, i18n("A calligraphic shape"))
{
    setToolTip(i18n("Calligraphic Shape"));
    setIcon("calligraphy");
    setLoadingPriority(1);
    setHidden(true);
}

ArtworkCalligraphicShapeFactory::~ArtworkCalligraphicShapeFactory()
{
}

KShape *ArtworkCalligraphicShapeFactory::createDefaultShape(KResourceManager *) const
{
    ArtworkCalligraphicShape *path = new ArtworkCalligraphicShape();

    // FIXME: add points
    path->setShapeId(ArtworkCalligraphicShapeId);

    return path;
}

bool ArtworkCalligraphicShapeFactory::supports(const KXmlElement & e, KShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return false;
}
