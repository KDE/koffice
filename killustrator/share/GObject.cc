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
#include "GObject.h"
#include "GObject.moc"

#include "GPolyline.h"
#include "GPolygon.h"
#include "GOval.h"
#include "GText.h"
#include "GGroup.h"
#include "GClipart.h"
#include "GBezier.h"
#include "Gradient.h"

QDict<GObject> GObject::prototypes;
GObject::OutlineInfo GObject::defaultOutlineInfo;
GObject::FillInfo GObject::defaultFillInfo;

void GObject::setDefaultOutlineInfo (const OutlineInfo& oi) {
  if (oi.mask & OutlineInfo::Color)
    defaultOutlineInfo.color = oi.color;
  if (oi.mask & OutlineInfo::Style)
    defaultOutlineInfo.style = oi.style;
  if (oi.mask & OutlineInfo::Width)
    defaultOutlineInfo.width = oi.width;
}

void GObject::setDefaultFillInfo (const FillInfo& fi) {
  if (fi.mask & FillInfo::Color) 
    defaultFillInfo.color = fi.color;
  if (fi.mask & FillInfo::FillStyle)
    defaultFillInfo.fstyle = fi.fstyle;
  if (fi.mask & FillInfo::Pattern)
    defaultFillInfo.pattern = fi.pattern;
  if (fi.mask & FillInfo::GradientInfo)
    defaultFillInfo.gradient = fi.gradient;
}

GObject::OutlineInfo GObject::getDefaultOutlineInfo () {
  return defaultOutlineInfo;
}

GObject::FillInfo GObject::getDefaultFillInfo () {
  return defaultFillInfo;
}

GObject::GObject () {
  sflag = false;
  layer = 0L;
  inWork = false;

  outlineInfo = defaultOutlineInfo;
  outlineInfo.mask = OutlineInfo::Color | OutlineInfo::Style | 
    OutlineInfo::Width | OutlineInfo::Custom;
  outlineInfo.roundness = 0;
  outlineInfo.shape = OutlineInfo::DefaultShape;
  outlineInfo.startArrowId = outlineInfo.endArrowId = 0;

  fillInfo = defaultFillInfo;
  fillInfo.mask = FillInfo::Color | FillInfo::FillStyle;

  rcount = 1;
}

GObject::GObject (const list<XmlAttribute>& attribs) {
  list<XmlAttribute>::const_iterator first = attribs.begin ();
  layer = 0L;
  inWork = false;

  while (first != attribs.end ()) {
    const string& attr = (*first).name ();
    if (attr == "matrix") {
      tMatrix = (*first).matrixValue ();
      iMatrix = tMatrix.invert ();
      tmpMatrix = tMatrix;
    }
    else if (attr == "strokecolor")
      outlineInfo.color = (*first).colorValue ();
    else if (attr == "strokestyle")
      outlineInfo.style = (PenStyle) (*first).intValue ();
    else if (attr == "linewidth")
      outlineInfo.width = (*first).floatValue ();
    else if (attr == "fillcolor")
      fillInfo.color = (*first).colorValue ();
    else if (attr == "fillstyle") {
      // just a temporary hack
      int fill = (*first).intValue ();
      if (fill > 1) {
	fillInfo.fstyle = FillInfo::PatternFill;
	fillInfo.pattern = (BrushStyle) fill;
      }
      else if (fill == 1)
	fillInfo.fstyle = FillInfo::SolidFill;
      else
	fillInfo.fstyle = FillInfo::NoFill;
    }
    first++;
  }
}

GObject::GObject (const GObject& obj) : QObject() 
{
  sflag = false;
  outlineInfo = obj.outlineInfo;
  fillInfo = obj.fillInfo;
  tMatrix = obj.tMatrix;
  tmpMatrix = tMatrix;
  iMatrix = obj.iMatrix;
  layer = obj.layer;
  inWork = false;

  rcount = 1;
}

GObject::~GObject () {
}

void GObject::ref () {
  rcount++;
}

void GObject::unref () {
  if (--rcount == 0)
    delete this;
}

void GObject::updateRegion (bool recalcBBox) {
  Rect newbox = box;
  
  if (recalcBBox) {
    Rect oldbox = box;
    calcBoundingBox ();
    newbox = box.unite (oldbox);
  }

  if (isSelected ())
    // the object is selected, so enlarge the update region in order
    // to redraw the handle
    newbox.enlarge (8);

  emit changed (newbox);
}

void GObject::transform (const QWMatrix& m, bool update) {
  tMatrix = tMatrix * m;
  iMatrix = tMatrix.invert ();
  initTmpMatrix ();
  gShape.setInvalid ();
  if (update) 
    updateRegion ();
}

void GObject::initTmpMatrix () {
  tmpMatrix = tMatrix;
}

void GObject::ttransform (const QWMatrix& m, bool update) {
  tmpMatrix = tmpMatrix * m;
  if (update) 
    updateRegion ();
}

void GObject::setOutlineInfo (const GObject::OutlineInfo& info) {
  if (info.mask & OutlineInfo::Color)
    outlineInfo.color = info.color;
  if (info.mask & OutlineInfo::Style)
    outlineInfo.style = info.style;
  if (info.mask & OutlineInfo::Width)
    outlineInfo.width = info.width;
  if (info.mask & OutlineInfo::Custom) {
    outlineInfo.roundness = info.roundness;
    outlineInfo.shape = info.shape;
    outlineInfo.startArrowId = info.startArrowId;
    outlineInfo.endArrowId = info.endArrowId;
  }
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, info.mask);
}

GObject::OutlineInfo GObject::getOutlineInfo () const {
  return outlineInfo;
}
  
void GObject::setOutlineShape (OutlineInfo::Shape s) {
  outlineInfo.shape = s;
  updateRegion ();
  emit propertiesChanged (Prop_Outline, OutlineInfo::Custom);
}

void GObject::setOutlineColor (const QColor& color) {
  outlineInfo.color = color;
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, OutlineInfo::Color);
}

void GObject::setOutlineStyle (PenStyle style) {
  outlineInfo.style = style;
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, OutlineInfo::Style);
}

void GObject::setOutlineWidth (float width) {
  outlineInfo.width = width;
  updateRegion (false);
  emit propertiesChanged (Prop_Outline, OutlineInfo::Width);
}

const QColor& GObject::getOutlineColor () const {
  return outlineInfo.color;
}

PenStyle GObject::getOutlineStyle () const {
  return outlineInfo.style;
}

float GObject::getOutlineWidth () const {
  return outlineInfo.width;
}

void GObject::setFillInfo (const GObject::FillInfo& info) {
  if (info.mask & FillInfo::Color)
    fillInfo.color = info.color;
  if (info.mask & FillInfo::FillStyle)
    fillInfo.fstyle = info.fstyle;
  if (info.mask & FillInfo::Pattern)
    fillInfo.pattern = info.pattern;
  if (info.mask & FillInfo::GradientInfo)
    fillInfo.gradient = info.gradient;
  gShape.setInvalid ();
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, info.mask);
}

GObject::FillInfo GObject::getFillInfo () const {
  return fillInfo;
}
  
void GObject::setFillColor (const QColor& color) {
  fillInfo.color = color;
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::Color);
}

const QColor& GObject::getFillColor () const {
  return fillInfo.color;
}

void GObject::setFillPattern (BrushStyle b) {
  fillInfo.pattern = b;
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::Pattern);
}

void GObject::setFillGradient (const Gradient& g) {
  fillInfo.gradient = g;
  gShape.setInvalid ();
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::GradientInfo);
}

void GObject::setFillStyle (GObject::FillInfo::Style s) {
  fillInfo.fstyle = s;
  gShape.setInvalid ();
  updateRegion (false);
  emit propertiesChanged (Prop_Fill, FillInfo::FillStyle);
}

GObject::FillInfo::Style GObject::getFillStyle () const {
  return fillInfo.fstyle;
}

const Gradient& GObject::getFillGradient () const {
  return fillInfo.gradient;
}

BrushStyle GObject::getFillPattern () const {
  return fillInfo.pattern;
}

void GObject::select (bool flag) {
  sflag = flag;
}

bool GObject::contains (const Coord& p) {
  return box.contains (p);
}

void GObject::draw (Painter&, bool) {
}

void GObject::setLayer (GLayer* l) {
  layer = l;
}

void GObject::writeToPS (ostream& os) {
  // line width
  os << outlineInfo.width << " setlinewidth\n";
  // pen style
  os << (int) outlineInfo.style << " SPSt\n";
  // outline color
  os << outlineInfo.color.red () / 255.0 << ' ' 
     << outlineInfo.color.green () / 255.0
     << ' ' << outlineInfo.color.blue () / 255.0 << " DOCol\n";
  // fill color
  os << fillInfo.color.red () / 255.0 << ' ' 
     << fillInfo.color.green () / 255.0
     << ' ' << fillInfo.color.blue () / 255.0 << " DFCol\n";
  // transformation matrix
  os << '[' << tMatrix.m11 () << ' ' << tMatrix.m12 () << ' ' 
     << tMatrix.m21 () << ' ' << tMatrix.m22 () << ' '
     << tMatrix.dx () << ' ' << tMatrix.dy () << "] SMatrix\n";
}

void GObject::updateBoundingBox (const Rect& r) {
  box = r.normalize ();
}

void GObject::updateBoundingBox (const Coord& p1, const Coord& p2) {
  Rect r (p1, p2);
  updateBoundingBox (r);
}

GOState* GObject::saveState () {
  GOState* state = new GOState;
  GObject::initState (state);
  return state;
}

void GObject::initState (GOState* state) {
  state->matrix = tMatrix;
  state->fInfo = fillInfo;
  state->oInfo = outlineInfo;
}

void GObject::restoreState (GOState* state) {
  tMatrix = state->matrix;
  iMatrix = tMatrix.invert ();
  tmpMatrix = tMatrix;
  setFillInfo (state->fInfo);
  setOutlineInfo (state->oInfo);

  updateRegion ();
}

void GObject::calcUntransformedBoundingBox (const Coord& tleft, 
					    const Coord& tright,
					    const Coord& bright, 
					    const Coord& bleft) {
  Coord p[4];
  Rect r;

  p[0] = tleft.transform (tmpMatrix);
  p[1] = tright.transform (tmpMatrix);
  p[2] = bleft.transform (tmpMatrix);
  p[3] = bright.transform (tmpMatrix);

  r.left (p[0].x ());
  r.top (p[0].y ());
  r.right (p[0].x ());
  r.bottom (p[0].y ());

  for (unsigned int i = 1; i < 4; i++) {
    r.left (QMIN(p[i].x (), r.left ()));
    r.top (QMIN(p[i].y (), r.top ()));
    r.right (QMAX(p[i].x (), r.right ()));
    r.bottom (QMAX(p[i].y (), r.bottom ()));
  }
  updateBoundingBox (r);
}

void GObject::initBrush (QBrush& brush) {
  switch (fillInfo.fstyle) {
  case GObject::FillInfo::NoFill:
    brush.setStyle (NoBrush);
    break;
  case GObject::FillInfo::SolidFill:
    brush.setColor (fillInfo.color);
    brush.setStyle (SolidPattern);
    break;
  case GObject::FillInfo::PatternFill:
    brush.setColor (fillInfo.color);
    brush.setStyle (fillInfo.pattern);
    break;
  default:
    brush.setStyle (NoBrush);
    break;
  }
}

void GObject::initPen (QPen& pen) {
  pen.setColor (inWork ? black : outlineInfo.color);
  pen.setWidth ((uint) outlineInfo.width);
  pen.setStyle (inWork ? SolidLine : outlineInfo.style);
}

void GObject::writePropertiesToXml (XmlWriter& xml) {
  xml.addAttribute ("matrix", tMatrix);
  xml.addAttribute ("strokecolor", outlineInfo.color);
  xml.addAttribute ("strokestyle", (int) outlineInfo.style);
  xml.addAttribute ("linewidth", outlineInfo.width);
  xml.addAttribute ("fillcolor", fillInfo.color);
  xml.addAttribute ("fillstyle", (int) fillInfo.fstyle);
}

void GObject::printInfo () {
    cout << className () << " bbox = [" << boundingBox () << "]" << endl;
}

