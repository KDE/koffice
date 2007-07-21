/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoRgbU16ColorSpace.h"

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

KoRgbU16ColorSpace::KoRgbU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
 KoLcmsColorSpace<KoRgbU16Traits>(colorSpaceId(), i18n("RGB 16-bit integer/channel)"), parent, TYPE_BGRA_16, icSigRgbData, p)
{
    addChannel(new KoChannelInfo(i18n("Red"), 2* sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, 4, QColor(255,0,0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1* sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, 4, QColor(0,255,0)));
    addChannel(new KoChannelInfo(i18n("Blue"), 0* sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, 4, QColor(0,0,255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3* sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16));
    init();
    
    addStandardCompositeOps<KoRgbU16Traits>(this);
}

bool KoRgbU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) 
        return true;
    else
        return false;
}

QString KoRgbU16ColorSpace::colorSpaceId()
{
    return QString("RGBA16");
}

