/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoToolRegistry.h"
#include <k3staticdeleter.h>

#include <KoPluginLoader.h>

KoToolRegistry::KoToolRegistry()
{
}

void KoToolRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Flake"),
                                      QString::fromLatin1("[X-Flake-MinVersion] <= 0"));
    config.whiteList = "ToolPlugins";
    config.blacklist = "ToolPluginsDisabled";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Tool"),
                                      QString::fromLatin1("[X-Flake-MinVersion] <= 0"));
}

KoToolRegistry::~KoToolRegistry()
{
}

// static
KoToolRegistry *KoToolRegistry::s_instance = 0;
static K3StaticDeleter<KoToolRegistry> staticToolRegistryDeleter;

KoToolRegistry* KoToolRegistry::instance()
{
    if (KoToolRegistry::s_instance == 0) {
        staticToolRegistryDeleter.setObject(s_instance, new KoToolRegistry());
        KoToolRegistry::s_instance->init();
    }
    return KoToolRegistry::s_instance;
}

#include "KoToolRegistry.moc"
