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

#ifndef PolygonConfigDialog_h
#define PolygonConfigDialog_h

#include <qdialog.h>
#include <qradiobt.h>
#include <qspinbox.h>
#include <qslider.h>

#include "PolygonTool.h"
#include "PolygonPreview.h"

class PolygonConfigDialog : public QDialog {
  Q_OBJECT
public:
  PolygonConfigDialog (QWidget* parent = 0L, const char* name = 0L);

  unsigned int numCorners ();
  unsigned int sharpness ();
  bool concavePolygon ();

  void setNumCorners (unsigned int num);
  void setSharpness (unsigned int value);
  void setConcavePolygon (bool flag);

  static void setupTool (PolygonTool* tool);

protected:
  QWidget* createWidget (QWidget* parent);

private slots:
  void helpPressed ();
  void slotConcavePolygon ();
  void slotConvexPolygon ();

private:
  QSpinBox *spinbox;
  QSlider *slider;
  QRadioButton *concaveButton, *convexButton;
  PolygonPreview *preview;
};

#endif
