/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KarbonGradientItem.h"
#include <KoAbstractGradient.h>
#include <KoResourceChooser.h>
#include <KoStopGradient.h>

#include <kdebug.h>

#include <QtGui/QPainter>

KarbonGradientItem::KarbonGradientItem( KoAbstractGradient * gradient )
    : m_gradient( gradient )
{
    Q_ASSERT( m_gradient );
    //setSizeHint( QSize( 200, 16 ) );
    setIcon( QIcon( QPixmap::fromImage( thumbnail( QSize( 300, 20 ) ) ) ) );
}

QImage KarbonGradientItem::thumbnail( const QSize &thumbSize ) const
{
    QGradient * g = m_gradient->toQGradient();

    QLinearGradient paintGradient;
    paintGradient.setStops( g->stops() );
    paintGradient.setStart( QPointF( 0, 0 ) );
    paintGradient.setFinalStop( QPointF( thumbSize.width() - 1, 0 ) );

    QPixmap checker(8, 8);
    QPainter p(&checker);
    p.fillRect(0, 0, 4, 4, Qt::lightGray);
    p.fillRect(4, 0, 4, 4, Qt::darkGray);
    p.fillRect(0, 4, 4, 4, Qt::darkGray);
    p.fillRect(4, 4, 4, 4, Qt::lightGray);
    p.end();

    QImage image( thumbSize, QImage::Format_ARGB32 );
    QPainter painter( &image );
    painter.fillRect( QRect( 0, 0, image.width(), image.height() ), QBrush( checker ) );
    painter.fillRect( QRect( 0, 0, image.width(), image.height() ), QBrush( paintGradient ) );

    delete g;

    return image;
}

QVariant KarbonGradientItem::data( int role ) const
{
    if( role == KoResourceChooser::LargeThumbnailRole )
    {
        return thumbnail( QSize( 100,100 ) );
    }
    else
        return QTableWidgetItem::data( role );
}

KoAbstractGradient * KarbonGradientItem::gradient()
{
    return m_gradient;
}
