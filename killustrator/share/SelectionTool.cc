/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998-99 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

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
#include <stdlib.h>
#include <math.h>
#include "SelectionTool.h"
#include "SelectionTool.moc"
#include "GDocument.h"
#include "Canvas.h"
#include "Coord.h"
#include <qkeycode.h>
#include "TranslateCmd.h"
#include "ScaleCmd.h"
#include "RotateCmd.h"
#include "ShearCmd.h"
#include "CommandHistory.h"
#include "units.h"
#include "PStateManager.h"
#include <kapp.h>
#include <klocale.h>
#include "version.h"

#include <list>
#include <algorithm>

struct is_a : public binary_function<GObject*, const char*, bool> {
  bool operator () (GObject* obj, const char* tname) {
    return obj->isA (tname);
  }
};

struct finalize_obj {
  void operator () (GObject* obj) {
    obj->setWorkInProgress (false);
  }
};

SelectionTool::SelectionTool (CommandHistory *history) : Tool (history) {
  state = S_Init;
}

void SelectionTool::processEvent (QEvent* e, GDocument *doc, Canvas* canvas) {
#if QT_VERSION >= 199
  if (e->type () == QEvent::MouseButtonPress)
    processButtonPressEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == QEvent::MouseMove) 
    processMouseMoveEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == QEvent::MouseButtonRelease)
    processButtonReleaseEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == QEvent::KeyPress)
    processKeyPressEvent ((QKeyEvent *) e, doc, canvas);
#else
  if (e->type () == Event_MouseButtonPress)
    processButtonPressEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == Event_MouseMove) 
    processMouseMoveEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == Event_MouseButtonRelease)
    processButtonReleaseEvent ((QMouseEvent *) e, doc, canvas);
  else if (e->type () == Event_KeyPress)
    processKeyPressEvent ((QKeyEvent *) e, doc, canvas);
#endif
}

void SelectionTool::processButtonReleaseEvent (QMouseEvent *me,
					       GDocument *doc, 
					       Canvas* canvas) {
  canvas->setCursor (arrowCursor);
  ctype = C_Arrow;
  Handle::Mode mode = Handle::HMode_Default;

  doc->handle ().show (true);
  float xpos = me->x (), ypos = me->y ();

  /**********
   * S_Rubberband
   */
  if (state == S_Rubberband) {
    QList<GObject> olist;
    Rect selRect (selPoint[0], selPoint[1]);
    if (doc->findObjectsContainedIn (selRect.normalize (), olist)) {
      QListIterator<GObject> it (olist);
      for (; it.current (); ++it) 
	doc->selectObject (it.current ());
      state = S_Pick;
    }
    else {
      // no object found - repaint canvas to remove the rubberband
      canvas->repaint ();
      state = S_Init;
    }
  }
  /**********
   * S_Pick
   */
  else if (state == S_Pick) {
    // ?
  }
  /**********
   * S_Scale, S_Translate
   */
  else if (state == S_Scale) {
    state = S_Pick;
    //    canvas->snapPositionToGrid (xpos, ypos);
    float xoff = xpos - firstpos.x ();
    float yoff = ypos - firstpos.y ();
    if (me->state () & ControlButton) {
      if (fabs (xoff) > fabs (yoff)) {
	yoff = xoff;
	if ((oldmask & (Handle::HPos_Left | Handle::HPos_Bottom)) ||
	    (oldmask & (Handle::HPos_Right | Handle::HPos_Top)))
	  yoff = -yoff;
      }
      else {
	xoff = yoff;
	if ((oldmask & (Handle::HPos_Left | Handle::HPos_Bottom)) ||
	    (oldmask & (Handle::HPos_Right | Handle::HPos_Top)))
	  xoff = -xoff;
      }
    }
    scale (doc, canvas, oldmask, xoff, yoff, true);
  }
  else if (state == S_Translate) {
    state = S_Pick;
    if (me->state () & ControlButton) {
      if (fabs (xpos - firstpos.x ()) > fabs (ypos - firstpos.y ()))
	ypos = firstpos.y ();
      else
	xpos = firstpos.x ();
    }
    translate (doc, canvas, xpos - firstpos.x (), ypos - firstpos.y (), 
	       true, true);
  }
  /**********
   * S_Intermediate1
   */
  else if (state == S_Intermediate1) {
    state = S_RotateSelect;
    mode = Handle::HMode_Rotate;
  }
  /**********
   * S_Rotate
   */
  else if (state == S_Rotate) {
    state = S_RotateSelect;
    mode = Handle::HMode_Rotate;
    canvas->snapPositionToGrid (xpos, ypos);
    rotate (doc, xpos - firstpos.x (), ypos - firstpos.y (), 
	    me->x (), me->y (), true);
  }
  else if (state == S_Shear) {
    state = S_RotateSelect;
    mode = Handle::HMode_Rotate;
    canvas->snapPositionToGrid (xpos, ypos);
    shear (doc, oldmask, xpos - firstpos.x (), ypos - firstpos.y (),
	   true);
  }
  /**********
   * S_MoveRotCenter
   */
  else if (state == S_MoveRotCenter) {
    state = S_RotateSelect;
    rotCenter = doc->handle().rotCenter ();
    return;
  }
  /**********
   * S_Intermediate2
   */
  else if (state == S_Intermediate2) {
    state = S_Pick;
    //    doc->handle ().setMode (Handle::HMode_Default);
  }
  doc->handle ().setMode (mode, true);
  if (doc->selectionIsEmpty ()) {
    emit modeSelected (i18n ("Selection Mode"));
  }
  else {
    Rect box = doc->boundingBoxForSelection ();
    MeasurementUnit unit = 
      PStateManager::instance ()->defaultMeasurementUnit ();
    const char *u = unitToString (unit);
    float x, y, w, h;
    x = cvtPtToUnit (unit, box.x ());
    y = cvtPtToUnit (unit, box.y ());
    w = cvtPtToUnit (unit, box.width ());
    h = cvtPtToUnit (unit, box.height ());
    if (doc->selectionCount () > 1) {
      sprintf (msgbuf, "%s [%.3f %s, %.3f %s, %.3f %s, %.3f %s]",
	       I18N ("Multiple Selection"), x, u, y, u, w, u, h, u);
    }
    else {
      GObject *sobj = doc->getSelection ().front ();
      sprintf (msgbuf, "%s [%.3f %s, %.3f %s, %.3f %s, %.3f %s]",
	       I18N (sobj->typeName ()), x, u, y, u, w, u, h, u);
    }
    emit modeSelected (msgbuf);
  }
  origbox = doc->boundingBoxForSelection ();
}

void SelectionTool::processMouseMoveEvent (QMouseEvent *me, GDocument *doc, 
					   Canvas* canvas) {
  int hmask;

  if (state == S_Inactive)
    return;
  /**********
   * S_Rubberband
   */
  else if (state == S_Rubberband) {
    selPoint[1].x (me->x ());
    selPoint[1].y (me->y ());
    canvas->repaint ();
    QPainter painter;
    painter.save ();
    QPen pen (blue, 1, DotLine);
    painter.begin (canvas);
    painter.setPen (pen);
    float sfactor = canvas->scaleFactor ();
    painter.scale (sfactor, sfactor);
    Rect selRect (selPoint[0], selPoint[1]);
    painter.drawRect ((int) selRect.x (), (int) selRect.y (),
		      (int) selRect.width (), (int) selRect.height ());
    painter.restore ();
    painter.end ();
    return;
  }

  if (! doc->selectionIsEmpty ()) {
    float xpos = me->x (), ypos = me->y ();

    /**********
     * S_Pick
     */
    if (state == S_Pick) {
      hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
      if (hmask && hmask != Handle::HPos_Center) {
	if (ctype != C_Size) {
	  ctype = C_Size;
	  canvas->setCursor (crossCursor);
	}
      }
      else if (ctype != C_Arrow) {
	ctype = C_Arrow;
	canvas->setCursor (arrowCursor);
      }

      if (me->state () & LeftButton)
	state = S_Translate;
    }
    /**********
     * S_Intermediate1
     */
    else if (state == S_Intermediate1) {
      if (me->state () & LeftButton) {
	//	hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
	if (ctype == C_Size)
	  state = S_Scale;
	else
	  state = S_Translate;
      }
    }
    /**********
     * S_MoveRotCenter
     */
    else if (state == S_MoveRotCenter) {
      doc->handle ().setRotCenter (Coord (me->x (), me->y ()));
      return;
    }
    /**********
     * S_Rotate
     */
    else if (state == S_RotateSelect || state == S_Rotate) {
      hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
      if (hmask) {
	if (ctype != C_Size) {
	  ctype = C_Size;
	  canvas->setCursor (crossCursor);
	}
      }
      else if (ctype != C_Arrow) {
	ctype = C_Arrow;
	canvas->setCursor (arrowCursor);
      }
	
    }
    if (me->state () & LeftButton) {
      //      canvas->snapPositionToGrid (xpos, ypos);
      float xoff = xpos - firstpos.x ();
      float yoff = ypos - firstpos.y ();

      doc->handle ().show (false);

      switch (state) {
      case S_Scale: 
        {
	  if (ctype != C_Size) {
	    ctype = C_Size;
	    canvas->setCursor (crossCursor);
	  }
	  if (me->state () & ControlButton) {
	    if (fabs (xoff) > fabs (yoff)) {
	      yoff = xoff;
	      if ((oldmask & (Handle::HPos_Left | Handle::HPos_Bottom)) ||
		  (oldmask & (Handle::HPos_Right | Handle::HPos_Top)))
		yoff = -yoff;
	    }
	    else {
	      xoff = yoff;
	      if ((oldmask & (Handle::HPos_Left | Handle::HPos_Bottom)) ||
		  (oldmask & (Handle::HPos_Right | Handle::HPos_Top)))
		xoff = -xoff;
	    }
	  }
	  scale (doc, canvas, oldmask, xoff, yoff);
	  break;
        }
      case S_Translate:
	if (ctype != C_Move) {
	  ctype = C_Move;
	  canvas->setCursor (sizeAllCursor);
	}
	if (me->state () & ControlButton) {
	  if (fabs (xoff) > fabs (yoff))
	    yoff = 0;
	  else
	    xoff = 0;
	}
	translate (doc, canvas, xoff, yoff, true);
	break;
      case S_Rotate:
	rotate (doc, xoff, yoff, me->x (), me->y ());
	break;
      case S_Shear:
	shear (doc, oldmask, xoff, yoff);
	break;
      default:
	break;
      }
    }
  }
}

void SelectionTool::processButtonPressEvent (QMouseEvent *me, GDocument *doc, 
					     Canvas* /*canvas*/) {
  int hmask;
  GObject *obj = 0L;

  float xpos = me->x (), ypos = me->y ();
  //  canvas->snapPositionToGrid (xpos, ypos);

  firstpos.x (xpos);
  firstpos.y (ypos);
  
  hmask = doc->handle ().contains (Coord (me->x (), me->y ()));
  bool shiftFlag = me->state () & ShiftButton;

  if (state == S_Inactive)
    state = S_Init;

  /************
   * S_Init
   */
  if (state == S_Init) {
    obj = doc->findContainingObject (me->x (), me->y ());
    if (obj) {
      // an object will be selected
      state = S_Pick;
      if (!shiftFlag)
	doc->unselectAllObjects ();
      // add the object to the selection
      doc->selectObject (obj);
      origbox = doc->boundingBoxForSelection ();
    }
    else {
      // no object
      state = S_Rubberband;
      doc->unselectAllObjects ();
      selPoint[0].x(me->x ()); selPoint[0].y(me->y ());
      selPoint[1].x(me->x ()); selPoint[1].y(me->y ());
    }
  }
  /************
   * S_Pick
   */
  else if (state == S_Pick) {
    origbox = doc->boundingBoxForSelection ();
    if (hmask) {
      state = S_Intermediate1;
      oldmask = hmask;
    }
    else {
      obj = doc->findContainingObject (me->x (), me->y ());
      if (obj) { 
	if (obj->isSelected ()) {

	  //
	  // a ugly workaround, because cliparts cannot be rotated (WHY NOT ?)
	  //
	  if (doc->selectionCount () == 1) {
	    GObject* selObj = doc->getSelection ().front ();
	    if (selObj->isA ("GClipart")) {
	      // the selected object is a clipart, 
	      // so don't show rotation handles
	      return;
	    }
	    else if (selObj->isA ("GPart")) {
	      cout << "activate part !!!" << endl;
	      state = S_Inactive;
	      emit partSelected (selObj);
	      return;
	    }
	  }

	  // the object is already selected
	  if (shiftFlag)
	    // remove it from the selection
	    doc->unselectObject (obj);
	  else
	    state = S_Intermediate1;
	}
	else {
	  if (!shiftFlag)
	    doc->unselectAllObjects ();
	  // add the object to the selection
	  doc->selectObject (obj);
	}
      }
      else {
	// nothing selected
	// unselect all
	doc->unselectAllObjects ();
	
	// and switch to rubberband mode
	state = S_Rubberband;
	selPoint[0].x(me->x ()); selPoint[0].y(me->y ());
	selPoint[1].x(me->x ()); selPoint[1].y(me->y ());
      }
    }
  }
  /************
   * S_RotateSelect
   */
  else if (state == S_RotateSelect) {
    if (hmask) {
      origbox = doc->boundingBoxForSelection ();
      oldmask = hmask;
      if (hmask == (Handle::HPos_Top | Handle::HPos_Left) ||
	  hmask == (Handle::HPos_Bottom | Handle::HPos_Left) ||
	  hmask == (Handle::HPos_Top | Handle::HPos_Right) ||
	  hmask == (Handle::HPos_Bottom | Handle::HPos_Right)) {
	state = S_Rotate;
	// rotCenter = doc->boundingBoxForSelection ().center ();
	rotCenter = doc->handle().rotCenter ();
      }
      else if (hmask == Handle::HPos_Center) {
	state = S_MoveRotCenter;
      }
      else {
	state = S_Shear;
	rotCenter = doc->handle().rotCenter ();
      }
    }
    else
      state = S_Intermediate2;
  }
}

void SelectionTool::processKeyPressEvent (QKeyEvent *ke, GDocument *doc, 
					     Canvas* canvas) {
  if (doc->selectionIsEmpty ())
    return;

  if (ke->key () == Key_Escape) {
      // clear selection
      doc->unselectAllObjects ();
      return;
  }

  float big_step = PStateManager::instance ()->bigStepSize ();
  float small_step = PStateManager::instance ()->smallStepSize ();
  float dx = 0, dy = 0;
  bool shift = ke->state () & ShiftButton;

  switch (ke->key ()) {
  case Key_Left:
    dx = (shift ? -small_step : -big_step);
    break;
  case Key_Right:
    dx = (shift ? small_step : big_step);
    break;
  case Key_Up:
    dy = (shift ? -small_step : -big_step);
    break;
  case Key_Down:
    dy = (shift ? small_step : big_step);
    break;
  default:
    break;
  }
  if (dx != 0 || dy != 0) 
    translate (doc, canvas, dx, dy, false, true);
}

void SelectionTool::translate (GDocument* doc, Canvas* canvas,
			       float dx, float dy, bool snap,
			       bool permanent) {
  if (snap) {
    const Rect& obox = origbox; 
    Rect newbox = canvas->snapTranslatedBoxToGrid (obox.translate (dx, dy));
    if (newbox != obox) {
      dx = newbox.x () - obox.x ();
      dy = newbox.y () - obox.y ();
    }
  }
  if (dx == 0 && dy == 0)  return;

  if (permanent) {
    for_each (doc->getSelection ().begin (), doc->getSelection ().end (), 
	      finalize_obj ());
    TranslateCmd *cmd = new TranslateCmd (doc, dx, dy);
    history->addCommand (cmd, true);
  }
  else {
    list<GObject*>::iterator it;
    QWMatrix m;
    m.translate (dx, dy);
    for (it = doc->getSelection ().begin (); 
	 it != doc->getSelection ().end (); it++) {
      (*it)->setWorkInProgress (true);
      (*it)->initTmpMatrix ();
      (*it)->ttransform (m, true);
    }
  }

  MeasurementUnit unit = 
    PStateManager::instance ()->defaultMeasurementUnit ();
  const char *u = unitToString (unit);
  float xval, yval;
  xval = cvtPtToUnit (unit, dx);
  yval = cvtPtToUnit (unit, dy);

  sprintf (msgbuf, "%s [%.3f %s, %.3f %s]", 
	   I18N ("Translate"), xval, u, yval, u);
  emit modeSelected (msgbuf);
}

void SelectionTool::rotate (GDocument* doc, float , float ,
			    float xp, float yp, bool permanent) {
  //  float adx = fabs (dx);
  //  float ady = fabs (dy);
  float angle = 0;

  //  Rect& r = origbox;

  /*
  if (adx > ady) {
    angle = adx / r.width () * 180.0;
    if ((yp > rotCenter.y () && dx > 0) ||
	(yp < rotCenter.y () && dx < 0))
      angle = -angle;
  }
  else if (adx < ady) {
    angle = ady / r.height () * 180.0;
    if ((xp > rotCenter.x () && dy < 0) ||
	(xp < rotCenter.x () && dy > 0))
      angle = -angle;
  }
  */
  // proposed by Stefan Eickeler <eickeler@fb9-ti.uni-duisburg.de>
  angle=atan2 ((float)rotCenter.y()-yp,(float)rotCenter.x()-xp);
  angle-=atan2 ((float)(rotCenter.y()-firstpos.y()),
		(float)(rotCenter.x()-firstpos.x()));
  angle*=180.0/3.141;
  if (angle<180.0) angle+=360.0;
  if (angle>180.0) angle-=360.0;

  if (permanent) {
    for_each (doc->getSelection ().begin (), doc->getSelection ().end (), 
	      finalize_obj ());
    RotateCmd *cmd = new RotateCmd (doc, rotCenter, angle);
    history->addCommand (cmd, true);
  }
  else {
    QWMatrix m1, m2, m3;
    m1.translate (-rotCenter.x (), -rotCenter.y ());
    m2.rotate (angle);
    m3.translate (rotCenter.x (), rotCenter.y ());

    for (list<GObject*>::iterator it = doc->getSelection ().begin (); 
	 it != doc->getSelection ().end (); it++) {
      (*it)->setWorkInProgress (true);
      (*it)->initTmpMatrix ();
      (*it)->ttransform (m1);
      (*it)->ttransform (m2);
      (*it)->ttransform (m3, true);
    }
  }
  MeasurementUnit unit = 
    PStateManager::instance ()->defaultMeasurementUnit ();
  const char *u = unitToString (unit);
  float xval, yval;
  xval = cvtPtToUnit (unit, rotCenter.x ());
  yval = cvtPtToUnit (unit, rotCenter.y ());

  sprintf (msgbuf, "%s [%.3f - %.3f %s, %.3f %s]", 
	   I18N ("Rotate"), angle, xval, u, yval, u);
  emit modeSelected (msgbuf);
}

void SelectionTool::scale (GDocument* doc, Canvas* canvas, 
			   int mask, float dx, float dy, 
			   bool permanent) {
  Rect& r = origbox;
  Rect newbox (origbox);
  float sx = 1, sy = 1;
  float xoff = r.x (), yoff = r.y ();
  float xback = xoff, yback = yoff;

  if (mask & Handle::HPos_Right)
    newbox.right (newbox.right () + dx);
  if (mask & Handle::HPos_Bottom) 
    newbox.bottom (newbox.bottom () + dy);
  if (mask & Handle::HPos_Left)
    newbox.left (newbox.left () + dx);
  if (mask & Handle::HPos_Top)
    newbox.top (newbox.top () + dy);

  Rect sbox = canvas->snapScaledBoxToGrid (newbox, mask);
  sx = sbox.width () / origbox.width ();
  sy = sbox.height () / origbox.height ();

  if (mask & Handle::HPos_Left) {
    xback = r.left () + r.width () * (1 - sx);
  }
  if (mask & Handle::HPos_Top) {
    yback = r.top () + r.height () * (1 - sy);
  }
  if (permanent) {
    for_each (doc->getSelection ().begin (), doc->getSelection ().end (), 
	      finalize_obj ());
    ScaleCmd *cmd = new ScaleCmd (doc, oldmask, sx, sy, r);
    history->addCommand (cmd, true);
  }
  else {
    QWMatrix m1, m2, m3;

    m1.translate (-xoff, -yoff);
    m2.scale (sx, sy);
    m3.translate (xback, yback);
    
    for (list<GObject*>::iterator it = doc->getSelection ().begin (); 
	 it != doc->getSelection ().end (); it++) {
      (*it)->setWorkInProgress (true);
      (*it)->initTmpMatrix ();
      
      (*it)->ttransform (m1);
      (*it)->ttransform (m2);
      (*it)->ttransform (m3, true);
    }
  }
  sprintf (msgbuf, "%s [%.3f %%, %.3f %%]", 
	   I18N ("Scale"), sx * 100.0, sy * 100.0);
  emit modeSelected (msgbuf);
}

void SelectionTool::shear (GDocument* doc, int mask, float dx, float dy, 
			   bool permanent) {
  Rect& r = origbox;
  float sx = 0.0, sy = 0.0;
  if (mask == Handle::HPos_Top)
    sx = -dx / r.width ();
  else if (mask == Handle::HPos_Bottom)
    sx = dx / r.width ();
  else if (mask == Handle::HPos_Left)
    sy = -dy / r.height ();
  else if (mask == Handle::HPos_Right)
    sy = dy / r.height ();

  if (permanent) {
    for_each (doc->getSelection ().begin (), doc->getSelection ().end (), 
	      finalize_obj ());
    ShearCmd *cmd = new ShearCmd (doc, rotCenter, sx, sy);
    history->addCommand (cmd, true);
  }
  else {
    QWMatrix m1, m2, m3;

    m1.translate (-rotCenter.x (), -rotCenter.y ());
    m2.shear (sx, sy);
    m3.translate (rotCenter.x (), rotCenter.y ());
    
    for (list<GObject*>::iterator it = doc->getSelection ().begin (); 
	 it != doc->getSelection ().end (); it++) {
      (*it)->setWorkInProgress (true);
      (*it)->initTmpMatrix ();
      
      (*it)->ttransform (m1);
      (*it)->ttransform (m2);
      (*it)->ttransform (m3, true);
    }
  }
  sprintf (msgbuf, "%s [%.3f %%, %.3f %%]", 
	   I18N ("Shear"), sx * 100.0, sy * 100.0);
  emit modeSelected (msgbuf);
}

void SelectionTool::activate (GDocument* doc, Canvas*) {
  doc->handle ().show (true);
  if (doc->lastObject ()) {
    if (doc->selectionIsEmpty ())
      doc->selectObject (doc->lastObject ());
    else
      doc->setAutoUpdate (true);
    state = S_Pick;
  }
  else
    state = S_Init;
  ctype = C_Arrow;

  if (doc->selectionIsEmpty ()) {
    emit modeSelected (I18N ("Selection Mode"));
  }
  else {
    Rect box = doc->boundingBoxForSelection ();
    MeasurementUnit unit = 
      PStateManager::instance ()->defaultMeasurementUnit ();
    const char *u = unitToString (unit);
    float x, y, w, h;
    x = cvtPtToUnit (unit, box.x ());
    y = cvtPtToUnit (unit, box.y ());
    w = cvtPtToUnit (unit, box.width ());
    h = cvtPtToUnit (unit, box.height ());
    if (doc->selectionCount () > 1) {
      sprintf (msgbuf, "%s [%.3f %s, %.3f %s, %.3f %s, %.3f %s]",
	       I18N ("Multiple Selection"), x, u, y, u, w, u, h, u);
    }
    else {
      GObject *sobj = doc->getSelection ().front ();
      sprintf (msgbuf, "%s [%.3f %s, %.3f %s, %.3f %s, %.3f %s]",
#if QT_VERSION >= 199
	       sobj->typeName ().ascii ()
#else
	       (const char *) sobj->typeName ()
#endif
, x, u, y, u, w, u, h, u);
    }
    emit modeSelected (msgbuf);
  }
}

void SelectionTool::deactivate (GDocument* doc, Canvas* canvas) {
  doc->handle ().show (false);
  canvas->updateView ();
}
