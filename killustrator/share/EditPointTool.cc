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

#include <iostream.h>
#include "EditPointTool.h"
#include "EditPointTool.moc"
#include "GDocument.h"
#include "Canvas.h"
#include "Coord.h"
#include "CommandHistory.h"
#include "EditPointCmd.h"
#include "InsertPointCmd.h"
#include "RemovePointCmd.h"
#include "SplitLineCmd.h"
#include "GPolyline.h"
#include "GBezier.h"
#include <qkeycode.h>
#include <qbitmap.h>
#include <kapp.h>
#include <klocale.h>
#include "version.h"

#define right_ptr_width 16
#define right_ptr_height 16
#define right_ptr_x_hot 12
#define right_ptr_y_hot 1
static unsigned char right_ptr_bits[] = {
   0x00, 0x00, 0x00, 0x10, 0x00, 0x18, 0x00, 0x1c, 0x00, 0x1e, 0x00, 0x1f,
   0x80, 0x1f, 0xc0, 0x1f, 0xe0, 0x1f, 0xf0, 0x1f, 0xf8, 0x1e, 0x1c, 0x1c,
   0x00, 0x1c, 0x00, 0x18, 0x00, 0x18, 0x00, 0x10};

#define right_ptrmsk_width 16
#define right_ptrmsk_height 16
static unsigned char right_ptrmsk_bits[] = {
   0x00, 0x30, 0x00, 0x38, 0x00, 0x3c, 0x00, 0x3e, 0x00, 0x3f, 0x80, 0x3f,
   0xc0, 0x3f, 0xe0, 0x3f, 0xf0, 0x3f, 0xf8, 0x3f, 0xfc, 0x3f, 0x7e, 0x3e,
   0x0e, 0x3c, 0x00, 0x38, 0x00, 0x38, 0x00, 0x30};

EditPointTool::EditPointTool (CommandHistory* history) : Tool (history) {
  obj = 0L;
  pointIdx = -1;
  mode = MovePoint;
  cursor = new QCursor (QBitmap (right_ptr_width,
				 right_ptr_height,
				 right_ptr_bits, true), 
			QBitmap (right_ptrmsk_width,
				 right_ptrmsk_height,
				 right_ptrmsk_bits, true),
			right_ptr_x_hot, right_ptr_y_hot);
}

EditPointTool::~EditPointTool () {
  delete cursor;
}

void EditPointTool::setMode (Mode m) {
  mode = m;
  switch (m) {
  case MovePoint:
    emit modeSelected (i18n ("Move Point"));
    break;
  case InsertPoint:
    emit modeSelected (i18n ("Insert Point"));
    break;
  case RemovePoint:
    emit modeSelected (i18n ("Remove Point"));
    break;
  case Split:
    emit modeSelected (i18n ("Split Line"));
    break;
  default:
    break;
  }
}

void EditPointTool::processEvent (QEvent* e, GDocument *doc, 
				  Canvas* canvas) {
  if (doc->selectionIsEmpty ())
    return;

  if (e->type () == 
#if QT_VERSION >= 199
      QEvent::MouseButtonPress
#else
      Event_MouseButtonPress
#endif
      ) {
    QMouseEvent *me = (QMouseEvent *) e;
    float xpos = me->x (), ypos = me->y ();
    //    canvas->snapPositionToGrid (xpos, ypos);

    obj = 0L;
    pointIdx = -1;
    // for performance reasons check if an object from the selection
    // has to be edited
    for (list<GObject*>::iterator it = doc->getSelection ().begin ();
	 it != doc->getSelection ().end (); it++) {
      GObject* o = *it;
      int idx = o->getNeighbourPoint (Coord (xpos, ypos));
      if (idx != -1) {
	obj = o;
	pointIdx = idx;
	startPos = Coord (xpos, ypos);
	lastPos = startPos;
	canvas->setCursor (*cursor);
	break;
      }
    }
    // if no currently selected object was found at the mouse position ...
    if (obj == 0L) {
      if ((obj = doc->findContainingObject (xpos, ypos)) != 0L) {
	// select and edit this object
	doc->unselectAllObjects ();
	doc->selectObject (obj);
	pointIdx = obj->getNeighbourPoint (Coord (xpos, ypos));
	startPos = Coord (xpos, ypos);
	lastPos = startPos;
	canvas->setCursor (*cursor);
      }
    }
  }
  else if (e->type () == 
#if QT_VERSION >= 199
	   QEvent::MouseMove
#else
	   Event_MouseMove
#endif
	   ) {
    if (mode == InsertPoint)
      return;

    QMouseEvent *me = (QMouseEvent *) e;
    float xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);

    if (obj == 0L) {
      bool isOver = false;
      int pidx;

      for (list<GObject*>::iterator it = doc->getSelection ().begin ();
	   it != doc->getSelection ().end (); it++) {
	GObject* o = *it;
	if ((pidx = o->getNeighbourPoint (Coord (xpos, ypos))) != -1) {
	  if (mode == RemovePoint && o->isA ("GBezier")) {
	    if (((GBezier *) o)->isEndPoint (pidx)) {
	      isOver = true;
	      break;
	    }
	  }
	  else
	    isOver = true;
	}
      }
      if (isOver)
	canvas->setCursor (*cursor);
      else
	canvas->setCursor (arrowCursor);
    }
    else if (pointIdx != -1) {
      float dx = xpos - lastPos.x ();
      float dy = ypos - lastPos.y ();
      if (dx != 0 || dy != 0) {
	obj->movePoint (pointIdx, dx, dy);
	lastPos = Coord (xpos, ypos);
      }
    }
  }
  else if (e->type () == 
#if QT_VERSION >= 199
	   QEvent::MouseButtonRelease
#else
	   Event_MouseButtonRelease
#endif
	   ) {
    if (obj == 0L) 
      return;

    QMouseEvent *me = (QMouseEvent *) e;
    float xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);
    if (mode == MovePoint) {
      if (pointIdx != -1) {
	float dx = xpos - lastPos.x ();
	float dy = ypos - lastPos.y ();
	if (dx != 0 || dy != 0) 
	  obj->movePoint (pointIdx, dx, dy);
	
	EditPointCmd *cmd = new EditPointCmd (doc, obj, pointIdx,
					      xpos - startPos.x (), 
					      ypos - startPos.y ());
	history->addCommand (cmd);
      }
    }
    else if (mode == InsertPoint && obj->inherits ("GPolyline")) {
      GPolyline* pline = (GPolyline *) obj;
      // compute the segment of intersection
      int idx = pline->containingSegment (xpos, ypos);
      if (idx != -1) {
	if (obj->isA ("GBezier"))
	  idx = (idx + 1) * 3;
	else
	  idx += 1;
	InsertPointCmd *cmd = new InsertPointCmd (doc, pline, idx,
						  xpos, ypos);
	history->addCommand (cmd, true);
      }
    }
    else if (mode == RemovePoint) {
      bool removable = true;
      if (pointIdx != -1 /*&& obj->inherits ("GPolyline")*/) {
	if (obj->isA ("GBezier"))
	  // we cannot remove control points of bezier curves
	  removable = ((GBezier *) obj)->isEndPoint (pointIdx);

	if (removable) {
	  GPolyline *pline = (GPolyline *) obj;
	  RemovePointCmd *cmd = new RemovePointCmd (doc, pline, pointIdx);
	  history->addCommand (cmd, true);
	}
      }
    }
    else if (mode == Split) {
      if (pointIdx != -1) {
	bool removable = true;

	if (obj->isA ("GBezier"))
	  // we cannot remove control points of bezier curves
	  removable = ((GBezier *) obj)->isEndPoint (pointIdx);

	if (removable) {
	  GPolyline *pline = (GPolyline *) obj;
	  SplitLineCmd *cmd = new SplitLineCmd (doc, pline, pointIdx);
	  history->addCommand (cmd, true);
	}
      }
    }

    canvas->setCursor (arrowCursor);

    obj = 0L;
  }
}

void EditPointTool::activate (GDocument* doc, Canvas* canvas) {
  mode = MovePoint;
  emit modeSelected (i18n ("Edit Point"));
  if (! doc->selectionIsEmpty ()) {
    doc->handle ().show (false);
    // redraw with highlighted points
    canvas->showBasePoints (true);
  }
}

void EditPointTool::deactivate (GDocument* doc, Canvas* canvas) {
  if (!doc->selectionIsEmpty ()) {
    doc->handle ().show (true);
    // redraw with unhighlighted points
    canvas->showBasePoints (false);
  }
}
