/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#include <klocale.h>

#include "vccmd_rectangle.h"
#include "vpath.h"

#include <kdebug.h>
VCCmdRectangle::VCCmdRectangle( KarbonPart* part,
		const double tlX, const double tlY,
		const double brX, const double brY )
	: VCommand( part, i18n("Create rectangle-shape") ), m_object( 0L ),
	  m_tlX( tlX ), m_tlY( tlY ), m_brX( brX ), m_brY( brY )
{
}

void
VCCmdRectangle::execute()
{
	if ( m_object )
		m_object->setDeleted( false );
	else
	{
		m_object = new VPath();
		m_object->moveTo( m_tlX, m_tlY );
		m_object->lineTo( m_brX, m_tlY );
		m_object->lineTo( m_brX, m_brY );
		m_object->lineTo( m_tlX, m_brY );
		m_object->close();

		// add path:
		m_part->layers().getLast()->objects().append( m_object );
	}
}

void
VCCmdRectangle::unexecute()
{
	if ( m_object )
		m_object->setDeleted();
}
