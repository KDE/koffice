/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (c) 2006-2010 Thomas Zander <zander@kde.org>
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

#ifndef PLUGIN_LOADER_P_H
#define PLUGIN_LOADER_P_H

#include <QObject>
#include <QStringList>

class PluginLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Config object for load()
     * It is possible to limit which plugins will be loaded in the KConfig configuration file by
     * stating explicitly which plugins are wanted.
     */
    struct PluginsConfig {
        PluginsConfig() : group(0), whiteList(0), blacklist(0) {}
        /**
         * The properties are retrieved from the config using the following construct;
         * /code
         *  KConfigGroup configGroup = KGlobal::config()->group(config.group);
         * /endcode
         * For most cases you can pass the string "koffice" into this variable.
         */
        const char * group;
        /// This contains the variable name for the list of plugins (by library name) the user wants to load
        const char * whiteList;
        /// This contains the variable name for the list of plugins (by library name) that will not be loaded
        const char * blacklist;
        /// A registry can state it wants to load a default set of plugins instead of all plugins
        /// when the application starts the first time.  Append all such plugin (library) names to this list.
        QStringList defaults;
    };

    ~PluginLoader();

    /**
     * Return an instance of the PluginLoader
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static PluginLoader * instance();

    /**
     * Load all plugins that conform to the versiontype and versionstring,
     * for instance:
     * PluginLoader::instance()->load("KOffice/Flake", "([X-Flake-Version] == 3)");
     * This method allows you to optionally limit the plugins that are loaded by version, but also
     * using a user configurable set of config options.
     * If you pass a PluginsConfig struct only those plugins are loaded that are specified in the
     * application config file.  New plugins found since last start will be automatically loaded.
     * @param serviceType The string used to identify the plugins.
     * @param versionString A string match that allows you to check for a specific version
     * @param config when passing a valid config only the wanted plugins are actually loaded
     * @return a list of services (by library name) that were not know in the config
     */
    void load(const QString &serviceType, const QString &versionString = QString(), const PluginsConfig &config = PluginsConfig());

private:
    PluginLoader();
    PluginLoader(const PluginLoader&);
    PluginLoader operator=(const PluginLoader&);

private:
    QStringList m_loadedServiceTypes;
};

#endif
