/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
   
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

#include "KPointerEvent.h"
#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>

class KPointerEvent::Private
{
public:
    Private()
            : tabletEvent(0), mouseEvent(0), wheelEvent(0),
            tabletButton(Qt::NoButton),
            globalPos(0, 0), pos(0, 0), posZ(0), rotationX(0), rotationY(0),
            rotationZ(0) {}
    QTabletEvent * tabletEvent;
    QMouseEvent * mouseEvent;
    QWheelEvent * wheelEvent;
    Qt::MouseButton tabletButton;
    QPoint globalPos, pos;
    int posZ;
    int rotationX, rotationY, rotationZ;
};

KPointerEvent::KPointerEvent(QMouseEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->mouseEvent = ev;
}

KPointerEvent::KPointerEvent(QTabletEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->tabletEvent = ev;
}

KPointerEvent::KPointerEvent(QWheelEvent *ev, const QPointF &pnt)
        : point(pnt),
        m_event(ev),
        d(new Private())
{
    Q_ASSERT(m_event);
    d->wheelEvent = ev;
}

KPointerEvent::KPointerEvent(KPointerEvent *event, const QPointF &point)
    : point(point)
    , m_event(event->m_event)
    , d(new Private(*(event->d)))
{
    Q_ASSERT(m_event);
}

KPointerEvent::~KPointerEvent()
{
    delete d;
}

Qt::MouseButton KPointerEvent::button() const
{
    if (d->mouseEvent)
        return d->mouseEvent->button();
    else if (d->tabletEvent)
        return d->tabletButton;
    else
        return Qt::NoButton;
}

Qt::MouseButtons KPointerEvent::buttons() const
{
    if (d->mouseEvent)
        return d->mouseEvent->buttons();
    else if (d->wheelEvent)
        return d->wheelEvent->buttons();
    else if (d->tabletEvent)
        return d->tabletButton;
    return Qt::NoButton;
}

QPoint KPointerEvent::globalPos() const
{
    if (d->mouseEvent)
        return d->mouseEvent->globalPos();
    else if (d->wheelEvent)
        return d->wheelEvent->globalPos();
    else if (d->tabletEvent)
        return d->tabletEvent->globalPos();
    else
        return d->globalPos;
}

QPoint KPointerEvent::pos() const
{
    if (d->mouseEvent)
        return d->mouseEvent->pos();
    else if (d->wheelEvent)
        return d->wheelEvent->pos();
    else if (d->tabletEvent)
        return d->tabletEvent->pos();
    else
        return d->pos;
}

qreal KPointerEvent::pressure() const
{
    if (d->tabletEvent)
        return d->tabletEvent->pressure();
    else
        return 0.5;
}

qreal KPointerEvent::rotation() const
{
    if (d->tabletEvent)
        return d->tabletEvent->rotation();
    else
        return 0.0;
}

qreal KPointerEvent::tangentialPressure() const
{
    if (d->tabletEvent)
        return d->tabletEvent->tangentialPressure();
    else
        return 0.0;
}

int KPointerEvent::x() const
{
    if (d->tabletEvent)
        return d->tabletEvent->x();
    if (d->wheelEvent)
        return d->wheelEvent->x();
    else if (d->mouseEvent)
        return d->mouseEvent->x();
    else
        return pos().x();
}

int KPointerEvent::xTilt() const
{
    if (d->tabletEvent)
        return d->tabletEvent->xTilt();
    else
        return 0;
}

int KPointerEvent::y() const
{
    if (d->tabletEvent)
        return d->tabletEvent->y();
    if (d->wheelEvent)
        return d->wheelEvent->y();
    else if (d->mouseEvent)
        return d->mouseEvent->y();
    else
        return pos().y();
}

int KPointerEvent::yTilt() const
{
    if (d->tabletEvent)
        return d->tabletEvent->yTilt();
    else
        return 0;
}

int KPointerEvent::z() const
{
    if (d->tabletEvent)
        return d->tabletEvent->z();
    else
        return 0;
}

int KPointerEvent::delta() const
{
    if (d->wheelEvent)
        return d->wheelEvent->delta();
    else
        return 0;
}

int KPointerEvent::rotationX() const
{
    return d->rotationX;
}

int KPointerEvent::rotationY() const
{
    return d->rotationY;
}

int KPointerEvent::rotationZ() const
{
    return d->rotationZ;
}

Qt::Orientation KPointerEvent::orientation() const
{
    if (d->wheelEvent)
        return d->wheelEvent->orientation();
    else
        return Qt::Horizontal;
}

void KPointerEvent::setTabletButton(Qt::MouseButton button)
{
    d->tabletButton = button;
}

Qt::KeyboardModifiers KPointerEvent::modifiers() const
{
    if (d->tabletEvent)
        return d->tabletEvent->modifiers();
    else if (d->mouseEvent)
        return d->mouseEvent->modifiers();
    else if (d->wheelEvent)
        return d->wheelEvent->modifiers();
    else
        return Qt::NoModifier;
}
