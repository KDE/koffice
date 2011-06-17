/*
* This file is part of the KDE project
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: Amit Aggarwal <amit.5.aggarwal@nokia.com>
*
* Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#include "PresentationVariable.h"

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KProperties.h>
#include <kdebug.h>
#include <KShape.h>
#include <KShapeSavingContext.h>
#include <KShapeLoadingContext.h>
#include <KTextShapeData.h>
#include <KOdfXmlNS.h>
#include <KoPATextPage.h>
#include <SCPage.h>

PresentationVariable::PresentationVariable()
    : KoVariable(true)
    , m_type(SCDeclarations::Footer)
{
}

void PresentationVariable::setProperties(const KProperties *props)
{
    switch (props->intProperty("vartype")) {
    case 1:
        m_type = SCDeclarations::Header;
        break;
    case 2:
        m_type = SCDeclarations::Footer;
        break;
    case 3:
        m_type = SCDeclarations::DateTime;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

void PresentationVariable::positionChanged()
{
    if (KoPATextPage *textPage = dynamic_cast<KoPATextPage*>(page())) {
        if (SCPage *page = dynamic_cast<SCPage*>(textPage->page())) {
            setValue(page->declaration(m_type));
        }
    }
}

void PresentationVariable::saveOdf(KShapeSavingContext & context)
{
    KXmlWriter *writer = &context.xmlWriter();
    const char * type = "";
    switch (m_type) {
    case SCDeclarations::Footer:
        type = "presentation:footer";
        break;
    case SCDeclarations::Header:
        type = "presentation:header";
        break;
    case SCDeclarations::DateTime:
        type = "presentation:date-time";
        break;
    }
    writer->startElement(type);
    writer->endElement();
}

bool PresentationVariable::loadOdf(const KXmlElement & element, KShapeLoadingContext & context)
{
    Q_UNUSED(context);
    const QString localName(element.localName());

    if (localName == "footer") {
        m_type = SCDeclarations::Footer;
    }
    else if (localName == "header") {
        m_type = SCDeclarations::Header;
    }
    else if (localName == "date-time") {
        m_type = SCDeclarations::DateTime;
    }
    return true;
}
