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
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCOMPOSITEOPOVER_H_
#define KOCOMPOSITEOPOVER_H_

#include "KoColorSpaceMaths.h"
#include "KoCompositeOp.h"

#define NATIVE_OPACITY_OPAQUE KoColorSpaceMathsTraits<channels_type>::max()
#define NATIVE_OPACITY_TRANSPARENT KoColorSpaceMathsTraits<channels_type>::min()

/**
 * A template version of the over composite operation to use in colorspaces.
 */
template<class _CSTraits>
class KoCompositeOpOver : public KoCompositeOp {
    typedef typename _CSTraits::channels_type channels_type;
    public:

        KoCompositeOpOver(KoColorSpace * cs)
        : KoCompositeOp(cs, COMPOSITE_OVER, i18n("Normal" ) )
        {
        }

    public:
        struct Pixel {
            quint16 lightness;
            quint16 a;
            quint16 b;
            quint16 alpha;
        };

        void composite(quint8 *dstRowStart,
                        qint32 dststride,
                        const quint8 *srcRowStart,
                        qint32 srcstride,
                        const quint8 *maskRowStart,
                        qint32 maskstride,
                        qint32 rows,
                        qint32 cols,
                        quint8 U8_opacity,
                        const QBitArray & channelFlags) const
        {
            Q_UNUSED( channelFlags );

            channels_type opacity = KoColorSpaceMaths<quint8, channels_type>::scaleToA(U8_opacity);
            qint32 pixelSize = colorSpace()->pixelSize();

            while (rows > 0) {
                const channels_type *srcN = reinterpret_cast<const channels_type *>(srcRowStart);
                channels_type *dstN = reinterpret_cast<channels_type *>(dstRowStart);
                const quint8 *mask = maskRowStart;
                
                qint32 columns = cols;

                while (columns > 0) {
                    
                    channels_type srcAlpha = srcN[_CSTraits::alpha_pos];

                    // apply the alphamask
                    if (mask != 0) {
                        if (*mask != OPACITY_OPAQUE) {
                            srcAlpha = KoColorSpaceMaths<channels_type,quint8>::multiply(srcAlpha, *mask);
                        }
                        mask++;
                    }

                    if (srcAlpha != NATIVE_OPACITY_TRANSPARENT) {

                        if (opacity != NATIVE_OPACITY_OPAQUE) {
                            srcAlpha = KoColorSpaceMaths<channels_type>::multiply(srcAlpha, opacity);
                        }

                        if (srcAlpha == NATIVE_OPACITY_OPAQUE) {
                            memcpy(dstN, srcN, pixelSize);
                        } else {
                            channels_type dstAlpha = dstN[_CSTraits::alpha_pos];

                            channels_type srcBlend;

                            if (dstAlpha == NATIVE_OPACITY_OPAQUE) {
                                srcBlend = srcAlpha;
                            } else {
                                channels_type newAlpha = dstAlpha + KoColorSpaceMaths<channels_type>::multiply(NATIVE_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                                dstN[_CSTraits::alpha_pos] = newAlpha;

                                if (newAlpha != 0) {
                                    srcBlend = KoColorSpaceMaths<channels_type>::divide(srcAlpha, newAlpha);
                                } else {
                                    srcBlend = srcAlpha;
                                }
                            }

                            if (srcBlend == NATIVE_OPACITY_OPAQUE) {
                                memcpy(dstN, srcN, pixelSize);
                            } else {
                                for(uint i = 0; i <  _CSTraits::channels_nb; i++)
                                {
                                    if( (int)i != _CSTraits::alpha_pos)
                                        dstN[i] = KoColorSpaceMaths<channels_type>::blend(srcN[i], dstN[i], srcBlend);
                                }
                            }
                        }
                    }
                    columns--;
                    srcN+=_CSTraits::channels_nb;
                    dstN+=_CSTraits::channels_nb;
                }

                rows--;
                srcRowStart += srcstride;
                dstRowStart += dststride;
                if(maskRowStart) {
                    maskRowStart += maskstride;
                }
            }
        }

};

#endif
