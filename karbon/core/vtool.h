/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VTOOL_H__
#define __VTOOL_H__

// The abstract base class for all VObject-manipulating classes.

class QEvent;
class QWidget;

class KarbonView;
class VCommand;
class VObject;

class VTool
{
public:
	virtual ~VTool() = 0;

	// handling mouse/keyboard events:
	virtual bool eventFilter( KarbonView* /*view*/, QEvent* /*event*/ )
		{ return false; }

	// only manipulating (opposed to creating) tools have to implement this:
	virtual VCommand* manipulate( VObject* /*object*/ ) const { return 0L; }
};

#endif
