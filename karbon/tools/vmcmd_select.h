/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VMCMDSELECT_H__
#define __VMCMDSELECT_H__

#include "vcommand.h"

// Select object(s)
// TODO : is this command really necessary ?

class VPath;

class VMCmdSelect : public VCommand
{
public:
	VMCmdSelect( KarbonPart* part, const double tlX, const double tlY,
		 const double brX, const double brY );
	virtual ~VMCmdSelect() {}

	virtual void execute();
	//virtual void unexecute();

	// for complex shapes. needed to draw while creation (creation tool):
	//VPath* createPath();

private:
	double m_tlX;
	double m_tlY;
	double m_brX;
	double m_brY;
};

#endif
