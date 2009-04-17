/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOVARIABLEREGISTRY_H
#define KOVARIABLEREGISTRY_H

#include <KoGenericRegistry.h>
#include "kotext_export.h"
#include <KoVariableFactory.h>
#include <QObject>
#include <QList>

#include "KoXmlReaderForward.h"
class KoShapeLoadingContext;
class KoVariable;
class KoCanvasBase;
class QAction;

/**
 * This singleton class keeps a register of all available variable factories.
 * @see KoVariableFactory
 * @see KoInlineTextObjectManager
 * @see KoInlineObject
 * @see KoVariable
 */
class KOTEXT_EXPORT KoVariableRegistry : public KoGenericRegistry<KoVariableFactory*>
{
public:
    class Singleton;

    /**
     * Return an instance of the KoVariableRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoVariableRegistry *instance();

    /**
     * Use the element to find out which variable plugin can load it, and returns the loaded
     * variable. The element expected is one of 'text:subject', 'text:date' / etc.
     *
     * @returns the variable or 0 if no variable could be created
     */
    KoVariable *createFromOdf(const KoXmlElement & e, KoShapeLoadingContext & context) const;

    /**
     * Create a list of actions that can be used to plug into a menu, for example.
     * This method will find all the InlineObjectFactories that are installed in the system and
     * find out which object they provide. If a factory provides a variable, then all its
     * templates will be added to the response.
     * Each of thse actions, when executed, will insert the relevant variable in the current text-position.
     * The actions assume that the text tool is selected, if thats not the case then they will silently fail.
     * @param host the canvas for which these actions are created.  Note that the actions will get these
     *  actions as a parent (for memory management purposes) as well.
     * @see KoInlineTextObjectManager::createInsertVariableActions()
     */
    QList<QAction*> createInsertVariableActions(KoCanvasBase *host) const;

private:
    static KoVariableRegistry *s_instance;

    KoVariableRegistry();
    ~KoVariableRegistry();

    void init();

    class Private;
    Private *const d;
};

#endif // KOVARIABLEREGISTRY_H
