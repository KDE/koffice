/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
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

#include "KoToolDockerFactory.h"

class KoToolDockerFactory::Private {
public:
    Private( int id ) : dockWidgetId(id) {}
    int dockWidgetId;
};

KoToolDockerFactory::KoToolDockerFactory( int dockWidgetId )
: d( new Private(dockWidgetId))
{
}

KoToolDockerFactory::~KoToolDockerFactory()
{
    delete d;
}

QString KoToolDockerFactory::id() const
{
    if (d->dockWidgetId==0)
        return QString("KoToolOptionsDocker");
    else
        return QString("KoToolOptionsDocker %1").arg(d->dockWidgetId+1);
}

QDockWidget* KoToolDockerFactory::createDockWidget()
{
    KoToolDocker * dockWidget = new KoToolDocker();
    dockWidget->setObjectName( id() );
    return dockWidget;
}
