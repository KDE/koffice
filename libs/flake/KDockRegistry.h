/* This file is part of the KDE project
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

#ifndef KDOCKREGISTRY_H
#define KDOCKREGISTRY_H

#include "KGenericRegistry.h"
#include "KDockFactoryBase.h"
#include "flake_export.h"

#include <QObject>

/**
 * This singleton class keeps a register of all available dockers,
 * or rather, of the factories that can create the QDockWidget instances
 * for the mainwindows.
 * Note that adding your KDockFactoryBase to this registry will mean it will automatically be
 * added to an application, no extra code is required for that.
 *
 * @see KCanvasObserverBase
 */
class FLAKE_EXPORT KDockRegistry : public QObject, public KGenericRegistry<KDockFactoryBase*>
{
    Q_OBJECT

public:
    ~KDockRegistry();

    /**
     * Return an instance of the KDockRegistry
     * Create a new instance on first call and return the singleton.
     */
    static KDockRegistry *instance();

private:
    KDockRegistry();
    KDockRegistry(const KDockRegistry&);
    KDockRegistry operator=(const KDockRegistry&);
    void init();

    class Private;
    Private *d;
};

#endif
