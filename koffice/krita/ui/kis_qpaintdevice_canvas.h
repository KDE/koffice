/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_QPAINTDEVICE_CANVAS_H_
#define KIS_QPAINTDEVICE_CANVAS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>

#include "kis_global.h"
#include "kis_canvas.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif // Q_WS_X11

class KisQPaintDeviceCanvasWidget : public virtual QWidget, public virtual KisCanvasWidget {
public:
    KisQPaintDeviceCanvasWidget(QWidget *parent = 0, const char *name = 0);
    ~KisQPaintDeviceCanvasWidget();

    virtual KisCanvasWidgetPainter *createPainter();

#if defined(EXTENDED_X11_TABLET_SUPPORT)
    virtual void selectTabletDeviceEvents();
#endif

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void tabletEvent(QTabletEvent *event);
    virtual void enterEvent(QEvent *event );
    virtual void leaveEvent(QEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
#ifdef Q_WS_X11
    bool x11Event(XEvent *event);
#endif // Q_WS_X11
};

#endif // KIS_QPAINTDEVICE_CANVAS_H_

