/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_STRATEGY_COLORSPACE_LMS_F32_H_
#define KIS_STRATEGY_COLORSPACE_LMS_F32_H_

#include <qcolor.h>

#include <klocale.h>

#include <koffice_export.h>

#include "kis_global.h"
#include "kis_f32_base_colorspace.h"

class KisColorSpaceFactoryRegistry;

class KRITATOOL_EXPORT KisLmsF32ColorSpace : public KisF32BaseColorSpace {
public:
    KisLmsF32ColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p);
    virtual ~KisLmsF32ColorSpace();

    virtual bool willDegrade(ColorSpaceIndependence independence)
        {
            if (independence == TO_RGBA8 || independence == TO_LAB16)
                return true;
            else
                return false;
        };


public:
    void setPixel(Q_UINT8 *pixel, float longWave, float middleWave, float shortWave, float alpha) const;
    void getPixel(const Q_UINT8 *pixel, float *longWave, float *middleWave, float *shortWave, float *alpha) const;

    virtual void fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile * profile = 0);
    virtual void fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile * profile = 0);

    virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfile * profile = 0);
    virtual void toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile * profile = 0);

    virtual Q_UINT8 difference(const Q_UINT8 *src1, const Q_UINT8 *src2);
    virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;

    virtual QValueVector<KisChannelInfo *> channels() const;
    virtual Q_UINT32 nChannels() const;
    virtual Q_UINT32 nColorChannels() const;
    virtual Q_UINT32 pixelSize() const;

    virtual bool hasHighDynamicRange() const { return false; }

    virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                       KisProfile *  dstProfile,
                       Q_INT32 renderingIntent,
                       float exposure = 0.0f);

    virtual KisCompositeOpList userVisiblecompositeOps() const;


protected:

    virtual void bitBlt(Q_UINT8 *dst,
                Q_INT32 dstRowStride,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op);

    void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, float opacity);
    void compositeErase(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, float opacity);
    void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, float opacity);

private:
    inline Q_UINT8 computeRed(float l, float m, float s) const
    {
        return FLOAT_TO_UINT8(4.4679*l - 3.58738*m + 0.1193*s);
    }
    inline Q_UINT8 computeGreen(float l, float m, float s) const
    {
        return FLOAT_TO_UINT8(-1.2186*l + 2.3809*m - 0.1624*s);
    }
    inline Q_UINT8 computeBlue(float l, float m, float s) const
    {
        return FLOAT_TO_UINT8(0.0497*l - 0.2439*m + 1.2045*s);
    }
    inline float computeLong(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b) const
    {
        return 0.3811*UINT8_TO_FLOAT(r) + 0.5783*UINT8_TO_FLOAT(g) + 0.0402*UINT8_TO_FLOAT(b);
    }
    inline float computeMiddle(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b) const
    {
        return 0.1967*UINT8_TO_FLOAT(r) + 0.7244*UINT8_TO_FLOAT(g) + 0.0782*UINT8_TO_FLOAT(b);
    }
    inline float computeShort(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b) const
    {
        return 0.0241*UINT8_TO_FLOAT(r) + 0.1288*UINT8_TO_FLOAT(g) + 0.8444*UINT8_TO_FLOAT(b);
    }

    friend class KisLmsF32ColorSpaceTester;

    static const Q_UINT8 PIXEL_LONGWAVE = 0;
    static const Q_UINT8 PIXEL_MIDDLEWAVE = 1;
    static const Q_UINT8 PIXEL_SHORTWAVE = 2;
    static const Q_UINT8 PIXEL_ALPHA = 3;

    struct Pixel {
        float longWave;
        float middleWave;
        float shortWave;
        float alpha;
    };
};

class KisLmsF32ColorSpaceFactory : public KisColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KisID id() const { return KisID("LMSAF32", i18n("LMS Cone Space (32-bit float/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual Q_UINT32 colorSpaceType() { return 0; }; // FIXME: lcms do not support LMS cone space

    virtual icColorSpaceSignature colorSpaceSignature() { return icMaxEnumData; };

    virtual KisColorSpace *createColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) { return new KisLmsF32ColorSpace(parent, p); };

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_LMS_F32_H_

