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

#include "KoLabColorSpace.h"

#include <QDomElement>

#include <klocale.h>

#include "../compositeops/KoCompositeOpOver.h"
#include "../compositeops/KoCompositeOpErase.h"
#include "../compositeops/KoCompositeOpMultiply.h"
#include "../compositeops/KoCompositeOpDivide.h"
#include "../compositeops/KoCompositeOpBurn.h"

KoLabColorSpace::KoLabColorSpace( KoColorProfile *p) :
 KoLcmsColorSpace<KoLabU16Traits>(colorSpaceId(), i18n("L*a*b* (16-bit integer/channel)"),  COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1), icSigLabData, p)
{
    addChannel(new KoChannelInfo(i18n("Lightness"), CHANNEL_L * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(100,100,100)));
    addChannel(new KoChannelInfo(i18n("a*"), CHANNEL_A * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(150,150,150)));
    addChannel(new KoChannelInfo(i18n("b*"), CHANNEL_B * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(200,200,200)));
    addChannel(new KoChannelInfo(i18n("Alpha"), CHANNEL_ALPHA * sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));
    init();
    addCompositeOp( new KoCompositeOpOver<KoLabU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<KoLabU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpMultiply<KoLabU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpDivide<KoLabU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpBurn<KoLabU16Traits>( this ) );
}

bool KoLabColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8)
        return true;
    else
        return false;
}

QString KoLabColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    const KoLabU16Traits::channels_type *pix = reinterpret_cast<const  KoLabU16Traits::channels_type *>(pixel);
    Q_ASSERT(channelIndex < channelCount());

    // These convert from lcms encoded format to standard ranges.

    switch(channelIndex)
    {
        case CHANNEL_L:
            return QString().setNum(100.0 * static_cast<float>(pix[CHANNEL_L]) / MAX_CHANNEL_L);
        case CHANNEL_A:
            return QString().setNum(100.0 * ((static_cast<float>(pix[CHANNEL_A]) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case CHANNEL_B:
            return QString().setNum(100.0 * ((static_cast<float>(pix[CHANNEL_B]) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case CHANNEL_ALPHA:
            return QString().setNum(100.0 * static_cast<float>(pix[CHANNEL_ALPHA]) / UINT16_MAX);
        default:
            return QString("Error");
    }

}

KoColorSpace* KoLabColorSpace::clone() const
{
    return new KoLabColorSpace( profile()->clone());
}

QString KoLabColorSpace::colorSpaceId()
{
    return QString("LABA");
}

void KoLabColorSpace::colorToXML( const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoLabU16Traits::Pixel* p = reinterpret_cast<const KoLabU16Traits::Pixel*>( pixel );
    QDomElement labElt = doc.createElement( "Lab" );
    labElt.setAttribute("L", KoColorSpaceMaths< KoLabU16Traits::channels_type, qreal>::scaleToA( p->L) );
    labElt.setAttribute("a", KoColorSpaceMaths< KoLabU16Traits::channels_type, qreal>::scaleToA( p->a) );
    labElt.setAttribute("b", KoColorSpaceMaths< KoLabU16Traits::channels_type, qreal>::scaleToA( p->b) );
    labElt.setAttribute("space", profile()->name() );
    colorElt.appendChild( labElt );
}

void KoLabColorSpace::colorFromXML( quint8* pixel, const QDomElement& elt) const
{
    KoLabU16Traits::Pixel* p = reinterpret_cast<KoLabU16Traits::Pixel*>( pixel );
    p->L = KoColorSpaceMaths< qreal, KoLabU16Traits::channels_type >::scaleToA(elt.attribute("L").toDouble());
    p->a = KoColorSpaceMaths< qreal, KoLabU16Traits::channels_type >::scaleToA(elt.attribute("a").toDouble());
    p->b = KoColorSpaceMaths< qreal, KoLabU16Traits::channels_type >::scaleToA(elt.attribute("b").toDouble());
}
