/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "PictureShape.h"

#include <KoImageData.h>
#include <KoViewConverter.h>

#include <QPainter>
#include <kdebug.h>

PictureShape::PictureShape()
{
    setKeepAspectRatio(true);
}

PictureShape::~PictureShape() {
}

void PictureShape::paint( QPainter& painter, const KoViewConverter& converter ) {
    if(m_imageData != userData())
        m_imageData = dynamic_cast<KoImageData*> (userData());
    if(m_imageData == 0)
        return;

    QPixmap pm = m_imageData->pixmap();
    QRectF target = converter.documentToView(QRectF(QPointF(0,0), size()));
    painter.drawPixmap(target.toRect(), pm, QRect(0, 0, pm.width(), pm.height()));
}

void PictureShape::saveOdf( KoShapeSavingContext & context ) const
{
    //TODO
}

bool PictureShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{

    // TODO

    QDomNamedNodeMap attrs = element.attributes();
    for (int iAttr = 0 ; iAttr < attrs.count() ; iAttr++)
        kDebug(32500) << "PictureShape::loadOdf Attribute " << iAttr << " : " << attrs.item(iAttr).nodeName() << "\t" << attrs.item(iAttr).nodeValue() << endl;

    return false;
}
