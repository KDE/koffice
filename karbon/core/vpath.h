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

#ifndef __VPATH_H__
#define __VPATH_H__


#include <koPoint.h>

#include "vobject.h"

class QDomElement;
class QWMatrix;
class VPathIteratorList;
class VSegment;
class VVisitor;


/**
 * VPath provides a sophisticated list of VSegment. Noted: it also may contain
 * segments which are marked "deleted". If you are not interested in those undo/redo
 * housholding data, please always use a VPathIterator to access segments.
 */

class VPath : public VObject
{
	friend class VPathIterator;

public:
	VPath( VObject* parent );
	VPath( const VPath& list );
	VPath( const VSegment& segment );
	virtual ~VPath();

	const KoPoint& currentPoint() const;

	bool moveTo( const KoPoint& p );
	bool lineTo( const KoPoint& p );
	bool curveTo(
		const KoPoint& p1, const KoPoint& p2, const KoPoint& p3 );
	bool curve1To(
		const KoPoint& p2, const KoPoint& p3 );
	bool curve2To(
		const KoPoint& p1, const KoPoint& p3 );
	bool arcTo(
		const KoPoint& p1, const KoPoint& p2, const double r );

	bool isClosed() const
	{
		return m_isClosed;
	}

	void close();


	/**
	 * Returns true if point p is located inside this path.
	 */
	bool isInside( const KoPoint& p ) const;


	/**
	 * Returns false if segmentlist is oriented clockwise.
	 */
	bool counterClockwise() const;

	/**
	 * Reverts the winding orientation.
	 */
	void revert();


	/**
	 * Returns true if the current path is "emtpy". That means that it has
	 * zero or just one ( == "begin") segment.
	 */
	bool isEmpty() const
	{
		return count() <= 1;
	}


	virtual const KoRect& boundingBox() const;


	virtual void save( QDomElement& ) const
		{}	// VPaths cant be saved.

	virtual void load( const QDomElement& element );

	void saveSvgPath( QString & ) const;


	virtual VPath* clone() const;

	virtual void accept( VVisitor& visitor );


	// General list stuff.
	VPath& operator=( const VPath& list );

	bool insert( const VSegment* segment );
	bool insert( uint i, const VSegment* segment );
	void prepend( const VSegment* segment );
	void append( const VSegment* segment );
	void clear();

	uint count() const
	{
		return m_number;
	}

	VSegment* current() const
	{
		return m_current;
	}

	VSegment* getFirst() const
	{
		return m_first;
	}

	VSegment* getLast() const
	{
		return m_last;
	}

	VSegment* first();
	VSegment* last();
	VSegment* prev();
	VSegment* next();

private:
	VSegment* locate( uint index );

	VSegment* m_first;
	VSegment* m_last;
	VSegment* m_current;

	int m_currentIndex;
	uint m_number : 31;

	bool m_isClosed : 1;

	VPathIteratorList* m_iteratorList;
};


/**
 * VPathIterator provides an iterator class for highlevel path access.
 * Use VPathIterator whenever you want to access segments but are not interested
 * in undo/redo operations with (deleted) segments.
 */

class VPathIterator
{
	friend class VPathIteratorList;

public:
	VPathIterator( const VPath& list );
	VPathIterator( const VPathIterator& itr );
	~VPathIterator();

	VPathIterator& operator=( const VPathIterator& itr );

	VSegment* current() const;
	VSegment* operator()();
	VSegment* operator++();
	VSegment* operator+=( uint i );
	VSegment* operator--();
	VSegment* operator-=( uint i );

private:
	VPath* m_list;
	VSegment* m_current;
};

#endif

