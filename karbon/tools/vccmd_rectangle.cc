/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#include <klocale.h>

#include "vccmd_rectangle.h"
#include "vpath.h"

VCCmdRectangle::VCCmdRectangle( KarbonPart* part,
		const double tlX, const double tlY,
		const double brX, const double brY )
	: VCCommand( part, i18n("Insert Rectangle") ), 
	  m_tlX( tlX ), m_tlY( tlY ), m_brX( brX ), m_brY( brY )
{
}

void
VCCmdRectangle::execute()
{
	if ( m_object )
		m_object->setState( VObject::normal );
	else
	{
		m_object = createPath();
		// add path:
		m_part->unselectObjects();
		m_part->insertObject( m_object );
		m_object->setState( VObject::selected );
	}
}

void
VCCmdRectangle::unexecute()
{
	if ( m_object )
		m_object->setState( VObject::deleted );
}

VPath*
VCCmdRectangle::createPath()
{
	VPath* path = new VPath();

	path->moveTo( m_tlX, m_tlY );
	path->lineTo( m_brX, m_tlY );
	path->lineTo( m_brX, m_brY );
	path->lineTo( m_tlX, m_brY );
	path->close();

	return path;
}
