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

#include <iostream.h>
#include "CreatePolygonCmd.h"
#include <klocale.h>

CreatePolygonCmd::CreatePolygonCmd (GDocument* doc, GPolygon* obj) 
  : Command(i18n("Create Polygon"))
{
  document = doc;
  object = obj;
  object->ref ();
}

CreatePolygonCmd::CreatePolygonCmd (GDocument* doc, const Coord& p0, 
				    const Coord& p1, int num, int sval, 
				    bool concaveFlag) 
  : Command(i18n("Create Polygon"))
{
  document = doc;
  object = 0L;
  spos = p0;
  epos = p1;
  nCorners = num;
  sharpness = sval;
  isConcave = concaveFlag;
}

CreatePolygonCmd::~CreatePolygonCmd () {
  if (object)
    object->unref ();
}

void CreatePolygonCmd::execute () {
  if (object == 0L) {
    // create polygon
    object = new GPolygon (GPolygon::PK_Polygon);
    object->setSymmetricPolygon (spos, epos, nCorners, isConcave, sharpness);
    //    object->ref ();
  }
  document->insertObject (object);
}

void CreatePolygonCmd::unexecute () {
  document->deleteObject (object);
}
