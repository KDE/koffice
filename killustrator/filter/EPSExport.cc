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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "version.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <qprinter.h>
#include <qglobal.h>
#include "version.h"
#include "Painter.h"
#include "EPSExport.h"

EPSExport::EPSExport () {
}

EPSExport::~EPSExport () {
}

bool EPSExport::setup (GDocument *, const char* ) {
  return true;
}

bool EPSExport::exportToFile (GDocument* doc) {
  // compute bounding box
  Rect box = doc->boundingBoxForAllObjects ();

  QPrinter printer;
  printer.setDocName (doc->fileName ());
  printer.setCreator ("KIllustrator");
  printer.setOutputFileName (outputFileName ());
  printer.setOutputToFile (true);
  switch (doc->pageLayout ().format) {
  case PG_DIN_A4:
    printer.setPageSize (QPrinter::A4);
    break;
  case PG_DIN_A5:
    printer.setPageSize (QPrinter::B5);
    break;
  case PG_US_LETTER:
    printer.setPageSize (QPrinter::Letter);
    break;
  case PG_US_LEGAL:
    printer.setPageSize (QPrinter::Legal);
    break;
  default:
    break;
  }
  printer.setOrientation (doc->pageLayout ().orientation == PG_PORTRAIT ?
			  QPrinter::Portrait : QPrinter::Landscape);

  Painter paint;
  paint.begin (&printer);
#if 0
  // define the bounding box as clipping region
  paint.setClipRect (0, 0, box.width () + 2, box.height () + 2);
  // and move the objects to the origin
  paint.translate (-box.left () + 1, -box.top () + 1);
  // force update of cliping regions (only for gradient pixmaps)
#else
  paint.setClipRect (box.left (), box.top (), 
		     box.width () + 2 + box.left (), 
		     box.height () + 2 + box.top ());
#endif
  doc->invalidateClipRegions ();
  doc->drawContents (paint);
  doc->invalidateClipRegions ();
  paint.end ();
  return true;
}
