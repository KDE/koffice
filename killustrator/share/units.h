/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by  
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef units_h_
#define units_h_

enum MeasurementUnit { 
  UnitPoint, UnitMillimeter, UnitInch, UnitPica, UnitCentimeter, 
  UnitDidot, UnitCicero
};

float cvtPtToMm (float value);
float cvtPtToCm (float value);
float cvtPtToInch (float value);
float cvtPtToPica (float value);
float cvtPtToDidot (float value);
float cvtPtToCicero (float value);
float cvtMmToPt (float value);
float cvtCmToPt (float value);
float cvtInchToPt (float value);
float cvtPicaToPt (float value);
float cvtDidotToPt (float value);
float cvtCiceroToPt (float value);

float cvtPtToUnit (MeasurementUnit unit, float value);
float cvtUnitToPt (MeasurementUnit unit, float value);

const char* unitToString (MeasurementUnit unit);

#endif
