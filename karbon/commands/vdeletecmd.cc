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

#include <klocale.h>

#include "vdeletecmd.h"
#include "vselection.h"
#include "vdocument.h"

VDeleteCmd::VDeleteCmd( VDocument* doc )
	: VCommand( doc, i18n( "Delete Objects" ), "editdelete" )
{
	m_selection = document()->selection()->clone();
	document()->selection()->clear();

	if( m_selection->objects().count() == 1 )
		setName( i18n( "Delete Object" ) );
}

VDeleteCmd::VDeleteCmd( VDocument* doc, VObject* object )
	: VCommand( doc, i18n( "Delete Object" ), "editdelete" )
{
	m_selection = new VSelection();
	m_selection->append( object );
}

VDeleteCmd::~VDeleteCmd()
{
	delete( m_selection );
}

void
VDeleteCmd::execute()
{
	VObjectListIterator itr( m_selection->objects() );
	for ( ; itr.current() ; ++itr )
	{
		itr.current()->setState( VObject::deleted );
	}

	setSuccess( true );
}

void
VDeleteCmd::unexecute()
{
	VObjectListIterator itr( m_selection->objects() );
	for ( ; itr.current() ; ++itr )
	{
		itr.current()->setState( VObject::normal );
	}

	setSuccess( false );
}

