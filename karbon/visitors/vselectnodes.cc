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


#include "vpath.h"
#include "vsegment.h"
#include "vselectnodes.h"
#include "vlayer.h"
#include "vdocument.h"

void
VSelectNodes::visitVPath( VPath& path )
{
	path.first();

	// skip "begin":
	while( path.current() )
	{
		if( m_rect.isEmpty() )
		{
/*
			path.current()->selectCtrlPoint1( m_select );
			path.current()->selectCtrlPoint2( m_select );
			path.current()->selectKnot( m_select );
*/

			setSuccess();
		}
		else
		{
			if( m_exclusive )
			{
				for( int i = 0; i < path.current()->degree(); i++ )
					path.current()->selectPoint( i, false );
			}

			if( path.current()->type() == VSegment::curve )
			{
				// select first control point, when previous knot is selected:
				if(
					path.current()->prev() &&
					path.current()->prev()->knotIsSelected() )
				{
					path.current()->selectPoint( 0, true );
				}
			}

//			if( path.current()->type() == VSegment::curve 
//				path.current()->ctrlPointFixing() != VSegment::second &&
//				m_rect.contains( path.current()->point( 1 ) ) )
//			{
				// select second control point, when knot is selected:
/*				if( path.current()->knotSelected() )
				{
					path.current()->selectPoint( 1, m_select );

					setSuccess();
				}
*/
//			}

			if( m_rect.contains( path.current()->knot() ) )
			{
				path.current()->selectKnot( m_select );
				if( path.current()->type() == VSegment::curve )
					path.current()->selectPoint( 1, m_select );

				setSuccess();
			}
		}
		path.next();
	}
}

void
VSelectNodes::visitVLayer( VLayer& layer )
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

void
VTestNodes::visitVPath( VPath& path )
{
	path.first();

	while( path.current() )
	{
		for( int i = 0; i < path.current()->degree(); i++ )
			if( m_rect.contains( path.current()->point( i ) ) &&
				path.current()->pointIsSelected( i ) )
					setSuccess();

		path.next();
	}
}

void
VTestNodes::visitVLayer( VLayer& layer )
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

