/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCEventActionData.h"

class SCEventActionData::Private
{
public:
    Private(KoShape *s, KEventAction *ea, SCSoundCollection *sc)
            : shape(s),
            eventAction(ea),
            soundCollection(sc)
    {
    }

    KoShape *shape;
    KEventAction *eventAction;
    SCSoundCollection *soundCollection;
};

SCEventActionData::SCEventActionData(KoShape * shape, KEventAction * eventAction , SCSoundCollection * soundCollection)
    : d(new Private(shape, eventAction, soundCollection))
{
}

SCEventActionData::~SCEventActionData()
{
    delete d;
}

SCSoundCollection * SCEventActionData::soundCollection() const
{
    return d->soundCollection;
}

KoShape *SCEventActionData::shape() const
{
    return d->shape;
}

KEventAction *SCEventActionData::eventAction() const
{
    return d->eventAction;
}
