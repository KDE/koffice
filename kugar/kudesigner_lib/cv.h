	/***************************************************************************
                          cv.h  -  description
                             -------------------
    begin                : 07.06.2002
    copyright            : (C) 2002 by Alexander Dymo
    email                : cloudtemple@mksat.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
#ifndef CV_H
#define CV_H

#include <map>

#include <qcanvas.h>
#include <qpainter.h>
#include <qptrlist.h>

#include "property.h"

class CanvasReportItem;
class QMouseEvent;
class QCanvasItemList;
class CanvasBox;
class KuDesignerPlugin;
class MyCanvas;
class KudesignerView;
class CanvasBand;

class SelectionRect: public QCanvasRectangle{
public:
    SelectionRect( int x, int y, int width, int height, QCanvas * canvas):
        QCanvasRectangle(x, y, width, height, canvas) {}

    virtual void draw(QPainter & painter);
};

class ReportCanvas: public QCanvasView{
    Q_OBJECT
public:
    ReportCanvas(QCanvas * canvas, QWidget * parent, const char * name = 0, WFlags f = 0);

    int itemToInsert;

    enum RequestType {RequestNone = 0, RequestProps, RequestDelete};

    void setRequest(RequestType r);
    void clearRequest();
    bool requested();

    void finishSelection();

    void setPlugin(KuDesignerPlugin *plugin);
protected:
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);
    void contentsMouseDoubleClickEvent( QMouseEvent * );

    void contentsDragEnterEvent ( QDragEnterEvent * );
    void contentsDragMoveEvent ( QDragMoveEvent * );
//    void contentsDragLeaveEvent ( QDragLeaveEvent * );
//    void contentsDropEvent ( QDropEvent * );
    void keyPressEvent( QKeyEvent * );

    void startMoveOrResizeOrSelectItem(QCanvasItemList &l, QMouseEvent *e, QPoint &p);
    bool startResizing(QMouseEvent *e, QPoint &p);
    void placeItem(QCanvasItemList &l, QMouseEvent *e);
    void editItem(QCanvasItemList &l);
    void deleteItem(QCanvasItemList &l);
    void selectItemFromList(QCanvasItemList &l);

private:
    CanvasReportItem *moving;
    QPoint moving_start;
    double moving_offsetY;
    double moving_offsetX;
    QRect resizing_constraint;
    QSize resizing_minSize;
    int resizing_type;
    CanvasBox *resizing;
    bool selectionStarted;
    KuDesignerPlugin *m_plugin;
    KudesignerView *m_view;

    SelectionRect *selectionRect;

    RequestType request;

    MyCanvas *m_canvas;

    void fixMinValues(double &pos,double minv,double &offset);
    void fixMaxValues(double &pos,double size,double maxv,double &offset);

//    std::map<QString, PropPtr > selProps;

signals: // Signals
  /** Emitted when user clicks on the canvas, so a button
or a menu item associated with the selected item should
be unchecked. */
  void selectedActionProcessed();
  void selectedEditActionProcessed();
  void modificationPerformed();

  /** Emitted when selection is made, so that property editor can display properties
      for the selected items. */
  void selectionMade(std::map<QString, PropPtr > *v_props,CanvasBox*);
  void selectionClear();

  void itemPlaced(int x, int y, int band, int bandLevel);

public slots:
    void updateProperty(QString name, QString value);

    void selectItem();
};

#endif
