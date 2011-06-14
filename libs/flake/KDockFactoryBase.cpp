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

#include "KDockFactoryBase.h"

class KoDockFactoryBasePrivate
{
public:
    KoDockFactoryBasePrivate() {
        collapsable = true;
        defaultCollapsed = false;
        defaultDockPosition = KDockFactoryBase::DockRight;
    }
    QString id;
    bool collapsable, defaultCollapsed;
    KDockFactoryBase::DockPosition defaultDockPosition;
};

KDockFactoryBase::KDockFactoryBase(QObject *parent, const QString &dockerid)
    : QObject(parent),
    d_ptr(new KoDockFactoryBasePrivate)
{
    Q_D(KDockFactoryBase);
    d->id = dockerid;
}

KDockFactoryBase::~KDockFactoryBase()
{
    Q_D(KDockFactoryBase);
    delete d;
}

void KDockFactoryBase::setDefaultDockPosition(DockPosition pos)
{
    Q_D(KDockFactoryBase);
    d->defaultDockPosition = pos;
}

void KDockFactoryBase::setIsCollapsable(bool collapsable)
{
    Q_D(KDockFactoryBase);
    d->collapsable = collapsable;
}

void KDockFactoryBase::setDefaultCollapsed(bool collapsed)
{
    Q_D(KDockFactoryBase);
    d->defaultCollapsed = collapsed;
}

QString KDockFactoryBase::id() const
{
    Q_D(const KDockFactoryBase);
    return d->id;
}

KDockFactoryBase::DockPosition KDockFactoryBase::defaultDockPosition() const
{
    Q_D(const KDockFactoryBase);
    return d->defaultDockPosition;
}

bool KDockFactoryBase::isCollapsable() const
{
    Q_D(const KDockFactoryBase);
    return d->collapsable;
}

bool KDockFactoryBase::isDefaultCollapsed() const
{
    Q_D(const KDockFactoryBase);
    return d->defaultCollapsed;
}

