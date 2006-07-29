/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 
#include "kis_grid_drawer.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GL
#include <qgl.h>
#endif

#include "kis_config.h"
#include "kis_image.h"
#include "kis_perspective_grid.h"
#include "kis_perspective_grid_manager.h"

Qt::PenStyle GridDrawer::gs2style(Q_UINT32 s)
{
    switch(s)
    {
        case 1:
            return Qt::DashLine;
        case 2:
            return Qt::DotLine;
        case 3:
            return Qt::DashDotLine;
        case 4:
            return Qt::DashDotDotLine;
        default:
            return Qt::SolidLine;
    }
}

void GridDrawer::drawPerspectiveGrid(KisImageSP image, const QRect& /*wr*/, const KisSubPerspectiveGrid* grid)
{
    KisConfig cfg;
    QPen mainPen = QPen ( cfg.getGridMainColor(), 1, gs2style( cfg.getGridMainStyle() ) );
    QPen subdivisionPen =  QPen ( cfg.getGridSubdivisionColor(), 1, gs2style( cfg.getGridSubdivisionStyle() ) );
    setPen(subdivisionPen );
    // 1 -> top-left corner
    // 2 -> top-right corner
    // 3 -> bottom-right corner
    // 4 -> bottom-left corner
    // d12 line from top-left to top-right
    // note that the notion of top-left is purely theorical
    KisSubPerspectiveGrid::LineEquation d12 = KisSubPerspectiveGrid::computeLineEquation( grid->topLeft(), grid->topRight() ) ;
    KisPoint v12 = KisPoint(*grid->topLeft() - *grid->topRight());
    v12.setX( v12.x() / grid->subdivisions()); v12.setY( v12.y() / grid->subdivisions() );
    KisSubPerspectiveGrid::LineEquation d23 = KisSubPerspectiveGrid::computeLineEquation( grid->topRight(), grid->bottomRight() );
    KisPoint v23 = KisPoint(*grid->topRight() - *grid->bottomRight());
    v23.setX( v23.x() / grid->subdivisions()); v23.setY( v23.y() / grid->subdivisions() );
    KisSubPerspectiveGrid::LineEquation d34 = KisSubPerspectiveGrid::computeLineEquation( grid->bottomRight(), grid->bottomLeft() );
    KisSubPerspectiveGrid::LineEquation d41 = KisSubPerspectiveGrid::computeLineEquation( grid->bottomLeft(), grid->topLeft() );
    
    KisPoint horizVanishingPoint = KisSubPerspectiveGrid::computeIntersection(d12,d34);
    KisPoint vertVanishingPoint = KisSubPerspectiveGrid::computeIntersection(d23,d41);
    
    for(uint i = 1; i < grid->subdivisions(); i ++)
    {
        KisPoint pol1 = *grid->topRight() + i * v12;
        KisSubPerspectiveGrid::LineEquation d1 = KisSubPerspectiveGrid::computeLineEquation( &pol1, &vertVanishingPoint );
        KisPoint pol1b =  KisSubPerspectiveGrid::computeIntersection(d1,d34);
        drawLine( pol1.roundQPoint(), pol1b.roundQPoint() );
        
        KisPoint pol2 = *grid->bottomRight() + i * v23;
        KisSubPerspectiveGrid::LineEquation d2 = KisSubPerspectiveGrid::computeLineEquation( &pol2, &horizVanishingPoint );
        KisPoint pol2b = KisSubPerspectiveGrid::computeIntersection(d2,d41);
        drawLine( pol2.roundQPoint(), pol2b.roundQPoint() );
    }
    setPen(mainPen);
    drawLine( grid->topLeft(), grid->topRight() );
    drawLine( grid->topRight(), grid->bottomRight() );
    drawLine( grid->bottomRight(), grid->bottomLeft() );
    drawLine( grid->bottomLeft(), grid->topLeft() );
}

void GridDrawer::drawGrid(KisImageSP image, const QRect& wr)
{
    KisConfig cfg;
    
    Q_UINT32 offsetx = cfg.getGridOffsetX();
    Q_UINT32 offsety = cfg.getGridOffsetY();
    Q_UINT32 hspacing = cfg.getGridHSpacing();
    Q_UINT32 vspacing = cfg.getGridVSpacing();
    Q_UINT32 subdivision = cfg.getGridSubdivisions() - 1;
    //double ihspsub = hspacing / (double)subdivision;
    //double ivspsub = hspacing / (double)subdivision;

    Q_INT32 imageWidth = image->width();
    Q_INT32 imageHeight = image->height();

    // Draw vertical line
    QPen mainPen = QPen ( cfg.getGridMainColor(), 1, gs2style( cfg.getGridMainStyle() ) );
    QPen subdivisionPen = QPen ( cfg.getGridSubdivisionColor(), 1, gs2style( cfg.getGridSubdivisionStyle() ) );
    Q_UINT32 i = 0;
    for( Q_INT32 x = offsetx; x <= wr.right(); x +=hspacing)
    {
        if( i == subdivision )
        {
            setPen(mainPen);
            i = 0;
        } else {
            setPen(subdivisionPen);
            i++;
        }
        if( x >= wr.x() )
        {
            // Always draw the full line otherwise the line stippling varies
            // with the location of wr and we get glitchy patterns.
            drawLine(x, 0, x, imageHeight);
        }
    }
    // Draw horizontal line
    i = 0;
    for( Q_INT32 y = offsety; y <= wr.bottom(); y +=vspacing)
    {
        if( i == subdivision )
        {
            setPen(mainPen);
            i = 0;
        } else {
            setPen(subdivisionPen);
            i++;
        }
        if( y >= wr.y() )
        {
            drawLine(0, y, imageWidth, y);
        }
    }
}

OpenGLGridDrawer::OpenGLGridDrawer()
{
#ifdef HAVE_GL
    glPushAttrib(GL_ALL_ATTRIB_BITS);
#endif
}

OpenGLGridDrawer::~OpenGLGridDrawer()
{
#ifdef HAVE_GL
    glPopAttrib();
#endif
}

void OpenGLGridDrawer::setPen(const QPen& pen)
{
#ifdef HAVE_GL
    Qt::PenStyle penStyle = pen.style();

    if (penStyle == Qt::SolidLine) {
        glDisable(GL_LINE_STIPPLE);
    } else {
        GLushort lineStipple;

        switch (penStyle) {
        case Qt::NoPen:
            lineStipple = 0;
            break;
        default:
        case Qt::SolidLine:
            lineStipple = 0xffff;
            break;
        case Qt::DashLine:
            lineStipple = 0x3fff;
            break;
        case Qt::DotLine:
            lineStipple = 0x3333;
            break;
        case Qt::DashDotLine:
            lineStipple = 0x33ff;
            break;
        case Qt::DashDotDotLine:
            lineStipple = 0x333f;
            break;
        }

        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, lineStipple);
    }

    QColor penColor = pen.color();

    glColor3ub(penColor.red(), penColor.green(), penColor.blue());
#else
    Q_UNUSED(pen);
#endif
}

void OpenGLGridDrawer::drawLine(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2)
{
#ifdef HAVE_GL
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
#else
    Q_UNUSED(x1);
    Q_UNUSED(y1);
    Q_UNUSED(x2);
    Q_UNUSED(y2);
#endif
}
