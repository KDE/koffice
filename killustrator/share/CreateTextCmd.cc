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
#include "CreateTextCmd.h"

CreateTextCmd::CreateTextCmd (GDocument* doc, GText* obj) :
  Command(i18n("Create Text")) 
{
  document = doc;
  object = obj;
  object->ref ();
}

CreateTextCmd::CreateTextCmd (GDocument* doc, const Coord& p, 
			      const char* str) :
  Command(i18n("Create Text"))
{
  document = doc;
  origin = p;
  text = str;
}

CreateTextCmd::~CreateTextCmd () {
  if (object)
    object->unref ();
}

void CreateTextCmd::execute () {
  if (object == 0L) {
    // create new text object
    object = new GText ();
    object->setOrigin (origin);
    object->setText (text);
    object->ref ();
  }
  document->insertObject (object);
}

void CreateTextCmd::unexecute () {
  document->deleteObject (object);
}
