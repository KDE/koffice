/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PERCHANNEL_FILTER_H_
#define _KIS_PERCHANNEL_FILTER_H_

#include <qpair.h>
#include <qptrlist.h>
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_filter_config_widget.h"

class WdgPerChannel;

class KisPerChannelFilterConfiguration
    : public KisFilterConfiguration
{
public:
    KisPerChannelFilterConfiguration(int n);
    ~KisPerChannelFilterConfiguration();

    virtual void fromXML( const QString&  );
    virtual QString toString();
    
public:
    QPtrList<QPair<double,double> > *curves;
    Q_UINT16 *transfers[256];
    Q_UINT16 nTransfers;
    // Caching of adjustment
    bool dirty;
    KisColorSpace* oldCs;
    KisColorAdjustment* adjustment;
};


/**
 * This class is generic for filters that affect channel separately
 */
class KisPerChannelFilter
    : public KisFilter
{
public:
    KisPerChannelFilter() : KisFilter( id(), "adjust", i18n("&Color Adjustment...")) {};
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);
    virtual KisFilterConfiguration* configuration() { return new KisPerChannelFilterConfiguration(0); };
    virtual void process(KisPaintDeviceSP, KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("perchannel", i18n("Color Adjustment")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP dev);

    virtual ColorSpaceIndependence colorSpaceIndependence() { return TO_LAB16; };
private:
};

class KisPerChannelConfigWidget : public KisFilterConfigWidget {

    typedef KisFilterConfigWidget super;
    Q_OBJECT

public:
    KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name = 0, WFlags f = 0 );
    virtual ~KisPerChannelConfigWidget() {};

    KisPerChannelFilterConfiguration * config();
    void setConfiguration(KisFilterConfiguration * config);

private slots:
    virtual void setActiveChannel(int ch);

private:
    WdgPerChannel * m_page;
    KisPaintDeviceSP m_dev;
    KisHistogram *m_histogram;
    QPtrList<QPair<double,double> > *m_curves;
    int m_activeCh;
};

#endif
