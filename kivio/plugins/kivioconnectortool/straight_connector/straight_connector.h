/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
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
#ifndef STRAIGHT_CONNECTOR_H
#define STRAIGHT_CONNECTOR_H


#include "kivio_arrowhead.h"
#include "kivio_connector_point.h"
#include "kivio_custom_drag_data.h"
#include "kivio_intra_stencil_data.h"
#include "kivio_point.h"
#include "kivio_stencil.h"

class KivioPage;

#include <qcolor.h>
#include <qdom.h>

#include "kivio_1d_stencil.h"


class KivioStraightConnector : public Kivio1DStencil
{
protected:
    KivioArrowHead *m_startAH;
    KivioArrowHead *m_endAH;

    bool loadArrowHeads( const QDomElement & );
    QDomElement saveArrowHeads( QDomDocument & );

public:
    KivioStraightConnector();
    virtual ~KivioStraightConnector();

    virtual void setStartPoint( float, float );
    virtual void setEndPoint( float, float );

    virtual KivioCollisionType checkForCollision( KivioPoint *, float );

    virtual KivioStencil* duplicate();

    virtual void paint( KivioIntraStencilData * );
    virtual void paintOutline( KivioIntraStencilData * );

    virtual bool loadCustom( const QDomElement & );
    virtual bool saveCustom( QDomElement &, QDomDocument & );


    virtual void setStartAHType( int i )        { m_startAH->setType(i); }
    virtual void setStartAHWidth( float f )     { m_startAH->setWidth(f); }
    virtual void setStartAHLength( float f )    { m_startAH->setLength(f); }
    virtual void setEndAHType( int i )          { m_endAH->setType(i); }
    virtual void setEndAHWidth( float f )       { m_endAH->setWidth(f); }
    virtual void setEndAHLength( float f )      { m_endAH->setLength(f); }

    virtual int startAHType()                   { return m_startAH->type(); }
    virtual float startAHWidth()                { return m_startAH->width(); }
    virtual float startAHLength()               { return m_startAH->length(); }
    virtual int endAHType()                     { return m_endAH->type(); }
    virtual float endAHWidth()                  { return m_endAH->width(); }
    virtual float endAHLength()                 { return m_endAH->length(); }

};


#endif

