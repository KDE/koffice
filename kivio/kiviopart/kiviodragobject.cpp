/* This file is part of the KDE project
   Copyright (C) 2003 Peter Simonsson <psn@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kiviodragobject.h"

#include <qcstring.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qpixmap.h>
#include <qimage.h>

#include <kdebug.h>

#include <kozoomhandler.h>

#include "kivio_layer.h"
#include "kivio_page.h"
#include "kivio_screen_painter.h"
#include "kivio_intra_stencil_data.h"
#include "kivio_doc.h"
#include "kivio_stencil_spawner.h"

KivioDragObject::KivioDragObject(QWidget* dragSource, const char* name)
  : QDragObject(dragSource, name)
{
  m_decodeMimeList.append("application/vnd.kde.kivio");
  m_decodeMimeList.append("text/plain");
  m_encodeMimeList[0] = "application/vnd.kde.kivio";
  m_encodeMimeList[1] = "text/xml";
  m_encodeMimeList[2] = "text/plain";
  m_stencilList.setAutoDelete(true);
}

const char* KivioDragObject::format(int i) const
{
  if(i < NumEncodeFormats) {
    return m_encodeMimeList[i];
  } else {
    QImageDrag id;
    id.setImage(QImage()); // We need the format list!!!
    return id.format(i - NumEncodeFormats);
  }

  return 0;
}

QByteArray KivioDragObject::encodedData(const char* mimetype) const
{
  if((m_encodeMimeList[0] == mimetype) ||
    (m_encodeMimeList[1] == mimetype) ||
    (m_encodeMimeList[2] == mimetype))
  {
    return kivioEncoded();
  } else if(qstrnicmp(mimetype, "image/", 6) == 0) {
    return imageEncoded(mimetype);
  }

  return QByteArray();
}

bool KivioDragObject::canDecode(QMimeSource* e)
{
  for(QValueList<QCString>::Iterator it = m_decodeMimeList.begin(); it != m_decodeMimeList.end(); ++it) {
    if(e->provides(*it)) {
      return true;
    }
  }

  return false;
}

bool KivioDragObject::decode(QMimeSource* e, QPtrList<KivioStencil>& sl, KivioPage* page)
{
  bool ok = false;

  if(e->provides(m_decodeMimeList[0])) {
    QDomDocument doc("KivioSelection");
    QByteArray data = e->encodedData(m_decodeMimeList[0]);
    doc.setContent( QCString( data, data.size()+1 ) );
    KivioLayer l(page);
    ok = l.loadXML(doc.documentElement());
    KivioStencil* stencil = l.stencilList()->first();
    sl.clear();

    while(stencil) {
      sl.append(stencil->duplicate());
      stencil = l.stencilList()->next();
    }
  } else if(e->provides(m_decodeMimeList[1])) {
    QString str;
    ok = QTextDrag::decode(e, str);
    KivioStencilSpawner* ss = page->doc()->findInternalStencilSpawner("Dave Marotti - Text");
    KivioStencil* stencil = ss->newStencil();
    stencil->setPosition(0, 0);
    stencil->setDimensions(100, 100);
    stencil->setText(str);
    stencil->setTextFont(page->doc()->defaultFont());
    sl.clear();
    sl.append(stencil);
  }

  return ok;
}

void KivioDragObject::setStencilList(QPtrList<KivioStencil> l)
{
  KivioStencil* stencil = l.first();
  m_stencilList.clear();

  while(stencil) {
    m_stencilList.append(stencil->duplicate());
    stencil = l.next();
  }
}

void KivioDragObject::setStencilRect(KivioRect r)
{
  m_stencilRect = r;
}

QByteArray KivioDragObject::kivioEncoded() const
{
  if(m_stencilList.count() <= 0)
    return QByteArray();

  QDomDocument doc("KivioSelection");
  QDomElement elem = doc.createElement( "KivioSelection" );
  doc.appendChild(elem);
  KivioStencil *stencil = 0;
  QPtrListIterator<KivioStencil> it(m_stencilList);

  while((stencil = it.current()) != 0) {
    ++it;
    elem.appendChild(stencil->saveXML(doc));
  }

  return doc.toCString();
}

QByteArray KivioDragObject::imageEncoded(const char* mimetype) const
{
  KoZoomHandler zoomHandler;
  zoomHandler.setZoomAndResolution(100, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY());
  QPixmap buffer(zoomHandler.zoomItX(m_stencilRect.w()), zoomHandler.zoomItY(m_stencilRect.h()));
  buffer.fill(Qt::white);
  KivioScreenPainter p;
  p.start( &buffer );
  p.setTranslation(-zoomHandler.zoomItX(m_stencilRect.x()), -zoomHandler.zoomItY(m_stencilRect.y()));
  KivioIntraStencilData data;
  data.painter = &p;
  data.zoomHandler = &zoomHandler;
  data.printing = true;
  KivioStencil *stencil = 0;
  QPtrListIterator<KivioStencil> it(m_stencilList);

  while((stencil = it.current()) != 0) {
    ++it;
    stencil->paint(&data);
  }

  p.stop();

  QImageDrag id;
  id.setImage(buffer.convertToImage());
  return id.encodedData(mimetype);
}

#include "kiviodragobject.moc"
