/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007, 2010 Thomas Zander <zander@kde.org>
   Copyright 2009 Inge Wallin <inge@lysator.liu.se>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KOdfPageLayoutData.h"

#include <KDebug>

#include "KOdfXmlNS.h"
#include "KUnit.h"

KOdfGenericStyle KOdfPageLayoutData::saveOdf() const
{
    KOdfGenericStyle style(KOdfGenericStyle::PageLayoutStyle);

    // Save page dimension.
    style.addPropertyPt("fo:page-width", width);
    style.addPropertyPt("fo:page-height", height);

    qreal left = leftMargin;
    if (left < 0)
        left = pageEdge;
    qreal right = rightMargin;
    if (right < 0)
        right = bindingSide;

    // Save margins. If all margins are the same, only one value needs to be saved.
    if (left == topMargin && leftMargin == right && leftMargin == bottomMargin) {
        style.addPropertyPt("fo:margin", leftMargin);
    } else {
        style.addPropertyPt("fo:margin-left", left);
        style.addPropertyPt("fo:margin-right", right);
        style.addPropertyPt("fo:margin-top", topMargin);
        style.addPropertyPt("fo:margin-bottom", bottomMargin);
        if (left != leftMargin || right != rightMargin)
            style.addProperty("koffice:facing-pages", "true");
    }

    // Save padding. If all paddings are the same, only one value needs to be saved.
    if (leftPadding == topPadding && leftPadding == rightPadding && leftPadding == bottomPadding) {
        style.addPropertyPt("fo:padding", leftPadding);
    }
    else {
        style.addPropertyPt("fo:padding-left", leftPadding);
        style.addPropertyPt("fo:padding-right", rightPadding);
        style.addPropertyPt("fo:padding-top", topPadding);
        style.addPropertyPt("fo:padding-bottom", bottomPadding);
    }

    // If there are any page borders, add them to the style.
    border.saveOdf(style);

    style.addProperty("style:print-orientation",
                      (orientation == KOdfPageFormat::Landscape
                       ? "landscape" : "portrait"));
    return style;
}

void KOdfPageLayoutData::loadOdf(const KXmlElement &style)
{
    KXmlElement  properties(KoXml::namedItemNS(style, KOdfXmlNS::style,
                                                "page-layout-properties"));

    if (!properties.isNull()) {
        KOdfPageLayoutData standard;

        // Page dimension -- width / height
        width = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "page-width"),
                                   standard.width);
        height = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "page-height"),
                                    standard.height);

        // Page orientation
        if (properties.attributeNS(KOdfXmlNS::style, "print-orientation", QString()) == "portrait")
            orientation = KOdfPageFormat::Portrait;
        else
            orientation = KOdfPageFormat::Landscape;

        // Margins.  Check if there is one "margin" attribute and use it for all
        // margins if there is.  Otherwise load the individual margins.
        if (properties.hasAttributeNS(KOdfXmlNS::fo, "margin")) {
            leftMargin  = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "margin"));
            topMargin = leftMargin;
            rightMargin = leftMargin;
            bottomMargin = leftMargin;
        } else {
            /*
                If one of the individual margins is specified then the default for the others is zero.
                Otherwise all of them are set to 20mm.
            */
            qreal defaultValue = 0;
            if (!(properties.hasAttributeNS(KOdfXmlNS::fo, "margin-left")
                    || properties.hasAttributeNS(KOdfXmlNS::fo, "margin-top")
                    || properties.hasAttributeNS(KOdfXmlNS::fo, "margin-right")
                    || properties.hasAttributeNS(KOdfXmlNS::fo, "margin-bottom")))
                defaultValue = MM_TO_POINT(20.0); // no margin specified at all, lets make it 20mm

            leftMargin   = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "margin-left"), defaultValue);
            topMargin    = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "margin-top"), defaultValue);
            rightMargin  = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "margin-right"), defaultValue);
            bottomMargin = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "margin-bottom"), defaultValue);
        }
        if (properties.attributeNS(KOdfXmlNS::koffice, "facing-pages", "false")
                .compare("true", Qt::CaseInsensitive) == 0) {
            pageEdge = leftMargin;
            leftMargin = -1;
            bindingSide = rightMargin;
            rightMargin = -1;
        }

        // Padding.  Same reasoning as for margins
        if (properties.hasAttributeNS(KOdfXmlNS::fo, "padding")) {
            leftPadding  = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "padding"));
            topPadding = leftPadding;
            rightPadding = leftPadding;
            bottomPadding = leftPadding;
        }
        else {
            leftPadding   = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "padding-left"));
            topPadding    = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "padding-top"));
            rightPadding  = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "padding-right"));
            bottomPadding = KUnit::parseValue(properties.attributeNS(KOdfXmlNS::fo, "padding-bottom"));
        }

        // Parse border properties if there are any.
        border.loadOdf(properties);

        // guessFormat takes millimeters
        if (orientation == KOdfPageFormat::Landscape)
            format = KOdfPageFormat::guessFormat(POINT_TO_MM(height), POINT_TO_MM(width));
        else
            format = KOdfPageFormat::guessFormat(POINT_TO_MM(width), POINT_TO_MM(height));
    }
}

bool KOdfPageLayoutData::operator==(const KOdfPageLayoutData &l) const
{
    return qFuzzyCompare(width,l.width)
        && qFuzzyCompare(height,l.height)
        && qFuzzyCompare(leftMargin,l.leftMargin)
        && qFuzzyCompare(rightMargin,l.rightMargin)
        && qFuzzyCompare(topMargin,l.topMargin)
        && qFuzzyCompare(bottomMargin,l.bottomMargin)
        && qFuzzyCompare(pageEdge,l.pageEdge)
        && qFuzzyCompare(bindingSide,l.bindingSide)
        && border == l.border;
}

bool KOdfPageLayoutData::operator!=(const KOdfPageLayoutData& l) const
{
    return !((*this) == l);
}

KOdfPageLayoutData::KOdfPageLayoutData()
: format(KOdfPageFormat::defaultFormat())
, orientation(KOdfPageFormat::Portrait)
, width(MM_TO_POINT(KOdfPageFormat::width(format, orientation)))
, height(MM_TO_POINT(KOdfPageFormat::height(format, orientation)))
, leftMargin(MM_TO_POINT(20.0))
, rightMargin(MM_TO_POINT(20.0))
, topMargin(MM_TO_POINT(20.0))
, bottomMargin(MM_TO_POINT(20.0))
, pageEdge(-1)
, bindingSide(-1)
, leftPadding(0)
, rightPadding(0)
, topPadding(0)
, bottomPadding(0)
{
    border.setLeftBorderStyle(KOdfBorders::BorderNone);
    border.setTopBorderStyle(KOdfBorders::BorderNone);
    border.setRightBorderStyle(KOdfBorders::BorderNone);
    border.setBottomBorderStyle(KOdfBorders::BorderNone);
}
