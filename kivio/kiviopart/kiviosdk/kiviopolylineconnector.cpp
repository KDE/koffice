/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>,

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

#include "kiviopolylineconnector.h"

#include <kdebug.h>

#include <kozoomhandler.h>

#include "kivio_line_style.h"
#include "kivio_painter.h"
#include "kivio_intra_stencil_data.h"
#include "kivio_connector_point.h"
#include "kivio_custom_drag_data.h"
#include "tkmath.h"


namespace Kivio {
  PolyLineConnector::PolyLineConnector()
    : Kivio1DStencil()
  {
    m_startArrow = new KivioArrowHead();
    m_endArrow = new KivioArrowHead();
  
    m_needsWidth = false;
    m_needsText = false; // FIXME add text support
  
    m_pCanProtect->clearBit(kpAspect);
    m_pCanProtect->clearBit(kpWidth);
    m_pCanProtect->clearBit(kpHeight);
    m_pCanProtect->clearBit(kpX);
    m_pCanProtect->clearBit(kpY);
    
    // This is a stencil of type connector
    setType(kstConnector);
  }
  
  PolyLineConnector::~PolyLineConnector()
  {
    delete m_startArrow;
    delete m_endArrow;
  }

  KivioCollisionType PolyLineConnector::checkForCollision(KoPoint* p, double threshold)
  {
    unsigned int i = 0;
    KoPoint point;
    double px = p->x();
    double py = p->y();
    
    while(i < m_points.count()) {
      point = m_points[i];
      
      if(px >= point.x() - threshold && px <= point.x() + threshold &&
        py >= point.y() - threshold && py <= point.y() + threshold)
      {
        return static_cast<KivioCollisionType>(i + kctCustom + 1);
      }

      if(i < (m_points.count() - 1)) {
        if(collisionLine(point.x(), point.y(),
          m_points[i + 1].x(), m_points[i + 1].y(), px, py, threshold))
        {
          return kctBody;
        }
      }
      
      i++;
    }
        
    return kctNone;
  }

  void PolyLineConnector::paint(KivioIntraStencilData* data)
  {
    KoZoomHandler* zoom = data->zoomHandler;
    KivioPainter* painter = data->painter;
    
    painter->setLineStyle(m_pLineStyle);
    painter->setLineWidth(zoom->zoomItY(m_pLineStyle->width()));
    
    QPointArray pa(m_points.count());
    QValueList<KoPoint>::iterator it;
    int i = 0;
    
    for(it = m_points.begin(); it != m_points.end(); it++) {
      pa.setPoint(i, zoom->zoomPoint(*it));
      i++;
    }
    
    painter->drawPolyline(pa);
  }
  
  void PolyLineConnector::paintOutline(KivioIntraStencilData* data)
  {
    paint(data);
  }

  void PolyLineConnector::paintSelectionHandles( KivioIntraStencilData* data )
  {
    KivioPainter* painter = data->painter;
    KoZoomHandler* zoomHandler = data->zoomHandler;
    QValueList<KoPoint>::Iterator it;
    int x, y, flag;
    x = y = flag = 0;
    
    for(it = m_points.begin(); it != m_points.end(); it++) {
      x = zoomHandler->zoomItX((*it).x());
      y = zoomHandler->zoomItY((*it).y());    
      
      painter->drawHandle(x, y, flag);
    }
  }
    
  void PolyLineConnector::addPoint(const KoPoint& p)
  {
    if(m_points.count() == 0) {
      m_pStart->setPosition(p.x(), p.y(), false);
      m_pStart->disconnect();
    } else {
      m_pEnd->setPosition(p.x(), p.y(), false);
      m_pEnd->disconnect();
    }
    
    m_points.append(p);
  }
  
  void PolyLineConnector::movePoint(int index, double xOffset, double yOffset)
  {
    KoPoint p(xOffset, yOffset);
    m_points[index] += p;
    
    if(index == (m_points.count() - 1)) {
      m_pEnd->setPosition(m_points[index].x(), m_points[index].y(), false);
      m_pEnd->disconnect();
    } else if(index == 0) {
      m_pStart->setPosition(m_points[index].x(), m_points[index].y(), false);
      m_pStart->disconnect();
    }
  }

  void PolyLineConnector::movePointTo(int index, const KoPoint& p)
  {
    m_points[index] = p;
    
    if(index == (m_points.count() - 1)) {
      m_pEnd->setPosition(p.x(), p.y(), false);
      m_pEnd->disconnect();
    } else if(index == 0) {
      m_pStart->setPosition(p.x(), p.y(), false);
      m_pStart->disconnect();
    }
  }
  
  void PolyLineConnector::moveLastPointTo(const KoPoint& p)
  {
    movePointTo(m_points.count() - 1, p);
  }

  void PolyLineConnector::customDrag(KivioCustomDragData* data)
  {
    int index = data->id - kctCustom + 1;
    
    if((index < 0) || index >= m_points.count()) {
      kdDebug() << "PolyLineConnector::customDrag: Index out of range! Index = " << index << endl;
    }
    
    movePointTo(index, KoPoint(data->x, data->y));
  }

  void PolyLineConnector::move(double xOffset, double yOffset)
  {
    for(int i = 0; i < m_points.count(); i++) {
      movePoint(i, xOffset, yOffset);
    }
  }

  double PolyLineConnector::x()
  {
    if(m_points.count() == 0) {
      return 0;
    }
    
    return m_points.first().x();
  }
  
  void PolyLineConnector::setX(double newX)
  {
    double dx = newX - x();
    move(dx, 0);
  }
  
  double PolyLineConnector::y()
  {
    if(m_points.count() == 0) {
      return 0;
    }
  
    return m_points.first().y();
  }
  
  void PolyLineConnector::setY(double newY)
  {
    double dy = newY - y();
    move(0, dy);
  }
}
