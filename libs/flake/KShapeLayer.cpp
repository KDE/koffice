/* This file is part of the KDE project
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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

#include "KShapeLayer.h"
#include "SimpleShapeContainerModel_p.h"
#include "KShapeSavingContext.h"
#include "KShapeLoadingContext.h"
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KOdfXmlNS.h>

KShapeLayer::KShapeLayer()
        : KShapeContainer(new SimpleShapeContainerModel())
{
    setSelectable(false);
}

KShapeLayer::KShapeLayer(KShapeContainerModel *model)
        : KShapeContainer(model)
{
    setSelectable(false);
}

bool KShapeLayer::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    return false;
}

QRectF KShapeLayer::boundingRect() const
{
    QRectF bb;

    foreach (KShape *shape, shapes()) {
        if (bb.isEmpty())
            bb = shape->boundingRect();
        else
            bb = bb.unite(shape->boundingRect());
    }

    return bb;
}

void KShapeLayer::saveOdf(KShapeSavingContext &context) const
{
    QList<KShape*> shapes = this->shapes();
    qSort(shapes.begin(), shapes.end(), KShape::compareShapeZIndex);

    foreach(KShape *shape, shapes) {
        shape->saveOdf(context);
    }
}

bool KShapeLayer::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    // set layer name
    setName(element.attributeNS(KOdfXmlNS::draw, "name"));
    // layer locking
    setGeometryProtected(element.attributeNS(KOdfXmlNS::draw, "protected", "false") == "true");
    // layer visibility
    setVisible(element.attributeNS(KOdfXmlNS::draw, "display", "false") != "none");

    // add layer by name into shape context
    context.addLayer(this, name());

    return true;
}
