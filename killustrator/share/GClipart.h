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

#ifndef GClipart_h_
#define GClipart_h_

#include <kurl.h>
#include <GObject.h>

class QWinMetaFile;
class QPicture;
class QPainter;

class GClipart : public GObject {
  Q_OBJECT
public:
  GClipart (QWinMetaFile& wmf, const char* name);
  GClipart (const QDomElement &element);

  GClipart ();
  GClipart (const GClipart& obj);
  ~GClipart () {}

  virtual void draw (QPainter& p, bool withBasePoints = false,
                     bool outline = false);

  virtual QString typeName () const;

  virtual GObject* copy ();
  virtual GObject* clone (const QDomElement &element);

  virtual QDomElement writeToXml(QDomDocument &document);

protected:
  void calcBoundingBox ();

private:
  QPicture *pic;
  KURL url;
  float width, height;
};

#endif
