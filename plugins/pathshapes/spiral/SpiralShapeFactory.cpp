/* This file is part of the KDE project
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "SpiralShapeFactory.h"
#include "SpiralShape.h"
#include "SpiralShapeConfigWidget.h"
#include <KLineBorder.h>
#include <KShapeLoadingContext.h>

#include <klocale.h>


SpiralShapeFactory::SpiralShapeFactory(QObject *parent)
: KShapeFactoryBase(parent, SpiralShapeId, i18n("Spiral"))
{
    setToolTip(i18n("A spiral shape"));
    setIcon("spiral-shape");
    setFamily("geometric");
    setLoadingPriority(1);
}

KShape *SpiralShapeFactory::createDefaultShape(KResourceManager *) const
{
    SpiralShape *spiral = new SpiralShape();

    spiral->setBorder(new KLineBorder(1.0));
    spiral->setShapeId(KoPathShapeId);

    return spiral;
}

bool SpiralShapeFactory::supports(const KXmlElement & e, KShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return false;
}

KShapeConfigWidgetBase *SpiralShapeFactory::createConfigWidget(KCanvasBase *canvas)
{
    return new SpiralShapeConfigWidget(canvas);
}

