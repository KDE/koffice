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
#ifndef KIVIO_ARROWHEAD_H
#define KIVIO_ARROWHEAD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qcolor.h>
#include <qdom.h>

#define KIVIO_CUT_LENGTH         -1.0f
#define KIVIO_CUT_HALF_LENGTH    -2.0f

class KivioPainter;
class KoZoomHandler;

typedef enum {
    kahtNone=0,
    kahtArrowLine,
    kahtArrowTriangleSolid,
    kahtArrowTriangleHollow,
    kahtDoubleTriangleSolid,
    kahtDoubleTriangleHollow,
    kahtForwardSlash,                   //   -----/
    kahtBackSlash,
    kahtPipe                            //   -----|
/*    kahtArrowTriangleConcaveSolid,
    kahtForwardSlash,                   //   -----/
    kahtBackSlash,
    kahtPipe,                           //   -----|
    kahtCircleSolid,
    kahtRectangleSolid,
    kahtDiamondSolid,
    kahtMidForwardSlash,                //   -----/--
    kahtMidBackSlash,
    kahtMidPipe,                        //   -----|--

    kahtCrowFoot,
    kahtCrowFootPipe,
    kahtCrowFootCircleHollow,
    kahtCrowFootCircleSolid,

    kahtFemaleSignHollow,                //   ------o|-
    kahtPipeCircleHollow,                //   -------|o
    kaht2PipeCircleHollow,               //   ------||o
    kaht3PipeCircleHollow,               //   -----|||o
    kahtDiamondCircleHollow,             //   -----<>o
    kahtDoubleTriangleHollow,            //   -----|>|>

    kahtPipeFemaleSignSolid,             //   ------o|-
    kahtPipeCircleSolid,                 //   -------|o
    kaht2PipeCircleSolid,                //   ------||o
    kaht3PipeCircleSolid,                //   -----|||o
    kahtDiamondCircleSolid,              //   -----<>o
    kahtDoubleTriangleSolid              //   -----|>|>*/
} KivioArrowHeadType;


typedef struct KivioArrowHeadData KivioArrowHeadData;
struct KivioArrowHeadData
{
    float x, y;
    float vecX, vecY;
    KoZoomHandler* zoomHandler;

    KivioPainter* painter;
};


class KivioArrowHead
{
  protected:
    /**
     * The cut is the distance 'into' the arrowhead the line should continue
     */
    float m_cut;

    /**
     * The width/length of the arrowhead
     */
    float m_w, m_l;

    /**
     * The type of arrow
     */
    int m_type;


    void paintArrowLine( KivioArrowHeadData * );
    void paintArrowTriangle( KivioArrowHeadData *, bool );
    void paintDoubleTriangle( KivioArrowHeadData *, bool );
    void paintForwardSlash( KivioArrowHeadData * );
    void paintBackSlash( KivioArrowHeadData * );
    void paintPipe( KivioArrowHeadData * );

  public:
    KivioArrowHead();
    virtual ~KivioArrowHead();

    void setType( int );
    inline int type() { return m_type; }

    float cut();

    void setWidth( float f ) { m_w = f; }
    void setLength( float f ) { m_l = f; }

    void setSize( float f1, float f2 ) { m_w=f1; m_l=f2; }

    inline float width() { return m_w; }
    inline float length() { return m_l; }

    void paint( KivioPainter *, float, float, float, float, KoZoomHandler* zoomHandler );

    bool loadXML( const QDomElement & );
    QDomElement saveXML( QDomDocument & );
};

#endif

