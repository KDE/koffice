/* This file is part of the KDE project
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

#include <core/vselection.h>
#include "whirlpinchplugin.h"
#include <karbon_view_base.h>
#include <karbon_part_base.h>
#include <kgenericfactory.h>
#include <core/vdocument.h>
#include <core/vcomposite.h>
#include <core/vpath.h>
#include <core/vsegment.h>
#include <core/vglobal.h>

#include <kdebug.h>

#include <qgroupbox.h>
#include <qlabel.h>

#include <knuminput.h>

typedef KGenericFactory<WhirlPinchPlugin, KarbonViewBase> WhirlPinchPluginFactory;
K_EXPORT_COMPONENT_FACTORY( karbon_whirlpinchplugin, WhirlPinchPluginFactory( "karbonwhirlpinchplugin" ) )

WhirlPinchPlugin::WhirlPinchPlugin( KarbonViewBase *parent, const char* name, const QStringList & ) : Plugin( parent, name )
{
	new KAction(
		i18n( "&Whirl/Pinch..." ), "14_whirl", 0, this,
		SLOT( slotWhirlPinch() ), actionCollection(), "path_whirlpinch" );

	m_whirlPinchDlg = new VWhirlPinchDlg();
	m_whirlPinchDlg->setAngle( 20.0 );
	m_whirlPinchDlg->setPinch( 0.0 );
	m_whirlPinchDlg->setRadius( 100.0 );
}

void
WhirlPinchPlugin::slotWhirlPinch()
{
	KarbonPartBase *part = ((KarbonViewBase *)parent())->part();
	if( part && m_whirlPinchDlg->exec() )
		part->addCommand( new VWhirlPinchCmd( &part->document(), m_whirlPinchDlg->angle(), m_whirlPinchDlg->pinch(), m_whirlPinchDlg->radius() ), true );
}

VWhirlPinchDlg::VWhirlPinchDlg( QWidget* parent, const char* name )
	: KDialogBase( parent, name, true, i18n( "Whirl Pinch" ), Ok | Cancel )
{
	// add input fields:
	QGroupBox* group = new QGroupBox( 2, Qt::Horizontal, i18n( "Properties" ), this );

	new QLabel( i18n( "Angle:" ), group );
	m_angle = new KDoubleNumInput( group );
	new QLabel( i18n( "Pinch:" ), group );
	m_pinch = new KDoubleNumInput( group );
	new QLabel( i18n( "Radius:" ), group );
	m_radius = new KDoubleNumInput( group );
	group->setMinimumWidth( 300 );

	// signals and slots:
	connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );
	connect( this, SIGNAL( cancelClicked() ), this, SLOT( reject() ) );

	setMainWidget( group );
	setFixedSize( baseSize() );
}

double
VWhirlPinchDlg::angle() const
{
	return m_angle->value();
}

double
VWhirlPinchDlg::pinch() const
{
	return m_pinch->value();
}

double
VWhirlPinchDlg::radius() const
{
	return m_radius->value();
}

void
VWhirlPinchDlg::setAngle( double value )
{
	m_angle->setValue( value);
}

void
VWhirlPinchDlg::setPinch( double value )
{
	m_pinch->setValue(value);
}

void
VWhirlPinchDlg::setRadius( double value )
{
	m_radius->setValue( value);
}

VWhirlPinchCmd::VWhirlPinchCmd( VDocument* doc,
	double angle, double pinch, double radius )
		: VCommand( doc, i18n( "Whirl Pinch" ) )
{
	m_selection = document()->selection()->clone();

	m_angle = angle;
	m_pinch = pinch;
	m_radius = radius;
	m_center = m_selection->boundingBox().center();
}

VWhirlPinchCmd::~VWhirlPinchCmd()
{
	delete( m_selection );
}

void
VWhirlPinchCmd::execute()
{
	VObjectListIterator itr( m_selection->objects() );
	for ( ; itr.current() ; ++itr )
		visit( *itr.current() );
}

void
VWhirlPinchCmd::unexecute()
{
}

void
VWhirlPinchCmd::visitVPath( VPath& composite )
{
	// first subdivide:
//	VInsertKnots insertKnots( 2 );
//	insertKnots.visit( composite );

	VVisitor::visitVPath( composite );
}

void
VWhirlPinchCmd::visitVSubpath( VSubpath& path )
{
	QWMatrix m;
	KoPoint delta;
	double dist;

	path.first();

	while( path.current() )
	{
// TODO: selfmade this function since it's gone:
//		path.current()->convertToCurve();


		// Apply three times separately to each segment node.

		delta = path.current()->knot() - m_center;
		dist = sqrt( delta.x() * delta.x() + delta.y() * delta.y() );

		if( dist < m_radius )
		{
			m.reset();

			dist /= m_radius;

			// pinch:
			m.translate( m_center.x(), m_center.y() );
			m.scale(
				pow( sin( VGlobal::pi_2 * dist ), -m_pinch ),
				pow( sin( VGlobal::pi_2 * dist ), -m_pinch ) );

			// whirl:
			m.rotate( m_angle * ( 1.0 - dist ) * ( 1.0 - dist ) );
			m.translate( -m_center.x(), -m_center.y() );

			path.current()->setKnot( path.current()->knot().transform( m ) );
		}

		if( path.current()->isCurve() )
		{
			delta = path.current()->point( 0 ) - m_center;
			dist = sqrt( delta.x() * delta.x() + delta.y() * delta.y() );

			if( dist < m_radius )
			{
				m.reset();

				dist /= m_radius;

				// Pinch.
				m.translate( m_center.x(), m_center.y() );
				m.scale(
					pow( sin( VGlobal::pi_2 * dist ), -m_pinch ),
					pow( sin( VGlobal::pi_2 * dist ), -m_pinch ) );

				// Whirl.
				m.rotate( m_angle * ( 1.0 - dist ) * ( 1.0 - dist ) );
				m.translate( -m_center.x(), -m_center.y() );

				path.current()->setPoint( 0, path.current()->point(0).transform( m ) );
			}


			delta = path.current()->point( 1 ) - m_center;
			dist = sqrt( delta.x() * delta.x() + delta.y() * delta.y() );

			if( dist < m_radius )
			{
				m.reset();

				dist /= m_radius;

				// Pinch.
				m.translate( m_center.x(), m_center.y() );
				m.scale(
					pow( sin( VGlobal::pi_2 * dist ), -m_pinch ),
					pow( sin( VGlobal::pi_2 * dist ), -m_pinch ) );

				// Whirl.
				m.rotate( m_angle * ( 1.0 - dist ) * ( 1.0 - dist ) );
				m.translate( -m_center.x(), -m_center.y() );

				path.current()->setPoint( 1, path.current()->point(1).transform( m ) );
			}
		}


		if( !success() )
			setSuccess();


		path.next();
	}

	// Invalidate bounding box once.
	path.invalidateBoundingBox();
}

#include "whirlpinchplugin.moc"

