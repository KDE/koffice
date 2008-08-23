/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_cmyk_colorspace.h"

#include <QDomElement>

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

KisCmykU8ColorSpace::KisCmykU8ColorSpace( KoColorProfile *p) :
 KoLcmsColorSpace<CmykU8Traits>("CMYK", i18n("CMYK (8-bit integer/channel)"),  TYPE_CMYK5_8, icSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint8), KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));
    init();
    
    addCompositeOp( new KoCompositeOpOver<CmykU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<CmykU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpMultiply<CmykU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpDivide<CmykU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpBurn<CmykU8Traits>( this ) );
}

bool KisCmykU8ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) 
        return true;
    else
        return false;
}

KoColorSpace* KisCmykU8ColorSpace::clone() const
{
    return new KisCmykU8ColorSpace( profile()->clone());
}

void KisCmykU8ColorSpace::colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const CmykU8Traits::Pixel* p = reinterpret_cast<const CmykU8Traits::Pixel*>( pixel );
    QDomElement labElt = doc.createElement( "CMYK" );
    labElt.setAttribute("c", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA( p->cyan) );
    labElt.setAttribute("m", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA( p->magenta) );
    labElt.setAttribute("y", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA( p->yellow) );
    labElt.setAttribute("k", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA( p->black) );
    labElt.setAttribute("space", profile()->name() );
    colorElt.appendChild( labElt );
}

void KisCmykU8ColorSpace::colorFromXML( quint8* pixel, const QDomElement& elt) const
{
    CmykU8Traits::Pixel* p = reinterpret_cast<CmykU8Traits::Pixel*>( pixel );
    p->cyan = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("c").toDouble());
    p->magenta = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("m").toDouble());
    p->yellow = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("y").toDouble());
    p->black = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("k").toDouble());
}

