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

#include <stdlib.h>
#include <iostream.h>
#include <math.h>
#include <assert.h>
#include "GPolygon.h"
#include "GPolygon.moc"
#include "GradientShape.h"
#include "GCurve.h"

#include <qpointarray.h>
#include <klocale.h>
#include <kapp.h>

#define Roundness outlineInfo.roundness

static const int xfactors[] = { 0, 1, -1, 0, 0, -1, 1, 0 };
static const int yfactors[] = { 1, 0, 0, 1, -1, 0, 0, -1 };

static bool line_intersects (const Coord& p11, const Coord& p12,
	const Coord& p21, const Coord& p22) {
  float x11, x12, y11, y12, x21, x22, y21, y22;
  float m1, m2, n1, n2;
  float xp, yp;

  if (p11.x () <= p12.x ()) {
    x11 = p11.x (); y11 = p11.y ();
    x12 = p12.x (); y12 = p12.y ();
  }
  else {
    x11 = p12.x (); y11 = p12.y ();
    x12 = p11.x (); y12 = p11.y ();
  }
  if (p21.x () <= p22.x ()) {
    x21 = p21.x (); y21 = p21.y ();
    x22 = p22.x (); y22 = p22.y ();
  }
  else {
    x21 = p22.x (); y21 = p22.y ();
    x22 = p21.x (); y22 = p21.y ();
  }

  // compute ascent of first line
  m1 = (y12 - y11) / (x12 - x11);
  n1 = y11 - m1 * x11;

  // compute ascent of second line
  m2 = (y22 - y21) / (x22 - x21);
  n2 = y21 - m2 * x12;

  // special case: first line is perpendicular (Greetings to DP ;-))
  if (x12 == x11) {
    yp = m2 * x12 + n2;
    if ((y11 <= yp && yp <= y12 || y12 <= yp && yp <= y11) &&
        (x21 <= x11 && x11 <= x22 || x22 <= x11 && x11 <= x21)) {
      return true;
    }
  }

  // now compute the intersection point...
  xp = (n2 - n1) / (m1 - m2);
  yp = m1 * xp + n1;

  if (x11 <= xp && xp <= x12 && x21 <= xp && xp <= x22) {
    if ((y11 <= yp && yp <= y12 || y11 >= yp && yp >= y12) &&
        (y21 <= yp && yp <= y22 || y21 >= yp && yp >= y22))
      return true;
  }
  return false;
}

GPolygon::GPolygon (GPolygon::Kind pkind) : GPolyline () {
  points.setAutoDelete (true);
  kind = pkind;
}
  
GPolygon::GPolygon (const list<XmlAttribute>& attribs, Kind pkind) 
  : GPolyline (attribs) {
  points.setAutoDelete (true);
  kind = pkind;
  if (kind != PK_Polygon) {
    list<XmlAttribute>::const_iterator first = attribs.begin ();
    float x = 0, y = 0, w = 0, h = 0;
    
    while (first != attribs.end ()) {
      const string& attr = (*first).name ();
      if (attr == "x")
	x = (*first).floatValue ();
      else if (attr == "y")
	y = (*first).floatValue ();
      else if (attr == "width")
	w = (*first).floatValue ();
      else if (attr == "height")
	h = (*first).floatValue ();
      else if (attr == "rounding")
	Roundness = (*first).floatValue ();
      first++;
    }
    points.append (new Coord (x, y));
    points.append (new Coord (x + w, y));
    points.append (new Coord (x + w, y + h));
    points.append (new Coord (x, y + h));
  }
  calcBoundingBox ();
}

GPolygon::GPolygon (const GPolygon& obj) : GPolyline (obj) {
  kind = obj.kind;
}
  
GPolygon::GPolygon (QList<Coord>& coords) : GPolyline () {
  Coord *p1 = coords.first (), *p2 = 0L;
  bool ready = false;

  while (! ready) {
    p2 = p1;
    p1 = coords.next ();
    if (p1 == 0L) {
      p1 = coords.first ();
      ready = true;
    }
    if (*p1 != *p2)
      points.append (new Coord (*p2));
  }
  kind = PK_Polygon;
  calcBoundingBox ();
}

void GPolygon::setKind (GPolygon::Kind k) {
  if (k != PK_Polygon && points.count () == 4)
    kind = k;
  else if (k == PK_Polygon)
    kind = k;
}

QString GPolygon::typeName () const {
  if (kind == PK_Polygon)
    return SI18N ("Polygon");
  else if (kind == PK_Rectangle)
    return SI18N ("Rectangle");
  else
    return SI18N ("Square");
}

bool GPolygon::isFilled () const {
  return fillInfo.fstyle != GObject::FillInfo::NoFill;
}

void GPolygon::draw (QPainter& p, bool withBasePoints, bool outline) {
  unsigned int i, num;

  QPen pen;
  QBrush brush;
  initPen (pen);
  p.save ();
  p.setPen (pen);
  p.setWorldMatrix (tmpMatrix, true);

  if (! workInProgress () && !outline) {
    initBrush (brush);
    p.setBrush (brush);
    
    if (gradientFill ()) {
      if (! gShape.valid ())
	updateGradientShape (p);
      gShape.draw (p);
    }
  }

  num = points.count ();
  if (kind == PK_Polygon) {
    QPointArray parray (num);
    for (i = 0; i < num; i++) {
      parray.setPoint (i, (int) points.at (i)->x (), 
		       (int) points.at (i)->y ());
    }
    p.drawPolygon (parray);
  }
  else {
    const Coord& p1 = *(points.at (0));
    const Coord& p2 = *(points.at (2));
    if (Roundness != 0)
      p.drawRoundRect (p1.x (), p1.y (),
		       p2.x () - p1.x (),
		       p2.y () - p1.y (), Roundness, Roundness);
    else
      Painter::drawRect (p, p1.x (), p1.y (), p2.x () - p1.x (), 
			 p2.y () - p1.y ());
  }

  p.restore ();
  p.save ();
  if (withBasePoints) {
    p.setPen (black);
    // p.setBrush (white);
    if (kind == PK_Polygon || Roundness == 0) {
      for (i = 0; i < num; i++) {
	Coord c = points.at (i)->transform (tmpMatrix);
	int x = (int) c.x ();
	int y = (int) c.y ();
	p.drawRect (x - 2, y - 2, 4, 4);
      }
    }
    else {
      for (i = 0; i < rpoints.count (); i++) {
	Coord c = rpoints.at (i)->transform (tmpMatrix);
	Painter::drawRect (p, c.x () - 2, c.y () - 2, 4, 4);
      }
    }
  }
  p.setClipping (false);
  p.restore ();
}

void GPolygon::insertPoint (int idx, const Coord& p, bool update) {
  if (kind != PK_Polygon) 
    kind = PK_Polygon;
  GPolyline::insertPoint (idx, p, update);
}

bool GPolygon::contains (const Coord& p) {
  if (box.contains (p)) {
    QPoint pp = iMatrix.map (QPoint ((int) p.x (), (int) p.y ()));
    if (kind != PK_Polygon) {
      // the simplest case: the polygon is a square or a rectangle
      Rect r (*(points.at (0)), *(points.at (2)));
      return r.normalize ().contains (Coord (pp.x (), pp.y ()));
    }
    else
      return inside_polygon (Coord (pp.x (), pp.y ()));
  }
  return false;
}

void GPolygon::setEndPoint (const Coord& p) {
  assert (kind != PK_Polygon);

  gShape.setInvalid ();

  Coord& p0 = *(points.at (0));
  Coord& p2 = *(points.at (2));

  if (kind == PK_Square && p2.x () != 0 && p2.y () != 0) {
    float dx = (float) fabs (p.x () - p0.x ());
    float dy = (float) fabs (p.y () - p0.y ());
    float xoff = p.x () - p0.x ();
    float yoff = p.y () - p0.y ();
    if (dx > dy) {
      p2.x (p.x ());
      p2.y (p0.y () + xoff);
    }
    else {
      p2.x (p0.x () + yoff);
      p2.y (p.y ());
    }
  }
  else
    p2 = p;
  setPoint (1, Coord (p2.x (), p0.y ()));
  setPoint (3, Coord (p0.x (), p2.y ()));
  updateRegion ();
}

void GPolygon::setSymmetricPolygon (const Coord& sp, const Coord& ep, 
				    int nCorners, 
				    bool concave, int sharpness) {
  int i;
  points.clear ();

  float a, angle = 2 * M_PI / nCorners;
  float dx = (float) fabs (sp.x () - ep.x ());
  float dy = (float) fabs (sp.y () - ep.y ());
  float radius = (dx > dy ? dx / 2.0 : dy / 2.0); 
  float xoff = sp.x () + radius;
  float yoff = sp.y () + radius;
  float xp, yp;

  points.append (new Coord (xoff, -radius + yoff));
  if (concave) {
    angle = angle / 2.0;
    a = angle;
    float r = radius - (sharpness / 100.0 * radius);
    for (i = 1; i < nCorners * 2; i++) {
      if (i % 2) {
	xp =  r * sin (a);
	yp = -r * cos (a);
      }
      else {
	xp = radius * sin (a);
	yp = - radius * cos (a);
      }
      a += angle;
      points.append (new Coord (xp + xoff, yp + yoff));
    }
  }
  else {
    a = angle;
    for (i = 1; i < nCorners; i++) {
      xp = radius * sin (a);
      yp = - radius * cos (a);
      a += angle;
      points.append (new Coord (xp + xoff,  yp + yoff));
    }
  }
  updateRegion ();
}

void GPolygon::movePoint (int idx, float dx, float dy) {
  gShape.setInvalid ();
  if (kind == PK_Polygon)
    GPolyline::movePoint (idx, dx, dy);
  else {
    // round the corner
    float xoff, yoff, off;
    float w = points.at (1)->x () - points.at (0)->x ();
    float h = points.at (2)->y () - points.at (1)->y ();
    xoff = dx * 200.0 / w * xfactors[idx];
    yoff = dy * 200.0 / h * yfactors[idx];
    off = (fabs (xoff) > fabs (yoff) ? xoff : yoff);
    Roundness += off;
    if (Roundness < 0) Roundness = 0;
    if (Roundness > 100) Roundness = 100;
    updateRegion ();
  }
}

GObject* GPolygon::copy () {
  return new GPolygon (*this);
}

GObject* GPolygon::clone (const list<XmlAttribute>& attribs) {
  return new GPolygon (attribs);
}

void GPolygon::calcBoundingBox () {
  GPolyline::calcBoundingBox ();
  update_rpoints ();
}

bool GPolygon::isRectangle () const {
  return kind != PK_Polygon;
}

bool GPolygon::inside_polygon (const Coord& p) {
  Coord *p1, *p2;
  Coord t1 (p), t2 (10000.0, p.y ());
  int counter = 0;
  bool ready = false;
  unsigned int i = 0, num = points.count ();

  while (i < num) {
    p1 = points.at (i);
    if (! line_intersects (*p1, *p1, t1, t2)) {
      if (i == num - 1)
	p2 = points.at (0);
      else
	p2 = points.at (i + 1);
      if (line_intersects (*p1, *p2, t1, t2)) {
        counter++;
      }
    }
    i++;
  }
  /*
  p1 = points.first ();
  while (! ready) {
    if (! line_intersects (*p1, *p1, t1, t2)) {
      p2 = points.next ();
      if (p2 == 0L) {
        p2 = points.first ();
        ready = true;
      }
      if (line_intersects (*p1, *p2, t1, t2)) 
        counter++;
      p1 = p2;
    } 
  }
  */
  return counter & 1;
}

void GPolygon::update_rpoints () {
  int i;

  if (kind != PK_Polygon && Roundness > 0) {
    if (points.count () < 4) 
      return;

    float w = points.at (1)->x () - points.at (0)->x ();
    float h = points.at (2)->y () - points.at (1)->y ();
    float xoff = w * Roundness / 200.0;
    float yoff = h * Roundness / 200.0;

    for (i = rpoints.count (); i < 8; i++)
      rpoints.append (new Coord (0, 0));

    for (i = 0; i < 4; i++) {
      rpoints.at (i * 2)->x (points.at (i)->x () + xoff * xfactors[i * 2]);
      rpoints.at (i * 2)->y (points.at (i)->y () + yoff * yfactors[i * 2]);
      rpoints.at (i * 2 + 1)->x (points.at (i)->x () + 
				 xoff * xfactors[i * 2 + 1]);
      rpoints.at (i * 2 + 1)->y (points.at (i)->y () + 
				 yoff * yfactors[i * 2 + 1]);
    }
  }
}

int GPolygon::getNeighbourPoint (const Coord& p) {
  if (kind != PK_Polygon && Roundness > 0) {
    for (unsigned int i = 0; i < rpoints.count (); i++) { 
      Coord c = rpoints.at (i)->transform (tMatrix);
      if (c.isNear (p, NEAR_DISTANCE))
	return i;
    }
    return -1;
 }
  else
    return GPolyline::getNeighbourPoint (p);
}

void GPolygon::writeToXml (XmlWriter& xml) {
  Rect r (*(points.at (0)), *(points.at (2)));
  Rect nr = r.normalize ();

  if (kind == PK_Polygon)
    xml.startTag ("polygon", false);
  else
    xml.startTag ("rectangle", false);

  writePropertiesToXml (xml);
  xml.addAttribute ("x", nr.left ());
  xml.addAttribute ("y", nr.top ());
  xml.addAttribute ("width", nr.width ());
  xml.addAttribute ("height", nr.height ());
  xml.addAttribute ("rounding", (Roundness > 0.1 ? Roundness : 0.0));

  if (kind == PK_Polygon) {
    xml.closeTag (false);
    for (QListIterator<Coord> it (points); it.current (); ++it) {
      xml.startTag ("point", false);
      xml.addAttribute ("x", it.current ()->x ());
      xml.addAttribute ("y", it.current ()->y ());
      xml.closeTag (true);
    }
    xml.endTag ();
  }
  else
    xml.closeTag (true);
}

void GPolygon::updateGradientShape (QPainter& p) {
  // define the rectangular box for the gradient pixmap 
  // (in object coordinate system)
  if (kind != PK_Polygon) {
    const Coord& p1 = *(points.at (0));
    const Coord& p2 = *(points.at (2));
    gShape.setBox (Rect (p1, p2));
  }
  else
    gShape.setBox (calcEnvelope ());

  // define the clipping region
  QWMatrix matrix = p.worldMatrix ();
  unsigned int num = points.count ();
  QPointArray pnts (num);
  for (unsigned int i = 0; i < num; i++)
    pnts.setPoint (i, qRound (points.at (i)->x ()),
		   qRound (points.at (i)->y ()));
  
  if (kind == PK_Polygon || 
      (kind != PK_Polygon && (Roundness == 0 || Roundness == 100))) {
    if (kind == PK_Polygon || Roundness == 0) {
      QRegion region (matrix.map (pnts));
      gShape.setRegion (region);
    }
    else if (Roundness == 100) {
      // special case: ellipse
      unsigned int w, h;
      w = pnts.point (2).x () - pnts.point (0).x ();
      h = pnts.point (2).y () - pnts.point (0).y ();
      QRect rect (pnts.point (0).x (), pnts.point (0).y (), w, h);
      rect = rect.normalize ();
      QPointArray epnts;
      epnts.makeEllipse (rect.x (), rect.y (), 
			 rect.width (), rect.height ());
      gShape.setRegion (QRegion (matrix.map (epnts)));
    }
  }
  else if (kind != PK_Polygon) {
    unsigned int w, h;
    w = pnts.point (2).x () - pnts.point (0).x ();
    h = pnts.point (2).y () - pnts.point (0).y ();
    QRect rect (pnts.point (0), pnts.point (2));
    QRect nrect = rect.normalize ();

    float xrad = nrect.width () * Roundness / 200.0;
    float yrad = nrect.height () * Roundness / 200.0;

    QRect trect (nrect.x (), nrect.y () + yrad, 
		 nrect.width (), nrect.height () - 2 * yrad);
    QPointArray tarray (trect, true);
    QPointArray clip1 = matrix.map (tarray);

    trect = QRect (nrect.x () + xrad, nrect.y (), 
		   nrect.width () - 2 * xrad, nrect.height ());
    tarray = QPointArray (trect, true);
    QPointArray clip2 = matrix.map (tarray);
    

    QRegion region (clip1);
    region = region.unite (QRegion (clip2));

    tarray.makeEllipse (nrect.x (), nrect.y (), xrad * 2 , yrad * 2);
    region = region.unite (matrix.map (tarray));

    tarray.makeEllipse (nrect.right () - (2 * xrad), nrect.y (), 
			xrad * 2, yrad * 2);
    region = region.unite (matrix.map (tarray));

    tarray.makeEllipse (nrect.x (), nrect.bottom () - (2 * yrad), 
			xrad * 2, yrad * 2);
    region = region.unite (matrix.map (tarray));

    tarray.makeEllipse (nrect.right () - (2 * xrad), 
			nrect.bottom () - (2 * yrad), xrad * 2, yrad * 2);
    region = region.unite (matrix.map (tarray));

    gShape.setRegion (region);
  }

  // update the gradient information
  gShape.setGradient (fillInfo.gradient);

  // and create a new gradient pixmap
  gShape.updatePixmap ();
}

void GPolygon::getPath (vector<Coord>& path) {
  unsigned int num = points.count ();
  path.resize (num + 1);
  for (unsigned int i = 0; i < num; i++) {
    const Coord& pi = *points.at (i);
    path[i] = pi.transform (tMatrix);
  }
  path[num] = points.at (0)->transform (tMatrix);
}

// Check for a valid polygon
//  (1) more than 3 points
//  (2) dimension greater or equal 1
bool GPolygon::isValid () {
  if (points.count () >= 3) 
    return GPolyline::isValid ();
  else
    return false;
}

bool GPolygon::splitAt (unsigned int idx, GObject*& obj1, GObject*& obj2) {
  bool result = false;

  if (idx >= 0 && idx < points.count ()) {
    GPolyline* other = new GPolyline (* ((GPolyline *) this));
    other->removeAllPoints ();
    unsigned int i, num = points.count ();
    for (i = idx; i < num; i++) 
      other->_addPoint (other->numOfPoints (), *points.at (i));
    for (i = 0; i <= idx; i++)
      other->_addPoint (other->numOfPoints (), *points.at (i));

    other->calcBoundingBox ();
    obj1 = other;
    obj2 = 0L;
    result = true;
  }
  return result;
}

GCurve* GPolygon::convertToCurve () const {
  GCurve* curve = new GCurve ();
  QListIterator<Coord> it (points);
  Coord p0 = it.current ()->transform (tmpMatrix), p = p0;
  ++it;
  for (; it.current (); ++it) {
    Coord p1 = it.current ()->transform (tmpMatrix);
    curve->addLineSegment (p0, p1);
    p0 = p1;
  }
  curve->addLineSegment (p0, p);
  curve->setClosed (true);
  curve->setOutlineInfo (outlineInfo);
  curve->setFillInfo (fillInfo);
  return curve;
}
