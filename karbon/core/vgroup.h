/* This file is part of the KDE project
   Copyright (C) 2002, 2003 The Karbon Developers

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

#ifndef __VGROUP_H__
#define __VGROUP_H__

#include <qptrlist.h>

#include "vobject.h"

typedef QPtrList<VObject> VObjectList;
typedef QPtrListIterator<VObject> VObjectListIterator;

/**
 * Base class for all sort of VObject conglomerats.
 */

class VGroup : public VObject
{
public:
	VGroup( VObject* parent, VState state = normal );
	VGroup( const VGroup& group );

	virtual ~VGroup();

	virtual void draw( VPainter* painter, const KoRect* rect = 0L ) const;

	virtual const KoRect& boundingBox() const;

	virtual void setStroke( const VStroke& stroke );
	virtual void setFill( const VFill& fill );

	virtual void setState( const VState state );

	virtual void save( QDomElement& element ) const;
	virtual void load( const QDomElement& element );

	virtual VGroup* clone() const;

	virtual void accept( VVisitor& visitor );


	/**
	 * Removes the reference to the object, not the object itself.
	 */
	void take( const VObject& object );

	/**
	 * Appends a new object.
	 */
	void append( VObject* object );

	/**
	 * This function is important for undo/redo. It inserts newObject in front
	 * of oldObject.
	 */
	virtual void insertInfrontOf( VObject* newObject, VObject* oldObject );

	/**
	 * Clears the group, without destroying the grouped objects.
	 */
	void clear();

	/**
	 * Read only access to the grouped objects.
	 */
	const VObjectList& objects() const { return m_objects; }

protected:
	VObjectList m_objects;
};

#endif

