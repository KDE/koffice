/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VSELECTTOOL_H__
#define __VSELECTTOOL_H__

#include "vtool.h"
#include "vselection.h"

class VSelectTool : public VTool
{
public:
	VSelectTool( KarbonView* view );
	virtual ~VSelectTool();

	virtual void activate();

	// draw the object while it is edited:
	void drawTemporaryObject();

protected:
	virtual void setCursor( const QPoint & ) const;
	virtual void mousePressed( QMouseEvent * );
	virtual void mouseReleased( QMouseEvent * );

private:
	enum { normal, moving, scaling, rotating } m_state;

	double m_s1;
	double m_s2;
	KoPoint m_sp;

	VHandleNode m_activeNode;
};

#endif

