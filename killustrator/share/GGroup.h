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

#ifndef GGroup_h_
#define GGroup_h_

#include <list>

using std::list;

#include <Coord.h>
#include <GObject.h>

class GGroup : public GObject {
  Q_OBJECT
public:
  GGroup ();
  GGroup (const QDomElement &element);
  GGroup (const GGroup& obj);
  ~GGroup ();

  void addObject (GObject* obj);

  virtual void draw (QPainter& p, bool withBasePoints = false,
		     bool outline = false);
  virtual bool contains (const Coord& p);

  virtual QString typeName () const;

  virtual GObject* copy ();
  virtual GObject* clone (const QDomElement &element);

  virtual QDomElement writeToXml(QDomDocument &document);

  const list<GObject*>& getMembers() { return members; }

  void calcBoundingBox ();

  void printInfo ();

protected slots:
  void propagateProperties (GObject::Property prop, int mask);

private:
  list<GObject*> members;
};

#endif
