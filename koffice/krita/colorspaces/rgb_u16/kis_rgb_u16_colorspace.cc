/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include LCMS_HEADER

#include <qimage.h>
#include <qcolor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include "kis_rgb_u16_colorspace.h"
#include "kis_u16_base_colorspace.h"
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"

namespace {
    const Q_INT32 MAX_CHANNEL_RGB = 3;
    const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

// XXX: already defined is superclass?
//const Q_UINT16 KisRgbU16ColorSpace::U16_OPACITY_OPAQUE;
//const Q_UINT16 KisRgbU16ColorSpace::U16_OPACITY_TRANSPARENT;

KisRgbU16ColorSpace::KisRgbU16ColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisU16BaseColorSpace(KisID("RGBA16", i18n("RGB (16-bit integer/channel)")), TYPE_BGRA_16, icSigRgbData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Red"), i18n("R"), PIXEL_RED * sizeof(Q_UINT16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(255,0,0)));
    m_channels.push_back(new KisChannelInfo(i18n("Green"), i18n("G"), PIXEL_GREEN * sizeof(Q_UINT16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(0,255,0)));
    m_channels.push_back(new KisChannelInfo(i18n("Blue"), i18n("B"), PIXEL_BLUE * sizeof(Q_UINT16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(0,0,255)));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), i18n("A"), PIXEL_ALPHA * sizeof(Q_UINT16), KisChannelInfo::ALPHA, KisChannelInfo::UINT16, sizeof(Q_UINT16)));

    m_alphaPos = PIXEL_ALPHA * sizeof(Q_UINT16);

    init();
}

KisRgbU16ColorSpace::~KisRgbU16ColorSpace()
{
}

void KisRgbU16ColorSpace::setPixel(Q_UINT8 *dst, Q_UINT16 red, Q_UINT16 green, Q_UINT16 blue, Q_UINT16 alpha) const
{
    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->red = red;
    dstPixel->green = green;
    dstPixel->blue = blue;
    dstPixel->alpha = alpha;
}

void KisRgbU16ColorSpace::getPixel(const Q_UINT8 *src, Q_UINT16 *red, Q_UINT16 *green, Q_UINT16 *blue, Q_UINT16 *alpha) const
{
    const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);

    *red = srcPixel->red;
    *green = srcPixel->green;
    *blue = srcPixel->blue;
    *alpha = srcPixel->alpha;
}

void KisRgbU16ColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT32 totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;

    while (nColors--)
    {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        Q_UINT32 alpha = pixel->alpha;
        Q_UINT32 alphaTimesWeight = UINT16_MULT(alpha, UINT8_TO_UINT16(*weights));

        totalRed += UINT16_MULT(pixel->red, alphaTimesWeight);
        totalGreen += UINT16_MULT(pixel->green, alphaTimesWeight);
        totalBlue += UINT16_MULT(pixel->blue, alphaTimesWeight);
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= U16_OPACITY_OPAQUE);

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > 0) {
        totalRed = UINT16_DIVIDE(totalRed, newAlpha);
        totalGreen = UINT16_DIVIDE(totalGreen, newAlpha);
        totalBlue = UINT16_DIVIDE(totalBlue, newAlpha);
    }

    dstPixel->red = totalRed;
    dstPixel->green = totalGreen;
    dstPixel->blue = totalBlue;
}


void KisRgbU16ColorSpace::convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst,
                                         Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    Q_INT32 totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    while (nColors--)
    {
        const Pixel * pixel = reinterpret_cast<const Pixel *>( *colors );

        Q_INT32 weight = *kernelValues;

        if (weight != 0) {
            totalRed += pixel->red * weight;
            totalGreen += pixel->green * weight;
            totalBlue += pixel->blue * weight;
            totalAlpha +=pixel->alpha * weight;
        }
        colors++;
        kernelValues++;
    }

    Pixel * p = reinterpret_cast< Pixel *>( dst );

    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        p->red = CLAMP( ( totalRed / factor) + offset, 0, Q_UINT16_MAX);
        p->green = CLAMP( ( totalGreen / factor) + offset, 0, Q_UINT16_MAX);
        p->blue = CLAMP( ( totalBlue / factor) + offset, 0, Q_UINT16_MAX);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        p->alpha = CLAMP((totalAlpha/ factor) + offset, 0, Q_UINT16_MAX);
    }
}


void KisRgbU16ColorSpace::invertColor(Q_UINT8 * src, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * p = reinterpret_cast< Pixel *>( src );
        p->red = Q_UINT16_MAX - p->red;
        p->green = Q_UINT16_MAX - p->green;
        p->blue = Q_UINT16_MAX - p->blue;
        src += psize;
    }
}

Q_UINT8 KisRgbU16ColorSpace::intensity8(const Q_UINT8 * src) const
{
    const Pixel * p = reinterpret_cast<const Pixel *>( src );
    
    return UINT16_TO_UINT8(static_cast<Q_UINT16>((p->red * 0.30 + p->green * 0.59 + p->blue * 0.11) + 0.5));
}


QValueVector<KisChannelInfo *> KisRgbU16ColorSpace::channels() const
{
    return m_channels;
}

Q_UINT32 KisRgbU16ColorSpace::nChannels() const
{
    return MAX_CHANNEL_RGBA;
}

Q_UINT32 KisRgbU16ColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_RGB;
}

Q_UINT32 KisRgbU16ColorSpace::pixelSize() const
{
    return MAX_CHANNEL_RGBA * sizeof(Q_UINT16);
}


void KisRgbU16ColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    while (rows > 0) {

        const Q_UINT16 *src = reinterpret_cast<const Q_UINT16 *>(srcRowStart);
        Q_UINT16 *dst = reinterpret_cast<Q_UINT16 *>(dstRowStart);
        const Q_UINT8 *mask = maskRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {

            Q_UINT16 srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }

            if (srcAlpha != U16_OPACITY_TRANSPARENT) {

                if (opacity != U16_OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == U16_OPACITY_OPAQUE) {
                    memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(Q_UINT16));
                } else {
                    Q_UINT16 dstAlpha = dst[PIXEL_ALPHA];

                    Q_UINT16 srcBlend;

                    if (dstAlpha == U16_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == U16_OPACITY_OPAQUE) {
                        memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(Q_UINT16));
                    } else {
                        dst[PIXEL_RED] = UINT16_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
                        dst[PIXEL_GREEN] = UINT16_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
                        dst[PIXEL_BLUE] = UINT16_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart) {
            maskRowStart += maskRowStride;
        }
    }
}

#define COMMON_COMPOSITE_OP_PROLOG() \
    while (rows > 0) { \
    \
        const Q_UINT16 *src = reinterpret_cast<const Q_UINT16 *>(srcRowStart); \
        Q_UINT16 *dst = reinterpret_cast<Q_UINT16 *>(dstRowStart); \
        Q_INT32 columns = numColumns; \
        const Q_UINT8 *mask = maskRowStart; \
    \
        while (columns > 0) { \
    \
            Q_UINT16 srcAlpha = src[PIXEL_ALPHA]; \
            Q_UINT16 dstAlpha = dst[PIXEL_ALPHA]; \
    \
            srcAlpha = QMIN(srcAlpha, dstAlpha); \
    \
            if (mask != 0) { \
                Q_UINT8 U8_mask = *mask; \
    \
                if (U8_mask != OPACITY_OPAQUE) { \
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask)); \
                } \
                mask++; \
            } \
    \
            if (srcAlpha != U16_OPACITY_TRANSPARENT) { \
    \
                if (opacity != U16_OPACITY_OPAQUE) { \
                    srcAlpha = UINT16_MULT(srcAlpha, opacity); \
                } \
    \
                Q_UINT16 srcBlend; \
    \
                if (dstAlpha == U16_OPACITY_OPAQUE) { \
                    srcBlend = srcAlpha; \
                } else { \
                    Q_UINT16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha); \
                    dst[PIXEL_ALPHA] = newAlpha; \
    \
                    if (newAlpha != 0) { \
                        srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha); \
                    } else { \
                        srcBlend = srcAlpha; \
                    } \
                }

#define COMMON_COMPOSITE_OP_EPILOG() \
            } \
    \
            columns--; \
            src += MAX_CHANNEL_RGBA; \
            dst += MAX_CHANNEL_RGBA; \
        } \
    \
        rows--; \
        srcRowStart += srcRowStride; \
        dstRowStart += dstRowStride; \
        if(maskRowStart) { \
            maskRowStart += maskRowStride; \
        } \
    }

void KisRgbU16ColorSpace::compositeMultiply(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {

        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = UINT16_MULT(srcColor, dstColor);

            dst[channel] = UINT16_BLEND(srcColor, dstColor, srcBlend);

        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeDivide(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN((dstColor * (UINT16_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT16_MAX);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeScreen(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = UINT16_MAX - UINT16_MULT(UINT16_MAX - dstColor, UINT16_MAX - srcColor);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeOverlay(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = UINT16_MULT(dstColor, dstColor + 2u * UINT16_MULT(srcColor, UINT16_MAX - dstColor));

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeDodge(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN((dstColor * (UINT16_MAX + 1u)) / (UINT16_MAX + 1u - srcColor), UINT16_MAX);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeBurn(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = kMin(((UINT16_MAX - dstColor) * (UINT16_MAX + 1u)) / (srcColor + 1u), UINT16_MAX);
            srcColor = kClamp(UINT16_MAX - srcColor, 0u, UINT16_MAX);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeDarken(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN(srcColor, dstColor);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeLighten(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMAX(srcColor, dstColor);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeHue(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float FSrcRed = static_cast<float>(src[PIXEL_RED]) / UINT16_MAX;
        float FSrcGreen = static_cast<float>(src[PIXEL_GREEN]) / UINT16_MAX;
        float FSrcBlue = static_cast<float>(src[PIXEL_BLUE]) / UINT16_MAX;

        Q_UINT16 dstRed = dst[PIXEL_RED];
        Q_UINT16 dstGreen = dst[PIXEL_GREEN];
        Q_UINT16 dstBlue = dst[PIXEL_BLUE];

        float FDstRed = static_cast<float>(dstRed) / UINT16_MAX;
        float FDstGreen = static_cast<float>(dstGreen) / UINT16_MAX;
        float FDstBlue = static_cast<float>(dstBlue) / UINT16_MAX;

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(FSrcRed, FSrcGreen, FSrcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(FDstRed, FDstGreen, FDstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(srcHue, dstSaturation, dstValue, &FSrcRed, &FSrcGreen, &FSrcBlue);

        Q_UINT16 srcRed = static_cast<Q_UINT16>(FSrcRed * UINT16_MAX + 0.5);
        Q_UINT16 srcGreen = static_cast<Q_UINT16>(FSrcGreen * UINT16_MAX + 0.5);
        Q_UINT16 srcBlue = static_cast<Q_UINT16>(FSrcBlue * UINT16_MAX + 0.5);

        dst[PIXEL_RED] = UINT16_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = UINT16_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = UINT16_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeSaturation(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float FSrcRed = static_cast<float>(src[PIXEL_RED]) / UINT16_MAX;
        float FSrcGreen = static_cast<float>(src[PIXEL_GREEN]) / UINT16_MAX;
        float FSrcBlue = static_cast<float>(src[PIXEL_BLUE]) / UINT16_MAX;

        Q_UINT16 dstRed = dst[PIXEL_RED];
        Q_UINT16 dstGreen = dst[PIXEL_GREEN];
        Q_UINT16 dstBlue = dst[PIXEL_BLUE];

        float FDstRed = static_cast<float>(dstRed) / UINT16_MAX;
        float FDstGreen = static_cast<float>(dstGreen) / UINT16_MAX;
        float FDstBlue = static_cast<float>(dstBlue) / UINT16_MAX;

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(FSrcRed, FSrcGreen, FSrcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(FDstRed, FDstGreen, FDstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(dstHue, srcSaturation, dstValue, &FSrcRed, &FSrcGreen, &FSrcBlue);

        Q_UINT16 srcRed = static_cast<Q_UINT16>(FSrcRed * UINT16_MAX + 0.5);
        Q_UINT16 srcGreen = static_cast<Q_UINT16>(FSrcGreen * UINT16_MAX + 0.5);
        Q_UINT16 srcBlue = static_cast<Q_UINT16>(FSrcBlue * UINT16_MAX + 0.5);

        dst[PIXEL_RED] = UINT16_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = UINT16_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = UINT16_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeValue(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float FSrcRed = static_cast<float>(src[PIXEL_RED]) / UINT16_MAX;
        float FSrcGreen = static_cast<float>(src[PIXEL_GREEN]) / UINT16_MAX;
        float FSrcBlue = static_cast<float>(src[PIXEL_BLUE]) / UINT16_MAX;

        Q_UINT16 dstRed = dst[PIXEL_RED];
        Q_UINT16 dstGreen = dst[PIXEL_GREEN];
        Q_UINT16 dstBlue = dst[PIXEL_BLUE];

        float FDstRed = static_cast<float>(dstRed) / UINT16_MAX;
        float FDstGreen = static_cast<float>(dstGreen) / UINT16_MAX;
        float FDstBlue = static_cast<float>(dstBlue) / UINT16_MAX;

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(FSrcRed, FSrcGreen, FSrcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(FDstRed, FDstGreen, FDstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(dstHue, dstSaturation, srcValue, &FSrcRed, &FSrcGreen, &FSrcBlue);

        Q_UINT16 srcRed = static_cast<Q_UINT16>(FSrcRed * UINT16_MAX + 0.5);
        Q_UINT16 srcGreen = static_cast<Q_UINT16>(FSrcGreen * UINT16_MAX + 0.5);
        Q_UINT16 srcBlue = static_cast<Q_UINT16>(FSrcBlue * UINT16_MAX + 0.5);

        dst[PIXEL_RED] = UINT16_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = UINT16_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = UINT16_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeColor(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float FSrcRed = static_cast<float>(src[PIXEL_RED]) / UINT16_MAX;
        float FSrcGreen = static_cast<float>(src[PIXEL_GREEN]) / UINT16_MAX;
        float FSrcBlue = static_cast<float>(src[PIXEL_BLUE]) / UINT16_MAX;

        Q_UINT16 dstRed = dst[PIXEL_RED];
        Q_UINT16 dstGreen = dst[PIXEL_GREEN];
        Q_UINT16 dstBlue = dst[PIXEL_BLUE];

        float FDstRed = static_cast<float>(dstRed) / UINT16_MAX;
        float FDstGreen = static_cast<float>(dstGreen) / UINT16_MAX;
        float FDstBlue = static_cast<float>(dstBlue) / UINT16_MAX;

        float srcHue;
        float srcSaturation;
        float srcLightness;

        float dstHue;
        float dstSaturation;
        float dstLightness;

        RGBToHSL(FSrcRed, FSrcGreen, FSrcBlue, &srcHue, &srcSaturation, &srcLightness);
        RGBToHSL(FDstRed, FDstGreen, FDstBlue, &dstHue, &dstSaturation, &dstLightness);

        HSLToRGB(srcHue, srcSaturation, dstLightness, &FSrcRed, &FSrcGreen, &FSrcBlue);

        Q_UINT16 srcRed = static_cast<Q_UINT16>(FSrcRed * UINT16_MAX + 0.5);
        Q_UINT16 srcGreen = static_cast<Q_UINT16>(FSrcGreen * UINT16_MAX + 0.5);
        Q_UINT16 srcBlue = static_cast<Q_UINT16>(FSrcBlue * UINT16_MAX + 0.5);

        dst[PIXEL_RED] = UINT16_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = UINT16_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = UINT16_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbU16ColorSpace::compositeErase(Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src,
            Q_INT32 srcRowSize,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_INT32 rows,
            Q_INT32 cols,
            Q_UINT16 /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const Q_UINT8 *mask = srcAlphaMask;

        for (Q_INT32 i = cols; i > 0; i--, s++, d++)
        {
            Q_UINT16 srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_BLEND(srcAlpha, U16_OPACITY_OPAQUE, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }
            d->alpha = UINT16_MULT(srcAlpha, d->alpha);
        }

        dst += dstRowSize;
        src += srcRowSize;
        if(srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisRgbU16ColorSpace::bitBlt(Q_UINT8 *dst,
                      Q_INT32 dstRowStride,
                      const Q_UINT8 *src,
                      Q_INT32 srcRowStride,
                      const Q_UINT8 *mask,
                      Q_INT32 maskRowStride,
                      Q_UINT8 U8_opacity,
                      Q_INT32 rows,
                      Q_INT32 cols,
                      const KisCompositeOp& op)
{
    Q_UINT16 opacity = UINT8_TO_UINT16(U8_opacity);

    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_IN:
        //compositeIn(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    case COMPOSITE_OUT:
        //compositeOut(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ATOP:
        //compositeAtop(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_XOR:
        //compositeXor(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_PLUS:
        //compositePlus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MINUS:
        //compositeMinus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ADD:
        //compositeAdd(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SUBTRACT:
        //compositeSubtract(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIFF:
        //compositeDiff(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MULT:
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIVIDE:
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BUMPMAP:
        //compositeBumpmap(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, U8_opacity);
        break;
    case COMPOSITE_COPY_RED:
        //compositeCopyRed(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_GREEN:
        //compositeCopyGreen(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_BLUE:
        //compositeCopyBlue(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_OPACITY:
        //compositeCopyOpacity(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_CLEAR:
        //compositeClear(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISSOLVE:
        //compositeDissolve(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISPLACE:
        //compositeDisplace(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
#if 0
    case COMPOSITE_MODULATE:
        compositeModulate(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_THRESHOLD:
        compositeThreshold(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
#endif
    case COMPOSITE_NO:
        // No composition.
        break;
    case COMPOSITE_DARKEN:
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LIGHTEN:
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_HUE:
        compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SATURATION:
        compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_VALUE:
        compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLOR:
        compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLORIZE:
        //compositeColorize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LUMINIZE:
        //compositeLuminize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SCREEN:
        compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_OVERLAY:
        compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DODGE:
        compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BURN:
        compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    default:
        break;
    }
}

KisCompositeOpList KisRgbU16ColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));
    list.append(KisCompositeOp(COMPOSITE_MULT));
    list.append(KisCompositeOp(COMPOSITE_BURN));
    list.append(KisCompositeOp(COMPOSITE_DODGE));
    list.append(KisCompositeOp(COMPOSITE_DIVIDE));
    list.append(KisCompositeOp(COMPOSITE_SCREEN));
    list.append(KisCompositeOp(COMPOSITE_OVERLAY));
    list.append(KisCompositeOp(COMPOSITE_DARKEN));
    list.append(KisCompositeOp(COMPOSITE_LIGHTEN));
    list.append(KisCompositeOp(COMPOSITE_HUE));
    list.append(KisCompositeOp(COMPOSITE_SATURATION));
    list.append(KisCompositeOp(COMPOSITE_VALUE));
    list.append(KisCompositeOp(COMPOSITE_COLOR));

    return list;
}
