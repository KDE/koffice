/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <math.h>

#include <qcursor.h>

#include <koPoint.h>
#include <koRect.h>

#include "karbon_part.h"
#include "karbon_view.h"
#include "vmtool_handle.h"
#include "vmtool_scale.h"
#include "vpainter.h"
#include "vpainterfactory.h"
#include "vtransformcmd.h"

VMToolScale* VMToolScale::s_instance = 0L;

VMToolScale::VMToolScale( KarbonPart* part )
	: VTool( part ), m_isDragging( false )
{
}

VMToolScale::~VMToolScale()
{
}

VMToolScale*
VMToolScale::instance( KarbonPart* part )
{
	if ( s_instance == 0L )
	{
		s_instance = new VMToolScale( part );
	}

	s_instance->m_part = part;
	return s_instance;
}

void
VMToolScale::setCursor( KarbonView* view ) const
{
	switch( VMToolHandle::instance( m_part )->activeNode() )
	{
	case NODE_LT:
	case NODE_RB:	view->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeFDiagCursor ) );
					break;
	case NODE_RT:
	case NODE_LB:	view->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeBDiagCursor ) );
					break;
	case NODE_LM:
	case NODE_RM:	view->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeHorCursor ) );
					break;
	case NODE_MT:
	case NODE_MB:	view->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeVerCursor ) );
					break;
	default:		view->canvasWidget()->viewport()->setCursor( QCursor( Qt::arrowCursor ) );
	}
}

void
VMToolScale::drawTemporaryObject( KarbonView* view )
{
	VPainter *painter = view->painterFactory()->editpainter();
	painter->setRasterOp( Qt::NotROP );

	KoPoint lp = view->canvasWidget()->viewportToContents( QPoint( m_lp.x(), m_lp.y() ) );

	KoRect rect = part()->document().selection().boundingBox();

	// already selected, so must be a handle operation (move, scale etc.)
	if( !part()->document().selection().isEmpty() && VMToolHandle::instance( m_part )->activeNode() != NODE_MM )
	{
		setCursor( view );
		// scale operation
		QWMatrix mat;
		if( VMToolHandle::instance( m_part )->activeNode() == NODE_LT )
		{
			m_sp = KoPoint( rect.right(), rect.bottom() );
			m_s1 = ( rect.right() - lp.x() ) / double( rect.width() );
			m_s2 = ( rect.bottom() - lp.y() ) / double( rect.height() );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_MT )
		{
			m_sp = KoPoint( ( ( rect.right() + rect.left() ) / 2 ), rect.bottom() );
			m_s1 = ( rect.right() - lp.x() ) / double( rect.width() / 2 );
			m_s2 = ( rect.bottom() - lp.y() ) / double( rect.height() );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_RT )
		{
			m_sp = KoPoint( rect.x(), rect.bottom() );
			m_s1 = ( lp.x() - rect.x() ) / double( rect.width() );
			m_s2 = ( rect.bottom() - lp.y() ) / double( rect.height() );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_RM)
		{
			m_sp = KoPoint( rect.x(), ( rect.bottom() + rect.top() )  / 2 );
			m_s1 = ( lp.x() - rect.x() ) / double( rect.width() );
			m_s2 = ( rect.bottom() - lp.y() ) / double( rect.height() / 2 );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_RB )
		{
			m_sp = KoPoint( rect.x(), rect.y() );
			m_s1 = ( lp.x() - rect.x() ) / double( rect.width() );
			m_s2 = ( lp.y() - rect.y() ) / double( rect.height() );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_MB )
		{
			m_sp = KoPoint( ( ( rect.right() + rect.left() ) / 2 ), rect.y() );
			m_s1 = ( rect.right() - lp.x() ) / double( rect.width() / 2 );
			m_s2 = ( lp.y() - rect.y() ) / double( rect.height() );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_LB )
		{
			m_sp = KoPoint( rect.right(), rect.y() );
			m_s1 = ( rect.right() - lp.x() ) / double( rect.width() );
			m_s2 = ( lp.y() - rect.y() ) / double( rect.height() );
		}
		else if( VMToolHandle::instance( m_part )->activeNode() == NODE_LM )
		{
			m_sp = KoPoint( rect.right(), ( rect.bottom() + rect.top() )  / 2 );
			m_s1 = ( rect.right() - lp.x() ) / double( rect.width() );
			m_s2 = ( rect.bottom() - lp.y() ) / double( rect.height() / 2 );
		}
		KoPoint sp = KoPoint( m_sp.x() - view->canvasWidget()->contentsX(), m_sp.y() - view->canvasWidget()->contentsY() );
		mat.translate( sp.x() / view->zoom(), sp.y() / view->zoom());
		mat.scale( m_s1, m_s2 );
		mat.translate(	- ( sp.x() + view->canvasWidget()->contentsX() ) / view->zoom(),
						- ( sp.y() + view->canvasWidget()->contentsY() ) / view->zoom() );

		// TODO :  makes a copy of the selection, do assignment operator instead
		VObjectListIterator itr = part()->document().selection();
		VObjectList list;
		list.setAutoDelete( true );
	    for( ; itr.current() ; ++itr )
		{
			list.append( itr.current()->clone() );
		}
		painter->setZoomFactor( view->zoom() );
		VObjectListIterator itr2 = list;
		for( ; itr2.current() ; ++itr2 )
		{
			itr2.current()->transform( mat );
			itr2.current()->setState( state_edit );
			itr2.current()->draw( painter, itr2.current()->boundingBox() );
		}
		painter->setZoomFactor( 1.0 );
	}
	else
		m_isDragging = false;
}

bool
VMToolScale::eventFilter( KarbonView* view, QEvent* event )
{
	if ( event->type() == QEvent::MouseMove )
	{
		if( m_isDragging )
		{
			// erase old object:
			drawTemporaryObject( view );

			QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
			m_lp.setX( mouse_event->pos().x() );
			m_lp.setY( mouse_event->pos().y() );

			// paint new object:
			drawTemporaryObject( view );
		}
		else
		{
			VMToolHandle::instance( m_part )->eventFilter( view, event );
			setCursor( view );
		}

		return true;
	}

	if ( event->type() == QEvent::MouseButtonRelease && m_isDragging )
	{
		view->canvasWidget()->viewport()->setCursor( QCursor( Qt::arrowCursor ) );

		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		part()->addCommand(
			new VScaleCmd( &part()->document(), m_sp * ( 1.0 / view->zoom() ), m_s1, m_s2 ),
			true );

		m_isDragging = false;

		return true;
	}

	// handle pressing of keys:
	if ( event->type() == QEvent::KeyPress )
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*>( event );

		// cancel dragging with ESC-key:
		if ( key_event->key() == Qt::Key_Escape && m_isDragging )
		{
			m_isDragging = false;

			// erase old object:
			drawTemporaryObject( view );

			return true;
		}
	}

	// the whole story starts with this event:
	if ( event->type() == QEvent::MouseButtonPress )
	{
		view->painterFactory()->painter()->end();
		VMToolHandle::instance( m_part )->eventFilter( view, event );
		QMouseEvent* mouse_event = static_cast<QMouseEvent*>( event );
		m_fp.setX( mouse_event->pos().x() );
		m_fp.setY( mouse_event->pos().y() );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		// draw initial object:
		drawTemporaryObject( view );
		m_isDragging = true;

		return true;
	}

	return false;
}
