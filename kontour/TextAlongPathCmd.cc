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

#include <TextAlongPathCmd.h>
#include <klocale.h>
#include <GDocument.h>
#include <GText.h>

TextAlongPathCmd::TextAlongPathCmd (GDocument* doc, GText* obj, GObject* path)
  : Command(i18n("Text Along Path"))
{
  document = doc;
  object = obj;
  object->ref ();
  oldPath = obj->pathObject ();
  if (oldPath)
    oldPath->ref ();
  newPath = path;
  if (newPath)
    newPath->ref ();
}

TextAlongPathCmd::~TextAlongPathCmd () {
  object->unref ();
  if (oldPath)
    oldPath->unref ();
  if (newPath)
    newPath->unref ();
}

void TextAlongPathCmd::execute () {
  if (oldPath)
    oldPath->unref ();
  oldPath = object->pathObject ();
  if (oldPath)
    oldPath->ref ();
  object->setPathObject (newPath);
}

void TextAlongPathCmd::unexecute () {
  object->setPathObject (oldPath);
}

