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

#include "SCPlaceholderShape.h"

#include <QPainter>
#include <QTextOption>
#include <KoShapeSavingContext.h>
#include <KoViewConverter.h>
#include <KoXmlWriter.h>
#include <KoOdfWorkaround.h>

#include "SCPlaceholderStrategy.h"

SCPlaceholderShape::SCPlaceholderShape()
: m_strategy(0)
{
}

SCPlaceholderShape::SCPlaceholderShape(const QString & presentationClass)
: m_strategy(0)
{
    m_strategy = SCPlaceholderStrategy::create(presentationClass);
}

SCPlaceholderShape::~SCPlaceholderShape()
{
    delete m_strategy;
}

void SCPlaceholderShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    QRectF rect(QPointF(0, 0), size());
    if (m_strategy) {
        m_strategy->paint(painter, converter, rect);
    }
    else {
        applyConversion(painter, converter);
        QPen pen(Qt::gray);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);
        painter.drawRect(rect);
    }
}

bool SCPlaceholderShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);
#ifndef NWORKAROUND_ODF_BUGS
    KoOdfWorkaround::fixPresentationPlaceholder(this);
#endif

    delete m_strategy;

    m_strategy = SCPlaceholderStrategy::create(additionalAttribute("presentation:class"));
    if (m_strategy == 0) {
        return false;
    }
    m_strategy->loadOdf(element, context);

    return true;
}

void SCPlaceholderShape::saveOdf(KoShapeSavingContext & context) const
{
    KoXmlWriter & writer = context.xmlWriter();
    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    if (m_strategy) {
        m_strategy->saveOdf(context);
    }
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw:frame
}

KoShape *SCPlaceholderShape::createShape(KoResourceManager *documentResources)
{
    Q_ASSERT(m_strategy);
    KoShape * shape = 0;
    if (m_strategy) {
        shape = m_strategy->createShape(documentResources);
    }
    return shape;
}

void SCPlaceholderShape::initStrategy(KoResourceManager *documentResources)
{
    Q_ASSERT(m_strategy);
    if (m_strategy) {
        m_strategy->init(documentResources);
    }
}

KoShapeUserData * SCPlaceholderShape::userData() const
{
    Q_ASSERT(m_strategy);
    return m_strategy ? m_strategy->userData() : 0;
}
