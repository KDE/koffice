/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
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

#include "irisWipe/SCIrisWipeEffectFactory.h"
#include "triangleWipe/SCTriangleWipeEffectFactory.h"
#include "arrowHeadWipe/SCArrowHeadWipeEffectFactory.h"
#include "ellipseWipe/SCEllipseWipeEffectFactory.h"
#include "roundRectWipe/SCRoundRectWipeEffectFactory.h"
#include "pentagonWipe/SCPentagonWipeEffectFactory.h"
#include "hexagonWipe/SCHexagonWipeEffectFactory.h"
#include "starWipe/SCStarWipeEffectFactory.h"
#include "eyeWipe/SCEyeWipeEffectFactory.h"
#include "miscShapeWipe/SCMiscShapeWipeEffectFactory.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(PluginFactory("SCPageEffect"))

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    SCPageEffectRegistry::instance()->add(new SCIrisWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCTriangleWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCArrowHeadWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCArrowHeadWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCEllipseWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCRoundRectWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCPentagonWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCHexagonWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCStarWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCEyeWipeEffectFactory());
    SCPageEffectRegistry::instance()->add(new SCMiscShapeWipeEffectFactory());
}

#include "Plugin.moc"
