/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#include "vccmd_sinus.h"	// command
#include "vcdlg_sinus.h"	// dialog
#include "vctool_sinus.h"

VCToolSinus* VCToolSinus::s_instance = 0L;

VCToolSinus::VCToolSinus( KarbonPart* part )
	: m_part( part ), m_isDragging( false ), m_isSquare( false ),
	  m_isCentered( false )
{
	// create config dialog:
	m_dialog = new VCDlgSinus();
	m_dialog->setValueWidth( 100.0 );
	m_dialog->setValueHeight( 100.0 );
	m_dialog->setValuePeriods( 1 );
}

VCToolSinus::~VCToolSinus()
{
	delete( m_dialog );
}

VCToolSinus*
VCToolSinus::instance( KarbonPart* part )
{
	if ( s_instance == 0L )
	{
		s_instance = new VCToolSinus( part );
	}

	return s_instance;
}

bool
VCToolSinus::eventFilter( KarbonView* view, QEvent* event )
{

	if ( event->type() == QEvent::MouseMove && m_isDragging )
	{
		// erase old object:
		drawTemporaryObject( view );

		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		recalcCoords();

		// paint new object:
		drawTemporaryObject( view );

		return true;
	}

	if ( event->type() == QEvent::MouseButtonRelease && m_isDragging )
	{
		m_isDragging = false;
		m_isSquare = false;
		m_isCentered = false;

		// erase old object:
		drawTemporaryObject( view );

		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_lp.setX( mouse_event->pos().x() );
		m_lp.setY( mouse_event->pos().y() );

		// did we drag the mouse?
		if ( m_fp == m_lp )
		{
			// we didnt drag => show a config-dialog:
			if ( m_dialog->exec() )
			{
				m_part->addCommand(
					new VCCmdSinus( m_part,
						m_fp.x(), m_fp.y(),
						m_fp.x() + m_dialog->valueWidth(),
						m_fp.y() + m_dialog->valueHeight(),
						m_dialog->valuePeriods() ) );
			}
		}
		else
		{
			m_part->addCommand(
				new VCCmdSinus( m_part, m_tl.x(), m_tl.y(), m_br.x(), m_br.y(),
					m_dialog->valuePeriods() )
 );
		}

		return true;
	}

	// handle pressing of keys:
	if ( event->type() == QEvent::KeyPress )
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*> ( event );

		// cancel dragging with ESC-key:
		if ( key_event->key() == Qt::Key_Escape && m_isDragging )
		{
			m_isDragging = false;
			m_isSquare = false;
			m_isCentered = false;

			// erase old object:
			drawTemporaryObject( view );

			return true;
		}

		// if SHIFT is pressed, we want a square:
		if ( key_event->key() == Qt::Key_Shift )
		{
			m_isSquare = true;

			if ( m_isDragging )
			{
				// erase old object:
				drawTemporaryObject( view );
				recalcCoords();
				// draw new old object:
				drawTemporaryObject( view );
			}

			return true;
		}

		// if Ctrl is pressed, we want a centered path:
		if ( key_event->key() == Qt::Key_Control )
		{
			m_isCentered = true;

			if ( m_isDragging )
			{
				// erase old object:
				drawTemporaryObject( view );
				recalcCoords();
				// draw new old object:
				drawTemporaryObject( view );
			}

			return true;
		}
	}

	// handle releasing of keys:
	if ( event->type() == QEvent::KeyRelease )
	{
		QKeyEvent* key_event = static_cast<QKeyEvent*> ( event );

		if ( key_event->key() == Qt::Key_Shift )
		{
			m_isSquare = false;

			if ( m_isDragging )
			{
				// erase old object:
				drawTemporaryObject( view );
				recalcCoords();
				// draw new old object:
				drawTemporaryObject( view );
			}

			return true;
		}

		if ( key_event->key() == Qt::Key_Control )
		{
			m_isCentered = false;

			if ( m_isDragging )
			{
				// erase old object:
				drawTemporaryObject( view );
				recalcCoords();
				// draw new old object:
				drawTemporaryObject( view );
			}

			return true;
		}
	}

	// the whole story starts with this event:
	if ( event->type() == QEvent::MouseButtonPress )
	{
		QMouseEvent* mouse_event = static_cast<QMouseEvent*> ( event );
		m_fp.setX( mouse_event->pos().x() );
		m_fp.setY( mouse_event->pos().y() );

		// set initial object:
		m_tl.setX( m_fp.x() );
		m_tl.setY( m_fp.y() );
		m_br.setX( m_fp.x() + 1 );
		m_br.setY( m_fp.y() + 1 );

		// draw initial object:
		drawTemporaryObject( view );

		m_isDragging = true;

		return true;
	}

	return false;
}


