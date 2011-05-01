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

#include "KCFunctionModuleRegistry.h"

#include "KCFunction.h"
#include "KCFunctionRepository.h"

#include <KDebug>
#include <KGlobal>
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <KStandardDirs>

class KCFunctionModuleRegistry::Private
{
public:
    void registerFunctionModule(KCFunctionModule* module);
    void removeFunctionModule(KCFunctionModule* module);

public:
    bool repositoryInitialized;
};

void KCFunctionModuleRegistry::Private::registerFunctionModule(KCFunctionModule* module)
{
    const QList<QSharedPointer<KCFunction> > functions = module->functions();
    for (int i = 0; i < functions.count(); ++i) {
        KCFunctionRepository::self()->add(functions[i]);
    }
    Q_ASSERT(!module->descriptionFileName().isEmpty());
    const KStandardDirs* dirs = KGlobal::activeComponent().dirs();
    const QString fileName = dirs->findResource("functions", module->descriptionFileName());
    if (fileName.isEmpty()) {
        kDebug(36002) << module->descriptionFileName() << "not found.";
        return;
    }
    KCFunctionRepository::self()->loadFunctionDescriptions(fileName);
}

void KCFunctionModuleRegistry::Private::removeFunctionModule(KCFunctionModule* module)
{
    const QList<QSharedPointer<KCFunction> > functions = module->functions();
    for (int i = 0; i < functions.count(); ++i) {
        KCFunctionRepository::self()->remove(functions[i]);
    }
}


KCFunctionModuleRegistry::KCFunctionModuleRegistry()
        : d(new Private)
{
    d->repositoryInitialized = false;
}

KCFunctionModuleRegistry::~KCFunctionModuleRegistry()
{
    delete d;
}

KCFunctionModuleRegistry* KCFunctionModuleRegistry::instance()
{
    K_GLOBAL_STATIC(KCFunctionModuleRegistry, s_instance)
    return s_instance;
}

void KCFunctionModuleRegistry::loadFunctionModules()
{
    const quint32 minKCellsVersion = KOFFICE_MAKE_VERSION(2, 1, 0);
    const QString serviceType = QLatin1String("KCells/Plugin");
    const QString query = QLatin1String("([X-KCells-InterfaceVersion] == 0) and "
                                        "([X-KDE-PluginInfo-Category] == 'KCFunctionModule')");
    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, query);
    const KConfigGroup moduleGroup = KGlobal::config()->group("Plugins");
    const KPluginInfo::List pluginInfos = KPluginInfo::fromServices(offers, moduleGroup);
    kDebug(36002) << pluginInfos.count() << "function modules found.";
    foreach(KPluginInfo pluginInfo, pluginInfos) {
        pluginInfo.load(); // load activation state
        KPluginLoader loader(*pluginInfo.service());
        // Let's be paranoid: do not believe the service type.
        if (loader.pluginVersion() < minKCellsVersion) {
            kDebug(36002) << pluginInfo.name()
            << "was built against KCells" << loader.pluginVersion()
            << "; required version >=" << minKCellsVersion;
            continue;
        }
        if (pluginInfo.isPluginEnabled() && !contains(pluginInfo.pluginName())) {
            // Plugin enabled, but not registered. Add it.
            KPluginFactory* const factory = loader.factory();
            if (!factory) {
                kDebug(36002) << "Unable to create plugin factory for" << pluginInfo.name();
                continue;
            }
            KCFunctionModule* const module = factory->create<KCFunctionModule>(this);
            if (!module) {
                kDebug(36002) << "Unable to create function module for" << pluginInfo.name();
                continue;
            }
            add(pluginInfo.pluginName(), module);

            // Delays the function registration until the user needs one.
            if (d->repositoryInitialized) {
                d->registerFunctionModule(module);
            }
        } else if (!pluginInfo.isPluginEnabled() && contains(pluginInfo.pluginName())) {
            // Plugin disabled, but registered. Remove it.
            KCFunctionModule* const module = get(pluginInfo.pluginName());
            // Delay the function registration until the user needs one.
            if (d->repositoryInitialized) {
                d->removeFunctionModule(module);
            }
            remove(pluginInfo.pluginName());
            if (module->isRemovable()) {
                delete module;
                delete loader.factory();
                loader.unload();
            } else {
                // Put it back in.
                add(pluginInfo.pluginName(), module);
                // Delay the function registration until the user needs one.
                if (d->repositoryInitialized) {
                    d->registerFunctionModule(module);
                }
            }
        }
    }
}

void KCFunctionModuleRegistry::registerFunctions()
{
    d->repositoryInitialized = true;
    const QList<KCFunctionModule*> modules = values();
    for (int i = 0; i < modules.count(); ++i) {
        d->registerFunctionModule(modules[i]);
    }
}
