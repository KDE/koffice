/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
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

#ifndef __VCOLORDOCKER_H__
#define __VCOLORDOCKER_H__

#include <vdocker.h>

class QTabWidget;
class QWidget;
class KoMainWindow;
class KarbonView;
class KarbonPart;
class VColor;
class VColorSlider;

class VColorDocker : public VDocker
{
	Q_OBJECT

public:
	 VColorDocker( KarbonPart* part, KarbonView* parent = 0L, const char* name = 0L );
	 virtual bool isStrokeDocker() { return m_isStrokeDocker; };
	 VColor color() { return *m_Color; }
  
public slots:
	virtual void setFillDocker();
	virtual void setStrokeDocker();
	virtual void setColor( VColor *);

private:
	virtual void mouseReleaseEvent( QMouseEvent *e );
	QWidget *mainWidget;
	QTabWidget *mTabWidget;
	QWidget *mRGBWidget;
	QWidget *mCMYKWidget;
	VColorSlider *mRedSlider;
	VColorSlider *mGreenSlider;
	VColorSlider *mBlueSlider;
	VColorSlider *mCyanSlider;
	VColorSlider *mMagentaSlider;
	VColorSlider *mYellowSlider;
	VColorSlider *mBlackSlider;
	VColorSlider *mOpacity;
	KarbonPart *m_part;
	KarbonView *m_view;
	
private slots:
	void updateCanvas();
	void updateCMYK();
	void updateRGB();
	void updateOpacity();
	void updateSliders();

protected:
	bool m_isStrokeDocker; //Are we setting stroke color ( true ) or fill color ( false )
	VColor *m_Color;
};

#endif

