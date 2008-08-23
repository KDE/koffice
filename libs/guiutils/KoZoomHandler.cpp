/* This file is part of the KDE project
   Copyright (C) 2001-2005 David Faure <faure@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoZoomHandler.h"
#include <kdebug.h>
#include <KoUnit.h> // for POINT_TO_INCH
#include <KoGlobal.h>


KoZoomHandler::KoZoomHandler()
    : KoViewConverter()
{
    // Note that this calls the method below, not the derived one
    setZoomAndResolution( 100, KoGlobal::dpiX(), KoGlobal::dpiY() );
    setZoomMode( KoZoomMode::ZOOM_CONSTANT );
}

KoZoomHandler::~KoZoomHandler()
{
}

void KoZoomHandler::setZoomAndResolution( int zoom, int dpiX, int dpiY )
{
    // m_resolution[XY] is in pixel per pt
    m_resolutionX = POINT_TO_INCH( static_cast<qreal>(dpiX) );
    m_resolutionY = POINT_TO_INCH( static_cast<qreal>(dpiY) );
    setZoom( zoom / 100.0 );
    /*kDebug(30004) <<"KoZoomHandler::setZoomAndResolution" << zoom <<"" << dpiX <<"," << dpiY
              << " m_resolutionX=" << m_resolutionX
              << " m_zoomedResolutionX=" << m_zoomedResolutionX
              << " m_resolutionY=" << m_resolutionY
              << " m_zoomedResolutionY=" << m_zoomedResolutionY << endl;*/
}

void KoZoomHandler::setResolutionToStandard()
{
    setResolution( POINT_TO_INCH( qreal(KoGlobal::dpiX())),
                   POINT_TO_INCH( qreal(KoGlobal::dpiY())) );
}

void KoZoomHandler::setResolution( qreal resolutionX, qreal resolutionY )
{
    m_resolutionX = resolutionX;
    m_resolutionY = resolutionY;
    m_zoomedResolutionX = m_zoom * resolutionX;
    m_zoomedResolutionY = m_zoom * resolutionY;
}

void KoZoomHandler::setZoomedResolution( qreal zoomedResolutionX, qreal zoomedResolutionY )
{
    // m_zoom doesn't matter, it's only used in setZoom() to calculated the zoomed resolutions
    // Here we know them. The whole point of this method is to allow a different zoom factor
    // for X and for Y, as can be useful for e.g. fullscreen kpresenter presentations.
    m_zoomedResolutionX = zoomedResolutionX;
    m_zoomedResolutionY = zoomedResolutionY;
}

void KoZoomHandler::setZoom( qreal zoom )
{
    m_zoom = zoom;
    if( m_zoom == 1.0 ) {
        m_zoomedResolutionX = m_resolutionX;
        m_zoomedResolutionY = m_resolutionY;
    } else {
        m_zoomedResolutionX = m_zoom * m_resolutionX;
        m_zoomedResolutionY = m_zoom * m_resolutionY;
    }
}

QPointF KoZoomHandler::documentToView( const QPointF &documentPoint )  const
{
    return QPointF( zoomItX( documentPoint.x() ),
                    zoomItY( documentPoint.y() ));
}

QPointF KoZoomHandler::viewToDocument( const QPointF &viewPoint )  const
{
    return QPointF( unzoomItX( viewPoint.x() ),
                    unzoomItY( viewPoint.y() ) );
}

QRectF KoZoomHandler::documentToView( const QRectF &documentRect )  const
{
    QRectF r (zoomItX( documentRect.x() ),
              zoomItY( documentRect.y() ),
              zoomItX( documentRect.width() ),
              zoomItY( documentRect.height() ) );
    return r;
}

QRectF KoZoomHandler::viewToDocument( const QRectF &viewRect )  const
{
    QRectF r (  unzoomItX( viewRect.x() ),
                unzoomItY( viewRect.y()),
                unzoomItX( viewRect.width() ),
                unzoomItY( viewRect.height() ) );
    return r;
}

QSizeF KoZoomHandler::documentToView( const QSizeF &documentSize ) const
{
    return QSizeF( zoomItX( documentSize.width() ),
                   zoomItY( documentSize.height() ) );
}

QSizeF KoZoomHandler::viewToDocument( const QSizeF &viewSize ) const
{
    return QSizeF( unzoomItX( viewSize.width() ),
                   unzoomItY( viewSize.height() ) );
}

qreal KoZoomHandler::documentToViewX( qreal documentX ) const
{
    return zoomItX( documentX );
}

qreal KoZoomHandler::documentToViewY( qreal documentY ) const
{
    return zoomItY( documentY );
}

qreal KoZoomHandler::viewToDocumentX( qreal viewX ) const
{
    return unzoomItX( viewX );
}

qreal KoZoomHandler::viewToDocumentY( qreal viewY ) const
{
    return unzoomItY( viewY );
}

void KoZoomHandler::zoom(qreal *zoomX, qreal *zoomY) const
{
    *zoomX = zoomItX(100.0) / 100.0;
    *zoomY = zoomItY(100.0) / 100.0;
}
