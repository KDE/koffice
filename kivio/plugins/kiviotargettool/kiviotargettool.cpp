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
#include "kiviotargettool.h"

#include <qcursor.h>

#include <kactionclasses.h>
#include <klocale.h>

#include "kivio_stencil.h"
#include "kivio_view.h"
#include "kivio_canvas.h"
#include "kivio_page.h"
#include "kivio_doc.h"

namespace Kivio {
  TargetTool::TargetTool(KivioView* parent) : MouseTool(parent, "Add Target Mouse Tool")
  {
    m_targetAction = new KRadioAction(i18n("Add Connector Target"), "add_target", actionCollection(), "addTargetTool");
    connect(m_targetAction, SIGNAL(toggled(bool)), this, SLOT(setActivated(bool)));
  }
  
  TargetTool::~TargetTool()
  {
  }
  
  bool TargetTool::processEvent( QEvent* e )
  {
    if(e->type() == QEvent::MouseButtonPress) {
      mousePress(static_cast<QMouseEvent*>(e));
      return true;
    } else if(e->type() == QEvent::MouseMove) {
      mouseMove(static_cast<QMouseEvent*>(e));
      return true;
    }
    
    return false;
  }
  
  void TargetTool::setActivated(bool a)
  {
    if(a) {
      m_targetAction->setChecked(true);
      emit activated(this);
    } else if(m_targetAction->isChecked()) {
      m_targetAction->setChecked(false);
    }
  }
  
  void TargetTool::applyToolAction(KivioStencil* stencil, const KoPoint& pos)
  {
    //TODO Add undo support!
    stencil->addConnectorTarget(pos);
    view()->doc()->updateView(view()->activePage());
  }

  void TargetTool::mousePress(QMouseEvent* e)
  {
    KoPoint p = view()->canvasWidget()->mapFromScreen(e->pos());
    int colType;
    KivioStencil* stencil = view()->canvasWidget()->activePage()->checkForStencil(&p, &colType, 0, false);
    
    if(stencil) {
      applyToolAction(stencil, p);
    }
  }

  void TargetTool::mouseMove(QMouseEvent* e)
  {
    KoPoint p = view()->canvasWidget()->mapFromScreen(e->pos());
    int colType;
    
    if(view()->canvasWidget()->activePage()->checkForStencil(&p, &colType, 0, false)) {
      view()->canvasWidget()->setCursor(Qt::CrossCursor);
    } else {
      view()->canvasWidget()->setCursor(Qt::ArrowCursor);
    }
  }
};
