/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kimage_gui_h__
#define __kimage_gui_h__

#include <qpixmap.h>
#include <qwidget.h>

#include <container.h>

class KAction;
class KToggleAction;
class QPaintEvent;

class KImageDocument;

class KImageView : public ContainerView
{
  Q_OBJECT

public:
  KImageView( KImageDocument* doc, QWidget* _parent = 0, const char* _name = 0 );

  virtual void paintEvent( QPaintEvent* );

 public slots:
//  void slotDocUpdated();
//  void slotDocUpdated(const QRect&);

 signals:
  void mousePressed(QMouseEvent *);
  void mouseMoved(QMouseEvent *);
  void mouseReleased(QMouseEvent *);

 protected slots:

  // edit action slots

  void undo();
  void redo();
  void copy();
  void cut();
  void paste();

/*
  void editImportImage();
  void editExportImage();
  void editPageLayout();
  void editPreferences();
*/

  void viewZoomFactor();
  void viewFitToView();
  void viewFitWithProportions();
  void viewOriginalSize();
  void viewCentered();
  void viewScrollbars();
  void viewInformations();
  void viewBackgroundColor();

  void transformRotateRight();
  void transformRotateLeft();
  void transformRotateAngle();
  void transformFlipVertical();
  void transformFlipHorizontal();
  void transformZoomFactor();
  void transformZoomIn10();
  void transformZoomOut10();
  void transformZoomDouble();
  void transformZoomHalf();
  void transformZoomMax();
  void transformZoomMaxAspect();

public slots:
  // Document signals
  void slotUpdateView();

protected:
  enum DrawMode { OriginalSize, FitToView, FitWithProps, ZoomFactor };

/*
  virtual void resizeEvent( QResizeEvent* _ev );
  virtual void paintEvent( QPaintEvent* _ev );
  virtual void mousePressEvent ( QMouseEvent * );
  virtual void mouseReleaseEvent ( QMouseEvent * );
  virtual void mouseMoveEvent ( QMouseEvent * );
*/

 private:

  // edit menu
  KAction *m_undo, *m_redo, *m_cut, *m_copy, *m_paste, *m_import, *m_export, *m_pageSetup, *m_preferences;

  // view menu
  KAction *m_viewFactor, *m_fitToView, *m_fitWithProps, *m_original, *m_center, *m_scrollbars, *m_info, *m_backgroundColor;

  // transform menu
  KAction *m_rotateRight, *m_rotateLeft, *m_rotateAngle, *m_flipVertical, *m_flipHorizontal;
  KAction *m_zoomFactor, *m_zoomIn10, *m_zoomOut10, *m_zoomDouble, *m_zoomHalf, *m_zoomMax, *m_zoomMaxAspect;

  // help menu
  KAction        *m_helpAbout;
  KAction        *m_helpUsing;

  QPoint          m_zoomFactorValue;
  KImageDocument *m_pDoc;
  QPixmap         m_pixmap;
  DrawMode        m_drawMode;
  int             m_centerMode;
};

#endif



























































































