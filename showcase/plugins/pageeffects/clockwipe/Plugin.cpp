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
#include <kpluginfactory.h>
#include <SCPageEffectRegistry.h>

#include "clockwipe/SCClockWipeEffectFactory.h"
#include "pinwheelwipe/SCPinWheelWipeEffectFactory.h"
#include "singlesweepwipe/SCSingleSweepWipeEffectFactory.h"
#include "fanwipe/SCFanWipeEffectFactory.h"
#include "doublefanwipe/SCDoubleFanWipeEffectFactory.h"
#include "doublesweepwipe/SCDoubleSweepWipeEffectFactory.h"
#include "saloondoorwipe/SCSaloonDoorWipeEffectFactory.h"
#include "windshieldwipe/SCWindShieldWipeEffectFactory.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(PluginFactory("SCPageEffect"))

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    SCPageEffectRegistry::instance()->add(new SCClockWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCPinWheelWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCSingleSweepWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCFanWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCDoubleFanWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCDoubleSweepWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCSaloonDoorWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCWindShieldWipeEffectFactory());

}

#include "Plugin.moc"

