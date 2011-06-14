/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCPlaceholderStrategy.h"

#include "SCPlaceholderPictureStrategy.h"
#include "SCPlaceholderTextStrategy.h"

#include <QPainter>
#include <QPen>
#include <QTextOption>

#include <klocale.h>
#include <KoShape.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeRegistry.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KXmlWriter.h>
#include <kdebug.h>

static const class PlaceholderData {
    public:
    const char * m_presentationClass;
    const char * m_shapeId;
    const char * m_xmlElement;
    const char * m_text;
} placeholderData[] = {
    { "title", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add a title") },
    { "outline", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add an outline") },
    { "subtitle", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add a text") },
    { "text", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add a text") },
    { "notes", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add notes") },
    /*
    { "date-time", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add data/time") },
    { "footer", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add footer") },
    { "header", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add header") },
    { "page-number", "TextShapeID", "<draw:text-box/>", I18N_NOOP("Double click to add page number") },
    */
    { "graphic", "PictureShape", "<draw:image xlink:href=\"\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>", 
                                       I18N_NOOP("Double click to add a picture") },
    { "chart", "ChartShape", "<draw:object xlink:href=\"\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>",
                                       I18N_NOOP("Double click to add a chart") },
    { "object", "ChartShape", "<draw:object xlink:href=\"\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>",
                                       I18N_NOOP("Double click to add an object") }
};

static QMap<QString, const PlaceholderData *> s_placeholderMap;

void fillPlaceholderMap()
{
    const unsigned int numPlaceholderData = sizeof(placeholderData) / sizeof(*placeholderData);
    for (unsigned int i = 0; i < numPlaceholderData; ++i) {
        s_placeholderMap.insert(placeholderData[i].m_presentationClass, &placeholderData[i]);
    }
}

SCPlaceholderStrategy * SCPlaceholderStrategy::create(const QString &presentationClass)
{
    if (s_placeholderMap.isEmpty()) {
        fillPlaceholderMap();
    }

    SCPlaceholderStrategy * strategy = 0;
    if (presentationClass == "graphic") {
        strategy = new SCPlaceholderPictureStrategy();
    }
    // TODO make nice
    else if (presentationClass == "outline" || presentationClass == "title" || presentationClass == "subtitle") {
        strategy = new SCPlaceholderTextStrategy(presentationClass);
    }
    else {
        if (s_placeholderMap.contains(presentationClass)) {
            strategy = new SCPlaceholderStrategy(presentationClass);
        }
        else {
            kWarning(33001) << "Unsupported placeholder strategy:" << presentationClass;
        }
    }
    return strategy;
}

bool SCPlaceholderStrategy::supported(const QString &presentationClass)
{
    if (s_placeholderMap.isEmpty()) {
        fillPlaceholderMap();
    }

    return s_placeholderMap.contains(presentationClass);
}

SCPlaceholderStrategy::SCPlaceholderStrategy(const QString &presentationClass)
: m_placeholderData(s_placeholderMap[presentationClass])
{
}

SCPlaceholderStrategy::~SCPlaceholderStrategy()
{
}

KoShape *SCPlaceholderStrategy::createShape(KResourceManager *rm)
{
    KoShape * shape = 0;
    KoShapeFactoryBase * factory = KoShapeRegistry::instance()->value(m_placeholderData->m_shapeId);
    if (factory) {
        shape = factory->createDefaultShape(rm);
    }
    return shape;
}

void SCPlaceholderStrategy::paint(QPainter &painter, const KoViewConverter &converter, const QRectF &rect)
{
    KoShape::applyConversion(painter, converter);
    QPen penText(Qt::black);
    painter.setPen(penText);
    //painter.setFont()
    QTextOption options(Qt::AlignCenter);
    options.setWrapMode(QTextOption::WordWrap);
    painter.drawText(rect, text(), options);

    QPen pen(Qt::gray);
    //pen.setStyle(Qt::DashLine); // endless loop
    //pen.setStyle(Qt::DotLine); // endless loop
    //pen.setStyle(Qt::DashDotLine); // endless loop
    painter.setPen(pen);
    painter.drawRect(rect);
}

void SCPlaceholderStrategy::saveOdf(KoShapeSavingContext &context)
{
    KXmlWriter &writer = context.xmlWriter();
    writer.addCompleteElement(m_placeholderData->m_xmlElement);
}

bool SCPlaceholderStrategy::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return true;
}

QString SCPlaceholderStrategy::text() const
{
    return i18n(m_placeholderData->m_text);
}

void SCPlaceholderStrategy::init(KResourceManager *)
{
}

KoShapeUserData * SCPlaceholderStrategy::userData() const
{
    return 0;
}
