/* This file is part of the KDE project
   Copyright (C) 1999 Matthias Kalle Dalheimer <kalle@kde.org>

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


#ifndef __KCCONFIGDIALOG_H__
#define __KCCONFIGDIALOG_H__


#include <q3tabdialog.h>

#include "koChart.h"


class QWidget;

class KDChartParams;


namespace KChart
{

class KChartParams;
class KChartAuxiliary;

class KCConfigDataPage;
class KCConfigSubtypePage;
class KChartHeaderFooterConfigPage;

class KChartParameter3dConfigPage;
class KChartBarslinesConfigPage;
class KChartParameterPieConfigPage;
class KChartPieConfigPage;
class KChartComboPage;
class KChartLine3dConfigPage;
class KChartParameterPolarConfigPage;

class KCConfigLegendPage;
class KCConfigAxesPage;
class KCConfigColorPage;
class KCConfigFontPage;
class KCConfigBackgroundPage;

class KCConfigDialog : public Q3TabDialog
{
    Q_OBJECT

public:
    enum {
      KC_FONT         = 1,
      KC_COLORS       = 2,
      KC_BACK         = 4,
      KC_LEGEND       = 8,
      KC_SUBTYPE      = 16,
      KC_HEADERFOOTER = 32,
      KC_DATAFORMAT   = 64,
      KC_ALL          = 256
    };


    KCConfigDialog( KChartParams* params,
		    QWidget* parent, int flags,
		    KDChartTableData *dat );

    void subtypePage();

signals:
    void dataChanged();

protected:
    KChartParams                     *m_params;

    KCConfigDataPage             *m_dataPage;
    KCConfigSubtypePage          *m_subTypePage;

    KChartParameter3dConfigPage      *_parameter3dpage;
    KChartLine3dConfigPage           *_linepage3d;
    KChartBarslinesConfigPage        *m_barslinesPage;
    KChartParameterPieConfigPage     *_parameterpiepage;
    KChartParameterPolarConfigPage   *_polarpage;

    KChartHeaderFooterConfigPage     *m_headerfooterpage;
    KCConfigLegendPage           *m_legendPage;
    KCConfigAxesPage             *m_axespage;
    KCConfigColorPage            *_colorpage;
    KCConfigFontPage             *_fontpage;
    KCConfigBackgroundPage       *_backgroundpixpage;

    //KChartGeometryConfigPage* _geompage;
    //KChartPieConfigPage*_piepage;
    //KChartComboPage *_hlcChart;

protected slots:
    void         init();
    virtual void apply();
    virtual void defaults();
};

}  //KChart namespace

#endif
