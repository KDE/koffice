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

#include "SpiralShapeConfigWidget.h"
#include "SpiralShapeConfigCommand.h"
#include <klocale.h>

#include <KCanvasBase.h>

SpiralShapeConfigWidget::SpiralShapeConfigWidget(KCanvasBase *canvas)
    : m_canvas(canvas),
    m_blocking(false)
{
    widget.setupUi(this);

    widget.spiralType->clear();
    widget.spiralType->addItem(i18n("Curve"));
    widget.spiralType->addItem(i18n("Line"));

    widget.fade->setMinimum(0.0);
    widget.fade->setMaximum(1.0);

    widget.clockWise->clear();
    widget.clockWise->addItem("ClockWise");
    widget.clockWise->addItem("Anti-ClockWise");

    connect(widget.spiralType, SIGNAL(currentIndexChanged(int)), this, SLOT(propertyChanged()));
    connect(widget.clockWise, SIGNAL(currentIndexChanged(int)), this, SLOT(propertyChanged()));
    connect(widget.fade, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
}

void SpiralShapeConfigWidget::open(KoShape *shape)
{
    if (m_blocking)
        return;
    SpiralShape *spiral = dynamic_cast<SpiralShape*>(shape);
    if (spiral != m_spiral) // if its equal, we just update our values
        m_command = 0;
    m_spiral = 0;
    if (!spiral) {
        setEnabled(false);
        return;
    }
    setEnabled(true);

    m_blocking = true;
    widget.spiralType->setCurrentIndex(spiral->type());
    widget.clockWise->setCurrentIndex(spiral->clockWise() ? 0 : 1);
    widget.fade->setValue(spiral->fade());
    m_blocking = false;
    m_spiral = spiral;
}

void SpiralShapeConfigWidget::propertyChanged()
{
    if (!m_spiral)
        return;
    if (m_blocking)
        return;
    m_blocking = true;
    SpiralShape::SpiralType type = static_cast<SpiralShape::SpiralType>(widget.spiralType->currentIndex());

    if (m_command && commandIsValid()) {
        m_command->setSpiralType(type);
        m_command->setDirection(widget.clockWise->currentIndex() == 0);
        m_command->setFade(widget.fade->value());
        m_spiral->update();
        m_spiral->setType(type);
        m_spiral->setClockWise(widget.clockWise->currentIndex() == 0);
        m_spiral->setFade(widget.fade->value());
        m_spiral->update();
    } else {
        m_command = new SpiralShapeConfigCommand(m_spiral, type,
                (widget.clockWise->currentIndex() == 0), widget.fade->value());
        m_canvas->addCommand(m_command);
    }
    m_time.restart();
    m_blocking = false;
}

bool SpiralShapeConfigWidget::commandIsValid() const
{
    if (!m_time.isValid())
        return false;

    if (m_time.elapsed() > 4000)
        return false;

    return true;
}
#include <SpiralShapeConfigWidget.moc>
