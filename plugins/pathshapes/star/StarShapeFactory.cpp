/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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

#include "star/StarShapeFactory.h"
#include "star/StarShape.h"
#include "star/StarShapeConfigWidget.h"

#include <KShapeFactoryBase.h>
#include <KLineBorder.h>
#include <KProperties.h>
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KColorBackground.h>
#include <KShapeLoadingContext.h>

#include <klocale.h>

StarShapeFactory::StarShapeFactory(QObject *parent)
    : KShapeFactoryBase(parent, StarShapeId, i18n("A star shape"))
{
    setToolTip(i18n("A star"));
    setIcon("star");
    QStringList elementNames;
    elementNames << "regular-polygon" << "custom-shape";
    setOdfElementNames(KOdfXmlNS::draw, elementNames);
    setLoadingPriority(5);

    KoShapeTemplate t;
    t.id = KoPathShapeId;
    t.templateId = "star";
    t.name = i18n("Star");
    t.family = "geometric";
    t.toolTip = i18n("A star");
    t.icon = "star-shape";
    KProperties *props = new KProperties();
    props->setProperty("corners", 5);
    QVariant v;
    v.setValue(QColor(Qt::yellow));
    props->setProperty("background", v);
    t.properties = props;
    addTemplate(t);

    t.id = KoPathShapeId;
    t.templateId = "flower";
    t.name = i18n("Flower");
    t.family = "funny";
    t.toolTip = i18n("A flower");
    t.icon = "flower-shape";
    props = new KProperties();
    props->setProperty("corners", 5);
    props->setProperty("baseRadius", 10.0);
    props->setProperty("tipRadius", 50.0);
    props->setProperty("baseRoundness", 0.0);
    props->setProperty("tipRoundness", 40.0);
    v.setValue(QColor(Qt::magenta));
    props->setProperty("background", v);
    t.properties = props;
    addTemplate(t);

    t.id = KoPathShapeId;
    t.templateId = "pentagon";
    t.name = i18n("Pentagon");
    t.family = "geometric";
    t.toolTip = i18n("A pentagon");
    t.icon = "pentagon-shape";
    props = new KProperties();
    props->setProperty("corners", 5);
    props->setProperty("convex", true);
    props->setProperty("tipRadius", 50.0);
    props->setProperty("tipRoundness", 0.0);
    v.setValue(QColor(Qt::blue));
    props->setProperty("background", v);
    t.properties = props;
    addTemplate(t);

    t.id = KoPathShapeId;
    t.templateId = "hexagon";
    t.name = i18n("Hexagon");
    t.family = "geometric";
    t.toolTip = i18n("A hexagon");
    t.icon = "hexagon-shape";
    props = new KProperties();
    props->setProperty("corners", 6);
    props->setProperty("convex", true);
    props->setProperty("tipRadius", 50.0);
    props->setProperty("tipRoundness", 0.0);
    v.setValue(QColor(Qt::blue));
    props->setProperty("background", v);
    t.properties = props;
    addTemplate(t);
}

KShape *StarShapeFactory::createDefaultShape(KResourceManager *) const
{
    StarShape *star = new StarShape();

    star->setBorder(new KLineBorder(1.0));
    star->setShapeId(KoPathShapeId);

    return star;
}

KShape *StarShapeFactory::createShape(const KProperties *params, KResourceManager *) const
{
    StarShape *star = new StarShape();
    if (! star)
        return 0;

    star->setCornerCount(params->intProperty("corners", 5));
    star->setConvex(params->boolProperty("convex", false));
    star->setBaseRadius(params->doubleProperty("baseRadius", 25.0));
    star->setTipRadius(params->doubleProperty("tipRadius", 50.0));
    star->setBaseRoundness(params->doubleProperty("baseRoundness", 0.0));
    star->setTipRoundness(params->doubleProperty("tipRoundness", 0.0));
    star->setBorder(new KLineBorder(1.0));
    star->setShapeId(KoPathShapeId);
    QVariant v;
    if (params->property("background", v))
        star->setBackground(new KColorBackground(v.value<QColor>()));

    return star;
}

bool StarShapeFactory::supports(const KXmlElement &e, KShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    if (e.localName() == "regular-polygon" && e.namespaceURI() == KOdfXmlNS::draw)
        return true;
    return (e.localName() == "custom-shape" && e.namespaceURI() == KOdfXmlNS::draw
            && e.attributeNS(KOdfXmlNS::draw, "engine", "") == "koffice:star");
}

KShapeConfigWidgetBase *StarShapeFactory::createConfigWidget(KCanvasBase *canvas)
{
    return new StarShapeConfigWidget(canvas);
}

#include <StarShapeFactory.moc>
