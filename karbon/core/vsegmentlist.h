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

#ifndef __VSEGMENTLIST_H__
#define __VSEGMENTLIST_H__

#include <koPoint.h>

#include "vobject.h"


class QDomElement;
class QWMatrix;
class VSegment;
class VVisitor;


// The general list stuff is stolen from Qt.

class VSegmentListIteratorList;

class VSegmentList : public VObject
{
friend class VSegmentListIterator;

public:
	VSegmentList( VObject* parent );
	VSegmentList( const VSegmentList& list );
	VSegmentList( const VSegment& segment );
	virtual ~VSegmentList();

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

	bool isClosed() const { return m_isClosed; }
	void close();

	/**
	 * Returns false if segmentlist is oriented clockwise.
	 */
	bool counterClockwise() const;

	/**
	 * Reverts the winding orientation.
	 */
	void revert();

	/**
	 * Applies an affine transformation matrix to all segments.
	 */
	virtual void transform( const QWMatrix& m );

	virtual const KoRect& boundingBox() const;

	virtual void save( QDomElement& element ) const;
	virtual void load( const QDomElement& element );

	virtual VSegmentList* clone() const;

	virtual void accept( VVisitor& visitor );


	// general list stuff:
	VSegmentList& operator=( const VSegmentList& list );

	bool insert( const VSegment* segment );
	bool insert( uint i, const VSegment* segment );
	void prepend( const VSegment* segment );
	void append( const VSegment* segment );
	void clear();

	uint count() const { return m_number; }

	VSegment* current() const { return m_current; }
	VSegment* getFirst() const { return m_first; }
	VSegment* getLast() const { return m_last; }
	VSegment* first();
	VSegment* last();
	VSegment* prev();
	VSegment* next();

private:
	VSegment* locate( uint index );

	bool m_isClosed;

	VSegment* m_first;
	VSegment* m_last;
	VSegment* m_current;

	int m_currentIndex;
	uint m_number;

	VSegmentListIteratorList* m_iteratorList;
};


class VSegmentListIterator
{
friend class VSegmentListIteratorList;

public:
	VSegmentListIterator( const VSegmentList& list );
	VSegmentListIterator( const VSegmentListIterator& itr );
	~VSegmentListIterator();

	VSegmentListIterator& operator=( const VSegmentListIterator& itr );

	VSegment* current() const { return m_current; }
	VSegment* operator()();
	VSegment* operator++();
	VSegment* operator+=( uint i );
	VSegment* operator--();
	VSegment* operator-=( uint i );

private:
	VSegmentList* m_list;
	VSegment* m_current;
};

#endif

