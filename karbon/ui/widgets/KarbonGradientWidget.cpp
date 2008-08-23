/* This file is part of the KDE project
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KarbonGradientWidget.h"
#include "KarbonCursor.h"

#include <KoUniColorDialog.h>

#include <klocale.h>
#include <kiconloader.h>

#include <QLabel>
#include <QBitmap>
#include <QPaintEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QPointF>
#include <QRectF>
#include <QPainter>


#define midPoint_width 7
#define midPoint_height 10
static unsigned char midPoint_bits[] = {
   0x08, 0x08, 0x1c, 0x1c, 0x2a, 0x2a, 0x08, 0x08, 0x08, 0x08
};

#define colorStopBorder_width 11
#define colorStopBorder_height 11
static unsigned char colorStopBorder_bits[] = {
   0x20, 0x00, 0x50, 0x00, 0x50, 0x00, 0x88, 0x00, 0x88, 0x00, 0x04, 0x01,
   0x04, 0x01, 0x02, 0x02, 0x02, 0x02, 0x01, 0x04, 0xff, 0x07
};

#define colorStop_width 9
#define colorStop_height 10
static unsigned char colorStop_bits[] = {
   0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x38, 0x00, 0x38, 0x00, 0x7c, 0x00,
   0x7c, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xff, 0x01
};

KarbonGradientWidget::KarbonGradientWidget( QWidget* parent )
: QWidget( parent ), m_currentStop( -1 ), m_checkerPainter( 4 )
{
    // initialize the gradient with some sane values
    m_gradient.setColorAt( 0.0, Qt::white );
    m_gradient.setColorAt( 1.0, Qt::green );

    QPalette p = palette();
    p.setBrush(QPalette::Window, QBrush(Qt::NoBrush));
    // TODO: check if this is equivalent with the line underneath. It might need autoFillBackground = true
    //setBackgroundMode( Qt::NoBackground );

    setMinimumSize( 105, 35 );
}

KarbonGradientWidget::~KarbonGradientWidget()
{
}

void KarbonGradientWidget::setStops( const QGradientStops & stops )
{
    m_gradient.setStops( stops );
    update();
}

QGradientStops KarbonGradientWidget::stops() const
{
    return m_gradient.stops();
}

void KarbonGradientWidget::paintColorStop( QPainter& p, int x, const QColor& color )
{
    QBitmap bitmap = QBitmap::fromData( QSize(colorStop_width, colorStop_height), colorStop_bits );
    bitmap.setMask( bitmap );
    p.setPen( color );
    p.drawPixmap( x - 4, 1, bitmap );

    bitmap = QBitmap::fromData( QSize(colorStopBorder_width, colorStopBorder_height), colorStopBorder_bits );
    bitmap.setMask( bitmap );
    p.setPen( Qt::black );
    p.drawPixmap( x - 5, 1, bitmap );
}

void KarbonGradientWidget::paintMidPoint( QPainter& p, int x )
{
    QBitmap bitmap = QBitmap::fromData( QSize(midPoint_width, midPoint_height), midPoint_bits );
    bitmap.setMask( bitmap );
    p.setPen( Qt::black );
    p.drawPixmap( x - 3, 1, bitmap );
}

void KarbonGradientWidget::paintEvent( QPaintEvent* )
{
    int w = width() - 4;  // available width for gradient and points
    int h = height() - 7; // available height for gradient and points
    int ph = colorStopBorder_height + 2; // point marker height
    int gh = h - ph;       // gradient area height

    QPainter painter( this );

    m_gradient.setStart( QPointF( 2, 2 ) );
    m_gradient.setFinalStop( QPointF( width()-2, 2 ) );

    m_checkerPainter.paint( painter, QRectF( 2, 2, w, gh ) );

    painter.setBrush( QBrush( m_gradient ) );
    painter.drawRect( QRectF( 2, 2, w, gh ) );

    painter.setBrush( QBrush() );
    painter.setPen( palette().light().color() );

    // light frame around widget
    QRect frame( 1, 1, width()-2, height()-2 );
    painter.drawRect( frame );

    // light line between gradient and point area
    painter.drawLine( QLine( QPoint( 1, 3 + gh ), QPoint( width() - 1, 3 + gh ) ) );

    painter.setPen( colorGroup().dark() );
    // left-top frame around widget
    painter.drawLine( QPoint(), QPoint( 0, height() - 1 ) );
    painter.drawLine( QPoint(), QPoint( width() - 1, 0 ) );

    // right-bottom from around gradient
    painter.drawLine( QPoint( width() - 2, 2 ), QPoint( width() - 2, 2 + gh ) );
    painter.drawLine( QPoint( width() - 2, 2 + gh ), QPoint( 2, 2 + gh ) );

    // upper line around point area
    painter.drawLine( QPoint( 1, height() - 3 - ph ), QPoint( width() - 1, height() - 3 - ph ) );

    // right-bottom line around point area
    painter.drawLine( QPoint( width() - 2, height() - ph - 1 ), QPoint( width() - 2, height() - 2 ) );
    painter.drawLine( QPoint( width() - 2, height() - 2 ), QPoint( 2, height() - 2 ) );

    m_pntArea.setRect( 2, height() - ph - 2, w, ph );
    painter.fillRect( m_pntArea.x(), m_pntArea.y(), m_pntArea.width(), m_pntArea.height(), palette().window().color() );

    painter.setClipRect( m_pntArea.x(), m_pntArea.y(), m_pntArea.width(), m_pntArea.height() );
    painter.translate( m_pntArea.x(), m_pntArea.y() );

    QGradientStops colorStops = m_gradient.stops();
    if( colorStops.count() > 1 )
    {
        foreach( const QGradientStop & stop, colorStops )
            paintColorStop( painter, (int)( stop.first * m_pntArea.width() ), stop.second );
    }
}

void KarbonGradientWidget::mousePressEvent( QMouseEvent* e )
{
    if( ! m_pntArea.contains( e->x(), e->y() ) )
        return;

    QGradientStops colorStops = m_gradient.stops();

    m_currentStop = -1;

    int x = e->x() - m_pntArea.left();

    for( int i = colorStops.count() - 1; i >= 0; i-- )
    {
        int r = int( colorStops[i].first * m_pntArea.width() );
        if( ( x > r - 5 ) && ( x < r + 5 ) )
        {
            // found ramp point at position
            m_currentStop = i;
            if( e->button() == Qt::LeftButton )
                setCursor( KarbonCursor::horzMove() );
            return;
        }
    }
}

void KarbonGradientWidget::mouseReleaseEvent( QMouseEvent* e )
{
    if( e->button() == Qt::RightButton && m_currentStop >= 0 )
    {
        if( m_pntArea.contains( e->x(), e->y() ) )
        {
            QGradientStops colorStops = m_gradient.stops();
            int x = e->x() - m_pntArea.left();
            // check if we are still above the actual ramp point
            int r = int( colorStops[ m_currentStop ].first * m_pntArea.width() );
            if( ( x > r - 5 ) && ( x < r + 5 ) )
            {
                colorStops.remove( m_currentStop );
                m_gradient.setStops( colorStops );
                update();
                emit changed();
            }
        }
    }
    setCursor( QCursor( Qt::ArrowCursor ) );
}

void KarbonGradientWidget::mouseDoubleClickEvent( QMouseEvent* e )
{
    if( ! m_pntArea.contains( e->x(), e->y() ) )
        return;

    if( e->button() != Qt::LeftButton )
        return;

    QGradientStops colorStops = m_gradient.stops();

    if( m_currentStop >= 0 )
    {
        // ramp point hit -> change color
        KoColor oldColor;
        oldColor.fromQColor( colorStops[m_currentStop].second );

        KoUniColorDialog * d = new KoUniColorDialog( oldColor, this->topLevelWidget() );
        if( d->exec() == QDialog::Accepted )
        {
            colorStops[m_currentStop].second = d->color().toQColor();
            m_gradient.setStops( colorStops );
            update();
            emit changed();
        }
        delete d;
    }
    else if( m_currentStop == -1 )
    {
        KoColor newColor;
        newColor.fromQColor( colorStops[0].second );

        // no point hit -> create new color stop
        KoUniColorDialog * d = new KoUniColorDialog( newColor, this->topLevelWidget() );
        if( d->exec() == QDialog::Accepted )
        {
            m_gradient.setColorAt( (float)( e->x()-m_pntArea.left() ) / m_pntArea.width(), d->color().toQColor() );
            update();
            emit changed();
        }
        delete d;
    }
}

void KarbonGradientWidget::mouseMoveEvent( QMouseEvent* e )
{
    if( e->buttons() & Qt::RightButton )
        return;

    QGradientStops colorStops = m_gradient.stops();

    if( m_currentStop < 0 || m_currentStop >= colorStops.count() )
        return;

    qreal x = (qreal)( e->x() - m_pntArea.left() ) / (qreal)m_pntArea.width();

    // move ramp point
    qreal minX = m_currentStop > 0 ? colorStops[m_currentStop-1].first : 0.0f;
    qreal maxX = m_currentStop < colorStops.count()-1 ? colorStops[m_currentStop+1].first : 1.0f;

    // Clip the color stop between to others.
    x = qMin( x, maxX );
    x = qMax( x, minX );
    colorStops[m_currentStop].first = x;
    m_gradient.setStops( colorStops );
    update();
    emit changed();
}

#include "KarbonGradientWidget.moc"
