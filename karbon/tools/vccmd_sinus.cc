/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#include <klocale.h>
#include <qwmatrix.h>

#include "vccmd_sinus.h"
#include "vglobal.h"
#include "vpath.h"

VCCmdSinus::VCCmdSinus( KarbonPart* part,
		const double tlX, const double tlY,
		const double brX, const double brY, const uint periods )
	: VCommand( part, i18n("Insert Sinus") ), m_object( 0L ),
	  m_tlX( tlX ), m_tlY( tlY ), m_brX( brX ), m_brY( brY )
{
	// we want at least 1 period:
	m_periods = periods < 1 ? 1 : periods;
}

void
VCCmdSinus::execute()
{
	if ( m_object )
		m_object->setDeleted( false );
	else
	{
		m_object = createPath();
		// add path:
		m_part->insertObject( m_object );
	}
}

void
VCCmdSinus::unexecute()
{
	if ( m_object )
		m_object->setDeleted();
}

VPath*
VCCmdSinus::createPath()
{
	VPath* path = new VPath();

	path->moveTo( 0.0, 0.0 );

	for ( uint i = 0; i < m_periods; ++i )
	{
		path->curveTo(
			i + 1.0/24.0,	( 2.0 * VGlobal::sqrt2 - 1.0 ) * VGlobal::one_7,
			i + 1.0/12.0,	( 4.0 * VGlobal::sqrt2 - 2.0 ) * VGlobal::one_7,
			i + 1.0/8.0,	VGlobal::sqrt2 * 0.5 );
		path->curveTo(
			i + 1.0/6.0,	( 3.0 * VGlobal::sqrt2 + 2.0 ) * VGlobal::one_7,
			i + 5.0/24.0,	1.0,
			i + 1.0/4.0,	1.0  );
		path->curveTo(
			i + 7.0/24.0,	1.0,
			i + 1.0/3.0,	( 3.0 * VGlobal::sqrt2 + 2.0 ) * VGlobal::one_7,
			i + 3.0/8.0,	VGlobal::sqrt2 * 0.5 );
		path->curveTo(
			i + 5.0/12.0,	( 4.0 * VGlobal::sqrt2 - 2.0 ) * VGlobal::one_7,
			i + 11.0/24.0,	( 2.0 * VGlobal::sqrt2 - 1.0 ) * VGlobal::one_7,
			i + 1.0/2.0,	0.0 );
		path->curveTo(
			i + 13.0/24.0,	-( 2.0 * VGlobal::sqrt2 - 1.0 ) * VGlobal::one_7,
			i + 7.0/12.0,	-( 4.0 * VGlobal::sqrt2 - 2.0 ) * VGlobal::one_7,
			i + 5.0/8.0,	-VGlobal::sqrt2 * 0.5 );
		path->curveTo(
			i + 2.0/3.0,	-( 3.0 * VGlobal::sqrt2 + 2.0 ) * VGlobal::one_7,
			i + 17.0/24.0,	-1.0,
			i + 3.0/4.0,	-1.0 );
		path->curveTo(
			i + 19.0/24.0,	-1.0,
			i + 5.0/6.0,	-( 3.0 * VGlobal::sqrt2 + 2.0 ) * VGlobal::one_7,
			i + 7.0/8.0,	-VGlobal::sqrt2 * 0.5 );
		path->curveTo(
			i + 11.0/12.0,	-( 4.0 * VGlobal::sqrt2 - 2.0 ) * VGlobal::one_7,
			i + 23.0/24.0,	-( 2.0 * VGlobal::sqrt2 - 1.0 ) * VGlobal::one_7,
			i + 1.0,		0.0 );
	}

	double w = m_brX - m_tlX;
	double h = m_tlY - m_brY;

	// translate path and scale:
	QWMatrix m;
	m.scale( w/m_periods, h*0.5 );
	m.translate( m_tlX, m_brY + h*0.5 );
	path->transform( m );

	return path;
}
