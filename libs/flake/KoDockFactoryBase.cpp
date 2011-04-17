/* This file is part of the KDE project
   Copyright (C) 2010-2011 Thomas Zander <zander@kde.org>

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

#include "KoDockFactoryBase.h"

class KoDockFactoryBasePrivate
{
public:
    KoDockFactoryBasePrivate() {
        collapsable = true;
        defaultCollapsed = false;
        defaultDockPosition = KoDockFactoryBase::DockRight;
    }
    QString id;
    bool collapsable, defaultCollapsed;
    KoDockFactoryBase::DockPosition defaultDockPosition;
};

KoDockFactoryBase::KoDockFactoryBase(QObject *parent, const QString &dockerid)
    : QObject(parent),
    d_ptr(new KoDockFactoryBasePrivate)
{
    Q_D(KoDockFactoryBase);
    d->id = dockerid;
}

KoDockFactoryBase::~KoDockFactoryBase()
{
    Q_D(KoDockFactoryBase);
    delete d;
}

void KoDockFactoryBase::setDefaultDockPosition(DockPosition pos)
{
    Q_D(KoDockFactoryBase);
    d->defaultDockPosition = pos;
}

void KoDockFactoryBase::setIsCollapsable(bool collapsable)
{
    Q_D(KoDockFactoryBase);
    d->collapsable = collapsable;
}

void KoDockFactoryBase::setDefaultCollapsed(bool collapsed)
{
    Q_D(KoDockFactoryBase);
    d->defaultCollapsed = collapsed;
}

QString KoDockFactoryBase::id() const
{
    Q_D(const KoDockFactoryBase);
    return d->id;
}

KoDockFactoryBase::DockPosition KoDockFactoryBase::defaultDockPosition() const
{
    Q_D(const KoDockFactoryBase);
    return d->defaultDockPosition;
}

bool KoDockFactoryBase::isCollapsable() const
{
    Q_D(const KoDockFactoryBase);
    return d->collapsable;
}

bool KoDockFactoryBase::isDefaultCollapsed() const
{
    Q_D(const KoDockFactoryBase);
    return d->defaultCollapsed;
}

