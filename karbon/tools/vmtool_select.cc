/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <koRect.h>

#include "karbon_part.h"
#include "karbon_view.h"
#include "vmtool_handle.h"
#include "vmtool_select.h"
#include "vpainterfactory.h"
#include "vpainter.h"
#include "vtransformcmd.h"

#include <kdebug.h>

#include <math.h>

VMToolSelect* VMToolSelect::s_instance = 0L;

VMToolSelect::VMToolSelect( KarbonPart* part )
	: VTool( part ), m_state( normal ), m_isDragging( false )
{
}

VMToolSelect::~VMToolSelect()
{
}

VMToolSelect*
VMToolSelect::instance( KarbonPart* part )
{
	if ( s_instance == 0L )
	{
		s_instance = new VMToolSelect( part );
	}

	s_instance->m_part = part;
	return s_instance;
}

void
VMToolSelect::drawTemporaryObject( KarbonView* view )
{
	VPainter *painter = view->painterFactory()->editpainter();
	painter->setRasterOp( Qt::NotROP );

	KoPoint fp = view->canvasWidget()->viewportToContents( QPoint( m_fp.x(), m_fp.y() ) );

	// already selected, so must be a handle operation (move, scale etc.)
	KoRect rect = part()->document().selection().boundingBox();

	kdDebug() << " x: " << rect.x() << " y: " << rect.y() << " rect.width: " << rect.width() << " rect.height: " << rect.height() << endl;
	if( !part()->document().selection().isEmpty()
		&& ( m_state != normal || rect.contains( fp /* view->zoom() */ ) ) )
	{
		if( m_state != moving )
			m_state = moving;

		// move operation
		QWMatrix mat;
		mat.translate(	( m_lp.x() - fp.x() ) / view->zoom(),
						( m_lp.y() - fp.y() ) / view->zoom() );

		// TODO :  makes a copy of the selection, do assignment operator instead
		VObjectListIterator itr = part()->document().selection();
		VObjectList list;
		list.setAutoDelete( true );
		for( ; itr.current() ; ++itr )
		{
			list.append( itr.current()->clone() );
		}
		VObjectListIterator itr2 = list;
		painter->setZoomFactor( view->zoom() );
		for( ; itr2.current() ; ++itr2 )
		{
			itr2.current()->transform( mat );
			itr2.current()->setState( state_edit );

			itr2.current()->draw(
				painter,
				itr2.current()->boundingBox() );
		}
		painter->setZoomFactor( 1.0 );
	}
	else
	{
		painter->setPen( Qt::DotLine );
		painter->setZoomFactor( 1 );
		painter->moveTo( KoPoint( m_fp.x(), m_fp.y() ) );
		painter->lineTo( KoPoint( m_lp.x(), m_fp.y() ) );
		painter->lineTo( KoPoint( m_lp.x(), m_lp.y() ) );
		painter->lineTo( KoPoint( m_fp.x(), m_lp.y() ) );
		painter->lineTo( KoPoint( m_fp.x(), m_fp.y() ) );
		painter->setZoomFactor( view->zoom() );
		painter->strokePath();

		m_state = normal;
	}
}

bool
VMToolSelect::eventFilter( KarbonView* view, QEvent* event )
{
	if ( event->type() == QEvent::MouseMove && m_isDragging )
	{
		// erase old object:
		drawTemporaryObject( view );

		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		// paint new object:
		drawTemporaryObject( view );

		return true;
	}

	if ( event->type() == QEvent::MouseButtonRelease && m_isDragging )
	{
		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		// adjust to real viewport contents instead of raw mouse coords:
		KoPoint fp = view->canvasWidget()->viewportToContents( QPoint( m_fp.x(), m_fp.y() ) );
		KoPoint lp = view->canvasWidget()->viewportToContents( QPoint( m_lp.x(), m_lp.y() ) );

		if( m_state == moving )
		{
			m_state = normal;
			part()->addCommand(
				new VTranslateCmd(
					part(),
					part()->document().selection(),
					qRound( ( lp.x() - fp.x() ) * ( 1.0 / view->zoom() ) ),
					qRound( ( lp.y() - fp.y() ) * ( 1.0 / view->zoom() ) ) ),
				true );

//			part()->repaintAllViews();
		}
		else
		{

			if ( (fabs(lp.x()-fp.x()) + fabs(lp.y()-fp.y())) < 3.0 ) {
				// AK - should take the middle point here
				fp = lp - KoPoint(8.0, 8.0);
				lp = lp + KoPoint(8.0, 8.0);
			}

			// erase old object:
			drawTemporaryObject( view );

			part()->document().deselectAllObjects();

			part()->document().selectObjectsWithinRect(
				KoRect(
					fp.x() * view->zoom(), fp.y() * view->zoom(),
					( lp.x() - fp.x() ) * view->zoom(),
					( lp.y() - fp.y() ) * view->zoom() ).normalize(),
				true );

			//if( part()->selection().count() > 0  )
				part()->repaintAllViews();
		}

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
