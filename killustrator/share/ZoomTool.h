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

#ifndef ZoomTool_h_
#define ZoomTool_h_

#include <Tool.h>
#include <qvaluelist.h>

class Canvas;
class GDocument;
class Coord;

class ZoomTool : public Tool {
public:
  ZoomTool (CommandHistory *history);

  virtual void processEvent (QEvent* e, GDocument* _doc, Canvas* _canvas);
  virtual void activate (GDocument* _doc, Canvas* _canvas);
  
  void zoomIn (Canvas* cnv);
  void zoomOut (Canvas* cnv);
  void zoomInRegion(int x1,int y1, int x2, int y2);
  void zoomOutRegion(int x1,int y1, int x2, int y2);

protected:
  void processMouseMoveEvent (QMouseEvent* e);
  void processButtonReleaseEvent (QMouseEvent* e);
  
private:
  GDocument *doc;
  Canvas *canvas;

  enum State { S_Init, S_Rubberband};
  State state;
  Coord selPoint[2];
};

#endif
