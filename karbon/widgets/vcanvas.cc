/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qcursor.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "karbon_view_base.h"
#include "karbon_part_base.h"
#include "vcanvas.h"
#include "vdocument.h"
#include "vpainter.h"
#include "vqpainter.h"
#include "vpainterfactory.h"
#include "vselection.h"

#include <kdebug.h>

int
VCanvas::pageOffsetX() const
{
	double zoomedWidth = m_part->document().width() * m_view->zoom();
	if( contentsWidth() < viewport()->width() )
	{
		//kdDebug() << "offsetx : " << int( ( viewport()->width() - zoomedWidth ) / 2.0 ) << endl;
		return int( ( viewport()->width() - zoomedWidth ) / 2.0 );
	}
	else
		return int( ( contentsWidth() - zoomedWidth ) / 2.0 );
}

int
VCanvas::pageOffsetY() const
{
	double zoomedHeight = m_part->document().height() * m_view->zoom();
	if( contentsHeight() < viewport()->height() )
	{
		//kdDebug() << "offsetx : " << int( ( viewport()->height() - zoomedHeight ) / 2.0 ) << endl;
		return int( ( viewport()->height() - zoomedHeight ) / 2.0 );
	}
	else
		return int( ( contentsHeight() - zoomedHeight ) / 2.0 );
}

VCanvas::VCanvas( QWidget *parent, KarbonViewBase* view, KarbonPartBase* part )
    : QScrollView( parent, "canvas", WStaticContents/*WNorthWestGravity*/ | WResizeNoErase  |
	  WRepaintNoErase ), m_part( part ), m_view( view )
{
	connect(this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( slotContentsMoving( int, int ) ) );
	viewport()->setFocusPolicy( QWidget::StrongFocus );

	viewport()->setMouseTracking( true );
	setMouseTracking( true );

	viewport()->setBackgroundColor( Qt::white );
	viewport()->setBackgroundMode( QWidget::NoBackground );
	viewport()->installEventFilter( this );

	resizeContents( 800, 600 );
	m_pixmap = new QPixmap( 800, 600 );

	setFocus();

	m_bScrolling = false;
}

VCanvas::~VCanvas()
{
	delete m_pixmap;
}

void
VCanvas::setPos( const KoPoint& p )
{
	QCursor::setPos( int( p.x() * m_view->zoom() ), int( p.y() * m_view->zoom() ) );
}

bool
VCanvas::eventFilter( QObject* object, QEvent* event )
{
	QScrollView::eventFilter( object, event );

	if( event->type() == QEvent::AccelOverride || event->type() == QEvent::Accel )
		return QScrollView::eventFilter( object, event );

	if( event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease )
		return m_view->keyEvent( event );

	QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );

	KoPoint canvasCoordinate = toContents( KoPoint( mouseEvent->pos() ) );
	if( mouseEvent && m_view )
		return m_view->mouseEvent( mouseEvent, canvasCoordinate );
	else
		return false;
}


// This causes a repaint normally, so just overwriting it omits the repainting
void
VCanvas::focusInEvent( QFocusEvent * )
{
}

KoPoint
VCanvas::toViewport( const KoPoint &p ) const
{
	KoPoint p2 = p;
	p2.setX( ( p.x() * m_view->zoom() ) - contentsX() / m_view->zoom() + pageOffsetX() );
	if( contentsHeight() > height() )
		p2.setY( ( contentsHeight() - ( p.y() * m_view->zoom() + contentsY() + pageOffsetY() ) ) );
	else
		p2.setY( ( height() - p.y() * m_view->zoom() + pageOffsetY() ) );
	return p2;
}

KoPoint
VCanvas::toContents( const KoPoint &p ) const
{
	KoPoint p2 = p;
	p2.setX( ( p.x() + contentsX() - pageOffsetX() ) / m_view->zoom() );
	if( contentsHeight() > height() )
		p2.setY( ( contentsHeight() - ( p.y() + contentsY() + pageOffsetY()) ) / m_view->zoom() );
	else
		p2.setY( ( height() - p.y() - pageOffsetY() ) / m_view->zoom() );
	return p2;
}

KoRect
VCanvas::boundingBox() const
{
	KoPoint p1( 0, 0 );
	p1 = toContents( p1 );
	KoPoint p2( width(), height() );
	p2 = toContents( p2 );
	return KoRect( p1, p2 ).normalize();
}

void
VCanvas::setYMirroring( VPainter *p )
{
	QWMatrix mat;

	mat.scale( 1, -1 );
	mat.translate( pageOffsetX(), pageOffsetY() );

	if( contentsHeight() > height() )
		mat.translate( -contentsX(), contentsY() - contentsHeight() );
	else
		mat.translate( 0, -height() );

	p->setWorldMatrix( mat );
}

void
VCanvas::viewportPaintEvent( QPaintEvent *e )
{
	setYMirroring( m_view->painterFactory()->editpainter() );
	//kdDebug() << "viewp e->rect() : " << e->rect().x() << ", " << e->rect().y() << ", " << e->rect().width() << ", " << e->rect().height() << endl;
	viewport()->setUpdatesEnabled( false );
	KoRect rect( e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height() );
	VPainter *p = m_view->painterFactory()->painter();
	//if( m_bScrolling )
	//{
		// TODO : only update ROIs
		KoRect r( 0, 0, viewport()->width(), viewport()->height() );
		p->begin();
		p->clear( rect, QColor( 195, 194, 193 ) );
		p->setZoomFactor( m_view->zoom() );
		setYMirroring( p );
		// TRICK : slightly adjust the matrix so libart AA looks better
		QWMatrix mat = p->worldMatrix();
		p->setWorldMatrix( mat.translate( -.5, -.5 ) );

		// set up clippath
		p->newPath();
		p->moveTo( KoPoint( rect.x(), rect.y() ) );
		p->lineTo( KoPoint( rect.right(), rect.y() ) );
		p->lineTo( KoPoint( rect.right(), rect.bottom() ) );
		p->lineTo( KoPoint( rect.x(), rect.bottom() ) );
		p->lineTo( KoPoint( rect.x(), rect.y() ) );
		p->setClipPath();

		m_part->document().drawPage( p );
		KoRect r2 = boundingBox();
		m_part->document().draw( p, &r2 );

		p->resetClipPath();
		m_bScrolling = false;

	//}
	p->blit( rect );

	// draw handle:
	VQPainter qpainter( p->device() );
	setYMirroring( &qpainter );
	qpainter.setZoomFactor( m_view->zoom() );
	m_part->document().selection()->draw( &qpainter, m_view->zoom() );

	bitBlt( viewport(), QPoint( int( rect.x() ), int( rect.y() ) ), p->device(), rect.toQRect() );
	viewport()->setUpdatesEnabled( true );
	//bitBlt( this, QPoint( rect.x(), rect.y() - pageOffsetY() ), p->device(), rect );
}

void
VCanvas::setViewport( double centerX, double centerY )
{
	setContentsPos( int( centerX * contentsWidth() - visibleWidth() / 2 ),
					int( centerY * contentsHeight() - visibleHeight() / 2 ) );
}

void
VCanvas::setViewportRect( const KoRect &r )
{
	viewport()->setUpdatesEnabled( false );
	double zoomX = m_view->zoom() * ( ( visibleWidth() / m_view->zoom() ) / r.width() );
	double zoomY = m_view->zoom() * ( ( visibleHeight() / m_view->zoom() ) / r.height() );
	double centerX = double( ( r.center().x() ) * m_view->zoom() + pageOffsetX() ) / double( contentsWidth() );
	double centerY = double( ( r.center().y() ) * m_view->zoom() + pageOffsetY() ) / double( contentsHeight() );
	double zoom = zoomX < zoomY ? zoomX : zoomY;
	resizeContents( int( ( zoom / m_view->zoom() ) * contentsWidth() ),
					int( ( zoom / m_view->zoom() ) * contentsHeight() ) );
	setViewport( centerX, 1.0 - centerY );
	m_view->setZoomAt( zoom );
	viewport()->setUpdatesEnabled( true );
}

void
VCanvas::drawContents( QPainter* painter, int clipx, int clipy,
	int clipw, int cliph  )
{
	drawDocument( painter, KoRect( clipx, clipy, clipw, cliph ) );
}

void
VCanvas::drawDocument( QPainter* /*painter*/, const KoRect&, bool drawVObjects )
{
	setYMirroring( m_view->painterFactory()->editpainter() );
	//kdDebug() << "drawDoc rect : " << rect.x() << ", " << rect.y() << ", " << rect.width() << ", " << rect.height() << endl;
	VPainter* p = m_view->painterFactory()->painter();
	if( drawVObjects )
	{
		p->begin();
		p->clear( QColor( 195, 194, 193 ) );
		p->setZoomFactor( m_view->zoom() );
		setYMirroring( p );
		// TRICK : slightly adjust the matrix so libart AA looks better
		QWMatrix mat = p->worldMatrix();
		p->setWorldMatrix( mat.translate( -.5, -.5 ) );

		m_part->document().drawPage( p );
		KoRect r2 = boundingBox();
		m_part->document().draw( p, &r2 );

		p->end();
	}

	// draw handle:
	VQPainter qpainter( p->device() );
	setYMirroring( &qpainter );
	qpainter.setZoomFactor( m_view->zoom() );
	m_part->document().selection()->draw( &qpainter, m_view->zoom() );

	bitBlt( viewport(), 0, 0, p->device(), 0, 0, width(), height() );
}

void
VCanvas::repaintAll( bool drawVObjects )
{
	//if( m_view->layersDocker() )
	//	m_view->layersDocker()->updatePreviews();
	//drawContents( 0, 0, 0, width(), height() );
	drawDocument( 0, KoRect( 0, 0, width(), height() ), drawVObjects );
	//viewport()->repaint( erase );
}

/// repaints just a rect area (no scrolling)
void
VCanvas::repaintAll( const KoRect & )
{
	//if( m_view->layersDocker() )
//		m_view->layersDocker()->updatePreviews();
	VPainter *p = m_view->painterFactory()->painter();
	KoRect rect( rect().x(), rect().y(), rect().width(), rect().height() );
	p->blit( rect );

	// draw handle:
	VQPainter qpainter( p->device() );
	setYMirroring( &qpainter );
	qpainter.setZoomFactor( m_view->zoom() );
	m_part->document().selection()->draw( &qpainter, m_view->zoom() );

	bitBlt( viewport(), QPoint( int( rect.x() ), int( rect.y() ) ), p->device(), rect.toQRect() );
}

void
VCanvas::resizeEvent( QResizeEvent* event )
{
	double centerX = double( contentsX() + visibleWidth() / 2 ) / double( contentsWidth() );
	double centerY = double( contentsY() + visibleHeight() / 2 ) / double( contentsHeight() );

	QScrollView::resizeEvent( event );
	if( !m_pixmap )
		m_pixmap = new QPixmap( width(), height() );
	else
		m_pixmap->resize( width(), height() );

	VPainter *p = m_view->painterFactory()->painter();
	p->resize( width(), height() );
	p->clear( QColor( 195, 194, 193 ) );
	m_bScrolling = true;
	setViewport( centerX, centerY );
}

void
VCanvas::slotContentsMoving( int /*x*/, int /*y*/ )
{
	m_bScrolling = true;
	emit viewportChanged();
}

#include <vcanvas.moc>
