/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "kivio_canvas.h"
#include "kivio_guidelines.h"
#include "kivio_page.h"
#include "kivio_map.h"
#include "kivio_view.h"
#include "kivio_doc.h"
#include "kivio_tabbar.h"

#include "kivio_icon_view.h"
#include "kivio_stencil.h"
#include "kivio_stencil_spawner.h"
#include "kivio_stencil_spawner_info.h"
#include "kivio_stackbar.h"
#include "kivio_screen_painter.h"
#include "kivio_grid_data.h"

#include "tool_controller.h"
#include "tool.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <koGlobal.h>
#include <kozoomhandler.h>
#include <koSize.h>
#include <koRuler.h>

#include <assert.h>
#include <stdio.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qscrollbar.h>
#include <qtimer.h>
#include <qsize.h>

using namespace Kivio;

KivioCanvas::KivioCanvas( QWidget *par, KivioView* view, KivioDoc* doc, ToolController* tc, QScrollBar* vs, QScrollBar* hs)
: QWidget(par, "KivioCanvas", WResizeNoErase | WRepaintNoErase),
  m_pView(view),
  m_pDoc(doc),
  m_pToolsController(tc),
  m_pVertScrollBar(vs),
  m_pHorzScrollBar(hs)
{
  setBackgroundMode(NoBackground);
  setAcceptDrops(true);
  setMouseTracking(true);
  setFocusPolicy(StrongFocus);
  setFocus();

  delegateThisEvent = true;
  storedCursor = 0;
  pressGuideline = 0;

  m_pVertScrollBar->setLineStep(1);
  m_pHorzScrollBar->setLineStep(1);


  m_pVertScrollBar->setPageStep(10);
  m_pHorzScrollBar->setPageStep(10);

  connect(m_pVertScrollBar, SIGNAL(valueChanged(int)), SLOT(scrollV(int)));
  connect( m_pHorzScrollBar, SIGNAL(valueChanged(int)), SLOT(scrollH(int)));

  m_iXOffset = 0;
  m_iYOffset = 0;

  m_pScrollX = 0;
  m_pScrollY = 0;

  m_pasteMoving = false;

  m_buffer = new QPixmap();

  m_pDragStencil = 0L;
  unclippedSpawnerPainter = 0L;
  unclippedPainter = 0L;

  m_borderTimer = new QTimer(this);
  connect(m_borderTimer,SIGNAL(timeout()),SLOT(borderTimerTimeout()));

  m_guideLinesTimer = new QTimer(this);
  connect(m_guideLinesTimer,SIGNAL(timeout()),SLOT(guideLinesTimerTimeout()));
}

KivioCanvas::~KivioCanvas()
{
  delete m_buffer;
  delete m_borderTimer;
  delete m_guideLinesTimer;
  delete storedCursor;
  delete unclippedPainter;
}

KivioPage* KivioCanvas::findPage( const QString& _name )
{
  return m_pDoc->map()->findPage( _name );
}

const KivioPage* KivioCanvas::activePage() const
{
  return m_pView->activePage();
}

KivioPage* KivioCanvas::activePage()
{
  return m_pView->activePage();
}

void KivioCanvas::scrollH( int value )
{
  eraseGuides();

  // Relative movement
  int dx = m_iXOffset - value;
  // New absolute position
  m_iXOffset = value;

  bitBlt(m_buffer, dx, 0, m_buffer);
  scroll(dx, 0);

  emit visibleAreaChanged();
}

void KivioCanvas::scrollV( int value )
{
  eraseGuides();

  // Relative movement
  int dy = m_iYOffset - value;
  // New absolute position
  m_iYOffset = value;

  bitBlt(m_buffer, 0, dy, m_buffer);
  scroll(0, dy);

  emit visibleAreaChanged();
}

void KivioCanvas::scrollDx( int dx )
{
  if ( dx == 0 )
    return;

  int value = m_iXOffset - dx;
  m_pHorzScrollBar->setValue(value);
}

void KivioCanvas::scrollDy( int dy )
{
  if ( dy == 0 )
    return;

  int value = m_iYOffset - dy;
  m_pVertScrollBar->setValue(value);
}

void KivioCanvas::resizeEvent( QResizeEvent* )
{
  KivioGuideLines::resize(size(),m_pDoc);
  m_buffer->resize(size());
  updateScrollBars();

  emit visibleAreaChanged();
}

void KivioCanvas::wheelEvent( QWheelEvent* ev )
{
  ev->accept();
/*
  QPoint p = ev->pos();
  if ((ev->delta()<0)) {
    zoomIn(p);
  } else {
    zoomOut(p);
  }
*/
  //QPoint p = ev->pos();
  if( (ev->delta()>0))
  {
     m_pVertScrollBar->setValue(m_pVertScrollBar->value() - 30);
  }
  else
  {
     m_pVertScrollBar->setValue(m_pVertScrollBar->value() + 30);
  }
}

void KivioCanvas::setUpdatesEnabled( bool isUpdate )
{
  static int i = 0;

  QWidget::setUpdatesEnabled(isUpdate);
  if (isUpdate) {
    --i;
    if (i == 0) {
      update();
      updateScrollBars();

      blockSignals(false);

      emit zoomChanges();
      emit visibleAreaChanged();
    }
  } else {
    i++;
    blockSignals(true);
  }
}

void KivioCanvas::zoomIn(const QPoint &p)
{
  setUpdatesEnabled(false);
  KoPoint p0 = mapFromScreen(p);
  setZoom(m_pView->zoomHandler()->zoom() + 25);
  QPoint p1 = mapToScreen(p0);
  scrollDx(-p1.x()+p.x());
  scrollDy(-p1.y()+p.y());
  setUpdatesEnabled(true);
}

void KivioCanvas::zoomOut(const QPoint &p)
{
  setUpdatesEnabled(false);
  KoPoint p0 = mapFromScreen(p);
  int newZoom = m_pView->zoomHandler()->zoom() - 25;

  if(newZoom > 0) {
    setZoom(newZoom);
    QPoint p1 = mapToScreen(p0);
    scrollDx(-p1.x()+p.x());
    scrollDy(-p1.y()+p.y());
  }
  setUpdatesEnabled(true);
}

void KivioCanvas::paintEvent( QPaintEvent* ev )
{
  if ( m_pDoc->isLoading() || !activePage() )
    return;

  KivioPage* page = activePage();
  eraseGuides();

  QPainter painter;
  painter.begin(m_buffer);

  QRect rect( ev->rect() );
  painter.fillRect(rect,white);

  // Draw Grid
  if (m_pDoc->grid().isShow) {
    int x = rect.x();
    int y = rect.y();
    int w = x + rect.width();
    int h = y + rect.height();

    KoSize dxy = m_pDoc->grid().freq;

    KoPoint p(0, 0);

    painter.setPen(m_pDoc->grid().color);

    QPoint p0 = mapToScreen(p);

    while (p0.x() < x) {
      p.rx() += dxy.width();
      p0 = mapToScreen(p);
    }

    while (p0.x() <= w) {
      painter.drawLine(p0.x(),y,p0.x(),h);

      p.rx() += dxy.width();
      p0 = mapToScreen(p);
    }

    p.setCoords(0, 0);
    p0 = mapToScreen(p);

    while (p0.x() > w) {
      p.rx() -= dxy.width();
      p0 = mapToScreen(p);
    }

    while (p0.x() >= x) {
      painter.drawLine(p0.x(),y,p0.x(),h);

      p.rx() -= dxy.width();
      p0 = mapToScreen(p);
    }

    p.setCoords(0, 0);
    p0 = mapToScreen(p);

    while (p0.y() < y) {
      p.ry() += dxy.height();
      p0 = mapToScreen(p);
    }

    while (p0.y() <= h) {
      painter.drawLine(x,p0.y(),w,p0.y());

      p.ry() += dxy.height();
      p0 = mapToScreen(p);
    }

    p.setCoords(0, 0);
    p0 = mapToScreen(p);

    while (p0.y() > h) {
      p.ry() -= dxy.height();
      p0 = mapToScreen(p);
    }

    while (p0.y() >= y) {
      painter.drawLine(x,p0.y(),w,p0.y());

      p.ry() -= dxy.height();
      p0 = mapToScreen(p);
    }
  }

  painter.translate(-m_iXOffset,-m_iYOffset);

  KoPageLayout pl = page->paperLayout();
  int pw = m_pView->zoomHandler()->zoomItX(pl.ptWidth);
  int ph = m_pView->zoomHandler()->zoomItY(pl.ptHeight);

  if (m_pView->isShowPageMargins()) {
    int ml = m_pView->zoomHandler()->zoomItX(pl.ptLeft);
    int mt = m_pView->zoomHandler()->zoomItY(pl.ptTop);
    int mr = m_pView->zoomHandler()->zoomItX(pl.ptRight);
    int mb = m_pView->zoomHandler()->zoomItY(pl.ptBottom);

    painter.save();
    painter.setPen(QPen(blue,1,DotLine));
    painter.drawRect(ml,mt,pw-ml-mr,ph-mt-mb);
    painter.restore();
  }

  if (m_pView->isShowPageBorders()) {
    painter.setPen(black);
    painter.fillRect(pw, 3, 5, ph, gray);
    painter.fillRect(3, ph, pw, 3, gray);
    painter.drawRect(0, 0, pw, ph);
  }

  // Draw content
  KivioScreenPainter kpainter;
  kpainter.start( m_buffer );
  kpainter.painter()->translate( -m_iXOffset, -m_iYOffset );
  kpainter.painter()->translate( 0, 0 );
  m_pDoc->paintContent(kpainter, rect, false, page, QPoint(0, 0), m_pView->zoomHandler(), true);
  kpainter.stop();

  paintGuides(false);
  painter.end();

  bitBlt(this,rect.left(),rect.top(),m_buffer,rect.left(),rect.top(),rect.width(),rect.height());
}

void KivioCanvas::updateScrollBars()
{
  KoPageLayout pl = activePage()->paperLayout();
  m_pScrollX = m_pView->zoomHandler()->zoomItX(pl.ptWidth) - width();
  m_pScrollY = m_pView->zoomHandler()->zoomItY(pl.ptHeight) - height();

  m_pHorzScrollBar->setRange(0, m_pScrollX);
  if ( m_pHorzScrollBar->value() > m_pHorzScrollBar->maxValue() ||
       m_pHorzScrollBar->value() < m_pHorzScrollBar->minValue() )
  {
    m_pHorzScrollBar->setValue(0);
  }

  m_pVertScrollBar->setRange(0, m_pScrollY);
  if ( m_pVertScrollBar->value() > m_pVertScrollBar->maxValue() ||
       m_pVertScrollBar->value() < m_pVertScrollBar->minValue() )
  {
    m_pVertScrollBar->setValue(0);
  }

  m_pVertScrollBar->setPageStep( height() );
  m_pHorzScrollBar->setPageStep( width() );
}

QSize KivioCanvas::actualSize()
{
  return QSize(m_pScrollX, m_pScrollY);
}

void KivioCanvas::setZoom(int zoom)
{
  if(zoom < 10 || zoom > 2000) {
    return;
  }
  
  m_pView->zoomHandler()->setZoomAndResolution(zoom, QPaintDevice::x11AppDpiX(),
    QPaintDevice::x11AppDpiY());

  updateScrollBars();
  erase();
  repaint();

  emit zoomChanges();
  emit visibleAreaChanged();
}

KoSize KivioCanvas::actualGridFrequency()
{
  KoSize actual;
  actual.setWidth(m_pView->zoomHandler()->zoomItX(m_pDoc->grid().freq.width()));
  actual.setHeight(m_pView->zoomHandler()->zoomItY(m_pDoc->grid().freq.height()));

  return actual;
}

bool KivioCanvas::event( QEvent* e )
{
  bool f = QWidget::event(e);
  if (m_pToolsController && delegateThisEvent)
    m_pToolsController->delegateEvent(e,this);

  delegateThisEvent = true;
  return f;
}

void KivioCanvas::enterEvent( QEvent* )
{
}

void KivioCanvas::leaveEvent( QEvent* )
{
  m_pView->setMousePos(-1, -1);
}

void KivioCanvas::mousePressEvent(QMouseEvent* e)
{
    if(!m_pDoc->isReadWrite())
        return;
    if(m_pasteMoving) {
      endPasteMoving();
      return;
    }

    if(m_pView->isShowGuides())
    {
        lastPoint = e->pos();
        KoPoint p = mapFromScreen(e->pos());
        KivioGuideLines* gl = activePage()->guideLines();

        bool unselectAllGuideLines = true;
        pressGuideline = 0;

        if ((e->state() & ~ShiftButton) == NoButton) {
            KivioGuideLineData* gd = gl->find(p.x(),p.y(), m_pView->zoomHandler()->unzoomItY(2));
            if (gd) {
                pressGuideline = gd;
                if ((e->button() == RightButton) || ((e->button() & ShiftButton) == ShiftButton)) {
                    if (gd->isSelected())
                        gl->unselect(gd);
                    else
                        gl->select(gd);
                } else {
                    if (!gd->isSelected()) {
                        gl->unselectAll();
                        gl->select(gd);
                    }
                }
                unselectAllGuideLines = false;
                delegateThisEvent = false;
                updateGuides();
                m_guideLinesTimer->start(500,true);
            }
        }

        if (unselectAllGuideLines && gl->hasSelected()) {
            gl->unselectAll();
            updateGuides();
        }
    }
}

void KivioCanvas::mouseReleaseEvent(QMouseEvent* e)
{
  if(!m_pDoc->isReadWrite())
    return;

  if (pressGuideline) {
    m_guideLinesTimer->stop();
    KoPoint p = mapFromScreen(e->pos());
    bool insideCanvas = geometry().contains(mapFromGlobal(e->globalPos()));
    KivioGuideLines* gl = activePage()->guideLines();

    if(!insideCanvas) {
      eraseGuides();
      gl->remove(pressGuideline);
      paintGuides();
      delegateThisEvent = false;
      pressGuideline = 0;
      return;
    }

    KivioGuideLineData* gd = gl->find(p.x(),p.y(),m_pView->zoomHandler()->unzoomItY(2));
    if (gd) {
      setCursor(gd->orientation()==Qt::Vertical ? sizeHorCursor:sizeVerCursor);
    } else {
      updateGuidesCursor();
    }
    delegateThisEvent = false;
    pressGuideline = 0;
  }
}

void KivioCanvas::mouseMoveEvent(QMouseEvent* e)
{
  if(!m_pDoc->isReadWrite())
    return;

  if(m_pasteMoving) {
    continuePasteMoving(e->pos());
  } else {
    if(m_pView->isShowGuides())
    {
      m_pView->setMousePos(e->pos().x(),e->pos().y());

      KivioGuideLines* gl = activePage()->guideLines();

      if ((e->state() & LeftButton == LeftButton) && gl->hasSelected()) {
        if (m_guideLinesTimer->isActive()) {
          m_guideLinesTimer->stop();
          guideLinesTimerTimeout();
        }
        delegateThisEvent = false;
        eraseGuides();
        QPoint p = e->pos();
        p -= lastPoint;
        if (p.x() != 0)
          gl->moveSelectedByX(m_pView->zoomHandler()->unzoomItX(p.x()));
        if (p.y() != 0)
          gl->moveSelectedByY(m_pView->zoomHandler()->unzoomItY(p.y()));
        m_pDoc->setModified( true );
        paintGuides();
      } else {
        if ((e->state() & ~ShiftButton) == NoButton) {
          KoPoint p = mapFromScreen(e->pos());
          KivioGuideLineData* gd = gl->find(p.x(), p.y(), m_pView->zoomHandler()->unzoomItY(2));
          if (gd) {
            delegateThisEvent = false;
            if (!storedCursor)
              storedCursor = new QCursor(cursor());
            setCursor(gd->orientation()==Qt::Vertical ? sizeHorCursor:sizeVerCursor);
          } else {
            updateGuidesCursor();
          }
        }
      }
    }
  }

  lastPoint = e->pos();
}

QPoint KivioCanvas::mapToScreen( KoPoint pos )
{
  QPoint p;
  int x = m_pView->zoomHandler()->zoomItX(pos.x());
  int y = m_pView->zoomHandler()->zoomItY(pos.y());

  p.setX( x - m_iXOffset );
  p.setY( y - m_iYOffset );

  return p;
}

KoPoint KivioCanvas::mapFromScreen( const QPoint & pos )
{
  int x = pos.x() + m_iXOffset;
  int y = pos.y() + m_iYOffset;
  double xf = m_pView->zoomHandler()->unzoomItX(x);
  double yf = m_pView->zoomHandler()->unzoomItY(y);

  KoPoint p(xf, yf);
  return p;
}

void KivioCanvas::startRectDraw( const QPoint &p, RectType )
{
  currRect = QRect( 0, 0, -1, -1 );

  QPoint pos( p );
  oldRectValid = false;
  beginUnclippedPainter();
  rectAnchor = pos;
  currRect = QRect( rectAnchor, QPoint(0,0) );

  m_borderTimer->start(100);
}

void KivioCanvas::continueRectDraw( const QPoint &p, RectType )
{
  QPoint pos = p;
  QPoint p2 = pos;
  QRect r( rectAnchor, p2 );
  r = r.normalize();

  if ( oldRectValid )
    unclippedPainter->drawRect( currRect );
  if ( r.width() > 1 || r.height() > 1 ) {
    oldRectValid = true;
    currRect = r;
    unclippedPainter->drawRect( currRect );
  } else {
    oldRectValid = false;
  }
}

void KivioCanvas::endRectDraw()
{
  m_borderTimer->stop();

  if ( !unclippedPainter )
    return;

  if ( oldRectValid )
    unclippedPainter->drawRect( currRect );

  endUnclippedPainter();
}

/**
 * Starts a new drag & draw (called from the drag enter event)
 *
 * @param p The point to begin at
 *
 * This will allocate a new KivioStencil for drawing with and
 * set some class variables used during redraws.
 */
void KivioCanvas::startSpawnerDragDraw( const QPoint &p )
{
  currRect = QRect( 0, 0, -1, -1 );

  KivioStencilSpawner *pSpawner = KivioIconView::curDragSpawner();
  if( !pSpawner )
    return;

  // If we for some reason didn't delete an old drag stencil,
  // do so now.
  if( m_pDragStencil )
  {
    kdDebug(43000) << "KivioCanvas::startSpawnerDragDraw() - m_pDragStencil still exists.  BUG!" << endl;
    delete m_pDragStencil;
    m_pDragStencil = 0L;
  }

  // Map the point from screenspace to page space
  KoPoint qp = mapFromScreen( p );
  qp = snapToGrid(qp);

  // Allocate a new stencil for dragging around
  m_pDragStencil = pSpawner->newStencil();
  m_pDragStencil->setPosition( qp.x(), qp.y() );

  // Invalidate the rectangle
  oldRectValid = true;

  // Create a new painter object
  beginUnclippedSpawnerPainter();

  // Translate the painter so that 0,0 means where the page starts on the canvas
  unclippedSpawnerPainter->painter()->save();
  unclippedSpawnerPainter->painter()->translate( -m_iXOffset, -m_iYOffset );

  // Assign the painter object to the intra-stencil data object, as well
  // as the zoom factor
  m_dragStencilData.painter = unclippedSpawnerPainter;
  m_dragStencilData.zoomHandler = m_pView->zoomHandler();

  // Draw the outline of the stencil
  m_pDragStencil->paintOutline( &m_dragStencilData );

  unclippedSpawnerPainter->painter()->restore();
}

/**
 * Undraws the old stencil outline, draws the new one
 */
void KivioCanvas::continueSpawnerDragDraw( const QPoint &p )
{
  bool snappedX, snappedY;

  // Translate the painter so that 0,0 means where the page starts on the canvas
  unclippedSpawnerPainter->painter()->save();
  unclippedSpawnerPainter->painter()->translate( -m_iXOffset, -m_iYOffset );

  // Undraw the old outline
  if( oldRectValid )
  {
    m_pDragStencil->paintOutline( &m_dragStencilData );
  }

  // Map the new point from screenspace to page space
  KoPoint orig = mapFromScreen(p);
  KoPoint qp = snapToGrid( orig );

  // First snap to screen
  qp = snapToGrid(qp);
  m_pDragStencil->setPosition( qp.x(), qp.y() );

  // Now snap to the guides
  qp.setCoords(orig.x() + m_pDragStencil->w(), orig.y() + m_pDragStencil->h());
  qp = snapToGuides(qp, snappedX, snappedY);

  if(snappedX) {
    m_pDragStencil->setX(qp.x() - m_pDragStencil->w());
  }

  if(snappedY) {
    m_pDragStencil->setY(qp.y() - m_pDragStencil->h());
  }

  qp.setCoords(orig.x(), orig.y());
  qp = snapToGuides(qp, snappedX, snappedY);

  if(snappedX) {
    m_pDragStencil->setX(qp.x());
  }

  if(snappedY) {
    m_pDragStencil->setY(qp.y());
  }

  // Redraw the new outline
  oldRectValid = true;
  m_pDragStencil->paintOutline( &m_dragStencilData );
  unclippedSpawnerPainter->painter()->restore();

}


/**
 * Ends the ability to draw a drag & drop spawner object
 */
void KivioCanvas::endSpawnerDragDraw()
{
  // Avoid the noid
  if ( !unclippedSpawnerPainter )
    return;

  // If we have a valid old drawing spot, undraw it
  if ( oldRectValid )
  {
    unclippedSpawnerPainter->painter()->save();
    unclippedSpawnerPainter->painter()->translate( -m_iXOffset, -m_iYOffset );
    m_pDragStencil->paintOutline( &m_dragStencilData );
    unclippedSpawnerPainter->painter()->restore();
  }

  // Smack the painter around a bit
  endUnclippedSpawnerPainter();

  // If we have a stencil we were dragging around, delete it.
  if( m_pDragStencil )
  {
    delete m_pDragStencil;
    m_pDragStencil = 0L;
  }

  setFocus();
}


/**
 * Creates a new spawner drawing object
 */
void KivioCanvas::beginUnclippedSpawnerPainter()
{
    // End any previous attempts
    endUnclippedSpawnerPainter();

    // I have no idea what this does.  Max?
    bool unclipped = testWFlags( WPaintUnclipped );
    setWFlags( WPaintUnclipped );


    // Allocate a new painter object for use in drawing
    unclippedSpawnerPainter = new KivioScreenPainter();

    // Tell it to start (allocates a Qpainter object)
    unclippedSpawnerPainter->start(this);

    // Uhhhhh??
    if( !unclipped )
        clearWFlags( WPaintUnclipped );


    // Make sure it's doing NOT drawing.
    unclippedSpawnerPainter->painter()->setRasterOp( NotROP );
    unclippedSpawnerPainter->painter()->setPen( QColor(0,0,250) );

}


/**
 * Deletes the current spawner drawing object
 */
void KivioCanvas::endUnclippedSpawnerPainter()
{
    if( unclippedSpawnerPainter )
    {
        unclippedSpawnerPainter->stop();
        delete unclippedSpawnerPainter;
        unclippedSpawnerPainter = 0L;
    }
}

void KivioCanvas::beginUnclippedPainter()
{
    endUnclippedPainter();
    bool unclipped = testWFlags( WPaintUnclipped );

    setWFlags( WPaintUnclipped );
    unclippedPainter = new QPainter;
    unclippedPainter->begin( this );

    if ( !unclipped )
        clearWFlags( WPaintUnclipped );

    unclippedPainter->setRasterOp( NotROP );
    unclippedPainter->setPen( QPen(blue,1,DotLine) );
}

void KivioCanvas::endUnclippedPainter()
{
    if ( unclippedPainter )
    {
        unclippedPainter->end();
        delete unclippedPainter;
        unclippedPainter = 0;
    }
}

void KivioCanvas::borderTimerTimeout()
{
  QPoint p = mapFromGlobal(QCursor::pos());
  int dx = 0;
  int dy = 0;
  int d = 10;

  QRect r(currRect);
  int vpos = m_pVertScrollBar->value();
  int vmax = m_pVertScrollBar->maxValue();
  int vmin = m_pVertScrollBar->minValue();

  int hpos = m_pHorzScrollBar->value();
  int hmax = m_pHorzScrollBar->maxValue();
  int hmin = m_pHorzScrollBar->minValue();

  if ( p.x() < 0 && hpos > hmin ) {
    dx = QMIN(d,hpos-hmin);
    r.setRight(r.right()+dx);
    rectAnchor.setX(rectAnchor.x()+dx);
  }

  if ( p.y() < 0 && vpos > vmin ) {
    dy = QMIN(d,vpos-vmin);
    r.setBottom(r.bottom()+dy);
    rectAnchor.setY(rectAnchor.y()+dy);
  }

  if ( p.x() > width() && hpos < hmax ) {
    dx = -QMIN(d,hmax-hpos);
    r.setLeft(r.left()+dx);
    rectAnchor.setX(rectAnchor.x()+dx);
  }

  if ( p.y() > height() && vpos < vmax  ) {
    dy = -QMIN(d,vmax-vpos);
    r.setTop(r.top()+dy);
    rectAnchor.setY(rectAnchor.y()+dy);
  }

  if ( dx != 0 || dy != 0 ) {
    unclippedPainter->drawRect( currRect );
    scrollDx(dx);
    scrollDy(dy);
    unclippedPainter->drawRect( r );
    currRect = r;
  }
}


/**
 * Handles the initial drag event
 *
 * @param e The event
 *
 * This will check to make sure the drag object is of the correct mimetype.
 * If it is, it accepts it, and the calls startSpawnerDragDraw which will
 * allocate a new drawing object and set some class variables for future
 * drawing.
 */
void KivioCanvas::dragEnterEvent( QDragEnterEvent *e )
{
    if( e->provides("kivio/stencilSpawner") )
    {
        e->accept();
        startSpawnerDragDraw( e->pos() );
    }
}



/**
 * Handles drag-move events
 *
 * @param e The event
 *
 * This makes sure the drag object is of type kivio/stencilSpawner since these
 * are currently the only type of mime-types accepted.  If so, it accepts the
 * event, and then tells the drawing object to update itself with a new position.
 */
void KivioCanvas::dragMoveEvent( QDragMoveEvent *e )
{
    // Does it speak our language?
    if( e->provides("kivio/stencilSpawner") )
    {
        e->accept();
        continueSpawnerDragDraw( e->pos() );
    }
}


/**
 * Handles drops for this object
 *
 * @param e The drop event object
 *
 * This function takes care of handling the final drop event of this object.  It will
 * allocate a new object of the currently dragged Spawner type (stencil), and add it
 * to the active page of the document (which actually adds it to the active layer
 * of the active page).
 */
void KivioCanvas::dropEvent( QDropEvent *e )
{
    // Terminate the drawing object
    endSpawnerDragDraw();

//    // Get as handle on the icon view
//    KivioIconView *pIconView = (KivioIconView *)m_pView->stackBar()->visiblePage();

//    // Get a pointer to the currently dragged KivioStencilSpawner object
//    KivioStencilSpawner *pSpawner = pIconView->curDragSpawner();
    KivioStencilSpawner *pSpawner = KivioIconView::curDragSpawner();
    if( !pSpawner )
        return;


    // Get a pointer to the current KivioPage
    KivioPage *pPage = activePage();
    if( !pPage )
    {
       kdDebug(43000) << "KivioCanvas::dropEvent() - No active page for stencil to drop on" << endl;
        return;
    }

    // Allocate a new stencil
    KivioStencil *pNewStencil = pSpawner->newStencil();

    // Set the current stencil settings
    QPoint pos = e->pos();
    KoPoint pagePoint = snapToGrid(mapFromScreen( pos ));
    pNewStencil->setX( pagePoint.x() );
    pNewStencil->setY( pagePoint.y() );
    pNewStencil->setW( pSpawner->defWidth() );
    pNewStencil->setH( pSpawner->defHeight() );
    pNewStencil->setTextFont(doc()->defaultFont());

    // Only set these properties if we held ctrl down
    // FIXME: Make this happen!
//    pNewStencil->setFGColor( m_pView->fgColor() );
//    pNewStencil->setBGColor( m_pView->bgColor() );
//    pNewStencil->setLineWidth( (float)m_pView->lineWidth() );


    // Add the new stencil to the page
    pPage->addStencil( pNewStencil );

    pPage->unselectAllStencils();
    pPage->selectStencil( pNewStencil );

    // Select the "selection tool" in case it's not done
    Tool *t = m_pToolsController->findTool("Select");
    if( t )
    {
        m_pToolsController->selectTool(t);
    }

    m_pDoc->updateView(activePage());
}


/**
 * Handles when a drag leaves this object
 *
 * @param e The event object
 *
 * This will call endSpawnerDragDraw() which terminates the drawing
 * object and ends interaction with the drag.
 */
void KivioCanvas::dragLeaveEvent( QDragLeaveEvent * )
{
    endSpawnerDragDraw();
}



void KivioCanvas::drawSelectedStencilsXOR()
{
    // This should never happen, but check just in case
    if ( !unclippedSpawnerPainter )
        return;

    // Translate the painter so that 0,0 means where the page starts on the canvas
    unclippedSpawnerPainter->painter()->save();
    unclippedSpawnerPainter->painter()->translate( -m_iXOffset, -m_iYOffset );

    // Assign the painter object to the intra-stencil data object, as well
    // as the zoom factor
    m_dragStencilData.painter = unclippedSpawnerPainter;
    m_dragStencilData.zoomHandler = m_pView->zoomHandler();

    KivioStencil *pStencil = activePage()->selectedStencils()->first();
    while( pStencil )
    {
        pStencil->paintOutline( &m_dragStencilData );

        pStencil = activePage()->selectedStencils()->next();
    }

    unclippedSpawnerPainter->painter()->restore();
}

void KivioCanvas::drawStencilXOR( KivioStencil *pStencil )
{
    // This should never happen, but check just in case
    if ( !unclippedSpawnerPainter )
        return;

    // Translate the painter so that 0,0 means where the page starts on the canvas
    unclippedSpawnerPainter->painter()->save();
    unclippedSpawnerPainter->painter()->translate( -m_iXOffset, -m_iYOffset );

    // Assign the painter object to the intra-stencil data object, as well
    // as the zoom factor
    m_dragStencilData.painter = unclippedSpawnerPainter;
    m_dragStencilData.zoomHandler = m_pView->zoomHandler();

    pStencil->paintOutline( &m_dragStencilData );

    unclippedSpawnerPainter->painter()->restore();
}

void KivioCanvas::keyReleaseEvent( QKeyEvent *e )
{
    switch( e->key() )
    {
        case Key_Delete: {
            KivioGuideLines* gl = activePage()->guideLines();
            if (gl->hasSelected()) {
              eraseGuides();
              gl->removeSelected();
              paintGuides();
              updateGuidesCursor();
              m_pDoc->setModified( true );
            } else {
              activePage()->deleteSelectedStencils();
              m_pDoc->updateView(activePage());
            }
            break;
        }
        case Key_Escape: {
            m_pToolsController->activateDefault();
        }break;
    }
}

KoPoint KivioCanvas::snapToGridAndGuides(KoPoint point)
{
  KoPoint p = point;

  KoSize dist = m_pDoc->grid().snap;
  KoSize dxy = m_pDoc->grid().freq;

  int dx = qRound(p.x()/dxy.width());
  int dy = qRound(p.y()/dxy.height());

  float distx = QMIN(QABS(p.x() - dxy.width() * dx), QABS(p.x() - dxy.width() * (dx + 1)));
  float disty = QMIN(QABS(p.y() - dxy.height() * dy), QABS(p.y() - dxy.height()* (dy + 1)));

  if( m_pDoc->grid().isSnap)
  {
    if ( distx < dist.width()) {
      if (QABS(p.x() - dxy.width() * dx) < QABS(p.x() - dxy.width() * (dx + 1))) {
        p.rx() = dxy.width() * dx;
      } else {
        p.rx() = dxy.width() * (dx + 1);
      }
    }

    if ( disty < dist.height()) {
      if (QABS(p.y() - dxy.height() * dy) < QABS(p.y() - dxy.height() * (dy + 1))) {
        p.ry() = dxy.height() * dy;
      } else {
        p.ry() = dxy.height() * (dy + 1);
      }
    }
  }

  /*
  * Now if the point is within 4 pixels of a gridline, snap
  * to the grid line.
  */
  if (m_pView->isSnapGuides())
  {
    float four = m_pView->zoomHandler()->unzoomItY(4);
    KivioGuideLines *pGuides = activePage()->guideLines();
    KivioGuideLineData *pData = pGuides->findHorizontal( point.y(), four );

    if( pData )
    {
      p.ry() = (float)pData->position();
    }

    pData = pGuides->findVertical( point.x(), four );

    if( pData )
    {
      p.rx() = (float)pData->position();
    }
  }

  return p;
}

KoPoint KivioCanvas::snapToGrid(KoPoint point)
{
  if (!m_pDoc->grid().isSnap)
    return point;

  KoPoint p = point;

  KoSize dist = m_pDoc->grid().snap;
  KoSize dxy = m_pDoc->grid().freq;

  int dx = qRound(p.x() / dxy.width());
  int dy = qRound(p.y() / dxy.height());

  float distx = QMIN(QABS(p.x() - dxy.width() * dx), QABS(p.x() - dxy.width() * (dx + 1)));
  float disty = QMIN(QABS(p.y() - dxy.height() * dy), QABS(p.y() - dxy.height() * (dy + 1)));

  if(distx < dist.width()) {
    if(QABS(p.x() - dxy.width() * dx) < QABS(p.x() - dxy.width() * (dx + 1))) {
      p.rx() = dxy.width() * dx;
    } else {
      p.rx() = dxy.width() * (dx + 1);
    }
  }

  if(disty < dist.height()) {
    if(QABS(p.y() - dxy.height() * dy) < QABS(p.y() - dxy.height() * (dy + 1))) {
      p.ry() = dxy.height() * dy;
    } else {
      p.ry() = dxy.height() * (dy + 1);
    }
  }

  return p;
}

KoPoint KivioCanvas::snapToGuides(KoPoint point, bool &snappedX, bool &snappedY)
{
  snappedX = false;
  snappedY = false;
  KoPoint p = point;

  if (m_pView->isSnapGuides())
  {
    float four = m_pView->zoomHandler()->unzoomItY(4);
    KivioGuideLines *pGuides = activePage()->guideLines();
    KivioGuideLineData *pData = pGuides->findHorizontal( point.y(), four );

    if(pData)
    {
      snappedY = true;
      p.ry() = pData->position();
    }

    pData = pGuides->findVertical( point.x(), four );

    if(pData)
    {
      snappedX = true;
      p.rx() = pData->position();
    }
  }

  return p;
}

double KivioCanvas::snapToGridX(double z)
{
  KoPoint p(z, 0);
  return snapToGrid(p).x();
}

double KivioCanvas::snapToGridY(double z)
{
  KoPoint p(0, z);
  return snapToGrid(p).y();
}

void KivioCanvas::updateGuides()
{
  eraseGuides();
  paintGuides();
}

void KivioCanvas::guideLinesTimerTimeout()
{
  if (!storedCursor) {
    storedCursor = new QCursor(cursor());
  }

  setCursor(sizeAllCursor);
}

void KivioCanvas::updateGuidesCursor()
{
  if (storedCursor) {
    setCursor(*storedCursor);
    delete storedCursor;
    storedCursor = 0;
  }
}

bool KivioCanvas::eventFilter(QObject* o, QEvent* e)
{
  if ((o == view()->vertRuler() || o == view()->horzRuler()) && (e->type() == QEvent::MouseMove || e->type() ==
    QEvent::MouseButtonRelease) && m_pView->isShowGuides())
  {
    QMouseEvent* me = (QMouseEvent*)e;
    QPoint p = mapFromGlobal(me->globalPos());
    KivioGuideLines* gl = activePage()->guideLines();

    if (e->type() == QEvent::MouseMove) {
      bool f = geometry().contains(p);
      if (!pressGuideline && f && (me->state() == QMouseEvent::LeftButton)) {
        enterEvent(0);

        eraseGuides();
        gl->unselectAll();
        KivioGuideLineData* gd;
        KoPoint tp = mapFromScreen(p);

        if (o == view()->vertRuler()) {
          gd = gl->add(tp.x(),Qt::Vertical);
        } else {
          gd = gl->add(tp.y(),Qt::Horizontal);
        }

        pressGuideline = gd;
        gl->select(gd);
        paintGuides();

        updateGuidesCursor();
        QWidget* w = (QWidget*)o;
        storedCursor = new QCursor(w->cursor());
        w->setCursor(sizeAllCursor);

        lastPoint = p;
      } else if (pressGuideline && !f) {
        leaveEvent(0);

        eraseGuides();
        gl->remove(pressGuideline);
        paintGuides();

        if (storedCursor) {
          QWidget* w = (QWidget*)o;
          w->setCursor(*storedCursor);
          delete storedCursor;
          storedCursor = 0;
        }

        pressGuideline = 0;
      } else if (pressGuideline && f) {
        QMouseEvent* m = new QMouseEvent(QEvent::MouseMove, p, me->globalPos(), me->button(), me->state());
        mouseMoveEvent(m);
        delete m;
        delegateThisEvent = true;
      }
    }

    if (e->type() == QEvent::MouseButtonRelease && pressGuideline) {
      eraseGuides();
      gl->unselect(pressGuideline);
      paintGuides();

      pressGuideline = 0;
      if (storedCursor) {
        QWidget* w = (QWidget*)o;
        w->setCursor(*storedCursor);
        delete storedCursor;
        storedCursor = 0;
      }
      enterEvent(0);
      QMouseEvent* m = new QMouseEvent(QEvent::MouseMove, p, me->globalPos(), NoButton, NoButton);
      mouseMoveEvent(m);
      delete m;
      delegateThisEvent = true;
    }

    if(o == view()->vertRuler()) {
      view()->vertRuler()->update();
    } else {
      view()->horzRuler()->update();
    }
  }

  return QWidget::eventFilter(o, e);
}

void KivioCanvas::eraseGuides()
{
  KivioGuideLines* gl = activePage()->guideLines();
  gl->erase(m_buffer,this);
}

void KivioCanvas::paintGuides(bool show)
{
  if (!m_pView->isShowGuides())
    return;

  KivioGuideLines* gl = activePage()->guideLines();
  gl->paint(m_buffer,this);
  if (show)
    bitBlt(this,0,0,m_buffer);
}

void KivioCanvas::setViewCenterPoint(KivioPoint p)
{
  setUpdatesEnabled(false);

  KivioRect va = visibleArea();

  float x = QMAX(0, p.x() - (va.w() / 2));
  float y = QMAX(0, p.y() - (va.h() / 2));

  m_pVertScrollBar->setValue(m_pView->zoomHandler()->zoomItY(y));
  m_pHorzScrollBar->setValue(m_pView->zoomHandler()->zoomItX(x));

  setUpdatesEnabled(true);
}

KivioRect KivioCanvas::visibleArea()
{
  KoPoint p0 = mapFromScreen(QPoint(0,0));
  KoPoint p1 = mapFromScreen(QPoint(width()-1,height()-1));

  return KivioRect(p0.x(), p0.y(), p1.x() - p0.x(), p1.y() - p0.y());
}

void KivioCanvas::setVisibleArea(KivioRect r, int margin)
{
  setUpdatesEnabled(false);
  KoZoomHandler zoom;
  zoom.setZoomAndResolution(100, QPaintDevice::x11AppDpiX(),
    QPaintDevice::x11AppDpiY());

  float cw = width() - 2 * margin;
  float ch = height() - 2 * margin;

  float zw = cw / (float)zoom.zoomItX(r.w());
  float zh = ch / (float)zoom.zoomItY(r.h());
  float z = QMIN(zw, zh);

  setZoom(qRound(z * 100));

  KivioPoint c = r.center();

  setViewCenterPoint(c);
  setUpdatesEnabled(true);
}

void KivioCanvas::setVisibleAreaByWidth(KivioRect r, int margin)
{
  setUpdatesEnabled(false);
  KoZoomHandler zoom;
  zoom.setZoomAndResolution(100, QPaintDevice::x11AppDpiX(),
    QPaintDevice::x11AppDpiY());

  float cw = width() - 2*margin;
  float z = cw / (float)zoom.zoomItX(r.w());

  setZoom(qRound(z * 100));

  KivioPoint c = r.center();

  setViewCenterPoint(c);
  setUpdatesEnabled(true);
}

void KivioCanvas::setVisibleAreaByHeight(KivioRect r, int margin)
{
  setUpdatesEnabled(false);
  KoZoomHandler zoom;
  zoom.setZoomAndResolution(100, QPaintDevice::x11AppDpiX(),
    QPaintDevice::x11AppDpiY());

  float ch = height() - 2*margin;
  float z = ch / (float)zoom.zoomItY(r.h());

  setZoom(qRound(z * 100));

  KivioPoint c = r.center();

  setViewCenterPoint(c);
  setUpdatesEnabled(true);
}

void KivioCanvas::startPasteMoving()
{
  setEnabled(false);
  KivioPoint p = activePage()->getRectForAllSelectedStencils().center();
  m_origPoint.setCoords(p.x(), p.y());

  // Create a new painter object
  beginUnclippedSpawnerPainter();
  drawSelectedStencilsXOR();

  // Build the list of old geometry
  KivioRect *pData;
  m_lstOldGeometry.clear();
  KivioStencil* pStencil = activePage()->selectedStencils()->first();

  while( pStencil )
  {
    pData = new KivioRect;
    *pData = pStencil->rect();
    m_lstOldGeometry.append(pData);

    pStencil = activePage()->selectedStencils()->next();
  }

  continuePasteMoving(lastPoint);
  m_pasteMoving = true;
  setEnabled(true);
}

void KivioCanvas::continuePasteMoving(const QPoint &pos)
{
  KoPoint pagePoint = mapFromScreen( pos );

  double dx = pagePoint.x() - m_origPoint.x();
  double dy = pagePoint.y() - m_origPoint.y();

  bool snappedX;
  bool snappedY;

  double newX, newY;

  // Undraw the old stencils
  drawSelectedStencilsXOR();

  // Translate to the new position
  KoPoint p;
  KivioRect selectedRect = activePage()->getRectForAllSelectedStencils();

  newX = selectedRect.x() + dx;
  newY = selectedRect.y() + dy;

  // First attempt a snap-to-grid
  p.setCoords(newX, newY);
  p = snapToGrid(p);

  newX = p.x();
  newY = p.y();

  // Now the guides override the grid so we attempt to snap to them
  p.setCoords(selectedRect.x() + dx + selectedRect.w(), selectedRect.y() + dy + selectedRect.h());
  p = snapToGuides(p, snappedX, snappedY);

  if(snappedX) {
    newX = p.x() - selectedRect.w();
  }

  if(snappedY) {
    newY = p.y() - selectedRect.h();
  }

  p.setCoords(selectedRect.x() + dx, selectedRect.y() + dy);
  p = snapToGuides(p, snappedX, snappedY);

  if(snappedX) {
    newX = p.x();
  }

  if(snappedY) {
    newY = p.y();
  }

  dx = newX - selectedRect.x();
  dy = newY - selectedRect.y();

  // Translate to the new position
  KivioStencil *pStencil = activePage()->selectedStencils()->first();
  KivioRect* pData = m_lstOldGeometry.first();
  // bool move = true;

  while( pStencil && pData )
  {
    newX = pData->x() + dx;
    newY = pData->y() + dy;

    if( pStencil->protection()->at( kpX ) == false ) {
      pStencil->setX(newX);
    }
    if( pStencil->protection()->at( kpY ) == false ) {
      pStencil->setY(newY);
    }

    pData = m_lstOldGeometry.next();
    pStencil = activePage()->selectedStencils()->next();
  }

  // Draw the stencils
  drawSelectedStencilsXOR();
  m_pView->updateToolBars();
}

void KivioCanvas::endPasteMoving()
{
  KivioStencil *pStencil = activePage()->selectedStencils()->first();
  KivioRect *pData = m_lstOldGeometry.first();

  while( pStencil && pData )
  {
    if(pStencil->type() == kstConnector) {
      pStencil->searchForConnections(m_pView->activePage(), m_pView->zoomHandler()->unzoomItY(4));
    }

    pData = m_lstOldGeometry.next();
    pStencil = activePage()->selectedStencils()->next();
  }

  drawSelectedStencilsXOR();

  endUnclippedSpawnerPainter();

  // Clear the list of old geometry
  m_lstOldGeometry.clear();
  m_pasteMoving = false;
}

#include "kivio_canvas.moc"
