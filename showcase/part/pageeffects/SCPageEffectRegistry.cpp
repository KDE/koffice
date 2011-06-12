/* This file is part of the KDE project
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>

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

#include "SCPageEffectRegistry.h"

#include <QString>

#include <kglobal.h>
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KoPluginLoader.h>
#include <pageeffects/SCPageEffectFactory.h>
#include <kdebug.h>

class SCPageEffectRegistry::Singleton
{
public:
    Singleton()
    : initDone(false)
    {
    }

    SCPageEffectRegistry q;
    bool initDone;
};

struct SCPageEffectRegistry::Private
{
    QHash<QPair<QString, bool>, SCPageEffectFactory *> tagToFactory;
};

K_GLOBAL_STATIC(SCPageEffectRegistry::Singleton, singleton)

SCPageEffectRegistry * SCPageEffectRegistry::instance()
{
    SCPageEffectRegistry * registry = &(singleton->q);
    if (! singleton->initDone) {
        singleton->initDone = true;
        registry->init();
    }
    return registry;
}

SCPageEffect * SCPageEffectRegistry::createPageEffect(const KoXmlElement &element)
{
    Q_UNUSED(element);

    SCPageEffect * pageEffect = 0;
    if (element.hasAttributeNS(KOdfXmlNS::smil, "type")) {
        QString smilType(element.attributeNS(KOdfXmlNS::smil, "type"));
        bool reverse = false;
        if (element.hasAttributeNS(KOdfXmlNS::smil, "direction") && element.attributeNS(KOdfXmlNS::smil, "direction") == "reverse") {
            reverse = true;
        }

        QHash<QPair<QString, bool>, SCPageEffectFactory *>::iterator it(d->tagToFactory.find(QPair<QString, bool>(smilType, reverse)));

        // call the factory to create the page effect 
        if (it != d->tagToFactory.end()) {
            pageEffect = it.value()->createPageEffect(element);
        }
        else {
            kWarning(33002) << "page effect of smil:type" << smilType << "not supported";
        }
    }
    // return it
    return pageEffect;
}

SCPageEffectRegistry::SCPageEffectRegistry()
: d(new Private())
{
}

SCPageEffectRegistry::~SCPageEffectRegistry()
{
    foreach (SCPageEffectFactory* factory, values())
    {
        delete factory;
    }
    delete d;
}

void SCPageEffectRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "PageEffectPlugins";
    config.blacklist = "PageEffectPluginsDisabled";
    config.group = "showcase";

    // XXX: Use minversion here?
    // The plugins are responsible for adding a factory to the registry
    KoPluginLoader::instance()->load(QString::fromLatin1("Showcase/PageEffect"),
            QString::fromLatin1("[X-Showcase-Version] <= 0"),
            config);

    QList<SCPageEffectFactory*> factories = values();

    foreach (SCPageEffectFactory * factory, factories) {
        QList<QPair<QString, bool> > tags(factory->tags());
        QList<QPair<QString, bool> >::iterator it(tags.begin());
        for (; it != tags.end(); ++it) {
            d->tagToFactory.insert(*it, factory);
        }
    }
}
