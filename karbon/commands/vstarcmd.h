/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VSTARCMD_H__
#define __VSTARCMD_H__

#include "vshapecmd.h"

// create a star-shape.

class VObject;

class VStarCmd : public VShapeCmd
{
public:
	VStarCmd( KarbonPart* part, double centerX, double centerY,
		double outerR, double innerR, uint edges, double angle = 0.0 );
	virtual ~VStarCmd() {}

	virtual VObject* createPath();

private:
	double m_centerX;
	double m_centerY;
	double m_outerR;
	double m_innerR;
	uint m_edges;
	double m_angle;
};

#endif

