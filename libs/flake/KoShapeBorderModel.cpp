/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoShapeBorderModel.h"

class KoShapeBorderModel::Private
{
public:
    Private() : refCount(0) { }
    QAtomicInt refCount;
};

KoShapeBorderModel::KoShapeBorderModel()
        : d(new Private())
{
}

KoShapeBorderModel::~KoShapeBorderModel()
{
    delete d;
}

KoInsets KoShapeBorderModel::borderInsets(const KoShape *shape)
{
    KoInsets insets;
    borderInsets(shape, insets);
    return insets;
}

void KoShapeBorderModel::addUser()
{
    d->refCount.ref();
}

bool KoShapeBorderModel::removeUser()
{
    return d->refCount.deref();
}

int KoShapeBorderModel::useCount() const
{
    return d->refCount;
}

