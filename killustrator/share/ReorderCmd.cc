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

#include "ReorderCmd.h"

ReorderCmd::ReorderCmd (GDocument* doc, ReorderPosition pos) {
  objects.resize (doc->selectionCount ());
  oldpos.resize (doc->selectionCount ());

  QListIterator<GObject> it (doc->getSelection ());
  for (unsigned int i = 0; it.current (); ++it, ++i) {
    it.current ()->ref ();
    objects.insert (i, it.current ());
  }
  document = doc;
  position = pos;
}

ReorderCmd::~ReorderCmd () {
  for (unsigned int i = 0; i < objects.count (); i++)
    objects[i]->unref ();
}

void ReorderCmd::execute () {
  for (unsigned int i = 0; i < objects.count (); i++) {
    unsigned int newidx =  0;

    // look for the object
    unsigned int idx = document->findIndexOfObject (objects[i]);
    oldpos[i] = idx;
    if (position == RP_ToFront || position == RP_ForwardOne) {
      if (idx == document->objectCount () - 1)
	// already at the first position
	continue;

      // move the object
      if (position == RP_ToFront)
	newidx = document->objectCount () - 1;
      else
	newidx = idx + 1;
    }
    else {
      if (idx == 0)
	// already at the last position
	continue;

      // move the object
      if (position == RP_ToBack)
	newidx = 0;
      else
	newidx = idx - 1;
    }
    document->moveObjectToIndex (objects[i], newidx);
  }
}

void ReorderCmd::unexecute () {
  for (unsigned int i = 0; i < objects.count (); i++) 
    document->moveObjectToIndex (objects[i], oldpos[i]);
}
