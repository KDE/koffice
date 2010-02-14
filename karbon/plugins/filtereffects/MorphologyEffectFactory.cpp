/* This file is part of the KDE project
 * Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MorphologyEffectFactory.h"
#include "MorphologyEffect.h"
#include "MorphologyEffectConfigWidget.h"

#include <KLocale>

MorphologyEffectFactory::MorphologyEffectFactory(QObject *parent)
        : KoFilterEffectFactoryBase(parent, MorphologyEffectId, i18n("Morphology"))
{
}

KoFilterEffect * MorphologyEffectFactory::createFilterEffect() const
{
    return new MorphologyEffect();
}

KoFilterEffectConfigWidgetBase * MorphologyEffectFactory::createConfigWidget() const
{
    return new MorphologyEffectConfigWidget();
}
