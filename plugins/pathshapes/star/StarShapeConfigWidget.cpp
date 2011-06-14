/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "StarShapeConfigWidget.h"
#include "StarShape.h"
#include "StarShapeConfigCommand.h"

#include <KCanvasBase.h>

StarShapeConfigWidget::StarShapeConfigWidget(KCanvasBase *canvas)
    : m_canvas(canvas),
    m_blocking(false)
{
    widget.setupUi(this);

    connect(widget.corners, SIGNAL(valueChanged(int)), this, SLOT(propertyChanged()));
    connect(widget.innerRadius, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
    connect(widget.outerRadius, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
    connect(widget.convex, SIGNAL(stateChanged(int)), this, SLOT(propertyChanged()));
    connect(widget.convex, SIGNAL(clicked()), this, SLOT(typeChanged()));
}

void StarShapeConfigWidget::setUnit(const KUnit &unit)
{
    widget.innerRadius->setUnit(unit);
    widget.outerRadius->setUnit(unit);
}

void StarShapeConfigWidget::open(KShape *shape)
{
    m_star = dynamic_cast<StarShape*>(shape);
    if (! m_star)
        return;
    if (m_blocking)
        return;
    StarShape *star = dynamic_cast<StarShape*>(shape);
    if (star != m_star) // if its equal, we just update our values
        m_command = 0;
    m_star = 0;
    if (!star) {
        setEnabled(false);
        return;
    }
    setEnabled(true);

    m_blocking = true;
    widget.corners->setValue(star->cornerCount());
    widget.innerRadius->changeValue(star->baseRadius());
    widget.outerRadius->changeValue(star->tipRadius());
    widget.convex->setCheckState(star->convex() ? Qt::Checked : Qt::Unchecked);
    typeChanged();
    m_blocking = false;

    m_star = star;
}

void StarShapeConfigWidget::propertyChanged()
{
    if (!m_star)
        return;
    if (m_blocking)
        return;
    m_blocking = true;
    if (m_command && commandIsValid()) {
        m_command->setCornerCount(widget.corners->value());
        m_command->setBaseRadius(widget.innerRadius->value());
        m_command->setTipRadius(widget.outerRadius->value());
        m_command->setConvex(widget.convex->checkState() == Qt::Checked);
        m_star->update();
        m_star->setCornerCount(widget.corners->value());
        m_star->setBaseRadius(widget.innerRadius->value());
        m_star->setTipRadius(widget.outerRadius->value());
        m_star->setConvex(widget.convex->checkState() == Qt::Checked);
        m_star->update();
    } else {
        m_command = new StarShapeConfigCommand(m_star, widget.corners->value(),
                widget.innerRadius->value(), widget.outerRadius->value(),
                widget.convex->checkState() == Qt::Checked);
        m_canvas->addCommand(m_command);
    }
    m_time.restart();
    m_blocking = false;

    m_star->setCornerCount(widget.corners->value());
    m_star->setBaseRadius(widget.innerRadius->value());
    m_star->setTipRadius(widget.outerRadius->value());
    m_star->setConvex(widget.convex->checkState() == Qt::Checked);
}

void StarShapeConfigWidget::typeChanged()
{
    if (widget.convex->checkState() == Qt::Checked) {
        widget.innerRadius->setEnabled(false);
    } else {
        widget.innerRadius->setEnabled(true);
    }
}

bool StarShapeConfigWidget::commandIsValid() const
{
    if (!m_time.isValid())
        return false;

    if (m_time.elapsed() > 4000)
        return false;

    return true;
}

#include <StarShapeConfigWidget.moc>
