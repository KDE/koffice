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
/**************************************************************************************
 *
 * The code for dragging and resizing stencils is all contained in this class. KivioCanvas
 * is used only for drawing since it's a canvas.
 *
 */

#include "tool_select.h"

#include "kivio_view.h"
#include "kivio_doc.h"
#include "kivio_canvas.h"
#include "kivio_page.h"

#include "kivio_custom_drag_data.h"
#include "kivio_layer.h"
#include "kivio_point.h"
#include "kivio_stencil.h"
#include <X11/Xlib.h>

#include <kaction.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kozoomhandler.h>
#include "kivio_command.h"

SelectTool::SelectTool( KivioView* view )
:Tool(view,"Select")
{
  setSortNum(0);
  controller()->setDefaultTool(this);

  ToolSelectAction* select = new ToolSelectAction( actionCollection(), "ToolAction" );
  KAction* m_z1 = new KAction( i18n("Select"), "kivio_arrow", Key_Space, actionCollection(), "select" );
  select->insert(m_z1);

  m_mode = stmNone;
  m_pResizingStencil = NULL;
  m_pCustomDraggingStencil = NULL;

  m_lstOldGeometry.setAutoDelete(true);

  m_customDragID = 0;

  m_pMenu = new KActionMenu( i18n("Select Tool Menu"), this, "selectToolMenu" );

  buildPopupMenu();

}

SelectTool::~SelectTool()
{
}


/**
 * Event delegation
 *
 * @param e The event to be identified and processed
 *
 */
void SelectTool::processEvent( QEvent* e )
{
    QMouseEvent *m;

    switch (e->type())
    {
    case QEvent::MouseButtonDblClick:
        m = (QMouseEvent *)e;
        if( m->button() == LeftButton )
            leftDoubleClick(m->pos());
        break;
    case QEvent::MouseButtonPress:
        m = (QMouseEvent *)e;
        if( m->button() == RightButton )
            showPopupMenu(m->globalPos());
        else if( m->button() == LeftButton )
            mousePress( m->pos() );
        break;

    case QEvent::MouseButtonRelease:
        mouseRelease( ((QMouseEvent *)e)->pos() );
        break;

    case QEvent::MouseMove:
        mouseMove( ((QMouseEvent *)e)->pos() );
        break;

    default:
      break;
    }
}

void SelectTool::activate()
{
   kdDebug() << "SelectTool activate" << endl;
    m_pCanvas->setCursor(arrowCursor);
    m_mode = stmNone;
}

void SelectTool::deactivate()
{
}

void SelectTool::configure()
{
}


/**
 * Selects all stencils inside a given rect
 */
void SelectTool::select(const QRect &r)
{
    // Calculate the start and end clicks in terms of page coordinates
    KoPoint startPoint = m_pCanvas->mapFromScreen( QPoint( r.x(), r.y() ) );
    KoPoint releasePoint = m_pCanvas->mapFromScreen( QPoint( r.x() + r.width(), r.y() + r.height() ) );


    double x, y, w, h;

    // Calculate the x,y position of the selection box
    x = startPoint.x() < releasePoint.x() ? startPoint.x() : releasePoint.x();
    y = startPoint.y() < releasePoint.y() ? startPoint.y() : releasePoint.y();

    // Calculate the w/h of the selection box
    w = releasePoint.x() - startPoint.x();
    
    if( w < 0.0 ) {
        w *= -1.0;
    }

    h = releasePoint.y() - startPoint.y();
    
    if( h < 0.0 ) {
        h *= -1.0;
    }

    // Tell the page to select all stencils in this box
    m_pView->activePage()->selectStencils( x, y, w, h );
}

void SelectTool::mousePress(const QPoint &pos)
{
    // Gets the list of keys held down and check if the shift key is one of them. If yes, set the flag
    XQueryKeymap( qt_xdisplay(), m_keys );
    if( !(m_keys[6] & 4 ) && !(m_keys[7] & 64) )
        m_shiftKey = false;
    else
        m_shiftKey = true;

    // Last point is used for undrawing at the last position and calculating the distance the mouse has moved
    m_lastPoint = m_pCanvas->mapFromScreen(pos);
    m_origPoint = m_lastPoint;

    // Check if we nailed a custom drag point on a selected stencil
    if( startCustomDragging(pos, true) )
    {
        m_mode = stmCustomDragging;
        return;
    }

    // Check if we are resizing
    if( startResizing(pos) )
    {
        m_mode = stmResizing;
        return;
    }


    // Check if we nailed a custom drag point on any other stencil
    if( startCustomDragging(pos, false) )
    {
       m_mode = stmCustomDragging;
       return;
    }

    // Check if we can drag a stencil (only the selected stencils first)
    if( startDragging(pos, true) )
    {
        m_mode = stmDragging;
        return;
    }

    // Check if we can drag a stencil
    if( startDragging(pos, false) )
    {
        m_mode = stmDragging;
        return;
    }

    // This should always be the last 'start' call since it always returns true
    if( startRubberBanding(pos) )
    {
        m_mode = stmDrawRubber;
        return;
    }
}


/**
 * Tests if we should start rubber banding (always returns true).
 */
bool SelectTool::startRubberBanding(const QPoint &pos)
{
    // We didn't find a stencil, so unselect everything if we aren't holding the shift key down
    if( !m_shiftKey )
        m_pCanvas->activePage()->unselectAllStencils();

    m_pCanvas->startRectDraw( pos, KivioCanvas::Rubber );
    m_pCanvas->repaint();

    return true;
}


/**
 * Tests if we can start dragging a stencil.
 */
bool SelectTool::startDragging(const QPoint &pos, bool onlySelected)
{
    KivioPage *pPage = m_pCanvas->activePage();
    KivioPoint kPoint;
    KivioStencil *pStencil;
    int colType;

    // Figure out how big 4 pixels is in terms of points
    double threshhold =  m_pView->zoomHandler()->unzoomItY(4);

    KoPoint pagePoint = m_pCanvas->mapFromScreen( pos );

    kPoint.set( pagePoint.x(), pagePoint.y() );

    pStencil = pPage->checkForStencil( &kPoint, &colType, threshhold, onlySelected );

    if( !pStencil )
        return false;

    if( pStencil->isSelected() )
    {
        // If we are clicking an already selected stencil, and the shift
        // key down, then unselect this stencil
        if( m_shiftKey==true )
            pPage->unselectStencil( pStencil );

        // Otherwise, it means we are just moving
    }
    else
    {
        // Clicking a new stencil, and the shift key is not down
        if( !m_shiftKey )
            pPage->unselectAllStencils();

        pPage->selectStencil( pStencil );
    }

    // Set the mode
    m_mode = stmDragging;

    // Create a new painter object
    m_pCanvas->beginUnclippedSpawnerPainter();
    m_pCanvas->drawSelectedStencilsXOR();

    // Tell the view to update the toolbars to reflect the
    // first selected stencil's settings
    m_pView->updateToolBars();


    // Build the list of old geometry
    KivioSelectDragData *pData;
    m_lstOldGeometry.clear();
    pStencil = m_pCanvas->activePage()->selectedStencils()->first();
    while( pStencil )
    {
        pData = new KivioSelectDragData;
        pData->rect = pStencil->rect();
        m_lstOldGeometry.append(pData);


        pStencil = m_pCanvas->activePage()->selectedStencils()->next();
    }


    changeMouseCursor(pos);

    return true;
}

bool SelectTool::startCustomDragging(const QPoint &pos, bool selectedOnly )
{
    KivioPage *pPage = m_pCanvas->activePage();
    KivioPoint kPoint;
    KivioStencil *pStencil;
    int colType;

    KoPoint pagePoint = m_pCanvas->mapFromScreen( pos );

    kPoint.set( pagePoint.x(), pagePoint.y() );

    pStencil = pPage->checkForStencil( &kPoint, &colType, 0.0, selectedOnly );

    if( !pStencil || colType < kctCustom )
        return false;


    if( pStencil->isSelected() )
    {
        // If we are clicking an already selected stencil, and the shift
        // key down, then unselect this stencil
        if( m_shiftKey==true )
        {
            m_pCustomDraggingStencil = NULL;
            pPage->unselectStencil( pStencil );
        }
        else
            m_pCustomDraggingStencil = pStencil;

        // Otherwise, it means we are just moving
    }
    else
    {
        // Clicking a new stencil, and the shift key is not down
        if( !m_shiftKey )
            pPage->unselectAllStencils();

        m_pCustomDraggingStencil = pStencil;

        pPage->selectStencil( pStencil );
    }

    // Set the mode
    m_mode = stmCustomDragging;

    m_customDragID = colType;

    // Create a new painter object
    m_pCanvas->beginUnclippedSpawnerPainter();
    m_pCanvas->drawSelectedStencilsXOR();

    return true;
}

/**
 * Tests if we can start resizing a stencil
 */
bool SelectTool::startResizing(const QPoint &pos)
{
    KoPoint pagePoint = m_pCanvas->mapFromScreen(pos);
    KivioSelectDragData *pData;

    double x = pagePoint.x();
    double y = pagePoint.y();

    // Search selected stencils to see if we have a resizing point
    KivioStencil *pStencil = m_pCanvas->activePage()->selectedStencils()->first();
    while( pStencil )
    {
        m_resizeHandle = isOverResizeHandle(pStencil, x, y);
        if( m_resizeHandle > 0 )
        {
            switch( m_resizeHandle )
            {
                case 1: // top left
                    m_origPoint.setCoords(pStencil->x(), pStencil->y());
                    break;

                case 2:
                    m_origPoint.setCoords((pStencil->x() + pStencil->w()) / 2.0, pStencil->y());
                    break;

                case 3:
                    m_origPoint.setCoords(pStencil->x() + pStencil->w(), pStencil->y());
                    break;

                case 4:
                    m_origPoint.setCoords(pStencil->x() + pStencil->w(), (pStencil->y() + pStencil->h()) / 2.0);
                    break;

                case 5:
                    m_origPoint.setCoords(pStencil->x() + pStencil->w(), pStencil->y() + pStencil->h());
                    break;

                case 6:
                    m_origPoint.setCoords((pStencil->x() + pStencil->w()) / 2.0, pStencil->y() + pStencil->h());
                    break;

                case 7:
                    m_origPoint.setCoords(pStencil->x(), pStencil->y() + pStencil->h());
                    break;

                case 8:
                    m_origPoint.setCoords(pStencil->x(), (pStencil->y() + pStencil->h()) / 2.0);
                    break;
            }

            m_lstOldGeometry.clear();
            pData = new KivioSelectDragData;
            pData->rect = pStencil->rect();
            m_lstOldGeometry.append(pData);

            m_pResizingStencil = pStencil;
//            m_pCanvas->startSelectedStencilResize( pStencil );

            // Create a new painter object
            m_pCanvas->beginUnclippedSpawnerPainter();
            m_pCanvas->drawStencilXOR( pStencil );

            return true;
        }

        pStencil = m_pCanvas->activePage()->selectedStencils()->next();
    }

    return false;
}



void SelectTool::mouseMove(const QPoint &pos)
{
    switch( m_mode )
    {
        case stmDrawRubber:
            continueRubberBanding(pos);
            break;

        case stmDragging:
            continueDragging(pos);
            break;

        case stmCustomDragging:
            continueCustomDragging(pos);
            break;

        case stmResizing:
            continueResizing(pos);
            break;

        default:
            changeMouseCursor(pos);
            break;
    }

    m_lastPoint = m_pCanvas->mapFromScreen(pos);
}

void SelectTool::continueRubberBanding(const QPoint &pos)
{
    m_pCanvas->continueRectDraw( pos, KivioCanvas::Rubber );
}


/**
 * Continues the dragging process of a stencil (moving)
 *
 * How does this work?  Initially we create a list of all the original
 * geometry of all the selected stencils.  We use that to calculate delta
 * movements and snap them to the grid.
 */
void SelectTool::continueDragging(const QPoint &pos)
{
    KoPoint pagePoint = m_pCanvas->mapFromScreen( pos );

    double dx = pagePoint.x() - m_origPoint.x();
    double dy = pagePoint.y() - m_origPoint.y();

    bool snappedX;
    bool snappedY;

    double newX, newY;

    // Undraw the old stencils
    m_pCanvas->drawSelectedStencilsXOR();


    // Translate to the new position
    KivioSelectDragData *pData;
    KivioStencil *pStencil = m_pCanvas->activePage()->selectedStencils()->first();
    pData = m_lstOldGeometry.first();

    while( pStencil && pData )
    {
        KoPoint p;

        // First attempt a snap-to-grid
        p.setCoords(pData->rect.x() + dx, pData->rect.y() + dy);
        p = m_pCanvas->snapToGrid(p);

        newX = p.x();
        newY = p.y();

        // Now the guides override the grid so we attempt to snap to them
        p.setCoords(pData->rect.x() + dx + pStencil->w(), pData->rect.y() + dy + pStencil->h());
        p = m_pCanvas->snapToGuides(p, snappedX, snappedY);
        
        if(snappedX) {
          newX = p.x() - pStencil->w();
        }
        
        if(snappedY) {
          newY = p.y() - pStencil->h();
        }

        p.setCoords(pData->rect.x() + dx, pData->rect.y() + dy);
        p = m_pCanvas->snapToGuides(p, snappedX, snappedY);
        
        if(snappedX) {
          newX = p.x();
        }
        
        if(snappedY) {
          newY = p.y();
        }

        if( pStencil->protection()->at( kpX )==false ) {
          pStencil->setX(newX);
        }
        if( pStencil->protection()->at( kpY )==false ) {
          pStencil->setY(newY);
        }

        pData = m_lstOldGeometry.next();
        pStencil = m_pCanvas->activePage()->selectedStencils()->next();
    }

    // Draw the stencils
    m_pCanvas->drawSelectedStencilsXOR();
    m_pView->updateToolBars();
}

void SelectTool::continueCustomDragging(const QPoint &pos)
{
    KoPoint pagePoint = m_pCanvas->mapFromScreen(pos);
    pagePoint = m_pCanvas->snapToGrid(pagePoint);

    KivioCustomDragData data;
    data.page = m_pCanvas->activePage();
    data.dx = pagePoint.x() - m_lastPoint.x();
    data.dy = pagePoint.y() - m_lastPoint.y();
    data.x = pagePoint.x();
    data.y = pagePoint.y();
    data.id = m_customDragID;
    data.scale = m_pView->zoomHandler()->zoomedResolutionY();


    // Undraw the old stencils
    m_pCanvas->drawSelectedStencilsXOR();


    // Custom dragging can only occur on one stencil
    if( m_pCustomDraggingStencil )
        m_pCustomDraggingStencil->customDrag( &data );

    // Draw the stencils
    m_pCanvas->drawSelectedStencilsXOR();
    m_pView->updateToolBars();
}


void SelectTool::continueResizing(const QPoint &pos)
{
    KoPoint pagePoint = m_pCanvas->snapToGridAndGuides( m_pCanvas->mapFromScreen(pos) );
    KivioSelectDragData *pData = m_lstOldGeometry.first();

    if( !pData )
    {
       kdDebug() << "SelectTool::continueResizing() - Original geometry not found" << endl;
        return;
    }

    double dx = pagePoint.x() - m_origPoint.x();
    double dy = pagePoint.y() - m_origPoint.y();


    // Undraw the old outline
    m_pCanvas->drawStencilXOR( m_pResizingStencil );

    double sx = pData->rect.x();
    double sy = pData->rect.y();
    double sw = pData->rect.w();
    double sh = pData->rect.h();

    switch( m_resizeHandle )
    {
        case 1: // top left
            if( m_pResizingStencil->protection()->testBit( kpWidth )==false &&
              m_pResizingStencil->protection()->testBit( kpHeight )==false )
            {
                m_pResizingStencil->setX( sx + dx );
                m_pResizingStencil->setW( sw - dx );

                m_pResizingStencil->setY( sy + dy );
                m_pResizingStencil->setH( sh - dy );
            }
            break;

        case 2: // top
            if( m_pResizingStencil->protection()->testBit( kpHeight )==false )
            {
                m_pResizingStencil->setY( sy + dy );
                m_pResizingStencil->setH( sh - dy );
            }
            break;

        case 3: // top right
            if( m_pResizingStencil->protection()->testBit( kpHeight )==false &&
              m_pResizingStencil->protection()->testBit( kpWidth )==false )
            {
               m_pResizingStencil->setY( sy + dy );
               m_pResizingStencil->setH( sh - dy );

               m_pResizingStencil->setW( sw + dx );
            }
            break;

        case 4: // right
            if( m_pResizingStencil->protection()->testBit( kpWidth )==false )
            {
                // see old kivio source when snaptogrid gets implemented
                //setX( SnapToGrid(sx+sw+dx)-sx )
                m_pResizingStencil->setW( sw + dx );
            }
            break;

        case 5: // bottom right
            if( m_pResizingStencil->protection()->testBit( kpWidth )==false &&
              m_pResizingStencil->protection()->testBit( kpHeight )==false )
            {
                m_pResizingStencil->setW( sw+dx );
                m_pResizingStencil->setH( sh + dy );
            }
            break;

        case 6: // bottom
            if( m_pResizingStencil->protection()->testBit( kpHeight )==false )
            {
                m_pResizingStencil->setH( sh + dy );
            }
            break;

        case 7: // bottom left
            if( m_pResizingStencil->protection()->testBit( kpWidth )==false &&
              m_pResizingStencil->protection()->testBit( kpHeight )==false )
            {
              m_pResizingStencil->setX( sx + dx );
              m_pResizingStencil->setW( sw - dx );

              m_pResizingStencil->setH( sh + dy );
            }
            break;

        case 8: // left
            if( m_pResizingStencil->protection()->testBit( kpWidth )==false )
            {
                m_pResizingStencil->setX( sx + dx );
                m_pResizingStencil->setW( sw - dx );
            }
            break;

        default:
            kdDebug() << "SelectTool::continueResizing() - unknown resize handle: " <<  m_resizeHandle << endl;
            break;
    }

    m_pCanvas->drawStencilXOR( m_pResizingStencil );
    m_pView->updateToolBars();
}


/**
 * Change the mouse cursor based on what it is over.
 */
void SelectTool::changeMouseCursor(const QPoint &pos)
{
    KoPoint pagePoint = m_pCanvas->mapFromScreen(pos);
    KivioStencil *pStencil;
    KivioPoint col;
    double threshhold = m_pView->zoomHandler()->unzoomItY(4);
    int cursorType;

    double x = pagePoint.x();
    double y = pagePoint.y();

    col.set(x, y);

    // Iterate through all the selected stencils
    pStencil = m_pCanvas->activePage()->selectedStencils()->first();
    while( pStencil )
    {
        cursorType = isOverResizeHandle(pStencil, x, y);
        switch( cursorType )
        {
            case 1: // top left
                m_pCanvas->setCursor( sizeFDiagCursor );
                return;

            case 2: // top
                m_pCanvas->setCursor( sizeVerCursor );
                return;

            case 3: // top right
                m_pCanvas->setCursor( sizeBDiagCursor );
                return;

            case 4: // right
                m_pCanvas->setCursor( sizeHorCursor );
                return;

            case 5: // bottom right
                m_pCanvas->setCursor( sizeFDiagCursor );
                return;

            case 6: // bottom
                m_pCanvas->setCursor( sizeVerCursor );
                return;

            case 7: // bottom left
                m_pCanvas->setCursor( sizeBDiagCursor );
                return;

            case 8: // left
                m_pCanvas->setCursor( sizeHorCursor );
                return;

            default:
                if( pStencil->checkForCollision( &col, threshhold )!= kctNone )
                {
                    m_pCanvas->setCursor( sizeAllCursor );
                    return;
                }
                break;

        }


        pStencil = m_pCanvas->activePage()->selectedStencils()->next();
    }

    m_pCanvas->setCursor( arrowCursor );
}


/**
 * Tests a box for a point.  Used only by isOverResizeHandle().
 */
#define RESIZE_BOX_TEST( x, y, bx, by ) \
x >= bx-three_pixels && \
x <= bx+three_pixels && \
y >= by-three_pixels && \
y <= by+three_pixels

/**
 * Tests if a point is over a stencils
 */
int SelectTool::isOverResizeHandle( KivioStencil *pStencil, const double x, const double y )
{
    double three_pixels = 4.0;
    double newX, newY, newW, newH;

    int available;

    newX = pStencil->x();
    newY = pStencil->y();
    newW = pStencil->w();
    newH = pStencil->h();

    available = pStencil->resizeHandlePositions();

    // Quick reject
    if( !available )
        return 0;


    // Top left
    if( available & krhpNW &&
        RESIZE_BOX_TEST( x, y, newX, newY ) )
        return 1;

    // Top
    if( available & krhpN &&
        RESIZE_BOX_TEST( x, y, newX+newW/2, newY ) )
        return 2;

    // Top right
    if( available & krhpNE &&
        RESIZE_BOX_TEST( x, y, newX+newW, newY  ) )
        return 3;

    // Right
    if( available & krhpE &&
        RESIZE_BOX_TEST( x, y, newX+newW, newY+newH/2 ) )
        return 4;

    // Bottom right
    if( available & krhpSE &&
        RESIZE_BOX_TEST( x, y, newX+newW, newY+newH ) )
        return 5;

    // Bottom
    if( available & krhpS &&
        RESIZE_BOX_TEST( x, y, newX+newW/2, newY+newH ) )
        return 6;

    // Bottom left
    if( available & krhpSW &&
        RESIZE_BOX_TEST( x, y, newX, newY+newH ) )
        return 7;

    // Left
    if( available & krhpW &&
        RESIZE_BOX_TEST( x, y, newX, newY+newH/2 ) )
        return 8;

    // Nothing found
    return 0;
}


void SelectTool::mouseRelease(const QPoint &pos)
{
    m_releasePoint = pos;

    switch( m_mode )
    {
        case stmDrawRubber:
            endRubberBanding(pos);
            break;

        case stmCustomDragging:
            endCustomDragging(pos);
            break;

        case stmDragging:
            endDragging(pos);
            break;

        case stmResizing:
            endResizing(pos);
            break;
    }

  	m_mode = stmNone;

    m_pView->doc()->updateView(m_pView->activePage());
}

void SelectTool::endRubberBanding(const QPoint &pos)
{
   // End the rubber-band drawing
   m_pCanvas->endRectDraw();

   KoPoint p = m_pCanvas->mapFromScreen(pos);

    // We can't select if the start and end points are the same
//    if( m_startPoint != m_releasePoint )
    if( m_origPoint.x() != p.x() && m_origPoint.y() != p.y() )
    {
        select(m_pCanvas->rect());
    }

    m_pView->updateToolBars();
}

void SelectTool::endDragging(const QPoint&)
{
    KMacroCommand *macro=new KMacroCommand( i18n("Move Stencil"));
    KivioStencil *pStencil = m_pCanvas->activePage()->selectedStencils()->first();
    KivioSelectDragData *pData = m_lstOldGeometry.first();

    while( pStencil && pData )
    {
        KivioMoveStencilCommand * cmd = new KivioMoveStencilCommand( i18n("Move Stencil"), pStencil, pData->rect, pStencil->rect(), m_pCanvas->activePage());
        macro->addCommand( cmd);
        pData = m_lstOldGeometry.next();
        pStencil = m_pCanvas->activePage()->selectedStencils()->next();

    }

    m_pCanvas->doc()->addCommand( macro );

    m_pCanvas->drawSelectedStencilsXOR();

    m_pCanvas->endUnclippedSpawnerPainter();

    // Clear the list of old geometry
    m_lstOldGeometry.clear();
}

void SelectTool::endCustomDragging(const QPoint&)
{
    m_customDragID = 0;
    m_pCanvas->drawSelectedStencilsXOR();

    m_pCanvas->endUnclippedSpawnerPainter();
}

void SelectTool::endResizing(const QPoint&)
{
    KivioResizeStencilCommand * cmd = new KivioResizeStencilCommand( i18n("Resize Stencil"), m_pResizingStencil, m_lstOldGeometry.first()->rect, m_pResizingStencil->rect(), m_pView->activePage());
    m_pCanvas->doc()->addCommand( cmd );
    // Undraw the last outline
    m_pCanvas->drawStencilXOR( m_pResizingStencil );

    // Deallocate the painter object
    m_pCanvas->endUnclippedSpawnerPainter();

    // Set the class vars to nothing
    m_pResizingStencil = NULL;
    m_resizeHandle = 0;
}


/**
 * Builds the popup menu for this tool.
 */
void SelectTool::buildPopupMenu()
{
  // Add existing actions
  m_pMenu->insert( new KAction( i18n("Cut"), "editcut", 0, m_pView, SLOT(cutStencil()), actionCollection(), "cutStencil" ) );
  m_pMenu->insert( new KAction( i18n("Copy"), "editcopy", 0, m_pView, SLOT(copyStencil()), actionCollection(), "copyStencil" ) );
  m_pMenu->insert( new KAction( i18n("Paste"), "editpaste", 0, m_pView, SLOT(pasteStencil()), actionCollection(), "pasteStencil" ) );

  m_pMenu->popupMenu()->insertSeparator();

  m_pMenu->insert( new KAction( i18n("Group Selected Stencils"), "group_stencils", 0, m_pView, SLOT(groupStencils()), actionCollection(), "groupStencils" ) );
  m_pMenu->insert( new KAction( i18n("Ungroup Selected Stencils"), "ungroup_stencils", 0, m_pView, SLOT(ungroupStencils()), actionCollection(), "ungroupStencils" ) );

  m_pMenu->popupMenu()->insertSeparator();

  m_pMenu->insert( new KAction( i18n("Bring to Front"), "bring_stencil_to_front", 0, m_pView, SLOT(bringStencilToFront()), actionCollection(), "bringStencilToFront" ) );
  m_pMenu->insert( new KAction( i18n("Send to Back"), "send_stencil_to_back", 0, m_pView, SLOT(sendStencilToBack()), actionCollection(), "sendStencilToBack" ) );

}


/**
 * Shows the popupmenu at a given point.
 */
void SelectTool::showPopupMenu( const QPoint &pos )
{
    m_pMenu->popup( pos );
}


/**
 * Handles what happens when a left-button double click occurs.
 *
 * If there are no stencils selected, this function returns.  Otherwise
 * it launches the text tool on the selected stencils and switches back
 * to this tool when it's done.
 */
void SelectTool::leftDoubleClick( const QPoint &/*p*/ )
{
    if( m_pView->activePage()->selectedStencils()->count() <= 0 )
        return;

    // Locate the text tool.  If not found, bail with an error
    Tool *t = controller()->findTool("Text");
    if( !t )
    {
       kdDebug() << "SelectTool::leftDoubleClick() - unable to locate Text Tool" << endl;
        return;
    }

    // Select the text tool (which makes the text dialog pop up)
    controller()->selectTool(t);

    // Reselect this tool so we are back in selection mode
    controller()->selectTool(this);
}
#include "tool_select.moc"
