/* This file is part of the KDE project
 *
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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
#include <kpluginfactory.h>
#include <SCPageEffectRegistry.h>

#include "diagonalwipe/DiagonalWipeEffectFactory.h"
#include "miscdiagonalwipe/MiscDiagonalWipeEffectFactory.h"
#include "boxwipe/BoxWipeEffectFactory.h"
#include "fourboxwipe/FourBoxWipeEffectFactory.h"
#include "barndoorwipe/BarnDoorWipeEffectFactory.h"
#include "veewipe/VeeWipeEffectFactory.h"
#include "barnveewipe/BarnVeeWipeEffectFactory.h"
#include "zigzagwipe/ZigZagWipeEffectFactory.h"
#include "barnzigzagwipe/BarnZigZagWipeEffectFactory.h"

K_PLUGIN_FACTORY(PluginFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(PluginFactory("SCPageEffect"))

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    SCPageEffectRegistry::instance()->add(new DiagonalWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new MiscDiagonalWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new BoxWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new FourBoxWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new BarnDoorWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new VeeWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new BarnVeeWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new ZigZagWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new BarnZigZagWipeEffectFactory());
}

#include "Plugin.moc"

