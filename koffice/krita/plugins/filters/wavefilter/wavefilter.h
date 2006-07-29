/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef WAVEFILTER_H
#define WAVEFILTER_H

#include <kparts/plugin.h>
#include "kis_filter.h"

class KisFilterConfigWidget;

class KritaWaveFilter : public KParts::Plugin
{
public:
    KritaWaveFilter(QObject *parent, const char *name, const QStringList &);
    virtual ~KritaWaveFilter();
};

class KisFilterWave : public KisFilter
{
    public:
        KisFilterWave();
    public:
        virtual void process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration*, const QRect&);
        virtual ColorSpaceIndependence colorSpaceIndependence() { return FULLY_INDEPENDENT; };
        static inline KisID id() { return KisID("wave", i18n("Wave")); };
        virtual bool supportsPainting() { return true; }
        virtual bool supportsPreview() { return true; }
        virtual bool supportsIncrementalPainting() { return false; }
    public:
        virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
        virtual KisFilterConfiguration* configuration(QWidget*);
};

#endif
