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

/* This is the color slider widget that is used to select color or color components */

/* vselecttoolbar.h */
#ifndef VSELECTTOOLBAR_H
#define VSELECTTOOLBAR_H

#include <ktoolbar.h>

class TKUFloatSpinBox;

class VSelectToolBar : public KToolBar
{
	Q_OBJECT
public:
	VSelectToolBar( QWidget* parent = 0L, const char* name = 0L );
	~VSelectToolBar();
	
private:
	TKUFloatSpinBox *m_x;
	TKUFloatSpinBox *m_y;
	TKUFloatSpinBox *m_width;
	TKUFloatSpinBox *m_height;
};

#endif

