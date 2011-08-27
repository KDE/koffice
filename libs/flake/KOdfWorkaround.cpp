/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Johannes Simon <johannes.simon@gmail.com>
   Copyright (C) 2010 Jan Hambrecht <jaham@gmx.net>

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

#include "KOdfWorkaround.h"

#include "KShapeLoadingContext.h"
#include "KShape.h"
#include <KPathShape.h>
#include <KOdfLoadingContext.h>
#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KColorBackground.h>
#include <KOdfStyleStack.h>

#include <QPen>
#include <QColor>

#include <kdebug.h>

static bool s_workaroundPresentationPlaceholderBug = false;

void KOdfWorkaround::fixPenWidth(QPen & pen, KShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KOdfLoadingContext::OpenOffice && pen.widthF() == 0.0) {
        pen.setWidthF(0.5);
        kDebug(30006) << "Work around OO bug with pen width 0";
    }
}

void KOdfWorkaround::fixEnhancedPath(QString & path, const KXmlElement &element, KShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KOdfLoadingContext::OpenOffice) {
        if (path.isEmpty() && element.attributeNS(KOdfXmlNS::draw, "type", "") == "ellipse") {
            path = "U 10800 10800 10800 10800 0 360 Z N";
        }
    }
}

void KOdfWorkaround::fixEnhancedPathPolarHandlePosition(QString &position, const KXmlElement &element, KShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KOdfLoadingContext::OpenOffice) {
        if (element.hasAttributeNS(KOdfXmlNS::draw, "handle-polar")) {
            QStringList tokens = position.simplified().split(' ');
            if (tokens.count() == 2) {
                position = tokens[1] + ' ' + tokens[0];
            }
        }
    }
}

QColor KOdfWorkaround::fixMissingFillColor(const KXmlElement &element, KShapeLoadingContext &context)
{
    // Default to an invalid color
    QColor color;

    if (element.prefix() == "chart") {
        KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();

        bool hasStyle = element.hasAttributeNS(KOdfXmlNS::chart, "style-name");
        if (hasStyle) {
            context.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::chart, "style-name", "chart");
            styleStack.setTypeProperties("graphic");
        }

        if (context.odfLoadingContext().generatorType() == KOdfLoadingContext::OpenOffice) {
            if (hasStyle && !styleStack.hasProperty(KOdfXmlNS::draw, "fill") &&
                             styleStack.hasProperty(KOdfXmlNS::draw, "fill-color")) {
                color = QColor(styleStack.property(KOdfXmlNS::draw, "fill-color"));
            } else if (!hasStyle || (!styleStack.hasProperty(KOdfXmlNS::draw, "fill")
                                    && !styleStack.hasProperty(KOdfXmlNS::draw, "fill-color"))) {
                KXmlElement plotAreaElement = element.parentNode().toElement();
                KXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "wall") {
                    if (chartElement.hasAttributeNS(KOdfXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KOdfXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if (chartType == "chart:line" ||
                             chartType == "chart:area" ||
                             chartType == "chart:bar" ||
                             chartType == "chart:scatter")
                        color = QColor(0xe0e0e0);
                    }
                } else if (element.tagName() == "series") {
                    if (chartElement.hasAttributeNS(KOdfXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KOdfXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if (chartType == "chart:area" ||
                             chartType == "chart:bar")
                            color = QColor(0x99ccff);
                    }
                }
                else if (element.tagName() == "chart")
                    color = QColor(0xffffff);
            }
        }

        styleStack.restore();
    }

    return color;
}

bool KOdfWorkaround::fixMissingStroke(QPen &pen, const KXmlElement &element, KShapeLoadingContext &context, const KShape *shape)
{
    bool fixed = false;

    KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
    if (element.prefix() == "chart") {
        styleStack.save();

        bool hasStyle = element.hasAttributeNS(KOdfXmlNS::chart, "style-name");
        if (hasStyle) {
            context.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::chart, "style-name", "chart");
            styleStack.setTypeProperties("graphic");
        }

        if (context.odfLoadingContext().generatorType() == KOdfLoadingContext::OpenOffice) {
            if (hasStyle && styleStack.hasProperty(KOdfXmlNS::draw, "stroke") &&
                            !styleStack.hasProperty(KOdfXmlNS::draw, "stroke-color")) {
                fixed = true;
                pen.setColor(Qt::black);
            } else if (!hasStyle) {
                KXmlElement plotAreaElement = element.parentNode().toElement();
                KXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "series") {
                    if (chartElement.hasAttributeNS(KOdfXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KOdfXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if (chartType == "chart:line" ||
                             chartType == "chart:scatter") {
                            fixed = true;
                            pen = QPen(0x99ccff);
                        }
                    }
                } else if (element.tagName() == "legend") {
                    fixed = true;
                    pen = QPen(Qt::black);
                }
            }
        }

        styleStack.restore();
    } else {
        const KPathShape *pathShape = dynamic_cast<const KPathShape*>(shape);
        if (pathShape) {
            const QString strokeColor(styleStack.property(KOdfXmlNS::draw, "stroke-color"));
            if (strokeColor.isEmpty()) {
                pen.setColor(Qt::black);
            } else {
                pen.setColor(styleStack.property(KOdfXmlNS::svg, "stroke-color"));
            }
            fixed = true;
        }
    }

    return fixed;
}

bool KOdfWorkaround::fixMissingStyle_DisplayLabel(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_UNUSED(element);
    // If no axis style is specified, OpenOffice.org hides the axis' data labels
    if (context.odfLoadingContext().generatorType() == KOdfLoadingContext::OpenOffice)
        return false;

    // In all other cases, they're visible
    return true;
}

void KOdfWorkaround::setFixPresentationPlaceholder(bool fix, KShapeLoadingContext &context)
{
    KOdfLoadingContext::GeneratorType type(context.odfLoadingContext().generatorType());
    if (type == KOdfLoadingContext::OpenOffice || type == KOdfLoadingContext::MicrosoftOffice) {
        s_workaroundPresentationPlaceholderBug = fix;
    }
}

bool KOdfWorkaround::fixPresentationPlaceholder()
{
    return s_workaroundPresentationPlaceholderBug;
}

void KOdfWorkaround::fixPresentationPlaceholder(KShape *shape)
{
    if (s_workaroundPresentationPlaceholderBug && !shape->hasAdditionalAttribute("presentation:placeholder")) {
        shape->setAdditionalAttribute("presentation:placeholder", "true");
    }
}

KColorBackground *KOdfWorkaround::fixBackgroundColor(const KShape *shape, KShapeLoadingContext &context)
{
    KColorBackground *colorBackground = 0;
    KOdfLoadingContext &odfContext = context.odfLoadingContext();
    if (odfContext.generatorType() == KOdfLoadingContext::OpenOffice) {
        const KPathShape *pathShape = dynamic_cast<const KPathShape*>(shape);
        //check shape type
        if (pathShape) {
            KOdfStyleStack &styleStack = odfContext.styleStack();
            const QString color(styleStack.property(KOdfXmlNS::draw, "fill-color"));
            if (color.isEmpty()) {
                colorBackground = new KColorBackground(QColor(153, 204, 255));
            } else { 
                colorBackground = new KColorBackground(color);
            }
        }
    }
    return colorBackground;
}
