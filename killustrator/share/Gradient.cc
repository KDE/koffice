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

#include <qpainter.h>
#include "Gradient.h"

Gradient::Gradient (const QColor& c1, const QColor& c2, Style s) 
  : color1 (c1), color2 (c2), style (s) {
}

void Gradient::setColor1 (const QColor& c) {
  color1 = c;
}

void Gradient::setColor2 (const QColor& c) {
  color2 = c;
}

void Gradient::setStyle (Gradient::Style s) {
  style = s;
}

const QColor& Gradient::getColor1 () const {
  return color1;
}

const QColor& Gradient::getColor2 () const {
  return color2;
}

Gradient::Style Gradient::getStyle () const {
  return style;
}

QPixmap Gradient::createPixmap (unsigned int width, unsigned int height) {
  QPixmap pix (width, height);
  QPainter p;
  p.begin (&pix);
  switch (style) {
  case Horizontal:
  case Vertical:
    createHVGradient (p, width, height);
    break;
  case Radial:
    pix.fill (color2);
    createRadGradient (p, width, height);
    break;
  case Rectangular:
    createRectGradient (p, width, height);
    break;
  default:
    break;
  }
  p.end ();
  return pix;
}

void Gradient::createHVGradient (QPainter& p, unsigned int width, 
				 unsigned int height) {
  QColor col;
  QPen pen;
  double delta, dd;
  int r, g, b;

  if (style == Horizontal) {
    delta = 1.0 / (double) width;
    dd = width;
  }
  else {
    delta = 1.0 / (double) height;
    dd = height;
  }
  int rdiff = color1.red () - color2.red ();
  int gdiff = color1.green () - color2.green ();
  int bdiff = color1.blue () - color2.blue ();

  for (double d = 0.0; d < 1.0; d += delta) {
    r = color1.red () - qRound (rdiff * d);
    g = color1.green () - qRound (gdiff * d);
    b = color1.blue () - qRound (bdiff * d);
    col.setRgb (r, g, b);
    pen.setColor (col);
    p.setPen (pen);
    int x = qRound (dd * d);
    if (style == Horizontal)
      p.drawLine (x, 0, x, height);
    else
      p.drawLine (0, x, width, x);
  }
}

void Gradient::createRadGradient (QPainter& p, unsigned int width, 
				 unsigned int height) {
  QColor col;
  QPen pen;
  double delta, dd, scalx = 1.0, scaly = 1.0;
  int r, g, b;

  dd = QMAX(width, height) / 2.0;
  delta = 1.0 / dd;
  if (width > height)
      scaly = (double) height / (double) width;
  else
      scalx  = (double) width / (double) height;

  int rdiff = color1.red () - color2.red ();
  int gdiff = color1.green () - color2.green ();
  int bdiff = color1.blue () - color2.blue ();

  unsigned int mx = width / 2;
  unsigned int my = height / 2;
  unsigned int dx, dy;

  for (double d = 1.0; d > 0.0; d -= delta) {
    r = color1.red () - qRound (rdiff * d);
    g = color1.green () - qRound (gdiff * d);
    b = color1.blue () - qRound (bdiff * d);
    col.setRgb (r, g, b);
    pen.setColor (col);
    p.setPen (pen);
    p.setBrush (col);
    dx = qRound (dd * d * scalx);
    dy = qRound (dd * d * scaly);
    p.drawEllipse (mx - dx, my - dy, dx * 2, dy * 2);
  }
}

void Gradient::createRectGradient (QPainter& p, unsigned int width, 
				   unsigned int height) {
  QColor col;
  QPen pen;
  double delta, dd, scalx = 1.0, scaly = 1.0;
  int r, g, b;

  dd = QMAX(width, height) / 2.0;
  delta = 1.0 / dd;
  if (width > height)
      scaly = (double) height / (double) width;
  else
      scalx  = (double) width / (double) height;

  int rdiff = color1.red () - color2.red ();
  int gdiff = color1.green () - color2.green ();
  int bdiff = color1.blue () - color2.blue ();

  unsigned int mx = width / 2;
  unsigned int my = height / 2;
  unsigned int dx, dy;

  for (double d = 1.0; d > 0.0; d -= delta) {
    r = color1.red () - qRound (rdiff * d);
    g = color1.green () - qRound (gdiff * d);
    b = color1.blue () - qRound (bdiff * d);
    col.setRgb (r, g, b);
    pen.setColor (col);
    p.setPen (pen);
    p.setBrush (col);
    dx = qRound (dd * d * scalx);
    dy = qRound (dd * d * scaly);
    p.drawRect (mx - dx, my - dy, dx * 2, dy * 2);
  }
}
