/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VCCMDSTAR_H__
#define __VCCMDSTAR_H__

#include "vccommand.h"

// create a star-shape.

class VPath;

class VCCmdStar : public VCCommand
{
public:
	VCCmdStar( KarbonPart* part, const double centerX, const double centerY,
		const double outerR, const double innerR, const uint edges );
	virtual ~VCCmdStar() {}

	// for complex shapes. needed to draw while creation (creation tool):
	VPath* createPath();

private:
	double m_centerX;
	double m_centerY;
	double m_outerR;
	double m_innerR;
	uint m_edges;
};

#endif
