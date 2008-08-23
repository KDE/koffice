/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_LCMS_COLORPROFILE_H
#define _KO_LCMS_COLORPROFILE_H

#include "KoIccColorProfile.h"

#include <lcms.h>

#include <QByteArray>
#include <QString>


struct KoRGBChromaticities;

/**
 * This class contains an LCMS color profile. Don't use it outside KoLcmsColorSpace.
 */
class PIGMENTCMS_EXPORT KoLcmsColorProfileContainer : public KoIccColorProfile::Container {
    friend class KoIccColorProfile;
protected:
    KoLcmsColorProfileContainer( KoIccColorProfile::Data *);
private:
    /**
     * Create a byte array from a lcms profile.
     */
    static QByteArray lcmsProfileToByteArray(const cmsHPROFILE profile);
    /**
     * Create a byte array holding an ICC profile generated with the given RGB Chromaticities
     */
    static QByteArray createFromChromacities(const KoRGBChromaticities& chromacities, qreal gamma, QString name);
public:
    /**
     * @param profile lcms memory structure with the profile, it is freed after the call
     *                to this function
     * @return an ICC profile created from an LCMS profile
     */
    static KoIccColorProfile* createFromLcmsProfile(const cmsHPROFILE profile);
public:

    virtual ~KoLcmsColorProfileContainer();

    /**
     * @return the ICC color space signature
     */
    icColorSpaceSignature colorSpaceSignature() const;
    /**
     * @return the class of the color space signature
     */
    icProfileClassSignature deviceClass() const;
    /**
     * @return the description of the profile
     */
    QString productDescription() const;
    /**
     * @return some information about the profile
     */
    QString productInfo() const;
    /**
     * @return the name of the manufacturer
     */
    QString manufacturer() const;

    /**
     * @return the structure to use with LCMS functions
     */
    cmsHPROFILE lcmsProfile() const;

    virtual bool valid() const;

    virtual bool isSuitableForOutput() const;

    virtual bool isSuitableForPrinting() const;

    virtual bool isSuitableForDisplay() const;
    virtual QString name() const;
    virtual QString info() const;

protected:
    KoLcmsColorProfileContainer();
//     void setProfile(const cmsHPROFILE profile);
private:
    KoRGBChromaticities* chromaticitiesFromProfile() const;

private:
    bool init();

    class Private;
    Private * const d;
};

#endif // KOCOLORPROFILE_H

