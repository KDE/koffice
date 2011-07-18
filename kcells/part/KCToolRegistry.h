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

#ifndef KC_TOOL_REGISTRY
#define KC_TOOL_REGISTRY

#include <QObject>

#include "kcells_export.h"


/**
 * Registry for tools.
 * \ingroup Plugin
 */
class KCELLS_EXPORT KCToolRegistry : public QObject
{
public:
    /**
     * Creates the registry.
     */
    KCToolRegistry();
    ~KCToolRegistry();

    /**
     * \return the singleton instance
     */
    static KCToolRegistry* instance();

    /**
     * Loads the tools.
     * Depending on their activation state read from the config,
     * the tools are added or removed from the registry.
     */
    void loadTools();

private:
    class Private;
    Private * const d;
};

#endif // KC_TOOL_REGISTRY
