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

#include "XfigImport.h"
#include "GGroup.h"
#include "GPolyline.h"
#include "GOval.h"
#include "GPolygon.h"
#include "GText.h"
#include <fstream.h>
#include <limits.h>
#include <math.h>
#include <algorithm>

#define RAD_FACTOR 180.0 / M_PI

unsigned int colors[] = {
    0x000090,
    0x0000b0,
    0x0000d0,
    0x87ceff,
    0x009000,
    0x00b000,
    0x00d000,
    0x009090,
    0x00b0b0,
    0x00d0d0,
    0x900000,
    0xb00000,
    0xd00000,
    0x900090,
    0xb000b0,
    0xd000d0,
    0x803000,
    0xa04000,
    0xc06000,
    0xff8080,
    0xffa0a0,
    0xffc0c0,
    0xffe0e0,
    0xffd700
};

struct PSFont {
  const char* family;
  QFont::Weight weight;
  bool italic;
} psFontTable[] = {
  { "times", QFont::Normal, false },      // Times Roman
  { "times", QFont::Normal, true },       // Times Italic
  { "times", QFont::Bold, false },        // Times Bold
  { "times", QFont::Bold, true },         // Times Bold Italic
  { "avantgarde", QFont::Normal, false },   // AvantGarde Book
  { "avantgarde", QFont::Normal, true },    // AvantGarde Book Oblique
  { "avantgarde", QFont::DemiBold, false }, // AvantGarde Demi
  { "avantgarde", QFont::DemiBold, true },  // AvantGarde Demi Oblique
  { "bookman", QFont::Light, false },     // Bookman Light
  { "bookman", QFont::Light, true },      // Bookman Light Italic
  { "bookman", QFont::DemiBold, false },  // Bookman Demi
  { "bookman", QFont::DemiBold, true },   // Bookman Demi Italic
  { "courier", QFont::Normal, false },    // Courier
  { "courier", QFont::Normal, true },     // Courier Oblique
  { "courier", QFont::Bold, false },      // Courier Bold
  { "courier", QFont::Bold, true },       // Courier Bold Oblique
  { "helvetica", QFont::Normal, false },  // Helvetica
  { "helvetica", QFont::Normal, true },   // Helvetica Oblique
  { "helvetica", QFont::Bold, false },    // Helvetica Bold
  { "helvetica", QFont::Bold, true },     // Helvetica Bold Oblique
  { "helvetica", QFont::Normal, false },  // Helvetica Narrow
  { "helvetica", QFont::Normal, true },   // Helvetica Narrow Oblique
  { "helvetica", QFont::Bold, false },    // Helvetica Narrow Bold
  { "helvetica", QFont::Bold, true },     // Helvetica Narrow Bold Oblique
  { "newcenturyschoolbook", QFont::Normal, false },// New Century Schoolbook 
  { "newcenturyschoolbook", QFont::Normal, true }, // New Century Italic
  { "newcenturyschoolbook", QFont::Bold, false },  // New Century Bold
  { "newcenturyschoolbook", QFont::Bold, true },   // New Century Bold Italic
  { "palatino", QFont::Normal, false },   // Palatino Roman
  { "palatino", QFont::Normal, true },    // Palatino Italic
  { "palatino", QFont::Bold, false },     // Palatino Bold
  { "palatino", QFont::Bold, true },      // Palatino Bold Italic
  { "symbol", QFont::Normal, false },     // Symbol
  { "zapfchancery", QFont::Normal, false }, // Zapf Chancery Medium Italic
  { "zapfdingbats", QFont::Normal, false }, // Zapf Dingbats
};

bool greater_than (const pair<int, GObject*>& x, 
		const pair<int, GObject*>& y) {
  return x.first > y.first;
}

int hexstrToInt (const char *str) {
    const int fak[] = { 16, 1 };
   int value = 0, v;

   for (int i = 0; i < 2; i++) {
       if (str[i] >= '0' && str[i] <= '9')
	   v = str[i] - '0';
       else
	   v = str[i] - 'a' + 10;
       value += v * fak[i];
   }

   return value;
}

XfigImport::XfigImport () {
}

XfigImport::~XfigImport () {
}

bool XfigImport::setup (GDocument* doc, const char* format) {
  fig_resolution = 1200.0 / 72.0;
  coordinate_system = 2;

  colorTable.insert (0, new QColor (black));
  colorTable.insert (1, new QColor (blue));
  colorTable.insert (2, new QColor (green));
  colorTable.insert (3, new QColor (cyan));
  colorTable.insert (4, new QColor (red));
  colorTable.insert (5, new QColor (magenta));
  colorTable.insert (6, new QColor (yellow));
  colorTable.insert (7, new QColor (white));

  for (int i = 0; i <= 23; i++) 
      colorTable.insert (i + 8, new QColor (colors[i]));

  objList.clear ();

  return true;
}

bool XfigImport::importFromFile (GDocument *doc) {
  char buf[255];
  int value;
  KoPageLayout layout;

  ifstream fin (inputFileName ());
  if (! fin)
    return false;

  layout = doc->pageLayout ();

  fin.getline (buf, 255);
  if (::strncmp (buf, "#FIG 3", 6)) {
    cerr << "ERROR: no xfig file or wrong header" << endl;
    return false;
  }

  if (buf[7] == '2') {
      version = 320;
  }
  else if (buf[7] == '1') {
      version = 310;
  }
  else {
    cerr << "ERROR: unsupported xfig version" << endl;
    return false;
  }

  /*
   * read the header
   */

  // orientation
  fin.getline (buf, 255);
  if (::strcmp (buf, "Landscape") == 0)
      layout.orientation = PG_LANDSCAPE;
  else if (::strcmp (buf, "Portrait") == 0)
      layout.orientation = PG_PORTRAIT;
  else 
      cerr << "ERROR: invalid orientation" << endl;

  // justification (don't know how to handle this)
  fin.getline (buf, 255);

  // units
  fin.getline (buf, 255);
  if (::strcmp (buf, "Metric") == 0)
      layout.unit = PG_MM;
  else if (::strcmp (buf, "Inches") == 0)
      layout.unit = PG_INCH;
  else 
      cerr << "ERROR: invalid units" << endl;

  if (version >= 320) {
      // paper size (don't know how to handle this)
      fin.getline (buf, 255);
      
      // magnification
      float magnification;
      fin >> magnification;
      fin.ignore (INT_MAX, '\n');
      
      //multiple page (not supported yet)
      fin.getline (buf, 255);
      
      // transparent color (not supported yet)
      int transColor;
      fin >> transColor;
      fin.ignore (INT_MAX, '\n');
  }

  // resolution and coordinate system
  fin >> value >> coordinate_system;
  fig_resolution = value / 72.0;
  fin.ignore (INT_MAX, '\n');

  // now read in the objects
  while (! fin.eof ()) {
    int tag = -1;
    fin >> tag;
    if (tag == -1) {
      // EOF
      buildDocument (doc);
      return true;
    }

    switch (tag) {
    case 0:
      // a color pseudo object
      parseColorObject (fin);
      break;
    case 1:
      // a ellipse
      parseEllipse (fin, doc);
      break;
    case 2:
      // a polyline
      parsePolyline (fin, doc);
      break;
    case 3:
      // a spline
      cout << "spline\n";
      break;
    case 4:
      // a text
      parseText (fin, doc);
      break;
    case 5:
      // an arc
      parseArc (fin, doc);
      break;
    case 6:
      // a compound object
      parseCompoundObject (fin, doc);
      break;
    case -6:
      // end of compound object --> ignore it
      break;
    default:
      // should not occur
      cerr << "unknown object type: " << tag << endl;
      break;
    }
  }
  buildDocument (doc);
  return true;
}

void XfigImport::parseColorObject (istream& fin) {
    int number, red, green, blue;
    char buf[20], red_str[3], green_str[3], blue_str[3];

    fin >> number >> buf;
    strncpy (red_str, &buf[1], 2); red_str[2] = '\0';
    strncpy (green_str, &buf[3], 2); green_str[2] = '\0';
    strncpy (blue_str, &buf[5], 2); blue_str[2] = '\0';

    red = hexstrToInt (red_str);
    green = hexstrToInt (green_str);
    blue = hexstrToInt (blue_str);

    colorTable.insert (number, new QColor (red, green, blue));
}

void XfigImport::parseArc (istream& fin, GDocument* doc) {
  int sub_type, line_style, thickness, pen_color, fill_color,
    depth, pen_style, area_fill, cap_style, direction, 
    forward_arrow, backward_arrow, x1, y1, x2, y2, x3, y3;
  float center_x, center_y;
  float style_val;
  GOval *obj = new GOval ();

  // first line
  fin >> sub_type >> line_style >> thickness >> pen_color >> fill_color
      >> depth >> pen_style >> area_fill >> style_val >> cap_style
      >> direction >> forward_arrow >> backward_arrow
      >> center_x >> center_y >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;

  if (forward_arrow > 0) {
    // forward arow line
    fin.ignore (INT_MAX, '\n');
  }

  if (backward_arrow > 0) {
    // backward arrow line
    fin.ignore (INT_MAX, '\n');
  }

  // compute radius
  float dx = x1 - center_x;
  float dy = y1 - center_y;
  float radius = sqrt (dx * dx + dy * dy);

  Coord p1 ((center_x - radius) / fig_resolution,
	    (center_y - radius) / fig_resolution);
  Coord p2 ((center_x + radius) / fig_resolution,
	    (center_y + radius) / fig_resolution);

  obj->setStartPoint (p1);
  obj->setEndPoint (p2);

  if (sub_type == 0)
      obj->setOutlineShape (GObject::OutlineInfo::PieShape);
  else if (sub_type == 1)
      obj->setOutlineShape (GObject::OutlineInfo::ArcShape);

  p1 = Coord (center_x / fig_resolution, center_y /fig_resolution);
  p2 = Coord (x1 / fig_resolution, y1 /fig_resolution);
  float m = ((p2.y () - p1.y ()) / (p2.x () - p1.x ()));
  float angle1 = atan (m) * RAD_FACTOR;

  p2 = Coord (x3 / fig_resolution, y3 /fig_resolution);
  m = ((p2.y () - p1.y ()) / (p2.x () - p1.x ()));
  float angle2 = atan (m) * RAD_FACTOR;

  obj->setAngles (angle1, angle2);

  // now set the properties
  setProperties (obj, pen_color, pen_style, thickness, area_fill, fill_color);

  objList.push_back (pair<int, GObject*> (depth, obj));
}

void XfigImport::parseEllipse (istream& fin, GDocument* doc) {
  int sub_type, line_style, thickness, pen_color, fill_color,
    depth, pen_style, area_fill, direction, center_x, center_y, 
    radius_x, radius_y, start_x, start_y, end_x, end_y;
  float style_val, angle;
  GOval *obj = new GOval ();

  // first line
  fin >> sub_type >> line_style >> thickness >> pen_color >> fill_color
      >> depth >> pen_style >> area_fill >> style_val >> direction
      >> angle >> center_x >> center_y >> radius_x >> radius_y
      >> start_x >> start_y >> end_x >> end_y;
  Coord p1, p2;

  p1 = Coord ((center_x - radius_x) /fig_resolution,
	      (center_y - radius_y) /fig_resolution);
  p2 = Coord ((center_x + radius_x) /fig_resolution,
	      (center_y + radius_y) /fig_resolution);

  obj->setStartPoint (p1);
  obj->setEndPoint (p2);

  // now set the properties
  setProperties (obj, pen_color, pen_style, thickness, area_fill, fill_color);

  objList.push_back (pair<int, GObject*> (depth, obj));
}

void XfigImport::parsePolyline (istream& fin, GDocument* doc) {
  int sub_type, line_style, thickness, pen_color, fill_color,
    depth, pen_style, area_fill, join_style, cap_style, radius, 
    forward_arrow, backward_arrow, npoints;
  float style_val;
  GPolyline *obj = NULL;

  // first line
  fin >> sub_type >> line_style >> thickness >> pen_color >> fill_color
      >> depth >> pen_style >> area_fill >> style_val >> join_style 
      >> cap_style >> radius >> forward_arrow >> backward_arrow 
      >> npoints;
  fin.ignore (INT_MAX, '\n');

  switch (sub_type) {
  case 1: // polyline
    obj = new GPolyline ();
    break;
  case 2: // box
    obj = new GPolygon ();
    break;
  case 3: // polygon
    obj = new GPolygon ();
    break;
  case 4: // arc-box
    obj = new GPolygon ();
    break;
  case 5: // imported picture
    return;
    break;
  default:
    // doesn't occur
    cout << "unknown subtype: " << sub_type << endl;
    break;
  }

  assert (obj != NULL);

  if (forward_arrow > 0) {
    // forward arow line
    fin.ignore (INT_MAX, '\n');
  }

  if (backward_arrow > 0) {
    // backward arrow line
    fin.ignore (INT_MAX, '\n');
  }

  // points line
  for (int i = 0; i < npoints; i++) {
    int x, y;
    fin >> x >> y;
    if ((sub_type == 2 || sub_type == 3) && i == npoints -1)
      // first point == last point
      break;

    Coord p (x / fig_resolution, y / fig_resolution);
    obj->_addPoint (i, p);
  }

  // now set the properties
  setProperties (obj, pen_color, line_style, thickness, area_fill, fill_color);

  // and insert the object
  objList.push_back (pair<int, GObject*> (depth, obj));
}

void XfigImport::parseSpline (istream& fin, GDocument* doc) {
}

void XfigImport::parseText (istream& fin, GDocument* doc) {
  int sub_type, color, depth, pen_style, font, font_flags, x, y;
  float font_size, angle, height, length;
  GText *obj = new GText ();
  char c;
  char ocode[4];
  bool finished = false;
  QString text;
  QFont qfont;

  fin >> sub_type >> color >> depth >> pen_style >> font >> font_size
      >> angle >> font_flags >> height >> length >> x >> y;

  if (font_flags & 4) {
    // PostScript font
    if (font == -1)
      font = 0;
    qfont = QFont (psFontTable[font].family, qRound (font_size), 
		   psFontTable[font].weight, psFontTable[font].italic);
  }
  else {
    // LaTeX font
    switch (font) {
    case 1: // Roman
      qfont.setFamily ("times");
      break;
    case 2: // Bold
      qfont.setBold (true);
      break;
    case 3: // Italic
      qfont.setItalic (true);
      break;
    case 4: // Sans Serif
      qfont.setFamily ("helvetica");
      break;
    case 5: // Typewriter
      qfont.setFamily ("Courier");
      break;
    default:
      break;
    }
  }
  qfont.setPointSize (qRound (font_size));
  obj->setFont (qfont);

  while (! finished) {
    fin.get (c);
    if (c == '\\') {
      fin.get (ocode, 4);
      int code = (ocode[0] - '0') * 64 + 
	(ocode[1] - '0') * 8 + 
	(ocode[2] - '0');
      if (code == 1)
	finished = true;
      else
	text += (char) code;
    }
    else
      text += c;
  }
  obj->setText (text);

  if (sub_type == 1) {
    // x, y is the lower center
    x -= qRound (length / 2);
  }
  else if (sub_type == 2) {
    // x, y is the lower right corner
    x -= qRound (length);
  }
  Coord origin (x / fig_resolution, y / fig_resolution - qfont.pointSize ());
  obj->setOrigin (origin);

  if (angle != 0) {
    // rotate the text 
    float nangle = angle * RAD_FACTOR;
    QWMatrix m1, m2, m3;
    Coord rotCenter;
 
    if (sub_type == 0) {
      rotCenter = Coord (obj->boundingBox ().left (), 
			 obj->boundingBox ().bottom ());
    }
    else if (sub_type == 1) {
      rotCenter = Coord (obj->boundingBox ().width () / 2,
			 obj->boundingBox ().bottom ());
    }
    else if (sub_type == 2) {
      rotCenter = Coord (obj->boundingBox ().right (), 
			 obj->boundingBox ().bottom ());
    }
    m1.translate (-rotCenter.x (), -rotCenter.y ());
    m2.rotate (-nangle);
    m3.translate (rotCenter.x (), rotCenter.y ());
    obj->transform (m1);
    obj->transform (m2);
    obj->transform (m3, true);
  }

  objList.push_back (pair<int, GObject*> (depth, obj));
}

void XfigImport::parseCompoundObject (istream& fin, GDocument* doc) {
  int upperright_x, upperright_y, lowerleft_x, lowerleft_y;

  fin >> upperright_x >> upperright_y >> lowerleft_x >> lowerleft_y;
  fin.ignore (INT_MAX, '\n');
}


/**
 * Copy all parsed objects from the sorted list to the document.
 */
void XfigImport::buildDocument (GDocument *doc) {
  doc->setAutoUpdate (false);
  objList.sort (greater_than);
  list<pair<int, GObject*> >::iterator i = objList.begin ();
  for (; i != objList.end (); i++) {
    GObject* obj = i->second;
    obj->ref ();
    doc->insertObject (obj);
  }
  doc->setAutoUpdate (true);
}

void XfigImport::setProperties (GObject* obj, int pen_color, int style, 
				int thickness, int area_fill, int fill_color) {
  if (pen_color >= 0)
    obj->setOutlineColor (*colorTable[pen_color]);

  if (style < 1)
    obj->setOutlineStyle (SolidLine);
  else if (style == 1)
    obj->setOutlineStyle (DashLine);
  else if (style == 2)
    obj->setOutlineStyle (DotLine);

  obj->setOutlineWidth (thickness * 72.0 / 80.0);

  if (area_fill == -1)
    obj->setFillStyle (NoBrush);
  else {
    obj->setFillStyle (SolidPattern);
    if (fill_color < 1) {
      // for BALCK or DEFAULT color
      int val = qRound ((20 - area_fill) * 255.0 / 20.0);
      obj->setFillColor (QColor (val, val, val));
    }
    else if (fill_color == 7) {
      // for WHITE color
      int val = qRound ( area_fill * 255.0 / 20.0);
      obj->setFillColor (QColor (val, val, val));
    }
    else
      obj->setFillColor (*colorTable[fill_color]);
  }
}

