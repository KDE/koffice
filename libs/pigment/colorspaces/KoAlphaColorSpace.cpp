/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <limits.h>
#include <stdlib.h>

#include <QImage>
#include <QBitArray>

#include <klocale.h>

#include <lcms.h>

#include "KoAlphaColorSpace.h"
#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"

namespace {
    const quint8 PIXEL_MASK = 0;

    class CompositeOver : public KoCompositeOp {

    public:

        CompositeOver(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_OVER, i18n("Normal" ), KoCompositeOp::categoryMix() )
            {
            }

    public:

        using KoCompositeOp::composite;

        void composite(quint8 *dst,
                       qint32 dststride,
                       const quint8 *src,
                       qint32 srcstride,
                       const quint8 *maskRowStart,
                       qint32 maskstride,
                       qint32 rows,
                       qint32 cols,
                       quint8 opacity,
                       const QBitArray & channelFlags) const
            {
                Q_UNUSED(channelFlags);

                quint8 *d;
                const quint8 *s;

                if (rows <= 0 || cols <= 0)
                    return;
                if (opacity == OPACITY_TRANSPARENT)
                    return;
                if (opacity != OPACITY_OPAQUE) {
                    while (rows-- > 0) {

                        const quint8 *mask = maskRowStart;

                        d = dst;
                        s = src;
                        for (qint32 i = cols; i > 0; i--, d++, s++) {
                            // If the mask tells us to completely not
                            // blend this pixel, continue.
                            if ( mask != 0 ) {
                                if ( mask[0] == OPACITY_TRANSPARENT ) {
                                    mask++;
                                    continue;
                                }
                                mask++;
                            }
                            if (s[PIXEL_MASK] == OPACITY_TRANSPARENT)
                                continue;
                            int srcAlpha = (s[PIXEL_MASK] * opacity + UINT8_MAX / 2) / UINT8_MAX;
                            d[PIXEL_MASK] = (d[PIXEL_MASK] * (UINT8_MAX - srcAlpha) + srcAlpha * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
                        }
                        dst += dststride;
                        src += srcstride;
                        if(maskRowStart) {
                            maskRowStart += maskstride;
                        }
                    }
                }
                else {
                    while (rows-- > 0) {
                        const quint8 *mask = maskRowStart;

                        d = dst;
                        s = src;
                        for (qint32 i = cols; i > 0; i--, d++, s++) {

                            if ( mask != 0 ) {
                                // If the mask tells us to completely not
                                // blend this pixel, continue.
                                if ( mask[0] == OPACITY_TRANSPARENT ) {
                                    mask++;
                                    continue;
                                }
                                mask++;
                            }

                            if (s[PIXEL_MASK] == OPACITY_TRANSPARENT)
                                continue;
                            if (d[PIXEL_MASK] == OPACITY_TRANSPARENT || s[PIXEL_MASK] == OPACITY_OPAQUE) {
                                memcpy(d, s, 1);
                                continue;
                            }
                            int srcAlpha = s[PIXEL_MASK];
                            d[PIXEL_MASK] = (d[PIXEL_MASK] * (UINT8_MAX - srcAlpha) + srcAlpha * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
                        }
                        dst += dststride;
                        src += srcstride;
                        if(maskRowStart) {
                            maskRowStart += maskstride;
                        }
                    }
                }
            }
    };

    class CompositeClear : public KoCompositeOp {

    public:

        CompositeClear(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_CLEAR, i18n("Clear" ), KoCompositeOp::categoryMix() )
            {
            }

    public:

        using KoCompositeOp::composite;

        void composite(quint8 *dst,
                       qint32 dststride,
                       const quint8 *src,
                       qint32 srcstride,
                       const quint8 *maskRowStart,
                       qint32 maskstride,
                       qint32 rows,
                       qint32 cols,
                       quint8 opacity,
                       const QBitArray & channelFlags) const
            {
                Q_UNUSED( src );
                Q_UNUSED( srcstride );
                Q_UNUSED( opacity );
                Q_UNUSED( channelFlags );

                quint8 *d;
                qint32 linesize;

                if (rows <= 0 || cols <= 0)
                    return;

                if ( maskRowStart == 0 ) {

                    linesize = sizeof(quint8) * cols;
                    d = dst;
                    while (rows-- > 0) {

                        memset(d, OPACITY_TRANSPARENT, linesize);
                        d += dststride;
                    }
                }
                else {
                    while (rows-- > 0) {

                        const quint8 *mask = maskRowStart;

                        d = dst;

                        for (qint32 i = cols; i > 0; i--, d++) {
                            // If the mask tells us to completely not
                            // blend this pixel, continue.
                            if ( mask != 0 ) {
                                if ( mask[0] == OPACITY_TRANSPARENT ) {
                                    mask++;
                                    continue;
                                }
                                mask++;
                            }
                            // linesize is uninitialized here, so it just crashes
                            //memset(d, OPACITY_TRANSPARENT, linesize);
                        }
                        dst += dststride;
                        src += srcstride;
                        maskRowStart += maskstride;
                    }
                }

            }

    };

    class CompositeErase : public KoCompositeOp {

    public:

        CompositeErase(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_ERASE, i18n("Erase" ), KoCompositeOp::categoryMix() )
            {
            }

    public:

        using KoCompositeOp::composite;

        void composite(quint8 *dst,
                       qint32 dststride,
                       const quint8 *src,
                       qint32 srcstride,
                       const quint8 *maskRowStart,
                       qint32 maskstride,
                       qint32 rows,
                       qint32 cols,
                       quint8 opacity,
                       const QBitArray & channelFlags) const
            {

                Q_UNUSED( opacity );
                Q_UNUSED( channelFlags );

                quint8 *d;
                const quint8 *s;
                qint32 i;
                if (rows <= 0 || cols <= 0)
                    return;

                while (rows-- > 0) {
                    const quint8 *mask = maskRowStart;
                    d = dst;
                    s = src;

                    for (i = cols; i > 0; i--, d ++, s ++) {

                        if ( mask != 0 ) {
                            if ( mask[0] == OPACITY_TRANSPARENT ) {
                                mask++;
                                continue;
                            }
                            mask++;
                        }

                        if (d[PIXEL_MASK] < s[PIXEL_MASK]) {
                            continue;
                        }
                        else {
                            d[PIXEL_MASK] = s[PIXEL_MASK];
                        }

                    }

                    dst += dststride;
                    src += srcstride;
                    if ( maskRowStart ) maskRowStart += maskstride;
                }
            }
    };

    class CompositeSubtract : public KoCompositeOp {

    public:

        CompositeSubtract(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_SUBSTRACT, i18n("Subtract" ), KoCompositeOp::categoryArithmetic() )
            {
            }

    public:

        using KoCompositeOp::composite;

        void composite(quint8 *dst,
                       qint32 dststride,
                       const quint8 *src,
                       qint32 srcstride,
                       const quint8 *maskRowStart,
                       qint32 maskstride,
                       qint32 rows,
                       qint32 cols,
                       quint8 opacity,
                       const QBitArray & channelFlags) const
            {

                Q_UNUSED( opacity );
                Q_UNUSED( channelFlags );


                quint8 *d;
                const quint8 *s;
                qint32 i;

                if (rows <= 0 || cols <= 0)
                    return;

                while (rows-- > 0) {
                    const quint8 *mask = maskRowStart;
                    d = dst;
                    s = src;

                    for (i = cols; i > 0; i--, d++, s++) {

                        // If the mask tells us to completely not
                        // blend this pixel, continue.
                        if ( mask != 0 ) {
                            if ( mask[0] == OPACITY_TRANSPARENT ) {
                                mask++;
                                continue;
                            }
                            mask++;
                        }

                        if (d[PIXEL_MASK] <= s[PIXEL_MASK]) {
                            d[PIXEL_MASK] = OPACITY_TRANSPARENT;
                        } else {
                            d[PIXEL_MASK] -= s[PIXEL_MASK];
                        }
                    }

                    dst += dststride;
                    src += srcstride;

                    if(maskRowStart) {
                        maskRowStart += maskstride;
                    }
                }
            }
    };

}

KoAlphaColorSpace::KoAlphaColorSpace() :
    KoColorSpaceAbstract<AlphaU8Traits>("ALPHA", i18n("Alpha mask") )
{
    addChannel(new KoChannelInfo(i18n("Alpha"), 0, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));
    addCompositeOp( new CompositeOver( this ) );
    addCompositeOp( new CompositeClear( this ) );
    addCompositeOp( new CompositeErase( this ) );
    addCompositeOp( new CompositeSubtract( this ) );
}

KoAlphaColorSpace::~KoAlphaColorSpace()
{
}

void KoAlphaColorSpace::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
    dst[PIXEL_MASK] = c.alpha();
}

void KoAlphaColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    c->setRgba(qRgba(255, 255, 255, src[PIXEL_MASK]));
}

quint8 KoAlphaColorSpace::difference(const quint8 *src1, const quint8 *src2) const
{
    // Arithmetic operands smaller than int are converted to int automatically
    return qAbs(src2[PIXEL_MASK] - src1[PIXEL_MASK]);
}

void KoAlphaColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    if (nColors > 0) {
        quint32 total = 0;

        while(nColors)
        {
            nColors--;
            total += *colors[nColors] * weights[nColors];
        }
        *dst = total / 255;
    }
}

bool KoAlphaColorSpace::convertPixelsTo(const quint8 *src,
                     quint8 *dst, const KoColorSpace * dstColorSpace,
                     quint32 numPixels,
                     KoColorConversionTransformation::Intent /*renderingIntent*/) const
{
    // No lcms trickery here, we are only a opacity channel
    qint32 size = dstColorSpace->pixelSize();

    memset(dst, 0, numPixels * size);
    dstColorSpace->applyInverseAlphaU8Mask(dst, src, numPixels);

    return true;

}



QString KoAlphaColorSpace::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < channelCount());
    quint32 channelPosition = channels()[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KoAlphaColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < channelCount());
    quint32 channelPosition = channels()[channelIndex]->pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}


void KoAlphaColorSpace::convolveColors(quint8** colors, qint32 * kernelValues, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors, const QBitArray & channelFlags) const
{
    qint32 totalAlpha = 0;

    while (nColors--)
    {
        qint32 weight = *kernelValues;

        if (weight != 0) {
            totalAlpha += (*colors)[PIXEL_MASK] * weight;
        }
        colors++;
        kernelValues++;
    }

    if ( channelFlags.isEmpty() || channelFlags.testBit(PIXEL_MASK) )
        dst[PIXEL_MASK] = CLAMP((totalAlpha/ factor) + offset, 0, SCHAR_MAX);
}

QImage KoAlphaColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  /*dstProfile*/, KoColorConversionTransformation::Intent /*renderingIntent*/) const
{
    QImage img(width, height, QImage::Format_RGB32);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            quint8 c = data[x + y * width];
            img.setPixel(x, y, qRgb(c, c, c));
        }
    }
    return img;
}

KoColorSpace* KoAlphaColorSpace::clone() const
{
    return new KoAlphaColorSpace();
}

