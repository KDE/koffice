/****************************************************************************
 ** Copyright (C) 2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Chart licenses may use this file in
 ** accordance with the KD Chart Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdchart for
 **   information about KDChart Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/

#include "KDChartBackgroundAttributes.h"
#include <QPixmap>

#include <KDABLibFakes>

#define d d_func()


using namespace KDChart;

class BackgroundAttributes::Private
{
    friend class KDChart::BackgroundAttributes;
public:
    Private();
private:
    bool visible;
    QBrush brush;
    BackgroundPixmapMode pixmapMode;
    QPixmap pixmap;
};

BackgroundAttributes::Private::Private() :
    visible( false ),
    pixmapMode( BackgroundAttributes::BackgroundPixmapModeNone )
{
}


BackgroundAttributes::BackgroundAttributes()
    : _d( new Private() )
{
}

BackgroundAttributes::BackgroundAttributes( const BackgroundAttributes& r )
    : _d( new Private( *r.d ) )
{
}

BackgroundAttributes & BackgroundAttributes::operator=( const BackgroundAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

bool BackgroundAttributes::operator==( const BackgroundAttributes& r ) const
{
    return ( isVisible() == r.isVisible() &&
	     brush() == r.brush() &&
	     pixmapMode() == r.pixmapMode() &&
	     pixmap().serialNumber() == r.pixmap().serialNumber() );
}


BackgroundAttributes::~BackgroundAttributes()
{
    delete _d; _d = 0;
}




void BackgroundAttributes::setVisible( bool visible )
{
    d->visible = visible;
}


bool BackgroundAttributes::isVisible() const
{
    return d->visible;
}

void BackgroundAttributes::setBrush( const QBrush &brush )
{
    d->brush = brush;
}

QBrush BackgroundAttributes::brush() const
{
    return d->brush;
}

void BackgroundAttributes::setPixmapMode( BackgroundPixmapMode mode )
{
    d->pixmapMode = mode;
}

BackgroundAttributes::BackgroundPixmapMode BackgroundAttributes::pixmapMode() const
{
    return d->pixmapMode;
}

void BackgroundAttributes::setPixmap( const QPixmap &backPixmap )
{
    d->pixmap = backPixmap;
}

QPixmap BackgroundAttributes::pixmap() const
{
    return d->pixmap;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::BackgroundAttributes& ba)
{
    dbg << "KDChart::BackgroundAttributes("
	<< "visible="<<ba.isVisible()
	<< "brush="<<ba.brush()
	<< "pixmapmode="<<ba.pixmapMode()
	<< "pixmap="<<ba.pixmap()
	<< ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */
