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

#ifndef KCELLS_FUNCTION_MODULE
#define KCELLS_FUNCTION_MODULE

#include <kofficeversion.h>

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <KPluginFactory>

#include "kcells_export.h"

class KCFunction;

/**
 * \ingroup KCValue
 * A function module provides several KCFunction objects.
 */
class KCELLS_EXPORT KCFunctionModule : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates the function module.
     * The derived class should create here the KCFunction objects and
     * should register them via \ref add.
     */
    KCFunctionModule(QObject* parent);

    /**
     * Destroys the module and the provided KCFunction objects.
     * Check, if this module isRemovable(), before you unload the plugin.
     */
    virtual ~KCFunctionModule();

    /**
     * Returns the file name of the XML description for the functions.
     */
    virtual QString descriptionFileName() const = 0;

    /**
     * Returns a list of the provided KCFunction objects.
     */
    QList<QSharedPointer<KCFunction> > functions() const;

    /**
     * Checks wether this module can be removed, because none of its
     * KCFunction objects is in use.
     * Used by the KCFunctionModuleRegistry to check, if the plugin can be unloaded.
     * \return \c true on success; \c false on failure
     */
    bool isRemovable();

    /**
     * Returns the identifier (if defined). KCFunction of the KGenericRegistry
     * template, that has to be implemented.
     */
    virtual QString id() const;

protected:
    /**
     * Adds \p function to the list of provided KCFunction objects.
     */
    void add(KCFunction* function);

private:
    class Private;
    Private * const d;
};

/**
* Register a function module when it is contained in a loadable plugin
*/
#define KCELLS_EXPORT_FUNCTION_MODULE(libname, classname) \
    K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
    K_EXPORT_PLUGIN(factory("kcells-functions-" #libname)) \
    K_EXPORT_PLUGIN_VERSION(KOFFICE_VERSION)

#endif // KCELLS_FUNCTION_MODULE
