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

#ifndef KIS_STRATEGY_COLORSPACE_CMYK_U8_H_
#define KIS_STRATEGY_COLORSPACE_CMYK_U8_H_

#include <KoLcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

typedef KoColorSpaceTrait<quint8, 5, 4> CmykU8Traits;

class KisCmykU8ColorSpace : public KoLcmsColorSpace<CmykU8Traits>
{
    public:
        KisCmykU8ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence) const;
};

class KisCmykU8ColorSpaceFactory : public KoLcmsColorSpaceFactory
{
    public:
        KisCmykU8ColorSpaceFactory(): KoLcmsColorSpaceFactory(TYPE_CMYK5_8, icSigCmykData)
        {}
        virtual QString id() const { return "CMYK"; }
        virtual QString name() const { return i18n("CMYK (8-bit integer/channel)"); }

        virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KisCmykU8ColorSpace(parent, p); };

        virtual QString defaultProfile() { return "Adobe CMYK"; };
};


#endif
