/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "Plugin.h"
#include <kgenericfactory.h>
#include <KPrPageEffectRegistry.h>

#include "snakewipe/KPrSnakeWipeEffectFactory.h"
#include "spiralwipe/KPrSpiralWipeEffectFactory.h"
#include "parallelsnakes/KPrParallelSnakesWipeEffectFactory.h"
#include "boxsnakes/KPrBoxSnakesWipeEffectFactory.h"
#include "waterfallwipe/KPrWaterfallWipeEffectFactory.h"

K_EXPORT_COMPONENT_FACTORY( kpr_pageeffect_matrixwipe, KGenericFactory<Plugin>( "KPrPageEffect" ) )

Plugin::Plugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    KPrPageEffectRegistry::instance()->add(new KPrSnakeWipeEffectFactory());
    KPrPageEffectRegistry::instance()->add(new KPrSpiralWipeEffectFactory());
    KPrPageEffectRegistry::instance()->add(new KPrParallelSnakesWipeEffectFactory());
    KPrPageEffectRegistry::instance()->add(new KPrBoxSnakesWipeEffectFactory());
    KPrPageEffectRegistry::instance()->add(new KPrWaterfallWipeEffectFactory());
}

#include "Plugin.moc"

