/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MergeEffect.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include <KLocale>
#include <QtCore/QRect>
#include <QtXml/QDomElement>
#include <QtGui/QPainter>
#include <limits.h>

MergeEffect::MergeEffect()
: KoFilterEffect(MergeEffectId, i18n( "Merge" ))
{
    setRequiredInputCount(2);
    setMaximalInputCount(INT_MAX);
}

QImage MergeEffect::processImage(const QImage &image, const QRect &filterRegion, const KoViewConverter &converter) const
{
    Q_UNUSED(image);
    Q_UNUSED(filterRegion);
    Q_UNUSED(converter);
    return image;
}

QImage MergeEffect::processImages(const QList<QImage> &images, const QRect &filterRegion, const KoViewConverter &converter) const
{
    Q_ASSERT(images.count() == 2);
    
    int imageCount = images.count();
    
    QImage result = images[0];
    
    QPainter p(&result);
    
    for (int i = 1; i < imageCount; ++i) {
        p.drawImage(QPoint(), images[i]);
    }
    
    return result;
}

bool MergeEffect::load(const QDomElement &element)
{
    if (element.tagName() != id())
        return false;

    for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement node = n.toElement();
        if (node.tagName() == "feMergeNode") {
            if( node.hasAttribute("in"))
                addInput(node.attribute("in"));
        }
    }

    return true;
}

void MergeEffect::save(KoXmlWriter &writer)
{
    writer.startElement(MergeEffectId);
    
    foreach (QString input, inputs()) {
        writer.startElement("feMergeNode");
        writer.addAttribute("in", input);
        writer.endElement();
    }
    
    writer.endElement();
}
