/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

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

#include "KEventActionRegistry.h"

#include <QHash>
#include <KoPluginLoader.h>
#include <kglobal.h>
#include <kdebug.h>

#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include "KEventActionFactoryBase.h"
#include "KEventAction.h"

class KEventActionRegistry::Singleton
{
public:
    Singleton()
            : initDone(false) {}

    KEventActionRegistry q;
    bool initDone;
};

K_GLOBAL_STATIC(KEventActionRegistry::Singleton, singleton)

class KEventActionRegistry::Private
{
public:
    QHash<QString, KEventActionFactoryBase*> presentationEventActionFactories;
    QHash<QString, KEventActionFactoryBase*> presentationEventActions;
    QHash<QString, KEventActionFactoryBase*> scriptEventActionFactories;
};

KEventActionRegistry * KEventActionRegistry::instance()
{
    KEventActionRegistry * registry = &(singleton->q);
    if (! singleton->initDone) {
        singleton->initDone = true;
        registry->init();
    }
    return registry;
}

KEventActionRegistry::KEventActionRegistry()
        : d(new Private())
{
}

KEventActionRegistry::~KEventActionRegistry()
{
    delete d;
}

void KEventActionRegistry::addPresentationEventAction(KEventActionFactoryBase * factory)
{
    const QString & action = factory->action();
    if (! action.isEmpty()) {
        d->presentationEventActionFactories.insert(factory->id(), factory);
        d->presentationEventActions.insert(action, factory);
    }
}

void KEventActionRegistry::addScriptEventAction(KEventActionFactoryBase * factory)
{
    d->scriptEventActionFactories.insert(factory->id(), factory);
}

QList<KEventActionFactoryBase *> KEventActionRegistry::presentationEventActions()
{
    return d->presentationEventActionFactories.values();
}

QList<KEventActionFactoryBase *> KEventActionRegistry::scriptEventActions()
{
    return d->scriptEventActionFactories.values();
}

void KEventActionRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "PresentationEventActionPlugins";
    config.blacklist = "PresentationEventActionPluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/PresentationEventAction"),
                                     QString::fromLatin1("[X-PresentationEventAction-MinVersion] <= 0"),
                                     config);

    config.whiteList = "ScriptEventActionPlugins";
    config.blacklist = "ScriptEventActionPluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/ScriptEventAction"),
                                     QString::fromLatin1("[X-ScriptEventAction-MinVersion] <= 0"),
                                     config);
}

QSet<KEventAction*> KEventActionRegistry::createEventActionsFromOdf(const KXmlElement & e, KShapeLoadingContext & context) const
{
    QSet<KEventAction *> eventActions;

    if (e.namespaceURI() == KOdfXmlNS::office && e.tagName() == "event-listeners") {
        KXmlElement element;
        forEachElement(element, e) {
            if (element.tagName() == "event-listener") {
                if (element.namespaceURI() == KOdfXmlNS::presentation) {
                    QString action(element.attributeNS(KOdfXmlNS::presentation, "action", QString()));
                    QHash<QString, KEventActionFactoryBase *>::const_iterator it(d->presentationEventActions.find(action));

                    if (it != d->presentationEventActions.constEnd()) {
                        KEventAction * eventAction = it.value()->createEventAction();
                        if (eventAction) {
                            if (eventAction->loadOdf(element, context)) {
                                eventActions.insert(eventAction);
                            } else {
                                delete eventAction;
                            }
                        }
                    } else {
                        kWarning(30006) << "presentation:event-listerer action = " << action << "not supported";
                    }
                } else if (element.namespaceURI() == KOdfXmlNS::script) {
                    // TODO
                } else {
                    kWarning(30006) << "element" << e.namespaceURI() << e.tagName() << "not supported";
                }
            } else {
                kWarning(30006) << "element" << e.namespaceURI() << e.tagName() << "not supported";
            }
        }
    } else {
        kWarning(30006) << "office:event-listeners not found got:" << e.namespaceURI() << e.tagName();
    }

    return eventActions;
}

