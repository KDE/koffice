/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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

#ifndef __VELLIPSE_H__
#define __VELLIPSE_H__

#include "vcomposite.h"

class VEllipse : public VPath
{
public:
	enum VEllipseType
	{
		full,
		section,
		cut,
		arc
	};
	VEllipse( VObject* parent, VState state = edit );
	VEllipse( VObject* parent,
		const KoPoint& topLeft, double width, double height,
		VEllipseType type = full, double startAngle = 0, double endAngle = 0 );

	virtual QString name() const;

	virtual void save( QDomElement& element ) const;
	virtual void saveOasis( KoStore *store, KoXmlWriter *docWriter );
	virtual void load( const QDomElement& element );

protected:
	void init();

private:
	VEllipseType m_type;
	KoPoint m_center;
	double m_rx;
	double m_ry;
	double m_startAngle;
	double m_endAngle;
};

#endif

