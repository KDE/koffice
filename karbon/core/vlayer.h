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

#ifndef __VLAYER_H__
#define __VLAYER_H__

#include <qptrlist.h>
#include <qstring.h>

#include "vgroup.h"

class QDomElement;
class DCOPObject;


/**
 * VLayer manages a set of vobjects. It keeps the objects from bottom to top
 * in a list, ie. objects higher in the list are drawn above lower objects.
 * Objects in a layer can be manipulated and worked on independant of objects
 * in other layers.
 */

class VLayer : public VGroup
{
public:
	VLayer( VObject* parent, VState state = normal );
	VLayer( const VLayer& layer );

	virtual ~VLayer();
	virtual DCOPObject* dcopObject();

	virtual void draw( VPainter *painter, const KoRect* rect = 0L ) const;

	virtual void save( QDomElement& element ) const;
	virtual void load( const QDomElement& element );

	virtual VLayer* clone() const;

	virtual void accept( VVisitor& visitor );


	void bringToFront( const VObject& object );

	/// moves the object one step up the list.
	/// When the object is at the top this method has no effect.
	void upwards( const VObject& object );

	/// moves the object one step down the list.
	/// When the object is at the bottom this method has no effect.
	void downwards( const VObject& object );

	void sendToBack( const VObject& object );

	void setName( const QString& name ) { m_name = name; }
	const QString& name() { return m_name; }
	
	void setSelected( bool state ) { m_selected = state; }
	bool selected() { return m_selected; }

private:
	bool    m_selected; /// True if the layer is checked in the layer docker
	QString m_name;     /// id for the layer
	DCOPObject *dcop;
};

#endif

