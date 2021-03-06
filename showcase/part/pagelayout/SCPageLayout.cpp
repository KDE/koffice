/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCPageLayout.h"

#include <QBuffer>
#include <kdebug.h>

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KoPASavingContext.h>
#include "SCPlaceholder.h"

#include <KoPAView.h>
#include <QSize>
#include <QPainter>
#include <QPixmap>
#include <kiconloader.h>
#include <QSvgRenderer>

SCPageLayout::SCPageLayout()
: m_layoutType(Page)
{
}

SCPageLayout::~SCPageLayout()
{
    qDeleteAll(m_placeholders);
}

bool SCPageLayout::loadOdf(const KXmlElement &element, const QRectF &pageRect)
{
    if (element.hasAttributeNS(KOdfXmlNS::style, "display-name")) {
        m_name = element.attributeNS(KOdfXmlNS::style, "display-name");
    }
    else {
        m_name = element.attributeNS(KOdfXmlNS::style, "name");
    }

    KXmlElement child;
    forEachElement(child, element) {
        if (child.tagName() == "placeholder" && child.namespaceURI() == KOdfXmlNS::presentation) {
            SCPlaceholder * placeholder = new SCPlaceholder;
            if (placeholder->loadOdf(child, pageRect)) {
                m_placeholders.append(placeholder);
                if (placeholder->presentationObject() == "handout") {
                    m_layoutType = Handout;
                }
            }
            else {
                kWarning(33000) << "loading placeholder failed";
                delete placeholder;
            }
        }
        else {
            kWarning(33000) << "unknown tag" << child.namespaceURI() << child.tagName() << "when loading page layout";
        }
    }

    bool retval = true;
    if (m_placeholders.isEmpty()) {
        kWarning(33000) << "no placeholder for page layout" << m_name << "found";
        retval = false;
    }
    else {
        /* 
         * do fixups for wrong saved data from OO somehow they save negative values for width and height somethimes
         * <style:presentation-page-layout style:name="AL10T12">
         *   <presentation:placeholder presentation:object="title" svg:x="2.057cm" svg:y="1.743cm" svg:width="23.911cm" svg:height="3.507cm"/>
         *   <presentation:placeholder presentation:object="outline" svg:x="2.057cm" svg:y="5.838cm" svg:width="11.669cm" svg:height="13.23cm"/>
         *   <presentation:placeholder presentation:object="object" svg:x="14.309cm" svg:y="5.838cm" svg:width="-0.585cm" svg:height="6.311cm"/>
         *   <presentation:placeholder presentation:object="object" svg:x="14.309cm" svg:y="12.748cm" svg:width="-0.585cm" svg:height="-0.601cm"/>
         * </style:presentation-page-layout>
         */
        QList<SCPlaceholder *>::iterator it(m_placeholders.begin());
        SCPlaceholder * last = *it;
        ++it;
        for (; it != m_placeholders.end(); ++it) {
            (*it)->fix(last->rect(QSizeF(1, 1)));
            last = *it;
        }
    }
    return retval;
}

QString SCPageLayout::saveOdf(KoPASavingContext &context) const
{
    KOdfGenericStyle style(KOdfGenericStyle::PresentationPageLayoutStyle);

    style.addAttribute("style:display-name", m_name);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KXmlWriter elementWriter(&buffer);

    QList<SCPlaceholder *>::const_iterator it(m_placeholders.begin());
    for (; it != m_placeholders.end(); ++it) {
        (*it)->saveOdf(elementWriter);
    }

    QString placeholders = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    style.addChildElement("placeholders", placeholders);

    // return the style name so we can save the ptr -> style in the saving context so the pages can use it during saving
    return context.mainStyles().insert(style, "pl");
}

QList<SCPlaceholder *> SCPageLayout::placeholders() const
{
    return m_placeholders;
}

QPixmap SCPageLayout::thumbnail() const
{
    static KIconLoader * loader = KIconLoader::global();

    QSvgRenderer renderer;

    QSize size(80, 60);
    QPixmap pic(size);
    pic.fill();
    QPainter p(&pic);

    QString file = loader->iconPath("layout-elements", KIconLoader::User);
    if (renderer.load(file)) {
        QList<SCPlaceholder *>::const_iterator it(m_placeholders.begin());
        for (; it != m_placeholders.end(); ++it) {
            kDebug(33001) << "-----------------" <<(*it)->presentationObject() << (*it)->rect(size);
            renderer.render(&p, (*it)->presentationObject(), (*it)->rect(size));
        }
    }
    else {
        kWarning(33001) << "could not load" << file;
    }

    return pic;
}

SCPageLayout::Type SCPageLayout::type() const
{
    return m_layoutType;
}

bool comparePlaceholder(const SCPlaceholder * p1, const SCPlaceholder * p2)
{
    return (* p1) < (* p2);
}

bool SCPageLayout::operator<(const SCPageLayout &other) const
{
    if (m_placeholders.size() == other.m_placeholders.size()) {
        QList<SCPlaceholder *> placeholders(m_placeholders);
        QList<SCPlaceholder *> otherPlaceholders(other.m_placeholders);
        qSort(placeholders.begin(), placeholders.end(), comparePlaceholder);
        qSort(otherPlaceholders.begin(), otherPlaceholders.end(), comparePlaceholder);

        QList<SCPlaceholder *>::iterator it(placeholders.begin());
        QList<SCPlaceholder *>::iterator otherIt(otherPlaceholders.begin());
        kDebug(33001) << "SCPageLayout::operator< start" << (*it)->rect(QSizeF(1, 1)) << (*otherIt)->rect(QSizeF(1, 1));

        for (; it != placeholders.end(); ++it, ++otherIt) {
            kDebug(33001) << "SCPageLayout::operator<" << (*it)->rect(QSizeF(1, 1)) << (*otherIt)->rect(QSizeF(1, 1));
            if (*(*it) == *(*otherIt)) {
                kDebug(33001) << "SCPageLayout::operator< 0" << (*(*it) < *(*otherIt));
                continue;
            }
            kDebug(33001) << "SCPageLayout::operator< 1" << (*(*it) < *(*otherIt));
            return *(*it) < *(*otherIt);
        }
        kDebug(33001) << "SCPageLayout::operator< 2" << false;
        return false;
        // sort of the different placeholders by position and type
    }
    kDebug(33001) << "SCPageLayout::operator< 3" << (m_placeholders.size() < other.m_placeholders.size());
    return m_placeholders.size() < other.m_placeholders.size();
}

bool comparePlaceholderByPosition(const SCPlaceholder * p1, const SCPlaceholder * p2)
{
    return SCPlaceholder::comparePosition(*p1,* p2);
}

bool SCPageLayout::compareByContent(const SCPageLayout &pl1, const SCPageLayout &pl2)
{
    if (pl1.m_placeholders.size() == pl2.m_placeholders.size()) {
        QList<SCPlaceholder *> placeholders(pl1.m_placeholders);
        QList<SCPlaceholder *> otherPlaceholders(pl2.m_placeholders);
        qSort(placeholders.begin(), placeholders.end(), comparePlaceholderByPosition);
        qSort(otherPlaceholders.begin(), otherPlaceholders.end(), comparePlaceholderByPosition);

        QList<SCPlaceholder *>::iterator it(placeholders.begin());
        QList<SCPlaceholder *>::iterator otherIt(otherPlaceholders.begin());

        for (; it != placeholders.end(); ++it, ++otherIt) {
            QString presentationObject1 = (*it)->presentationObject();
            QString presentationObject2 = (*otherIt)->presentationObject();
            if (presentationObject1 == presentationObject2) {
                continue;
            }
            return presentationObject1 < presentationObject2;
        }
        return false;
    }
    return pl1.m_placeholders.size() < pl2.m_placeholders.size();
}
