/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_ICC_COLOR_PROFILE_H_
#define _KO_ICC_COLOR_PROFILE_H_

#include "KoColorProfile.h"

class KoLcmsColorProfileContainer;
struct KoRGBChromaticities;

/**
 * This class contains an ICC color profile.
 */
class PIGMENTCMS_EXPORT KoIccColorProfile : public KoColorProfile {
    public:

        using KoColorProfile::save;
        
        /**
         * Contains the data associated with a profile. This is
         * shared through internal representation.
         */
        class Data {
            public:
                Data();
                Data(QByteArray rawData);
                ~Data();
                QByteArray rawData();
                void setRawData( const QByteArray &);
            private:
                struct Private;
                Private* const d;
        };
        /**
         * This class should be used to wrap the ICC profile
         * representation coming from various CMS engine.
         */
        class Container {
            public:
                Container();
                virtual ~Container();
            public:
                virtual QString name() const =0;
                virtual QString info() const =0;
                virtual bool valid() const =0;
                virtual bool isSuitableForOutput() const =0;
                virtual bool isSuitableForPrinting() const =0;
                virtual bool isSuitableForDisplay() const =0;
        };
    public:
        KoIccColorProfile(const KoRGBChromaticities& chromacities, qreal gamma, QString name = "");
        KoIccColorProfile(QString fileName = "");
        KoIccColorProfile(const QByteArray& rawData);
        KoIccColorProfile(const KoIccColorProfile& rhs);
        virtual ~KoIccColorProfile();
        
        virtual KoColorProfile* clone() const;
        
        virtual bool load();
        virtual bool save();

        /**
        * @return an array with the raw data of the profile
        */
        QByteArray rawData() const;
        virtual bool valid() const;
        virtual bool isSuitableForOutput() const;
        virtual bool isSuitableForPrinting() const;
        virtual bool isSuitableForDisplay() const;
        virtual bool operator==(const KoColorProfile&) const;
    protected:
        void setRawData(const QByteArray& rawData);
    public:
        KoLcmsColorProfileContainer* asLcms() const;
    public:
        /**
        *
        * XXX: We need to make sure we always have the right profile even
        *      when running on multiple screens.
        *
        * @return the color profile of the screen.
        */
        static KoIccColorProfile *getScreenProfile(int screen = -1);
    protected:
        virtual bool init();
    private:
        struct Private;
        Private* const d;
};

#endif
