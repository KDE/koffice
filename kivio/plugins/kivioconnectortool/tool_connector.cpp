/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "tool_connector.h"

#include <qcursor.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <koPoint.h>
#include <kozoomhandler.h>
#include <kactionclasses.h>
#include <klocale.h>

#include "kivio_view.h"
#include "kivio_canvas.h"
#include "kivio_page.h"
#include "kivio_doc.h"
#include "kivio_factory.h"

#include "kivio_stencil_spawner_set.h"
#include "kivio_stencil_spawner.h"
#include "kivio_custom_drag_data.h"
#include "kivio_layer.h"
#include "kivio_point.h"
#include "kivio_stencil.h"
#include "straight_connector.h"
#include "kivio_pluginmanager.h"
#include "kivio_1d_stencil.h"
#include "kiviopolylineconnector.h"

ConnectorTool::ConnectorTool( KivioView* parent ) : Kivio::MouseTool(parent, "Connector Mouse Tool")
{
  m_connectorAction = new KToggleAction(i18n("Straight Connector"), "kivio_connector", 0, actionCollection(), "connector");
  connect(m_connectorAction, SIGNAL(toggled(bool)), this, SLOT(setActivated(bool)));
  m_connectorAction->setExclusiveGroup("ConnectorTool");

  m_polyLineAction = new KToggleAction(i18n("Polyline Connector"), "kivio_connector", 0,
    actionCollection(), "polyLineConnector");
  connect(m_polyLineAction, SIGNAL(toggled(bool)), this, SLOT(setActivated(bool)));
  m_polyLineAction->setExclusiveGroup("ConnectorTool");
  
  m_mode = stmNone;
  m_pDragData = 0;

  m_pConnectorCursor1 = new QCursor(BarIcon("kivio_connector_cursor1",KivioFactory::global()),2,2);
  m_pConnectorCursor2 = new QCursor(BarIcon("kivio_connector_cursor2",KivioFactory::global()),2,2);
}

ConnectorTool::~ConnectorTool()
{
  delete m_pConnectorCursor1;
  delete m_pConnectorCursor2;
  delete m_pDragData;
  m_pDragData = 0;
}


/**
 * Event delegation
 *
 * @param e The event to be identified and processed
 *
 */
bool ConnectorTool::processEvent( QEvent* e )
{
  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    mousePress( static_cast<QMouseEvent*>(e) );
    return true;
    break;

  case QEvent::MouseButtonRelease:
    mouseRelease( static_cast<QMouseEvent*>(e) );
    return true;
    break;

  case QEvent::MouseMove:
    mouseMove( static_cast<QMouseEvent*>(e) );
    return true;
    break;

  default:
    break;
  }

  return false;
}

void ConnectorTool::setActivated(bool a)
{
  if(a) {
    if(!m_polyLineAction->isChecked()) {
      m_connectorAction->setChecked(true);
      m_type = StraightConnector;
    } else {
      m_type = PolyLineConnector;
    }
    
    view()->canvasWidget()->setCursor(*m_pConnectorCursor1);
    m_mode = stmNone;
    m_pStencil = 0;
    m_pDragData = 0;
    emit activated(this);
  } else {
    m_pStencil = 0;
    delete m_pDragData;
    m_pDragData = 0;
    m_connectorAction->setChecked(false);
    m_polyLineAction->setChecked(false);
  }
}

void ConnectorTool::connector(QRect)
{
  if (!m_pStencil)
    return;

  delete m_pDragData;
  m_pDragData = 0;

  KivioDoc* doc = view()->doc();
  KivioPage* page = view()->activePage();

  if (m_pStencil->w() < 3.0 && m_pStencil->h() < 3.0) {
    page->unselectAllStencils();
    page->selectStencil(m_pStencil);
    page->deleteSelectedStencils();
    m_pStencil = 0;
    doc->updateView(page);
    return;
  }

  m_pStencil->searchForConnections(page, view()->zoomHandler()->unzoomItY(4));
  doc->updateView(page);
}

void ConnectorTool::mousePress( QMouseEvent *e )
{
  if(e->button() == LeftButton) {
    bool ok = true;
    if(!m_pStencil || (m_type == StraightConnector)) {
      ok = startRubberBanding(e);
    } else {
      if(m_pStencil) {
        Kivio::PolyLineConnector* connector = static_cast<Kivio::PolyLineConnector*>(m_pStencil);
        KivioCanvas* canvas = view()->canvasWidget();
        KivioPage* pPage = canvas->activePage();
        bool hit = false;
        KoPoint point = pPage->snapToTarget(canvas->mapFromScreen(e->pos()), 8.0, hit);
      
        if(!hit) {
          point = canvas->snapToGrid(startPoint);
        }
        
        connector->addPoint(point);
      }
    }
    
    if(ok) {
      m_mode = stmDrawRubber;
    } else {
      m_mode = stmNone;
    }
  } else if(e->button() == RightButton) {
    if(m_type == PolyLineConnector) {
      if(m_mode == stmDrawRubber) {
        endRubberBanding(e);
      }
      
      view()->canvasWidget()->setCursor(*m_pConnectorCursor1);
      m_mode = stmNone;
    }
  }
}


/**
 * Tests if we should start rubber banding (always returns true).
 */
bool ConnectorTool::startRubberBanding( QMouseEvent *e )
{
  KivioCanvas* canvas = view()->canvasWidget();
  KivioDoc* doc = view()->doc();
  KivioPage* pPage = canvas->activePage();
  
  if(m_type == StraightConnector) {
    KivioStencilSpawner* ss = doc->findInternalStencilSpawner("Dave Marotti - Straight Connector");
    
    if (!ss) {
      kdDebug(43000) << "ConnectorTool: Failed to find StencilSpawner!" << endl;
      return false;
    }
    
    // Create the stencil
    m_pStencil = static_cast<Kivio1DStencil*>(ss->newStencil());
  } else {
    // Create the stencil
    m_pStencil = static_cast<Kivio1DStencil*>(new Kivio::PolyLineConnector());
  }

  bool hit = false;
  startPoint = pPage->snapToTarget(canvas->mapFromScreen(e->pos()), 8.0, hit);

  if(!hit) {
    startPoint = canvas->snapToGrid(startPoint);
  }

  
  if(!m_pStencil) {
    return false;
  }
  
  m_pStencil->setTextFont(doc->defaultFont());

  // Unselect everything, add the stencil to the page, and select it
  pPage->unselectAllStencils();
  pPage->addStencil(m_pStencil);
  pPage->selectStencil(m_pStencil);

  if(m_type == StraightConnector) {
    KivioStraightConnector* connector = static_cast<KivioStraightConnector*>(m_pStencil);
    // Get drag info ready
    m_pDragData = new KivioCustomDragData();
    m_pDragData->page = pPage;
    m_pDragData->x = startPoint.x();
    m_pDragData->y = startPoint.y();
    m_pDragData->id = kctCustom + 2;
  
    connector->setStartPoint(startPoint.x() + 10.0f, startPoint.y() + 10.0f);
    connector->setEndPoint(startPoint.x(), startPoint.y());
    connector->customDrag(m_pDragData);
  } else {
    Kivio::PolyLineConnector* connector = static_cast<Kivio::PolyLineConnector*>(m_pStencil);
    connector->addPoint(startPoint);
    connector->addPoint(startPoint);
  }

  canvas->repaint();
  canvas->setCursor(*m_pConnectorCursor2);
  return true;
}

void ConnectorTool::mouseMove( QMouseEvent * e )
{
  switch( m_mode )
  {
    case stmDrawRubber:
      continueRubberBanding(e);
      break;

    default:
      break;
  }
}

void ConnectorTool::continueRubberBanding( QMouseEvent *e )
{
  KivioCanvas* canvas = view()->canvasWidget();
  KivioPage* pPage = view()->activePage();
  bool hit = false;
  KoPoint endPoint = pPage->snapToTarget(canvas->mapFromScreen(e->pos()), 8.0, hit);

  if(!hit) {
    endPoint = canvas->snapToGrid(endPoint);
  }

  if(m_type == StraightConnector) {
    KivioStraightConnector* connector = static_cast<KivioStraightConnector*>(m_pStencil);
    connector->setStartPoint(endPoint.x(), endPoint.y()); // !!
  
    m_pDragData->x = endPoint.x();
    m_pDragData->y = endPoint.y();
    m_pDragData->id = kctCustom + 1;
    connector->customDrag(m_pDragData);
  } else {
    Kivio::PolyLineConnector* connector = static_cast<Kivio::PolyLineConnector*>(m_pStencil);
    connector->moveLastPointTo(endPoint);
  }

  m_pStencil->updateGeometry();
  canvas->repaint();
}

void ConnectorTool::mouseRelease( QMouseEvent *e )
{
  if(m_type == StraightConnector) {
    switch( m_mode )
    {
      case stmDrawRubber:
        endRubberBanding(e);
        break;
    }
  
    view()->canvasWidget()->setCursor(*m_pConnectorCursor1);
    m_mode = stmNone;
  }
}

void ConnectorTool::endRubberBanding(QMouseEvent *)
{
  connector(view()->canvasWidget()->rect());
  m_pStencil = 0;
}

#include "tool_connector.moc"
