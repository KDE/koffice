/* This file is part of the KDE project
   Copyright (C) 2002 - 2005, The Karbon Developers

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

#include <qcolor.h>

#include <kdebug.h>
#include <KoPoint.h>

#include "karbon_part.h"
#include "vcolordlg.h"
#include "vfill.h"
#include "vfillcmd.h"
#include "vkopainter.h"
#include "vselection.h"
#include "vstroke.h"
#include "vstrokecmd.h"
#include "vstrokefillpreview.h"

#define PANEL_SIZEX		50.0
#define PANEL_SIZEY		50.0

#define FILL_TOPX		15.0
#define FILL_TOPY		15.0
#define FILL_BOTTOMX		45.0
#define FILL_BOTTOMY		45.0

#define STROKE_TOPX		5.0
#define STROKE_TOPY		5.0
#define STROKE_BOTTOMX		35.0
#define STROKE_BOTTOMY		35.0

#define STROKE_TOPX_INNER	STROKE_TOPX + 4
#define STROKE_TOPY_INNER	STROKE_TOPY + 4
#define STROKE_BOTTOMX_INNER	STROKE_BOTTOMX - 4
#define STROKE_BOTTOMY_INNER	STROKE_BOTTOMY - 4


VStrokeFillPreview::VStrokeFillPreview(
	KarbonPart *part, QWidget* parent, const char* name )
		: QFrame( parent, name ), m_part( part )
{
	m_strokeWidget = false;
	setFocusPolicy( QWidget::NoFocus );

#if QT_VERSION < 0x030100
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
#else
	setFrameStyle( QFrame::GroupBoxPanel | QFrame::Sunken );
#endif

	installEventFilter( this );
	m_pixmap.resize( int( PANEL_SIZEX ), int( PANEL_SIZEY ) );
	m_painter = new VKoPainter( &m_pixmap, uint( PANEL_SIZEX ), uint( PANEL_SIZEY ) );
}

VStrokeFillPreview::~VStrokeFillPreview()
{
	delete( m_painter );
}

void
VStrokeFillPreview::paintEvent( QPaintEvent* event )
{
        bitBlt( this,
                (int)( width() - PANEL_SIZEX ) / 2, (int)( height() - PANEL_SIZEY ) / 2,
		&m_pixmap,
                0, 0, (int)PANEL_SIZEX, (int)PANEL_SIZEY );

	QFrame::paintEvent( event );
}

bool
VStrokeFillPreview::eventFilter( QObject *, QEvent *event )
{
	QMouseEvent* e = static_cast<QMouseEvent *>( event );

	int ex = e->x() - int( ( width() - PANEL_SIZEX ) / 2 );
	int ey = e->y() - int( ( height() - PANEL_SIZEY ) / 2 );

	if( event && event->type() == QEvent::MouseButtonPress )
	{
		if ( m_strokeWidget )
		{
			if(
				ex >= STROKE_TOPX && ex <= STROKE_BOTTOMX &&
				ey >= STROKE_TOPY && ey <= STROKE_BOTTOMY )
			{
				m_strokeWidget = true;
				emit strokeSelected();
			}
			else if(
				ex >= FILL_TOPX && ex <= FILL_BOTTOMX &&
				ey >= FILL_TOPY && ey <= FILL_BOTTOMY )
			{
				m_strokeWidget = false;
				emit fillSelected();
			}
		}
		else
		{
			if(
				ex >= FILL_TOPX && ex <= FILL_BOTTOMX &&
				ey >= FILL_TOPY && ey <= FILL_BOTTOMY )
			{
				m_strokeWidget = false;
				emit fillSelected();
			}
			else if(
				ex >= STROKE_TOPX && ex <= STROKE_BOTTOMX &&
				ey >= STROKE_TOPY && ey <= STROKE_BOTTOMY )
			{
				m_strokeWidget = true;
				emit strokeSelected();
			}
		}
		update( m_stroke, m_fill );
	}

	if( event && event->type() == QEvent::MouseButtonDblClick )
	{
		if(
			ex >= FILL_TOPX && ex <= FILL_BOTTOMX &&
			ey >= FILL_TOPY && ey <= FILL_BOTTOMY )
		{
			VColorDlg* dialog = new VColorDlg( m_fill.color(), this );
			if( dialog->exec() == QDialog::Accepted )
			{
				if( m_part && m_part->document().selection() ) m_part->addCommand( new VFillCmd( &m_part->document(), VFill( dialog->Color() ) ), true );
			}
			delete dialog;
		}
		else if(
			ex >= STROKE_TOPX && ex <= STROKE_BOTTOMX
			&& ey >= STROKE_TOPY && ey <= STROKE_BOTTOMY )
		{
			VColorDlg* dialog = new VColorDlg( m_stroke.color(), this );
			if( dialog->exec() == QDialog::Accepted )
			{
				if( m_part && m_part->document().selection() ) m_part->addCommand( new VStrokeCmd( &m_part->document(), dialog->Color() ), true );
			}
			delete dialog;
		}
	}
	return false;
}

void
VStrokeFillPreview::update( const VStroke &s, const VFill &f )
{
	m_painter->begin();

	if( &f )
		m_fill = f;
	else
		m_fill = VFill();
	if( &s )
		m_stroke = s;
	else
		m_stroke = VStroke();

	// draw checkerboard
	VFill fill;
	m_painter->setPen( Qt::NoPen );

	for( unsigned char y = 0; y < PANEL_SIZEY; y += 10 )
		for( unsigned char x = 0; x < PANEL_SIZEX; x += 10 )
		{
			fill.setColor( ( ( ( x + y ) % 20 ) == 0 ) ? QColor( 180, 180, 180 ) : QColor( 100, 100, 100 ) );
			m_painter->setBrush( fill );
			m_painter->drawRect( x, y, 10, 10 );
		}

	if ( m_strokeWidget )
	{
		drawFill( m_fill );
		drawStroke( m_stroke );
	}
	else
	{
 		drawStroke( m_stroke );
		drawFill( m_fill );
	}

	m_painter->end();

	repaint();
}

void
VStrokeFillPreview::drawFill( const VFill &f )
{
	VStroke stroke;

	if( f.type() != VFill::none )
	{
		if( f.type() != VFill::solid )
		{
			VFill fill;
			fill = f;

			if( f.type() == VFill::grad )
			{
				if( f.gradient().type() == VGradient::linear )
				{
					fill.gradient().setOrigin( KoPoint( 30, 20 ) );
					fill.gradient().setVector( KoPoint( 30, 50 ) );
				}
				else if( f.gradient().type() == VGradient::radial ||
						 f.gradient().type() == VGradient::conic )
				{
					fill.gradient().setOrigin( KoPoint( 30, 35 ) );
					fill.gradient().setFocalPoint( KoPoint( 30, 35 ) );
					fill.gradient().setVector( KoPoint( 30, 50 ) );
				}
			}
			if( f.type() == VFill::patt )
			{
				fill.pattern() = f.pattern();
				fill.pattern().setOrigin( KoPoint( 20, 10 ) );
				fill.pattern().setVector( KoPoint( 30, 10 ) );
				fill.setType( VFill::patt );
			}

			m_painter->setBrush( fill );
		}
		else
			m_painter->setBrush( f );
		m_painter->setPen( Qt::NoPen );
		m_painter->drawRect( KoRect( FILL_TOPX, FILL_TOPY, FILL_BOTTOMX - FILL_TOPX, FILL_BOTTOMY - FILL_TOPY ) );
	}
	else
	{
		VFill fill;
		fill.setColor( Qt::white );
		m_painter->setBrush( fill );
		m_painter->setPen( Qt::NoPen );

		m_painter->drawRect( KoRect(	FILL_TOPX, FILL_TOPY,
										FILL_BOTTOMX - FILL_TOPX,
										FILL_BOTTOMY - FILL_TOPY ) );
	}

	// show 3D outline of fill part
	VColor color;

	m_painter->setBrush( Qt::NoBrush );
	color.set( 1.0, 1.0, 1.0 );
	stroke.setColor( color );
	m_painter->setPen( stroke );

	m_painter->newPath();
	m_painter->moveTo( KoPoint( FILL_BOTTOMX, FILL_TOPY ) );
	m_painter->lineTo( KoPoint( FILL_TOPX, FILL_TOPY ) );
	m_painter->lineTo( KoPoint( FILL_TOPX, FILL_BOTTOMY ) );
	m_painter->strokePath();

	color.set( 0.5, 0.5, 0.5 );
	stroke.setColor( color );
	m_painter->setPen( stroke );

	m_painter->newPath();
	m_painter->moveTo( KoPoint( FILL_BOTTOMX, FILL_TOPY ) );
	m_painter->lineTo( KoPoint( FILL_BOTTOMX, FILL_BOTTOMY ) );
	m_painter->lineTo( KoPoint( FILL_TOPX, FILL_BOTTOMY ) );
	m_painter->strokePath();

	if( f.type() == VFill::none )
	{
		stroke.setColor( Qt::red );
		m_painter->setPen( stroke );
		m_painter->newPath();
		m_painter->moveTo( KoPoint( FILL_BOTTOMX, FILL_TOPY ) );
		m_painter->lineTo( KoPoint( FILL_TOPX, FILL_BOTTOMY ) );
		m_painter->strokePath();
	}
}

void
VStrokeFillPreview::drawStroke( const VStroke &s )
{
	VStroke stroke;
	stroke.setLineWidth( 2.0 );

	m_painter->setPen( Qt::NoPen );

	if( s.type() != VStroke::none )
	{
		VFill fill;

		if( s.type() != VStroke::solid )
		{
			if( s.type() == VStroke::grad )
			{
				fill.gradient() = s.gradient();

				if( s.gradient().type() == VGradient::linear )
				{
					fill.gradient().setOrigin( KoPoint( FILL_TOPX, 10 ) );
					fill.gradient().setVector( KoPoint( FILL_TOPX, 40 ) );
				}
				else if( s.gradient().type() == VGradient::radial ||
						 s.gradient().type() == VGradient::conic )
				{
					fill.gradient().setOrigin( KoPoint( FILL_TOPX, 25 ) );
					fill.gradient().setFocalPoint( KoPoint( FILL_TOPX, 25 ) );
					fill.gradient().setVector( KoPoint( FILL_TOPX, 40 ) );
				}

				fill.setType( VFill::grad );
			}
			if( s.type() == VStroke::patt )
			{
				fill.pattern() = s.pattern();
				fill.pattern().setOrigin( KoPoint( FILL_TOPX, 10 ) );
				fill.pattern().setVector( KoPoint( FILL_TOPX, 40 ) );
				fill.setType( VFill::patt );
			}
		}
		else
			fill.setColor( s.color() );

		m_painter->setFillRule( evenOdd );

		m_painter->setBrush( fill );

		m_painter->newPath();
		m_painter->moveTo( KoPoint( STROKE_TOPX, STROKE_TOPY ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX, STROKE_TOPY ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX, STROKE_BOTTOMY ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX, STROKE_BOTTOMY ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX, STROKE_TOPY ) );

		m_painter->moveTo( KoPoint( STROKE_TOPX_INNER, STROKE_TOPY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX_INNER, STROKE_TOPY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX_INNER, STROKE_BOTTOMY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX_INNER, STROKE_BOTTOMY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX_INNER, STROKE_TOPY_INNER ) );
		m_painter->fillPath();
	}
	else
	{
		VFill fill;
		m_painter->setFillRule( evenOdd );
		fill.setColor( Qt::white );

		m_painter->setBrush( fill );
		m_painter->setPen( Qt::NoPen );

		m_painter->newPath();
		m_painter->moveTo( KoPoint( STROKE_TOPX, STROKE_TOPY ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX, STROKE_TOPY ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX, STROKE_BOTTOMY ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX, STROKE_BOTTOMY ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX, STROKE_TOPY ) );

		m_painter->moveTo( KoPoint( STROKE_TOPX_INNER, STROKE_TOPY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX_INNER, STROKE_TOPY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_BOTTOMX_INNER, STROKE_BOTTOMY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX_INNER, STROKE_BOTTOMY_INNER ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX_INNER, STROKE_TOPY_INNER ) );
		m_painter->fillPath();
	}

	// show 3D outline of stroke part
	VColor color;

	color.set( 1.0, 1.0, 1.0 );
	stroke.setColor( color );
	m_painter->setBrush( Qt::NoBrush );
	m_painter->setPen( stroke );

	m_painter->newPath();
	m_painter->moveTo( KoPoint( STROKE_BOTTOMX + 1, STROKE_TOPY - 1 ) );
	m_painter->lineTo( KoPoint( STROKE_TOPX - 1, STROKE_TOPY - 1 ) );
	m_painter->lineTo( KoPoint( STROKE_TOPX - 1, STROKE_BOTTOMY + 1 ) );
	m_painter->strokePath();

	color.set( 0.5, 0.5, 0.5 );
	stroke.setColor( color );
	m_painter->setPen( stroke );

	m_painter->newPath();
	m_painter->moveTo( KoPoint( STROKE_BOTTOMX + 1, STROKE_TOPY - 1 ) );
	m_painter->lineTo( KoPoint( STROKE_BOTTOMX + 1, STROKE_BOTTOMY + 1 ) );
	m_painter->lineTo( KoPoint( STROKE_TOPX - 1, STROKE_BOTTOMY + 1 ) );
	m_painter->strokePath();

	//stroke.setColor( Qt::black.rgb() );
	//m_painter->setPen( stroke );
	m_painter->newPath();
	m_painter->moveTo( KoPoint( STROKE_BOTTOMX_INNER - 1, STROKE_TOPY_INNER + 1 ) );
	m_painter->lineTo( KoPoint( STROKE_TOPX_INNER + 1, STROKE_TOPY_INNER + 1 ) );
	m_painter->lineTo( KoPoint( STROKE_TOPX_INNER + 1, STROKE_BOTTOMY_INNER - 1 ) );
	m_painter->strokePath();

	color.set( 1.0, 1.0, 1.0 );
	stroke.setColor( color );
	m_painter->setPen( stroke );

	m_painter->newPath();
	m_painter->moveTo( KoPoint( STROKE_BOTTOMX_INNER - 1, STROKE_TOPY_INNER + 1 ) );
	m_painter->lineTo( KoPoint( STROKE_BOTTOMX_INNER - 1, STROKE_BOTTOMY_INNER - 1 ) );
	m_painter->lineTo( KoPoint( STROKE_TOPX_INNER + 1, STROKE_BOTTOMY_INNER - 1 ) );
	m_painter->strokePath();

	if( s.type() == VStroke::none )
	{
		stroke.setColor( Qt::red );
		m_painter->setPen( stroke );

		m_painter->newPath();
		m_painter->moveTo( KoPoint( STROKE_BOTTOMX, STROKE_TOPY ) );
		m_painter->lineTo( KoPoint( STROKE_TOPX, STROKE_BOTTOMY ) );
		m_painter->strokePath();
	}
}

void
VStrokeFillPreview::setFillSelected()
{
	m_strokeWidget = false;
	update( m_stroke, m_fill );
	emit fillSelected();
}

void
VStrokeFillPreview::setStrokeSelected()
{
	m_strokeWidget = true;
	update( m_stroke, m_fill );
	emit strokeSelected();
}

#include "vstrokefillpreview.moc"

