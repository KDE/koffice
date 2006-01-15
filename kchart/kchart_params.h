/* This file is part of the KDE project
   Copyright (C) 2001,2002,2003,2004 Laurent Montel <montel@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#ifndef KCHART_PARAMS_H
#define KCHART_PARAMS_H


class KoOasisStyles;
class KDChartParams;
class DCOPObject;


#include "kdchart/KDChartParams.h"

namespace KChart
{

class KChartParams : public KDChartParams
{
  public:
    typedef enum {
	// From KDChart
	NoType     = KDChartParams::NoType,
	Bar        = KDChartParams::Bar,
	Line       = KDChartParams::Line,
	Area       = KDChartParams::Area,
	Pie        = KDChartParams::Pie,
	HiLo       = KDChartParams::HiLo,
	Ring       = KDChartParams::Ring,
	Polar      = KDChartParams::Polar,
	BoxWhisker = KDChartParams::BoxWhisker,
    
	// Only in KChart
	BarLines,
    } ChartType;

    KChartParams();
    ~KChartParams();

    DCOPObject  *dcopObject();

    bool loadOasis( const QDomElement& chartElem,
                    KoOasisStyles& oasisStyles,
                    QString& errorMessage );

  private:
    DCOPObject  *m_dcop;
};

}  //KChart namespace

#endif
