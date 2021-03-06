/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KFilterEffectRegistry.h"
#include "KFilterEffect.h"
#include <KoPluginLoader.h>
#include <KGlobal>
#include <KXmlReader.h>

KFilterEffectRegistry::KFilterEffectRegistry()
{
}

void KFilterEffectRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FilterEffectPlugins";
    config.blacklist = "FilterEffectPluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/FilterEffect"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);
}


KFilterEffectRegistry::~KFilterEffectRegistry()
{
}

KFilterEffectRegistry* KFilterEffectRegistry::instance()
{
    K_GLOBAL_STATIC(KFilterEffectRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

KFilterEffect * KFilterEffectRegistry::createFilterEffectFromXml(const KXmlElement & element, const KFilterEffectLoadingContext &context)
{
    KFilterEffectFactoryBase * factory = get(element.tagName());
    if (!factory)
        return 0;

    KFilterEffect * filterEffect = factory->createFilterEffect();
    if (filterEffect->load(element, context))
        return filterEffect;

    delete filterEffect;
    return 0;
}

#include <KFilterEffectRegistry.moc>
