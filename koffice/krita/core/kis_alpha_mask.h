/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_ALPHA_MASK_
#define KIS_ALPHA_MASK_

#include <qimage.h>
#include <qvaluevector.h>

#include <ksharedptr.h>

#include "kis_global.h"
#include "kis_types.h"

/**
 * KisAlphaMask is intended to create alpha values from a QImage for use
 * in brush creation. It is not a generic alpha mask that can be used with
 * KisPaintDevices: use a KisSelection for that.
 */
class KisAlphaMask : public KShared {
    
 public:
    /**
       Create an alpha mask based on the specified QImage. If the image is
       not a grayscale, the mask value is calculated from the effective grey
       level and alpha value.
    */
    KisAlphaMask(const QImage& img);

    /**
       As above except quicker as the image does not need to be scanned
       to see if it has any colour pixels.
    */
    KisAlphaMask(const QImage& img, bool hasColor);

    /**
       Create a transparent mask.
    */
    KisAlphaMask(Q_INT32 width, Q_INT32 height);

    virtual ~KisAlphaMask();

    /**
       @return the number of alpha values in a scanline.
    */
    Q_INT32 height() const;

    /**
       @return the number of lines in the mask.
     */
       Q_INT32 width() const;

    /**
       @return the alpha value at the specified position.

       Returns Q_UINT8 OPACITY_TRANSPARENT if the value is
       outside the bounds of the mask.

       XXX: this is, of course, not the best way of masking.
       Better would be to let KisAlphaMask fill a chunk of memory
       with the alpha values at the right position, something like
       void applyMask(Q_UINT8 *pixeldata, Q_INT32 pixelWidth,
       Q_INT32 alphaPos). That would be fastest, or we could
       provide an iterator over the mask, that would be nice, too.
    */
    Q_UINT8 alphaAt(Q_INT32 x, Q_INT32 y) const;

    void setAlphaAt(Q_INT32 x, Q_INT32 y, Q_UINT8 alpha);

    // Create a new mask by interpolating between mask1 and mask2 as t
    // goes from 0 to 1.
    static KisAlphaMaskSP interpolate(KisAlphaMaskSP mask1, KisAlphaMaskSP mask2, double t);

private:
    void computeAlpha(const QImage& img);
    void copyAlpha(const QImage& img);

    QValueVector<Q_UINT8> m_data;
    Q_INT32 m_width;
    Q_INT32 m_height;
};

#endif // KIS_ALPHA_MASK_

