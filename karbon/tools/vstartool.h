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

#ifndef __VSTARTOOL_H__
#define __VSTARTOOL_H__

#include <qgroupbox.h>

#include <klocale.h>

#include "vshapetool.h"


class KoUnitDoubleSpinBox;
class KIntSpinBox;
class KComboBox;
class KarbonPart;

class VStarOptionsWidget : public QGroupBox
{
Q_OBJECT
public:
	VStarOptionsWidget( KarbonPart*part, QWidget* parent = 0L, const char* name = 0L );

	void refreshUnit();

	int edges() const;
	double innerRadius() const;
	double outerRadius() const;
	uint type() const;
	uint innerAngle() const;
	void setEdges( int );
	void setInnerRadius( double );
	void setOuterRadius( double );

public slots:
	void typeChanged( int );

private:
	KoUnitDoubleSpinBox	*m_innerR;
	KoUnitDoubleSpinBox	*m_outerR;
	KIntSpinBox			*m_edges;
	KIntSpinBox			*m_innerAngle;
	KComboBox			*m_type;
	KarbonPart			*m_part;
	QLabel				*m_innerRLabel;
	QLabel				*m_outerRLabel;
};

class VStarTool : public VShapeTool
{
public:
	VStarTool( KarbonView* view );
	virtual ~VStarTool();

	virtual QWidget* optionsWidget() { return m_optionsWidget; }
	virtual QString name() { return i18n( "Star Tool" ); }
	virtual QString icon() { return "14_star"; }

	virtual VComposite* shape( bool interactive = false ) const;

	void refreshUnit();

	virtual void arrowKeyReleased( Qt::Key );

private:
	VStarOptionsWidget* m_optionsWidget;
};

#endif

