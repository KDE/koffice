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

#include "OffsetEffect.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include <KLocale>
#include <QtCore/QRect>
#include <QtXml/QDomElement>
#include <QtGui/QPainter>

#include <KDebug>

OffsetEffect::OffsetEffect()
: KoFilterEffect(OffsetEffectId, i18n( "Offset" ))
, m_offset(0,0)
{
}

QPointF OffsetEffect::offset() const
{
    return m_offset;
}

void OffsetEffect::setOffset(const QPointF &offset)
{
    m_offset = offset;
}

void OffsetEffect::processImage(QImage &image, const QRect &filterRegion, const KoViewConverter &converter) const
{
    if (m_offset.x() == 0.0 && m_offset.y() == 0.0)
        return;
    
    // TODO: take filter region into account
    // TODO: blur with different kernels in x and y
    QPointF offset = converter.documentToView(m_offset);

    QImage result(image.size(), image.format());
    result.fill(qRgba(0,0,0,0));
    
    QPainter p(&result);
    p.drawImage(filterRegion.topLeft()+offset, image, filterRegion);
    image = result;
}

bool OffsetEffect::load(const QDomElement &element)
{
    if (element.tagName() != id())
        return false;

    if (element.hasAttribute("dx"))
        m_offset.rx() = element.attribute("dx").toDouble();
    if (element.hasAttribute("dy"))
        m_offset.ry() = element.attribute("dy").toDouble();
    
    return true;
}

void OffsetEffect::save(KoXmlWriter &writer)
{
    writer.startElement(OffsetEffectId);

    if (m_offset.x() != 0.0)
        writer.addAttribute("dx", m_offset.x());
    if (m_offset.y() != 0.0)
        writer.addAttribute("dy", m_offset.x());
    
    writer.endElement();
}
