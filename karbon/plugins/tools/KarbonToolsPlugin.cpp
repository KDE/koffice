/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KarbonToolsPlugin.h"
#include "KarbonPencilToolFactory.h"
#include "CalligraphyTool/KarbonCalligraphyToolFactory.h"
#include "CalligraphyTool/KarbonCalligraphicShapeFactory.h"
#include "KarbonGradientToolFactory.h"
#include "KarbonPatternToolFactory.h"

#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>

#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY( KarbonToolsPluginFactory, registerPlugin<KarbonToolsPlugin>(); )
K_EXPORT_PLUGIN( KarbonToolsPluginFactory("KarbonTools") )

KarbonToolsPlugin::KarbonToolsPlugin( QObject *parent, const QVariantList& )
    : QObject(parent)
{
    KoToolRegistry::instance()->add( new KarbonPencilToolFactory( parent ) );
    KoToolRegistry::instance()->add( new KarbonCalligraphyToolFactory( parent ) );
    KoToolRegistry::instance()->add( new KarbonGradientToolFactory( parent ) );
    KoToolRegistry::instance()->add( new KarbonPatternToolFactory( parent ) );
    
    KoShapeRegistry::instance()->add( new KarbonCalligraphicShapeFactory( parent ) );
}

#include "KarbonToolsPlugin.moc"
