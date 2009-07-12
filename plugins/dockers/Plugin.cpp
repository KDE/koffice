/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "strokedocker/StrokeDockerFactory.h"
#include "shapeproperties/ShapePropertiesDockerFactory.h"
#ifdef TINY_KOFFICE
#   include "shadowdocker/ShadowDockerFactory.h"
#   include "shapeselector/ShapeSelectorFactory.h"
#endif
#include "shapecollection/KoShapeCollectionDocker.h"

#include <KoDockRegistry.h>

#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(kofficedockers, KGenericFactory<Plugin>( "koffice-dockers" ) )

Plugin::Plugin( QObject *parent, const QStringList& )
    : QObject(parent)
{
    Q_UNUSED(parent);
    KoDockRegistry::instance()->add( new StrokeDockerFactory() );
    KoDockRegistry::instance()->add( new ShapePropertiesDockerFactory() );
#ifdef TINY_KOFFICE
    KoDockRegistry::instance()->add( new ShadowDockerFactory() );
    KoDockRegistry::instance()->add( new ShapeSelectorFactory() );
#endif
    KoDockRegistry::instance()->add( new KoShapeCollectionDockerFactory() );
}

#include "Plugin.moc"

