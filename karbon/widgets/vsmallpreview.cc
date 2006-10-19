/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.t-com.hr)
   Copyright (C) 2005, The Karbon Developers

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

/* vsmallpreview.cc */
#include <qcolor.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <klocale.h>
#include <KoPoint.h>

#include "vcolor.h"
#include "vfill.h"
#include "vgradient.h"
#include "vkopainter.h"
#include "vpattern.h"
#include "vstroke.h"

#include "vsmallpreview.h"

#define FRAMEWIDTH		40

VSmallPreview::VSmallPreview( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	/* Create widget layout */
	QHBoxLayout *layout = new QHBoxLayout( this, 4 );
	m_strokeLabel = new QLabel( i18n( "Stroke: None" ), this );
	layout->addWidget( m_strokeLabel );
	m_strokeFrame = new QFrame( this );
	m_strokeFrame->setFixedWidth ( FRAMEWIDTH );
	m_strokeFrame->setFrameStyle( QFrame::GroupBoxPanel | QFrame::Plain );
	layout->addWidget( m_strokeFrame );
	m_fillLabel = new QLabel( i18n( "Fill: None" ), this );
	layout->addWidget( m_fillLabel );
	m_fillFrame = new QFrame( this );
	m_fillFrame->setFixedWidth ( FRAMEWIDTH );
	m_fillFrame->setFrameStyle( QFrame::GroupBoxPanel | QFrame::Plain );
	layout->addWidget( m_fillFrame );
	layout->activate();

	m_fill = VFill();
	m_stroke = VStroke();
}

VSmallPreview::~VSmallPreview()
{
}

void
VSmallPreview::update( const VStroke &s, const VFill &f )
{
	if( &f )
		m_fill = f;
	else
		m_fill = VFill();
	if( &s )
		m_stroke = s;
	else
		m_stroke = VStroke();

	drawStroke( m_stroke );
	drawFill( m_fill );
}

void
VSmallPreview::paintEvent( QPaintEvent* /*event*/ )
{
	drawStroke( m_stroke );
	drawFill( m_fill );
}

void
VSmallPreview::drawFill( const VFill &f )
{
	VFill fill;
	VStroke stroke;
	
	QPixmap m_pixmap;
	m_pixmap.resize( m_fillFrame->width(), m_fillFrame->height() );
	VKoPainter* m_painter = new VKoPainter( &m_pixmap, m_fillFrame->width(), m_fillFrame->height() );

	m_painter->begin();
	m_painter->setPen( Qt::NoPen );
	fill.setColor( Qt::white );
	m_painter->setBrush( fill );
	m_painter->drawRect( KoRect( 0, 0, m_fillFrame->width(), m_fillFrame->height() ) );

	switch ( f.type() )
	{
		case VFill::solid:
		{
			switch ( f.color().colorSpace() )
			{
				case VColor::rgb:
					m_fillLabel->setText( i18n( "Fill: RGB") ); break;
				case VColor::cmyk:
					m_fillLabel->setText( i18n( "Fill: CMYK") ); break;
				case VColor::hsb:
					m_fillLabel->setText( i18n( "Fill: HSB") ); break;
				case VColor::gray:
					m_fillLabel->setText( i18n( "Fill: Grayscale") ); break;
				default:
					m_fillLabel->setText( i18n( "Fill: Color") );
			}
			fill.setColor( f.color() );
			break;
		}
		case VFill::grad:
		{
			fill.gradient() = f.gradient();
			fill.setType( VFill::grad );
			m_fillLabel->setText( i18n( "Fill: Gradient") );
			if( f.gradient().type() == VGradient::linear )
			{
				fill.gradient().setOrigin( KoPoint( m_fillFrame->width() * 0.5, 0 ) );
				fill.gradient().setVector( KoPoint( m_fillFrame->width() * 0.5, m_fillFrame->height() ) );
			}
			else if( f.gradient().type() == VGradient::radial ||
				f.gradient().type() == VGradient::conic )
			{
				fill.gradient().setOrigin( KoPoint( m_fillFrame->width() * 0.5, m_fillFrame->height() * 0.5 ) );
				fill.gradient().setFocalPoint( KoPoint( m_fillFrame->width() * 0.5, m_fillFrame->height() * 0.5 ) );
				fill.gradient().setVector( KoPoint( m_fillFrame->width() * 0.5, m_fillFrame->height() ) );
			}
			break;

		}
		case VFill::patt:
		{
			fill.pattern() = f.pattern();
			fill.pattern().setOrigin( KoPoint( 0, 0 ) );
			fill.pattern().setVector( KoPoint( m_fillFrame->width() * 0.5, 0 ) );
			fill.setType( VFill::patt );
			m_fillLabel->setText( i18n( "Fill: Pattern") );
			break;
		}
		default: //None or unknown
		{
			m_fillLabel->setText( i18n( "Fill: None") );
			fill.setColor( Qt::white );
			m_painter->setBrush( fill );
			m_painter->drawRect( KoRect( 0, 0, m_fillFrame->width(), m_fillFrame->height() ) );
			stroke.setColor( Qt::red );
			stroke.setLineWidth( 2.0 );
			m_painter->setPen( stroke );
			m_painter->newPath();
			m_painter->moveTo( KoPoint( 4, m_fillFrame->height() - 4 ) );
			m_painter->lineTo( KoPoint( m_fillFrame->width() - 4, 4 ) );
			m_painter->strokePath();
		}
	}

	if( f.type() != VFill::none )
	{
		m_painter->setPen( stroke );
		m_painter->setBrush( fill );
		m_painter->drawRect( KoRect( 0, 0, m_fillFrame->width(), m_fillFrame->height() ) );	
	}

	m_painter->end();

	bitBlt( m_fillFrame, m_fillFrame->frameWidth(), m_fillFrame->frameWidth(), &m_pixmap, m_fillFrame->frameWidth(), m_fillFrame->frameWidth(), m_fillFrame->width() - m_fillFrame->frameWidth(), m_fillFrame->height() - m_fillFrame->frameWidth(), CopyROP );

	delete ( m_painter );
}

void
VSmallPreview::drawStroke( const VStroke &s )
{
	VFill fill;
	VStroke stroke;

	QPixmap m_pixmap;
	m_pixmap.resize( m_fillFrame->width(), m_fillFrame->height() );
	VKoPainter* m_painter = new VKoPainter( &m_pixmap, m_fillFrame->width(), m_fillFrame->height() );

	m_painter->begin();
	m_painter->setPen( Qt::NoPen );
	fill.setColor( Qt::white );
	m_painter->setBrush( fill );
	m_painter->drawRect( KoRect( 0, 0, m_strokeFrame->width(), m_strokeFrame->height() ) );
	
	switch ( s.type() )
	{
		case VStroke::solid:
		{
			switch ( s.color().colorSpace() )
			{
				case VColor::rgb:
					m_strokeLabel->setText( i18n( "Stroke: RGB") ); break;
				case VColor::cmyk:
					m_strokeLabel->setText( i18n( "Stroke: CMYK") ); break;
				case VColor::hsb:
					m_strokeLabel->setText( i18n( "Stroke: HSB") ); break;
				case VColor::gray:
					m_strokeLabel->setText( i18n( "Stroke: Grayscale") ); break;
				default:
					m_strokeLabel->setText( i18n( "Stroke: Color") );
			}
			fill.setColor( s.color() );
			break;
		}
		case VStroke::grad:
		{
			fill.gradient() = s.gradient();
			fill.setType( VFill::grad );
			m_strokeLabel->setText( i18n( "Stroke: Gradient") );
			if( s.gradient().type() == VGradient::linear )
			{
				fill.gradient().setOrigin( KoPoint( m_strokeFrame->width() * 0.5, 0 ) );
				fill.gradient().setVector( KoPoint( m_strokeFrame->width() * 0.5, m_strokeFrame->height() ) );
			}
			else if( s.gradient().type() == VGradient::radial ||
				s.gradient().type() == VGradient::conic )
			{
				fill.gradient().setOrigin( KoPoint( m_strokeFrame->width() * 0.5, m_strokeFrame->height() * 0.5 ) );
				fill.gradient().setFocalPoint( KoPoint( m_strokeFrame->width() * 0.5, m_strokeFrame->height() * 0.5 ) );
				fill.gradient().setVector( KoPoint( m_strokeFrame->width() * 0.5, m_strokeFrame->height() ) );
			}
			break;
		}
		case VStroke::patt:
		{
			fill.pattern() = s.pattern();
			fill.pattern().setOrigin( KoPoint( 0, 0 ) );
			fill.pattern().setVector( KoPoint( m_strokeFrame->width() * 0.5, 0 ) );
			fill.setType( VFill::patt );
			m_strokeLabel->setText( i18n( "Stroke: Pattern") );
			break;
		}
		default: //None or unknown
		{
			m_strokeLabel->setText( i18n( "Stroke: None") );
			fill.setColor( Qt::white );
			m_painter->setBrush( fill );
			m_painter->drawRect( KoRect( 0, 0, m_strokeFrame->width(), m_strokeFrame->height() ) );
			stroke.setColor( Qt::red );
			stroke.setLineWidth( 2.0 );
			m_painter->setPen( stroke );
			m_painter->newPath();
			m_painter->moveTo( KoPoint( 4, m_strokeFrame->height() - 4 ) );
			m_painter->lineTo( KoPoint( m_strokeFrame->width() - 4, 4 ) );
			m_painter->strokePath();
		}
	}

	if( s.type() != VStroke::none )
	{
		m_painter->setPen( stroke );
		m_painter->setBrush( fill );
		m_painter->drawRect( KoRect( 0, 0, m_strokeFrame->width(), m_strokeFrame->height() ) );
	}

	m_painter->end();

	bitBlt( m_strokeFrame, m_strokeFrame->frameWidth(), m_strokeFrame->frameWidth(), &m_pixmap, m_strokeFrame->frameWidth(), m_strokeFrame->frameWidth(), m_strokeFrame->width() - m_strokeFrame->frameWidth(), m_strokeFrame->height() - m_strokeFrame->frameWidth(), CopyROP );

	delete ( m_painter );
}

#include "vsmallpreview.moc"

