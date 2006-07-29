/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#ifndef KIS_FILTER_STRATEGY_H_
#define KIS_FILTER_STRATEGY_H_

#include <klocale.h>

#include "kis_types.h"
#include "kis_generic_registry.h"
#include "kis_id.h"

class KisFilterStrategy
{
    public:
        KisFilterStrategy(KisID id) : m_id(id) {}
        virtual ~KisFilterStrategy() {}

        KisID id() {return m_id;};
        virtual double valueAt(double /*t*/) const {return 0;};
        virtual Q_UINT32 intValueAt(Q_INT32 t) const {return Q_UINT32(255*valueAt(t/256.0));};
        double support() { return supportVal;};
        Q_UINT32 intSupport() { return intSupportVal;};
        virtual bool boxSpecial() { return false;};
    protected:
        double supportVal;
        Q_UINT32 intSupportVal;
        KisID m_id;
};

class KisHermiteFilterStrategy : public KisFilterStrategy
{
    public:
        KisHermiteFilterStrategy() : KisFilterStrategy(KisID("Hermite", i18n("Hermite")))
            {supportVal = 1.0; intSupportVal = 256;}
        virtual ~KisHermiteFilterStrategy() {}
        
        virtual Q_UINT32 intValueAt(Q_INT32 t) const;
        virtual double valueAt(double t) const;
};

class KisCubicFilterStrategy : public KisFilterStrategy
{
    public:
        KisCubicFilterStrategy() : KisFilterStrategy(KisID("Bicubic", i18n("Bicubic")))
            {supportVal = 1.0; intSupportVal = 256;}
        virtual ~KisCubicFilterStrategy() {}
        
        virtual Q_UINT32 intValueAt(Q_INT32 t) const;
        virtual double valueAt(double t) const;
};

class KisBoxFilterStrategy : public KisFilterStrategy
{
    public:
        KisBoxFilterStrategy() : KisFilterStrategy(KisID("Box", i18n("Box")))
             {supportVal = 0.5; intSupportVal = 128;}
        virtual ~KisBoxFilterStrategy() {}

        virtual Q_UINT32 intValueAt(Q_INT32 t) const;
        virtual double valueAt(double t) const;
        virtual bool boxSpecial() { return true;};
};

class KisTriangleFilterStrategy : public KisFilterStrategy
{
    public:
        KisTriangleFilterStrategy() : KisFilterStrategy(KisID("Triangle", i18n("Triangle aka (bi)linear")))
            {supportVal = 1.0; intSupportVal = 256;}
        virtual ~KisTriangleFilterStrategy() {}

        virtual Q_UINT32 intValueAt(Q_INT32 t) const;
        virtual double valueAt(double t) const;
};

class KisBellFilterStrategy : public KisFilterStrategy
{
    public:
        KisBellFilterStrategy() : KisFilterStrategy(KisID("Bell", i18n("Bell")))
            {supportVal = 1.5; intSupportVal = 128+256;}
        virtual ~KisBellFilterStrategy() {}

        virtual double valueAt(double t) const;
};

class KisBSplineFilterStrategy : public KisFilterStrategy
{
    public:
        KisBSplineFilterStrategy() : KisFilterStrategy(KisID("BSpline", i18n("BSpline")))
            {supportVal = 2.0; intSupportVal = 512;}
        virtual ~KisBSplineFilterStrategy() {}

        virtual double valueAt(double t) const;
};

class KisLanczos3FilterStrategy : public KisFilterStrategy
{
    public:
        KisLanczos3FilterStrategy() : KisFilterStrategy(KisID("Lanczos3", i18n("Lanczos3")))
            {supportVal = 3.0; intSupportVal = 768;}
        virtual ~KisLanczos3FilterStrategy() {}

        virtual double valueAt(double t) const;
    private:
        double sinc(double x) const; 
};

class KisMitchellFilterStrategy : public KisFilterStrategy
{
    public:
        KisMitchellFilterStrategy() : KisFilterStrategy(KisID("Mitchell", i18n("Mitchell")))
            {supportVal = 2.0; intSupportVal = 256;}
        virtual ~KisMitchellFilterStrategy() {}

        virtual double valueAt(double t) const;
};

class KisFilterStrategyRegistry : public KisGenericRegistry<KisFilterStrategy *>
{
public:
    virtual ~KisFilterStrategyRegistry();
    
    static KisFilterStrategyRegistry* instance();

private:
    KisFilterStrategyRegistry();
     KisFilterStrategyRegistry(const KisFilterStrategyRegistry&);
     KisFilterStrategyRegistry operator=(const KisFilterStrategyRegistry&);

private:
    static KisFilterStrategyRegistry *m_singleton;
};

#endif // KIS_FILTER_STRATEGY_H_
