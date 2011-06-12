/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2002 Laurent Montel <montel@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Phillip Mueller <philipp.mueller@gmx.de>
   Copyright 2000 Werner Trobin <trobin@kde.org>
   Copyright 1999-2000 Simon Hausmann <hausmann@kde.org>
   Copyright 1999 David Faure <faure@kde.org>
   Copyright 1998-2000 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCCalculationSettings.h"

#include "KCLocalization.h"

#include <KOdfXmlNS.h>

#include <kdebug.h>

class KCCalculationSettings::Private
{
public:
    KLocale* locale;
    bool caseSensitiveComparisons : 1;
    bool precisionAsShown         : 1;
    bool wholeCellSearchCriteria  : 1;
    bool automaticFindLabels      : 1;
    bool useRegularExpressions    : 1;
    bool automaticCalculation     : 1;
    int refYear; // the reference year two-digit years are relative to
    QDate refDate; // the reference date all dates are relative to
    // The precision used for decimal numbers, if the default cell style's
    // precision is set to arbitrary.
    int precision;
    QString fileName;
};

/*****************************************************************************
 *
 * KCCalculationSettings
 *
 *****************************************************************************/

KCCalculationSettings::KCCalculationSettings()
        : d(new Private)
{
    d->locale = new KCLocalization();
    d->caseSensitiveComparisons = true;
    d->precisionAsShown         = false;
    d->wholeCellSearchCriteria  = true;
    d->automaticFindLabels      = true;
    d->useRegularExpressions    = true;
    d->automaticCalculation     = true;
    d->refYear = 1930;
    d->refDate = QDate(1899, 12, 30);
    d->precision = -1;
}

KCCalculationSettings::~KCCalculationSettings()
{
    delete d->locale;
    delete d;
}

void KCCalculationSettings::loadOdf(const KoXmlElement& body)
{
    KoXmlNode settings = KoXml::namedItemNS(body, KOdfXmlNS::table, "calculation-settings");
    kDebug() << "Calculation settings found?" << !settings.isNull();
    if (!settings.isNull()) {
        KoXmlElement element = settings.toElement();
        if (element.hasAttributeNS(KOdfXmlNS::table,  "case-sensitive")) {
            d->caseSensitiveComparisons = true;
            QString value = element.attributeNS(KOdfXmlNS::table, "case-sensitive", "true");
            if (value == "false")
                d->caseSensitiveComparisons = false;
        } else if (element.hasAttributeNS(KOdfXmlNS::table, "precision-as-shown")) {
            d->precisionAsShown = false;
            QString value = element.attributeNS(KOdfXmlNS::table, "precision-as-shown", "false");
            if (value == "true")
                d->precisionAsShown = true;
        } else if (element.hasAttributeNS(KOdfXmlNS::table, "search-criteria-must-apply-to-whole-cell")) {
            d->wholeCellSearchCriteria = true;
            QString value = element.attributeNS(KOdfXmlNS::table, "search-criteria-must-apply-to-whole-cell", "true");
            if (value == "false")
                d->wholeCellSearchCriteria = false;
        } else if (element.hasAttributeNS(KOdfXmlNS::table, "automatic-find-labels")) {
            d->automaticFindLabels = true;
            QString value = element.attributeNS(KOdfXmlNS::table, "automatic-find-labels", "true");
            if (value == "false")
                d->automaticFindLabels = false;
        } else if (element.hasAttributeNS(KOdfXmlNS::table, "use-regular-expressions")) {
            d->useRegularExpressions = true;
            QString value = element.attributeNS(KOdfXmlNS::table, "use-regular-expressions", "true");
            if (value == "false")
                d->useRegularExpressions = false;
        } else if (element.hasAttributeNS(KOdfXmlNS::table, "null-year")) {
            d->refYear = 1930;
            QString value = element.attributeNS(KOdfXmlNS::table, "null-year", "1930");
            if (value == "false")
                d->refYear = false;
        }

        forEachElement(element, settings) {
            if (element.namespaceURI() != KOdfXmlNS::table)
                continue;
            else if (element.tagName() ==  "null-date") {
                d->refDate = QDate(1899, 12, 30);
                QString valueType = element.attributeNS(KOdfXmlNS::table, "value-type", "date");
                if (valueType == "date") {
                    QString value = element.attributeNS(KOdfXmlNS::table, "date-value", "1899-12-30");
                    QDate date = QDate::fromString(value, Qt::ISODate);
                    if (date.isValid())
                        d->refDate = date;
                } else {
                    kDebug() << "KCCalculationSettings: Error on loading null date."
                    << "KCValue type """ << valueType << """ not handled"
                    << ", falling back to default." << endl;
                    // NOTE Stefan: I don't know why different types are possible here!
                }
            } else if (element.tagName() ==  "iteration") {
                // TODO
            }
        }
    }
}

bool KCCalculationSettings::saveOdf(KoXmlWriter &/*settingsWriter*/) const
{
    return true;
}

KLocale* KCCalculationSettings::locale() const
{
    return d->locale;
}

void KCCalculationSettings::setReferenceYear(int year)
{
    if (year < 100)
        d->refYear = 1900 + year;
    else
        d->refYear = year;
}

int KCCalculationSettings::referenceYear() const
{
    return d->refYear;
}

void KCCalculationSettings::setReferenceDate(const QDate& date)
{
    if (!date.isValid()) return;
    d->refDate.setDate(date.year(), date.month(), date.day());
}

QDate KCCalculationSettings::referenceDate() const
{
    return d->refDate;
}

void KCCalculationSettings::setDefaultDecimalPrecision(int precision)
{
    d->precision = precision;
}

int KCCalculationSettings::defaultDecimalPrecision() const
{
    return d->precision;
}

void KCCalculationSettings::setFileName(const QString& fileName)
{
    d->fileName = fileName;
}

const QString& KCCalculationSettings::fileName() const
{
    return d->fileName;
}

void KCCalculationSettings::setAutoCalculationEnabled(bool enable)
{
    d->automaticCalculation = enable;
}

bool KCCalculationSettings::isAutoCalculationEnabled() const
{
    return d->automaticCalculation;
}
