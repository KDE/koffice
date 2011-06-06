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

#include "RectangleShapeConfigWidget.h"
#include "RectangleShape.h"
#include "RectangleShapeConfigCommand.h"

#include <KoCanvasBase.h>

RectangleShapeConfigWidget::RectangleShapeConfigWidget(KoCanvasBase *canvas)
    : m_canvas(canvas),
    m_command(0),
    m_blocking(false)
{
    widget.setupUi(this);

    connect(widget.cornerRadiusX, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
    connect(widget.cornerRadiusY, SIGNAL(valueChanged(double)), this, SLOT(propertyChanged()));
}

void RectangleShapeConfigWidget::setUnit(const KoUnit &unit)
{
    widget.cornerRadiusX->setUnit(unit);
    widget.cornerRadiusY->setUnit(unit);
}

void RectangleShapeConfigWidget::open(KoShape *shape)
{
    if (m_blocking)
        return;
    RectangleShape *rect = dynamic_cast<RectangleShape*>(shape);
    if (rect != m_rectangle) // if its equal, we just update our values
        m_command = 0;
    m_rectangle = 0;
    if (!rect) {
        setEnabled(false);
        return;
    }
    setEnabled(true);

    m_blocking = true;
    QSizeF size = rect->size();

    widget.cornerRadiusX->setMaximum(0.5 * size.width());
    widget.cornerRadiusX->changeValue(0.01 * rect->cornerRadiusX() * 0.5 * size.width());
    widget.cornerRadiusY->setMaximum(0.5 * size.height());
    widget.cornerRadiusY->changeValue(0.01 * rect->cornerRadiusY() * 0.5 * size.height());
    m_blocking = false;

    m_rectangle = rect;
}

void RectangleShapeConfigWidget::propertyChanged()
{
    if (!m_rectangle)
        return;
    if (m_blocking)
        return;
    m_blocking = true;
    m_time.restart();
    const QSizeF size = m_rectangle->size();
    const qreal radiusX = 100 * widget.cornerRadiusX->value() / (0.5 * size.width());
    const qreal radiusY = 100 * widget.cornerRadiusY->value() / (0.5 * size.height());
    if (m_command && commandIsValid()) {
        m_command->setCornerRadiusX(radiusX);
        m_command->setCornerRadiusY(radiusY);
        m_rectangle->update();
        m_rectangle->setCornerRadiusX(radiusX);
        m_rectangle->setCornerRadiusY(radiusY);
        m_rectangle->update();
    } else {
        m_command = new RectangleShapeConfigCommand(m_rectangle, radiusX, radiusY);
        m_canvas->addCommand(m_command);
    }

    m_blocking = false;
}

bool RectangleShapeConfigWidget::commandIsValid() const
{
    if (!m_time.isValid())
        return false;

    if (m_time.elapsed() > 4000)
        return false;

    return true;
}

#include <RectangleShapeConfigWidget.moc>
