/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCFadeOverColorStrategy.h"
#include "SCFadeEffectFactory.h"

#include <QWidget>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>

#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KoXmlWriter.h>
#include <KOdfGenericStyle.h>
#include <kdebug.h>

SCFadeOverColorStrategy::SCFadeOverColorStrategy()
: SCPageEffectStrategy(SCFadeEffectFactory::FadeOverColor, "fade", "fadeOverColor", false, true)
, m_fadeColor(Qt::black)
{
}

SCFadeOverColorStrategy::~SCFadeOverColorStrategy()
{
}

void SCFadeOverColorStrategy::setup(const SCPageEffect::Data &data, QTimeLine &timeLine)
{
    timeLine.setFrameRange(0, 1000); // TODO might not be needed
    data.m_graphicsView->setBackgroundBrush(m_fadeColor);
    data.m_oldPageItem->setZValue(1);
    data.m_newPageItem->setZValue(2);
    data.m_newPageItem->setOpacity(0);
    data.m_oldPageItem->show();
    data.m_newPageItem->show();
}

void SCFadeOverColorStrategy::paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data)
{
    Q_UNUSED(p);
    Q_UNUSED(currPos);
    Q_UNUSED(data);
}

void SCFadeOverColorStrategy::next(const SCPageEffect::Data &data)
{
    int frame = data.m_timeLine.frameForTime(data.m_currentTime);
    if (frame >= data.m_timeLine.endFrame()) {
        finish(data);
    }
    else {
        qreal value = 1 - (data.m_timeLine.valueForTime(data.m_currentTime) * qreal(2.0));
        if (value >= 0) {
            data.m_oldPageItem->setOpacity(value);
        }
        else {
            data.m_oldPageItem->hide();
            data.m_newPageItem->setOpacity(-value);
        }
    }
}

void SCFadeOverColorStrategy::finish(const SCPageEffect::Data &data)
{
    data.m_graphicsView->hide();
}

void SCFadeOverColorStrategy::saveOdfSmilAttributes(KoXmlWriter & xmlWriter) const
{
    SCPageEffectStrategy::saveOdfSmilAttributes(xmlWriter);
    xmlWriter.addAttribute("smil:fadeColor", m_fadeColor.name());
}

void SCFadeOverColorStrategy::saveOdfSmilAttributes(KOdfGenericStyle & style) const
{
    SCPageEffectStrategy::saveOdfSmilAttributes(style);
    style.addProperty("smil:fadeColor", m_fadeColor.name());
}

void SCFadeOverColorStrategy::loadOdfSmilAttributes(const KoXmlElement & element)
{
    // use black as default
    m_fadeColor.setNamedColor(element.attributeNS(KOdfXmlNS::smil, "fadeColor", "#000000"));
}
