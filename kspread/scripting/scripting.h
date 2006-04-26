/*
 * This file is part of KSpread
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2006 ISaac Clerencia <isaac@warp.es>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <kparts/plugin.h>
#include <kspread_view.h>

namespace Kross {
    namespace Api {
        class ScriptGUIClient;
        class ScriptAction;
    }
}

class Scripting : public KParts::Plugin
{
    Q_OBJECT
    public:
        Scripting(QObject *parent, const char *name, const QStringList &);
        virtual ~Scripting();
    private slots:
        void executionFinished(const Kross::Api::ScriptAction*);
    private:
        KSpread::View * m_view;
        Kross::Api::ScriptGUIClient* m_scriptguiclient;
};


#endif
