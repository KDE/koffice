/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include <QString>

#include <kdebug.h>
#include <kstaticdeleter.h>

#include "KoPluginLoader.h"
#include "KoShapeRegistry.h"
#include "KoPathShapeFactory.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlReader.h"

KoShapeRegistry::KoShapeRegistry()
{
}

void KoShapeRegistry::init() {
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Flake"),
                                      QString::fromLatin1("[X-Flake-Version] == 1"), config);
    config.whiteList = "ShapePlugins";
    config.blacklist = "ShapePluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Shape"),
                                     QString::fromLatin1("[X-Flake-Version] == 1"), config);

    // Also add our hard-coded basic shape
    add( new KoPathShapeFactory(this, QStringList()) );
}


KoShapeRegistry::~KoShapeRegistry()
{
}

KoShapeRegistry *KoShapeRegistry::m_singleton = 0;
static KStaticDeleter<KoShapeRegistry> staticShapeRegistryDeleter;

KoShapeRegistry* KoShapeRegistry::instance()
{
    if(KoShapeRegistry::m_singleton == 0)
    {
        staticShapeRegistryDeleter.setObject(m_singleton, new KoShapeRegistry());
        KoShapeRegistry::m_singleton->init();
    }
    return KoShapeRegistry::m_singleton;
}

KoShape * KoShapeRegistry::createShapeFromOdf(KoShapeLoadingContext *context, KoXmlElement e) const
{
    Q_UNUSED( context );
    Q_UNUSED( e );
    return 0;
}

#include "KoShapeRegistry.moc"
