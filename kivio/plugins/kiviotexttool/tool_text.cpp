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
#include "tool_text.h"

#include <qcursor.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <koPoint.h>
#include <klocale.h>
#include <kactionclasses.h>

#include "kivio_view.h"
#include "kivio_canvas.h"
#include "kivio_page.h"
#include "kivio_doc.h"
#include "stencil_text_dlg.h"

#include "kivio_stencil_spawner_set.h"
#include "kivio_stencil_spawner.h"
#include "kivio_custom_drag_data.h"
#include "kivio_layer.h"
#include "kivio_point.h"
#include "kivio_stencil.h"
#include "kivio_factory.h"
#include "kivio_command.h"
#include "kivio_pluginmanager.h"


TextTool::TextTool( KivioView* parent ) : Kivio::MouseTool(parent, "Text Mouse Tool")
{
  m_textAction = new KToggleAction( i18n("Edit Stencil Text..."), "text", Key_F2, actionCollection(), "text" );
  connect(m_textAction, SIGNAL(toggled(bool)), this, SLOT(setActivated(bool)));

  m_mode = stmNone;

  QPixmap pix = BarIcon("kivio_text_cursor",KivioFactory::global());
  m_pTextCursor = new QCursor(pix,2,2);
}

TextTool::~TextTool()
{
  delete m_pTextCursor;
}


/**
 * Event delegation
 *
 * @param e The event to be identified and processed
 *
 */
void TextTool::processEvent( QEvent* e )
{
  switch (e->type())
  {
  case QEvent::MouseButtonPress:
    mousePress( (QMouseEvent*)e );
    break;

  case QEvent::MouseButtonRelease:
    mouseRelease( (QMouseEvent*)e );
    break;

  case QEvent::MouseMove:
    mouseMove( (QMouseEvent*)e );
    break;

  default:
    break;
  }
}

void TextTool::setActivated(bool a)
{
  if(a) {
    emit activated(this);
    m_textAction->setChecked(true);
    view()->canvasWidget()->setCursor(*m_pTextCursor);
    m_mode = stmNone;
  } else {
    m_textAction->setChecked(false);
  }
}

void TextTool::text(QRect r)
{
  // Calculate the start and end clicks in terms of page coordinates
  KoPoint startPoint = view()->canvasWidget()->mapFromScreen( QPoint( r.x(), r.y() ) );
  KoPoint releasePoint = view()->canvasWidget()->mapFromScreen( QPoint( r.x() + r.width(), r.y() + r.height() ) );

  // Calculate the x,y position of the textion box
  float x = startPoint.x() < releasePoint.x() ? startPoint.x() : releasePoint.x();
  float y = startPoint.y() < releasePoint.y() ? startPoint.y() : releasePoint.y();

  // Calculate the w/h of the textion box
  float w = releasePoint.x() - startPoint.x();

  if( w < 0.0 ) {
    w *= -1.0;
  }

  float h = releasePoint.y() - startPoint.y();

  if( h < 0.0 ) {
    h *= -1.0;
  }

  KivioDoc* doc = view()->doc();
  KivioPage* page = view()->activePage();

  KivioStencilSpawner* ss = doc->findInternalStencilSpawner("Dave Marotti - Text");

  if (!ss) {
    return;
  }

  KivioStencil* stencil = ss->newStencil();
  stencil->setType(kstText);
  stencil->setPosition(x,y);
  stencil->setDimensions(w,h);
  stencil->setText("");
  stencil->setTextFont(doc->defaultFont());
  page->unselectAllStencils();
  page->addStencil(stencil);
  page->selectStencil(stencil);

  doc->updateView(page);

  applyToolAction(page->selectedStencils());

  if (stencil->text().isEmpty()) {
    page->deleteSelectedStencils();
    doc->updateView(page);
  }
}

void TextTool::mousePress( QMouseEvent *e )
{
  if(e->button() == RightButton)
  {
    view()->pluginManager()->activateDefaultTool();
    return;
  }
  if( startRubberBanding( e ) )
  {
    m_mode = stmDrawRubber;
  }
}


/**
 * Tests if we should start rubber banding (always returns true).
 */
bool TextTool::startRubberBanding( QMouseEvent *e )
{
  view()->canvasWidget()->startRectDraw( e->pos(), KivioCanvas::Rubber );
  view()->canvasWidget()->repaint();

  return true;
}

void TextTool::mouseMove( QMouseEvent * e )
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

void TextTool::continueRubberBanding( QMouseEvent *e )
{
  view()->canvasWidget()->continueRectDraw( e->pos(), KivioCanvas::Rubber );
}

void TextTool::mouseRelease( QMouseEvent *e )
{
  m_releasePoint = e->pos();

  switch( m_mode )
  {
    case stmDrawRubber:
      endRubberBanding(e);
      break;
  }

  m_mode = stmNone;

  view()->canvasWidget()->repaint();
}

void TextTool::endRubberBanding(QMouseEvent */*e*/)
{
  // End the rubber-band drawing
  view()->canvasWidget()->endRectDraw();

  // We can't text if the start and end points are the same
  if( m_startPoint != m_releasePoint )
  {
    text(view()->canvasWidget()->rect());
  }
}

void TextTool::applyToolAction(QPtrList<KivioStencil>* stencils)
{
  KivioDoc* doc = view()->doc();
  KivioPage *page = view()->activePage();

  if( stencils->isEmpty() )
    return;

  KivioStencil* stencil = stencils->first();
  KivioStencilTextDlg d(0, stencil->text());

  if( !d.exec() )
    return;

  QString text = d.text();
  KMacroCommand *macro = new KMacroCommand( i18n("Change Stencil Text"));
  bool createMacro=false;
  
  while( stencil )
  {
    if ( stencil->text()!= text )
    {
      KivioChangeStencilTextCommand *cmd = new KivioChangeStencilTextCommand( i18n("Change Stencil Text"), stencil, stencil->text(), text, page);
      macro->addCommand( cmd);
      stencil->setText( text );
      createMacro=true;
    }
    stencil = stencils->next();
  }
  
  if ( createMacro )
    doc->addCommand( macro);
  else
    delete macro;
  
  doc->updateView(page);
}

#include "tool_text.moc"
