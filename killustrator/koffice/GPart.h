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

#ifndef GPart_h_
#define GPart_h_

#include <qobject.h>
#include <qcolor.h>
#include <qwmatrix.h>
#include <qpainter.h>
#include <qdstream.h>
#include <qpicture.h>

#include <koMainWindow.h>
#include <koView.h>
#include <openparts.h>

#include "Coord.h"
#include "GObject.h"

class KIllustratorFrame;
class KIllustratorChild;

class GPart : public GObject {
  Q_OBJECT
public:
  GPart ();
  GPart (KIllustratorChild *c);
  GPart (const list<XmlAttribute>& attribs);
  GPart (const GPart& p);

  ~GPart ();
  
  virtual void draw (Painter& p, bool withBasePoints = false,
		     bool outline = false);

  virtual const char* typeName ();

  virtual GObject* copy ();
  virtual GObject* clone (const list<XmlAttribute>& attribs);

  virtual void writeToXml (XmlWriter&);

  void setView (KIllustratorFrame *v) { view = v; }
  KIllustratorFrame *getView () { return view; }

  KIllustratorChild *getChild () { return child; }
  void setMainWindow(OpenParts::MainWindow_ptr _mainWindow);

  void setParentID (OpenParts::Id _id) { parentID = _id; }

  virtual void select (bool flag = false);
  
  virtual void activate (int xoff, int yoff);
  virtual void deactivate ();

protected:
  void calcBoundingBox ();

private:
  KIllustratorChild *child;
  KIllustratorFrame *view;
  KOffice::MainWindow_var mainWindow;
  OpenParts::Id parentID;
  QRect initialGeom, oldGeom;
};

#endif

