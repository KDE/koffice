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


#include "vselectobjects.h"
#include "vlayer.h"
#include "vdocument.h"
#include "vsegment.h"
#include <kdebug.h>

void
VSelectObjects::visitVComposite( VComposite& composite )
{
	// Never select a deleted object.
	if( composite.state() == VObject::deleted )
		return;


	bool selected = false;


	// Check if composite is completely inside the selection rectangle.
	// This test should be the first test since it's the less expensive one.
	if( m_rect.contains( composite.boundingBox() ) )
	{
		selected = true;
	}

	// Check if any of the rectangle corners is inside the composite.
	// This test should be done before the intersect test since it covers many
	// intersection cases.
	if( !selected )
	{
		if(
			composite.pointIsInside( m_rect.topLeft() ) ||
			composite.pointIsInside( m_rect.topRight() ) ||
			composite.pointIsInside( m_rect.bottomRight() ) ||
			composite.pointIsInside( m_rect.bottomLeft() ) )
		{
			selected = true;
		}
	}

	// Check if selection rectangle intersects the composite.
	if( !selected )
	{
		// Path for holding a helper segment.
		VPath path( 0L );

		path.moveTo( m_rect.topLeft() );
		path.lineTo( m_rect.topRight() );

		if( composite.intersects( *path.getLast() ) )
		{
			selected = true;
		}
		else
		{
			path.getFirst()->setKnot( m_rect.bottomRight() );

			if( composite.intersects( *path.getLast() ) )
			{
				selected = true;
			}
			else
			{
				path.getLast()->setKnot( m_rect.bottomLeft() );

				if( composite.intersects( *path.getLast() ) )
				{
					selected = true;
				}
				else
				{
					path.getFirst()->setKnot( m_rect.topLeft() );

					if( composite.intersects( *path.getLast() ) )
					{
						selected = true;
					}
				}
			}
		}
	}


	if( selected )
	{
		m_selection.append( &composite );

		setSuccess();
	}
}

void
VSelectObjects::visitVObject( VObject& object )
{
	// Never select a deleted object
	if( object.state() == VObject::deleted )
		return;

	if( !m_rect.isEmpty() )
	{
		if( m_select )
		{
			if( m_rect.intersects( object.boundingBox() ) )
			{
				//object.setState( VObject::selected );
				m_selection.append( &object );
				setSuccess();
			}
		}
		else
		{
			if( m_rect.intersects( object.boundingBox() ) )
			{
				object.setState( VObject::normal );
				m_selection.clear();
				setSuccess();
			}
		}
	}
	else
	{
		if( m_select )
		{
			object.setState( VObject::selected );
			m_selection.append( &object );
			setSuccess();
		}
		else
		{
			object.setState( VObject::normal );
			setSuccess();
		}
	}
}

void
VSelectObjects::visitVLayer( VLayer& layer )
{
	VDocument* doc = (VDocument*)layer.parent();
	if ( ( layer.state() != VObject::deleted ) &&
	     ( ( doc->selectionMode() == VDocument::AllLayers ) ||
	       ( doc->selectionMode() == VDocument::VisibleLayers && ( layer.state() == VObject::normal || layer.state() == VObject::normal_locked ) ) ||
	       ( doc->selectionMode() == VDocument::SelectedLayers && layer.selected() ) ||
	       ( doc->selectionMode() == VDocument::ActiveLayer && doc->activeLayer() == &layer ) ) )
	{
		VObjectListIterator itr( layer.objects() );
		for( ; itr.current(); ++itr )
			itr.current()->accept( *this );
	}
}
