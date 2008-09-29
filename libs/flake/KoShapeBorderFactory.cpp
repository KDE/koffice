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

#include "KoShapeBorderFactory.h"

class KoShapeBorderFactory::Private
{
public:
    Private(const QString &i, const QString &n) : id(i), name(n) { }
    const QString id;
    const QString name;
};

KoShapeBorderFactory::KoShapeBorderFactory(QObject *parent, const QString &id, const QString &name)
        : QObject(parent),
        d(new Private(id, name))
{
}

KoShapeBorderFactory::~KoShapeBorderFactory()
{
    delete d;
}

QString KoShapeBorderFactory::name() const
{
    return d->name;
}

QString KoShapeBorderFactory::id() const
{
    return d->id;
}

#include "KoShapeBorderFactory.moc"
