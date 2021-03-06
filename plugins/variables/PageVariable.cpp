/* This file is part of the KDE project
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2011 Thomas Zanader <zander@kde.org>
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

#include "PageVariable.h"

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KProperties.h>
#include <KOdfXmlNS.h>
#include <KShape.h>
#include <KShapeSavingContext.h>
#include <KShapeLoadingContext.h>

#include <kdebug.h>

PageVariable::PageVariable()
        : KVariable(true),
        m_type(PageNumber),
        m_pageselect(KTextPage::CurrentPage),
        m_pageadjust(0),
        m_fixed(false)
{
}

void PageVariable::readProperties(const KProperties *props)
{
    switch (props->intProperty("vartype")) {
    case 1:
        m_type = PageCount;
        break;
    case 2:
        m_type = PageNumber;
        break;
    case 3:
        m_type = PageContinuation;
        break;
    default:
        kWarning() << "property 'vartype' has to be 1, 2 or 3. Ignoring";
        break;
    }
}

void PageVariable::propertyChanged(Property property, const QVariant &value)
{
    if (m_type == PageCount && property == KInlineObject::PageCount)
        setValue(value.toString());
}

void PageVariable::positionChanged()
{
    switch (m_type) {
    case PageCount:
        break;
    case PageNumber:
        if (value().isEmpty() || ! m_fixed) {
            KTextPage *thePage = page();
            int pageNumber = 0;
            if (thePage)
                pageNumber = thePage->pageNumber(m_pageselect, m_pageadjust);
            setValue(pageNumber > 0 ? QString::number(pageNumber) : QString());
        }
        break;
    case PageContinuation: {
        KTextPage *thePage = page();
        int pageNumber = 0;
        if (thePage)
            pageNumber = thePage->pageNumber(m_pageselect);
        setValue(pageNumber >= 0 ? m_continuation : QString());
        break;
    }
    }
}

void PageVariable::saveOdf(KShapeSavingContext &context)
{
    KXmlWriter *writer = &context.xmlWriter();
    switch (m_type) {
    case PageCount:
        // <text:page-count>3</text:page-count>
        writer->startElement("text:page-count", false);
        writer->addTextNode(value());
        writer->endElement();
        break;
    case PageNumber:
        // <text:page-number text:select-page="current" text:page-adjust="2" text:fixed="true">3</text:page-number>
        writer->startElement("text:page-number", false);

        if (m_pageselect == KTextPage::CurrentPage)
            writer->addAttribute("text:select-page", "current");
        else if (m_pageselect == KTextPage::PreviousPage)
            writer->addAttribute("text:select-page", "previous");
        else if (m_pageselect == KTextPage::NextPage)
            writer->addAttribute("text:select-page", "next");

        if (m_pageadjust != 0)
            writer->addAttribute("text:page-adjust", QString::number(m_pageadjust));

        if (m_fixed)
            writer->addAttribute("text:fixed", "true");

        writer->addTextNode(value());
        writer->endElement();
        break;
    case PageContinuation:
        // <text:page-continuation-string text:select-page="previous">The Text</text:page-continuation-string>
        writer->startElement("page-continuation-string", false);

        if (m_pageselect == KTextPage::PreviousPage)
            writer->addAttribute("text:select-page", "previous");
        else if (m_pageselect == KTextPage::NextPage)
            writer->addAttribute("text:select-page", "next");

        writer->addTextNode(m_continuation);
        writer->endElement();
        break;
    }
}

bool PageVariable::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_UNUSED(context);
    const QString localName(element.localName());
    if (localName == "page-count") {
        m_type = PageCount;
    } else if (localName == "page-number") {
        m_type = PageNumber;

        // The text:select-page attribute is used to display the number of the previous or the following
        // page rather than the number of the current page.
        QString pageselect = element.attributeNS(KOdfXmlNS::text, "select-page");
        if (pageselect == "previous")
            m_pageselect = KTextPage::PreviousPage;
        else if (pageselect == "next")
            m_pageselect = KTextPage::NextPage;
        else // "current"
            m_pageselect = KTextPage::CurrentPage;

        // The value of a page number field can be adjusted by a specified number, allowing the display
        // of page numbers of following or preceding pages. The adjustment amount is specified using
        // the text:page-adjust attribute.
        m_pageadjust = element.attributeNS(KOdfXmlNS::text, "page-adjust").toInt();

        // The text:fixed attribute specifies whether or not the value of a field element is fixed. If
        // the value of a field is fixed, the value of the field element to which this attribute is
        // attached is preserved in all future edits of the document. If the value of the field is not
        // fixed, the value of the field may be replaced by a new value when the document is edited.
        m_fixed = element.attributeNS(KOdfXmlNS::text, "fixed", QString()) == "true";
    } else if (localName == "page-continuation-string") {
        m_type = PageContinuation;

        // This attribute specifies whether to check for a previous or next page and if the page exists, the
        // continuation text is printed.
        QString pageselect = element.attributeNS(KOdfXmlNS::text, "select-page");
        if (pageselect == "previous")
            m_pageselect = KTextPage::PreviousPage;
        else if (pageselect == "next")
            m_pageselect = KTextPage::NextPage;
        else
            m_pageselect = KTextPage::CurrentPage;

        // The text to display
        m_continuation = element.text();
    }
    return true;
}
