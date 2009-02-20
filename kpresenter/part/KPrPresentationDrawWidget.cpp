/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include <QtGui/QVBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMenu>

#include <QMenu>
#include <QIcon>

#include <klocale.h>
#include <KoPACanvas.h>
#include <KoPointerEvent.h>
#include <kdebug.h>

#include <QList>

#include "KPrPresentationDrawWidget.h"

KPrPresentationDrawWidget::KPrPresentationDrawWidget(KoPACanvas * canvas) : QWidget(canvas)
{
    setFocusPolicy( Qt::StrongFocus );
    setMouseTracking( true );
    m_size = canvas->size();

    resize( m_size );
    
    m_draw = false;
    m_penSize = 10;
    m_penColor = Qt::black;
}

KPrPresentationDrawWidget::~KPrPresentationDrawWidget()
{
}

void KPrPresentationDrawWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter( this );    
    QBrush brush( Qt::SolidPattern );
    QPen pen( brush, m_penSize, Qt::CustomDashLine, Qt::RoundCap, Qt::RoundJoin );
    painter.setPen( pen );
    foreach (const Path &path, m_pointVectors){
        pen.setColor( path.color );
        pen.setWidth( path.size );
        painter.setPen( pen );
        painter.drawPolyline( QPolygonF( path.points ) );
    }
}

void KPrPresentationDrawWidget::mousePressEvent( QMouseEvent* e )
{
    struct Path path;
    path.color = m_penColor;
    path.size = m_penSize;
    path.points = QVector<QPointF>() << e->pos();
    m_pointVectors.append( path );
    m_draw = true;
}

void KPrPresentationDrawWidget::mouseMoveEvent( QMouseEvent* e )
{
    if(m_draw)
    {
	m_pointVectors.last().points << e->pos();
	update();
    }
}

void KPrPresentationDrawWidget::mouseReleaseEvent( QMouseEvent* e )
{
    m_draw = false;
}

void KPrPresentationDrawWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu( this );
    
    QMenu *color = new QMenu( QString( "Color of the pen"), this );
    QMenu *size = new QMenu( QString( "Size of the pen"), this );

    color->addAction ( buildActionColor( Qt::black, "&Black" ) );
    color->addAction ( buildActionColor( Qt::white, "&White" ) );
    color->addAction ( buildActionColor( Qt::green, "&Green" ) );
    color->addAction ( buildActionColor( Qt::red, "&Red" ) );
    color->addAction ( buildActionColor( Qt::blue, "&Blue" ) );
    color->addAction ( buildActionColor( Qt::yellow, "&Yellow" ) );
    connect(color, SIGNAL(triggered(QAction*)), this, SLOT(updateColor(QAction*)));


    size->addAction( buildActionSize ( 9 ) );
    size->addAction( buildActionSize ( 10 ) );
    size->addAction( buildActionSize ( 12 ) );
    size->addAction( buildActionSize ( 14 ) );
    size->addAction( buildActionSize ( 16 ) );
    size->addAction( buildActionSize ( 18 ) );
    size->addAction( buildActionSize ( 20 ) );
    size->addAction( buildActionSize ( 22 ) );

    connect(size, SIGNAL(triggered(QAction*)), this, SLOT(updateSize(QAction*)));
    
    menu.addMenu( color );
    menu.addMenu( size );
    
    menu.exec(event->globalPos());
    m_draw = false;
}

QAction* KPrPresentationDrawWidget::buildActionSize ( int size ){
    QAction *action = new QAction( buildIconSize( size ), QString::number(size)+"px", this );
    action->setProperty( "size", size );
    return action;
}

QAction* KPrPresentationDrawWidget::buildActionColor ( QColor color, QString name )
{
    QAction *action;
    action = new QAction( buildIconColor ( color ) , name, this );
    action->setProperty("color", QVariant(color));
    return action;
}

QIcon KPrPresentationDrawWidget::buildIconSize ( int size )
{

//        QPen pen( brush, m_penSize, Qt::CustomDashLine, Qt::RoundCap, Qt::RoundJoin );
    QPen thumbPen ( Qt::black, Qt::MiterJoin );
    thumbPen.setCapStyle( Qt::RoundCap );
    thumbPen.setWidth ( size );
    QPixmap thumbPixmap ( QSize ( 26, 26 ) );
    thumbPixmap.fill ( );
    QPainter thumbPainter ( &thumbPixmap );
    thumbPainter.setPen ( thumbPen );
    thumbPainter.drawPoint ( 13, 13 );
    QIcon thumbIcon ( thumbPixmap );
    return thumbIcon;
}

QIcon KPrPresentationDrawWidget::buildIconColor ( QColor color )
{

    QPixmap thumbPixmap ( QSize ( 24, 20 ) );
    thumbPixmap.fill ( color );
    QIcon thumbIcon ( thumbPixmap );
    return thumbIcon;
}

void KPrPresentationDrawWidget::updateSize ( QAction *size )
{
    m_penSize = size->property("size").toInt();
    m_draw = false;
}

void KPrPresentationDrawWidget::updateColor ( QAction *color )
{
    m_penColor = color->property("color").value<QColor>();
    m_draw = false;
}
 
