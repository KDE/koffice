/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VCCMDRECTANGLE_H__
#define __VCCMDRECTANGLE_H__

#include "vccommand.h"

// create a rectangle-shape.

class VPath;

class VCCmdRectangle : public VCCommand
{
public:
	VCCmdRectangle( KarbonPart* part, const double tlX, const double tlY,
		 const double brX, const double brY );
	virtual ~VCCmdRectangle() {}

	// for complex shapes. needed to draw while creation (creation tool):
	VPath* createPath();

private:
	double m_tlX;
	double m_tlY;
	double m_brX;
	double m_brY;
};

#endif
