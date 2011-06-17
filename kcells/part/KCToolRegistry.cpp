/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#include "KCToolRegistry.h"

#include "KCCellTool.h"
#include "KCCellToolFactory.h"

#include <KGlobal>
#include <KPluginInfo>
#include <KServiceTypeTrader>

#include <KToolRegistry.h>


class KCToolRegistry::Private
{
public:
};


KCToolRegistry::KCToolRegistry()
        : d(new Private)
{
    // Add the built-in cell tool.
    KToolRegistry::instance()->add(new KCCellToolFactory(this, "KCellsCellToolId"));
    // Load the tool plugins.
    loadTools();
}

KCToolRegistry::~KCToolRegistry()
{
    delete d;
}

KCToolRegistry* KCToolRegistry::instance()
{
    K_GLOBAL_STATIC(KCToolRegistry, s_instance)
    return s_instance;
}

void KCToolRegistry::loadTools()
{
    const QString serviceType = QLatin1String("KCells/Plugin");
    const QString query = QLatin1String("([X-KCells-InterfaceVersion] == 0) and "
                                        "([X-KDE-PluginInfo-Category] == 'Tool')");
    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, query);
    const KConfigGroup moduleGroup = KGlobal::config()->group("Plugins");
    const KPluginInfo::List pluginInfos = KPluginInfo::fromServices(offers, moduleGroup);
    foreach(KPluginInfo pluginInfo, pluginInfos) {
        KPluginFactory *factory = KPluginLoader(*pluginInfo.service()).factory();
        if (!factory) {
            kDebug(36002) << "Unable to create plugin factory for" << pluginInfo.name();
            continue;
        }
        KCCellToolFactory* toolFactory = factory->create<KCCellToolFactory>(this);
        if (!toolFactory) {
            kDebug(36002) << "Unable to create tool factory for" << pluginInfo.name();
            continue;
        }
        pluginInfo.load(); // load activation state
        if (pluginInfo.isPluginEnabled()) {
            // Tool already registered?
            if (KToolRegistry::instance()->contains(toolFactory->id())) {
                continue;
            }
            toolFactory->setIcon(pluginInfo.service()->icon());
            toolFactory->setPriority(10);
            toolFactory->setToolTip(pluginInfo.service()->comment());
            KToolRegistry::instance()->add(toolFactory);
        } else {
            // Tool not registered?
            if (!KToolRegistry::instance()->contains(toolFactory->id())) {
                continue;
            }
            delete KToolRegistry::instance()->value(toolFactory->id());
            KToolRegistry::instance()->remove(toolFactory->id());
        }
    }
}
