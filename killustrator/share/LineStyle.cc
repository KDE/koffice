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

#include <qcolor.h>
#include <qpen.h>
#include "LineStyle.h"
#include "version.h"

QIntDict<LineStyle> LineStyle::styles;

LineStyle::LineStyle (long i) { 
  id = i;
  preview = 0L;
}

LineStyle::~LineStyle () {
  if (preview)
    delete preview;
}

long LineStyle::styleID () const {
  return id;
}

QPixmap& LineStyle::pixmap () {
  if (preview == 0L) {
    preview = new QPixmap (100, 20);
    preview->fill ();
    Painter p;
    QPen pen (QT_PRFX::black, 2, (QT_PRFX::PenStyle) id);
    p.begin (preview);
    p.setPen (pen);
    p.drawLine (0, 10, 100, 10);
    p.end ();
  }
  return *preview;
}

void LineStyle::install (LineStyle* style) {
  styles.insert (style->styleID (), style);
}

LineStyle* LineStyle::getLineStyle (long id) {
  if (styles.isEmpty ())
    LineStyle::initialize ();
  return styles.find (id);
}

QIntDictIterator<LineStyle> LineStyle::getLineStyles () {
  if (styles.isEmpty ())
    LineStyle::initialize ();
  
  return QIntDictIterator<LineStyle> (styles);
}

void LineStyle::initialize () {
  LineStyle::install (new LineStyle ((long) QT_PRFX::NoPen));
  LineStyle::install (new LineStyle ((long) QT_PRFX::SolidLine));
  LineStyle::install (new LineStyle ((long) QT_PRFX::DashLine));
  LineStyle::install (new LineStyle ((long) QT_PRFX::DotLine));
  LineStyle::install (new LineStyle ((long) QT_PRFX::DashDotLine));
  LineStyle::install (new LineStyle ((long) QT_PRFX::DashDotDotLine));
}
