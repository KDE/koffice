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

#ifndef KEVENTACTIONREGISTRY_H
#define KEVENTACTIONREGISTRY_H

#include <QList>
#include <QSet>

#include "flake_export.h"

class KEventAction;
class KEventActionFactoryBase;
class KShapeLoadingContext;
class KXmlElement;

/**
 * This singletion keeps a register of all available KEventActionFactoryBase objects.
 *
 * It creates the event actions when loaded from odf.
 */
class FLAKE_EXPORT KEventActionRegistry
{
public:
    class Singleton;

    ~KEventActionRegistry();

    /**
     * Return an instance of the KEventActionRegistry
     */
    static KEventActionRegistry *instance();

    /**
     * Create action events for the elements given
     */
    QSet<KEventAction*> createEventActionsFromOdf(const KXmlElement &element, KShapeLoadingContext &context) const;

    /**
     * Add presentation event action.
     */
    void addPresentationEventAction(KEventActionFactoryBase *factory);

    /**
     * Add script event action.
     */
    void addScriptEventAction(KEventActionFactoryBase *factory);

    /**
     * Get presentation event actions.
     */
    QList<KEventActionFactoryBase *> presentationEventActions();

    /**
     * Get script event actions.
     */
    QList<KEventActionFactoryBase *> scriptEventActions();

private:
    KEventActionRegistry();
    KEventActionRegistry(const KEventActionRegistry &);
    KEventActionRegistry operator=(const KEventActionRegistry &);

    void init();

    class Private;
    Private *d;
};

#endif /* KOEVENTACTIONREGISTRY_H */
