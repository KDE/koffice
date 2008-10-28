/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "KPrPlaceholderShapeFactory.h"

#include <klocale.h>

#include <KoXmlNS.h>
#include "KPrPlaceholderShape.h"

#include <kdebug.h>

KPrPlaceholderShapeFactory::KPrPlaceholderShapeFactory( QObject *parent )
: KoShapeFactory( parent, KPrPlaceholderShapeId, i18n( "Placeholder shape" ) )
{
    QStringList elementNames;
    elementNames << "text-box" << "object" << "image";
    setOdfElementNames( KoXmlNS::draw, elementNames );
    // use a really high number as we want to be used before the normal shapes try to load it
    setLoadingPriority( 1000 );
}

KPrPlaceholderShapeFactory::~KPrPlaceholderShapeFactory()
{
}

KoShape * KPrPlaceholderShapeFactory::createDefaultShape() const
{
    return new KPrPlaceholderShape();
}

KoShape * KPrPlaceholderShapeFactory::createShape(const KoProperties * params) const
{
    Q_UNUSED( params );
    return createDefaultShape();
}

bool KPrPlaceholderShapeFactory::supports(const KoXmlElement & e) const
{
    // check parent if placeholder is set to true
    KoXmlNode parent = e.parentNode();
    if ( !parent.isNull() ) {
        KoXmlElement element = parent.toElement();
        if ( !element.isNull() ) {
            kDebug(33001) << "placeholder:" << ( element.attributeNS( KoXmlNS::presentation, "placeholder", "false" ) == "true" ); 
            return ( element.attributeNS( KoXmlNS::presentation, "placeholder", "false" ) == "true" );
        }
    }
    return false;
}

bool KPrPlaceholderShapeFactory::hidden() const
{
    return true;
}
