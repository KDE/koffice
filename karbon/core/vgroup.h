/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VGROUP_H__
#define __VGROUP_H__

#include "vobject.h"
#include "vobjectlist.h"

class QDomElement;

// grouping of VObjects

class VGroup : public VObject
{
public:
	VGroup();
	VGroup( const VObjectList & );
	VGroup( const VGroup & );
	~VGroup();

	void draw( VPainter *painter, const KoRect& rect,
		const double zoomFactor );

	void setState( const VState state );
	// clear the group without deleting the objects
	void empty();
	void insertObject( const VObject* object );


	virtual void transform( const QWMatrix& m );

    virtual KoRect boundingBox( const double zoomFactor ) const;
    virtual bool intersects( const KoRect& rect, const double zoomFactor ) const;

    virtual VObject* clone();

	// read-only access to objects:
	const VObjectList& objects() const { return m_objects; }

	virtual void setFill( const VFill &fill );
    virtual void setStroke( const VStroke &stroke );

	void save( QDomElement& element ) const;
	void load( const QDomElement& element );

private:
	VObjectList m_objects;
};

#endif
