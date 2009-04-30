/* This file is part of the KDE project
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
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

#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>

#include <kdebug.h>
#include <QBuffer>

void KWPageStylePrivate::clear()
{
    // defaults
    footNoteSeparatorLineLength = 20; // 20%, i.e. 1/5th
    footNoteSeparatorLineWidth = 0.5; // like in OOo
    footNoteSeparatorLineType = Qt::SolidLine;

    mainFrame = true;
    headerDistance = 10; // ~3mm
    footerDistance = 10;
    footNoteDistance = 10;
    endNoteDistance = 10;
    headers = KWord::HFTypeNone;
    footers = KWord::HFTypeNone;
    columns.columns = 1;
    columns.columnSpacing = 17; // ~ 6mm
    pageLayout = KoPageLayout::standardLayout();
}

///////////

KWPageStyle::KWPageStyle(const QString& name)
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

const KoPageLayout KWPageStyle::pageLayout() const
{
    return d->pageLayout;
}

void KWPageStyle::setPageLayout(const KoPageLayout &pageLayout)
{
    d->pageLayout = pageLayout;
}

KoColumns KWPageStyle::columns() const
{
    return d->columns;
}

void KWPageStyle::setColumns(const KoColumns &columns)
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
    return d->headerDistance;
}

void KWPageStyle::setHeaderDistance(qreal distance)
{
    d->headerDistance = distance;
}

qreal KWPageStyle::footerDistance() const
{
    return d->footerDistance;
}

void KWPageStyle::setFooterDistance(qreal distance)
{
    d->footerDistance = distance;
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

KoGenStyle KWPageStyle::saveOdf() const
{
    KoGenStyle pageLayout = d->pageLayout.saveOdf();
    pageLayout.setAutoStyleInStylesDotXml(true);
    pageLayout.addAttribute("style:page-usage", "all");

    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter writer( &buffer );

    if (d->columns.columns > 1) {
        writer.startElement("style:columns");
        writer.addAttribute("fo:column-count", d->columns.columns);
        writer.addAttributePt("fo:column-gap", d->columns.columnSpacing);
        writer.endElement();
    }

    //<style:footnote-sep style:adjustment="left" style:width="0.5pt" style:rel-width="20%" style:line-style="solid"/>
    //writer.startElement("style:footnote-sep");
    // TODO
    //writer.addAttribute("style:adjustment", )
    //writer.addAttribute("style:width", )
    //writer.addAttribute("style:rel-width", )
    //writer.addAttribute("style:line-style", )
    //writer.endElement();

    QString contentElement = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    pageLayout.addChildElement("columnsEnzo", contentElement);

// the header/footer-style should be saved as a child of the style:page-layout; but using the
// addChildElement its instead saved as a child of style:page-layout-properties  I can't follow why...
// so lets disable this until I figure out how to save this in the right position in the tree.
#if 0
    if (headerPolicy() != KWord::HFTypeNone) {
        writer.startElement("style:header-style");
        writer.startElement("style:header-footer-properties");
        writer.addAttribute("fo:min-height", "0.01pt");
        writer.addAttributePt("fo:margin-bottom", headerDistance());
        // TODO there are quite some more properties we want to at least preserve between load and save
        writer.endElement();
        writer.endElement();
    }
    if (footerPolicy() != KWord::HFTypeNone) {
        writer.startElement("style:footer-style");
        writer.startElement("style:header-footer-properties");
        writer.addAttribute("fo:min-height", "0.01pt");
        writer.addAttributePt("fo:margin-top", footerDistance());
        // TODO there are quite some more properties we want to at least preserve between load and save
        writer.endElement();
        writer.endElement();
    }
#endif

    // TODO see how we should save margins if we use the 'closest to binding' stuff.

    return pageLayout;
}

void KWPageStyle::loadOdf(const KoXmlElement &style)
{
    d->pageLayout.loadOdf(style);
    KoXmlElement props = KoXml::namedItemNS(style, KoXmlNS::style, "page-layout-properties");
    if (props.isNull())
        return;
    KoXmlElement columns = KoXml::namedItemNS(props, KoXmlNS::style, "columns");
    if (!columns.isNull()) {
        d->columns.columns = columns.attributeNS(KoXmlNS::fo, "column-count", "15").toInt();
        if (d->columns.columns < 1)
            d->columns.columns = 1;
        d->columns.columnSpacing = KoUnit::parseValue(columns.attributeNS(KoXmlNS::fo, "column-gap"));
    } else {
        d->columns.columns = 1;
        d->columns.columnSpacing = 17; // ~ 6mm
    }

    KoXmlElement header = KoXml::namedItemNS(style, KoXmlNS::style, "header-style");
    if (! header.isNull()) {
        KoXmlElement hfprops = KoXml::namedItemNS(header, KoXmlNS::style, "header-footer-properties");
        if (! hfprops.isNull())
            d->headerDistance = KoUnit::parseValue(hfprops.attributeNS(KoXmlNS::fo, "margin-bottom"));
        // TODO there are quite some more properties we want to at least preserve between load and save
    }

    KoXmlElement footer = KoXml::namedItemNS(style, KoXmlNS::style, "footer-style");
    if (! footer.isNull()) {
        KoXmlElement hfprops = KoXml::namedItemNS(footer, KoXmlNS::style, "header-footer-properties");
        if (! hfprops.isNull())
            d->footerDistance = KoUnit::parseValue(hfprops.attributeNS(KoXmlNS::fo, "margin-top"));
        // TODO there are quite some more properties we want to at least preserve between load and save
    }
}

bool KWPageStyle::operator==(const KWPageStyle &other) const
{
    return d == other.d;
}

const KWPageStylePrivate *KWPageStyle::priv() const
{
    return d.data();
}

uint KWPageStyle::hash() const
{
    return ((uint) d) + 1;
}

uint qHash(const KWPageStyle &style)
{
    return style.hash();
}
