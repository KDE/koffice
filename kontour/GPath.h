/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998-1999 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2001-2002 Igor Janssen (rm@linux.ru.net)

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

#ifndef __GPath_h__
#define __GPath_h__

#include "GObject.h"

#include <qptrlist.h>
#include <koPoint.h>

/**
 * Abstract class for path segment.
 *
 */
 
class GSegment
{
public:
  GSegment();
  GSegment(const QDomElement &element);
  virtual ~GSegment();

  virtual const char type() const = 0;

  const KoPoint &point(int i) const;
  void point(int i, const KoPoint &c);
  void movePoint(int idx, double dx, double dy, bool ctrlPressed = false);

  virtual QDomElement writeToXml(QDomDocument &document) = 0;

  virtual double length() const = 0;

protected:
  QMemArray<KoPoint> points;
};

/**
 * Move segment.
 *
 */

class GMove : public GSegment
{
public:
  GMove();
  GMove(const QDomElement &element);

  const char type() const;

  QDomElement writeToXml(QDomDocument &document);

  double length() const;
};

/**
 * Line segment.
 *
 */

class GLine : public GSegment
{
public:
  GLine();
  GLine(const QDomElement &element);

  const char type() const;

  QDomElement writeToXml(QDomDocument &document);

  double length() const;
};


/**
 * Cubic Bezier segment.
 *
 */

class GCubicBezier : public GSegment
{
public:
  GCubicBezier();
  GCubicBezier(const QDomElement &element);

  const char type() const;

  QDomElement writeToXml(QDomDocument &document);

  double length() const;
};


class GPath : public GObject
{
  Q_OBJECT
public:
  GPath(bool aClosed = false);
  GPath(const QDomElement &element);
  GPath(const GPath &obj);

  virtual GObject *copy() const;

  bool closed() const {return mClosed; }
  void closed(bool aClosed);

  /* Construct path */
  void beginTo(const double x, const double y);
  void moveTo(const double x, const double y);
  void lineTo(const double x, const double y);
  void curveTo(const double x, const double y, const double x1, const double y1, const double x2, const double y2);
  void arcTo(const double x1, const double y1, const double x2, const double y2, const double r);

  QString typeName() const;
  QDomElement writeToXml(QDomDocument &document);
  void draw(KoPainter *p, const QWMatrix &m, bool withBasePoints = false, bool outline = false, bool withEditMarks = true);

  int getNeighbourPoint(const KoPoint &point);
  void movePoint(int idx, double dx, double dy, bool ctrlPressed = false);
  void removePoint(int idx);
  bool contains(const KoPoint &p);

  void calcBoundingBox();
  GPath *convertToPath() const;
  bool isConvertible() const;

private:
  QPtrList<GSegment> segments;
  bool mClosed;
};

#endif
