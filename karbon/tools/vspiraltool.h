/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
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

#ifndef __VSPIRALTOOL_H__
#define __VSPIRALTOOL_H__

#include <qgroupbox.h>

#include <klocale.h>

#include "vshapetool.h"

class KComboBox;
class KoUnitDoubleSpinBox;
class KDoubleNumInput;
class KIntSpinBox;
class KarbonPart;

class VSpiralTool : public VShapeTool
{
public:
	VSpiralTool( KarbonView *view );
	virtual ~VSpiralTool();

	virtual QWidget *optionsWidget() { return m_optionsWidget; }
	virtual QString name() { return i18n( "Spiral tool" ); }
	virtual QString icon() { return "14_spiral"; }

	virtual VComposite* shape( bool interactive = false ) const;

	void refreshUnit();

	virtual void arrowKeyReleased( Qt::Key );

private:
	class VSpiralOptionsWidget : public QGroupBox
	{
	public:
		VSpiralOptionsWidget( KarbonPart *part, QWidget *parent = 0L, const char* name = 0L );

		double radius() const;
		uint segments() const;
		double fade() const;
		bool clockwise() const;
		void setRadius( double value );
		void setSegments( uint value );
		void setFade( double value );
		void setClockwise( bool value );

		void refreshUnit();

	private:
		KoUnitDoubleSpinBox	*m_radius;
		KIntSpinBox			*m_segments;
		KDoubleNumInput		*m_fade;
		QComboBox			*m_clockwise;
		KarbonPart			*m_part;
	};

	VSpiralOptionsWidget *m_optionsWidget;
};

#endif

