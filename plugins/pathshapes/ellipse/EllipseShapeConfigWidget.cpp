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

#include "EllipseShapeConfigWidget.h"
#include "EllipseShapeConfigCommand.h"
#include <KoCanvasBase.h>

#include <klocale.h>

EllipseShapeConfigWidget::EllipseShapeConfigWidget(KoCanvasBase *canvas)
    : m_canvas(canvas),
    m_blocking(false)
{
    widget.setupUi(this);

    widget.ellipseType->clear();
    widget.ellipseType->addItem(i18n("Arc"));
    widget.ellipseType->addItem(i18n("Pie"));
    widget.ellipseType->addItem(i18n("Chord"));

    widget.startAngle->setMinimum(0.0);
    widget.startAngle->setMaximum(360.0);

    widget.endAngle->setMinimum(0.0);
    widget.endAngle->setMaximum(360.0);

    connect(widget.ellipseType, SIGNAL(currentIndexChanged(int)), this, SLOT(propertyChanged()));
    connect(widget.startAngle, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
    connect(widget.endAngle, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
    connect(widget.closeEllipse, SIGNAL(clicked(bool)), this, SLOT(closeEllipse()));
}

void EllipseShapeConfigWidget::open(KoShape *shape)
{
    if (m_blocking)
        return;
    EllipseShape *es = dynamic_cast<EllipseShape*>(shape);
    if (es != m_ellipse) // if its equal, we just update our values
        m_command = 0;
    m_ellipse = 0;
    if (!es) {
        setEnabled(false);
        return;
    }
    setEnabled(true);

    m_blocking = true;
    widget.ellipseType->setCurrentIndex(es->type());
    widget.startAngle->setValue(es->startAngle());
    widget.endAngle->setValue(es->endAngle());
    m_blocking = false;

    m_ellipse = es;
}

void EllipseShapeConfigWidget::propertyChanged()
{
    if (!m_ellipse)
        return;
    if (m_blocking)
        return;
    m_blocking = true;
    EllipseShape::EllipseType type = static_cast<EllipseShape::EllipseType>(widget.ellipseType->currentIndex());
    if (m_command && commandIsValid()) {
        m_command->setType(type);
        m_command->setStartAngle(widget.startAngle->value());
        m_command->setEndAngle(widget.endAngle->value());
        m_ellipse->update();
        m_ellipse->setType(type);
        m_ellipse->setStartAngle(widget.startAngle->value());
        m_ellipse->setEndAngle(widget.endAngle->value());
        m_ellipse->update();
    } else {
        m_command = new EllipseShapeConfigCommand(m_ellipse, type,
                    widget.startAngle->value(), widget.endAngle->value());
        m_canvas->addCommand(m_command);
    }
    m_time.restart();
    m_blocking = false;
}

void EllipseShapeConfigWidget::closeEllipse()
{
    widget.startAngle->setValue(0.0);
    widget.endAngle->setValue(360.0);
}

bool EllipseShapeConfigWidget::commandIsValid() const
{
    if (!m_time.isValid())
        return false;

    if (m_time.elapsed() > 4000)
        return false;

    return true;
}

#include <EllipseShapeConfigWidget.moc>
