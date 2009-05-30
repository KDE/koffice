/* This file is part of the KDE project
 * Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>
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

#include "KarbonCalligraphicShapeFactory.h"
#include "KarbonCalligraphicShape.h"

#include <klocale.h>


KarbonCalligraphicShapeFactory::KarbonCalligraphicShapeFactory( QObject *parent )
: KoShapeFactory( parent, KarbonCalligraphicShapeId, i18n( "A calligraphic shape" ) )
{
    setToolTip( i18n( "Calligraphic Shape" ) );
    setIcon("calligraphy");
    setLoadingPriority( 1 ); // TODO: to what should this be set??
}

KarbonCalligraphicShapeFactory::~KarbonCalligraphicShapeFactory()
{
}

KoShape * KarbonCalligraphicShapeFactory::createDefaultShape() const
{
    KarbonCalligraphicShape *path = new KarbonCalligraphicShape();

    // FIXME: add points
    path->setShapeId( KarbonCalligraphicShapeId );

    return path;
}

KoShape * KarbonCalligraphicShapeFactory::createShape( const KoProperties * params ) const
{
    Q_UNUSED(params);
    return createDefaultShape();
}

/*bool KarbonCalligraphicShapeFactory::supports(const KoXmlElement & e) const
{
    return ( e.localName() == "ellipse" || e.localName() == "circle" ) &&
           ( e.namespaceURI() == KoXmlNS::draw );
}

QList<KoShapeConfigWidgetBase*> KarbonCalligraphicShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append( new CalligraphicShapeConfigWidget() );
    return panels;
}*/

bool KarbonCalligraphicShapeFactory::hidden() const
{
    return true;
}

