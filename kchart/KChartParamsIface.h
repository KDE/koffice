/* This file is part of the KDE project
   Copyright (C) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef KCHART_PARAMS_IFACE_H
#define KCHART_PARAMS_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <qstring.h>

class KChartParams;

class KChartParamsIface : virtual public DCOPObject
{
    K_DCOP
public:
    KChartParamsIface( KChartParams *_params );

k_dcop:
    virtual void setChartType( /*ChartType chartType*/ );
    
    //bar chart config
    virtual bool threeDBars();
    virtual void setThreeDBars( bool threeDBars );
    virtual void setThreeDBarsShadowColors( bool shadow );
    virtual bool threeDBarsShadowColors() const;
    virtual void setThreeDBarAngle( uint angle );
    virtual uint threeDBarAngle() const;
    virtual void setThreeDBarDepth( double depth );
    virtual double threeDBarDepth() const;
    
    
    //pie config
    virtual bool threeDPies();
    virtual void setThreeDPieHeight( int pixels );
    virtual int threeDPieHeight();
    virtual void setPieStart( int degrees );
    virtual int pieStart();

    //legend
    virtual void hideLegend();
    virtual void setLegendPosition(const QString &);
    virtual QString legendPostion() const;
    virtual void setLegendTitleText( const QString& text );
    virtual void setLegendSpacing( uint space );
    virtual uint legendSpacing();

private:
    KChartParams *params;

};

#endif
