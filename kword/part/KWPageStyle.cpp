/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
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

#include "KWPageStyle.h"
#include "KWPageStyle_p.h"

#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KUnit.h>
#include <KColorBackground.h>
#include <KPatternBackground.h>
#include <KImageCollection.h>
#include <KOdfLoadingContext.h>

#include <kdebug.h>
#include <QBuffer>
#include <QColor>

KWPageStylePrivate::~KWPageStylePrivate()
{
    if (fullPageBackground && !fullPageBackground->deref()) {
        delete fullPageBackground;
    }
}

void KWPageStylePrivate::clear()
{
    // defaults
    footNoteSeparatorLineLength = 20; // 20%, i.e. 1/5th
    footNoteSeparatorLineWidth = 0.5; // like in OOo
    footNoteSeparatorLineType = Qt::SolidLine;

    mainFrame = true;
    headerMargin.bottom = MM_TO_POINT(5);
    footerMargin.top = MM_TO_POINT(5);
    headerMinimumHeight = MM_TO_POINT(10);
    footerMinimumHeight = MM_TO_POINT(10);
    footNoteDistance = 10;
    endNoteDistance = 10;
    headers = KWord::HFTypeNone;
    footers = KWord::HFTypeNone;
    columns.columns = 1;
    columns.columnSpacing = MM_TO_POINT(6);
    direction = KoText::AutoDirection;

    if (fullPageBackground && !fullPageBackground->deref()) {
        delete fullPageBackground;
    }
    fullPageBackground = 0;
    nextStyleName.clear();
    fixedHeaderSize = false;
    fixedFooterSize = false;
}

static void saveInsets(const KoInsets insets, KOdfGenericStyle &style, const QString &prefix, KOdfGenericStyle::PropertyType type)
{
    if (insets.left == insets.right && insets.top == insets.bottom && insets.left == insets.top) {
        if (qAbs(insets.top) > 1E-4)
            style.addPropertyPt(prefix, insets.top, type);
    } else {
        if (qAbs(insets.left) > 1E-4)
            style.addPropertyPt(prefix + "-left", insets.left, type);
        if (qAbs(insets.top) > 1E-4)
            style.addPropertyPt(prefix + "-top", insets.top, type);
        if (qAbs(insets.bottom) > 1E-4)
            style.addPropertyPt(prefix + "-bottom", insets.bottom, type);
        if (qAbs(insets.right) > 1E-4)
            style.addPropertyPt(prefix + "-right", insets.right, type);
    }
}


///////////

KWPageStyle::KWPageStyle(const QString &name)
    : d (new KWPageStylePrivate())
{
    d->name = name;
}

KWPageStyle::KWPageStyle(const KWPageStyle &ps)
    : d(ps.d)
{
}

KWPageStyle::KWPageStyle()
{
}

bool KWPageStyle::isValid() const
{
    return d && !d->name.isEmpty();
}


KWPageStyle &KWPageStyle::operator=(const KWPageStyle &ps)
{
    d = ps.d;
    return *this;
}

KWPageStyle::~KWPageStyle()
{
}

void KWPageStyle::setFooterPolicy(KWord::HeaderFooterType p)
{
    d->footers = p;
}

void KWPageStyle::setHeaderPolicy(KWord::HeaderFooterType p)
{
    d->headers = p;
}

const KOdfPageLayoutData &KWPageStyle::pageLayout() const
{
    return d->pageLayout;
}

void KWPageStyle::setPageLayout(const KOdfPageLayoutData &pageLayout)
{
    d->pageLayout = pageLayout;
}

const KOdfColumnData &KWPageStyle::columns() const
{
    return d->columns;
}

void KWPageStyle::setColumns(const KOdfColumnData &columns)
{
    d->columns = columns;
}

KWord::HeaderFooterType KWPageStyle::headerPolicy() const
{
    return d->headers;
}

KWord::HeaderFooterType KWPageStyle::footerPolicy() const
{
    return d->footers;
}

void KWPageStyle::setHasMainTextFrame(bool on)
{
    d->mainFrame = on;
}

bool KWPageStyle::hasMainTextFrame() const
{
    return d->mainFrame;
}

qreal KWPageStyle::headerDistance() const
{
    return d->headerMargin.bottom;
}

void KWPageStyle::setHeaderDistance(qreal distance)
{
    d->headerMargin.bottom = distance;
}

qreal KWPageStyle::headerMinimumHeight() const
{
    return d->headerMinimumHeight;
}

void KWPageStyle::setHeaderMinimumHeight(qreal height)
{
    d->headerMinimumHeight = height;
}

qreal KWPageStyle::footerMinimumHeight() const
{
    return d->footerMinimumHeight;
}

void KWPageStyle::setFooterMinimumHeight(qreal height)
{
    d->footerMinimumHeight = height;
}

qreal KWPageStyle::footerDistance() const
{
    return d->footerMargin.top;
}

void KWPageStyle::setFooterDistance(qreal distance)
{
    d->footerMargin.top = distance;
}

qreal KWPageStyle::footnoteDistance() const
{
    return d->footNoteDistance;
}

void KWPageStyle::setFootnoteDistance(qreal distance)
{
    d->footNoteDistance = distance;
}

qreal KWPageStyle::endNoteDistance() const
{
    return d->endNoteDistance;
}

void KWPageStyle::setEndNoteDistance(qreal distance)
{
    d->endNoteDistance = distance;
}

int KWPageStyle::footNoteSeparatorLineLength() const
{
    return d->footNoteSeparatorLineLength;
}

void KWPageStyle::setFootNoteSeparatorLineLength(int length)
{
    d->footNoteSeparatorLineLength = length;
}

qreal KWPageStyle::footNoteSeparatorLineWidth() const
{
    return d->footNoteSeparatorLineWidth;
}

void KWPageStyle::setFootNoteSeparatorLineWidth(qreal width)
{
    d->footNoteSeparatorLineWidth = width;
}

Qt::PenStyle KWPageStyle::footNoteSeparatorLineType() const
{
    return d->footNoteSeparatorLineType;
}

void KWPageStyle::setFootNoteSeparatorLineType(Qt::PenStyle type)
{
    d->footNoteSeparatorLineType = type;
}

KWord::FootNoteSeparatorLinePos KWPageStyle::footNoteSeparatorLinePosition() const
{
    return d->footNoteSeparatorLinePos;
}

void KWPageStyle::setFootNoteSeparatorLinePosition(KWord::FootNoteSeparatorLinePos position)
{
    d->footNoteSeparatorLinePos = position;
}

void KWPageStyle::clear()
{
    d->clear();
}

QString KWPageStyle::name() const
{
    return d->name;
}

KoShapeBackground *KWPageStyle::background() const
{
    return d->fullPageBackground;
}

void KWPageStyle::setBackground(KoShapeBackground *background)
{
    if (d->fullPageBackground) {
        if (!d->fullPageBackground->deref())
            delete d->fullPageBackground;
    }
    d->fullPageBackground = background;
    if (d->fullPageBackground)
        d->fullPageBackground->ref();
}

KOdfGenericStyle KWPageStyle::saveOdf() const
{
    KOdfGenericStyle pageLayout = d->pageLayout.saveOdf();
    pageLayout.setAutoStyleInStylesDotXml(true);
    pageLayout.addAttribute("style:page-usage", "all");

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KXmlWriter writer(&buffer);

    if (d->columns.columns > 1) {
        writer.startElement("style:columns");
        writer.addAttribute("fo:column-count", d->columns.columns);
        writer.addAttributePt("fo:column-gap", d->columns.columnSpacing);
        writer.endElement();
    }

    //<style:footnote-sep style:adjustment="left" style:width="0.5pt" style:rel-width="20%" style:line-style="solid"/>
    //writer.startElement("style:footnote-sep");
    // TODO
    //writer.addAttribute("style:adjustment",)
    //writer.addAttribute("style:width",)
    //writer.addAttribute("style:rel-width",)
    //writer.addAttribute("style:line-style",)
    //writer.endElement();

    // TODO save background

    QString contentElement = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    pageLayout.addChildElement("columnsEnzo", contentElement);


    if (headerPolicy() != KWord::HFTypeNone) {
        pageLayout.addProperty("style:dynamic-spacing", d->fixedHeaderSize, KOdfGenericStyle::PageHeaderType);
        pageLayout.addPropertyPt("fo:min-height", d->headerMinimumHeight, KOdfGenericStyle::PageHeaderType);
        saveInsets(d->headerMargin, pageLayout, "fo:margin", KOdfGenericStyle::PageHeaderType);
        saveInsets(d->headerInsets, pageLayout, "fo:padding", KOdfGenericStyle::PageHeaderType);
    }

    if (footerPolicy() != KWord::HFTypeNone) {
        pageLayout.addProperty("style:dynamic-spacing", d->fixedFooterSize, KOdfGenericStyle::PageFooterType);
        pageLayout.addPropertyPt("fo:min-height", d->footerMinimumHeight, KOdfGenericStyle::PageFooterType);
        saveInsets(d->footerMargin, pageLayout, "fo:margin", KOdfGenericStyle::PageFooterType);
        saveInsets(d->footerInsets, pageLayout, "fo:padding", KOdfGenericStyle::PageFooterType);
    }

    // TODO see how we should save margins if we use the 'closest to binding' stuff.

    return pageLayout;
}

void KWPageStyle::loadOdf(KOdfLoadingContext &context, const KXmlElement &masterNode, const KXmlElement &style, KoResourceManager *documentResources)
{
    d->pageLayout.loadOdf(style);
    KXmlElement props = KoXml::namedItemNS(style, KOdfXmlNS::style, "page-layout-properties");
    if (props.isNull())
        return;
    QString direction = props.attributeNS(KOdfXmlNS::style, "writing-mode", "lr-tb");
    d->direction = KoText::directionFromString(direction);

    KXmlElement columns = KoXml::namedItemNS(props, KOdfXmlNS::style, "columns");
    if (!columns.isNull()) {
        d->columns.columns = columns.attributeNS(KOdfXmlNS::fo, "column-count", "15").toInt();
        if (d->columns.columns < 1)
            d->columns.columns = 1;
        d->columns.columnSpacing = KUnit::parseValue(columns.attributeNS(KOdfXmlNS::fo, "column-gap"));
    } else {
        d->columns.columns = 1;
        d->columns.columnSpacing = 17; // ~ 6mm
    }

    d->headerMargin = KoInsets();
    d->headerInsets = KoInsets();
    d->headerMinimumHeight = 0;
    d->fixedHeaderSize = true;
    KXmlElement header = KoXml::namedItemNS(style, KOdfXmlNS::style, "header-style");
    if (! header.isNull()) {
        KXmlElement hfprops = KoXml::namedItemNS(header, KOdfXmlNS::style, "header-footer-properties");
        if (!hfprops.isNull()) {
            d->headerMargin.fillFrom(hfprops, KOdfXmlNS::fo, "margin");
            d->headerInsets.fillFrom(hfprops, KOdfXmlNS::fo, "padding");
            d->headerMinimumHeight = KUnit::parseValue(hfprops.attributeNS(KOdfXmlNS::fo, "min-height"));
            d->fixedHeaderSize = hfprops.attributeNS(KOdfXmlNS::style,
                    "dynamic-spacing").compare("true", Qt::CaseInsensitive) == 0;
        // TODO there are quite some more properties we want to at least preserve between load and save
          //fo:background-color
           // and a possible style::background-image element
          //fo:border
          //fo:border-bottom
          //fo:border-left
          //fo:border-right
          //fo:border-top
          //svg:height
          //style:border-line-width
          //style:border-line-width-bottom
          //style:border-line-width-left
          //style:border-line-width-right
          //style:border-line-width-top
          //style:shadow
        }
    }

    d->footerMargin = KoInsets();
    d->footerInsets = KoInsets();
    d->footerMinimumHeight = 0;
    d->fixedFooterSize = true;
    KXmlElement footer = KoXml::namedItemNS(style, KOdfXmlNS::style, "footer-style");
    if (! footer.isNull()) {
        KXmlElement hfprops = KoXml::namedItemNS(footer, KOdfXmlNS::style, "header-footer-properties");
        if (!hfprops.isNull()) {
            d->footerMargin.fillFrom(hfprops, KOdfXmlNS::fo, "margin");
            d->footerInsets.fillFrom(hfprops, KOdfXmlNS::fo, "padding");
            d->footerMinimumHeight = KUnit::parseValue(hfprops.attributeNS(KOdfXmlNS::fo, "min-height"));
            d->fixedFooterSize = hfprops.attributeNS(KOdfXmlNS::style,
                    "dynamic-spacing").compare("true", Qt::CaseInsensitive) == 0;
        }
    }

    // Load background picture
    KXmlElement propBackgroundImage = KoXml::namedItemNS(props, KOdfXmlNS::style, "background-image");
    if (!propBackgroundImage.isNull()) {
        const QString href = propBackgroundImage.attributeNS(KOdfXmlNS::xlink, "href", QString());
        if (!href.isEmpty()) {
            KPatternBackground *background = new KPatternBackground(documentResources->imageCollection());
            d->fullPageBackground = background;
            d->fullPageBackground->ref();

            KImageCollection *imageCollection = documentResources->imageCollection();
            if (imageCollection != 0) {
                KImageData *imageData = imageCollection->createImageData(href,context.store());
                background->setPattern(imageData);
            }
        }
        // TODO load another possible attributes
    }

    // Load background color
    QString backgroundColor = props.attributeNS(KOdfXmlNS::fo, "background-color", QString::null);
    if (!backgroundColor.isNull() && d->fullPageBackground == 0) {

        if (backgroundColor == "transparent") {
            d->fullPageBackground = 0;
        }
        else {
            d->fullPageBackground = new KColorBackground(QColor(backgroundColor));
            d->fullPageBackground->ref();
        }
    }

    // Load next master-page style name
    d->nextStyleName = masterNode.attributeNS(KOdfXmlNS::style, "next-style-name", QString());
}

QString KWPageStyle::nextStyleName() const
{
    return d->nextStyleName;
}

void KWPageStyle::setNextStyleName(const QString &nextStyleName)
{
    d->nextStyleName = nextStyleName;
}

KoText::Direction KWPageStyle::direction() const
{
    return d->direction;
}

void KWPageStyle::setDirection(KoText::Direction direction)
{
    d->direction = direction;
}

void KWPageStyle::setFixedHeaderSize(bool on)
{
    d->fixedHeaderSize = on;
}

bool KWPageStyle::hasFixedHeaderSize() const
{
    return d->fixedHeaderSize;
}

void KWPageStyle::setFixedFooterSize(bool on)
{
    d->fixedFooterSize = on;
}

bool KWPageStyle::hasFixedFooterSize() const
{
    return d->fixedFooterSize;
}


bool KWPageStyle::operator==(const KWPageStyle &other) const
{
    return d == other.d;
}

KWPageStylePrivate *KWPageStyle::priv()
{
    return d.data();
}

const KWPageStylePrivate *KWPageStyle::priv() const
{
    return d.data();
}

uint KWPageStyle::hash() const
{
    return ((uint) d) + 1;
}

bool KWPageStyle::isPageSpread() const
{
    return d->pageLayout.leftMargin < 0;
}

uint qHash(const KWPageStyle &style)
{
    return style.hash();
}

void KWPageStyle::detach(const QString &newName)
{
    if (d->fullPageBackground)
        d->fullPageBackground->ref();
    d.detach();
    d->name = newName;
}
