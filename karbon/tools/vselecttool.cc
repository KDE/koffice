/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers

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

#include <math.h>
#include <stdlib.h>

#include <qcursor.h>
#include <qlabel.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <koPoint.h>
#include <koRect.h>
#include <kdebug.h>

#include <karbon_part.h>
#include <karbon_view.h>
#include <render/vpainter.h>
#include <render/vpainterfactory.h>
#include <core/vselection.h>
#include <core/vcanvas.h>
#include "vselecttool.h"
#include <commands/vtransformcmd.h>

VSelectOptionsWidget::VSelectOptionsWidget( KarbonView* view )
	: QButtonGroup( 1, Qt::Horizontal, i18n( "Selection Mode" ) ), m_view( view )
{
	new QRadioButton( i18n( "Select in current layer" ), this );
	new QRadioButton( i18n( "Select in visible layers" ), this );
	new QRadioButton( i18n( "Select in selected layers" ), this );
	
	setRadioButtonExclusive( true );
	setButton( m_view->part()->document().selectionMode() );
	
	connect( this, SIGNAL( clicked( int ) ), this, SLOT( modeChange( int ) ) );
} // VSelectOptionsWidget::VSelectOptionsWidget

void VSelectOptionsWidget::modeChange( int mode )
{
	m_view->part()->document().setSelectionMode( (VDocument::VSelectionMode)mode );
} // VSelectOptionsWidget::modeChanged

VSelectTool::VSelectTool( KarbonView* view, const char* name )
	: VTool( view, name ), m_state( normal )
{
	m_lock = false;
	m_objects.setAutoDelete( true );
	m_optionsWidget = new VSelectOptionsWidget( view );
	registerTool( this );
}

VSelectTool::~VSelectTool()
{
	delete m_optionsWidget;
}

void
VSelectTool::activate()
{
	view()->statusMessage()->setText( i18n( "Select" ) );
	view()->canvasWidget()->viewport()->setCursor( QCursor( Qt::arrowCursor ) );
	view()->part()->document().selection()->showHandle();
	view()->part()->document().selection()->setSelectObjects();
	view()->part()->document().selection()->setState( VObject::selected );
	view()->part()->document().selection()->selectNodes();
}

QString VSelectTool::contextHelp()
{
	QString s = i18n( "<qt><b>Selection tool:</b><br>" );
	s += i18n( "<i>Select in current layer:</i><br>The selection is made in the layer selected in the layers docker.<br><br>" );
	s += i18n( "<i>Select in visible layers:</i><br>The selection is made in the visible layers (eye in the layers docker).<br><br>" );
	s += i18n( "<i>Select in selected layers:</i><br>The selection is made in the checked layers in the layers docker.<br><br>" );
	s += i18n( "<i>Position using arrow keys</i><br>The selection can be positioned up, down, left and right using the corresponding arrow keys." );
	return s;
} // VSelectTool::contextHelp

void
VSelectTool::draw()
{
	VPainter *painter = view()->painterFactory()->editpainter();
	//painter->setZoomFactor( view()->zoom() );
	painter->setRasterOp( Qt::NotROP );

	KoRect rect = view()->part()->document().selection()->boundingBox();

	if( m_state != normal || rect.contains( first() ) || m_activeNode != node_none )
	{
		if( m_state == normal )
		{
			m_state = ( m_activeNode == node_none ) ? moving : scaling;
			recalc();
		}

		VObjectListIterator itr = m_objects;
		for( ; itr.current(); ++itr )
		{
			itr.current()->draw( painter, &itr.current()->boundingBox() );
		}
	}
	else if( m_state == normal )
	{
		painter->setPen( Qt::DotLine );
		painter->newPath();
		painter->moveTo( KoPoint( first().x(), first().y() ) );
		painter->lineTo( KoPoint( m_current.x(), first().y() ) );
		painter->lineTo( KoPoint( m_current.x(), m_current.y() ) );
		painter->lineTo( KoPoint( first().x(), m_current.y() ) );
		painter->lineTo( KoPoint( first().x(), first().y() ) );
		painter->strokePath();

		m_state = normal;
	}
}

void
VSelectTool::setCursor() const
{
	if( m_state != normal ) return;
	switch( view()->part()->document().selection()->handleNode( last() ) )
	{
		case node_lt:
		case node_rb:
			view()->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeFDiagCursor ) );
			break;
		case node_rt:
		case node_lb:
			view()->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeBDiagCursor ) );
			break;
		case node_lm:
		case node_rm:
			view()->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeHorCursor ) );
			break;
		case node_mt:
		case node_mb:
			view()->canvasWidget()->viewport()->setCursor( QCursor( Qt::SizeVerCursor ) );
			break;
		default:
			view()->canvasWidget()->viewport()->setCursor( QCursor( Qt::arrowCursor ) );
	}
}

void
VSelectTool::mouseButtonPress()
{
	m_current = first();

	m_activeNode = view()->part()->document().selection()->handleNode( first() );

	recalc();

	view()->part()->document().selection()->setState( VObject::edit );
	view()->canvasWidget()->repaintAll( view()->part()->document().selection()->boundingBox() );
	view()->part()->document().selection()->setState( VObject::selected );

	draw();
}

void
VSelectTool::mouseDrag()
{
	draw();

	recalc();

	draw();
}

void
VSelectTool::mouseButtonRelease()
{
	if( m_state == normal )
	{
		KoPoint fp = first();
		KoPoint lp = last();

		if( (fabs( lp.x() - fp.x() ) + fabs( lp.y() - fp.y() ) ) < 3.0 )
		{
			// AK - should take the middle point here
			fp = lp - KoPoint( 8.0, 8.0 );
			lp = lp + KoPoint( 8.0, 8.0 );
		}

		view()->part()->document().selection()->clear();
		view()->part()->document().selection()->append(
			KoRect( fp.x(), fp.y(), lp.x() - fp.x(), lp.y() - fp.y() ).normalize() );

		view()->selectionChanged();
		view()->part()->repaintAllViews( KoRect( fp.x(), fp.y(), lp.x() - fp.x(), lp.y() - fp.y() ).normalize() );
	}
	else
		m_state = normal;

	updateStatusBar();
}

void
VSelectTool::mouseDragRelease()
{
	if( m_state == normal )
	{
		// Y mirroring
		KoPoint fp = first();
		KoPoint lp = last();
		view()->part()->document().selection()->clear();
		view()->part()->document().selection()->append(
			KoRect( fp.x(), fp.y(), lp.x() - fp.x(), lp.y() - fp.y() ).normalize() );

		view()->selectionChanged();
		view()->part()->repaintAllViews( KoRect( fp.x(), fp.y(), lp.x() - fp.x(), lp.y() - fp.y() ).normalize() );
	}
	else if( m_state == moving )
	{
		m_state = normal;
		recalc();
		if( m_lock )
			view()->part()->addCommand(
				 new VTranslateCmd(
					&view()->part()->document(),
					abs( int( m_distx ) ) >= abs( int( m_disty ) ) ? qRound( m_distx ) : 0,
					abs( int( m_distx ) ) <= abs( int( m_disty ) ) ? qRound( m_disty ) : 0 ),
				true );
		else
			view()->part()->addCommand(
				new VTranslateCmd( &view()->part()->document(), qRound( m_distx ), qRound( m_disty ) ),
				true );
	}
	else if( m_state == scaling )
	{
		m_state = normal;
		view()->part()->addCommand(
			new VScaleCmd( &view()->part()->document(), m_sp, m_s1, m_s2 ),
			true );
		m_s1 = m_s2 = 1;
	}
	m_lock = false;
	updateStatusBar();
}

void
VSelectTool::arrowKeyReleased( Qt::Key key )
{
	int dx = 0;
	int dy = 0;
	switch( key )
	{
		case Qt::Key_Up: dy = 10; break;
		case Qt::Key_Down: dy = -10; break;
		case Qt::Key_Right: dx = 10; break;
		case Qt::Key_Left: dx = -10; break;
		default: return;
	}
	m_state = normal;
	view()->part()->addCommand(
		new VTranslateCmd(
			&view()->part()->document(),
			dx, dy ),
		true );
}

void
VSelectTool::updateStatusBar() const
{
	if( view()->part()->document().selection()->objects().count() > 0 )
	{
		KoRect rect = view()->part()->document().selection()->boundingBox();

		QString selectMessage = QString( "Selection [(%1, %2), (%3, %4)] (%5)" ).arg( KoUnit::ptToUnit( rect.x(), view()->part()->unit() ) ).arg( KoUnit::ptToUnit( rect.y(), view()->part()->unit() ) ).arg( KoUnit::ptToUnit( rect.right(), view()->part()->unit() ) ).arg( KoUnit::ptToUnit( rect.bottom(), view()->part()->unit() ) ).arg( view()->part()->unitName() );
		view()->statusMessage()->setText( selectMessage );
	}
	else
		view()->statusMessage()->setText( i18n( "No selection" ) );
}

void
VSelectTool::mouseDragCtrlPressed()
{
	m_lock = true;
}

void
VSelectTool::mouseDragCtrlReleased()
{
	m_lock = false;
}

void
VSelectTool::recalc()
{
	if( m_state == normal )
	{
		m_current = last();
	}
	else
	{
		VTransformCmd* cmd;

		if( m_state == moving )
		{
			m_distx = last().x() - first().x();
			m_disty = last().y() - first().y();
			if( m_lock )
				cmd = new VTranslateCmd( 0L, abs( int( m_distx ) ) >= abs( int( m_disty ) ) ? m_distx : 0,
											 abs( int( m_distx ) ) <= abs( int( m_disty ) ) ? m_disty : 0 );
			else
				cmd = new VTranslateCmd( 0L, m_distx, m_disty );
		}
		else
		{
			KoRect rect = view()->part()->document().selection()->boundingBox();

			if( m_activeNode == node_lb )
			{
				m_sp = KoPoint( rect.right(), rect.bottom() );
				m_s1 = ( rect.right() - last().x() ) / double( rect.width() );
				m_s2 = ( rect.bottom() - last().y() ) / double( rect.height() );
			}
			else if( m_activeNode == node_mb )
			{
				m_sp = KoPoint( ( ( rect.right() + rect.left() ) / 2 ), rect.bottom() );
				m_s1 = 1;
				m_s2 = ( rect.bottom() - last().y() ) / double( rect.height() );
			}
			else if( m_activeNode == node_rb )
			{
				m_sp = KoPoint( rect.x(), rect.bottom() );
				m_s1 = ( last().x() - rect.x() ) / double( rect.width() );
				m_s2 = ( rect.bottom() - last().y() ) / double( rect.height() );
			}
			else if( m_activeNode == node_rm)
			{
				m_sp = KoPoint( rect.x(), ( rect.bottom() + rect.top() )  / 2 );
				m_s1 = ( last().x() - rect.x() ) / double( rect.width() );
				m_s2 = 1;
			}
			else if( m_activeNode == node_rt )
			{
				m_sp = KoPoint( rect.x(), rect.y() );
				m_s1 = ( last().x() - rect.x() ) / double( rect.width() );
				m_s2 = ( last().y() - rect.y() ) / double( rect.height() );
			}
			else if( m_activeNode == node_mt )
			{
				m_sp = KoPoint( ( ( rect.right() + rect.left() ) / 2 ), rect.y() );
				m_s1 = 1;
				m_s2 = ( last().y() - rect.y() ) / double( rect.height() );
			}
			else if( m_activeNode == node_lt )
			{
				m_sp = KoPoint( rect.right(), rect.y() );
				m_s1 = ( rect.right() - last().x() ) / double( rect.width() );
				m_s2 = ( last().y() - rect.y() ) / double( rect.height() );
			}
			else if( m_activeNode == node_lm )
			{
				m_sp = KoPoint( rect.right(), ( rect.bottom() + rect.top() )  / 2 );
				m_s1 = ( rect.right() - last().x() ) / double( rect.width() );
				m_s2 = 1;
			}

			cmd = new VScaleCmd( 0L, m_sp, m_s1, m_s2 );
		}

		// Copy selected objects and transform:
		m_objects.clear();
		VObject* copy;

		VObjectListIterator itr = view()->part()->document().selection()->objects();
		for( ; itr.current() ; ++itr )
		{
			if( itr.current()->state() != VObject::deleted )
			{
				copy = itr.current()->clone();
				copy->setState( VObject::edit );

				cmd->visit( *copy );

				m_objects.append( copy );
			}
		}

		delete( cmd );
	}
}

#include "vselecttool.moc"
