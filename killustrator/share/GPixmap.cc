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

#include <GPixmap.h>

#include <qwmf.h>
#include <qdom.h>

#include <klocale.h>
#include <kdebug.h>

GPixmap::GPixmap () {
  pix = 0L;
}

GPixmap::GPixmap (const QString &filename) : url (filename) {
  if (url.isLocalFile ()) {
    pix = new QPixmap (url.path ());
    if (pix->isNull ()) {
      delete pix;
      pix = 0L;
      kdWarning() << "pixmap is null !!!" << endl;
    }
  }
  if (pix) {
    // use real pixmap dimension
    width = pix->width ();
    height = pix->height ();
  }
  calcBoundingBox ();
}

GPixmap::GPixmap (const QDomElement &element) : GObject (element.namedItem("gobject").toElement()) {

    url=element.attribute("src");
    if (url.isLocalFile ()) {
        pix = new QPixmap (url.path ());
        if (pix->isNull ()) {
            delete pix;
            pix = 0L;
        }
    }
    if (pix) {
        // use real pixmap dimension
        width = pix->width ();
        height = pix->height ();
    }
    calcBoundingBox ();
}

GPixmap::GPixmap (const GPixmap& obj) : GObject (obj) {
  url = obj.url;
  if (obj.pix)
    pix = new QPixmap (*obj.pix);
  width = obj.width;
  height = obj.height;
  calcBoundingBox ();
}

GPixmap::~GPixmap () {
  if (pix)
    delete pix;
}

QString GPixmap::typeName () const {
  return i18n("Pixmap object");
}

void GPixmap::draw (QPainter& p, bool /*withBasePoints*/, bool outline) {
  p.save ();
  p.setWorldMatrix (tmpMatrix, true);
  if (outline) {
    p.setPen (black);
    p.drawRect (0, 0, width, height);
    float w5 = qRound (width / 5.0);
    float h5 = qRound (height / 5.0);
    p.drawLine (0, w5, w5, 0);
    p.drawLine (width - w5, height, width, height - h5);
  }
  else {
    if (pix != 0L)
      p.drawPixmap (0, 0, *pix);
    else {
      p.setPen (gray);
      p.fillRect (0, 0, width, height, gray);
    }
  }
  p.restore ();
}

void GPixmap::calcBoundingBox () {
  calcUntransformedBoundingBox (Coord (0, 0), Coord (width, 0),
                                Coord (width, height), Coord (0, height));
}

GObject* GPixmap::copy () {
  return new GPixmap (*this);
}

GObject* GPixmap::clone (const QDomElement &element) {
  return new GPixmap (element);
}

QDomElement GPixmap::writeToXml (QDomDocument &document) {

    // FIXME (Werner): Let the user decide where to pit the image (intern/extern)
    QDomElement element=document.createElement("pixmap");
    element.setAttribute ("src", url.url ());
    element.appendChild(GObject::writeToXml(document));
    return element;
}

#include <GPixmap.moc>
