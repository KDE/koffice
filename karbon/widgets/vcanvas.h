/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __VCANVAS_H__
#define __VCANVAS_H__


#include <QResizeEvent>
#include <QEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QTabletEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QWidget>
#include <QList>

#include <koffice_export.h>
#include <KoCanvasBase.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoShape.h>

#include <QBrush>
#include <QPainter>

class QPointF;
class QRectF;
class KoViewConverter;
class KoShapeManager;
class KoToolProxy;
class KCommand;
class KCommandHistory;
class VDocument;
class KarbonPart;

class KarbonCanvas: public QWidget, public KoCanvasBase
{
    Q_OBJECT

public:
    KarbonCanvas( KarbonPart *p );
    virtual ~KarbonCanvas();

    // from KoCanvasBase
    virtual void gridSize(double *horizontal, double *vertical) const;
    virtual bool snapToGrid() const;
    virtual KoUnit unit();

    void addCommand(KCommand *command, bool execute = true);

    KoShapeManager *shapeManager() const { return m_shapeManager; }

    /**
     * Tell the canvas repaint the specified rectangle. The coordinates
     * are document coordinates, not view coordinates.
     */
    void updateCanvas(const QRectF& rc);

    KoViewConverter *viewConverter() { return &m_zoomHandler; }

    QWidget *canvasWidget() { return this; }

    void setCommandHistory( KCommandHistory* history ) { m_commandHistory = history; }

    /** Sets the maximal available visible size. */
    void setVisibleSize( int visibleWidth, int visibleHeight );

    /** Sets the margins in pixel used when fitting to page or to width. */
    void setFitMargin( int fitMarginX, int fitMarginY );

    virtual QPoint documentOrigin();

    KoToolProxy * toolProxy() { return m_toolProxy; }


public slots:

    /**
     * Tell the canvas that it has to adjust its size.
     * The new size depends on the current document size and the actual zoom factor.
     * If the new calculated size is smaller than the visible size set
     * by setVisibleSize, the visible size is used as the new size.
     */
    void adjustSize();

signals:
    void documentOriginChanged( const QPoint &origin );
protected:
    void paintEvent(QPaintEvent * ev);
    void mouseEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void keyReleaseEvent (QKeyEvent *e);
    void keyPressEvent (QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void tabletEvent( QTabletEvent *e );
    void wheelEvent( QWheelEvent *e );

    QPoint widgetToView( const QPoint& p ) const;
    QRect widgetToView( const QRect& r ) const;
    QPoint viewToWidget( const QPoint& p ) const;
    QRect viewToWidget( const QRect& r ) const;

private:
    KoShapeManager* m_shapeManager;
    KoZoomHandler m_zoomHandler;

    KCommandHistory *m_commandHistory;

    KoToolProxy *m_toolProxy;

    VDocument *m_doc;
    QRectF m_contentRect;  ///< the content rect around all content of the document (>=m_documentRect)
    QRectF m_documentRect; ///< the doument page rect defining the documents page size
    QPoint m_origin;       ///< the origin of the document page rect
    int m_marginX;         ///< the minimum x margin around the document page rect
    int m_marginY;         ///< the minimum y margin around the document page rect
    int m_visibleWidth;    ///< available space in x-direction used for calulating zoom fit and margins
    int m_visibleHeight;   ///< available space in y-direction used for calulating zoom fit and margins
    int m_fitMarginX;      ///< x-margin used when zoom fitting to page or width
    int m_fitMarginY;      ///< y-margin used when zoom fitting to page or width
};

#endif
