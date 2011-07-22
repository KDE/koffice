/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KShapeBorderFactoryBase.h"

class KShapeBorderFactoryBase::Private
{
public:
    Private(const QString &i)
        : id(i),
        penStrokeConfigurable(false)
    {
    }

    const QString id;

    bool penStrokeConfigurable;
};

KShapeBorderFactoryBase::KShapeBorderFactoryBase(QObject *parent, const QString &id)
        : QObject(parent),
        d(new Private(id))
{
}

KShapeBorderFactoryBase::~KShapeBorderFactoryBase()
{
    delete d;
}

QString KShapeBorderFactoryBase::id() const
{
    return d->id;
}

bool KShapeBorderFactoryBase::penStrokeConfigurable() const
{
    return d->penStrokeConfigurable;
}

void KShapeBorderFactoryBase::setPenStrokeConfigurable(bool on)
{
    d->penStrokeConfigurable = on;
}

#include <KShapeBorderFactoryBase.moc>
