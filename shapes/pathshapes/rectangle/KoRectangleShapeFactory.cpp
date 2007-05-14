/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoRectangleShapeFactory.h"
#include "KoRectangleShape.h"
#include "KoLineBorder.h"
#include <KoXmlNS.h>
#include <KoXmlReader.h>

#include <klocale.h>

KoRectangleShapeFactory::KoRectangleShapeFactory( QObject *parent )
: KoShapeFactory( parent, KoRectangleShapeId, i18n( "A rectangle shape" ) )
{
    setToolTip( i18n( "A rectangle" ) );
    setIcon("rectangle-koffice");
    setOdfElementName( KoXmlNS::draw, "rect" );
    setLoadingPriority( 1 );
}

KoShape * KoRectangleShapeFactory::createDefaultShape() const
{
    KoRectangleShape * rect = new KoRectangleShape();

    rect->setBorder( new KoLineBorder( 1.0 ) );
    rect->setShapeId( KoPathShapeId );

    QLinearGradient gradient( QPointF(0,0), QPointF(100,100) );
    gradient.setColorAt( 0.0, Qt::white );
    gradient.setColorAt( 1.0, Qt::green );
    rect->setBackground( QBrush( gradient ) );

    return rect;
}

KoShape * KoRectangleShapeFactory::createShape( const KoProperties * params ) const 
{
    Q_UNUSED(params);
    return new KoRectangleShape();
}

bool KoRectangleShapeFactory::supports(const KoXmlElement & e) const
{
    if( e.localName() == "rect" && e.namespaceURI() == KoXmlNS::draw )
        return true;
    else
        return false;
}
