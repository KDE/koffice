/* This file is part of the KDE project
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "SCPageEffect.h"

#include <QWidget>
#include <QPainter>

#include <KXmlWriter.h>
#include <KOdfGenericStyle.h>
#include "SCPageEffectStrategy.h"

SCPageEffect::SCPageEffect(int duration, const QString &id, SCPageEffectStrategy * strategy)
: m_duration(duration)
, m_id(id)
, m_strategy(strategy)
{
    Q_ASSERT(strategy);
}

SCPageEffect::~SCPageEffect()
{
}

void SCPageEffect::setup(const Data &data, QTimeLine &timeLine)
{
    timeLine.setDuration(m_duration);
    m_strategy->setup(data, timeLine);
    timeLine.setCurveShape(QTimeLine::LinearCurve);
}

bool SCPageEffect::useGraphicsView()
{
    return m_strategy->useGraphicsView();
}

bool SCPageEffect::paint(QPainter &p, const Data &data)
{
    int currPos = data.m_timeLine.frameForTime(data.m_currentTime);

    bool finish = data.m_finished;

    if (currPos >= data.m_timeLine.endFrame()) {
        finish = true;
    }

    if (! finish) {
        m_strategy->paintStep(p, currPos, data);
    }
    else {
        p.drawPixmap(0, 0, data.m_newPage);
    }

    return !finish;
}

void SCPageEffect::next(const Data &data)
{
    m_strategy->next(data);
}

void SCPageEffect::finish(const Data &data)
{
    m_strategy->finish(data);
}

int SCPageEffect::duration() const
{
    return m_duration;
}

const QString &SCPageEffect::id() const
{
    return m_id;
}

int SCPageEffect::subType() const
{
    return m_strategy->subType();
}

void SCPageEffect::saveOdfSmilAttributes(KXmlWriter &xmlWriter) const
{
    qreal seconds = m_duration / qreal(1000.0);
    xmlWriter.addAttribute("smil:dur", seconds);
    return m_strategy->saveOdfSmilAttributes(xmlWriter);
}

void SCPageEffect::saveOdfSmilAttributes(KOdfGenericStyle &style) const
{
    QString speed("slow");
    if (m_duration < 2500) {
        speed = "fast";
    }
    else if (m_duration < 7500) {
        speed = "medium";
    }
    style.addProperty("presentation:transition-speed", speed);
    return m_strategy->saveOdfSmilAttributes(style);
}

void SCPageEffect::loadOdf(const KXmlElement &)
{
}
