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

#ifndef KCHART_VIEW_IFACE_H
#define KCHART_VIEW_IFACE_H

#include <KoViewIface.h>

#include <qstring.h>

class KChartView;

class KChartViewIface : public KoViewIface
{
    K_DCOP
public:
    KChartViewIface( KChartView *view_ );

k_dcop:
    virtual void wizard();
    virtual void editData();
    virtual void configureChart();

    virtual void configureBackground();

    virtual void configureFont();
    virtual void configureColor();
    virtual void configureLegend();


private:
    KChartView *view;

};

#endif
