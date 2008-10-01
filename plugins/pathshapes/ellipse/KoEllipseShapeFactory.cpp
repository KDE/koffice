/* This file is part of the KDE project
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoEllipseShapeFactory.h"
#include "KoEllipseShape.h"
#include "EllipseShapeConfigWidget.h"
#include <KoLineBorder.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoGradientBackground.h>

#include <klocale.h>


KoEllipseShapeFactory::KoEllipseShapeFactory( QObject *parent )
: KoShapeFactory( parent, KoEllipseShapeId, i18n( "Ellipse" ) )
{
    setToolTip( i18n( "An ellipse" ) );
    setIcon("ellipse-shape");
    setFamily("geometric");
    QStringList elementNames;
    elementNames << "ellipse" << "circle";
    setOdfElementNames( KoXmlNS::draw, elementNames );
    setLoadingPriority( 1 );
}

KoShape * KoEllipseShapeFactory::createDefaultShape() const
{
    KoEllipseShape * ellipse = new KoEllipseShape();

    ellipse->setBorder( new KoLineBorder( 1.0 ) );
    ellipse->setShapeId( KoPathShapeId );

    QRadialGradient * gradient = new QRadialGradient( QPointF(50,50), 50.0, QPointF(25,25) );
    gradient->setColorAt( 0.0, Qt::white );
    gradient->setColorAt( 1.0, Qt::green );
    ellipse->setBackground( new KoGradientBackground( gradient ) );

    return ellipse;
}

KoShape * KoEllipseShapeFactory::createShape( const KoProperties * params ) const
{
    Q_UNUSED(params);
    return createDefaultShape();
}

bool KoEllipseShapeFactory::supports(const KoXmlElement & e) const
{
    return ( e.localName() == "ellipse" || e.localName() == "circle" ) &&
           ( e.namespaceURI() == KoXmlNS::draw );
}

QList<KoShapeConfigWidgetBase*> KoEllipseShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append( new EllipseShapeConfigWidget() );
    return panels;
}
