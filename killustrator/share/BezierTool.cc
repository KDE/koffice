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
#ifdef __FreeBSD__
#include <math.h>
#else
#include <values.h>
#endif

#include "BezierTool.h"
#include "BezierTool.moc"
#include "GDocument.h"
#include "Canvas.h"
#include "Coord.h"
#include "GBezier.h"
#include "CreateBezierCmd.h"
#include "AddLineSegmentCmd.h"
#include "CommandHistory.h"
#include <qkeycode.h>
#include <kapp.h>
#include <klocale.h>
#include "version.h"

BezierTool::BezierTool (CommandHistory* history) : Tool (history) {
  curve = 0L;
  last = 0;
  newObj = true;
  oldNumOfPoints = 0;
}

void BezierTool::processEvent (QEvent* e, GDocument *doc, Canvas* canvas) {
  if (e->type () == 
#if QT_VERSION >= 199
      QEvent::KeyPress
#else
      Event_KeyPress
#endif
      ) {
    QKeyEvent *ke = (QKeyEvent *) e;
    if (ke->key () == QT_ESCAPE && curve != 0L) {
      /*
       * Abort the last operation
       */
      if (newObj) {
	if (last < 5) {
          // no valid curve - delete it
          doc->deleteObject (curve);
          curve = 0L;
	}
	else { // valid curve
	  int last_valid = last - ((last - 5) % 3);
	  // for a new object we have to remove the last points
	  for (int i = last; i > last_valid; i--)
	    curve->removePoint (i);

          CreateBezierCmd *cmd = new CreateBezierCmd (doc, curve);
          history->addCommand (cmd);
	  doc->setLastObject (curve);
        }
      }
      else { // segments were added
	QList<Coord> points;
	points.setAutoDelete (true);
	if (addAtEnd) {
	  int last_valid = last - ((last - 5) % 3);
	  int i;

	  for (i = last; i > last_valid; i--)
	    curve->removePoint (i);

	  for (int i = oldNumOfPoints; i < (int) curve->numOfPoints (); i++)
	    points.append (new Coord (curve->getPoint (i)));

	  if (points.count () > 0) {
	    last = last - points.count () + 1;
	    AddLineSegmentCmd *cmd =  
	      new AddLineSegmentCmd (doc, curve, last_valid - 2, points);
	    history->addCommand (cmd);
	  }
	}
	else { // not at end of the curve
	  int diff = curve->numOfPoints () - oldNumOfPoints;
	  if (diff > 0) {
	    // something was added
	    if (last == 1) {
	      // incomplete segment
	      for (int i = 0; i < 3; i++)
		curve->removePoint (0);
	      diff -= 3;
	    }
	    if (diff > 0) {
	      // line segment was complete
	      for (int i = 0; i < diff; i++) 
		points.append (new Coord (curve->getPoint (i)));
	      AddLineSegmentCmd *cmd =  
		new AddLineSegmentCmd (doc, curve, 0, points);
	      history->addCommand (cmd);
	    }
	  }
	}
      }
      if (curve)
	curve->setWorkingSegment (-1);
      curve = 0L; last = 0;
      emit operationDone ();
    }
  }
  else if (e->type () == 
#if QT_VERSION >= 199
	   QEvent::MouseButtonPress
#else
	   Event_MouseButtonPress
#endif
	   ) {
    QMouseEvent *me = (QMouseEvent *) e;
    if (me->button () != LeftButton)
      return;

    float xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);

    if (curve == 0L) {
      newObj = true;
      addAtEnd = true;

      GBezier* obj = 0L;
 
      if (me->state () & ShiftButton) {
	// magnetic mode
	GObject *o = 0L;
	int idx = -1;
	if (doc->findNearestObject ("GBezier", xpos, ypos, 
				    80, o, idx)) {
	  curve = (GBezier *) o;
	  last = idx;
	  addAtEnd = (last != 1);
	  newObj = false;
	  oldNumOfPoints = curve->numOfPoints ();
	}
      }
      else {
	QList<GObject> olist;
	// look for existing bezier curves with an
	// end point near the mouse pointer
	if (doc->findContainingObjects (xpos, ypos, olist)) {
	  QListIterator<GObject> it (olist);
	  while (it.current ()) {
	    if (it.current ()->isA ("GBezier")) {
	      obj = (GBezier *) it.current ();
	      break;
	    }
	    ++it;
	  }
	}
	if (obj && ((last = obj->getNeighbourPoint (Coord (xpos, ypos))) != -1)
	    && obj->isEndPoint (last) 
	    && (last == 1 || last == (int) obj->numOfPoints () - 2)) {
	  curve = obj;
	  addAtEnd = (last != 1);
	  newObj = false;
	  oldNumOfPoints = obj->numOfPoints ();
	}
      }
      
      if (curve == 0L) {
	curve = new GBezier ();
	// base point #1
	curve->addPoint (0, Coord (MAXFLOAT, MAXFLOAT));
	// first end point
	curve->addPoint (1, Coord (xpos, ypos));
	last = 2;
	doc->insertObject (curve);
	// base point #2
	curve->addPoint (last, Coord (xpos, ypos));
	curve->setWorkingSegment (0);
      }
      else {
	if (last == 1) {
	  last = 0;
	  curve->setWorkingSegment (0);
	}
	else {
	  // set current point to base point #1
	  last = curve->numOfPoints () - 1;
	  curve->setWorkingSegment (last / 3 - 1);
	}
      }
    }
    else {
      if (! addAtEnd) {
	if (last == 0) {
	  // add at beginning of curve
	  curve->addPoint (0, Coord (MAXFLOAT, MAXFLOAT), false);
	  curve->addPoint (0, Coord (xpos, ypos), false);
	  curve->addPoint (0, Coord (MAXFLOAT, MAXFLOAT));
	  last = 1;
	}
	else if (last == 1) 
	  last = 0;
	curve->setWorkingSegment (0);
      }
      else {
	// add at end of curve
	last = curve->numOfPoints ();
	if (! curve->isEndPoint (last - 1)) 
	  curve->initBasePoint (last - 3);
	
	if (last >= 3 && (last % 3 == 0))
	  // base point #2
	  curve->addPoint (last++, Coord (MAXFLOAT, MAXFLOAT), false);
	
	// next end point
	curve->addPoint (last, Coord (xpos, ypos));
	curve->setWorkingSegment (last / 3 - 1);
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
    if (curve == 0L)
      return;

    QMouseEvent *me = (QMouseEvent *) e;
    float xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);

    curve->setPoint (last, Coord (xpos, ypos));
  }
  else if (e->type () == 
#if QT_VERSION >= 199
	   QEvent::MouseButtonRelease
#else
	   Event_MouseButtonRelease
#endif
	   ) {
    if (curve == 0L)
      return;
    QMouseEvent *me = (QMouseEvent *) e;
    float xpos = me->x (), ypos = me->y ();
    canvas->snapPositionToGrid (xpos, ypos);
    
    curve->setPoint (last, Coord (xpos, ypos));
    if (me->button () == RightButton) {
      if ((addAtEnd && last >= 5 && (last % 3 == 2)) ||
	  (!addAtEnd && last == 0 && (curve->numOfPoints () % 3 == 0))) {
	doc->setLastObject (curve);
	curve->setWorkingSegment (-1);
	if (newObj) {
	  CreateBezierCmd *cmd = new CreateBezierCmd (doc, curve);
	  history->addCommand (cmd);
	}
	else {
	  QList<Coord> points;
	  points.setAutoDelete (true);

	  if (last == 0) {
	    for (int i = curve->numOfPoints () - oldNumOfPoints - 1; 
		 i >= 0; i--)
	      points.append (new Coord (curve->getPoint (i)));
	  }
	  else {
	    for (int i = oldNumOfPoints; i < (int) curve->numOfPoints (); i++)
	      points.append (new Coord (curve->getPoint (i)));
	    last = last - points.count () + 1;
	  }
	  AddLineSegmentCmd *cmd =  
	    new AddLineSegmentCmd (doc, curve, last, points);
	  history->addCommand (cmd);
	}
	curve = 0L; last = 0;
      }
      else if (last >= 7 && last % 3 == 1 &&
	       curve->getPoint (1).isNear (Coord (xpos, ypos), 
					   NEAR_DISTANCE)) {
	  curve->addPoint (last, Coord (xpos, ypos));
	  curve->setClosed (true);
	  curve->setWorkingSegment (-1);
	  doc->setLastObject (curve);
	curve = 0L; last = 0;
      }
    }
  }
  return;
}

void BezierTool::activate (GDocument* , Canvas*) {
  emit modeSelected (i18n ("Create Bezier Curve"));
}

void BezierTool::deactivate (GDocument*, Canvas*) {
  if (curve)
    curve->setWorkingSegment (-1);
  curve = 0L;
  last = 0;
}
