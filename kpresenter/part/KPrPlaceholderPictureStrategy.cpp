/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KPrPlaceholderPictureStrategy.h"

#include <QString>
#include <KFileDialog>
#include <KUrl>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShape.h>

KPrPlaceholderPictureStrategy::KPrPlaceholderPictureStrategy()
: KPrPlaceholderStrategy( "graphic" )
{
}

KPrPlaceholderPictureStrategy::~KPrPlaceholderPictureStrategy()
{
}

KoShape * KPrPlaceholderPictureStrategy::createShape( const QMap<QString, KoDataCenter *> & dataCenterMap )
{
    KoShape * shape = 0;

    KUrl url = KFileDialog::getOpenUrl();
    if ( !url.isEmpty() ) {
        shape = KPrPlaceholderStrategy::createShape( dataCenterMap );

        KoImageCollection * collection = dynamic_cast<KoImageCollection *>( dataCenterMap.value( "ImageCollection" ) );
        KoImageData * data = collection->getImage( url );
        if ( data ) {
            shape->setUserData( data );
            // TODO th pic should be fit into the space provided
            shape->setSize( data->imageSize() );
        }
    }
    return shape;
}
