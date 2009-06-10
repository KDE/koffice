/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeGroup.h"
#include "KoShapeContainerModel.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include "KoShapeRegistry.h"
#include "KoShapeBorderModel.h"
#include "KoShapeShadow.h"

#include <QPainter>

KoShapeGroup::KoShapeGroup()
        : KoShapeContainer(new SimpleShapeContainerModel())
{
    setSize(QSizeF(0, 0));
}

void KoShapeGroup::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

bool KoShapeGroup::hitTest(const QPointF &position) const
{
    Q_UNUSED(position);
    return false;
}

void KoShapeGroup::childCountChanged()
{
    // TODO: why is this needed here ? the group/ungroup command should take care of this
    QRectF br = boundingRect();
    setAbsolutePosition(br.topLeft(), KoFlake::TopLeftCorner);
    setSize(br.size());
}

void KoShapeGroup::saveOdf(KoShapeSavingContext & context) const
{
    context.xmlWriter().startElement("draw:g");
    saveOdfAttributes(context, (OdfMandatories ^ OdfLayer) | OdfAdditionalAttributes);
    context.xmlWriter().addAttributePt("svg:y", position().y());

    QList<KoShape*> shapes = childShapes();
    qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    foreach(KoShape* shape, shapes) {
        shape->saveOdf(context);
    }

    saveOdfCommonChildElements(context);
    context.xmlWriter().endElement();
}

bool KoShapeGroup::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfMandatories | OdfAdditionalAttributes | OdfCommonChildElements);

    KoXmlElement child;
    forEachElement(child, element) {
        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(child, context);
        if (shape) {
            addChild(shape);
        }
    }

    QRectF bound;
    bool boundInitialized = false;
    foreach(KoShape * shape, childShapes()) {
        if (! boundInitialized) {
            bound = shape->boundingRect();
            boundInitialized = true;
        } else
            bound = bound.united(shape->boundingRect());
    }

    setSize(bound.size());
    setPosition(bound.topLeft());

    foreach(KoShape * shape, childShapes())
        shape->setAbsolutePosition(shape->absolutePosition() - bound.topLeft());

    return true;
}

void KoShapeGroup::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    switch( type )
    {
        case KoShape::BorderChanged:
        {
            KoShapeBorderModel * stroke = border();
            if( stroke ) {
                if( stroke->removeUser() )
                    delete stroke;
                setBorder(0);
            }
            break;
        }
        case KoShape::ShadowChanged:
        {
            KoShapeShadow * shade = shadow();
            if( shade ) {
                if( shade->removeUser() )
                    delete shade;
                setShadow(0);
            }
            break;
        }
        default:
            break;
    }
}
