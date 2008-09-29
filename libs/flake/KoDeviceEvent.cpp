/* This file is part of the KDE project
 * Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoDeviceEvent.h"
#include "KoPointerEvent.h"
#include <QtGui/QApplication>

class KoDeviceEvent::Private
{
public:
    Private()
            : button(Qt::NoButton), buttons(Qt::NoButton) {}

    KoDeviceEvent::Type type;
    Qt::MouseButton button;
    Qt::MouseButtons buttons;
};

KoDeviceEvent::KoDeviceEvent(Type type)
        : QInputEvent(static_cast<QEvent::Type>(type)), m_event(0), d(new Private())
{
    modState = QApplication::keyboardModifiers();
}

KoDeviceEvent::~KoDeviceEvent()
{
    delete m_event;
    delete d;
}

Qt::MouseButton KoDeviceEvent::button() const
{
    return d->button;
}

Qt::MouseButtons KoDeviceEvent::buttons() const
{
    return d->buttons;
}

void KoDeviceEvent::setButton(Qt::MouseButton b)
{
    d->button = b;
}

void KoDeviceEvent::setButtons(Qt::MouseButtons b)
{
    d->buttons = b;
}
