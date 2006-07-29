/*
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_rgb_f16half_colorspace.h"
#include "kis_f32_base_colorspace.h"
#include "kis_color_conversions.h"

namespace {
    const Q_INT32 MAX_CHANNEL_RGB = 3;
    const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

#include "kis_integer_maths.h"

//#define HALF_MAX ((half)1.0f) //temp

#define EPSILON HALF_EPSILON

// FIXME: lcms doesn't support 16-bit float
#define RGBAF16HALF_LCMS_TYPE TYPE_BGRA_16

KisRgbF16HalfColorSpace::KisRgbF16HalfColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisF16HalfBaseColorSpace(KisID("RGBAF16HALF", i18n("RGB (16-bit float/channel)")), RGBAF16HALF_LCMS_TYPE, icSigRgbData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Red"), i18n("R"), PIXEL_RED * sizeof(half), KisChannelInfo::COLOR, KisChannelInfo::FLOAT16, sizeof(half)));
    m_channels.push_back(new KisChannelInfo(i18n("Green"), i18n("G"), PIXEL_GREEN * sizeof(half), KisChannelInfo::COLOR, KisChannelInfo::FLOAT16, sizeof(half)));
    m_channels.push_back(new KisChannelInfo(i18n("Blue"), i18n("B"), PIXEL_BLUE * sizeof(half), KisChannelInfo::COLOR, KisChannelInfo::FLOAT16, sizeof(half)));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), i18n("A"), PIXEL_ALPHA * sizeof(half), KisChannelInfo::ALPHA, KisChannelInfo::FLOAT16, sizeof(half)));

    //cmsHPROFILE hProfile = cmsCreate_sRGBProfile();
    //setDefaultProfile( new KisProfile(hProfile, RGBAF16HALF_LCMS_TYPE) );

    m_alphaPos = PIXEL_ALPHA * sizeof(half);
}

KisRgbF16HalfColorSpace::~KisRgbF16HalfColorSpace()
{
}

void KisRgbF16HalfColorSpace::setPixel(Q_UINT8 *dst, half red, half green, half blue, half alpha) const
{
    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->red = red;
    dstPixel->green = green;
    dstPixel->blue = blue;
    dstPixel->alpha = alpha;
}

void KisRgbF16HalfColorSpace::getPixel(const Q_UINT8 *src, half *red, half *green, half *blue, half *alpha) const
{
    const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);

    *red = srcPixel->red;
    *green = srcPixel->green;
    *blue = srcPixel->blue;
    *alpha = srcPixel->alpha;
}

void KisRgbF16HalfColorSpace::fromQColor(const QColor& c, Q_UINT8 *dstU8, KisProfile *)
{
    Pixel *dst = reinterpret_cast<Pixel *>(dstU8);

    dst->red = UINT8_TO_HALF(c.red());
    dst->green = UINT8_TO_HALF(c.green());
    dst->blue = UINT8_TO_HALF(c.blue());
}

void KisRgbF16HalfColorSpace::fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dstU8, KisProfile *)
{
    Pixel *dst = reinterpret_cast<Pixel *>(dstU8);

    dst->red = UINT8_TO_HALF(c.red());
    dst->green = UINT8_TO_HALF(c.green());
    dst->blue = UINT8_TO_HALF(c.blue());
    dst->alpha = UINT8_TO_HALF(opacity);
}

void KisRgbF16HalfColorSpace::toQColor(const Q_UINT8 *srcU8, QColor *c, KisProfile *)
{
    const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);

    c->setRgb(HALF_TO_UINT8(src->red), HALF_TO_UINT8(src->green), HALF_TO_UINT8(src->blue));
}

void KisRgbF16HalfColorSpace::toQColor(const Q_UINT8 *srcU8, QColor *c, Q_UINT8 *opacity, KisProfile *)
{
    const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);

    c->setRgb(HALF_TO_UINT8(src->red), HALF_TO_UINT8(src->green), HALF_TO_UINT8(src->blue));
    *opacity = HALF_TO_UINT8(src->alpha);
}

Q_UINT8 KisRgbF16HalfColorSpace::difference(const Q_UINT8 *src1U8, const Q_UINT8 *src2U8)
{
    const Pixel *src1 = reinterpret_cast<const Pixel *>(src1U8);
    const Pixel *src2 = reinterpret_cast<const Pixel *>(src2U8);

    return HALF_TO_UINT8(QMAX(QABS(src2->red - src1->red),
                QMAX(QABS(src2->green - src1->green),
                     QABS(src2->blue - src1->blue))));
}

void KisRgbF16HalfColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    half totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;

    while (nColors--)
    {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        half alpha = pixel->alpha;
        half alphaTimesWeight = alpha * UINT8_TO_HALF(*weights);

        totalRed += pixel->red * alphaTimesWeight;
        totalGreen += pixel->green * alphaTimesWeight;
        totalBlue += pixel->blue * alphaTimesWeight;
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= F16HALF_OPACITY_OPAQUE);

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > EPSILON) {
        totalRed = totalRed / newAlpha;
        totalGreen = totalGreen / newAlpha;
        totalBlue = totalBlue / newAlpha;
    }

    dstPixel->red = totalRed;
    dstPixel->green = totalGreen;
    dstPixel->blue = totalBlue;
}

void KisRgbF16HalfColorSpace::convolveColors(Q_UINT8** colors, Q_INT32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    half totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    while (nColors--)
    {
        const Pixel * pixel = reinterpret_cast<const Pixel *>( *colors );

        half weight = *kernelValues;

        if (weight != 0) {
            totalRed += pixel->red * UINT8_TO_HALF(weight);
            totalGreen += pixel->green * UINT8_TO_HALF(weight);
            totalBlue += pixel->blue * UINT8_TO_HALF(weight);
            totalAlpha += pixel->alpha * UINT8_TO_HALF(weight);
        }
        colors++;
        kernelValues++;
    }

    Pixel * p = reinterpret_cast< Pixel *>( dst );

    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        p->red = CLAMP( ( totalRed / factor) + offset, 0, HALF_MAX);
        p->green = CLAMP( ( totalGreen / factor) + offset, 0, HALF_MAX);
        p->blue = CLAMP( ( totalBlue / factor) + offset, 0, HALF_MAX);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        p->alpha = CLAMP((totalAlpha/ factor) + offset, 0, HALF_MAX);
    }
}


void KisRgbF16HalfColorSpace::invertColor(Q_UINT8 * src, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * p = reinterpret_cast< Pixel *>( src );
        p->red = 1.0 - p->red;
        p->green = 1.0 - p->green;
        p->blue = 1.0 - p->blue;
        src += psize;
    }

}


Q_UINT8 KisRgbF16HalfColorSpace::intensity8(const Q_UINT8 * src) const
{
    const Pixel * p = reinterpret_cast<const Pixel *>( src );

    return HALF_TO_UINT8((p->red * 0.30 + p->green * 0.59 + p->blue * 0.11) + 0.5);
}


QValueVector<KisChannelInfo *> KisRgbF16HalfColorSpace::channels() const
{
    return m_channels;
}

Q_UINT32 KisRgbF16HalfColorSpace::nChannels() const
{
    return MAX_CHANNEL_RGBA;
}

Q_UINT32 KisRgbF16HalfColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_RGB;
}

Q_UINT32 KisRgbF16HalfColorSpace::pixelSize() const
{
    return MAX_CHANNEL_RGBA * sizeof(half);
}

Q_UINT8 convertToDisplay(float value, float exposureFactor, float gamma)
{
    //value *= pow(2, exposure + 2.47393);
    value *= exposureFactor;

    value = powf(value, gamma);

    // scale middle gray to the target framebuffer value

    value *= 84.66f;

    int valueInt = (int)(value + 0.5);

    return CLAMP(valueInt, 0, 255);
}

QImage KisRgbF16HalfColorSpace::convertToQImage(const Q_UINT8 *dataU8, Q_INT32 width, Q_INT32 height,
                         KisProfile *  /*dstProfile*/,
                         Q_INT32 /*renderingIntent*/, float exposure)

{
    const half *data = reinterpret_cast<const half *>(dataU8);

    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    img.setAlphaBuffer(true);

    Q_INT32 i = 0;
    uchar *j = img.bits();

    // XXX: For now assume gamma 2.2.
    float gamma = 1 / 2.2;
    float exposureFactor = powf(2, exposure + 2.47393);

    while ( i < width * height * MAX_CHANNEL_RGBA) {
        *( j + 3)  = HALF_TO_UINT8(*( data + i + PIXEL_ALPHA ));
        *( j + 2 ) = convertToDisplay(*( data + i + PIXEL_RED ), exposureFactor, gamma);
        *( j + 1 ) = convertToDisplay(*( data + i + PIXEL_GREEN ), exposureFactor, gamma);
        *( j + 0 ) = convertToDisplay(*( data + i + PIXEL_BLUE ), exposureFactor, gamma);
        i += MAX_CHANNEL_RGBA;
        j += MAX_CHANNEL_RGBA;
    }

    /*
    if (srcProfile != 0 && dstProfile != 0) {
        convertPixelsTo(img.bits(), srcProfile,
                img.bits(), this, dstProfile,
                width * height, renderingIntent);
    }
    */
    return img;
}


void KisRgbF16HalfColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    while (rows > 0) {

        const half *src = reinterpret_cast<const half *>(srcRowStart);
        half *dst = reinterpret_cast<half *>(dstRowStart);
        const Q_UINT8 *mask = maskRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {

            half srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha *= UINT8_TO_HALF(U8_mask);
                }
                mask++;
            }

            if (srcAlpha > F16HALF_OPACITY_TRANSPARENT + EPSILON) {

                if (opacity < F16HALF_OPACITY_OPAQUE - EPSILON) {
                    srcAlpha *= opacity;
                }

                if (srcAlpha > F16HALF_OPACITY_OPAQUE - EPSILON) {
                    memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(half));
                } else {
                    half dstAlpha = dst[PIXEL_ALPHA];

                    half srcBlend;

                    if (dstAlpha > F16HALF_OPACITY_OPAQUE - EPSILON) {
                        srcBlend = srcAlpha;
                    } else {
                        half newAlpha = dstAlpha + (F16HALF_OPACITY_OPAQUE - dstAlpha) * srcAlpha;
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha > EPSILON) {
                            srcBlend = srcAlpha / newAlpha;
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend > F16HALF_OPACITY_OPAQUE - EPSILON) {
                        memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(half));
                    } else {
                        dst[PIXEL_RED] = HALF_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
                        dst[PIXEL_GREEN] = HALF_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
                        dst[PIXEL_BLUE] = HALF_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
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
        const half *src = reinterpret_cast<const half *>(srcRowStart); \
        half *dst = reinterpret_cast<half *>(dstRowStart); \
        Q_INT32 columns = numColumns; \
        const Q_UINT8 *mask = maskRowStart; \
    \
        while (columns > 0) { \
    \
            half srcAlpha = src[PIXEL_ALPHA]; \
            half dstAlpha = dst[PIXEL_ALPHA]; \
    \
            srcAlpha = QMIN(srcAlpha, dstAlpha); \
    \
            if (mask != 0) { \
                Q_UINT8 U8_mask = *mask; \
    \
                if (U8_mask != OPACITY_OPAQUE) { \
                    srcAlpha *= UINT8_TO_HALF(U8_mask); \
                } \
                mask++; \
            } \
    \
            if (srcAlpha > F16HALF_OPACITY_TRANSPARENT + EPSILON) { \
    \
                if (opacity < F16HALF_OPACITY_OPAQUE - EPSILON) { \
                    srcAlpha *= opacity; \
                } \
    \
                half srcBlend; \
    \
                if (dstAlpha > F16HALF_OPACITY_OPAQUE - EPSILON) { \
                    srcBlend = srcAlpha; \
                } else { \
                    half newAlpha = dstAlpha + (F16HALF_OPACITY_OPAQUE - dstAlpha) * srcAlpha; \
                    dst[PIXEL_ALPHA] = newAlpha; \
    \
                    if (newAlpha > EPSILON) { \
                        srcBlend = srcAlpha / newAlpha; \
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

void KisRgbF16HalfColorSpace::compositeMultiply(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        half srcColor = src[PIXEL_RED];
        half dstColor = dst[PIXEL_RED];

        srcColor = srcColor * dstColor;

        dst[PIXEL_RED] = HALF_BLEND(srcColor, dstColor, srcBlend);

        srcColor = src[PIXEL_GREEN];
        dstColor = dst[PIXEL_GREEN];

        srcColor = srcColor * dstColor;

        dst[PIXEL_GREEN] = HALF_BLEND(srcColor, dstColor, srcBlend);

        srcColor = src[PIXEL_BLUE];
        dstColor = dst[PIXEL_BLUE];

        srcColor = srcColor * dstColor;

        dst[PIXEL_BLUE] = HALF_BLEND(srcColor, dstColor, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeDivide(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = QMIN(dstColor / (srcColor + EPSILON), HALF_MAX);

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeScreen(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = HALF_MAX - ((HALF_MAX - dstColor) * (HALF_MAX - srcColor));

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeOverlay(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = dstColor * (dstColor + 2 * (srcColor * (HALF_MAX - dstColor)));

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeDodge(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = QMIN(dstColor / (HALF_MAX + EPSILON - srcColor), HALF_MAX);

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeBurn(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = QMIN((HALF_MAX - dstColor) / (srcColor + EPSILON), HALF_MAX);
            srcColor = CLAMP(HALF_MAX - srcColor, 0, HALF_MAX);

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeDarken(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = QMIN(srcColor, dstColor);

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeLighten(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            half srcColor = src[channel];
            half dstColor = dst[channel];

            srcColor = QMAX(srcColor, dstColor);

            half newColor = HALF_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeHue(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeSaturation(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeValue(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeColor(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, half opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcLightness;

        float dstHue;
        float dstSaturation;
        float dstLightness;

        RGBToHSL(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcLightness);
        RGBToHSL(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstLightness);

        HSLToRGB(srcHue, srcSaturation, dstLightness, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF16HalfColorSpace::compositeErase(Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src,
            Q_INT32 srcRowSize,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_INT32 rows,
            Q_INT32 cols,
            half /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const Q_UINT8 *mask = srcAlphaMask;

        for (Q_INT32 i = cols; i > 0; i--, s++, d++)
        {
            half srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = HALF_BLEND(srcAlpha, F16HALF_OPACITY_OPAQUE, UINT8_TO_HALF(U8_mask));
                }
                mask++;
            }
            d->alpha = srcAlpha * d->alpha;
        }

        dst += dstRowSize;
        src += srcRowSize;
        if(srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisRgbF16HalfColorSpace::bitBlt(Q_UINT8 *dst,
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
    half opacity = UINT8_TO_HALF(U8_opacity);

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

KisCompositeOpList KisRgbF16HalfColorSpace::userVisiblecompositeOps() const
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

